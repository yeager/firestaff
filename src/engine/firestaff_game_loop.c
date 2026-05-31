
#include "firestaff_game_loop.h"
#include "firestaff_l10n.h"
#include "firestaff_asset_pipeline.h"
#include "dm1_v1_input_command_queue_pc34_compat.h"
#include "dm1_v1_event_timer_pc34_compat.h"
#include "dm1_v1_sensor_trigger_pc34_compat.h"
#include "dm1_v1_viewport_3d_pc34_compat.h"
#include "dm1_v1_combat_pc34_compat.h"
#include "firestaff_input.h"
#include "firestaff_sdl_bridge.h"
#include "firestaff_save.h"
#include "firestaff_graphics_dat_reader.h"
#include "firestaff_wall_graphics.h"
#include "firestaff_dungeon_query.h"
#include "dm1_v2_anim_timing.h"
#include "csb_v1_viewport_pc34_compat.h"
#include "csb_v1_boot.h"
#include "csb_v1_dungeon_loader_pc34_compat.h"
#include "dm2_v1_boot.h"
#include "dm2_v2_runtime.h"
#include "dm2_v1_runtime.h"
#include <string.h>
#include <stdio.h>

/* ══════════════════════════════════════════════════════════════════════
 * Firestaff Game Loop — the core integration layer
 *
 * This connects:
 *   1. Asset loading (GRAPHICS.DAT, DUNGEON.DAT)
 *   2. V1 game engine (dm1_v1_game_loop_pc34_compat)
 *   3. V2 rendering pipeline (EPX upscale → SDL present)
 *   4. Input handling (SDL events → V1 command queue)
 *   5. Save/load system
 *
 * Frame timing:
 *   V1 tick: every 55ms (18.2 Hz) — game state advances
 *   Render: every ~16ms (60 Hz) — visual interpolation
 *   Input: polled each render frame, queued for next V1 tick
 * ══════════════════════════════════════════════════════════════════════ */

static V2_AnimClock g_clock;

/* Global FS_GameState instance — initialized by fs_game_init() and used
 * by firestaff_touch.c to bridge swipe gestures to the V1 input queue.
 * NULL until fs_game_init() is called. */
static FS_GameState *g_fs_state = NULL;

FS_InputQueue *fs_g_input_queue_get(void) {
    return g_fs_state ? &g_fs_state->input_queue : NULL;
}


/* ══════════════════════════════════════════════════════════════════════
 * #1: V1 viewport rendering — dungeon data → indexed framebuffer
 *
 * Flow per render frame:
 *   1. Read party position + direction from game state
 *   2. Query dungeon squares in view cone (D0-D3, left/center/right)
 *   3. Call V1 wall/floor/ceiling draw functions per square
 *   4. Write to 320x200 indexed framebuffer
 *   5. EPX upscale (V2.1) or direct present (V1)
 * ══════════════════════════════════════════════════════════════════════ */

#define FS_FB_W 320
#define FS_FB_H 200
#define FS_VP_W 224
#define FS_VP_H 136
#define FS_VP_X 0
#define FS_VP_Y 0

static uint8_t g_framebuffer[FS_FB_W * FS_FB_H];
static uint32_t g_rgba_buffer[FS_FB_W * 4 * FS_FB_H * 4]; /* up to 4x */
static uint32_t g_vga_palette[256];
static int g_palette_loaded = 0;

/* Default DM1 VGA palette (first 16 colors for testing) */
static extern const uint32_t g_dm1_vga_palette[16];
extern void fs_dm1_get_full_palette(uint32_t *out256);

void fs_init_default_palette(void) {
    if (g_palette_loaded) return;
    /* Use real DM1 VGA palette */
    fs_dm1_get_full_palette(g_vga_palette);
    g_palette_loaded = 1;
    return; /* skip hardcoded palette below */
    if (g_palette_loaded) return;
    g_vga_palette[0]  = 0xFF000000; /* black */
    g_vga_palette[1]  = 0xFF000088; /* dark blue */
    g_vga_palette[2]  = 0xFF008800; /* dark green */
    g_vga_palette[3]  = 0xFF008888; /* dark cyan */
    g_vga_palette[4]  = 0xFF880000; /* dark red */
    g_vga_palette[5]  = 0xFF880088; /* dark magenta */
    g_vga_palette[6]  = 0xFF885500; /* brown */
    g_vga_palette[7]  = 0xFFAAAAAA; /* light gray */
    g_vga_palette[8]  = 0xFF555555; /* dark gray */
    g_vga_palette[9]  = 0xFF5555FF; /* blue */
    g_vga_palette[10] = 0xFF55FF55; /* green */
    g_vga_palette[11] = 0xFF55FFFF; /* cyan */
    g_vga_palette[12] = 0xFFFF5555; /* red */
    g_vga_palette[13] = 0xFFFF55FF; /* magenta */
    g_vga_palette[14] = 0xFFFFFF55; /* yellow */
    g_vga_palette[15] = 0xFFFFFFFF; /* white */
    /* Fill rest with grays */
    for (int i = 16; i < 256; i++) {
        uint8_t v = (uint8_t)(i);
        g_vga_palette[i] = 0xFF000000 | ((uint32_t)v << 16) | ((uint32_t)v << 8) | v;
    }
    g_palette_loaded = 1;
}

/* Render a simple first-person dungeon view based on party position.
 * This is the bridge between game state and pixels.
 *
 * For FS_GAME_CSB: delegates to csb_v1_viewport_render_frame() which
 *   uses the DM1 V1 viewport engine with CSB-specific wall sets.
 * For other games: uses the existing placeholder rendering.
 */
static void fs_game_render_viewport(FS_GameState *state) {
    int x, y, px, py, dir;
    if (!state) return;
    px = state->party_x;
    py = state->party_y;
    dir = state->party_direction;

    fs_init_default_palette();

    /* Clear framebuffer */
    memset(g_framebuffer, 0, sizeof(g_framebuffer));

    /* ── CSB path: wire viewport into CSB V1 renderer ── */
    if (state->config.game == FS_GAME_CSB) {
        /* Configure CSB viewport with the global framebuffer.
         * The viewport occupies rows [DM1_VIEWPORT_SCREEN_Y..168]
         * within the 320×200 screen: rows 33..168 inclusive. */
        CSB_V1_ViewportConfig *cv = &state->csb_viewport;
        cv->viewport_pixels = g_framebuffer;
        cv->viewport_stride  = FS_FB_W;  /* 320 bytes/row */

        /* Wire dungeon grid for wall/door decision making.
         * Build from the live CSB dungeon on each render call.
         * When no dungeon is loaded, fall back to the test maze pattern
         * so the view cone has valid data even without CSB DUNGEON.DAT.
         * Grid layout matches DM1_Viewport3DState: grid[y*width+x] = raw
         * square type (low 5 bits of 16-bit record).  The current level
         * comes from csb_v1_dungeon_get_current_level().
         *
         * Source: ReDMCSB DUNGEON.C F0151 column-major layout;
         *   csb_v1_dungeon_loader_pc34_compat.c level_widths/level_heights;
         *   csb_v1_dungeon_get_square_type() for element-type routing */
        static uint8_t s_csb_dungeon[32*32];
        {
            const CSB_V1_DungeonData *dun = csb_v1_dungeon_get_current();
            if (dun && dun->raw_data && dun->level_count > 0) {
                int level = csb_v1_dungeon_get_current_level();
                level = (level >= 0 && level < dun->level_count) ? level : 0;
                int w = dun->level_widths[level];
                int h = dun->level_heights[level];
                int max_w = (w <= 32) ? w : 32;
                int max_h = (h <= 32) ? h : 32;
                for (int gy = 0; gy < max_h; gy++)
                    for (int gx = 0; gx < max_w; gx++)
                        s_csb_dungeon[gy*32+gx] = (uint8_t)csb_v1_dungeon_get_square_type(
                            dun, level, gx, gy);
                /* Mark out-of-bounds cells as WALL (0) */
                for (int gy = max_h; gy < 32; gy++)
                    for (int gx = 0; gx < 32; gx++)
                        s_csb_dungeon[gy*32+gx] = 0;
                for (int gy = 0; gy < max_h; gy++)
                    for (int gx = max_w; gx < 32; gx++)
                        s_csb_dungeon[gy*32+gx] = 0;
            } else {
                /* Fallback test maze — corridor pattern for no-dungeon case */
                for (int my = 0; my < 32; my++)
                    for (int mx = 0; mx < 32; mx++)
                        s_csb_dungeon[my*32+mx] = ((mx+my)%3==0 || mx==0 || my==0 || mx==31 || my==31) ? 0 : 1;
            }
        }
        cv->dungeon_grid  = s_csb_dungeon;
        cv->dungeon_width  = 32;
        cv->dungeon_height = 32;

        /* Delegate to the CSB viewport renderer (which calls
         * dm1_viewport_3d_draw_frame internally). */
        csb_v1_viewport_render_frame(cv, dir, px, py);

    } else if (state->config.game == FS_GAME_DM2) {
        /* ── DM2 V2 path: smooth movement + viewport renderer ── */
        DM2_V1_BootProfile *boot = (DM2_V1_BootProfile *)state->dm2_boot;
        if (boot && boot->dm2_state) {
            /* Phase 5: V2 smooth movement viewport rendering.
             * dm2_v2_runtime_render_frame updates smooth animation state
             * and renders with smooth-interpolated camera offset.
             * Source: Phase 5 runtime binding, dm2_v2_runtime.c */
            (void)dm2_v2_runtime_render_frame(state->party_direction,
                                               state->party_x, state->party_y,
                                               g_framebuffer, FS_FB_W,
                                               FS_VP_W, FS_VP_H);
        } else {
            /* DM2 boot not complete — render placeholder ceiling/floor */
            for (y = FS_VP_Y; y < FS_VP_Y + FS_VP_H / 2; y++)
                for (x = FS_VP_X; x < FS_VP_X + FS_VP_W; x++)
                    g_framebuffer[y * FS_FB_W + x] = 4;  /* dark red ceiling */
            for (y = FS_VP_Y + FS_VP_H / 2; y < FS_VP_Y + FS_VP_H; y++)
                for (x = FS_VP_X; x < FS_VP_X + FS_VP_W; x++)
                    g_framebuffer[y * FS_FB_W + x] = 6;  /* brown floor */
        }

    } else {

    /* Blit viewport background from GRAPHICS.DAT #0 (224x136) */
    if (g_assets_ready) {
        static uint8_t gfx_extract_bg[224 * 136];
        static int bg_loaded = 0;
        if (!bg_loaded) {
            int bw = 0, bh = 0;
            if (fs_gfx_get_bitmap(&g_gfx_dat, 0, gfx_extract_bg,
                    sizeof(gfx_extract_bg), &bw, &bh) > 0) {
                bg_loaded = 1;
            }
        }
        if (bg_loaded) {
            for (y = 0; y < FS_VP_H && y < 136; y++)
                for (x = 0; x < FS_VP_W && x < 224; x++)
                    g_framebuffer[(FS_VP_Y + y) * FS_FB_W + FS_VP_X + x] =
                        gfx_extract_bg[y * 224 + x];
        } else {
            /* Fallback: gray ceiling + brown floor */
            for (y = FS_VP_Y; y < FS_VP_Y + FS_VP_H / 2; y++)
                for (x = FS_VP_X; x < FS_VP_X + FS_VP_W; x++)
                    g_framebuffer[y * FS_FB_W + x] = 8;
            for (y = FS_VP_Y + FS_VP_H / 2; y < FS_VP_Y + FS_VP_H; y++)
                for (x = FS_VP_X; x < FS_VP_X + FS_VP_W; x++)
                    g_framebuffer[y * FS_FB_W + x] = 6;
        }
    } else {
        for (y = FS_VP_Y; y < FS_VP_Y + FS_VP_H / 2; y++)
            for (x = FS_VP_X; x < FS_VP_X + FS_VP_W; x++)
                g_framebuffer[y * FS_FB_W + x] = 8;
        for (y = FS_VP_Y + FS_VP_H / 2; y < FS_VP_Y + FS_VP_H; y++)
            for (x = FS_VP_X; x < FS_VP_X + FS_VP_W; x++)
                g_framebuffer[y * FS_FB_W + x] = 6;
    }

    /* Compute view cone from party position */
    FS_ViewCone view_cone;
    memset(&view_cone, 0, sizeof(view_cone));
    /* Dungeon data is loaded via M11 game view / tick orchestrator.
     * The legacy game loop fallback generates a test maze pattern. */
    {
        static uint8_t test_dungeon[32*32];
        static int maze_init = 0;
        if (!maze_init) {
            for (int my = 0; my < 32; my++)
                for (int mx = 0; mx < 32; mx++)
                    test_dungeon[my*32+mx] = ((mx+my)%3==0 || mx==0 || my==0 || mx==31 || my==31) ? 0 : 1;
            maze_init = 1;
        }
        fs_dungeon_compute_view_cone(test_dungeon, 32, 32, px, py, dir, &view_cone);
    }

    /* Draw walls using real GRAPHICS.DAT bitmaps when available */
    {
        int fx = px, fy = py;
        int ddx[] = {0, 1, 0, -1};
        int ddy[] = {-1, 0, 1, 0};
        fx += ddx[dir]; fy += ddy[dir];

        if (view_cone.has_wall[0][1]) { /* D0 center has wall */
            if (g_assets_ready) {
                /* Blit real wall bitmap from GRAPHICS.DAT */
                /* Wall set graphics start around index 30-60 in DM1 */
                /* D1C front wall = graphic index varies by wall set */
                static uint8_t wall_pixels[16384];
                int ww = 0, wh = 0;
                int wall_gfx_idx = 33; /* D1C front wall in default set */
                int got = fs_gfx_extract_bitmap(&g_gfx_dat, wall_gfx_idx,
                    wall_pixels, sizeof(wall_pixels), &ww, &wh);
                if (got > 0 && ww > 0 && wh > 0) {
                    /* Blit wall bitmap centered in viewport */
                    int ox = FS_VP_X + (FS_VP_W - ww) / 2;
                    int oy = FS_VP_Y + (FS_VP_H - wh) / 2;
                    for (int by = 0; by < wh && oy + by < FS_VP_Y + FS_VP_H; by++) {
                        for (int bx = 0; bx < ww && ox + bx < FS_VP_X + FS_VP_W; bx++) {
                            uint8_t pixel = wall_pixels[by * ww + bx];
                            if (pixel != 0) /* skip transparent */
                                g_framebuffer[(oy + by) * FS_FB_W + (ox + bx)] = pixel;
                        }
                    }
                } else {
                    /* Fallback: solid gray wall */
                    for (y = FS_VP_Y + 20; y < FS_VP_Y + FS_VP_H - 20; y++)
                        for (x = FS_VP_X + 40; x < FS_VP_X + FS_VP_W - 40; x++)
                            g_framebuffer[y * FS_FB_W + x] = 7;
                }
            } else {
                /* No assets: solid gray wall */
                for (y = FS_VP_Y + 20; y < FS_VP_Y + FS_VP_H - 20; y++)
                    for (x = FS_VP_X + 40; x < FS_VP_X + FS_VP_W - 40; x++)
                        g_framebuffer[y * FS_FB_W + x] = 7;
            }
        }
    }

    /* Draw HUD panel from GRAPHICS.DAT #1 (320x200, use bottom 64 rows) */
    if (g_assets_ready) {
        static uint8_t gfx_extract_hud[320 * 200];
        static int hud_loaded = 0;
        if (!hud_loaded) {
            int hw = 0, hh = 0;
            if (fs_gfx_get_bitmap(&g_gfx_dat, 1, gfx_extract_hud,
                    sizeof(gfx_extract_hud), &hw, &hh) > 0 && hw == 320) {
                hud_loaded = 1;
            }
        }
        if (hud_loaded) {
            for (y = FS_VP_H; y < FS_FB_H; y++)
                for (x = 0; x < FS_FB_W; x++)
                    g_framebuffer[y * FS_FB_W + x] =
                        gfx_extract_hud[y * 320 + x];
        } else {
            for (y = FS_VP_H; y < FS_FB_H; y++)
                for (x = 0; x < FS_FB_W; x++)
                    g_framebuffer[y * FS_FB_W + x] = 1;
        }
    } else {
        for (y = FS_VP_H; y < FS_FB_H; y++)
            for (x = 0; x < FS_FB_W; x++)
                g_framebuffer[y * FS_FB_W + x] = 1;
    }

    /* Draw compass indicator */
    {
        const char *dirs[] = {"N", "E", "S", "W"};
        int cx = 288, cy = FS_VP_H + 10;
        (void)dirs; (void)cx; (void)cy; (void)dir;
        /* Text rendering would go here */
    }

    /* Draw position text (debug) */
    /* Would use dm1_v1_text_message for proper rendering */
}

/* Convert indexed framebuffer to RGBA using palette */
static void fs_framebuffer_to_rgba(int scale) {
    if (scale == 1) {
        for (int i = 0; i < FS_FB_W * FS_FB_H; i++)
            g_rgba_buffer[i] = g_vga_palette[g_framebuffer[i]];
    } else {
        /* EPX-like nearest neighbor for now */
        int dw = FS_FB_W * scale, dh = FS_FB_H * scale;
        for (int y = 0; y < dh; y++)
            for (int x = 0; x < dw; x++) {
                int sx = x / scale, sy = y / scale;
                g_rgba_buffer[y * dw + x] = g_vga_palette[g_framebuffer[sy * FS_FB_W + sx]];
            }
    }
}


/* ── Startup error helper ─────────────────────────────────────────── */
static void fs_set_error(FS_StartupError *err, int code,
                         const char *msg, const char *detail, const char *suggestion) {
    if (!err) return;
    err->code = code;
    snprintf(err->message, FS_ERROR_MSG_MAX, "%s", msg ? msg : "Unknown error");
    snprintf(err->detail, FS_ERROR_MSG_MAX, "%s", detail ? detail : "");
    snprintf(err->suggestion, FS_ERROR_MSG_MAX, "%s", suggestion ? suggestion : "");
}

int fs_game_init(FS_GameState *state, const FS_GameConfig *config) {
    if (!state || !config) {
        if (state) fs_set_error(&state->last_error, -1,
            "Internal error: NULL state or config pointer",
            "fs_game_init called with NULL argument",
            "This is a programming error — report as a bug");
        return -1;
    }
    memset(state, 0, sizeof(*state));
    state->config = *config;
    state->running = 1;
    state->in_menu = 0;
    state->party_direction = 0; /* North */
    v2_anim_clock_init(&g_clock);
    g_fs_state = state;

    /* Set default window size based on version */
    if (state->config.window_width <= 0) {
        switch (config->version) {
            case FS_VERSION_V1:  state->config.window_width = 320; state->config.window_height = 200; break;
            case FS_VERSION_V21: state->config.window_width = 640; state->config.window_height = 400; break;
            case FS_VERSION_V22: state->config.window_width = 1280; state->config.window_height = 800; break;
        }
    }

        /* Auto-detect system language and set UI + asset language */
    fs_l10n_init_from_system();
    printf("Firestaff: language=%s\n", fs_l10n_language_name(fs_l10n_get_language()));

    if (!config->skip_menu) {
        printf("Firestaff: init game=%d version=%d %dx%d\n",
            config->game, config->version,
            state->config.window_width, state->config.window_height);
    }

    /* ── CSB V1 boot profile init ── */
    if (state->config.game == FS_GAME_CSB) {
        static CSB_V1_BootProfile s_csb_boot;
        csb_v1_boot_profile_init(&s_csb_boot);
        if (state->config.data_dir) {
            (void)csb_v1_boot_scan_assets(&s_csb_boot, state->config.data_dir);
        } else {
            (void)csb_v1_boot_scan_assets(&s_csb_boot, "~/.firestaff/data");
        }
        if (state->config.save_dir) {
            csb_v1_boot_set_save_root(&s_csb_boot, state->config.save_dir);
        } else {
            csb_v1_boot_set_save_root(&s_csb_boot, NULL);
        }
        (void)csb_v1_boot_enter_game(&s_csb_boot);
        csb_v1_boot_print_summary(&s_csb_boot);
        state->csb_boot = (void *)&s_csb_boot;
        if (!config->skip_menu) {
            char diag[1024];
            size_t dn = csb_v1_boot_diagnostic_report(&s_csb_boot, diag, sizeof(diag));
            if (dn > 0 && dn < sizeof(diag)) {
                printf("%.*s", (int)dn, diag);
            }
        }
    }

    /* ── DM2 V1 boot profile init ── */
    if (state->config.game == FS_GAME_DM2) {
        static DM2_V1_BootProfile s_dm2_boot;
        dm2_v1_boot_profile_init(&s_dm2_boot);
        /* Scan assets in data_dir/dm2/ */
        if (state->config.data_dir) {
            char scan_dir[512];
            snprintf(scan_dir, sizeof(scan_dir), "%s/dm2", state->config.data_dir);
            (void)dm2_v1_boot_scan_assets(&s_dm2_boot, scan_dir);
        } else {
            (void)dm2_v1_boot_scan_assets(&s_dm2_boot, "~/.firestaff/data/dm2");
        }
        /* Set save root */
        if (state->config.save_dir) {
            dm2_v1_boot_set_save_root(&s_dm2_boot, state->config.save_dir);
        } else {
            dm2_v1_boot_set_save_root(&s_dm2_boot, NULL);
        }
        dm2_v1_boot_print_summary(&s_dm2_boot);
        /* Enter game: allocate dungeon data and DM2 game state */
        (void)dm2_v1_boot_enter_game(&s_dm2_boot);
        /* Phase 5: init DM2 V2 runtime (smooth movement + V2 viewport).
         * Scale 2 = V2.0 EPX mode.  Source: dm2_v2_runtime.c */
        dm2_v2_runtime_init(2);
        /* Store in state */
        state->dm2_boot = (void *)&s_dm2_boot;
        /* Print diagnostics */
        if (!config->skip_menu) {
            char diag[1024];
            size_t dn = dm2_v1_diagnostic_report(&s_dm2_boot, diag, sizeof(diag));
            if (dn > 0 && dn < sizeof(diag)) {
                printf("%.*s", (int)dn, diag);
            }
        }
    }

    return 0;
}

int fs_game_load_assets(FS_GameState *state) {
    if (!state) return -1;
    memset(&state->last_error, 0, sizeof(state->last_error));
    /* Load GRAPHICS.DAT and DUNGEON.DAT based on game */
    if (!state->config.skip_menu) {
        printf("Firestaff: loading assets for game %d from %s\n",
            state->config.game, state->config.data_dir ? state->config.data_dir : "(default)");
    }

    /* Load assets per game */
    static FS_GraphicsDat g_gfx_dat;
    static int g_assets_ready = 0;
    {
        const char *game_subdirs[] = {"dm1", "csb", "dm2", "nexus"};
        const char *subdir = (state->config.game >= 0 && state->config.game < 4)
            ? game_subdirs[state->config.game] : "dm1";
        printf("Firestaff: loading %s assets from %s/%s\n",
            subdir, state->config.data_dir ? state->config.data_dir : ".", subdir);
    }

    /* Start position is set from DUNGEON.DAT header when available.
     * The M11 game view handles full asset loading via F0882. */
    /* DM1 Hall of Champions start position.
     * Source: ReDMCSB ENTRANCE.C — party enters at south end of hall.
     * Level 0 = entrance/Hall of Champions.
     * mapX=11, mapY=29, facing North (toward champion mirrors). */
    state->current_level = 0;
    /* Start position from DUNGEON.DAT header (ReDMCSB LOADSAVE.C:1941-1944) */
    if (fs_dungeon_get_width() > 0) {
        state->party_x = fs_dungeon_get_start_x();
        state->party_y = fs_dungeon_get_start_y();
        state->party_direction = fs_dungeon_get_start_dir();
        printf("Start: (%d,%d) facing %d (from DUNGEON.DAT)\n",
            state->party_x, state->party_y, state->party_direction);
    } else {
        state->party_x = 11;
        state->party_y = 29;
        state->party_direction = 0;
        printf("Start: (11,29) facing North (fallback)\n");
    }

    /* Load GRAPHICS.DAT and parse bitmap headers */
    if (state->config.data_dir) {
        FS_AssetBundle bundle;
        const char *game_subdirs[] = {"dm1", "csb", "dm2", "nexus"};
        const char *subdir = (state->config.game >= 0 && state->config.game < 4)
            ? game_subdirs[state->config.game] : "dm1";
        if (fs_assets_load_game(&bundle, state->config.data_dir, subdir) == 0) {
            fs_gfx_load(&g_gfx_dat, bundle.graphics_data, bundle.graphics_size);
            if (bundle.graphics_data) {
                fs_gfx_get_palette(&g_gfx_dat, g_vga_palette);
                g_palette_loaded = 1;
                g_assets_ready = 1;
                /* Load DUNGEON.DAT */
                if (bundle.dungeon_data && bundle.dungeon_size > 0) {
                    fs_dungeon_load_dat(bundle.dungeon_data, bundle.dungeon_size);
                    fs_dungeon_set_level(state->current_level);
                    /* Set start position from DUNGEON.DAT header AFTER loading */
                    state->party_x = fs_dungeon_get_start_x();
                    state->party_y = fs_dungeon_get_start_y();
                    state->party_direction = fs_dungeon_get_start_dir();
                    printf("Firestaff: start (%d,%d) facing %d from DUNGEON.DAT
",
                        state->party_x, state->party_y, state->party_direction);
                }
                printf("Firestaff: %d graphics loaded from GRAPHICS.DAT\n",
                    g_gfx_dat.graphic_count);
            }
        }
    }

    return 0;
}

void fs_game_tick_v1(FS_GameState *state, uint32_t now_ms) {
    if (!state || state->paused) return;

    /* V1 game tick — process one game logic frame */
    /* 1. Process input queue → V1 command queue */
    {
        FS_InputEvent evt;
        while (fs_input_queue_pop(&state->input_queue, &evt)) {
            switch (evt.cmd) {
                case FS_CMD_MOVE_FORWARD:
                    state->party_y--;
                    break;
                case FS_CMD_MOVE_BACKWARD:
                    state->party_y++;
                    break;
                case FS_CMD_TURN_LEFT:
                    state->party_direction = (state->party_direction + 3) & 3;
                    break;
                case FS_CMD_TURN_RIGHT:
                    state->party_direction = (state->party_direction + 1) & 3;
                    break;
                case FS_CMD_MENU:
                    state->paused = !state->paused;
                    break;
                default: break;
            }
        }
    }
    /* 2. Process timers/events */
    /* dm1v1_event_process_tick() — when fully wired */
    /* 3. Creature AI */
    /* dm1_creature_ai_tick() — when fully wired */
    /* 4. Apply pending damage */
    /* dm1_combat_apply_pending_damage_pc34() — when fully wired */

    /* DM2 V1: delegate to DM2 runtime tick if DM2 boot profile is active */
    if (state->config.game == FS_GAME_DM2 && state->dm2_boot) {
        dm2_v1_runtime_tick();
        /* Phase 5: advance V2 smooth animation clock on V1 boundary.
         * Source: dm2_v2_runtime.c */
        dm2_v2_runtime_v1_tick(now_ms);
    }

    state->frame_count++;
}

void fs_game_render_v2(FS_GameState *state) {
    if (!state) return;

    float sub_tick = v2_anim_clock_sub_tick(&g_clock);
    (void)sub_tick;
    /* 1-2: Render V1 viewport to indexed framebuffer */
    fs_game_render_viewport(state);
    /* 3: Convert to RGBA (V1=1x, V2.1=2x, V2.2=4x) */
    {
        int scale = (state->config.version == FS_VERSION_V1) ? 1 :
                    (state->config.version == FS_VERSION_V21) ? 2 : 4;
        fs_framebuffer_to_rgba(scale);
    }
    /* 4-6: SDL present (via bridge) */
    /* fs_sdl_present_rgba(&g_sdl, g_rgba_buffer, w, h); */
}

void fs_game_handle_sdl_event(FS_GameState *state, const void *sdl_event) {
    if (!state || !sdl_event) return;
    /* SDL event → V1 command queue translation
     * Arrow keys / WASD → movement commands
     * Mouse click → viewport click / inventory click
     * Escape → menu toggle */
}

void fs_game_run(FS_GameState *state) {
    if (!state) return;
    printf("Firestaff: entering game loop\n");

    /* Frame pacing: target ~60fps (16.67ms per frame).
     * Game logic ticks at V1_TICK_MS (~55ms) matching the original
     * DM1 VBlank-driven game speed.
     * Use wall-clock delta time so game speed is independent of
     * frame rate. */
#ifdef HAVE_SDL3
    uint64_t last_ticks = SDL_GetTicks();
#else
    uint32_t last_frame_ms = 0;
#endif
    const uint32_t TARGET_FRAME_MS = 16; /* ~60fps display rate */

    while (state->running) {
#ifdef HAVE_SDL3
        uint64_t current_ticks = SDL_GetTicks();
        uint32_t delta_ms = (uint32_t)(current_ticks - last_ticks);
        last_ticks = current_ticks;
        /* Clamp delta to avoid spiral-of-death on hitches */
        if (delta_ms > 200) delta_ms = 200;
        uint32_t now_ms = (uint32_t)current_ticks;
#else
        uint32_t delta_ms = 16; /* headless: assume 60fps */
        uint32_t now_ms = state->frame_count * 16;
#endif

        /* Accumulate real elapsed time for V1 game logic ticks */
        state->v1_tick_accumulator_ms += delta_ms;

        /* Process V1 ticks at original rate (ReDMCSB VBlank timing) */
        while (state->v1_tick_accumulator_ms >= V1_TICK_MS) {
            fs_game_tick_v1(state, now_ms);
            v2_anim_clock_v1_tick(&g_clock, now_ms);
            state->v1_tick_accumulator_ms -= V1_TICK_MS;
        }

        /* Render at display rate */
        v2_anim_clock_render_frame(&g_clock, now_ms);
        fs_game_render_v2(state);

#ifdef HAVE_SDL3
        /* SDL event poll */
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            fs_game_handle_sdl_event(state, &e);
        }

        /* Frame pacing: sleep until target frame time */
        uint64_t frame_end = SDL_GetTicks();
        uint32_t frame_elapsed = (uint32_t)(frame_end - current_ticks);
        if (frame_elapsed < TARGET_FRAME_MS) {
            SDL_Delay(TARGET_FRAME_MS - frame_elapsed);
        }
#else
        /* Headless: break after 100 frames */
        if (state->frame_count > 100) break;
#endif
    }

    printf("Firestaff: game loop exited after %u frames\n", state->frame_count);
}

void fs_game_shutdown(FS_GameState *state) {
    if (!state) return;
    printf("Firestaff: shutdown\n");
    state->running = 0;
}

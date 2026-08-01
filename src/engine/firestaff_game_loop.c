
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
#include "csb_v2_filter_config_pc34.h"
#include "firestaff_save.h"
#include "fs_portable_compat.h"
#include "firestaff_graphics_dat_reader.h"
#include "firestaff_wall_graphics.h"
#include "firestaff_dungeon_query.h"
#include "dm1_v2_anim_timing.h"
#include "csb_v1_viewport_pc34_compat.h"
#include "csb_v1_boot.h"
#include "csb_v1_dungeon_loader_pc34_compat.h"
#include "csb_v1_viewport_wall_ornament_ordinal_resolver_pc34_compat.h"
#include "dm2_v1_boot.h"
#include "dm2_v1_runtime.h"
#include "dm2_v2_hud_runtime.h"
#include "dm2_v2_phase_gate.h"
#include "nexus_v2_hud_runtime.h"
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
extern const uint32_t g_dm1_vga_palette[16];
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
 * Other games must bind their own source-backed renderer.  This legacy
 * integration point deliberately leaves a cleared frame instead of
 * manufacturing a dungeon from test geometry or substitute pixels.
 */
static void fs_game_render_viewport(FS_GameState *state) {
    int px, py, dir;
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

        /* Wire the loader-owned dungeon grid for wall/door decisions.
         * ReDMCSB F0128 draws from the active dungeon; a missing handoff
         * therefore leaves the cleared frame rather than inventing a map. */
        static uint8_t s_csb_dungeon[32*32];
        const CSB_V1_DungeonData *dun = csb_v1_dungeon_get_current();
        if (!csb_v1_viewport_bind_live_dungeon_grid(
                cv, dun, csb_v1_dungeon_get_current_level(), s_csb_dungeon)) {
            return;
        }

        /* Wire wall ornament ordinal resolver from live dungeon data. */
        static CSB_V1_WallOrnamentOrdinalResolverPc34 s_csb_ornament_resolver;
        s_csb_ornament_resolver.dungeon = dun;
        s_csb_ornament_resolver.level = csb_v1_dungeon_get_current_level();
        s_csb_ornament_resolver.randomWallOrnamentCount = 0;
        s_csb_ornament_resolver.ornamentRandomSeed = 0;
        cv->wall_ornament_ordinal_callback =
            csb_v1_viewport_wall_ornament_ordinal_resolve_pc34;
        cv->wall_ornament_ordinal_user_data = &s_csb_ornament_resolver;

        /* Wire floor ornament metadata for F0108 rendering. */
        {
            int lvl = csb_v1_dungeon_get_current_level();
            if (dun && lvl >= 0 && lvl < dun->level_count) {
                cv->floor_ornament_random_count =
                    dun->map_random_floor_ornament_count[lvl];
                cv->floor_ornament_index_table =
                    dun->map_floor_ornament_indices[lvl];
                cv->floor_ornament_index_table_count =
                    dun->map_floor_ornament_count[lvl];
                cv->ornament_random_seed = dun->ornament_random_seed;
            } else {
                cv->floor_ornament_random_count = 0;
                cv->floor_ornament_index_table = NULL;
                cv->floor_ornament_index_table_count = 0;
                cv->ornament_random_seed = 0;
            }
        }

        /* Delegate to the CSB viewport renderer (which calls
         * dm1_viewport_3d_draw_frame internally). */
        csb_v1_viewport_render_frame(cv, dir, px, py);

    } else if (state->config.game == FS_GAME_DM2) {
        /* DM2 presentation is owned by the M11 GDAT route. */
        DM2_V1_BootProfile *boot = (DM2_V1_BootProfile *)state->dm2_boot;
        if (boot && boot->dm2_state) {
            /* Phase 3: V2 HUD overlay (gated on phase gate).
             * Renders compass, depth, gold, champion bars, action strip
             * on top of the V1 viewport.  No-op when V1 is active
             * (framebuffer preserved for V1 chrome). */
            dm2_v2_hud_runtime_render(g_framebuffer, FS_FB_W, FS_FB_H);
        }
    } else {
        /* A game without a live, source-backed renderer remains blank here.
         * Never substitute a procedural maze, wall, HUD, or palette. */
        return;
    }
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
        char resolvedDataDir[FSP_PATH_MAX];
        const char* scanRoot = state->config.data_dir;
        csb_v1_boot_profile_init(&s_csb_boot);
        if (!scanRoot || !scanRoot[0]) {
            if (FSP_ResolveDataDir(resolvedDataDir,
                                   sizeof(resolvedDataDir),
                                   NULL)) {
                scanRoot = resolvedDataDir;
            }
        }
        if (csb_v1_boot_scan_assets(&s_csb_boot,
                                    scanRoot && scanRoot[0] ? scanRoot : NULL) != 0 ||
            !s_csb_boot.assets_verified || !s_csb_boot.graphics_verified ||
            !s_csb_boot.dungeon_verified || !s_csb_boot.graphics_path[0] ||
            !s_csb_boot.dungeon_path[0]) {
            fs_set_error(&state->last_error, -2,
                         "CSB original data required",
                         "Hash-verified CSB GRAPHICS.DAT and DUNGEON.DAT were not both found",
                         "Install a supported original CSB data set and select its data directory");
            state->running = 0;
            state->csb_boot = NULL;
            g_fs_state = NULL;
            return -1;
        }
        if (state->config.save_dir) {
            csb_v1_boot_set_save_root(&s_csb_boot, state->config.save_dir);
        } else {
            csb_v1_boot_set_save_root(&s_csb_boot, NULL);
        }
        if (csb_v1_boot_enter_game(&s_csb_boot) != 0 ||
            s_csb_boot.state != CSB_V1_BOOT_STATE_RUNTIME_READY ||
            !s_csb_boot.runtime.dungeon_handle ||
            csb_v1_dungeon_get_current() != s_csb_boot.runtime.dungeon_handle) {
            fs_set_error(&state->last_error, -2,
                         "CSB original game load failed",
                         "The verified CSB data could not materialize a source dungeon runtime",
                         "Check the original data set and restart from the launcher");
            csb_v1_boot_cleanup(&s_csb_boot);
            state->running = 0;
            state->csb_boot = NULL;
            g_fs_state = NULL;
            return -1;
        }
        csb_v1_boot_print_summary(&s_csb_boot);
        state->csb_boot = (void *)&s_csb_boot;
        state->current_level = s_csb_boot.runtime.current_level;
        state->party_x = s_csb_boot.runtime.party_x;
        state->party_y = s_csb_boot.runtime.party_y;
        state->party_direction = s_csb_boot.runtime.party_dir;
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
        char resolvedDataDir[FSP_PATH_MAX];
        const char* scanRoot = state->config.data_dir;
        dm2_v1_boot_profile_init(&s_dm2_boot);
        if (!scanRoot || !scanRoot[0]) {
            if (FSP_ResolveDataDir(resolvedDataDir,
                                   sizeof(resolvedDataDir),
                                   NULL)) {
                scanRoot = resolvedDataDir;
            }
        }
        if (scanRoot && scanRoot[0]) {
            (void)dm2_v1_boot_scan_assets(&s_dm2_boot, scanRoot);
        }
        /* Set save root */
        if (state->config.save_dir) {
            dm2_v1_boot_set_save_root(&s_dm2_boot, state->config.save_dir);
        } else {
            dm2_v1_boot_set_save_root(&s_dm2_boot, NULL);
        }
        /* This legacy loop is not the M11 launch path, but it remains a
         * public direct-start seam. SKProject INIT reaches GAME_LOAD only
         * after both original media owners are available. Do not initialize
         * a DM2 runtime from a missing, filename-shaped, or mixed corpus: a
         * blank/diagnostic frame is not a playable substitute either. */
        if (!s_dm2_boot.assets_verified || !s_dm2_boot.graphics_path[0] ||
            !s_dm2_boot.dungeon_path[0]) {
            fs_set_error(&state->last_error, -2,
                         "DM2 original data required",
                         "Hash-verified DM2 GRAPHICS.DAT and DUNGEON.DAT were not both found",
                         "Install a supported original DM2 data set and select its data directory");
            state->running = 0;
            state->dm2_boot = NULL;
            g_fs_state = NULL;
            return -1;
        }
        dm2_v1_boot_print_summary(&s_dm2_boot);
        /* Enter game only after the verified source pair has been admitted. */
        if (dm2_v1_boot_enter_game(&s_dm2_boot) != 0) {
            fs_set_error(&state->last_error, -2,
                         "DM2 original game load failed",
                         "The verified DM2 data could not complete the source boot handoff",
                         "Check the original data set and restart from the launcher");
            state->running = 0;
            state->dm2_boot = NULL;
            g_fs_state = NULL;
            return -1;
        }
        dm2_v1_runtime_init(&s_dm2_boot);
        if (s_dm2_boot.graphics_dat) {
            dm2_v1_runtime_set_viewport_asset_provider(
                dm2_v1_boot_viewport_asset_fetch, &s_dm2_boot);
        }
        /* Phase 3: init DM2 V2 HUD runtime (compass, depth, gold,
         * champion bars, action strip).  Gated on phase gate.
         * Source: dm2_v2_hud_runtime.c */
        dm2_v2_hud_runtime_init();
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
    /* The direct CSB loop owns its only admissible source handoff in the
     * CSB boot profile.  Re-parsing the data through the generic dungeon
     * reader would both duplicate the load and restore that reader's DM1
     * fallback coordinate path.  ReDMCSB LOADSAVE.C F0435 materializes the
     * selected CSB dungeon before the first view, which csb_v1_boot_enter_game
     * already did above. */
    if (state->config.game == FS_GAME_CSB) {
        const CSB_V1_BootProfile *boot =
            (const CSB_V1_BootProfile *)state->csb_boot;
        if (!boot || boot->state != CSB_V1_BOOT_STATE_RUNTIME_READY ||
            !boot->runtime.dungeon_handle ||
            csb_v1_dungeon_get_current() != boot->runtime.dungeon_handle) {
            fs_set_error(&state->last_error, -2,
                         "CSB original game load failed",
                         "No materialized CSB dungeon is available for the direct game loop",
                         "Return to the launcher and select verified original CSB data");
            state->running = 0;
            return -1;
        }
        state->current_level = boot->runtime.current_level;
        state->party_x = boot->runtime.party_x;
        state->party_y = boot->runtime.party_y;
        state->party_direction = boot->runtime.party_dir;
        return 0;
    }
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
                    printf("Firestaff: start (%d,%d) facing %d from DUNGEON.DAT\n",
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
    }

    state->frame_count++;
}

void fs_game_render_v2(FS_GameState *state) {
    if (!state) return;

    float sub_tick = v2_anim_clock_sub_tick(&g_clock);
    (void)sub_tick;
    /* 1-2: Render V1 viewport to indexed framebuffer */
    fs_game_render_viewport(state);
    /* 2.5: CSB V2.0 indexed filter chain (dither + palette interp).
     * Applied to the indexed framebuffer after the V1 render and
     * before the indexed-to-RGBA conversion. Only runs for CSB V2.0
     * (other games don't have a CSB V2 filter config). */
    if (state->config.game == FS_GAME_CSB) {
        (void)csb_v2_filter_chain_apply_indexed(g_framebuffer, FS_FB_W, FS_FB_H);
    }
    /* 3: Convert to RGBA (V1=1x, V2.1=2x, V2.2=4x) */
    {
        int scale = (state->config.version == FS_VERSION_V1) ? 1 :
                    (state->config.version == FS_VERSION_V21) ? 2 : 4;
        int w = FS_FB_W * scale;
        int h = FS_FB_H * scale;
        fs_framebuffer_to_rgba(scale);
        /* 3.5: CSB V2.0 RGBA filter chain (CRT scanlines).
         * Applied to the RGBA surface after the indexed-to-RGBA
         * conversion. The csb_v2_filter_chain_apply_rgba reads
         * csb_v2_filter_config_get() to decide whether to apply. */
        if (state->config.game == FS_GAME_CSB) {
            (void)csb_v2_filter_chain_apply_rgba((uint8_t*)g_rgba_buffer, w, h);
        }
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

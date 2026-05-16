
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
 * This is the bridge between game state and pixels. */
static void fs_game_render_viewport(FS_GameState *state) {
    int x, y, px, py, dir;
    if (!state) return;
    px = state->party_x;
    py = state->party_y;
    dir = state->party_direction;

    fs_init_default_palette();

    /* Clear framebuffer */
    memset(g_framebuffer, 0, sizeof(g_framebuffer));

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
    /* TODO: wire to real dungeon data when DUNGEON.DAT is loaded */
    /* For now: generate a simple maze pattern */
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

int fs_game_init(FS_GameState *state, const FS_GameConfig *config) {
    if (!state || !config) return -1;
    memset(state, 0, sizeof(*state));
    state->config = *config;
    state->running = 1;
    state->in_menu = 0;
    state->party_direction = 0; /* North */
    v2_anim_clock_init(&g_clock);

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

    printf("Firestaff: init game=%d version=%d %dx%d\n",
        config->game, config->version,
        state->config.window_width, state->config.window_height);
    return 0;
}

int fs_game_load_assets(FS_GameState *state) {
    if (!state) return -1;
    /* Load GRAPHICS.DAT and DUNGEON.DAT based on game */
    printf("Firestaff: loading assets for game %d from %s\n",
        state->config.game, state->config.data_dir ? state->config.data_dir : "(default)");

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

    /* TODO: call dm1_v1_graphics_loader / dm1_v1_dungeon_loader
     * For now, set starting position */
    /* DM1 Hall of Champions start position.
     * Source: ReDMCSB ENTRANCE.C — party enters at south end of hall.
     * Level 0 = entrance/Hall of Champions.
     * mapX=11, mapY=29, facing North (toward champion mirrors). */
    state->current_level = 0;
    state->party_x = 11;
    state->party_y = 29;
    state->party_direction = 0; /* North */

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
                }
                printf("Firestaff: %d graphics loaded from GRAPHICS.DAT\n",
                    g_gfx_dat.graphic_count);
            }
        }
    }

    return 0;
}

void fs_game_tick_v1(FS_GameState *state) {
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

    while (state->running) {
        uint32_t now_ms = state->frame_count * 16; /* approximate */

        /* Accumulate time for V1 ticks */
        state->v1_tick_accumulator_ms += 16; /* ~60fps */

        /* Process V1 ticks at original rate */
        while (state->v1_tick_accumulator_ms >= V1_TICK_MS) {
            fs_game_tick_v1(state);
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
        /* Present frame */
        /* fs_sdl_present_rgba(&g_sdl, g_rgba_buffer, w, h); */
        SDL_Delay(1); /* yield CPU */
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


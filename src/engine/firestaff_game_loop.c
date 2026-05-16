
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
static void fs_init_default_palette(void) {
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

    /* Draw ceiling (top half of viewport = dark gray) */
    for (y = FS_VP_Y; y < FS_VP_Y + FS_VP_H / 2; y++)
        for (x = FS_VP_X; x < FS_VP_X + FS_VP_W; x++)
            g_framebuffer[y * FS_FB_W + x] = 8; /* dark gray ceiling */

    /* Draw floor (bottom half = brown) */
    for (y = FS_VP_Y + FS_VP_H / 2; y < FS_VP_Y + FS_VP_H; y++)
        for (x = FS_VP_X; x < FS_VP_X + FS_VP_W; x++)
            g_framebuffer[y * FS_FB_W + x] = 6; /* brown floor */

    /* Draw walls based on surroundings (simplified) */
    /* Front wall at D0C if blocked */
    {
        int fx = px, fy = py;
        int dx[] = {0, 1, 0, -1}; /* N, E, S, W */
        int dy[] = {-1, 0, 1, 0};
        fx += dx[dir]; fy += dy[dir];

        /* Simple wall detection: draw front wall if position is "wall" */
        /* In full implementation this queries dungeon.dat via dm1_v1_dungeon_loader */
        if ((fx + fy) % 3 == 0) { /* placeholder: some squares are walls */
            /* Draw front wall (gray rectangle in center) */
            for (y = FS_VP_Y + 20; y < FS_VP_Y + FS_VP_H - 20; y++)
                for (x = FS_VP_X + 40; x < FS_VP_X + FS_VP_W - 40; x++)
                    g_framebuffer[y * FS_FB_W + x] = 7; /* light gray wall */
        }
    }

    /* Draw HUD panel (bottom 64 rows) */
    for (y = FS_VP_H; y < FS_FB_H; y++)
        for (x = 0; x < FS_FB_W; x++)
            g_framebuffer[y * FS_FB_W + x] = 1; /* dark blue panel */

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

    /* Load assets with language from settings */
    {
        FS_AssetBundle assets;
        int asset_lang = fs_l10n_to_asset_language(fs_l10n_get_language());
        if (state->config.data_dir) {
            fs_assets_load_dm1_multilang(&assets, state->config.data_dir, (FS_AssetLanguage)asset_lang);
        }
    }

    /* TODO: call dm1_v1_graphics_loader / dm1_v1_dungeon_loader
     * For now, set starting position */
    state->current_level = 0;
    state->party_x = 5;
    state->party_y = 5;
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


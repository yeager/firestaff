
#include "firestaff_game_loop.h"
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

    /* TODO: call dm1_v1_graphics_loader / dm1_v1_dungeon_loader
     * For now, set starting position */
    state->current_level = 0;
    state->party_x = 5;
    state->party_y = 5;
    return 0;
}

void fs_game_tick_v1(FS_GameState *state) {
    if (!state || state->paused) return;

    /* This is where V1 game logic runs — one tick at original speed.
     * In full implementation:
     *   dm1_v1_game_loop_tick()
     *   dm1_v1_event_timer_process()
     *   dm1_v1_creature_ai_tick()
     *   dm1_v1_combat_apply_pending_damage()
     * For now: advance frame count */
    state->frame_count++;
}

void fs_game_render_v2(FS_GameState *state) {
    if (!state) return;

    /* V2 render pipeline:
     * 1. Get sub-tick for interpolation
     * 2. V1 engine renders to indexed framebuffer (320x200)
     * 3. EPX upscale (V2.1) or direct blit (V1)
     * 4. V2.2 overlays: particles, damage numbers, minimap, weather
     * 5. HUD panel render
     * 6. SDL present */
    (void)v2_anim_clock_sub_tick(&g_clock);
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

        /* In headless mode (no SDL), break after 100 frames */
        if (state->frame_count > 100 && !state->config.fullscreen) {
            break;
        }
    }

    printf("Firestaff: game loop exited after %u frames\n", state->frame_count);
}

void fs_game_shutdown(FS_GameState *state) {
    if (!state) return;
    printf("Firestaff: shutdown\n");
    state->running = 0;
}


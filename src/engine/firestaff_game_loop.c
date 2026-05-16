
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


/* DM1 V1 Game Loop Integration — source-locked from ReDMCSB
 * GAMELOOP.C F0002_MAIN_GameLoop_CPSDF: infinite loop with vblank sync
 * GAMELOOP.C G0318 = 10 (PC34 max vblank wait), G0317 vblank counter
 * VBLANK.C: vertical blank handler increments G0317
 * DECOMPDU.C G0523_B_RestartGameRequested: game restart flag */

#include "dm1_v1_game_loop_integration_pc34_compat.h"
#include <string.h>

void m11_gl_init(M11_GL_State* state) {
    if (!state) return;
    memset(state, 0, sizeof(M11_GL_State));
    state->current_state = M11_GL_STATE_INIT;
    state->timing.max_vblank_wait = 10;  /* G0318 PC34 value */
    state->timing.target_frame_ms = 55;  /* ~18.2 Hz PC timer tick */
    state->running = true;
}

void m11_gl_register_handler(M11_GL_State* state, M11_GL_GameState gs,
                              M11_GL_StateCallback on_enter,
                              M11_GL_StateCallback on_exit,
                              M11_GL_StateCallback on_update,
                              void* userdata) {
    if (!state || (int)gs < 0 || (int)gs >= 10) return;
    state->handlers[gs].on_enter = on_enter;
    state->handlers[gs].on_exit = on_exit;
    state->handlers[gs].on_update = on_update;
    state->handlers[gs].userdata = userdata;
}

/* State transition with enter/exit callbacks */
void m11_gl_set_state(M11_GL_State* state, M11_GL_GameState new_state) {
    if (!state || (int)new_state < 0 || (int)new_state >= 10) return;
    if (new_state == state->current_state) return;

    /* Exit current state */
    M11_GL_StateHandler* old_h = &state->handlers[state->current_state];
    if (old_h->on_exit) {
        old_h->on_exit(old_h->userdata);
    }

    M11_GL_GameState prev = state->current_state;
    state->current_state = new_state;
    (void)prev; /* Available for debugging */

    /* Enter new state */
    M11_GL_StateHandler* new_h = &state->handlers[new_state];
    if (new_h->on_enter) {
        new_h->on_enter(new_h->userdata);
    }
}

M11_GL_GameState m11_gl_get_state(const M11_GL_State* state) {
    if (!state) return M11_GL_STATE_INIT;
    return state->current_state;
}

/* F0002 pattern: one frame of the game loop
 * Original: infinite for(;;) with G0317 vblank wait + input + update
 * Here: called once per frame by the host platform */
void m11_gl_frame_update(M11_GL_State* state, uint32_t current_ms) {
    if (!state || !state->running || state->timing.paused) return;

    /* G0523 restart check — DECOMPDU.C pattern */
    if (state->restart_requested) {
        state->restart_requested = false;
        m11_gl_set_state(state, M11_GL_STATE_TITLE);
        return;
    }

    /* Handle deferred state transitions */
    if (state->transition_pending) {
        state->transition_pending = false;
        m11_gl_set_state(state, state->pending_state);
    }

    /* Frame timing — vblank sync equivalent
     * G0317 counter resets each frame; G0318 is max wait */
    uint32_t elapsed = current_ms - state->timing.last_frame_ms;
    if (elapsed < state->timing.target_frame_ms) {
        return; /* Not time for next frame yet */
    }
    state->timing.last_frame_ms = current_ms;
    state->timing.frame_count++;
    state->timing.vblank_count = 0; /* G0317 reset */

    /* Call current state's update handler */
    M11_GL_StateHandler* h = &state->handlers[state->current_state];
    if (h->on_update) {
        h->on_update(h->userdata);
    }
}

bool m11_gl_is_dungeon_running(const M11_GL_State* state) {
    return state && state->current_state == M11_GL_STATE_DUNGEON;
}

void m11_gl_request_restart(M11_GL_State* state) {
    if (!state) return;
    state->restart_requested = true; /* G0523 pattern */
}

void m11_gl_stop(M11_GL_State* state) {
    if (!state) return;
    state->running = false;
}

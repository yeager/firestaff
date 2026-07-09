/* DM1 V1 Game Loop Integration — source-locked from ReDMCSB
 * GAMELOOP.C F0002_MAIN_GameLoop_CPSDF: infinite loop with vblank sync
 * GAMELOOP.C G0318 = 10 (PC34 max vblank wait), G0317 vblank counter
 * VBLANK.C: vertical blank handler increments G0317
 * DECOMPDU.C G0523_B_RestartGameRequested: game restart flag */

#include "dm1_v1_game_loop_integration_pc34_compat.h"
#include <string.h>

void DM1_V1_LoopIntegration_InitPc34Compat(DM1_V1_LoopIntegrationStatePc34* state) {
    if (!state) return;
    memset(state, 0, sizeof(DM1_V1_LoopIntegrationStatePc34));
    state->current_state = DM1_V1_LOOP_INTEGRATION_STATE_INIT_PC34;
    state->timing.max_vblank_wait = 10;  /* G0318 PC34 value */
    state->timing.target_frame_ms = 55;  /* ~18.2 Hz PC timer tick */
    state->running = true;
}

void DM1_V1_LoopIntegration_RegisterHandlerPc34Compat(DM1_V1_LoopIntegrationStatePc34* state, DM1_V1_LoopIntegrationGameStatePc34 gs,
                              DM1_V1_LoopIntegrationStateCallbackPc34 on_enter,
                              DM1_V1_LoopIntegrationStateCallbackPc34 on_exit,
                              DM1_V1_LoopIntegrationStateCallbackPc34 on_update,
                              void* userdata) {
    if (!state || (int)gs < 0 || (int)gs >= 10) return;
    state->handlers[gs].on_enter = on_enter;
    state->handlers[gs].on_exit = on_exit;
    state->handlers[gs].on_update = on_update;
    state->handlers[gs].userdata = userdata;
}

/* State transition with enter/exit callbacks */
void DM1_V1_LoopIntegration_SetStatePc34Compat(DM1_V1_LoopIntegrationStatePc34* state, DM1_V1_LoopIntegrationGameStatePc34 new_state) {
    if (!state || (int)new_state < 0 || (int)new_state >= 10) return;
    if (new_state == state->current_state) return;

    /* Exit current state */
    DM1_V1_LoopIntegrationStateHandlerPc34* old_h = &state->handlers[state->current_state];
    if (old_h->on_exit) {
        old_h->on_exit(old_h->userdata);
    }

    DM1_V1_LoopIntegrationGameStatePc34 prev = state->current_state;
    state->current_state = new_state;
    (void)prev; /* Available for debugging */

    /* Enter new state */
    DM1_V1_LoopIntegrationStateHandlerPc34* new_h = &state->handlers[new_state];
    if (new_h->on_enter) {
        new_h->on_enter(new_h->userdata);
    }
}

DM1_V1_LoopIntegrationGameStatePc34 DM1_V1_LoopIntegration_GetStatePc34Compat(const DM1_V1_LoopIntegrationStatePc34* state) {
    if (!state) return DM1_V1_LOOP_INTEGRATION_STATE_INIT_PC34;
    return state->current_state;
}

/* F0002 pattern: one frame of the game loop
 * Original: infinite for(;;) with G0317 vblank wait + input + update
 * Here: called once per frame by the host platform */
void DM1_V1_LoopIntegration_FrameUpdatePc34Compat(DM1_V1_LoopIntegrationStatePc34* state, uint32_t current_ms) {
    if (!state || !state->running || state->timing.paused) return;

    /* G0523 restart check — DECOMPDU.C pattern */
    if (state->restart_requested) {
        state->restart_requested = false;
        DM1_V1_LoopIntegration_SetStatePc34Compat(state, DM1_V1_LOOP_INTEGRATION_STATE_TITLE_PC34);
        return;
    }

    /* Handle deferred state transitions */
    if (state->transition_pending) {
        state->transition_pending = false;
        DM1_V1_LoopIntegration_SetStatePc34Compat(state, state->pending_state);
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
    DM1_V1_LoopIntegrationStateHandlerPc34* h = &state->handlers[state->current_state];
    if (h->on_update) {
        h->on_update(h->userdata);
    }
}

bool DM1_V1_LoopIntegration_IsDungeonRunningPc34Compat(const DM1_V1_LoopIntegrationStatePc34* state) {
    return state && state->current_state == DM1_V1_LOOP_INTEGRATION_STATE_DUNGEON_PC34;
}

void DM1_V1_LoopIntegration_RequestRestartPc34Compat(DM1_V1_LoopIntegrationStatePc34* state) {
    if (!state) return;
    state->restart_requested = true; /* G0523 pattern */
}

void DM1_V1_LoopIntegration_StopPc34Compat(DM1_V1_LoopIntegrationStatePc34* state) {
    if (!state) return;
    state->running = false;
}

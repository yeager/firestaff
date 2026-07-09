/* DM1 V1 Dialog/Scroll Message System — source-locked from ReDMCSB
 * DIALOG.C G2062_DialogSetIndex = C0_DIALOG_SET_VIEWPORT
 * SCRLMGMT.C: message scroll management
 * TEXT.C: text rendering to screen */

#include "dm1_v1_dialog_scroll_pc34_compat.h"
#include <string.h>

void DM1_V1_Dialog_InitPc34Compat(DM1_V1_DialogStatePc34* state) {
    if (!state) return;
    memset(state, 0, sizeof(DM1_V1_DialogStatePc34));
    state->active_set = DM1_V1_DIALOG_SET_VIEWPORT_PC34;
    /* Default message bar at bottom of viewport (below 136px viewport area) */
    state->bar_x = 0;
    state->bar_y = 170;
    state->bar_w = 224;
    state->bar_h = 10;
}

void DM1_V1_Dialog_SetBarPositionPc34Compat(DM1_V1_DialogStatePc34* state, int16_t x, int16_t y,
                              int16_t w, int16_t h) {
    if (!state) return;
    state->bar_x = x;
    state->bar_y = y;
    state->bar_w = w;
    state->bar_h = h;
}

void DM1_V1_Dialog_SetActivePc34Compat(DM1_V1_DialogStatePc34* state, DM1_V1_DialogSetPc34 set) {
    if (!state) return;
    state->active_set = set;
}

bool DM1_V1_Dialog_PushMessagePc34Compat(DM1_V1_DialogStatePc34* state, const char* text, uint8_t color) {
    if (!state || !text) return false;
    if (state->count >= DM1_V1_DIALOG_MSG_QUEUE_SIZE_PC34) return false;

    DM1_V1_DialogMessagePc34* msg = &state->queue[state->tail];
    size_t len = strlen(text);
    if (len >= DM1_V1_DIALOG_MAX_MSG_LEN_PC34) len = DM1_V1_DIALOG_MAX_MSG_LEN_PC34 - 1;
    memcpy(msg->text, text, len);
    msg->text[len] = '\0';
    msg->color = color;
    msg->display_ticks = DM1_V1_DIALOG_DISPLAY_TICKS_PC34;
    msg->active = true;

    state->tail = (state->tail + 1) % DM1_V1_DIALOG_MSG_QUEUE_SIZE_PC34;
    state->count++;
    return true;
}

void DM1_V1_Dialog_TickPc34Compat(DM1_V1_DialogStatePc34* state) {
    if (!state) return;

    /* Tick current message */
    if (state->current.active) {
        state->current.display_ticks--;
        if (state->current.display_ticks <= 0) {
            state->current.active = false;
        }
    }

    /* If no current message, pop from queue */
    if (!state->current.active && state->count > 0) {
        state->current = state->queue[state->head];
        state->head = (state->head + 1) % DM1_V1_DIALOG_MSG_QUEUE_SIZE_PC34;
        state->count--;
    }
}

bool DM1_V1_Dialog_HasMessagePc34Compat(const DM1_V1_DialogStatePc34* state) {
    return state && state->current.active;
}

const char* DM1_V1_Dialog_GetCurrentTextPc34Compat(const DM1_V1_DialogStatePc34* state) {
    if (!state || !state->current.active) return NULL;
    return state->current.text;
}

uint8_t DM1_V1_Dialog_GetCurrentColorPc34Compat(const DM1_V1_DialogStatePc34* state) {
    if (!state || !state->current.active) return 0;
    return state->current.color;
}

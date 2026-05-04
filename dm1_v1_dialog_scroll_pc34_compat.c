/* DM1 V1 Dialog/Scroll Message System — source-locked from ReDMCSB
 * DIALOG.C G2062_DialogSetIndex = C0_DIALOG_SET_VIEWPORT
 * SCRLMGMT.C: message scroll management
 * TEXT.C: text rendering to screen */

#include "dm1_v1_dialog_scroll_pc34_compat.h"
#include <string.h>

void m11_dg_init(M11_DG_State* state) {
    if (!state) return;
    memset(state, 0, sizeof(M11_DG_State));
    state->active_set = M11_DG_SET_VIEWPORT;
    /* Default message bar at bottom of viewport (below 136px viewport area) */
    state->bar_x = 0;
    state->bar_y = 170;
    state->bar_w = 224;
    state->bar_h = 10;
}

void m11_dg_set_bar_position(M11_DG_State* state, int16_t x, int16_t y,
                              int16_t w, int16_t h) {
    if (!state) return;
    state->bar_x = x;
    state->bar_y = y;
    state->bar_w = w;
    state->bar_h = h;
}

void m11_dg_set_active(M11_DG_State* state, M11_DG_DialogSet set) {
    if (!state) return;
    state->active_set = set;
}

bool m11_dg_push_message(M11_DG_State* state, const char* text, uint8_t color) {
    if (!state || !text) return false;
    if (state->count >= DM1_DG_MSG_QUEUE_SIZE) return false;

    M11_DG_Message* msg = &state->queue[state->tail];
    size_t len = strlen(text);
    if (len >= DM1_DG_MAX_MSG_LEN) len = DM1_DG_MAX_MSG_LEN - 1;
    memcpy(msg->text, text, len);
    msg->text[len] = '\0';
    msg->color = color;
    msg->display_ticks = DM1_DG_DISPLAY_TICKS;
    msg->active = true;

    state->tail = (state->tail + 1) % DM1_DG_MSG_QUEUE_SIZE;
    state->count++;
    return true;
}

void m11_dg_tick(M11_DG_State* state) {
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
        state->head = (state->head + 1) % DM1_DG_MSG_QUEUE_SIZE;
        state->count--;
    }
}

bool m11_dg_has_message(const M11_DG_State* state) {
    return state && state->current.active;
}

const char* m11_dg_get_current_text(const M11_DG_State* state) {
    if (!state || !state->current.active) return NULL;
    return state->current.text;
}

uint8_t m11_dg_get_current_color(const M11_DG_State* state) {
    if (!state || !state->current.active) return 0;
    return state->current.color;
}

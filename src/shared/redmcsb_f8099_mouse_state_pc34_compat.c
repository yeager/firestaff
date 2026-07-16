#include "redmcsb_f8099_mouse_state_pc34_compat.h"

void redmcsb_f8099_lock_mouse_pc34_compat(
    redmcsb_f8099_mouse_state_pc34_compat *state)
{
    state->lock_count++;
}

void redmcsb_f8100_unlock_mouse_pc34_compat(
    redmcsb_f8099_mouse_state_pc34_compat *state,
    redmcsb_f8099_cursor_position_fn set_cursor_position,
    void *context)
{
    if (state->mouse_installed == 1) {
        set_cursor_position(context, (int16_t)(state->current_x * 2),
                            state->current_y);
    }
    state->lock_count--;
}

void redmcsb_f8111_set_mouse_pointer_coordinates_pc34_compat(
    redmcsb_f8099_mouse_state_pc34_compat *state,
    int16_t x,
    int16_t y,
    redmcsb_f8099_cursor_position_fn set_cursor_position,
    redmcsb_f8099_mouse_pointer_fn erase_mouse_pointer,
    redmcsb_f8099_mouse_pointer_fn draw_mouse_pointer,
    void *context)
{
    redmcsb_f8099_lock_mouse_pc34_compat(state);
    state->current_x = state->assigned_x = x;
    state->current_y = state->assigned_y = y;
    if (state->mouse_installed == 1) {
        set_cursor_position(context, (int16_t)(x * 2), y);
    }
    if (state->mouse_pointer_drawn) {
        erase_mouse_pointer(context);
        draw_mouse_pointer(context);
    }
    redmcsb_f8100_unlock_mouse_pc34_compat(state, set_cursor_position, context);
}

void redmcsb_f8112_get_mouse_state_pc34_compat(
    const redmcsb_f8099_mouse_state_pc34_compat *state,
    int16_t *x,
    int16_t *y,
    int16_t *button_status)
{
    *x = state->current_x;
    *y = state->current_y;
    *button_status = state->formatted_button_status;
}

const char *redmcsb_f8099_mouse_state_source_evidence_pc34(void)
{
    return "ReDMCSB IBMIO.C:4 selects the PC I34E implementation with "
           "MEDIA701_I34E. IBMIO.C:851-859 F8099 increments G8058_; "
           "IBMIO.C:862-878 F8100 sets the MS Mouse cursor to "
           "G8040_CurrentMouseX << 1/G8041_CurrentMouseY only when G8075_ "
           "equals 1, then decrements G8058_. IBMIO.C:1190-1213 F8111 "
           "locks, assigns G8040/G8045 and G8041/G8046, conditionally sets "
           "the cursor, redraws a present pointer via F8095/F8094, then "
           "unlocks. IBMIO.C:1215-1225 F8112 returns G8040, G8041, and "
           "G8073_FormatedCurrentButtonsStatus.";
}

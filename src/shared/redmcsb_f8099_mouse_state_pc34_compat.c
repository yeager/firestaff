#include "redmcsb_f8099_mouse_state_pc34_compat.h"

static int16_t cursor_column_from_mouse_x(int16_t x)
{
    return (int16_t)(x * 2);
}

void redmcsb_f8099_lock_mouse_pc34_compat(
    redmcsb_f8099_mouse_state_pc34_compat *mouse_state)
{
    mouse_state->lock_count++;
}

void redmcsb_f8100_unlock_mouse_pc34_compat(
    redmcsb_f8099_mouse_state_pc34_compat *mouse_state,
    redmcsb_f8099_set_cursor_position_pc34_compat set_cursor_position,
    void *context)
{
    if (mouse_state->mouse_installed == 1 && set_cursor_position != 0) {
        set_cursor_position(context,
                            cursor_column_from_mouse_x(mouse_state->current_x),
                            mouse_state->current_y);
    }
    mouse_state->lock_count--;
}

void redmcsb_f8111_set_mouse_pointer_coordinates_pc34_compat(
    redmcsb_f8099_mouse_state_pc34_compat *mouse_state,
    int16_t x,
    int16_t y,
    redmcsb_f8099_set_cursor_position_pc34_compat set_cursor_position,
    redmcsb_f8099_mouse_pointer_action_pc34_compat erase_mouse_pointer,
    redmcsb_f8099_mouse_pointer_action_pc34_compat process_mouse_movement,
    void *context)
{
    redmcsb_f8099_lock_mouse_pc34_compat(mouse_state);
    mouse_state->current_x = x;
    mouse_state->assigned_x = x;
    mouse_state->current_y = y;
    mouse_state->assigned_y = y;

    if (mouse_state->mouse_installed == 1 && set_cursor_position != 0) {
        set_cursor_position(context, cursor_column_from_mouse_x(x), y);
    }
    if (mouse_state->mouse_pointer_drawn != 0) {
        if (erase_mouse_pointer != 0) {
            erase_mouse_pointer(context);
        }
        if (process_mouse_movement != 0) {
            process_mouse_movement(context);
        }
    }
    redmcsb_f8100_unlock_mouse_pc34_compat(mouse_state, set_cursor_position,
                                            context);
}

void redmcsb_f8112_get_mouse_state_pc34_compat(
    const redmcsb_f8099_mouse_state_pc34_compat *mouse_state,
    int16_t *out_x,
    int16_t *out_y,
    int16_t *out_buttons)
{
    *out_x = mouse_state->current_x;
    *out_y = mouse_state->current_y;
    *out_buttons = mouse_state->formatted_button_status;
}

const char *redmcsb_f8099_mouse_state_source_evidence_pc34(void)
{
    return "ReDMCSB IBMIO.C:851-878,1190-1225; MEDIA701_I34E PC route";
}

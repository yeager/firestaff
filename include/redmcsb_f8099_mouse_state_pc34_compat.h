/* ReDMCSB IBMIO.C F8099/F8100/F8111/F8112 PC 3.4 mouse-state route. */
#ifndef FIRESTAFF_REDMCSB_F8099_MOUSE_STATE_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F8099_MOUSE_STATE_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*redmcsb_f8099_set_cursor_position_pc34_compat)(
    void *context,
    int16_t x,
    int16_t y);
typedef void (*redmcsb_f8099_mouse_pointer_action_pc34_compat)(void *context);

typedef struct redmcsb_f8099_mouse_state_pc34_compat {
    int16_t current_x;
    int16_t current_y;
    int16_t assigned_x;
    int16_t assigned_y;
    int16_t formatted_button_status;
    int16_t lock_count;
    int16_t mouse_installed;
    int16_t mouse_pointer_drawn;
} redmcsb_f8099_mouse_state_pc34_compat;

void redmcsb_f8099_lock_mouse_pc34_compat(
    redmcsb_f8099_mouse_state_pc34_compat *mouse_state);

void redmcsb_f8100_unlock_mouse_pc34_compat(
    redmcsb_f8099_mouse_state_pc34_compat *mouse_state,
    redmcsb_f8099_set_cursor_position_pc34_compat set_cursor_position,
    void *context);

void redmcsb_f8111_set_mouse_pointer_coordinates_pc34_compat(
    redmcsb_f8099_mouse_state_pc34_compat *mouse_state,
    int16_t x,
    int16_t y,
    redmcsb_f8099_set_cursor_position_pc34_compat set_cursor_position,
    redmcsb_f8099_mouse_pointer_action_pc34_compat erase_mouse_pointer,
    redmcsb_f8099_mouse_pointer_action_pc34_compat process_mouse_movement,
    void *context);

void redmcsb_f8112_get_mouse_state_pc34_compat(
    const redmcsb_f8099_mouse_state_pc34_compat *mouse_state,
    int16_t *out_x,
    int16_t *out_y,
    int16_t *out_buttons);

const char *redmcsb_f8099_mouse_state_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif

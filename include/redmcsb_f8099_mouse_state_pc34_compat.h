#ifndef FIRESTAFF_REDMCSB_F8099_MOUSE_STATE_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F8099_MOUSE_STATE_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*redmcsb_f8099_cursor_position_fn)(void *context,
                                                  int16_t horizontal,
                                                  int16_t vertical);
typedef void (*redmcsb_f8099_mouse_pointer_fn)(void *context);

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
    redmcsb_f8099_mouse_state_pc34_compat *state);

void redmcsb_f8100_unlock_mouse_pc34_compat(
    redmcsb_f8099_mouse_state_pc34_compat *state,
    redmcsb_f8099_cursor_position_fn set_cursor_position,
    void *context);

void redmcsb_f8111_set_mouse_pointer_coordinates_pc34_compat(
    redmcsb_f8099_mouse_state_pc34_compat *state,
    int16_t x,
    int16_t y,
    redmcsb_f8099_cursor_position_fn set_cursor_position,
    redmcsb_f8099_mouse_pointer_fn erase_mouse_pointer,
    redmcsb_f8099_mouse_pointer_fn draw_mouse_pointer,
    void *context);

void redmcsb_f8112_get_mouse_state_pc34_compat(
    const redmcsb_f8099_mouse_state_pc34_compat *state,
    int16_t *x,
    int16_t *y,
    int16_t *button_status);

const char *redmcsb_f8099_mouse_state_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F8099_MOUSE_STATE_PC34_COMPAT_H */

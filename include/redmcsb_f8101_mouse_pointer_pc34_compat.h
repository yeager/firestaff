/* ReDMCSB IBMIO.C F8101/F8108/F8109 PC 3.4 mouse-pointer routes. */
#ifndef FIRESTAFF_REDMCSB_F8101_MOUSE_POINTER_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F8101_MOUSE_POINTER_PC34_COMPAT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    REDMCSB_F8101_MOUSE_POINTER_COUNT_PC34 = 4,
    REDMCSB_F8101_MOUSE_POINTER_BITMAP_BYTES_PC34 = 72
};

typedef int16_t (*redmcsb_f8101_mouse_handler_pc34_compat)(
    int16_t x,
    int16_t y,
    int16_t mouse_event);
typedef void (*redmcsb_f8101_mouse_action_pc34_compat)(void *context);

typedef struct redmcsb_f8101_mouse_pointer_state_pc34_compat {
    uint8_t bitmaps[REDMCSB_F8101_MOUSE_POINTER_COUNT_PC34]
                   [REDMCSB_F8101_MOUSE_POINTER_BITMAP_BYTES_PC34];
    int16_t hotspots[REDMCSB_F8101_MOUSE_POINTER_COUNT_PC34][2];
    redmcsb_f8101_mouse_handler_pc34_compat mouse_handler;
    int16_t mouse_handler_configured;
    int16_t mouse_pointer_type;
    int16_t mouse_pointer_drawn;
} redmcsb_f8101_mouse_pointer_state_pc34_compat;

void redmcsb_f8101_set_mouse_handler_pc34_compat(
    redmcsb_f8101_mouse_pointer_state_pc34_compat *mouse_state,
    redmcsb_f8101_mouse_handler_pc34_compat mouse_handler);

void redmcsb_f8108_register_mouse_pointer_bitmap_pc34_compat(
    redmcsb_f8101_mouse_pointer_state_pc34_compat *mouse_state,
    int16_t pointer_index,
    const uint8_t pointer_bitmap[REDMCSB_F8101_MOUSE_POINTER_BITMAP_BYTES_PC34],
    int16_t hotspot_x,
    int16_t hotspot_y,
    redmcsb_f8101_mouse_action_pc34_compat suspend_mouse_state,
    redmcsb_f8101_mouse_action_pc34_compat process_mouse_state_history,
    void *context);

void redmcsb_f8109_set_mouse_pointer_pc34_compat(
    redmcsb_f8101_mouse_pointer_state_pc34_compat *mouse_state,
    int16_t pointer_type,
    redmcsb_f8101_mouse_action_pc34_compat suspend_mouse_state,
    redmcsb_f8101_mouse_action_pc34_compat erase_mouse_pointer,
    redmcsb_f8101_mouse_action_pc34_compat process_mouse_movement,
    redmcsb_f8101_mouse_action_pc34_compat process_mouse_state_history,
    void *context);

const char *redmcsb_f8101_mouse_pointer_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif

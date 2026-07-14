#include "redmcsb_f8101_mouse_pointer_pc34_compat.h"

#include <string.h>

void redmcsb_f8101_set_mouse_handler_pc34_compat(
    redmcsb_f8101_mouse_pointer_state_pc34_compat *mouse_state,
    redmcsb_f8101_mouse_handler_pc34_compat mouse_handler)
{
    mouse_state->mouse_handler = mouse_handler;
    mouse_state->mouse_handler_configured = 1;
}

void redmcsb_f8108_register_mouse_pointer_bitmap_pc34_compat(
    redmcsb_f8101_mouse_pointer_state_pc34_compat *mouse_state,
    int16_t pointer_index,
    const uint8_t pointer_bitmap[REDMCSB_F8101_MOUSE_POINTER_BITMAP_BYTES_PC34],
    int16_t hotspot_x,
    int16_t hotspot_y,
    redmcsb_f8101_mouse_action_pc34_compat suspend_mouse_state,
    redmcsb_f8101_mouse_action_pc34_compat process_mouse_state_history,
    void *context)
{
    if (pointer_index < 0 ||
        pointer_index >= REDMCSB_F8101_MOUSE_POINTER_COUNT_PC34) {
        return;
    }

    if (suspend_mouse_state != 0) {
        suspend_mouse_state(context);
    }
    memcpy(mouse_state->bitmaps[pointer_index], pointer_bitmap,
           REDMCSB_F8101_MOUSE_POINTER_BITMAP_BYTES_PC34);
    mouse_state->hotspots[pointer_index][0] = hotspot_x;
    mouse_state->hotspots[pointer_index][1] = hotspot_y;
    if (process_mouse_state_history != 0) {
        process_mouse_state_history(context);
    }
}

void redmcsb_f8109_set_mouse_pointer_pc34_compat(
    redmcsb_f8101_mouse_pointer_state_pc34_compat *mouse_state,
    int16_t pointer_type,
    redmcsb_f8101_mouse_action_pc34_compat suspend_mouse_state,
    redmcsb_f8101_mouse_action_pc34_compat erase_mouse_pointer,
    redmcsb_f8101_mouse_action_pc34_compat process_mouse_movement,
    redmcsb_f8101_mouse_action_pc34_compat process_mouse_state_history,
    void *context)
{
    if (suspend_mouse_state != 0) {
        suspend_mouse_state(context);
    }
    if (mouse_state->mouse_pointer_drawn != 0) {
        if (erase_mouse_pointer != 0) {
            erase_mouse_pointer(context);
        }
        mouse_state->mouse_pointer_type = pointer_type;
        if (process_mouse_movement != 0) {
            process_mouse_movement(context);
        }
    } else {
        mouse_state->mouse_pointer_type = pointer_type;
    }
    if (process_mouse_state_history != 0) {
        process_mouse_state_history(context);
    }
}

const char *redmcsb_f8101_mouse_pointer_source_evidence_pc34(void)
{
    return "ReDMCSB IBMIO.C:881-890,1131-1168; MEDIA701_I34E PC route";
}

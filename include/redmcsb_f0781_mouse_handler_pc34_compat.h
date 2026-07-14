/*
 * ReDMCSB IO.C F0781_MouseHandler, PC 3.4 (I34E/I34M) route.
 */
#ifndef FIRESTAFF_REDMCSB_F0781_MOUSE_HANDLER_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0781_MOUSE_HANDLER_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    REDMCSB_F0781_POINTER_NONE = -1,
    REDMCSB_F0781_POINTER_ARROW = 0,
    REDMCSB_F0781_POINTER_HAND = 1,
    REDMCSB_F0781_POINTER_OBJECT_ICON = 2,
    REDMCSB_F0781_MOUSE_EVENT_CHANGE_SCREEN_REGION = 32,
    REDMCSB_F0781_MOUSE_EVENT_LEAVE_CHAMPION_ICON_REGION = 33
};

typedef struct {
    int16_t xyz[4];
    int16_t pointer_type;
    int16_t champion_ordinal;
} redmcsb_f0781_mouse_pointer_region_pc34;

typedef void (*redmcsb_f0781_process_click_pc34)(
    void *context, int16_t x, int16_t y, int16_t mouse_event);
typedef void (*redmcsb_f0781_set_mouse_bound_event_pc34)(
    void *context, const int16_t xyz[4], int16_t mouse_event);

typedef struct {
    bool use_champion_icon_ordinal_as_pointer;
    bool use_object_as_pointer;
    bool use_hand_as_pointer;
    const redmcsb_f0781_mouse_pointer_region_pc34 *regions;
    size_t region_count;
    const int16_t *champion_current_health;
    size_t party_champion_count;
    int16_t inventory_champion_ordinal;
    const int16_t *champion_icon_region_xyz;
    redmcsb_f0781_process_click_pc34 process_click;
    redmcsb_f0781_set_mouse_bound_event_pc34 set_mouse_bound_event;
    void *context;
} redmcsb_f0781_mouse_handler_state_pc34;

/*
 * Adapts IO.C:687-755. Returns true only when the original local pointer
 * type received a defined value. A false return preserves the original
 * routine's intentionally unused undefined return route after click handling
 * and when no configured region matches.
 */
bool redmcsb_f0781_mouse_handler_pc34_compat(
    const redmcsb_f0781_mouse_handler_state_pc34 *state,
    int16_t x,
    int16_t y,
    int16_t mouse_event,
    int16_t *out_pointer_type);

const char *redmcsb_f0781_mouse_handler_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif

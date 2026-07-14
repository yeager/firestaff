#ifndef FIRESTAFF_REDMCSB_F1041_GET_MOUSE_POINTER_TYPE_H
#define FIRESTAFF_REDMCSB_F1041_GET_MOUSE_POINTER_TYPE_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    REDMCSB_F1041_POINTER_NONE = -1,
    REDMCSB_F1041_POINTER_ARROW = 0,
    REDMCSB_F1041_POINTER_HAND = 1,
    REDMCSB_F1041_POINTER_OBJECT_ICON = 2,
    REDMCSB_F1041_POINTER_CHAMPION_ICON = 3,
    REDMCSB_F1041_MOUSE_POINTER_REGION_COUNT = 16
};

typedef struct {
    int16_t xyz[4];
    int16_t pointer_type;
    uint16_t champion_ordinal;
} redmcsb_f1041_mouse_pointer_screen_region;

typedef void (*redmcsb_f1041_process_click_fn)(void *context,
                                                 int16_t x,
                                                 int16_t y,
                                                 int16_t buttons_status);
typedef void (*redmcsb_f1041_set_mouse_bound_event_fn)(
    void *context,
    const int16_t xyz[4],
    int16_t buttons_status);

/*
 * champion_ordinal and inventory_champion_ordinal use ReDMCSB's one-based
 * ordinal representation; zero means no champion. The region array contains
 * the source-owned 16 entries. Callbacks provide the F0359/F1040 boundaries.
 */
typedef struct {
    int16_t hide_mouse_pointer_request_count;
    bool use_hand_as_mouse_pointer_bitmap;
    bool use_object_as_mouse_pointer_bitmap;
    bool mouse_bounded;
    int16_t mouse_bound_xyz[4];
    int16_t mouse_bound_buttons_status;
    uint16_t use_champion_icon_ordinal_as_mouse_pointer_bitmap;
    bool refresh_regions;
    bool was_outside_screen;
    const redmcsb_f1041_mouse_pointer_screen_region *regions;
    uint16_t screen_pixel_width;
    uint16_t screen_pixel_height;
    uint16_t party_champion_count;
    const int16_t *champion_current_health;
    uint16_t inventory_champion_ordinal;
    int16_t retained_pointer_type;
    redmcsb_f1041_process_click_fn process_click;
    redmcsb_f1041_set_mouse_bound_event_fn set_mouse_bound_event;
    void *context;
} redmcsb_f1041_get_mouse_pointer_type_state;

int16_t redmcsb_f1041_get_mouse_pointer_type(
    redmcsb_f1041_get_mouse_pointer_type_state *state,
    int16_t x,
    int16_t y);

const char *redmcsb_f1041_get_mouse_pointer_type_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F1041_GET_MOUSE_POINTER_TYPE_H */

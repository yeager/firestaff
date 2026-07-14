#include "redmcsb_f0781_mouse_handler_pc34_compat.h"

#include <stddef.h>

typedef struct {
    int click_count;
    int bound_count;
    int16_t click_x;
    int16_t click_y;
    int16_t click_event;
    int16_t bound_xyz[4];
    int16_t bound_event;
} trace;

static void record_click(void *context, int16_t x, int16_t y, int16_t event)
{
    trace *record = context;

    ++record->click_count;
    record->click_x = x;
    record->click_y = y;
    record->click_event = event;
}

static void record_bound_event(
    void *context, const int16_t xyz[4], int16_t event)
{
    size_t index;
    trace *record = context;

    ++record->bound_count;
    for (index = 0; index < 4; ++index) {
        record->bound_xyz[index] = xyz[index];
    }
    record->bound_event = event;
}

int main(void)
{
    const int16_t champion_icon_region[4] = { 50, 60, 10, 20 };
    const int16_t health[1] = { 100 };
    const redmcsb_f0781_mouse_pointer_region_pc34 regions[2] = {
        { { 0, 20, 0, 20 }, REDMCSB_F0781_POINTER_HAND, 0 },
        { { 21, 40, 0, 20 }, REDMCSB_F0781_POINTER_ARROW, 1 }
    };
    trace record = { 0 };
    redmcsb_f0781_mouse_handler_state_pc34 state = {
        false, true, true, regions, 2, health, 1, 1,
        champion_icon_region, record_click, record_bound_event, &record
    };
    int16_t pointer_type = 99;

    if (redmcsb_f0781_mouse_handler_pc34_compat(
            &state, 7, 8, 2, &pointer_type)) {
        return 1;
    }
    if (record.click_count != 1 || record.click_x != 7 ||
        record.click_y != 8 || record.click_event != 2 ||
        record.bound_count != 0 || pointer_type != 99) {
        return 2;
    }

    if (!redmcsb_f0781_mouse_handler_pc34_compat(
            &state, 4, 5,
            REDMCSB_F0781_MOUSE_EVENT_CHANGE_SCREEN_REGION,
            &pointer_type)) {
        return 3;
    }
    if (pointer_type != REDMCSB_F0781_POINTER_OBJECT_ICON ||
        record.bound_count != 1 || record.bound_event !=
            REDMCSB_F0781_MOUSE_EVENT_CHANGE_SCREEN_REGION) {
        return 4;
    }

    state.use_object_as_pointer = false;
    if (!redmcsb_f0781_mouse_handler_pc34_compat(
            &state, 30, 5,
            REDMCSB_F0781_MOUSE_EVENT_CHANGE_SCREEN_REGION,
            &pointer_type) ||
        pointer_type != REDMCSB_F0781_POINTER_ARROW) {
        return 5;
    }

    state.use_champion_icon_ordinal_as_pointer = true;
    if (!redmcsb_f0781_mouse_handler_pc34_compat(
            &state, 70, 70,
            REDMCSB_F0781_MOUSE_EVENT_CHANGE_SCREEN_REGION,
            &pointer_type) ||
        pointer_type != REDMCSB_F0781_POINTER_NONE ||
        record.bound_event !=
            REDMCSB_F0781_MOUSE_EVENT_LEAVE_CHAMPION_ICON_REGION) {
        return 6;
    }

    state.use_champion_icon_ordinal_as_pointer = false;
    state.use_object_as_pointer = false;
    if (!redmcsb_f0781_mouse_handler_pc34_compat(
            &state, 70, 70,
            REDMCSB_F0781_MOUSE_EVENT_CHANGE_SCREEN_REGION,
            &pointer_type) ||
        pointer_type != REDMCSB_F0781_POINTER_ARROW) {
        return 7;
    }

    return 0;
}

#include "redmcsb_f1041_get_mouse_pointer_type.h"

static bool redmcsb_f1041_contains(const int16_t xyz[4], int16_t x, int16_t y)
{
    return x >= xyz[0] && x <= xyz[1] && y >= xyz[2] && y <= xyz[3];
}

int16_t redmcsb_f1041_get_mouse_pointer_type(
    redmcsb_f1041_get_mouse_pointer_type_state *state,
    int16_t x,
    int16_t y)
{
    bool outside_mouse_bound;
    uint16_t region_index;

    if (state->hide_mouse_pointer_request_count > 0) {
        return REDMCSB_F1041_POINTER_NONE;
    }

    outside_mouse_bound = state->mouse_bounded &&
                          !redmcsb_f1041_contains(state->mouse_bound_xyz, x,
                                                    y);
    if (outside_mouse_bound && state->mouse_bound_buttons_status != 0) {
        state->process_click(state->context, x, y,
                             state->mouse_bound_buttons_status);
        state->mouse_bound_buttons_status = 0;
    }

    if (state->use_champion_icon_ordinal_as_mouse_pointer_bitmap != 0U) {
        return REDMCSB_F1041_POINTER_CHAMPION_ICON;
    }

    if (!state->use_hand_as_mouse_pointer_bitmap &&
        !state->use_object_as_mouse_pointer_bitmap) {
        return REDMCSB_F1041_POINTER_ARROW;
    }

    if (outside_mouse_bound || state->refresh_regions ||
        state->was_outside_screen) {
        state->refresh_regions = false;
        for (region_index = 0;
             region_index < REDMCSB_F1041_MOUSE_POINTER_REGION_COUNT;
             region_index++) {
            const redmcsb_f1041_mouse_pointer_screen_region *region =
                &state->regions[region_index];
            int16_t champion_index;
            bool over_champion_name;

            if (!redmcsb_f1041_contains(region->xyz, x, y)) {
                continue;
            }

            state->was_outside_screen = false;
            state->set_mouse_bound_event(state->context, region->xyz,
                                         state->mouse_bound_buttons_status);
            if (region->champion_ordinal == 0U) {
                if (state->use_object_as_mouse_pointer_bitmap &&
                    region->pointer_type == REDMCSB_F1041_POINTER_HAND) {
                    return REDMCSB_F1041_POINTER_OBJECT_ICON;
                }
                return region->pointer_type;
            }

            champion_index = (int16_t)((region->champion_ordinal & 7U) -
                                        1U);
            over_champion_name = (region->champion_ordinal >> 3) != 0U;
            if ((uint16_t)champion_index < state->party_champion_count &&
                state->champion_current_health[champion_index] != 0 &&
                ((uint16_t)(champion_index + 1) ==
                     state->inventory_champion_ordinal ||
                 over_champion_name)) {
                return REDMCSB_F1041_POINTER_ARROW;
            }
            return state->use_object_as_mouse_pointer_bitmap
                       ? REDMCSB_F1041_POINTER_OBJECT_ICON
                       : REDMCSB_F1041_POINTER_HAND;
        }
    }

    if (x < 0 || x >= (int16_t)state->screen_pixel_width || y < 0 ||
        y >= (int16_t)state->screen_pixel_height) {
        state->was_outside_screen = true;
        return REDMCSB_F1041_POINTER_ARROW;
    }

    return state->retained_pointer_type;
}

const char *redmcsb_f1041_get_mouse_pointer_type_source_evidence(void)
{
    return "ReDMCSB Toolchains/Common/Source/IO.C:3318-3394 defines "
           "F1041_GetMousePointerType. IO.C:3332-3393 orders hide-pointer, "
           "mouse-bound exit click, champion-icon override, hand/object "
           "fallback, the 16-region scan, out-of-screen latch, and retained "
           "pointer type. DEFS.H:2868-2872 defines pointer values; COMPILE.H:"
           "1038-1039 defines champion ordinal/index conversion.";
}

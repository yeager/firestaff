#include "redmcsb_f0781_mouse_handler_pc34_compat.h"

static bool point_is_in_region(
    const redmcsb_f0781_mouse_pointer_region_pc34 *region,
    int16_t x,
    int16_t y)
{
    return region->xyz[0] <= x && region->xyz[1] >= x &&
           region->xyz[2] <= y && region->xyz[3] >= y;
}

bool redmcsb_f0781_mouse_handler_pc34_compat(
    const redmcsb_f0781_mouse_handler_state_pc34 *state,
    int16_t x,
    int16_t y,
    int16_t mouse_event,
    int16_t *out_pointer_type)
{
    size_t region_index;

    if (state == NULL) {
        return false;
    }

    if (mouse_event < REDMCSB_F0781_MOUSE_EVENT_CHANGE_SCREEN_REGION) {
        if (state->process_click != NULL) {
            state->process_click(state->context, x, y, mouse_event);
        }
        return false;
    }

    if (mouse_event ==
        REDMCSB_F0781_MOUSE_EVENT_LEAVE_CHAMPION_ICON_REGION) {
        if (state->process_click != NULL) {
            state->process_click(state->context, x, y, mouse_event);
        }
    } else if (state->use_champion_icon_ordinal_as_pointer) {
        if (state->set_mouse_bound_event != NULL &&
            state->champion_icon_region_xyz != NULL) {
            state->set_mouse_bound_event(
                state->context,
                state->champion_icon_region_xyz,
                REDMCSB_F0781_MOUSE_EVENT_LEAVE_CHAMPION_ICON_REGION);
        }
        if (out_pointer_type != NULL) {
            *out_pointer_type = REDMCSB_F0781_POINTER_NONE;
        }
        return true;
    }

    if (!state->use_hand_as_pointer && !state->use_object_as_pointer) {
        if (out_pointer_type != NULL) {
            *out_pointer_type = REDMCSB_F0781_POINTER_ARROW;
        }
        return true;
    }

    for (region_index = 0; region_index < state->region_count; ++region_index) {
        const redmcsb_f0781_mouse_pointer_region_pc34 *region;

        region = &state->regions[region_index];
        if (!point_is_in_region(region, x, y)) {
            continue;
        }

        if (state->set_mouse_bound_event != NULL) {
            state->set_mouse_bound_event(
                state->context,
                region->xyz,
                REDMCSB_F0781_MOUSE_EVENT_CHANGE_SCREEN_REGION);
        }

        if (region->champion_ordinal == 0) {
            if (state->use_object_as_pointer &&
                region->pointer_type == REDMCSB_F0781_POINTER_HAND) {
                if (out_pointer_type != NULL) {
                    *out_pointer_type = REDMCSB_F0781_POINTER_OBJECT_ICON;
                }
            } else if (out_pointer_type != NULL) {
                *out_pointer_type = region->pointer_type;
            }
            return true;
        }

        {
            int16_t champion_index;
            bool over_champion_name;
            bool champion_is_eligible;

            champion_index = (int16_t)((region->champion_ordinal & 7) - 1);
            over_champion_name = (region->champion_ordinal >> 3) != 0;
            champion_is_eligible = champion_index >= 0 &&
                (size_t)champion_index < state->party_champion_count &&
                state->champion_current_health != NULL &&
                state->champion_current_health[champion_index] != 0 &&
                (((int16_t)(state->inventory_champion_ordinal - 1) ==
                  champion_index) || over_champion_name);

            if (out_pointer_type != NULL) {
                *out_pointer_type = champion_is_eligible
                    ? REDMCSB_F0781_POINTER_ARROW
                    : (state->use_object_as_pointer
                        ? REDMCSB_F0781_POINTER_OBJECT_ICON
                        : REDMCSB_F0781_POINTER_HAND);
            }
            return true;
        }
    }

    return false;
}

const char *redmcsb_f0781_mouse_handler_source_evidence_pc34(void)
{
    return "ReDMCSB IO.C:687-755 (PC 3.4 I34E/I34M route): "
           "F0781_MouseHandler dispatches click events below C32, "
           "rebinds screen regions, and chooses arrow/hand/object pointers.";
}

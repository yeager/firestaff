#include "dm1_v1_f0111_door_ornament_dispatch_pc34_compat.h"

#include <string.h>

int dm1_v1_f0111_door_ornament_dispatch_pc34(
    const DM1_V1_F0111DoorOrnamentInputPc34 *input,
    DM1_V1_F0111DoorOrnamentDispatchPc34 *out_dispatch)
{
    size_t i;

    if (!out_dispatch) {
        return 0;
    }
    memset(out_dispatch, 0, sizeof(*out_dispatch));
    if (!input || !input->is_door) {
        out_dispatch->rejection =
            DM1_V1_F0111_DOOR_ORNAMENT_REJECT_NOT_DOOR_PC34;
        return 0;
    }
    if (input->is_open) {
        out_dispatch->rejection = DM1_V1_F0111_DOOR_ORNAMENT_REJECT_OPEN_PC34;
        return 0;
    }
    if (input->ornament_ordinal <= 0) {
        out_dispatch->rejection =
            DM1_V1_F0111_DOOR_ORNAMENT_REJECT_NO_ORNAMENT_PC34;
        return 0;
    }
    if (input->depth_index < 0 || input->depth_index > 2) {
        out_dispatch->rejection =
            DM1_V1_F0111_DOOR_ORNAMENT_REJECT_INVALID_DEPTH_PC34;
        return 0;
    }
    if (!input->panels || input->panel_count == 0 ||
        input->panel_count > DM1_V1_F0111_DOOR_ORNAMENT_MAX_PANELS_PC34) {
        out_dispatch->rejection =
            DM1_V1_F0111_DOOR_ORNAMENT_REJECT_INVALID_PANELS_PC34;
        return 0;
    }
    for (i = 0; i < input->panel_count; ++i) {
        if (input->panels[i].width <= 0 || input->panels[i].height <= 0 ||
            input->panels[i].src_y < 0) {
            out_dispatch->rejection =
                DM1_V1_F0111_DOOR_ORNAMENT_REJECT_INVALID_PANELS_PC34;
            return 0;
        }
    }
    out_dispatch->rejection = DM1_V1_F0111_DOOR_ORNAMENT_ACCEPT_PC34;
    out_dispatch->ornament_ordinal = input->ornament_ordinal;
    out_dispatch->depth_index = input->depth_index;
    out_dispatch->panel_count = input->panel_count;
    for (i = 0; i < input->panel_count; ++i) {
        out_dispatch->panels[i] = input->panels[i];
    }
    return 1;
}

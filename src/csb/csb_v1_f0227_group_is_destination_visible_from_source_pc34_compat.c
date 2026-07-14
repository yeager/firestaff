#include "csb_v1_f0227_group_is_destination_visible_from_source_pc34_compat.h"

bool csb_v1_f0227_group_is_destination_visible_from_source_pc34(
    int direction,
    int source_map_x,
    int source_map_y,
    int destination_map_x,
    int destination_map_y)
{
    int forward_depth;
    int lateral_offset;

    /* GROUP2.C F0227 swaps coordinates to apply its west-facing test. */
    switch (direction) {
    case CSB_V1_F0227_DIRECTION_SOUTH_PC34:
        forward_depth = source_map_x;
        source_map_x = destination_map_y;
        destination_map_y = forward_depth;
        forward_depth = destination_map_x;
        destination_map_x = source_map_y;
        source_map_y = forward_depth;
        break;
    case CSB_V1_F0227_DIRECTION_EAST_PC34:
        forward_depth = source_map_x;
        source_map_x = destination_map_x;
        destination_map_x = forward_depth;
        forward_depth = source_map_y;
        source_map_y = destination_map_y;
        destination_map_y = forward_depth;
        break;
    case CSB_V1_F0227_DIRECTION_NORTH_PC34:
        forward_depth = source_map_x;
        source_map_x = source_map_y;
        source_map_y = forward_depth;
        forward_depth = destination_map_x;
        destination_map_x = destination_map_y;
        destination_map_y = forward_depth;
        break;
    }

    forward_depth = source_map_x - (destination_map_x - 1);
    lateral_offset = source_map_y - destination_map_y;
    if (lateral_offset < 0) {
        lateral_offset = -lateral_offset;
    }
    return forward_depth > 0 && lateral_offset <= forward_depth;
}

#include "firestaff/dm1/v1/object_clear_leader_hand_object_name_pc34_compat.h"

#include <limits.h>
#include <string.h>

int
DM1_V1_F0035_OBJECT_ClearLeaderHandObjectNamePc34Compat(
    DM1_V1_F0035LeaderHandStatePc34 *state)
{
    size_t required_size;
    size_t clear_width;
    size_t clear_height;
    size_t row;

    if (!state || !state->logical_screen || state->logical_screen_width <= 0 ||
        state->logical_screen_height <= 0 || state->logical_screen_stride == 0 ||
        state->logical_screen_stride < (size_t)state->logical_screen_width) {
        return 0;
    }
    if ((size_t)state->logical_screen_height >
        SIZE_MAX / state->logical_screen_stride) {
        return 0;
    }
    required_size = state->logical_screen_stride *
                    (size_t)state->logical_screen_height;
    if (state->logical_screen_size < required_size) {
        return 0;
    }
    if (state->logical_screen_width <= DM1_V1_F0035_LEADER_HAND_NAME_X_PC34 ||
        state->logical_screen_height <= DM1_V1_F0035_LEADER_HAND_NAME_Y_PC34) {
        return 1;
    }

    clear_width = (size_t)(state->logical_screen_width -
                           DM1_V1_F0035_LEADER_HAND_NAME_X_PC34);
    if (clear_width > DM1_V1_F0035_LEADER_HAND_NAME_WIDTH_PC34) {
        clear_width = DM1_V1_F0035_LEADER_HAND_NAME_WIDTH_PC34;
    }
    clear_height = (size_t)(state->logical_screen_height -
                            DM1_V1_F0035_LEADER_HAND_NAME_Y_PC34);
    if (clear_height > DM1_V1_F0035_LEADER_HAND_NAME_HEIGHT_PC34) {
        clear_height = DM1_V1_F0035_LEADER_HAND_NAME_HEIGHT_PC34;
    }

    for (row = 0; row < clear_height; ++row) {
        size_t offset = ((size_t)DM1_V1_F0035_LEADER_HAND_NAME_Y_PC34 + row) *
                        state->logical_screen_stride +
                        DM1_V1_F0035_LEADER_HAND_NAME_X_PC34;
        memset(state->logical_screen + offset,
               DM1_V1_F0035_LEADER_HAND_NAME_BLACK_PC34,
               clear_width);
    }
    return 1;
}

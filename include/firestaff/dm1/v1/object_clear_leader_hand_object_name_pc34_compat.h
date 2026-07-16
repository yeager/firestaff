#ifndef FIRESTAFF_DM1_V1_OBJECT_CLEAR_LEADER_HAND_OBJECT_NAME_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_OBJECT_CLEAR_LEADER_HAND_OBJECT_NAME_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

/*
 * ReDMCSB SOURCE/ENGINE/OBJECT.C F035_aaaw_OBJECT_ClearLeaderHandObjectName
 * fills G028_s_Graphic562_Box_LeaderHandObjectName with black. The current
 * DM1 layout resolves that box to {233, 33, 87, 6} in the 320x200 surface.
 * F0035 changes only that display region; leader-hand ownership and its name
 * remain source state for the caller.
 */
enum {
    DM1_V1_F0035_LEADER_HAND_NAME_X_PC34 = 233,
    DM1_V1_F0035_LEADER_HAND_NAME_Y_PC34 = 33,
    DM1_V1_F0035_LEADER_HAND_NAME_WIDTH_PC34 = 87,
    DM1_V1_F0035_LEADER_HAND_NAME_HEIGHT_PC34 = 6,
    DM1_V1_F0035_LEADER_HAND_NAME_BLACK_PC34 = 0,
    DM1_V1_F0035_LEADER_HAND_OBJECT_NAME_CAP_PC34 = 16
};

typedef struct DM1_V1_F0035LeaderHandStatePc34 {
    uint8_t *logical_screen;
    size_t logical_screen_size;
    size_t logical_screen_stride;
    int logical_screen_width;
    int logical_screen_height;
    uint16_t leader_hand_thing;
    int leader_hand_present;
    char leader_hand_object_name[DM1_V1_F0035_LEADER_HAND_OBJECT_NAME_CAP_PC34];
} DM1_V1_F0035LeaderHandStatePc34;

/*
 * Clears the visible leader-hand name box. The helper validates the complete
 * supplied surface before writing and clips its fixed source rectangle to it.
 * Returns zero for an invalid surface, otherwise one, including no overlap.
 */
int DM1_V1_F0035_OBJECT_ClearLeaderHandObjectNamePc34Compat(
    DM1_V1_F0035LeaderHandStatePc34 *state);

#endif

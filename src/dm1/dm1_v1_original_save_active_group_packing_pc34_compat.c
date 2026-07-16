#include "dm1_v1_original_save_pc34_handoff.h"

int F0145_DUNGEON_GetGroupCells(int packedCells, int creatureLane)
{
    unsigned int packed;
    if (creatureLane < 0 ||
        creatureLane >= DM1_ORIGINAL_SAVE_PC34_ACTIVE_GROUP_PACKED_LANE_COUNT) {
        return -1;
    }
    packed = (unsigned int)packedCells & 0xffu;
    return (int)((packed >> (unsigned int)(creatureLane * 2)) & 0x03u);
}

int F0147_DUNGEON_GetGroupDirections(int packedDirections, int creatureLane)
{
    unsigned int packed;
    if (creatureLane < 0 ||
        creatureLane >= DM1_ORIGINAL_SAVE_PC34_ACTIVE_GROUP_PACKED_LANE_COUNT) {
        return -1;
    }
    packed = (unsigned int)packedDirections & 0xffu;
    return (int)((packed >> (unsigned int)(creatureLane * 2)) & 0x03u);
}

int dm1_v1_original_save_pc34_unpack_active_group_lanes(
    int packedCells,
    int packedDirections,
    int outCells[DM1_ORIGINAL_SAVE_PC34_ACTIVE_GROUP_PACKED_LANE_COUNT],
    int outDirections[DM1_ORIGINAL_SAVE_PC34_ACTIVE_GROUP_PACKED_LANE_COUNT])
{
    int i;
    if (!outCells || !outDirections) {
        return 0;
    }
    for (i = 0; i < DM1_ORIGINAL_SAVE_PC34_ACTIVE_GROUP_PACKED_LANE_COUNT; ++i) {
        outCells[i] = F0145_DUNGEON_GetGroupCells(packedCells, i);
        outDirections[i] = F0147_DUNGEON_GetGroupDirections(packedDirections, i);
    }
    return 1;
}

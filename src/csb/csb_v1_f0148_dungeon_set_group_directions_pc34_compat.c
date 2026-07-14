#include "csb_v1_f0148_dungeon_set_group_directions_pc34_compat.h"

int csb_v1_f0148_dungeon_set_group_directions_pc34_compat(
    uint8_t *group_record,
    int record_size,
    uint16_t directions,
    uint16_t map_index,
    uint16_t party_map_index,
    CSB_V1_F0148_ActiveGroupPc34Compat *active_groups,
    int active_group_count)
{
    if (!group_record || record_size < 16) {
        return 0;
    }

    if (map_index == party_map_index) {
        const unsigned int active_group_index = group_record[5];

        if (!active_groups || active_group_count < 0 ||
            active_group_index >= (unsigned int)active_group_count) {
            return 0;
        }
        active_groups[active_group_index].directions = (uint8_t)directions;
        return 1;
    }

    /* DEFS.H:1323: M021_NORMALIZE writes GROUP.Direction, bits 8..9. */
    group_record[15] = (uint8_t)((group_record[15] & 0xfcu) |
                                 (directions & 0x0003u));
    return 1;
}

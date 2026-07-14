#include "csb_v1_f0146_dungeon_set_group_cells_pc34_compat.h"

void csb_v1_f0146_dungeon_set_group_cells_pc34(
    uint8_t group_record[static CSB_V1_F0146_DUNGEON_GROUP_RECORD_SIZE_PC34],
    uint8_t cells)
{
    group_record[CSB_V1_F0146_DUNGEON_GROUP_CELLS_OFFSET_PC34] = cells;
}

#ifndef CSB_V1_F0146_DUNGEON_SET_GROUP_CELLS_PC34_COMPAT_H
#define CSB_V1_F0146_DUNGEON_SET_GROUP_CELLS_PC34_COMPAT_H

#include <stdint.h>

/* ReDMCSB DEFS.H GROUP: C04 records are 16 bytes and Cells is byte 5.
 * The byte holds four two-bit creature-cell ordinals; 0xff is the native
 * single centered-group sentinel. */
enum {
    CSB_V1_F0146_DUNGEON_GROUP_RECORD_SIZE_PC34 = 16,
    CSB_V1_F0146_DUNGEON_GROUP_CELLS_OFFSET_PC34 = 5
};

/* ReDMCSB DUNGEON.C F0146_DUNGEON_SetGroupCells writes GROUP.Cells without
 * interpreting or normalizing the packed value. group_record must name a
 * complete native C04 GROUP record. */
void csb_v1_f0146_dungeon_set_group_cells_pc34(
    uint8_t group_record[static CSB_V1_F0146_DUNGEON_GROUP_RECORD_SIZE_PC34],
    uint8_t cells);

#endif

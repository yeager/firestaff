#ifndef FIRESTAFF_CSB_V1_F0148_DUNGEON_SET_GROUP_DIRECTIONS_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F0148_DUNGEON_SET_GROUP_DIRECTIONS_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB DUNGEON.C:1315-1329 F0148_DUNGEON_SetGroupDirections. DEFS.H:
 * 1311-1336 defines GROUP.Cells as ActiveGroupIndex on the party map and
 * packs Direction into bits 8..9 of the final PC34 little-endian word. */
typedef struct {
    uint8_t directions;
} CSB_V1_F0148_ActiveGroupPc34Compat;

int csb_v1_f0148_dungeon_set_group_directions_pc34_compat(
    uint8_t *group_record,
    int record_size,
    uint16_t directions,
    uint16_t map_index,
    uint16_t party_map_index,
    CSB_V1_F0148_ActiveGroupPc34Compat *active_groups,
    int active_group_count);

#ifdef __cplusplus
}
#endif

#endif

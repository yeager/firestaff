#ifndef FIRESTAFF_DM1_V1_DUNGEON_THING_DATA_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_DUNGEON_THING_DATA_PC34_COMPAT_H

#include "memory_dungeon_dat_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB DUNGEON.C F0156. Returns only a loaded raw PC3.4 record. */
const unsigned char *dm1_v1_dungeon_get_thing_data_pc34(
    const struct DungeonThings_Compat *things,
    unsigned short thing);

/* ReDMCSB DUNGEON.C F0141 / OBJECT.C F0032/F0033.
 * These consume the loaded PC3.4 raw Thing record.  Invalid Thing types,
 * out-of-range subtypes, and missing raw records fail closed with -1; they
 * never borrow a subtype-zero object or icon. */
int dm1_v1_dungeon_get_object_subtype_pc34(
    const struct DungeonThings_Compat *things,
    unsigned short thing);

int dm1_v1_dungeon_get_object_info_index_pc34(
    const struct DungeonThings_Compat *things,
    unsigned short thing);

int dm1_v1_dungeon_get_object_type_pc34(
    const struct DungeonThings_Compat *things,
    unsigned short thing);

int dm1_v1_dungeon_get_object_icon_index_pc34(
    const struct DungeonThings_Compat *things,
    unsigned short thing,
    int partyDirection);

/* ReDMCSB DUNGEON.C F0144: raw GROUP.Type -> G0243 Attributes. */
int dm1_v1_dungeon_get_creature_attributes_f0144_pc34(
    const struct DungeonThings_Compat *things,
    unsigned short thing,
    unsigned short *outAttributes);

/* ReDMCSB GROUP.C F0175: first C04 group in a loaded square Thing chain. */
unsigned short dm1_v1_group_get_thing_f0175_pc34(
    const struct DungeonDatState_Compat *dungeon,
    const struct DungeonThings_Compat *things,
    int mapIndex,
    int mapX,
    int mapY);

/* ReDMCSB GROUP.C F0176: one-based C04 creature ordinal for a cell.
 * groupCells/groupDirections are the F0145/F0147 effective values: callers
 * retain responsibility for selecting their active-group overlays. */
int dm1_v1_group_get_creature_ordinal_in_cell_f0176_pc34(
    const struct DungeonThings_Compat *things,
    unsigned short groupThing,
    unsigned int groupCells,
    unsigned int groupDirections,
    unsigned int cell);

const char *dm1_v1_dungeon_thing_data_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif

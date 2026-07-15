#ifndef FIRESTAFF_DM1_V1_DUNGEON_THING_DATA_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_DUNGEON_THING_DATA_PC34_COMPAT_H

#include "memory_dungeon_dat_pc34_compat.h"
#include "memory_creature_ai_pc34_compat.h"

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

/* ReDMCSB DUNGEON.C F0145-F0148.  On the party map, C04 Cells and
 * Directions live in the matching ACTIVE_GROUP record; elsewhere they live
 * in the loaded raw C04 record.  Missing raw C04 data or a missing active
 * record on the party map is an error, never a decoded/default fallback. */
int dm1_v1_dungeon_get_group_cells_f0145_pc34(
    const struct DungeonThings_Compat *things,
    const struct CreatureAIState_Compat *activeGroups,
    int activeGroupCount,
    int partyMapIndex,
    int mapIndex,
    int groupIndex,
    unsigned int *outCells);

int dm1_v1_dungeon_set_group_cells_f0146_pc34(
    struct DungeonThings_Compat *things,
    struct CreatureAIState_Compat *activeGroups,
    int activeGroupCount,
    int partyMapIndex,
    int mapIndex,
    int groupIndex,
    unsigned int cells);

int dm1_v1_dungeon_get_group_directions_f0147_pc34(
    const struct DungeonThings_Compat *things,
    const struct CreatureAIState_Compat *activeGroups,
    int activeGroupCount,
    int partyMapIndex,
    int mapIndex,
    int groupIndex,
    unsigned int *outDirections);

int dm1_v1_dungeon_set_group_directions_f0148_pc34(
    struct DungeonThings_Compat *things,
    struct CreatureAIState_Compat *activeGroups,
    int activeGroupCount,
    int partyMapIndex,
    int mapIndex,
    int groupIndex,
    unsigned int directions);

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

/* ReDMCSB GROUP.C F0177: resolve an adjacent raw C04 melee target. */
int dm1_v1_group_get_melee_target_ordinal_f0177_pc34(
    const struct DungeonDatState_Compat *dungeon,
    const struct DungeonThings_Compat *things,
    int mapIndex,
    int groupMapX,
    int groupMapY,
    int partyMapX,
    int partyMapY,
    unsigned int championCell,
    unsigned int groupCells,
    unsigned int groupDirections);

const char *dm1_v1_dungeon_thing_data_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif

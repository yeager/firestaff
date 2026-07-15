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

const char *dm1_v1_dungeon_thing_data_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif

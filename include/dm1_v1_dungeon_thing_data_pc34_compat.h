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

const char *dm1_v1_dungeon_thing_data_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif

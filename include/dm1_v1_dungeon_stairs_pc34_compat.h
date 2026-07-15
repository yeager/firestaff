#ifndef FIRESTAFF_DM1_V1_DUNGEON_STAIRS_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_DUNGEON_STAIRS_PC34_COMPAT_H

#include "memory_movement_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB DUNGEON.C F0154/F0155, consumed by DM1 command runtime. */
int dm1_v1_dungeon_resolve_stairs_transition_pc34(
    const struct DungeonDatState_Compat *dungeon,
    const struct PartyState_Compat *party,
    struct StairsTransitionResult_Compat *outResult);

const char *dm1_v1_dungeon_stairs_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif

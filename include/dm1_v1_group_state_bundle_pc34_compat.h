#ifndef FIRESTAFF_DM1_V1_GROUP_STATE_BUNDLE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_GROUP_STATE_BUNDLE_PC34_COMPAT_H

#include <stdint.h>

#include "memory_tick_orchestrator_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Source-shaped ACTIVE_GROUP row used at the C04/F0435 boundary. */
typedef struct DM1_V1_SourceActiveGroupPc34Compat {
    int groupThing;
    int cells;
    int directions;
    int lastMoveTime;
    int delayFleeingFromTarget;
    int targetMapX;
    int targetMapY;
    int priorMapX;
    int priorMapY;
    int homeMapX;
    int homeMapY;
    uint8_t aspect[4];
} DM1_V1_SourceActiveGroupPc34Compat;

typedef struct DM1_V1_GroupStateBundleReceiptPc34Compat {
    int valid;
    int activeGroupCount;
    int sourceCapacity;
    int groupIndex;
    int mapIndex;
    unsigned int cells;
    unsigned int directions;
    uint32_t fingerprint;
    const char *sourceSymbol;
} DM1_V1_GroupStateBundleReceiptPc34Compat;

/* GROUP.C F0196 for PC 3.4.  The exact sixty source owners are staged before
 * publishing, preserving any host-only storage beyond that range. */
int dm1_v1_group_state_initialize_f0196_pc34(
    struct GameWorld_Compat *world,
    DM1_V1_GroupStateBundleReceiptPc34Compat *outReceipt);

/* DUNGEON.C F0146/F0148 as one group-state transaction.  On the party map
 * only the matching ACTIVE_GROUP row changes; elsewhere the raw C04 row is
 * updated.  A bad owner or raw row leaves both fields unchanged. */
int dm1_v1_group_state_write_f0146_f0148_pc34(
    struct GameWorld_Compat *world,
    int mapIndex,
    int groupIndex,
    unsigned int cells,
    unsigned int directions,
    DM1_V1_GroupStateBundleReceiptPc34Compat *outReceipt);

/* LOADSAVE.C F0435 C04 handoff.  All rows are admitted into a candidate
 * runtime first, so malformed cells/directions/group owners cannot partially
 * replace the live active-group state. */
int dm1_v1_group_state_apply_save_handoff_pc34(
    struct GameWorld_Compat *world,
    const DM1_V1_SourceActiveGroupPc34Compat *rows,
    int currentCount,
    int sourceCapacity,
    DM1_V1_GroupStateBundleReceiptPc34Compat *outReceipt);

const char *dm1_v1_group_state_bundle_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif

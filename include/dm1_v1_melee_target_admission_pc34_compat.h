#ifndef FIRESTAFF_DM1_V1_MELEE_TARGET_ADMISSION_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_MELEE_TARGET_ADMISSION_PC34_COMPAT_H

#include <stdint.h>

#include "dm1_v1_creature_ai_behavior_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* GROUP.C F0229 / PROJEXPL.C F0230 boundary. This validates the source
 * C04/C38-C41 owner and previews F0229's RNG work before the existing F0230
 * damage consumer chooses and applies its champion target. */
typedef struct {
    const struct DungeonThings_Compat *things;
    int groupIndex;
    const struct DungeonGroup_Compat *group;
    const struct DM1ActiveGroup_Compat *activeGroup;
    const struct TimelineEvent_Compat *event;
    int partyMapIndex;
    int partyMapX;
    int partyMapY;
    int creatureIndex;
    const struct RngState_Compat *rng;
} DM1_MeleeTargetAdmissionInputPc34;

typedef struct {
    int valid;
    int targetMapIndex;
    int targetMapX;
    int targetMapY;
    int orderedCells[4];
    uint32_t rawC04FNV1a;
    uint32_t c29C41FNV1a;
    uint32_t rngBeforeFNV1a;
    uint32_t rngAfterFNV1a;
    const char *sourceAnchor;
} DM1_MeleeTargetAdmissionReceiptPc34;

int dm1_v1_melee_target_admit_f0229_f0230_pc34(
    const DM1_MeleeTargetAdmissionInputPc34 *input,
    DM1_MeleeTargetAdmissionReceiptPc34 *outReceipt);

#ifdef __cplusplus
}
#endif

#endif

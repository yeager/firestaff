#ifndef FIRESTAFF_DM1_V1_GROUP_LOS_DIRECTION_ADMISSION_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_GROUP_LOS_DIRECTION_ADMISSION_PC34_COMPAT_H

#include <stdint.h>

#include "dm1_v1_creature_ai_behavior_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* F0227/F0228 pre-dispatch admission. It verifies C04 plus the live C29-C41
 * identity and previews the source RNG on a copy. The caller keeps the only
 * mutable RNG consumption in its existing F0209 runtime owner. */
typedef struct {
    const struct DungeonThings_Compat *things;
    int groupIndex;
    const struct CreatureAIState_Compat *activeGroup;
    unsigned char activeDirections;
    const struct TimelineEvent_Compat *event;
    int partyMapIndex;
    int partyMapX;
    int partyMapY;
    const struct RngState_Compat *rng;
} DM1_GroupLosDirectionAdmissionInputPc34;

typedef struct {
    int valid;
    int primaryDirection;
    int secondaryDirection;
    uint32_t rawC04FNV1a;
    uint32_t c29C41FNV1a;
    uint32_t rngBeforeFNV1a;
    uint32_t rngAfterFNV1a;
    const char *sourceAnchor;
} DM1_GroupLosDirectionAdmissionReceiptPc34;

int dm1_v1_group_los_direction_admit_f0227_f0228_pc34(
    const DM1_GroupLosDirectionAdmissionInputPc34 *input,
    DM1_GroupLosDirectionAdmissionReceiptPc34 *outReceipt);

#ifdef __cplusplus
}
#endif

#endif

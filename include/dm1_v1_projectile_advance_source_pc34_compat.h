#ifndef FIRESTAFF_DM1_V1_PROJECTILE_ADVANCE_SOURCE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_PROJECTILE_ADVANCE_SOURCE_PC34_COMPAT_H

#include <stdint.h>

#include "dm1_v1_original_save_pc34_handoff.h"
#include "dm1_v1_throw_shoot_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB PROJEXPL.C F0812/F0813/F0814 source admission.  This receipt does
 * not simulate a projectile: it admits only a completed F0811/F0825 result
 * whose C14, C48/C49 and optional C15 identities still match PC34 data. */
typedef struct {
    const struct DungeonThings_Compat *things;
    const DM1OriginalSavePC34ProjectileEventPlan *sourcePlan;
    const struct TimelineEvent_Compat *sourceEvent;
    const struct ProjectileInstance_Compat *before;
    const struct ProjectileInstance_Compat *after;
    const struct ProjectileTickResult_Compat *result;
    const struct CellContentDigest_Compat *destination;
    const DM1_V1_F0248LiveEffectMaterialReceiptPc34 *projectileMaterial;
    const DM1_V1_F0248LiveEffectMaterialReceiptPc34 *explosionMaterial;
    const DM1_ProjectileTerminationSourceReceiptPc34 *termination;
    uint32_t dispatchTick;
} DM1_ProjectileAdvanceSourceInputPc34;

typedef struct {
    int valid;
    int shouldReschedule;
    int shouldTerminate;
    int collisionResult;
    unsigned short projectileThing;
    uint32_t rawC14FNV1a;
    uint32_t sourceEventFNV1a;
    uint32_t runtimeEventFNV1a;
    uint32_t rawC15FNV1a;
    const char *sourceAnchor;
} DM1_ProjectileAdvanceSourceReceiptPc34;

/* Returns one for a handled receipt. Invalid source data produces valid=0;
 * no runtime projectile, material, event, or palette fallback is invented. */
int dm1_v1_projectile_advance_source_receipt_f0812_f0814_pc34(
    const DM1_ProjectileAdvanceSourceInputPc34 *input,
    DM1_ProjectileAdvanceSourceReceiptPc34 *outReceipt);

#ifdef __cplusplus
}
#endif

#endif

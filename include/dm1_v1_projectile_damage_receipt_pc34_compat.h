#ifndef FIRESTAFF_DM1_V1_PROJECTILE_DAMAGE_RECEIPT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_PROJECTILE_DAMAGE_RECEIPT_PC34_COMPAT_H

#include <stdint.h>

#include "dm1_v1_projectile_impact_world_receipt_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB PROJEXPL.C F0221-F0226 source admission for post-impact damage.
 * It validates owners and already-produced result packets only; state changes
 * remain in the existing F0217/F0230/F0190/F0321 owners. */
typedef struct {
    const struct DungeonThings_Compat *things;
    const DM1_ProjectileAdvanceSourceReceiptPc34 *advance;
    const DM1_ProjectileTerminationSourceReceiptPc34 *termination;
    const struct TimelineEvent_Compat *c48C49Event;
    const struct ProjectileInstance_Compat *projectile;
    const struct ProjectileTickResult_Compat *impact;
    const DM1_V1_F0248LiveEffectMaterialReceiptPc34 *projectileMaterial;
    int groupIndex;
    const DM1_ProjectileCreatureActionPlanPc34 *creaturePlan;
    const DM1_ProjectileCreatureActionApplyPlanPc34 *creatureApply;
    const DM1_ProjectileCreatureImpactAftermathPc34 *creatureAftermath;
    const DM1_ProjectileChampionImpactPlanPc34 *championPlan;
    const DM1_ProjectileChampionDamageApplyPlanPc34 *championApply;
    const DM1_ProjectileChampionPoisonApplyPlanPc34 *championPoison;
    const struct TimelineEvent_Compat *postImpactEvent;
    const DM1_ProjectileImpactWorldReceiptPc34 *explosionWorld;
    uint32_t dispatchTick;
} DM1_ProjectileDamageSourceInputPc34;

typedef struct {
    int valid;
    int shouldApplyCreatureDamage;
    int shouldApplyChampionDamage;
    int shouldSchedulePostImpactEvent;
    int impactResult;
    unsigned short projectileThing;
    uint32_t rawC14FNV1a;
    uint32_t c48C49FNV1a;
    uint32_t c14MaterialFNV1a;
    const char *sourceAnchor;
} DM1_ProjectileDamageSourceReceiptPc34;

int dm1_v1_projectile_damage_source_receipt_f0221_f0226_pc34(
    const DM1_ProjectileDamageSourceInputPc34 *input,
    DM1_ProjectileDamageSourceReceiptPc34 *outReceipt);

#ifdef __cplusplus
}
#endif

#endif

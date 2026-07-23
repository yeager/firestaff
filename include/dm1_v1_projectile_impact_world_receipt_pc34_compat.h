#ifndef FIRESTAFF_DM1_V1_PROJECTILE_IMPACT_WORLD_RECEIPT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_PROJECTILE_IMPACT_WORLD_RECEIPT_PC34_COMPAT_H

#include <stdint.h>

#include "dm1_v1_c15_layout_pc34_compat.h"
#include "dm1_v1_projectile_advance_source_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB PROJEXPL.C F0216-F0220 post-collision admission. It consumes
 * already-authenticated F0812-F0814 and F0213-F0215 receipts; it never
 * allocates an effect, substitutes pixels, or mutates a world. */
typedef struct {
    const struct DungeonDatState_Compat *dungeon;
    const struct DungeonThings_Compat *things;
    const DM1_ProjectileAdvanceSourceReceiptPc34 *advance;
    const DM1_ProjectileTerminationSourceReceiptPc34 *termination;
    const struct ProjectileTickResult_Compat *impact;
    const DM1_V1_F0248LiveEffectMaterialReceiptPc34 *explosionMaterial;
    const DM1_C15C25PublicationReceiptPc34 *c25;
    const struct TimelineEvent_Compat *c25Event;
    const struct ExplosionInstance_Compat *explosionBefore;
    const struct ExplosionInstance_Compat *explosionAfter;
    const struct ExplosionTickResult_Compat *explosionResult;
} DM1_ProjectileImpactWorldInputPc34;

typedef struct {
    int valid;
    int shouldDispatchImpact;
    int shouldAdvanceExplosion;
    int shouldRemoveExplosion;
    int impactResult;
    unsigned short c15Thing;
    uint32_t rawC14FNV1a;
    uint32_t c48C49FNV1a;
    uint32_t rawC15FNV1a;
    uint32_t c25FNV1a;
    const char *sourceAnchor;
} DM1_ProjectileImpactWorldReceiptPc34;

/* Returns one for a handled receipt. Invalid or stale PC34 data produces
 * valid=0 and leaves all actual world mutation to its existing owners. */
int dm1_v1_projectile_impact_world_receipt_f0216_f0220_pc34(
    const DM1_ProjectileImpactWorldInputPc34 *input,
    DM1_ProjectileImpactWorldReceiptPc34 *outReceipt);

#ifdef __cplusplus
}
#endif

#endif

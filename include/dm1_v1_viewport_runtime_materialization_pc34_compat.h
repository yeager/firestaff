#ifndef FIRESTAFF_DM1_V1_VIEWPORT_RUNTIME_MATERIALIZATION_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_RUNTIME_MATERIALIZATION_PC34_COMPAT_H

#include "memory_projectile_pc34_compat.h"

/*
 * DM1 V1 visible viewport materialization receipt.
 *
 * ReDMCSB: DUNGEON.C F0172 publishes square facts; DUNVIEW.C F0115
 * (4547-4581, 4923, 5075, 5668-5683, 5915-5933) consumes those facts in
 * distinct wall, object, projectile and deferred-explosion passes.
 */

#ifdef __cplusplus
extern "C" {
#endif

typedef enum DM1_V1_ViewportRuntimeOriginPc34 {
    DM1_V1_VIEWPORT_RUNTIME_ORIGIN_NEW_START_PC34 = 0,
    DM1_V1_VIEWPORT_RUNTIME_ORIGIN_ORIGINAL_SAVE_PC34 = 1,
    DM1_V1_VIEWPORT_RUNTIME_ORIGIN_QUICKSAVE_RESUME_PC34 = 2
} DM1_V1_ViewportRuntimeOriginPc34;

typedef struct DM1_V1_ViewportRuntimeMaterializationInputPc34 {
    int relativeForward;
    int relativeSide;
    int elementType;
    int floorItemCount;
    int projectileCount;
    int projectileCell;
    int hasVisibleChampionMirrorPayload;
    int mapIndex;
    int mapX;
    int mapY;
    int partyDirection;
    int suppressFluxcages;
    const struct ProjectileList_Compat *liveProjectiles;
    const struct ExplosionList_Compat *liveExplosions;
    DM1_V1_ViewportRuntimeOriginPc34 runtimeOrigin;
} DM1_V1_ViewportRuntimeMaterializationInputPc34;

typedef struct DM1_V1_ViewportRuntimeMaterializationDecisionPc34 {
    int valid;
    int consumedF0172SquareFacts;
    int consumedF0115ThingPass;
    int viewSquare;
    int row;
    int itemZone;
    int projectileZone;
    int drawFloorItems;
    int drawRuntimeProjectiles;
    int drawDeferredSpellEffects;
    int suppressMaterializedItemPayload;
    int suppressMirrorAsFloorItem;
    int suppressMirrorAsProjectile;
    int suppressMirrorAsSpellEffect;
    int noM11Fallback;
    int liveProjectileCount;
    int liveProjectileSlot;
    int liveProjectileSubtype;
    unsigned short liveProjectileAssociatedThing;
    int liveProjectileCell;
    int liveProjectileDirection;
    int liveExplosionCount;
    int liveExplosionSlot;
    int liveExplosionType;
    int liveExplosionFrame;
    int liveExplosionMaxFrames;
    int liveExplosionAttack;
    const char *sourceAnchor;
} DM1_V1_ViewportRuntimeMaterializationDecisionPc34;

int dm1_v1_viewport_runtime_materialization_decide_pc34(
    const DM1_V1_ViewportRuntimeMaterializationInputPc34 *input,
    DM1_V1_ViewportRuntimeMaterializationDecisionPc34 *outDecision);

const char *dm1_v1_viewport_runtime_materialization_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_VIEWPORT_RUNTIME_MATERIALIZATION_PC34_COMPAT_H */

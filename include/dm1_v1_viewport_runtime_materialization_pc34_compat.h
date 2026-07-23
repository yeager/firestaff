#ifndef FIRESTAFF_DM1_V1_VIEWPORT_RUNTIME_MATERIALIZATION_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_RUNTIME_MATERIALIZATION_PC34_COMPAT_H

#include "memory_projectile_pc34_compat.h"
#include "dm1_v1_object_world_pc34_compat.h"

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

#define DM1_V1_VIEWPORT_RUNTIME_MATERIALIZATION_MAX_RENDERABLE_EXPLOSIONS 8
#define DM1_V1_VIEWPORT_RUNTIME_MATERIALIZATION_MAX_RENDERABLE_PROJECTILES \
    PROJECTILE_LIST_CAPACITY

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
    const DM1_V1_ObjectWorldGraphicsSurfacePc34 *pc34GraphicsSurfaces;
    int pc34GraphicsSurfaceCount;
    DM1_V1_ViewportRuntimeOriginPc34 runtimeOrigin;
} DM1_V1_ViewportRuntimeMaterializationInputPc34;

typedef struct DM1_V1_ViewportRuntimeMaterializationDecisionPc34 {
    int valid;
    int consumedF0172SquareFacts;
    int consumedF0115ThingPass;
    int f0115EligibleForSquare;
    int suppressedF0115ForStairs;
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
    int admittedLiveProjectileCount;
    int liveProjectileSlot;
    int liveProjectileSubtype;
    int liveProjectileCell;
    int liveProjectileDirection;
    unsigned short liveProjectileAssociatedThing;
    /* F0115 consumes every live C14 projectile on the visible square in
     * effect-list order.  The legacy scalar fields above retain the first
     * record for callers that only need a summary. */
    int liveRenderableProjectileCount;
    int liveRenderableProjectileSlots[
        DM1_V1_VIEWPORT_RUNTIME_MATERIALIZATION_MAX_RENDERABLE_PROJECTILES];
    int liveRenderableProjectileSubtypes[
        DM1_V1_VIEWPORT_RUNTIME_MATERIALIZATION_MAX_RENDERABLE_PROJECTILES];
    int liveRenderableProjectileCells[
        DM1_V1_VIEWPORT_RUNTIME_MATERIALIZATION_MAX_RENDERABLE_PROJECTILES];
    int liveRenderableProjectileDirections[
        DM1_V1_VIEWPORT_RUNTIME_MATERIALIZATION_MAX_RENDERABLE_PROJECTILES];
    unsigned short liveRenderableProjectileAssociatedThings[
        DM1_V1_VIEWPORT_RUNTIME_MATERIALIZATION_MAX_RENDERABLE_PROJECTILES];
    int liveExplosionCount;
    int admittedLiveExplosionCount;
    int liveExplosionSlot;
    int liveExplosionType;
    int liveExplosionFrame;
    int liveExplosionMaxFrames;
    int liveExplosionAttack;
    int liveRenderableExplosionCount;
    int liveRenderableExplosionTypes[
        DM1_V1_VIEWPORT_RUNTIME_MATERIALIZATION_MAX_RENDERABLE_EXPLOSIONS];
    int liveRenderableExplosionFrames[
        DM1_V1_VIEWPORT_RUNTIME_MATERIALIZATION_MAX_RENDERABLE_EXPLOSIONS];
    int liveRenderableExplosionMaxFrames[
        DM1_V1_VIEWPORT_RUNTIME_MATERIALIZATION_MAX_RENDERABLE_EXPLOSIONS];
    int liveRenderableExplosionAttacks[
        DM1_V1_VIEWPORT_RUNTIME_MATERIALIZATION_MAX_RENDERABLE_EXPLOSIONS];
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

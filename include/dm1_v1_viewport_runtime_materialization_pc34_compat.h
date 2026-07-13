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

/* ReDMCSB F0115 restarts the C15 loop and may consume every active
 * explosion record on the square.  Keep the runtime receipt bounded by the
 * PC34/M10 explosion slot table rather than collapsing it to a synthetic
 * "first effect" presentation. */
#define DM1_V1_VIEWPORT_RUNTIME_MAX_EXPLOSIONS_PC34 EXPLOSION_LIST_CAPACITY

/* ReDMCSB DUNVIEW.C F0115:5915-6220 restarts at the original C15 list and
 * dispatches each record by type and view square.  These route values are a
 * receipt of that source dispatch, not permission to substitute material. */
typedef enum DM1_V1_C15ExplosionRoutePc34 {
    DM1_V1_C15_EXPLOSION_ROUTE_ORDINARY_F0114_PC34 = 0,
    DM1_V1_C15_EXPLOSION_ROUTE_ORDINARY_D0C_M636_PC34 = 1,
    DM1_V1_C15_EXPLOSION_ROUTE_C100_C3000_BLOCKED_PC34 = 2,
    DM1_V1_C15_EXPLOSION_ROUTE_C101_C3007_BLOCKED_PC34 = 3,
    DM1_V1_C15_EXPLOSION_ROUTE_C101_D0C_M636_PC34 = 4,
    DM1_V1_C15_EXPLOSION_ROUTE_FLUXCAGE_F0113_PC34 = 5
} DM1_V1_C15ExplosionRoutePc34;

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
    int liveVisibleProjectileCount;
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
    /* Source-list receipt.  This preserves the live PC34/M10 C15 order
     * before the route-specific material gates below filter what M11 may
     * draw.  C100/C101 therefore cannot be lost or merged with ordinary
     * F0114 effects merely because their routes differ. */
    int liveExplosionSourceCount;
    int liveExplosionSourceSlots[DM1_V1_VIEWPORT_RUNTIME_MAX_EXPLOSIONS_PC34];
    int liveExplosionSourceTypes[DM1_V1_VIEWPORT_RUNTIME_MAX_EXPLOSIONS_PC34];
    int liveExplosionSourceRoutes[DM1_V1_VIEWPORT_RUNTIME_MAX_EXPLOSIONS_PC34];
    /* ReDMCSB F0115 routes C101 through D0C's native M636 fire pattern, but
     * C100 jumps to C3000/lightning geometry. Retain the latter as an
     * explicit no-draw receipt until its PC34 zone/scale decoder is proven. */
    int liveD0cRebirthStep1Count;
    int liveD0cRebirthStep1GeometryBlocked;
    /* Only ordinary F0114-scaled explosions are admitted here.  Fluxcages
     * use F0113's field route and rebirth has distinct C3000/C3007 geometry,
     * so neither may borrow this material path. */
    int liveRenderableExplosionCount;
    int liveRenderableExplosionSlots[DM1_V1_VIEWPORT_RUNTIME_MAX_EXPLOSIONS_PC34];
    int liveRenderableExplosionTypes[DM1_V1_VIEWPORT_RUNTIME_MAX_EXPLOSIONS_PC34];
    int liveRenderableExplosionFrames[DM1_V1_VIEWPORT_RUNTIME_MAX_EXPLOSIONS_PC34];
    int liveRenderableExplosionMaxFrames[DM1_V1_VIEWPORT_RUNTIME_MAX_EXPLOSIONS_PC34];
    int liveRenderableExplosionAttacks[DM1_V1_VIEWPORT_RUNTIME_MAX_EXPLOSIONS_PC34];
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

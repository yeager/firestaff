#include "dm1_v1_viewport_runtime_materialization_pc34_compat.h"

#include "dm1_v1_viewport_3d_pc34_compat.h"

#include <string.h>

static int dm1_v1_viewport_runtime_projectile_is_active_pc34(
    const struct ProjectileInstance_Compat *projectile)
{
    return projectile && projectile->slotIndex >= 0 && projectile->reserved3 != 0;
}

static int dm1_v1_viewport_runtime_explosion_is_active_pc34(
    const struct ExplosionInstance_Compat *explosion)
{
    return explosion && explosion->slotIndex >= 0 && explosion->reserved0 != 0;
}

const char *dm1_v1_viewport_runtime_materialization_source_evidence_pc34(void)
{
    return "ReDMCSB DUNGEON.C F0172 lines 2466-2589 publishes each visible "
           "square aspect; DUNVIEW.C F0115 lines 4547-4581 separates the "
           "thing passes, line 4923 clips non-visible cells, line 5075 "
           "selects C2500 object zones, lines 5668-5683 select C2900 "
           "projectile zones, and lines 5915-6200 restart and consume every "
           "deferred explosion. DUNVIEW.C lines 3913-3928 reserve the D1C champion "
           "mirror for the wall-overlay route.";
}

int dm1_v1_viewport_runtime_materialization_decide_pc34(
    const DM1_V1_ViewportRuntimeMaterializationInputPc34 *input,
    DM1_V1_ViewportRuntimeMaterializationDecisionPc34 *outDecision)
{
    DM1_V1_ViewportRuntimeMaterializationDecisionPc34 decision;
    int itemX;
    int itemY;
    int projectileX;
    int projectileY;
    int projectileCell;
    int isD1c;

    if (!input || !outDecision) {
        return 0;
    }
    memset(&decision, 0, sizeof(decision));
    decision.viewSquare = dm1_viewport_3d_f0115_view_square_index(
        input->relativeForward, input->relativeSide);
    decision.row = dm1_viewport_3d_f0115_c2500_c2900_row(
        input->relativeForward, input->relativeSide);
    decision.itemZone = -1;
    decision.projectileZone = -1;
    decision.liveProjectileSlot = -1;
    decision.liveProjectileSubtype = -1;
    decision.liveProjectileAssociatedThing = THING_NONE;
    decision.liveProjectileCell = -1;
    decision.liveProjectileDirection = -1;
    decision.liveExplosionSlot = -1;
    decision.liveExplosionType = -1;
    decision.liveExplosionFrame = -1;
    decision.liveExplosionMaxFrames = 0;
    decision.liveExplosionAttack = -1;
    decision.noM11Fallback = 1;
    decision.sourceAnchor = dm1_v1_viewport_runtime_materialization_source_evidence_pc34();

    /* The receipt is reconstructed from F0172/F0115 facts on every draw.
     * Save provenance never changes the source routing. */
    if (input->runtimeOrigin < DM1_V1_VIEWPORT_RUNTIME_ORIGIN_NEW_START_PC34 ||
        input->runtimeOrigin > DM1_V1_VIEWPORT_RUNTIME_ORIGIN_QUICKSAVE_RESUME_PC34 ||
        decision.viewSquare < 0 || decision.row < 0) {
        *outDecision = decision;
        return 1;
    }

    /* ReDMCSB: PROJEXPL.C F0219:644-764 and F0220:765 onward mutate the
     * active effect records. F0115:5668-5683 / 5916-5933 consumes those
     * live records at draw time, not an earlier square thing-chain snapshot. */
    if (input->liveProjectiles) {
        int i;
        for (i = 0; i < input->liveProjectiles->count &&
                    i < PROJECTILE_LIST_CAPACITY; ++i) {
            const struct ProjectileInstance_Compat *projectile =
                &input->liveProjectiles->entries[i];
            int viewCell;
            int projectileX;
            int projectileY;
            if (!dm1_v1_viewport_runtime_projectile_is_active_pc34(projectile) ||
                projectile->mapIndex != input->mapIndex ||
                projectile->mapX != input->mapX || projectile->mapY != input->mapY) {
                continue;
            }
            ++decision.liveProjectileCount;
            viewCell = (projectile->cell - input->partyDirection) & 3;
            /* ReDMCSB DUNVIEW.C F0115:5668-5683 visits every C14 record,
             * restores its view cell, and draws only when G2028/C2900 has
             * a coordinate for that cell.  Keep scanning after an invisible
             * record so it cannot hide a later materialized projectile. */
            if (!dm1_viewport_3d_c2900_projectile_raw_zone_point(
                    decision.row, viewCell, &projectileX, &projectileY)) {
                continue;
            }
            (void)projectileX;
            (void)projectileY;
            ++decision.liveVisibleProjectileCount;
            if (decision.liveProjectileSlot < 0) {
                decision.liveProjectileSlot = projectile->slotIndex;
                decision.liveProjectileSubtype = projectile->projectileSubtype;
                decision.liveProjectileAssociatedThing =
                    (unsigned short)projectile->reserved1;
                decision.liveProjectileCell = viewCell;
                decision.liveProjectileDirection = projectile->direction;
            }
        }
    }
    if (input->liveExplosions) {
        int i;
        for (i = 0; i < input->liveExplosions->count &&
                    i < EXPLOSION_LIST_CAPACITY; ++i) {
            const struct ExplosionInstance_Compat *explosion =
                &input->liveExplosions->entries[i];
            if (!dm1_v1_viewport_runtime_explosion_is_active_pc34(explosion) ||
                (input->suppressFluxcages &&
                 explosion->explosionType == C050_EXPLOSION_FLUXCAGE) ||
                explosion->mapIndex != input->mapIndex ||
                explosion->mapX != input->mapX || explosion->mapY != input->mapY) {
                continue;
            }
            ++decision.liveExplosionCount;
            if (decision.liveExplosionSlot < 0) {
                decision.liveExplosionSlot = explosion->slotIndex;
                decision.liveExplosionType = explosion->explosionType;
                decision.liveExplosionFrame = explosion->currentFrame;
                decision.liveExplosionMaxFrames =
                    explosion->maxFrames > 0 ? explosion->maxFrames : 1;
                decision.liveExplosionAttack = explosion->attack;
            }
            /* ReDMCSB DUNVIEW.C F0115:5915-6200 restarts the C15 walk and
             * draws every ordinary explosion through F0114. Fluxcages are
             * deferred to the F0113 field route, while C100/C101 rebirth
             * effects require their own C3000/C3007 geometry. Do not let
             * either class borrow the ordinary F0114 material path. */
            if (explosion->explosionType != C050_EXPLOSION_FLUXCAGE &&
                explosion->explosionType < C100_EXPLOSION_REBIRTH_STEP1 &&
                decision.liveRenderableExplosionCount <
                    DM1_V1_VIEWPORT_RUNTIME_MAX_EXPLOSIONS_PC34) {
                int index = decision.liveRenderableExplosionCount++;
                decision.liveRenderableExplosionSlots[index] = explosion->slotIndex;
                decision.liveRenderableExplosionTypes[index] = explosion->explosionType;
                decision.liveRenderableExplosionFrames[index] = explosion->currentFrame;
                decision.liveRenderableExplosionMaxFrames[index] =
                    explosion->maxFrames > 0 ? explosion->maxFrames : 1;
                decision.liveRenderableExplosionAttacks[index] = explosion->attack;
            }
        }
    }

    decision.valid = 1;
    decision.consumedF0172SquareFacts = 1;
    decision.consumedF0115ThingPass = 1;
    isD1c = input->relativeForward == 1 && input->relativeSide == 0;

    if (input->floorItemCount > 0 && input->elementType != DM1_VP_ELEMENT_WALL &&
        dm1_viewport_3d_c2500_object_raw_zone_point(decision.row, 2,
                                                     &itemX, &itemY)) {
        (void)itemX;
        (void)itemY;
        decision.itemZone = 2500 + decision.row * 4 + 2;
        decision.drawFloorItems = 1;
    }
    projectileCell = input->projectileCell;
    if (decision.liveProjectileSlot >= 0) {
        projectileCell = decision.liveProjectileCell;
    }
    if ((input->projectileCount > 0 || decision.liveProjectileSlot >= 0) &&
        dm1_viewport_3d_c2900_projectile_raw_zone_point(
            decision.row, projectileCell, &projectileX, &projectileY)) {
        (void)projectileX;
        (void)projectileY;
        decision.projectileZone = 2900 + decision.row * 4 + projectileCell;
        decision.drawRuntimeProjectiles = 1;
    }

    /* C127/C026 is a D1C wall overlay, never an F0115 item, projectile or
     * materialized spell payload. Independent real things still use F0115. */
    if (isD1c && input->hasVisibleChampionMirrorPayload) {
        decision.suppressMaterializedItemPayload = 1;
        decision.suppressMirrorAsFloorItem = 1;
        decision.suppressMirrorAsProjectile = 1;
        decision.suppressMirrorAsSpellEffect = 1;
    }
    decision.drawDeferredSpellEffects =
        decision.suppressMirrorAsSpellEffect ? 0 : 1;
    *outDecision = decision;
    return 1;
}

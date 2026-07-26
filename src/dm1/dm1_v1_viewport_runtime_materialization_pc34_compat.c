#include "dm1_v1_viewport_runtime_materialization_pc34_compat.h"

#include "dm1_v1_viewport_3d_pc34_compat.h"
#include "dm1_v1_c15_layout_pc34_compat.h"
#include "dm1_v1_dungeon_thing_data_pc34_compat.h"
#include "dm1_v1_object_world_pc34_compat.h"
#include "dm1_v1_projectile_explosion_render_pc34_compat.h"

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

int dm1_v1_viewport_runtime_fluxcage_source_bound_pc34(
    const struct DungeonDatState_Compat *dungeon,
    const struct DungeonThings_Compat *things,
    const struct ExplosionList_Compat *liveExplosions,
    int mapIndex,
    int mapX,
    int mapY)
{
    unsigned short thing;
    int safety = 0;

    if (!dungeon || !things || !liveExplosions || !things->loaded ||
        !things->explosions || !things->rawThingData[THING_TYPE_EXPLOSION] ||
        mapIndex < 0 || mapIndex >= dungeon->header.mapCount) {
        return 0;
    }
    /* Original PC34 data always uses F0160's compact SFT table.  Dense SFT
     * remains only for isolated legacy fixtures and has the same raw C15
     * validation below; it is never selected for a loaded compact dungeon. */
    if (dungeon->columnsCumulativeSquareFirstThingCount &&
        dungeon->dungeonColumnCount > 0) {
        thing = F0511_DUNGEON_GetSquareFirstThing_Compat(
            dungeon, things, mapIndex, mapX, mapY);
    } else {
        const struct DungeonMapDesc_Compat *map =
            dungeon->maps ? &dungeon->maps[mapIndex] : NULL;
        int denseIndex;
        if (!map || mapX < 0 || mapY < 0 || mapX >= map->width ||
            mapY >= map->height) return 0;
        denseIndex = mapX * (int)map->height + mapY;
        if (denseIndex < 0 || denseIndex >= things->squareFirstThingCount) {
            return 0;
        }
        thing = things->squareFirstThings[denseIndex];
    }
    while (thing != THING_NONE && thing != THING_ENDOFLIST && safety++ < 64) {
        if (THING_GET_TYPE(thing) == THING_TYPE_EXPLOSION) {
            const unsigned char *raw = dm1_v1_dungeon_get_thing_data_pc34(
                things, thing);
            uint32_t fingerprint;
            int sourceIndex = THING_GET_INDEX(thing);
            int slot;

            if (!raw || sourceIndex < 0 || sourceIndex >= things->explosionCount ||
                sourceIndex >= things->thingCounts[THING_TYPE_EXPLOSION] ||
                things->explosionCount != things->thingCounts[THING_TYPE_EXPLOSION] ||
                (unsigned short)(raw[0] | ((unsigned short)raw[1] << 8)) !=
                    things->explosions[sourceIndex].next ||
                (raw[2] & 0x7fu) != things->explosions[sourceIndex].type ||
                ((raw[2] >> 7) & 1u) != things->explosions[sourceIndex].centered ||
                raw[3] != things->explosions[sourceIndex].attack) {
                return 0;
            }
            if ((raw[2] & 0x7fu) != C050_EXPLOSION_FLUXCAGE) {
                thing = F0512_DUNGEON_GetThingNext_Compat(things, thing);
                continue;
            }
            fingerprint = dm1_v1_c15_layout_fingerprint_pc34(raw, 4u);
            if (fingerprint == 0u) return 0;
            for (slot = 0; slot < liveExplosions->count &&
                           slot < EXPLOSION_LIST_CAPACITY; ++slot) {
                const struct ExplosionInstance_Compat *live =
                    &liveExplosions->entries[slot];
                if (dm1_v1_viewport_runtime_explosion_is_active_pc34(live) &&
                    live->explosionType == C050_EXPLOSION_FLUXCAGE &&
                    live->mapIndex == mapIndex && live->mapX == mapX &&
                    live->mapY == mapY &&
                    (live->sourceC15Fingerprint == 0u ||
                     live->sourceC15Fingerprint == fingerprint)) {
                    return 1;
                }
            }
        }
        thing = F0512_DUNGEON_GetThingNext_Compat(things, thing);
    }
    return 0;
}

static const DM1_V1_F0248LiveEffectMaterialReceiptPc34 *
dm1_v1_viewport_runtime_find_effect_receipt_pc34(
    const DM1_V1_ViewportRuntimeMaterializationInputPc34 *input,
    DM1_V1_F0248LiveEffectKindPc34 kind,
    unsigned short rawThing)
{
    int i;
    if (!input || !input->c14C15MaterialReceipts ||
        input->c14C15MaterialReceiptCount <= 0) return NULL;
    for (i = 0; i < input->c14C15MaterialReceiptCount; ++i) {
        const DM1_V1_F0248LiveEffectMaterialReceiptPc34 *receipt =
            &input->c14C15MaterialReceipts[i];
        if (receipt->rawThing != rawThing) continue;
        if (kind == DM1_V1_F0248_LIVE_EFFECT_PROJECTILE_C14_PC34 &&
            THING_GET_TYPE(receipt->rawThing) == THING_TYPE_PROJECTILE) return receipt;
        if (kind == DM1_V1_F0248_LIVE_EFFECT_EXPLOSION_C15_PC34 &&
            THING_GET_TYPE(receipt->rawThing) == THING_TYPE_EXPLOSION) return receipt;
    }
    return NULL;
}

static int dm1_v1_viewport_runtime_admit_effect_pc34(
    const DM1_V1_ViewportRuntimeMaterializationInputPc34 *input,
    DM1_V1_F0248LiveEffectKindPc34 kind,
    unsigned short rawThing,
    unsigned short associatedThing,
    int expectedGraphicIndex)
{
    if (input && input->c14C15GraphicsCatalog) {
        return dm1_v1_c14_c15_graphics_catalog_admit_receipt_pc34(
            input->c14C15GraphicsCatalog,
            dm1_v1_viewport_runtime_find_effect_receipt_pc34(input, kind, rawThing),
            kind, rawThing, associatedThing, expectedGraphicIndex);
    }
    if (kind == DM1_V1_F0248_LIVE_EFFECT_PROJECTILE_C14_PC34 &&
        associatedThing != THING_NONE) return 0;
    if (!input || (!input->pc34GraphicsSurfaces && input->pc34GraphicsSurfaceCount <= 0))
        return 1;
    return DM1_V1_ObjectWorld_AdmitPc34GraphicsSurfacePc34Compat(
        input->pc34GraphicsSurfaces, input->pc34GraphicsSurfaceCount,
        expectedGraphicIndex);
}

static int dm1_v1_viewport_runtime_f0115_eligible_for_square_pc34(
    const DM1_V1_ViewportRuntimeMaterializationInputPc34 *input,
    int *out_suppressed_for_stairs)
{
    int stairs;
    int lateral_stairs;

    if (out_suppressed_for_stairs) {
        *out_suppressed_for_stairs = 0;
    }
    if (!input || input->elementType == DM1_VP_ELEMENT_WALL) {
        return 0;
    }

    stairs = input->elementType == DM1_VP_ELEMENT_STAIRS ||
             input->elementType == DM1_VP_ELEMENT_STAIRS_SIDE ||
             input->elementType == DM1_VP_ELEMENT_STAIRS_FRONT;
    if (!stairs) {
        return 1;
    }

    lateral_stairs = input->relativeSide != 0 &&
                     input->relativeForward >= 1 &&
                     input->relativeForward <= 3 &&
                     input->elementType != DM1_VP_ELEMENT_STAIRS_FRONT;
    if (!lateral_stairs && out_suppressed_for_stairs) {
        *out_suppressed_for_stairs = 1;
    }
    return lateral_stairs;
}

const char *dm1_v1_viewport_runtime_materialization_source_evidence_pc34(void)
{
    return "ReDMCSB DUNGEON.C F0172 lines 2466-2589 publishes each visible "
           "square aspect; DUNVIEW.C F0115 lines 4547-4581 separates the "
           "thing passes, line 4923 clips non-visible cells, line 5075 "
           "selects C2500 object zones, lines 5668-5683 select C2900 "
           "projectile zones, and lines 5915-5933 restart for deferred "
           "explosions. DUNVIEW.C lines 3913-3928 reserve the D1C champion "
           "mirror for the wall-overlay route. DUNVIEW.C F0116-F0123 routes "
           "D3-D1 lateral stairs through their door-side F0115 tails, while "
           "F0118/F0121/F0124 front-stairs and F0125/F0126 D0 lateral stairs "
           "return before F0115.";
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
    int f0115Eligible;

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
    decision.liveProjectileCell = -1;
    decision.liveProjectileDirection = -1;
    decision.liveProjectileAssociatedThing = THING_NONE;
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

    f0115Eligible = dm1_v1_viewport_runtime_f0115_eligible_for_square_pc34(
        input, &decision.suppressedF0115ForStairs);

    /* ReDMCSB: PROJEXPL.C F0219:644-764 and F0220:765 onward mutate the
     * active effect records. F0115:5668-5683 / 5916-5933 consumes those
     * live records at draw time, not an earlier square thing-chain snapshot. */
    if (f0115Eligible && input->liveProjectiles) {
        int i;
        for (i = 0; i < input->liveProjectiles->count &&
                    i < PROJECTILE_LIST_CAPACITY; ++i) {
            const struct ProjectileInstance_Compat *projectile =
                &input->liveProjectiles->entries[i];
            if (!dm1_v1_viewport_runtime_projectile_is_active_pc34(projectile) ||
                projectile->mapIndex != input->mapIndex ||
                projectile->mapX != input->mapX || projectile->mapY != input->mapY) {
                continue;
            }
            ++decision.liveProjectileCount;
            /* Keep the live C14 receipt for diagnostics and later exact
             * F0142/G0209 binding even when this draw is correctly denied. */
            if (decision.liveProjectileSlot < 0) {
                decision.liveProjectileSlot = projectile->slotIndex;
                decision.liveProjectileSubtype = projectile->projectileSubtype;
                decision.liveProjectileCell = projectile->cell;
                decision.liveProjectileDirection = projectile->direction;
                decision.liveProjectileAssociatedThing =
                    (unsigned short)projectile->reserved1;
            }
            /* F0115 must not promote a live C14 into a host marker. A C14
             * with an associated object needs its F0142/G0209 material
             * receipt from the caller; this native M613 branch is admitted
             * only with a verified PC34 GRAPHICS.DAT surface. */
            {
                unsigned short rawThing = (unsigned short)(
                    (THING_TYPE_PROJECTILE << 10) | (projectile->slotIndex & 0x03ff));
                const DM1_V1_F0248LiveEffectMaterialReceiptPc34 *receipt =
                    dm1_v1_viewport_runtime_find_effect_receipt_pc34(
                        input, DM1_V1_F0248_LIVE_EFFECT_PROJECTILE_C14_PC34,
                        rawThing);
                int expectedGraphicIndex = dm1_v1_projectile_subtype_graphic_index(
                    projectile->projectileSubtype);
                /* The production receipt is the F0142/G0209 authority for
                 * a carried C14 object. Native M613 material retains its
                 * projectile aspect. */
                if (projectile->reserved1 != THING_NONE && receipt) {
                    expectedGraphicIndex = receipt->graphicIndex;
                }
                if (!dm1_v1_viewport_runtime_admit_effect_pc34(
                    input, DM1_V1_F0248_LIVE_EFFECT_PROJECTILE_C14_PC34,
                    rawThing,
                    (unsigned short)projectile->reserved1,
                    expectedGraphicIndex)) {
                    continue;
                }
            }
            ++decision.admittedLiveProjectileCount;
            if (decision.liveRenderableProjectileCount <
                DM1_V1_VIEWPORT_RUNTIME_MATERIALIZATION_MAX_RENDERABLE_PROJECTILES) {
                int renderIndex = decision.liveRenderableProjectileCount++;
                decision.liveRenderableProjectileSlots[renderIndex] =
                    projectile->slotIndex;
                decision.liveRenderableProjectileSubtypes[renderIndex] =
                    projectile->projectileSubtype;
                decision.liveRenderableProjectileCells[renderIndex] =
                    projectile->cell;
                decision.liveRenderableProjectileDirections[renderIndex] =
                    projectile->direction;
                decision.liveRenderableProjectileAssociatedThings[renderIndex] =
                    (unsigned short)projectile->reserved1;
            }
        }
    }
    if (f0115Eligible && input->liveExplosions) {
        int i;
        for (i = 0; i < input->liveExplosions->count &&
                    i < EXPLOSION_LIST_CAPACITY; ++i) {
            const struct ExplosionInstance_Compat *explosion =
                &input->liveExplosions->entries[i];
            /* ReDMCSB DUNVIEW.C:5958-5994 branches the live C15 record's
             * type into fire/poison/smoke/spell aspects; a negative type
             * has no original material and is a no-draw for that record,
             * matching F0115's per-record no-draw rule for broken C14s. */
            if (!dm1_v1_viewport_runtime_explosion_is_active_pc34(explosion) ||
                explosion->explosionType < 0 ||
                (input->suppressFluxcages &&
                 explosion->explosionType == C050_EXPLOSION_FLUXCAGE) ||
                explosion->mapIndex != input->mapIndex ||
                explosion->mapX != input->mapX || explosion->mapY != input->mapY) {
                continue;
            }
            ++decision.liveExplosionCount;
            /* ReDMCSB F0115:6006-6219 defers C050 from the ordinary C15
             * bitmap branch and presents it as the source-bound fluxcage
             * field. It has no explosion-pattern GRAPHICS.DAT index. */
            if (explosion->explosionType != C050_EXPLOSION_FLUXCAGE &&
                !dm1_v1_viewport_runtime_admit_effect_pc34(
                    input, DM1_V1_F0248_LIVE_EFFECT_EXPLOSION_C15_PC34,
                    (unsigned short)((THING_TYPE_EXPLOSION << 10) |
                                     (explosion->slotIndex & 0x03ff)),
                    THING_NONE,
                    dm1_v1_explosion_pattern_graphic_index(
                        explosion->explosionType, explosion->attack))) {
                continue;
            }
            if (explosion->explosionType == C050_EXPLOSION_FLUXCAGE &&
                !input->sourceBoundFluxcage) {
                continue;
            }
            ++decision.admittedLiveExplosionCount;
            if (decision.liveExplosionSlot < 0) {
                decision.liveExplosionSlot = explosion->slotIndex;
                decision.liveExplosionType = explosion->explosionType;
                decision.liveExplosionFrame = explosion->currentFrame;
                decision.liveExplosionMaxFrames =
                    explosion->maxFrames > 0 ? explosion->maxFrames : 1;
                decision.liveExplosionAttack = explosion->attack;
            }
            if (decision.liveRenderableExplosionCount <
                DM1_V1_VIEWPORT_RUNTIME_MATERIALIZATION_MAX_RENDERABLE_EXPLOSIONS) {
                int renderIndex = decision.liveRenderableExplosionCount++;
                decision.liveRenderableExplosionTypes[renderIndex] =
                    explosion->explosionType;
                decision.liveRenderableExplosionFrames[renderIndex] =
                    explosion->currentFrame;
                decision.liveRenderableExplosionMaxFrames[renderIndex] =
                    explosion->maxFrames > 0 ? explosion->maxFrames : 1;
                decision.liveRenderableExplosionAttacks[renderIndex] =
                    explosion->attack;
            }
        }
    }

    decision.valid = 1;
    decision.consumedF0172SquareFacts = 1;
    decision.consumedF0115ThingPass = f0115Eligible;
    decision.f0115EligibleForSquare = f0115Eligible;
    isD1c = input->relativeForward == 1 && input->relativeSide == 0;

    if (f0115Eligible && input->floorItemCount > 0 &&
        dm1_viewport_3d_c2500_object_raw_zone_point(decision.row, 2,
                                                     &itemX, &itemY)) {
        (void)itemX;
        (void)itemY;
        decision.itemZone = 2500 + decision.row * 4 + 2;
        decision.drawFloorItems = 1;
    }
    projectileCell = input->projectileCell;
    /* F0115 examines each live C14 record in effect-list order.  A record
     * whose normalized cell has no C2900 source coordinate is a no-draw;
     * it must not suppress a later record that does have original material. */
    if (decision.liveRenderableProjectileCount > 0) {
        int projectileIndex;
        projectileCell = -1;
        for (projectileIndex = 0;
             projectileIndex < decision.liveRenderableProjectileCount;
             ++projectileIndex) {
            int candidateCell =
                (decision.liveRenderableProjectileCells[projectileIndex] -
                 input->partyDirection) & 3;
            if (dm1_viewport_3d_c2900_projectile_raw_zone_point(
                    decision.row, candidateCell, &projectileX, &projectileY)) {
                projectileCell = candidateCell;
                break;
            }
        }
    }
    if (f0115Eligible && projectileCell >= 0 &&
        (input->projectileCount > 0 || decision.admittedLiveProjectileCount > 0) &&
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
        f0115Eligible && !decision.suppressMirrorAsSpellEffect;
    *outDecision = decision;
    return 1;
}

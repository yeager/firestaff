#include "dm1_v1_projectile_advance_source_pc34_compat.h"

#include <string.h>

static uint32_t dm1_v1_projectile_advance_fnv1a_pc34(const unsigned char *bytes,
                                                      size_t byteCount)
{
    uint32_t hash = 2166136261u;
    size_t index;

    if (!bytes || byteCount == 0u) return 0u;
    for (index = 0u; index < byteCount; ++index) {
        hash ^= bytes[index];
        hash *= 16777619u;
    }
    return hash;
}

static int dm1_v1_projectile_advance_runtime_matches_source_pc34(
    const DM1OriginalSavePC34ProjectileEventPlan *plan,
    const struct TimelineEvent_Compat *event,
    const struct ProjectileInstance_Compat *projectile,
    uint32_t dispatchTick)
{
    if (!plan || !event || !projectile || !plan->valid ||
        projectile->slotIndex != plan->projectile_index ||
        projectile->reserved3 == 0 ||
        projectile->mapIndex != plan->map_index ||
        projectile->mapX != plan->map_x ||
        projectile->mapY != plan->map_y ||
        projectile->cell != plan->cell ||
        projectile->direction != plan->direction ||
        projectile->stepEnergy != plan->step_energy ||
        projectile->kineticEnergy != plan->kinetic_energy ||
        projectile->attack != plan->attack ||
        (unsigned short)projectile->reserved1 != plan->associated_thing ||
        event->kind != TIMELINE_EVENT_PROJECTILE_MOVE ||
        event->fireAtTick != dispatchTick ||
        event->mapIndex != plan->map_index || event->mapX != plan->map_x ||
        event->mapY != plan->map_y || event->cell != plan->cell ||
        event->aux0 != plan->projectile_index ||
        (unsigned short)event->aux1 != plan->source_thing ||
        event->aux2 != plan->source_event_type) {
        return 0;
    }
    return plan->source_event_type == DM1_EVENT_MOVE_PROJECTILE ||
           plan->source_event_type == DM1_EVENT_MOVE_PROJECTILE_IGNORE_IMPACTS;
}

static int dm1_v1_projectile_advance_result_is_collision_pc34(int resultKind)
{
    return resultKind >= PROJECTILE_RESULT_HIT_WALL &&
           resultKind <= PROJECTILE_RESULT_DESPAWN_BOUNDS;
}

static int dm1_v1_projectile_advance_reschedule_matches_pc34(
    const struct ProjectileInstance_Compat *before,
    const struct ProjectileInstance_Compat *after,
    const struct ProjectileTickResult_Compat *result,
    uint32_t dispatchTick)
{
    const struct TimelineEvent_Compat *next;
    uint32_t expectedTick;

    if (!before || !after || !result || result->despawn ||
        result->resultKind != PROJECTILE_RESULT_FLEW ||
        after->slotIndex != before->slotIndex || after->reserved3 == 0) {
        return 0;
    }
    expectedTick = dispatchTick + (uint32_t)(
        after->mapIndex == before->mapIndex ? 1 : 3);
    next = &result->outNextTick;
    return next->kind == TIMELINE_EVENT_PROJECTILE_MOVE &&
           next->fireAtTick == expectedTick &&
           next->mapIndex == after->mapIndex && next->mapX == after->mapX &&
           next->mapY == after->mapY && next->cell == (after->cell & 3) &&
           next->aux0 == after->slotIndex &&
           next->aux1 == after->ownerKind && next->aux2 == after->ownerIndex &&
           next->aux3 == after->projectileSubtype;
}

static int dm1_v1_projectile_advance_c15_matches_result_pc34(
    const struct DungeonThings_Compat *things,
    const struct ProjectileTickResult_Compat *result,
    const DM1_V1_F0248LiveEffectMaterialReceiptPc34 *material)
{
    const struct DungeonExplosion_Compat *explosion;
    const unsigned char *raw;
    int index;

    if (!things || !things->loaded || !result || !material ||
        !material->valid || material->noDraw || !material->saveReceiptBound ||
        THING_GET_TYPE(material->rawThing) != THING_TYPE_EXPLOSION ||
        material->rawRecordFNV1a == 0u || material->graphicsPixelsFNV1a == 0u ||
        material->paletteFNV1a == 0u) {
        return 0;
    }
    index = (int)THING_GET_INDEX(material->rawThing);
    if (index < 0 || index >= things->explosionCount ||
        things->thingCounts[THING_TYPE_EXPLOSION] != things->explosionCount ||
        !things->explosions || !things->rawThingData[THING_TYPE_EXPLOSION]) {
        return 0;
    }
    explosion = &things->explosions[index];
    raw = things->rawThingData[THING_TYPE_EXPLOSION] + (size_t)index * 4u;
    if ((unsigned short)(raw[0] | ((unsigned short)raw[1] << 8)) != explosion->next ||
        (raw[2] & 0x7fu) != explosion->type ||
        ((raw[2] >> 7) & 1u) != explosion->centered ||
        raw[3] != explosion->attack ||
        result->outExplosion.explosionType != explosion->type ||
        result->outExplosion.attack != explosion->attack ||
        result->outExplosion.centered != explosion->centered) {
        return 0;
    }
    return material->rawRecordFNV1a ==
        dm1_v1_projectile_advance_fnv1a_pc34(raw, 4u);
}

int dm1_v1_projectile_advance_source_receipt_f0812_f0814_pc34(
    const DM1_ProjectileAdvanceSourceInputPc34 *input,
    DM1_ProjectileAdvanceSourceReceiptPc34 *outReceipt)
{
    DM1_ProjectileAdvanceSourceReceiptPc34 receipt;
    int blocker;

    if (!outReceipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.sourceAnchor =
        "ReDMCSB PROJEXPL.C F0812/F0813/F0814, F0219/F0825; "
        "DEFS.H C14/C15/C48/C49";
    *outReceipt = receipt;
    if (!input || !input->things || !input->sourcePlan || !input->sourceEvent ||
        !input->before || !input->after || !input->result ||
        !input->destination || !input->projectileMaterial ||
        !dm1_v1_projectile_advance_runtime_matches_source_pc34(
            input->sourcePlan, input->sourceEvent, input->before,
            input->dispatchTick) ||
        !dm1_v1_original_save_pc34_projectile_replay_material_receipt_pc34(
            input->sourcePlan, input->projectileMaterial)) {
        return 1;
    }
    F0814_PROJECTILE_InspectDestination_Compat(input->destination, &blocker);
    if (input->result->resultKind == PROJECTILE_RESULT_INVALID ||
        (input->result->resultKind == PROJECTILE_RESULT_HIT_WALL &&
         blocker != PROJECTILE_BLOCKER_WALL && blocker != PROJECTILE_BLOCKER_STAIRS &&
         blocker != PROJECTILE_BLOCKER_BOUNDARY) ||
        (input->result->resultKind == PROJECTILE_RESULT_HIT_FLUXCAGE &&
         blocker != PROJECTILE_BLOCKER_FLUXCAGE) ||
        (input->result->resultKind == PROJECTILE_RESULT_HIT_OTHER_PROJECTILE &&
         blocker != PROJECTILE_BLOCKER_OTHER_PROJECTILE &&
         !input->destination->sourceHasOtherProjectile)) {
        return 1;
    }
    receipt.rawC14FNV1a = input->sourcePlan->raw_c14_fingerprint;
    receipt.sourceEventFNV1a = input->sourcePlan->source_event_fingerprint;
    receipt.runtimeEventFNV1a = dm1_v1_projectile_advance_fnv1a_pc34(
        (const unsigned char *)input->sourceEvent, sizeof(*input->sourceEvent));
    if (receipt.rawC14FNV1a == 0u || receipt.sourceEventFNV1a == 0u ||
        receipt.runtimeEventFNV1a == 0u) {
        return 1;
    }
    if (input->result->resultKind == PROJECTILE_RESULT_FLEW) {
        if (!dm1_v1_projectile_advance_reschedule_matches_pc34(
                input->before, input->after, input->result,
                input->dispatchTick)) {
            return 1;
        }
        receipt.valid = 1;
        receipt.shouldReschedule = 1;
    } else {
        if (!input->result->despawn ||
            !dm1_v1_projectile_advance_result_is_collision_pc34(
                input->result->resultKind) ||
            input->result->outNextTick.kind != 0 ||
            !input->termination || !input->termination->valid ||
            !input->termination->shouldTerminate ||
            input->termination->projectileThing != input->sourcePlan->source_thing ||
            input->termination->rawC14FNV1a != receipt.rawC14FNV1a) {
            return 1;
        }
        if (input->result->emittedExplosion) {
            if (!dm1_v1_projectile_advance_c15_matches_result_pc34(
                    input->things, input->result, input->explosionMaterial)) {
                return 1;
            }
            receipt.rawC15FNV1a = input->explosionMaterial->rawRecordFNV1a;
        }
        receipt.valid = 1;
        receipt.shouldTerminate = 1;
    }
    receipt.collisionResult = input->result->resultKind;
    receipt.projectileThing = input->sourcePlan->source_thing;
    *outReceipt = receipt;
    return 1;
}

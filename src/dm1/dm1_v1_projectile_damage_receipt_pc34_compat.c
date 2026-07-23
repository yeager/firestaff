#include "dm1_v1_projectile_damage_receipt_pc34_compat.h"

#include <string.h>

static uint32_t dm1_v1_projectile_damage_fnv1a_pc34(const unsigned char *bytes,
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

static int dm1_v1_projectile_damage_raw_c14_matches_pc34(
    const struct DungeonThings_Compat *things,
    const struct ProjectileInstance_Compat *projectile,
    const DM1_V1_F0248LiveEffectMaterialReceiptPc34 *material,
    uint32_t expectedFingerprint)
{
    const struct DungeonProjectile_Compat *decoded;
    const unsigned char *raw;
    int index;
    uint32_t fingerprint;

    if (!things || !things->loaded || !projectile || !material ||
        projectile->slotIndex < 0 || projectile->reserved3 == 0 ||
        projectile->slotIndex >= things->projectileCount ||
        things->thingCounts[THING_TYPE_PROJECTILE] != things->projectileCount ||
        !things->projectiles || !things->rawThingData[THING_TYPE_PROJECTILE] ||
        !material->valid || material->noDraw || !material->saveReceiptBound ||
        THING_GET_TYPE(material->rawThing) != THING_TYPE_PROJECTILE ||
        (int)THING_GET_INDEX(material->rawThing) != projectile->slotIndex) {
        return 0;
    }
    index = projectile->slotIndex;
    decoded = &things->projectiles[index];
    raw = things->rawThingData[THING_TYPE_PROJECTILE] + (size_t)index * 8u;
    fingerprint = dm1_v1_projectile_damage_fnv1a_pc34(raw, 8u);
    return fingerprint != 0u && fingerprint == expectedFingerprint &&
           fingerprint == material->rawRecordFNV1a &&
           (unsigned short)(raw[0] | ((unsigned short)raw[1] << 8)) == decoded->next &&
           (unsigned short)(raw[2] | ((unsigned short)raw[3] << 8)) == decoded->slot &&
           raw[4] == decoded->kineticEnergy && raw[5] == decoded->attack &&
           (unsigned short)(raw[6] | ((unsigned short)raw[7] << 8)) == decoded->eventIndex &&
           decoded->slot == (unsigned short)projectile->reserved1;
}

int dm1_v1_projectile_damage_source_receipt_f0221_f0226_pc34(
    const DM1_ProjectileDamageSourceInputPc34 *input,
    DM1_ProjectileDamageSourceReceiptPc34 *outReceipt)
{
    DM1_ProjectileDamageSourceReceiptPc34 receipt;
    const struct CombatAction_Compat *action;
    uint32_t eventFingerprint;

    if (!outReceipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.sourceAnchor =
        "ReDMCSB PROJEXPL.C F0221/F0222/F0223/F0224/F0225/F0226; "
        "F0217/F0219 C14/C48/C49 post-impact ownership";
    *outReceipt = receipt;
    if (!input || !input->advance || !input->termination ||
        !input->c48C49Event || !input->projectile || !input->impact ||
        !input->projectileMaterial || !input->advance->valid ||
        !input->termination->valid || !input->termination->shouldTerminate ||
        input->advance->rawC14FNV1a == 0u ||
        input->advance->rawC14FNV1a != input->termination->rawC14FNV1a ||
        !dm1_v1_projectile_damage_raw_c14_matches_pc34(
            input->things, input->projectile, input->projectileMaterial,
            input->advance->rawC14FNV1a)) {
        return 1;
    }
    eventFingerprint = dm1_v1_projectile_damage_fnv1a_pc34(
        (const unsigned char *)input->c48C49Event,
        sizeof(*input->c48C49Event));
    if (eventFingerprint == 0u ||
        eventFingerprint != input->advance->runtimeEventFNV1a ||
        input->c48C49Event->kind != TIMELINE_EVENT_PROJECTILE_MOVE ||
        input->c48C49Event->aux0 != input->projectile->slotIndex ||
        input->c48C49Event->fireAtTick != input->dispatchTick ||
        !input->impact->despawn || !input->impact->emittedCombatAction ||
        input->impact->outNextTick.kind != 0 ||
        (input->impact->resultKind != PROJECTILE_RESULT_HIT_CREATURE &&
         input->impact->resultKind != PROJECTILE_RESULT_HIT_CHAMPION)) {
        return 1;
    }
    if (input->impact->emittedExplosion &&
        (!input->explosionWorld || !input->explosionWorld->valid ||
         input->explosionWorld->rawC14FNV1a != input->advance->rawC14FNV1a)) {
        return 1;
    }
    action = &input->impact->outAction;
    if (input->impact->resultKind == PROJECTILE_RESULT_HIT_CREATURE) {
        if (!input->creaturePlan || !input->creatureApply ||
            action->kind != COMBAT_ACTION_APPLY_DAMAGE_GROUP ||
            !input->creaturePlan->handled ||
            input->creaturePlan->damageApplied != action->rawAttackValue ||
            !input->creatureApply->valid ||
            (input->creaturePlan->shouldApplyDamage &&
             (!input->creatureApply->handled ||
              input->creatureApply->damageApplied != action->rawAttackValue))) {
            return 1;
        }
        receipt.shouldApplyCreatureDamage = input->creaturePlan->shouldApplyDamage;
        if (input->creatureAftermath && input->creatureAftermath->scheduleReaction) {
            if (!input->postImpactEvent ||
                input->postImpactEvent->kind != TIMELINE_EVENT_CREATURE_REACTION ||
                input->postImpactEvent->fireAtTick < input->dispatchTick ||
                input->postImpactEvent->mapIndex != action->targetMapIndex ||
                input->postImpactEvent->mapX != action->targetMapX ||
                input->postImpactEvent->mapY != action->targetMapY ||
                input->postImpactEvent->aux0 != input->groupIndex) {
                return 1;
            }
            receipt.shouldSchedulePostImpactEvent = 1;
        }
    } else {
        if (!input->championPlan || !input->championApply ||
            action->kind != COMBAT_ACTION_APPLY_DAMAGE_CHAMPION ||
            !input->championPlan->handled || !input->championPlan->championPresent ||
            input->championPlan->championIndex != action->defenderSlotOrCreatureIndex ||
            !input->championApply->valid ||
            input->championApply->championIndex != input->championPlan->championIndex) {
            return 1;
        }
        receipt.shouldApplyChampionDamage = 1;
        if (input->championPoison && input->championPoison->schedulePoisonEvent) {
            if (!input->postImpactEvent ||
                memcmp(input->postImpactEvent, &input->championPoison->poisonEvent,
                       sizeof(*input->postImpactEvent)) != 0) {
                return 1;
            }
            receipt.shouldSchedulePostImpactEvent = 1;
        }
    }
    receipt.valid = 1;
    receipt.impactResult = input->impact->resultKind;
    receipt.projectileThing = input->projectileMaterial->rawThing;
    receipt.rawC14FNV1a = input->advance->rawC14FNV1a;
    receipt.c48C49FNV1a = input->advance->runtimeEventFNV1a;
    receipt.c14MaterialFNV1a = input->projectileMaterial->graphicsPixelsFNV1a;
    *outReceipt = receipt;
    return 1;
}

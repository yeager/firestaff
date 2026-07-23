#include "dm1_v1_projectile_impact_world_receipt_pc34_compat.h"

#include <string.h>

static int dm1_v1_projectile_impact_c25_event_matches_pc34(
    const DM1_C15C25PublicationReceiptPc34 *c25,
    const struct TimelineEvent_Compat *event,
    const struct ExplosionInstance_Compat *explosion)
{
    if (!c25 || !event || !explosion || !c25->active ||
        event->kind != TIMELINE_EVENT_EXPLOSION_ADVANCE ||
        event->fireAtTick != (c25->mapTime & 0x00ffffffu) ||
        event->mapIndex != (int)(c25->mapTime >> 24) ||
        event->mapX != c25->mapX || event->mapY != c25->mapY ||
        event->cell != (int)THING_GET_CELL(c25->slot) ||
        event->aux0 != (int)THING_GET_INDEX(c25->slot) ||
        event->aux1 != explosion->explosionType ||
        event->aux2 != explosion->attack || event->aux4 != c25->priority ||
        explosion->slotIndex != (int)THING_GET_INDEX(c25->slot) ||
        explosion->mapIndex != (int)(c25->mapTime >> 24) ||
        explosion->mapX != c25->mapX || explosion->mapY != c25->mapY ||
        explosion->cell != (int)THING_GET_CELL(c25->slot)) {
        return 0;
    }
    return 1;
}

static int dm1_v1_projectile_impact_c15_matches_pc34(
    const struct DungeonThings_Compat *things,
    const DM1_V1_F0248LiveEffectMaterialReceiptPc34 *material,
    const DM1_C15C25PublicationReceiptPc34 *c25,
    const struct ExplosionInstance_Compat *explosion)
{
    const unsigned char *raw;
    int index;

    if (!things || !things->loaded || !material || !c25 || !explosion ||
        !material->valid || material->noDraw || !material->saveReceiptBound ||
        THING_GET_TYPE(material->rawThing) != THING_TYPE_EXPLOSION ||
        THING_GET_TYPE(c25->slot) != THING_TYPE_EXPLOSION ||
        THING_GET_INDEX(material->rawThing) != THING_GET_INDEX(c25->slot) ||
        !things->rawThingData[THING_TYPE_EXPLOSION] || !things->explosions ||
        material->rawRecordFNV1a == 0u || material->graphicsPixelsFNV1a == 0u ||
        material->paletteFNV1a == 0u) {
        return 0;
    }
    index = (int)THING_GET_INDEX(c25->slot);
    if (index < 0 || index >= things->explosionCount ||
        things->thingCounts[THING_TYPE_EXPLOSION] != things->explosionCount) {
        return 0;
    }
    raw = things->rawThingData[THING_TYPE_EXPLOSION] + (size_t)index * 4u;
    return material->rawRecordFNV1a == c25->c15Fingerprint &&
           material->rawRecordFNV1a == dm1_v1_c15_layout_fingerprint_pc34(raw, 4u) &&
           things->explosions[index].next ==
               (unsigned short)(raw[0] | ((unsigned short)raw[1] << 8)) &&
           things->explosions[index].type == (raw[2] & 0x7fu) &&
           things->explosions[index].centered == ((raw[2] >> 7) & 1u) &&
           things->explosions[index].attack == raw[3] &&
           explosion->explosionType == things->explosions[index].type &&
           explosion->centered == things->explosions[index].centered &&
           explosion->attack == things->explosions[index].attack;
}

int dm1_v1_projectile_impact_world_receipt_f0216_f0220_pc34(
    const DM1_ProjectileImpactWorldInputPc34 *input,
    DM1_ProjectileImpactWorldReceiptPc34 *outReceipt)
{
    DM1_ProjectileImpactWorldReceiptPc34 receipt;
    const struct ProjectileTickResult_Compat *impact;
    const struct ExplosionTickResult_Compat *explosionResult;

    if (!outReceipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.sourceAnchor =
        "ReDMCSB PROJEXPL.C F0216/F0217/F0218/F0219/F0220; "
        "DEFS.H C14/C15/C48/C49/C25";
    *outReceipt = receipt;
    if (!input || !input->dungeon || !input->things || !input->advance ||
        !input->termination || !(impact = input->impact) ||
        !input->c25 || !input->c25Event || !input->explosionBefore ||
        !input->explosionAfter || !(explosionResult = input->explosionResult) ||
        !input->advance->valid || !input->termination->valid ||
        !input->termination->shouldTerminate || !impact->despawn ||
        !impact->emittedExplosion ||
        input->advance->rawC14FNV1a == 0u ||
        input->advance->sourceEventFNV1a == 0u ||
        input->advance->rawC14FNV1a != input->termination->rawC14FNV1a ||
        !dm1_v1_c15_c25_receipt_is_live_pc34(input->c25, input->dungeon,
                                               input->things) ||
        !dm1_v1_projectile_impact_c15_matches_pc34(
            input->things, input->explosionMaterial, input->c25,
            input->explosionBefore) ||
        !dm1_v1_projectile_impact_c25_event_matches_pc34(
            input->c25, input->c25Event, input->explosionBefore) ||
        impact->outExplosion.explosionType != input->explosionBefore->explosionType ||
        impact->outExplosion.attack != input->explosionBefore->attack ||
        impact->outExplosion.centered != input->explosionBefore->centered ||
        impact->outExplosion.mapIndex != input->explosionBefore->mapIndex ||
        impact->outExplosion.mapX != input->explosionBefore->mapX ||
        impact->outExplosion.mapY != input->explosionBefore->mapY ||
        impact->outExplosion.cell != input->explosionBefore->cell ||
        explosionResult->resultKind == EXPLOSION_RESULT_INVALID ||
        input->explosionAfter->currentFrame !=
            input->explosionBefore->currentFrame + 1) {
        return 1;
    }
    if (explosionResult->despawn) {
        if (explosionResult->resultKind != EXPLOSION_RESULT_ONE_SHOT ||
            explosionResult->outNextTick.kind != 0) {
            return 1;
        }
        receipt.shouldRemoveExplosion = 1;
    } else {
        if ((explosionResult->resultKind != EXPLOSION_RESULT_ADVANCED_FRAME &&
             explosionResult->resultKind != EXPLOSION_RESULT_PERSISTENT) ||
            explosionResult->outNextTick.kind != TIMELINE_EVENT_EXPLOSION_ADVANCE ||
            explosionResult->outNextTick.mapIndex != input->explosionAfter->mapIndex ||
            explosionResult->outNextTick.mapX != input->explosionAfter->mapX ||
            explosionResult->outNextTick.mapY != input->explosionAfter->mapY ||
            explosionResult->outNextTick.aux0 != input->explosionAfter->slotIndex) {
            return 1;
        }
        receipt.shouldAdvanceExplosion = 1;
    }
    receipt.valid = 1;
    receipt.shouldDispatchImpact = 1;
    receipt.impactResult = impact->resultKind;
    receipt.c15Thing = input->c25->slot;
    receipt.rawC14FNV1a = input->advance->rawC14FNV1a;
    receipt.c48C49FNV1a = input->advance->sourceEventFNV1a;
    receipt.rawC15FNV1a = input->explosionMaterial->rawRecordFNV1a;
    receipt.c25FNV1a = input->c25->c15Fingerprint;
    *outReceipt = receipt;
    return 1;
}

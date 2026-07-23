#include "dm1_v1_projectile_terminal_materialization_f0217_pc34_compat.h"

#include "dm1_v1_dungeon_thing_data_pc34_compat.h"
#include "memory_projectile_pc34_compat.h"

#include <string.h>

static int dm1_v1_f0217_raw_thing_size_pc34(int type)
{
    switch (type) {
        case THING_TYPE_WEAPON:
        case THING_TYPE_ARMOUR:
        case THING_TYPE_SCROLL:
        case THING_TYPE_POTION:
        case THING_TYPE_JUNK:
            return 4;
        case THING_TYPE_CONTAINER:
            return 8;
        default:
            return 0;
    }
}

static unsigned short dm1_v1_f0217_read_u16le_pc34(const unsigned char *bytes)
{
    return (unsigned short)(bytes[0] | ((unsigned short)bytes[1] << 8));
}

static int dm1_v1_f0217_c14_matches_pc34(
    const struct DungeonThings_Compat *things,
    const DM1_ProjectileTerminationSourceReceiptPc34 *termination,
    const DM1_ProjectileMaterializationReceiptPc34 *f0215,
    const DM1_V1_F0248LiveEffectMaterialReceiptPc34 *material)
{
    const unsigned char *raw;
    const struct DungeonProjectile_Compat *decoded;
    int index;

    if (!things || !things->loaded || !termination || !f0215 || !material ||
        !termination->valid || !termination->shouldTerminate || !f0215->valid ||
        !f0215->handled || !f0215->shouldDeleteProjectile ||
        !f0215->shouldClearProjectileNext || !f0215->shouldUnlinkProjectileFromSquare ||
        !material->valid || material->noDraw || !material->saveReceiptBound ||
        THING_GET_TYPE(termination->projectileThing) != THING_TYPE_PROJECTILE ||
        f0215->projectileThing != termination->projectileThing ||
        material->rawThing != termination->projectileThing ||
        !things->projectiles || !things->rawThingData[THING_TYPE_PROJECTILE]) {
        return 0;
    }
    index = (int)THING_GET_INDEX(termination->projectileThing);
    if (index < 0 || index >= things->projectileCount ||
        things->thingCounts[THING_TYPE_PROJECTILE] != things->projectileCount) {
        return 0;
    }
    raw = things->rawThingData[THING_TYPE_PROJECTILE] + (size_t)index * 8u;
    decoded = &things->projectiles[index];
    return dm1_v1_f0115_source_material_fnv1a_pc34(raw, 8u) != 0u &&
           dm1_v1_f0115_source_material_fnv1a_pc34(raw, 8u) ==
               termination->rawC14FNV1a &&
           termination->rawC14FNV1a == material->rawRecordFNV1a &&
           dm1_v1_f0217_read_u16le_pc34(raw) == decoded->next &&
           dm1_v1_f0217_read_u16le_pc34(raw + 2) == decoded->slot &&
           raw[4] == decoded->kineticEnergy && raw[5] == decoded->attack &&
           dm1_v1_f0217_read_u16le_pc34(raw + 6) == decoded->eventIndex;
}

static int dm1_v1_f0217_associated_material_matches_pc34(
    const struct DungeonThings_Compat *things,
    const DM1_ProjectileTerminationSourceReceiptPc34 *termination,
    const DM1_ProjectileMaterializationReceiptPc34 *f0215,
    const DM1_V1_F0115SourceMaterialHandoffPc34 *material,
    uint32_t *outRawFingerprint)
{
    const unsigned char *raw;
    int type;
    int size;
    uint32_t fingerprint;

    if (outRawFingerprint) *outRawFingerprint = 0u;
    if (!things || !termination || !f0215 || !material ||
        !f0215->shouldMaterialize || f0215->shouldConsumePotion ||
        !termination->shouldMaterializeAssociatedThing ||
        f0215->materialization.associatedThing != termination->associatedThing ||
        !f0215->squareAttach.valid || !f0215->squareAttach.shouldSetDroppedNextEnd ||
        !material->valid || material->noDraw || !material->usesF0791Blit ||
        material->transparentColor != 10 || material->graphicIndex <= 0 ||
        !material->pixels || material->pixelCount == 0u ||
        material->materialFNV1a == 0u ||
        material->materialFNV1a != dm1_v1_f0115_source_material_fnv1a_pc34(
            material->pixels, material->pixelCount)) {
        return 0;
    }
    type = (int)THING_GET_TYPE(termination->associatedThing);
    size = dm1_v1_f0217_raw_thing_size_pc34(type);
    raw = dm1_v1_dungeon_get_thing_data_pc34(things, termination->associatedThing);
    if (size == 0 || !raw) return 0;
    fingerprint = dm1_v1_f0115_source_material_fnv1a_pc34(raw, (size_t)size);
    if (fingerprint == 0u || fingerprint != termination->rawAssociatedThingFNV1a) {
        return 0;
    }
    if (outRawFingerprint) *outRawFingerprint = fingerprint;
    return 1;
}

static int dm1_v1_f0217_c15_matches_pc34(
    const DM1_ProjectileTerminalMaterializationInputF0217Pc34 *input)
{
    if (!input->termination->shouldCreateExplosion) {
        return !input->c15Publication && !input->c15Material && !input->impactWorld;
    }
    return input->dungeon && input->c15Publication && input->c15Material &&
           input->impactWorld && input->c15Material->valid &&
           !input->c15Material->noDraw && input->c15Material->saveReceiptBound &&
           THING_GET_TYPE(input->c15Material->rawThing) == THING_TYPE_EXPLOSION &&
           input->termination->c15FNV1a != 0u &&
           input->termination->c15FNV1a == input->c15Material->rawRecordFNV1a &&
           input->impactWorld->valid && input->impactWorld->rawC14FNV1a ==
               input->termination->rawC14FNV1a &&
           input->impactWorld->rawC15FNV1a == input->c15Material->rawRecordFNV1a &&
           input->impactWorld->c15Thing == input->c15Publication->slot &&
           dm1_v1_c15_c25_receipt_is_live_pc34(input->c15Publication,
                                                input->dungeon, input->things) &&
           input->c15Publication->c15Fingerprint ==
               input->c15Material->rawRecordFNV1a;
}

int dm1_v1_projectile_terminal_materialization_f0217_pc34(
    const DM1_ProjectileTerminalMaterializationInputF0217Pc34 *input,
    DM1_ProjectileTerminalMaterializationReceiptF0217Pc34 *outReceipt)
{
    DM1_ProjectileTerminalMaterializationReceiptF0217Pc34 receipt;
    uint32_t associatedFingerprint = 0u;

    if (!outReceipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.sourceAnchor =
        "ReDMCSB PROJEXPL.C F0217/F0215; DUNGEON.C F0163/F0166; "
        "DEFS.H C14/C15/C25";
    *outReceipt = receipt;
    if (!input || !input->things || !input->termination || !input->f0215 ||
        !input->c14Material || !dm1_v1_f0217_c14_matches_pc34(
            input->things, input->termination, input->f0215, input->c14Material) ||
        !dm1_v1_f0217_c15_matches_pc34(input)) {
        return 1;
    }
    if (input->damage && (!input->damage->valid ||
                          input->damage->rawC14FNV1a !=
                              input->termination->rawC14FNV1a)) {
        return 1;
    }
    if (input->f0215->shouldMaterialize) {
        if (!dm1_v1_f0217_associated_material_matches_pc34(
                input->things, input->termination, input->f0215,
                input->associatedMaterial, &associatedFingerprint)) {
            return 1;
        }
        receipt.shouldMaterializeAssociatedThing = 1;
        receipt.associatedMaterialFNV1a = input->associatedMaterial->materialFNV1a;
    } else if (input->f0215->shouldConsumePotion ||
               input->termination->shouldMaterializeAssociatedThing ||
               input->associatedMaterial) {
        return 1;
    }
    receipt.valid = 1;
    receipt.shouldDeleteC14 = 1;
    receipt.shouldConsumeAssociatedPotion = input->f0215->shouldConsumePotion;
    receipt.shouldAdvanceC15 = input->termination->shouldCreateExplosion;
    receipt.projectileThing = input->termination->projectileThing;
    receipt.associatedThing = input->termination->associatedThing;
    receipt.c15Thing = input->c15Publication ? input->c15Publication->slot : THING_NONE;
    receipt.rawC14FNV1a = input->termination->rawC14FNV1a;
    receipt.rawAssociatedThingFNV1a = associatedFingerprint;
    receipt.rawC15FNV1a = input->c15Material ? input->c15Material->rawRecordFNV1a : 0u;
    *outReceipt = receipt;
    return 1;
}

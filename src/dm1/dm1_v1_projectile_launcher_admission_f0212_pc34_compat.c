#include "dm1_v1_projectile_launcher_admission_f0212_pc34_compat.h"

#include "dm1_v1_dungeon_thing_data_pc34_compat.h"
#include "dm1_v1_projectile_explosion_render_pc34_compat.h"

#include <string.h>

static int dm1_v1_f0212_raw_thing_size_pc34(int type)
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

static uint32_t dm1_v1_f0212_create_input_fnv1a_pc34(
    const struct ProjectileCreateInput_Compat *input)
{
    return input ? dm1_v1_f0115_source_material_fnv1a_pc34(
                       (const uint8_t *)input, sizeof(*input)) : 0u;
}

int dm1_v1_projectile_launcher_plan_f0212_pc34(
    const DM1_ProjectileLauncherPlanInputF0212Pc34 *input,
    DM1_ProjectileLauncherPlanF0212Pc34 *outPlan)
{
    DM1_ProjectileLauncherPlanF0212Pc34 plan;
    DM1_ProjectileMaterialResolutionPc34 resolution;
    const unsigned char *raw;
    int rawSize;
    uint32_t rawFingerprint;

    if (!outPlan) return 0;
    memset(&plan, 0, sizeof(plan));
    plan.sourceAnchor =
        "ReDMCSB PROJEXPL.C F0212:43-92; DUNGEON.C F0142/F0516; "
        "DUNVIEW.C F0115 G0209";
    *outPlan = plan;
    if (!input || !input->things || !input->things->loaded ||
        !input->createInput || !input->createSource || !input->c14Reservation ||
        !input->associatedMaterial || !input->createSource->valid ||
        !input->c14Reservation->active ||
        input->createInput->associatedThing == THING_NONE ||
        input->createSource->associatedThing !=
            (unsigned short)input->createInput->associatedThing ||
        input->createSource->createInputFNV1a !=
            dm1_v1_f0212_create_input_fnv1a_pc34(input->createInput) ||
        THING_GET_TYPE(input->createInput->associatedThing) !=
            input->associatedThingType ||
        THING_GET_TYPE(input->c14Reservation->thing) != THING_TYPE_PROJECTILE ||
        !input->associatedMaterial->valid || input->associatedMaterial->noDraw ||
        !input->associatedMaterial->usesF0791Blit ||
        input->associatedMaterial->transparentColor != 10 ||
        !input->associatedMaterial->pixels ||
        input->associatedMaterial->pixelCount == 0u ||
        input->associatedMaterial->materialFNV1a == 0u ||
        input->associatedMaterial->materialFNV1a !=
            dm1_v1_f0115_source_material_fnv1a_pc34(
                input->associatedMaterial->pixels,
                input->associatedMaterial->pixelCount)) {
        return 1;
    }
    rawSize = dm1_v1_f0212_raw_thing_size_pc34(input->associatedThingType);
    raw = dm1_v1_dungeon_get_thing_data_pc34(
        input->things, (unsigned short)input->createInput->associatedThing);
    if (!raw || rawSize == 0 ||
        !dm1_v1_projectile_material_resolve_pc34(
            input->createInput->subtype, input->associatedThingType,
            input->associatedThingSubtype, input->weaponProjectileAspectOrdinal,
            &resolution) || !resolution.valid || !resolution.uses_object_aspect ||
        resolution.graphic_index != input->associatedMaterial->graphicIndex) {
        return 1;
    }
    rawFingerprint = dm1_v1_f0115_source_material_fnv1a_pc34(
        raw, (size_t)rawSize);
    if (rawFingerprint == 0u ||
        rawFingerprint != input->createSource->rawThingFNV1a) {
        return 1;
    }
    plan.valid = 1;
    plan.associatedThing = (unsigned short)input->createInput->associatedThing;
    plan.reservedC14Thing = input->c14Reservation->thing;
    plan.expectedGraphicIndex = resolution.graphic_index;
    plan.objectAspectIndex = resolution.aspect_index;
    plan.rawAssociatedThingFNV1a = rawFingerprint;
    plan.createInputFNV1a = input->createSource->createInputFNV1a;
    plan.associatedMaterialFNV1a = input->associatedMaterial->materialFNV1a;
    *outPlan = plan;
    return 1;
}

int dm1_v1_projectile_launcher_runtime_admit_f0212_pc34(
    const DM1_ProjectileLauncherRuntimeInputF0212Pc34 *input,
    DM1_ProjectileLauncherRuntimeReceiptF0212Pc34 *outReceipt)
{
    DM1_ProjectileLauncherRuntimeReceiptF0212Pc34 receipt;
    unsigned short c14Thing;

    if (!outReceipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.sourceAnchor =
        "ReDMCSB PROJEXPL.C F0212:76-92; DUNGEON.C F0516; "
        "DUNVIEW.C F0115 F0142/G0209 catalog consumption";
    *outReceipt = receipt;
    if (!input || !input->plan || !input->c14Reservation ||
        !input->graphicsCatalog || !input->c14Material || !input->plan->valid ||
        !input->c14Reservation->active ||
        THING_GET_TYPE(input->c14Reservation->thing) != THING_TYPE_PROJECTILE) {
        return 1;
    }
    c14Thing = input->c14Reservation->thing;
    if (THING_GET_INDEX(c14Thing) != THING_GET_INDEX(input->plan->reservedC14Thing) ||
        !dm1_v1_c14_c15_graphics_catalog_admit_receipt_pc34(
            input->graphicsCatalog, input->c14Material,
            DM1_V1_F0248_LIVE_EFFECT_PROJECTILE_C14_PC34, c14Thing,
            input->plan->associatedThing, input->plan->expectedGraphicIndex)) {
        return 1;
    }
    receipt.valid = 1;
    receipt.shouldPublishRuntimeProjectile = 1;
    receipt.c14Thing = c14Thing;
    receipt.associatedThing = input->plan->associatedThing;
    receipt.rawC14FNV1a = input->c14Material->rawRecordFNV1a;
    receipt.rawAssociatedThingFNV1a = input->plan->rawAssociatedThingFNV1a;
    receipt.graphicsPixelsFNV1a = input->c14Material->graphicsPixelsFNV1a;
    *outReceipt = receipt;
    return 1;
}

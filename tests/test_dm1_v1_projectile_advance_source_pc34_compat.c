#include "dm1_v1_projectile_advance_source_pc34_compat.h"
#include "dm1_v1_projectile_explosion_render_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int check(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        return 0;
    }
    return 1;
}

static DM1_V1_F0115SourcePixelsPc34 source_surface(unsigned int graphic)
{
    /* Fixture bytes model decoder-owned PC34 pixels; admission still verifies
     * the raw C14/C15 row and original palette receipt. */
    static const unsigned char pixels[32 * 32] = { 1 };
    DM1_V1_F0115SourcePixelsPc34 surface = {
        pixels, sizeof(pixels), graphic, 32, 32, 1, 1
    };
    return surface;
}

int main(void)
{
    struct DungeonThings_Compat things;
    struct DungeonProjectile_Compat projectile;
    struct DungeonExplosion_Compat explosion;
    struct DM1_Event_V1 originalEvent;
    struct TimelineEvent_Compat runtimeEvent;
    struct ProjectileInstance_Compat runtime;
    struct ProjectileInstance_Compat advanced;
    struct ProjectileTickResult_Compat result;
    struct CellContentDigest_Compat digest;
    DM1OriginalSavePC34ProjectileEventPlan plan;
    DM1_V1_F0248LiveEffectMaterialInputPc34 materialInput;
    DM1_V1_F0248LiveEffectMaterialReceiptPc34 c14Material;
    DM1_V1_F0248LiveEffectMaterialReceiptPc34 c15Material;
    DM1_ProjectileMaterializationReceiptPc34 materialization;
    DM1_ProjectileTerminationSourceInputPc34 terminationInput;
    DM1_ProjectileTerminationSourceReceiptPc34 termination;
    DM1_ProjectileAdvanceSourceInputPc34 input;
    DM1_ProjectileAdvanceSourceReceiptPc34 receipt;
    DM1_V1_F0115SourcePixelsPc34 surface;
    static const unsigned char palette[16] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
    };
    unsigned char c14Raw[8] = { 0xfe, 0xff, 0x00, 0x3c, 48, 48, 3, 0 };
    unsigned char c15Raw[4] = { 0xfe, 0xff, 0x00, 48 };
    unsigned short projectileThing = (unsigned short)(THING_TYPE_PROJECTILE << 10);
    unsigned short explosionThing = (unsigned short)(THING_TYPE_EXPLOSION << 10);
    unsigned short motion = (unsigned short)(4u | (5u << 5) | (2u << 12));
    int ok = 1;

    memset(&things, 0, sizeof(things));
    memset(&projectile, 0, sizeof(projectile));
    memset(&explosion, 0, sizeof(explosion));
    things.loaded = 1;
    things.projectileCount = 1;
    things.explosionCount = 1;
    things.thingCounts[THING_TYPE_PROJECTILE] = 1;
    things.thingCounts[THING_TYPE_EXPLOSION] = 1;
    things.rawThingData[THING_TYPE_PROJECTILE] = c14Raw;
    things.rawThingData[THING_TYPE_EXPLOSION] = c15Raw;
    things.projectiles = &projectile;
    things.explosions = &explosion;
    projectile.next = THING_ENDOFLIST;
    projectile.slot = explosionThing;
    projectile.kineticEnergy = 48;
    projectile.attack = 48;
    projectile.eventIndex = 3;
    explosion.next = THING_ENDOFLIST;
    explosion.type = DM1_EXPLOSION_FIREBALL;
    explosion.attack = 48;
    explosion.centered = 0;

    memset(&originalEvent, 0, sizeof(originalEvent));
    originalEvent.type = DM1_EVENT_MOVE_PROJECTILE;
    originalEvent.map_time = 100u;
    originalEvent.b_mapX = (unsigned char)(projectileThing & 0xffu);
    originalEvent.b_mapY = (unsigned char)(projectileThing >> 8);
    originalEvent.c_cell = (unsigned char)(motion & 0xffu);
    originalEvent.c_effect = (unsigned char)(motion >> 8);
    ok &= check(dm1_v1_original_save_pc34_handoff_projectile_event_plan(
                    &originalEvent, 3, &things, &plan),
                "C48/C49 plan binds raw C14");

    surface = source_surface((unsigned int)dm1_v1_projectile_subtype_graphic_index(
        PROJECTILE_SUBTYPE_FIREBALL));
    memset(&materialInput, 0, sizeof(materialInput));
    materialInput.kind = DM1_V1_F0248_LIVE_EFFECT_PROJECTILE_C14_PC34;
    materialInput.things = &things;
    materialInput.rawThing = projectileThing;
    materialInput.associatedThing = explosionThing;
    materialInput.expectedGraphicIndex = (int)surface.graphicIndex;
    materialInput.surface = &surface;
    materialInput.palette = palette;
    materialInput.paletteByteCount = sizeof(palette);
    materialInput.paletteOwnedByPc34GraphicsDat = 1;
    ok &= check(dm1_v1_f0248_live_effect_material_receipt_pc34(
                    &materialInput, &c14Material) && c14Material.valid,
                "C14 original material receipt");
    surface = source_surface((unsigned int)dm1_v1_explosion_pattern_graphic_index(
        DM1_EXPLOSION_FIREBALL, 48));
    materialInput.kind = DM1_V1_F0248_LIVE_EFFECT_EXPLOSION_C15_PC34;
    materialInput.rawThing = explosionThing;
    materialInput.associatedThing = THING_NONE;
    materialInput.explosionType = DM1_EXPLOSION_FIREBALL;
    materialInput.explosionAttack = 48;
    materialInput.explosionCentered = 0;
    materialInput.expectedGraphicIndex = (int)surface.graphicIndex;
    materialInput.surface = &surface;
    ok &= check(dm1_v1_f0248_live_effect_material_receipt_pc34(
                    &materialInput, &c15Material) && c15Material.valid,
                "C15 original material receipt");

    memset(&runtime, 0, sizeof(runtime));
    runtime.slotIndex = plan.projectile_index;
    runtime.projectileCategory = PROJECTILE_CATEGORY_MAGICAL;
    runtime.projectileSubtype = PROJECTILE_SUBTYPE_FIREBALL;
    runtime.mapIndex = plan.map_index;
    runtime.mapX = plan.map_x;
    runtime.mapY = plan.map_y;
    runtime.cell = plan.cell;
    runtime.direction = plan.direction;
    runtime.kineticEnergy = plan.kinetic_energy;
    runtime.attack = plan.attack;
    runtime.stepEnergy = plan.step_energy;
    runtime.reserved1 = plan.associated_thing;
    runtime.reserved3 = 1;
    memset(&runtimeEvent, 0, sizeof(runtimeEvent));
    runtimeEvent.kind = TIMELINE_EVENT_PROJECTILE_MOVE;
    runtimeEvent.fireAtTick = 100u;
    runtimeEvent.mapIndex = plan.map_index;
    runtimeEvent.mapX = plan.map_x;
    runtimeEvent.mapY = plan.map_y;
    runtimeEvent.cell = plan.cell;
    runtimeEvent.aux0 = plan.projectile_index;
    runtimeEvent.aux1 = plan.source_thing;
    runtimeEvent.aux2 = plan.source_event_type;
    memset(&digest, 0, sizeof(digest));
    digest.sourceMapIndex = plan.map_index;
    digest.sourceMapX = plan.map_x;
    digest.sourceMapY = plan.map_y;
    digest.destMapIndex = plan.map_index;
    digest.destMapX = plan.map_x;
    digest.destMapY = plan.map_y;
    digest.sourceSquareType = PROJECTILE_ELEMENT_CORRIDOR;
    digest.destSquareType = PROJECTILE_ELEMENT_CORRIDOR;
    ok &= check(F0811_PROJECTILE_Advance_Compat(&runtime, &digest, 100u, NULL,
                                                 &advanced, &result),
                "F0812/F0814 open route advances");
    memset(&input, 0, sizeof(input));
    input.things = &things;
    input.sourcePlan = &plan;
    input.sourceEvent = &runtimeEvent;
    input.before = &runtime;
    input.after = &advanced;
    input.result = &result;
    input.destination = &digest;
    input.projectileMaterial = &c14Material;
    input.dispatchTick = 100u;
    ok &= check(dm1_v1_projectile_advance_source_receipt_f0812_f0814_pc34(
                    &input, &receipt) && receipt.valid && receipt.shouldReschedule,
                "F0812/F0814 source receipt admits C49 reschedule");

    digest.destSquareType = PROJECTILE_ELEMENT_WALL;
    ok &= check(F0811_PROJECTILE_Advance_Compat(&runtime, &digest, 100u, NULL,
                                                 &advanced, &result),
                "F0813/F0814 wall collision advances");
    ok &= check(result.despawn && result.emittedExplosion,
                "wall collision terminates with C15 explosion");
    ok &= check(dm1_v1_projectile_materialization_receipt_f0215_pc34(
                    &runtime, &result, 0, 0, THING_ENDOFLIST, NULL, 0,
                    &materialization),
                "F0215 collision disposition builds");
    memset(&terminationInput, 0, sizeof(terminationInput));
    terminationInput.things = &things;
    terminationInput.projectileThing = projectileThing;
    terminationInput.timelineIndex = 3;
    terminationInput.timelineEvent = &runtimeEvent;
    terminationInput.runtimeProjectile = &runtime;
    terminationInput.impactResult = &result;
    terminationInput.termination = &materialization;
    terminationInput.projectileMaterial = &c14Material;
    terminationInput.explosionMaterial = &c15Material;
    ok &= check(dm1_v1_projectile_termination_source_receipt_f0213_f0215_pc34(
                    &terminationInput, &termination) && termination.valid,
                "F0213/F0215 termination receipt remains authoritative");
    input.after = &advanced;
    input.result = &result;
    input.destination = &digest;
    input.explosionMaterial = &c15Material;
    input.termination = &termination;
    ok &= check(dm1_v1_projectile_advance_source_receipt_f0812_f0814_pc34(
                    &input, &receipt) && receipt.valid && receipt.shouldTerminate &&
                    receipt.rawC15FNV1a != 0u,
                "F0813/F0814 collision receipt requires C14/C15/C49 identity");
    runtimeEvent.aux2 = DM1_EVENT_MOVE_PROJECTILE_IGNORE_IMPACTS;
    ok &= check(dm1_v1_projectile_advance_source_receipt_f0812_f0814_pc34(
                    &input, &receipt) && !receipt.valid,
                "stale C49 event type fails closed");

    if (!ok) return 1;
    puts("PASS: DM1 F0812/F0813/F0814 PC34 projectile advance receipts");
    return 0;
}

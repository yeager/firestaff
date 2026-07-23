#include "dm1_v1_projectile_damage_receipt_pc34_compat.h"
#include "dm1_v1_creature_ai_behavior_pc34_compat.h"
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

static unsigned int fnv1a(const unsigned char *bytes, size_t count)
{
    unsigned int hash = 2166136261u;
    size_t index;
    for (index = 0; index < count; ++index) {
        hash ^= bytes[index];
        hash *= 16777619u;
    }
    return hash;
}

int main(void)
{
    struct DungeonThings_Compat things;
    struct DungeonProjectile_Compat rawProjectile;
    struct DungeonGroup_Compat group;
    struct ProjectileInstance_Compat projectile;
    struct ProjectileInstance_Compat after;
    struct ProjectileTickResult_Compat impact;
    struct CellContentDigest_Compat digest;
    struct TimelineEvent_Compat event;
    DM1_V1_F0115SourcePixelsPc34 surface;
    DM1_V1_F0248LiveEffectMaterialInputPc34 materialInput;
    DM1_V1_F0248LiveEffectMaterialReceiptPc34 material;
    DM1_ProjectileAdvanceSourceReceiptPc34 advance;
    DM1_ProjectileTerminationSourceReceiptPc34 termination;
    DM1_ProjectileCreatureActionPlanPc34 creaturePlan;
    DM1_ProjectileCreatureActionApplyPlanPc34 creatureApply;
    DM1_ProjectileCreatureImpactAftermathPc34 aftermath;
    DM1_ProjectileChampionImpactPlanPc34 championPlan;
    DM1_ProjectileChampionDamageApplyPlanPc34 championApply;
    struct CombatantChampionSnapshot_Compat defender;
    struct ChampionState_Compat champion;
    struct RngState_Compat rng;
    DM1_ProjectileDamageSourceInputPc34 input;
    DM1_ProjectileDamageSourceReceiptPc34 receipt;
    unsigned char c14Raw[8] = { 0xfe, 0xff, 0x00, 0x14, 36, 24, 0, 0 };
    static const unsigned char pixels[32 * 32] = { 1 };
    static const unsigned char palette[16] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
    };
    unsigned short projectileThing = (unsigned short)(THING_TYPE_PROJECTILE << 10);
    unsigned short weaponThing = (unsigned short)(THING_TYPE_WEAPON << 10);
    int ok = 1;

    memset(&things, 0, sizeof(things));
    memset(&rawProjectile, 0, sizeof(rawProjectile));
    things.loaded = 1;
    things.projectileCount = 1;
    things.thingCounts[THING_TYPE_PROJECTILE] = 1;
    things.rawThingData[THING_TYPE_PROJECTILE] = c14Raw;
    things.projectiles = &rawProjectile;
    rawProjectile.next = THING_ENDOFLIST;
    rawProjectile.slot = weaponThing;
    rawProjectile.kineticEnergy = 36;
    rawProjectile.attack = 24;
    rawProjectile.eventIndex = 0;
    surface.pixels = pixels;
    surface.pixelCount = sizeof(pixels);
    surface.graphicIndex = (unsigned int)dm1_v1_projectile_subtype_graphic_index(
        PROJECTILE_SUBTYPE_KINETIC_ARROW);
    surface.width = 32;
    surface.height = 32;
    surface.sourceOwned = 1;
    surface.verifiedPc34GraphicsDat = 1;
    memset(&materialInput, 0, sizeof(materialInput));
    materialInput.kind = DM1_V1_F0248_LIVE_EFFECT_PROJECTILE_C14_PC34;
    materialInput.things = &things;
    materialInput.rawThing = projectileThing;
    materialInput.associatedThing = weaponThing;
    materialInput.expectedGraphicIndex = (int)surface.graphicIndex;
    materialInput.surface = &surface;
    materialInput.palette = palette;
    materialInput.paletteByteCount = sizeof(palette);
    materialInput.paletteOwnedByPc34GraphicsDat = 1;
    ok &= check(dm1_v1_f0248_live_effect_material_receipt_pc34(
                    &materialInput, &material) && material.valid,
                "raw C14 has original visual material receipt");

    memset(&projectile, 0, sizeof(projectile));
    projectile.slotIndex = 0;
    projectile.projectileCategory = PROJECTILE_CATEGORY_KINETIC;
    projectile.projectileSubtype = PROJECTILE_SUBTYPE_KINETIC_ARROW;
    projectile.ownerIndex = 1;
    projectile.mapX = 5;
    projectile.mapY = 5;
    projectile.cell = 1;
    projectile.direction = 0;
    projectile.kineticEnergy = 36;
    projectile.attack = 24;
    projectile.stepEnergy = 4;
    projectile.reserved1 = weaponThing;
    projectile.reserved3 = 1;
    memset(&event, 0, sizeof(event));
    event.kind = TIMELINE_EVENT_PROJECTILE_MOVE;
    event.fireAtTick = 901u;
    event.mapX = 5;
    event.mapY = 5;
    event.cell = 1;
    event.aux0 = 0;
    memset(&digest, 0, sizeof(digest));
    digest.sourceMapX = 5;
    digest.sourceMapY = 5;
    digest.sourceSquareType = PROJECTILE_ELEMENT_CORRIDOR;
    digest.destMapX = 5;
    digest.destMapY = 4;
    digest.destSquareType = PROJECTILE_ELEMENT_CORRIDOR;
    digest.destHasCreatureGroup = 1;
    digest.destCreatureType = 17;
    digest.destCreatureCellMask = 1 << 2;
    digest.destTeleporterNewDirection = -1;
    ok &= check(F0811_PROJECTILE_Advance_Compat(&projectile, &digest, 901u, NULL,
                                                 &after, &impact),
                "F0217 emits creature impact action");
    memset(&group, 0, sizeof(group));
    group.creatureType = 17;
    group.count = 1;
    group.cells = 2;
    group.health[0] = 100;
    ok &= check(dm1_v1_projectile_creature_action_plan_pc34(
                    &projectile, &impact.outAction, &group, 0, &creaturePlan),
                "F0221 creature damage plan uses impact action");
    ok &= check(dm1_v1_projectile_creature_action_apply_pc34(
                    &creaturePlan, &group, &creatureApply),
                "F0222 applies creature damage through live group owner");
    ok &= check(dm1_v1_projectile_creature_action_aftermath_pc34(
                    &creaturePlan, &projectile, 0, DM1_BEHAVIOR_ATTACK,
                    creatureApply.outcomeCode, 0, &aftermath),
                "F0223 derives post-impact creature aftermath");
    memset(&advance, 0, sizeof(advance));
    advance.valid = 1;
    advance.rawC14FNV1a = material.rawRecordFNV1a;
    advance.runtimeEventFNV1a = fnv1a((const unsigned char *)&event, sizeof(event));
    memset(&termination, 0, sizeof(termination));
    termination.valid = 1;
    termination.shouldTerminate = 1;
    termination.rawC14FNV1a = material.rawRecordFNV1a;
    memset(&input, 0, sizeof(input));
    input.things = &things;
    input.advance = &advance;
    input.termination = &termination;
    input.c48C49Event = &event;
    input.projectile = &projectile;
    input.impact = &impact;
    input.projectileMaterial = &material;
    input.groupIndex = 0;
    input.creaturePlan = &creaturePlan;
    input.creatureApply = &creatureApply;
    input.creatureAftermath = &aftermath;
    input.dispatchTick = 901u;
    if (aftermath.scheduleReaction) {
        static struct TimelineEvent_Compat reaction;
        memset(&reaction, 0, sizeof(reaction));
        reaction.kind = TIMELINE_EVENT_CREATURE_REACTION;
        reaction.fireAtTick = 902u;
        reaction.mapX = impact.outAction.targetMapX;
        reaction.mapY = impact.outAction.targetMapY;
        reaction.aux0 = 0;
        input.postImpactEvent = &reaction;
    }
    ok &= check(dm1_v1_projectile_damage_source_receipt_f0221_f0226_pc34(
                    &input, &receipt) && receipt.valid &&
                    receipt.shouldApplyCreatureDamage,
                "F0221-F0226 binds raw C14, C49 and creature mutation");
    c14Raw[4] ^= 1u;
    ok &= check(dm1_v1_projectile_damage_source_receipt_f0221_f0226_pc34(
                    &input, &receipt) && !receipt.valid,
                "drifted C14 blocks post-impact mutation receipt");
    c14Raw[4] ^= 1u;

    digest.destHasCreatureGroup = 0;
    digest.destHasChampion = 1;
    digest.destChampionCellMask = 1 << 2;
    ok &= check(F0811_PROJECTILE_Advance_Compat(&projectile, &digest, 901u, NULL,
                                                 &after, &impact),
                "F0217 emits champion impact action");
    ok &= check(dm1_v1_projectile_champion_impact_plan_pc34(
                    &projectile, &impact, 1, &championPlan),
                "F0224 champion damage plan uses impact action");
    memset(&defender, 0, sizeof(defender));
    memset(&champion, 0, sizeof(champion));
    memset(&rng, 0, sizeof(rng));
    defender.currentHealth = 100;
    defender.statisticVitality = 64;
    champion.present = 1;
    champion.hp.current = 100;
    rng.seed = 1u;
    ok &= check(dm1_v1_projectile_champion_damage_apply_pc34(
                    &championPlan, &defender, &rng, &champion, &championApply),
                "F0225 applies champion damage through live champion owner");
    input.creaturePlan = NULL;
    input.creatureApply = NULL;
    input.creatureAftermath = NULL;
    input.championPlan = &championPlan;
    input.championApply = &championApply;
    input.postImpactEvent = NULL;
    ok &= check(dm1_v1_projectile_damage_source_receipt_f0221_f0226_pc34(
                    &input, &receipt) && receipt.valid &&
                    receipt.shouldApplyChampionDamage,
                "F0224-F0226 binds champion impact mutation");

    if (!ok) return 1;
    puts("PASS: DM1 F0221-F0226 PC34 damage and post-impact receipts");
    return 0;
}

#include "dm1_v1_throw_shoot_pc34_compat.h"

#include "dm1_v1_combat_pc34_compat.h"
#include "dm1_v1_creature_ai_behavior_pc34_compat.h"
#include "memory_projectile_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_failures = 0;

#define ASSERT_EQ(actual, expected, label) do { \
    int _a = (actual); \
    int _e = (expected); \
    if (_a != _e) { \
        fprintf(stderr, "FAIL %s: expected %d got %d\n", label, _e, _a); \
        ++g_failures; \
    } \
} while (0)

static void test_throw_weight_and_stamina(void) {
    ASSERT_EQ(dm1_v1_throwing_stamina_cost_from_weight_pc34(0), 1,
              "zero-weight stamina clamp");
    ASSERT_EQ(dm1_v1_throwing_stamina_cost_from_weight_pc34(20), 10,
              "ten-weight stamina");
    ASSERT_EQ(dm1_v1_throwing_stamina_cost_from_weight_pc34(61), 25,
              "heavy-weight stamina tail");
    ASSERT_EQ(dm1_v1_throw_armour_weight_f0140_pc34(0), 3,
              "armour weight type 0");
    ASSERT_EQ(dm1_v1_throw_armour_weight_f0140_pc34(39), 120,
              "armour weight type 39");
    ASSERT_EQ(dm1_v1_throw_armour_weight_f0140_pc34(58), -1,
              "armour weight invalid");
    ASSERT_EQ(dm1_v1_throw_junk_base_weight_f0140_pc34(1), 3,
              "waterskin base weight");
    ASSERT_EQ(dm1_v1_throw_junk_weight_f0140_pc34(1, 4), 11,
              "waterskin charged weight");
    ASSERT_EQ(dm1_v1_throw_junk_weight_f0140_pc34(53, 0), -1,
              "junk weight invalid");
}

static void test_throw_runtime_math(void) {
    ASSERT_EQ(dm1_v1_throw_xp_for_object_pc34(0, 0, 0, 0), 8,
              "non-weapon throw xp");
    ASSERT_EQ(dm1_v1_throw_xp_for_object_pc34(1, 0, 0, 0), 12,
              "unknown weapon throw xp");
    ASSERT_EQ(dm1_v1_throw_xp_for_object_pc34(1, 1, 10, 20), 17,
              "kinetic weapon throw xp");
    ASSERT_EQ(dm1_v1_throw_xp_for_object_pc34(1, 1, 13, 20), 12,
              "non-kinetic-class throw xp");
    ASSERT_EQ(dm1_v1_throw_side_pc34(1, 0), 1, "throw side next");
    ASSERT_EQ(dm1_v1_throw_side_pc34(2, 0), 1, "throw side opposite");
    ASSERT_EQ(dm1_v1_throw_side_pc34(0, 0), 0, "throw side front");
    ASSERT_EQ(dm1_v1_throw_step_energy_pc34(0), 11, "throw step low skill");
    ASSERT_EQ(dm1_v1_throw_step_energy_pc34(6), 5, "throw step clamp");
    ASSERT_EQ(dm1_v1_throw_kinetic_energy_pc34(20, 3, 1, 10, 8, 5), 50,
              "throw kinetic source formula");
    ASSERT_EQ(dm1_v1_throw_kinetic_energy_pc34(20, 3, 1, 13, 8, 5), 39,
              "throw kinetic non-throwable class");
    ASSERT_EQ(dm1_v1_throw_attack_pc34(0, 0), 40,
              "throw attack lower clamp");
    ASSERT_EQ(dm1_v1_throw_attack_pc34(25, 31), 200,
              "throw attack upper clamp");
}

static void test_projectile_shapes_and_launch(void) {
    int subtype = -1;
    ASSERT_EQ(dm1_v1_thrown_potion_projectile_subtype_pc34(3, &subtype), 1,
              "ven potion maps");
    ASSERT_EQ(subtype, PROJECTILE_SUBTYPE_POISON_CLOUD,
              "ven potion subtype");
    ASSERT_EQ(dm1_v1_thrown_potion_projectile_subtype_pc34(19, &subtype), 1,
              "ful bomb maps");
    ASSERT_EQ(subtype, PROJECTILE_SUBTYPE_FIREBALL,
              "ful bomb subtype");
    ASSERT_EQ(dm1_v1_thrown_potion_projectile_subtype_pc34(2, &subtype), 0,
              "ordinary potion not projectile spell");
    ASSERT_EQ(subtype, PROJECTILE_SUBTYPE_KINETIC_ARROW,
              "ordinary potion keeps kinetic subtype");
    ASSERT_EQ(dm1_v1_projectile_launch_cell_pc34(0, 0), 0,
              "launch cell front north");
    ASSERT_EQ(dm1_v1_projectile_launch_cell_pc34(1, 0), 1,
              "launch cell side north");
    ASSERT_EQ(dm1_v1_projectile_launch_cell_pc34(3, 1), 2,
              "launch cell rotated east");
}

static void test_shoot_runtime_math(void) {
    int step = -1;
    ASSERT_EQ(dm1_v1_shoot_step_energy_pc34(16, &step), 1,
              "bow class step valid");
    ASSERT_EQ(step, 0, "bow class first step");
    ASSERT_EQ(dm1_v1_shoot_step_energy_pc34(31, &step), 1,
              "bow class last valid");
    ASSERT_EQ(step, 15, "bow class last step");
    ASSERT_EQ(dm1_v1_shoot_step_energy_pc34(32, &step), 1,
              "sling class first valid");
    ASSERT_EQ(step, 0, "sling class first step");
    ASSERT_EQ(dm1_v1_shoot_step_energy_pc34(15, &step), 0,
              "shoot class invalid");
    ASSERT_EQ(dm1_v1_shoot_ammunition_matches_pc34(16, 10), 1,
              "bow ammo matches");
    ASSERT_EQ(dm1_v1_shoot_ammunition_matches_pc34(16, 11), 0,
              "bow ammo rejects sling ammo");
    ASSERT_EQ(dm1_v1_shoot_ammunition_matches_pc34(32, 11), 1,
              "sling ammo matches");
    ASSERT_EQ(dm1_v1_shoot_attack_pc34(50, 3), 106,
              "shoot attack formula");
    ASSERT_EQ(dm1_v1_shoot_attack_pc34(200, 80), 255,
              "shoot attack clamp");
    ASSERT_EQ(dm1_v1_legacy_throw_attack_probe_pc34(50, 3), 106,
              "legacy throw probe formula");
}

static void test_projectile_create_input_model(void) {
    DM1_ProjectileCreateRequestPc34 req;
    struct ProjectileCreateInput_Compat input;
    unsigned short potionThing =
        (unsigned short)((THING_TYPE_POTION << 10) | 5);
    memset(&req, 0, sizeof(req));
    memset(&input, 0, sizeof(input));
    req.championIndex = 2;
    req.championCell = 1;
    req.partyMapIndex = 3;
    req.partyMapX = 10;
    req.partyMapY = 11;
    req.partyDirection = 0;
    req.gameTick = 1234;
    req.subtype = PROJECTILE_SUBTYPE_POISON_CLOUD;
    req.category = PROJECTILE_CATEGORY_KINETIC;
    req.kineticEnergy = 50;
    req.impactAttack = 60;
    req.attackTypeCode = DM1_ATTACK_NORMAL;
    req.launchCell = -1;
    req.launchDirection = -1;
    req.stepEnergy = 0;
    req.launcherStrength = 77;
    req.carriedThing = potionThing;
    req.potionPower = 22;
    ASSERT_EQ(dm1_v1_build_projectile_create_input_pc34(&req, &input), 1,
              "projectile create input builds");
    ASSERT_EQ(input.ownerKind, PROJECTILE_OWNER_CHAMPION,
              "projectile owner kind");
    ASSERT_EQ(input.ownerIndex, 2, "projectile owner index");
    ASSERT_EQ(input.mapIndex, 3, "projectile map");
    ASSERT_EQ(input.mapX, 10, "projectile x");
    ASSERT_EQ(input.mapY, 11, "projectile y");
    ASSERT_EQ(input.cell, dm1_v1_projectile_launch_cell_pc34(1, 0),
              "projectile default launch cell");
    ASSERT_EQ(input.direction, 0, "projectile default direction");
    ASSERT_EQ(input.stepEnergy, 1, "projectile step clamp");
    ASSERT_EQ(input.poisonAttack, 22, "thrown poison potion power");
    ASSERT_EQ(input.associatedThing, potionThing, "associated thing");
    ASSERT_EQ(input.firstMoveGraceFlag, 1, "first move grace");

    req.carriedThing = THING_NONE;
    req.subtype = PROJECTILE_SUBTYPE_POISON_BOLT;
    req.potionPower = 99;
    req.impactAttack = 44;
    ASSERT_EQ(dm1_v1_build_projectile_create_input_pc34(&req, &input), 1,
              "poison bolt input builds");
    ASSERT_EQ(input.poisonAttack, 44, "poison bolt uses impact attack");
    ASSERT_EQ(input.associatedThing, THING_NONE, "empty associated thing");
}

static void test_projectile_impact_model(void) {
    struct ProjectileInstance_Compat p;
    struct ProjectileTickResult_Compat r;
    memset(&p, 0, sizeof(p));
    memset(&r, 0, sizeof(r));
    ASSERT_EQ(dm1_v1_projectile_subtype_name_pc34(PROJECTILE_SUBTYPE_FIREBALL)[0],
              'F', "fireball name");
    ASSERT_EQ(dm1_v1_projectile_subtype_name_pc34(12345)[0],
              'P', "unknown projectile name");

    p.projectileCategory = PROJECTILE_CATEGORY_KINETIC;
    p.projectileSubtype = PROJECTILE_SUBTYPE_KINETIC_ARROW;
    r.resultKind = PROJECTILE_RESULT_FLEW;
    ASSERT_EQ(dm1_v1_projectile_impact_source_sound_index_pc34(&p, &r), -1,
              "no impact sound while flying");
    r.resultKind = PROJECTILE_RESULT_HIT_WALL;
    ASSERT_EQ(dm1_v1_projectile_impact_source_sound_index_pc34(&p, &r), 0,
              "kinetic missile impact sound");
    p.projectileCategory = PROJECTILE_CATEGORY_MAGICAL;
    p.projectileSubtype = PROJECTILE_SUBTYPE_POISON_BOLT;
    p.kineticEnergy = 20;
    ASSERT_EQ(dm1_v1_projectile_impact_source_sound_index_pc34(&p, &r), 16,
              "poison bolt impact sound");
    p.kineticEnergy = 3;
    ASSERT_EQ(dm1_v1_projectile_impact_source_sound_index_pc34(&p, &r), -1,
              "spent poison bolt silent");
    p.projectileSubtype = PROJECTILE_SUBTYPE_LIGHTNING_BOLT;
    p.kineticEnergy = 1;
    ASSERT_EQ(dm1_v1_projectile_impact_source_sound_index_pc34(&p, &r), -1,
              "spent lightning silent");
    r.emittedExplosion = 1;
    r.outExplosion.explosionType = C000_EXPLOSION_FIREBALL;
    r.outExplosion.attack = 81;
    ASSERT_EQ(dm1_v1_projectile_impact_source_sound_index_pc34(&p, &r), 5,
              "large explosion sound");
    r.outExplosion.attack = 80;
    ASSERT_EQ(dm1_v1_projectile_impact_source_sound_index_pc34(&p, &r), 6,
              "small explosion sound");
    r.outExplosion.explosionType = C040_EXPLOSION_SMOKE;
    ASSERT_EQ(dm1_v1_projectile_impact_source_sound_index_pc34(&p, &r), -1,
              "smoke explosion silent");
    ASSERT_EQ(dm1_v1_thrown_sharp_weapon_type_kept_by_creature_pc34(8), 1,
              "dagger kept");
    ASSERT_EQ(dm1_v1_thrown_sharp_weapon_type_kept_by_creature_pc34(32), 1,
              "throwing star kept");
    ASSERT_EQ(dm1_v1_thrown_sharp_weapon_type_kept_by_creature_pc34(9), 0,
              "ordinary weapon not kept");
}

static void test_projectile_associated_thing_disposition(void) {
    struct ProjectileInstance_Compat p;
    struct ProjectileTickResult_Compat r;
    DM1_ProjectileAssociatedThingDispositionPc34 d;
    unsigned short potionThing =
        (unsigned short)((THING_TYPE_POTION << 10) | 2);
    unsigned short weaponThing =
        (unsigned short)((THING_TYPE_WEAPON << 10) | 7);
    memset(&p, 0, sizeof(p));
    memset(&r, 0, sizeof(r));
    p.reserved1 = potionThing;
    p.flags = PROJECTILE_FLAG_REMOVE_POTION_ON_IMPACT;
    p.cell = 3;
    r.despawn = 1;
    ASSERT_EQ(dm1_v1_projectile_associated_thing_disposition_pc34(
                  &p, &r, 0, 4, &d), 1,
              "potion disposition builds");
    ASSERT_EQ(d.shouldConsumePotion, 1, "potion consumed");
    ASSERT_EQ(d.shouldMaterialize, 0, "consumed potion not materialized");
    ASSERT_EQ(d.associatedThing, potionThing, "consumed associated thing");

    p.reserved1 = weaponThing;
    p.flags = 0;
    p.cell = 2;
    ASSERT_EQ(dm1_v1_projectile_associated_thing_disposition_pc34(
                  &p, NULL, 0, 0, &d), 1,
              "weapon disposition builds");
    ASSERT_EQ(d.shouldConsumePotion, 0, "weapon not consumed");
    ASSERT_EQ(d.shouldMaterialize, 1, "weapon materialized");
    ASSERT_EQ(d.droppedThing,
              (unsigned short)(weaponThing | (unsigned short)(2u << 14)),
              "weapon dropped with projectile cell");

    ASSERT_EQ(dm1_v1_projectile_associated_thing_disposition_pc34(
                  &p, NULL, 1, 0, &d), 1,
              "moved-to-group disposition builds");
    ASSERT_EQ(d.shouldMaterialize, 0, "moved-to-group not materialized");

    p.reserved1 = THING_NONE;
    ASSERT_EQ(dm1_v1_projectile_associated_thing_disposition_pc34(
                  &p, NULL, 0, 0, &d), 1,
              "empty disposition builds");
    ASSERT_EQ(d.shouldMaterialize, 0, "empty not materialized");
}

static void test_black_flame_heal_and_group_cell(void) {
    struct DungeonGroup_Compat group;
    struct ProjectileInstance_Compat p;
    struct ProjectileTickResult_Compat r;
    int slot = -1;
    int newHealth = 0;
    memset(&group, 0, sizeof(group));
    memset(&p, 0, sizeof(p));
    memset(&r, 0, sizeof(r));

    group.count = 0;
    group.cells = DM1_PROJECTILE_SINGLE_CENTERED_CREATURE_CELL_PC34;
    group.health[0] = 900;
    ASSERT_EQ(dm1_v1_group_creature_index_for_cell_pc34(&group, 2), 0,
              "single centered group slot");

    group.count = 2;
    group.cells = (0 << 0) | (2 << 2) | (3 << 4);
    group.health[0] = 10;
    group.health[1] = 0;
    group.health[2] = 30;
    ASSERT_EQ(dm1_v1_group_creature_index_for_cell_pc34(&group, 2), -1,
              "dead target cell ignored");
    ASSERT_EQ(dm1_v1_group_creature_index_for_cell_pc34(&group, 3), 2,
              "packed group live target cell");

    group.creatureType = DM1_PROJECTILE_BLACK_FLAME_CREATURE_PC34;
    group.count = 0;
    group.cells = DM1_PROJECTILE_SINGLE_CENTERED_CREATURE_CELL_PC34;
    group.health[0] = 990;
    p.projectileSubtype = PROJECTILE_SUBTYPE_FIREBALL;
    r.resultKind = PROJECTILE_RESULT_HIT_CREATURE;
    r.emittedCombatAction = 1;
    r.outAction.targetCell = 1;
    r.outAction.rawAttackValue = 25;
    ASSERT_EQ(dm1_v1_black_flame_fireball_heal_pc34(
                  &p, &r, &group, &slot, &newHealth), 1,
              "black flame fireball heals");
    ASSERT_EQ(slot, 0, "black flame heal slot");
    ASSERT_EQ(newHealth, DM1_PROJECTILE_BLACK_FLAME_MAX_HEALTH_PC34,
              "black flame heal cap");

    p.projectileSubtype = PROJECTILE_SUBTYPE_LIGHTNING_BOLT;
    ASSERT_EQ(dm1_v1_black_flame_fireball_heal_pc34(
                  &p, &r, &group, &slot, &newHealth), 0,
              "non-fireball does not heal black flame");
}

static void test_projectile_creature_impact_plan(void) {
    struct DungeonGroup_Compat group;
    struct ProjectileInstance_Compat p;
    struct ProjectileTickResult_Compat r;
    DM1_ProjectileCreatureImpactPlanPc34 plan;
    DM1_ProjectileCreatureImpactAftermathPc34 aftermath;
    unsigned short weaponThing =
        (unsigned short)((THING_TYPE_WEAPON << 10) | 1);
    memset(&group, 0, sizeof(group));
    memset(&p, 0, sizeof(p));
    memset(&r, 0, sizeof(r));
    memset(&plan, 0, sizeof(plan));
    memset(&aftermath, 0, sizeof(aftermath));

    group.creatureType = 6;
    group.count = 2;
    group.cells = (1 << 0) | (3 << 2) | (0 << 4);
    group.health[0] = 10;
    group.health[1] = 20;
    group.health[2] = 30;
    p.projectileCategory = PROJECTILE_CATEGORY_KINETIC;
    p.projectileSubtype = PROJECTILE_SUBTYPE_KINETIC_ARROW;
    p.reserved1 = weaponThing;
    r.resultKind = PROJECTILE_RESULT_HIT_CREATURE;
    r.emittedCombatAction = 1;
    r.outAction.targetCell = 3;
    r.outAction.rawAttackValue = 17;

    ASSERT_EQ(dm1_v1_projectile_creature_impact_plan_pc34(
                  &p, &r, &group, 0, &plan), 1,
              "creature impact plan builds");
    ASSERT_EQ(plan.handled, 1, "creature impact handled");
    ASSERT_EQ(plan.shouldApplyDamage, 1, "creature impact applies damage");
    ASSERT_EQ(plan.slotIndex, 1, "creature impact target slot");
    ASSERT_EQ(plan.damageApplied, 17, "creature impact damage");
    ASSERT_EQ(plan.originalCreatureType, 6, "creature impact type snapshot");
    ASSERT_EQ(plan.originalGroupCount, 2, "creature impact count snapshot");
    ASSERT_EQ(plan.killedCell, 3, "creature impact killed cell");

    ASSERT_EQ(dm1_v1_projectile_creature_impact_aftermath_pc34(
                  &plan, &p,
                  DM1_PROJECTILE_ATTR_KEEP_THROWN_SHARP_WEAPONS_PC34,
                  DM1_BEHAVIOR_ATTACK,
                  COMBAT_OUTCOME_KILLED_NO_CREATURES,
                  27,
                  &aftermath), 1,
              "creature impact aftermath builds");
    ASSERT_EQ(aftermath.scheduleReaction, 1, "surviving group reacts");
    ASSERT_EQ(aftermath.keepSharpWeaponInGroup, 1,
              "sharp weapon kept by creature");
    ASSERT_EQ(aftermath.spawnDeathSmoke, 0, "no death smoke without kill");

    ASSERT_EQ(dm1_v1_projectile_creature_impact_aftermath_pc34(
                  &plan, &p,
                  DM1_PROJECTILE_ATTR_DROP_FIXED_POSSESSION_PC34,
                  DM1_BEHAVIOR_ATTACK,
                  COMBAT_OUTCOME_KILLED_SOME_CREATURES,
                  27,
                  &aftermath), 1,
              "partial kill aftermath builds");
    ASSERT_EQ(aftermath.cleanupEventsAndFear, 1, "partial kill cleans attack events");
    ASSERT_EQ(aftermath.dropFixedPossessions, 1, "partial kill drops fixed possessions");
    ASSERT_EQ(aftermath.spawnDeathSmoke, 1, "partial kill smoke");
    ASSERT_EQ(aftermath.keepSharpWeaponInGroup, 0,
              "killing hit does not keep sharp weapon");

    ASSERT_EQ(dm1_v1_projectile_creature_impact_plan_pc34(
                  &p, &r, &group,
                  DM1_PROJECTILE_ATTR_NON_MATERIAL_PC34, &plan), 1,
              "non-material plan builds");
    ASSERT_EQ(plan.blockedByNonMaterial, 1, "non-material blocks ordinary projectile");
    ASSERT_EQ(plan.shouldApplyDamage, 0, "non-material no ordinary damage");

    p.projectileSubtype = PROJECTILE_SUBTYPE_HARM_NON_MATERIAL;
    ASSERT_EQ(dm1_v1_projectile_creature_impact_plan_pc34(
                  &p, &r, &group,
                  DM1_PROJECTILE_ATTR_NON_MATERIAL_PC34, &plan), 1,
              "harm non-material plan builds");
    ASSERT_EQ(plan.blockedByNonMaterial, 0, "harm non-material bypasses block");
    ASSERT_EQ(plan.shouldApplyDamage, 1, "harm non-material damages");
}

static void test_projectile_champion_impact_plan(void) {
    struct ProjectileInstance_Compat p;
    struct ProjectileTickResult_Compat r;
    DM1_ProjectileChampionImpactPlanPc34 impact;
    DM1_ProjectileChampionPoisonPlanPc34 poison;
    memset(&p, 0, sizeof(p));
    memset(&r, 0, sizeof(r));
    memset(&impact, 0, sizeof(impact));
    memset(&poison, 0, sizeof(poison));

    p.projectileSubtype = PROJECTILE_SUBTYPE_POISON_BOLT;
    p.poisonAttack = 130;
    r.resultKind = PROJECTILE_RESULT_HIT_CHAMPION;
    r.emittedCombatAction = 1;
    r.newMapIndex = 2;
    r.newMapX = 7;
    r.newMapY = 8;
    r.newCell = 3;
    r.outAction.defenderSlotOrCreatureIndex = 1;
    r.outAction.attackTypeCode = COMBAT_ATTACK_MAGIC;
    r.outAction.rawAttackValue = 55;
    r.outAction.allowedWounds = COMBAT_WOUND_HEAD | COMBAT_WOUND_TORSO;

    ASSERT_EQ(dm1_v1_projectile_champion_impact_plan_pc34(
                  &p, &r, 1, &impact), 1,
              "champion impact plan builds");
    ASSERT_EQ(impact.handled, 1, "champion impact handled");
    ASSERT_EQ(impact.championPresent, 1, "champion present");
    ASSERT_EQ(impact.championIndex, 1, "champion index");
    ASSERT_EQ(impact.impactMapIndex, 2, "champion impact map");
    ASSERT_EQ(impact.impactMapX, 7, "champion impact x");
    ASSERT_EQ(impact.impactMapY, 8, "champion impact y");
    ASSERT_EQ(impact.impactCell, 3, "champion impact cell");
    ASSERT_EQ(impact.attackTypeCode, COMBAT_ATTACK_MAGIC,
              "champion attack type");
    ASSERT_EQ(impact.rawAttackValue, 55, "champion raw attack");
    ASSERT_EQ(impact.allowedWounds,
              COMBAT_WOUND_HEAD | COMBAT_WOUND_TORSO,
              "champion allowed wounds");

    ASSERT_EQ(dm1_v1_projectile_champion_poison_plan_pc34(
                  &impact, &p, 12, 30, 65000, 1, &poison), 1,
              "champion poison plan builds");
    ASSERT_EQ(poison.shouldApply, 1, "champion poison applies");
    ASSERT_EQ(poison.championIndex, 1, "champion poison index");
    ASSERT_EQ(poison.poisonDamage, 2, "champion poison damage");
    ASSERT_EQ(poison.newPoisonDose, 65130, "champion poison dose");
    ASSERT_EQ(poison.nextAttack, 129, "champion poison next attack");
    ASSERT_EQ(poison.scheduleDelayTicks, 36, "champion poison delay");

    ASSERT_EQ(dm1_v1_projectile_champion_poison_plan_pc34(
                  &impact, &p, 12, 1, 65530, 1, &poison), 1,
              "champion poison cap plan builds");
    ASSERT_EQ(poison.poisonDamage, 1, "champion poison damage caps to hp");
    ASSERT_EQ(poison.newPoisonDose, 65535, "champion poison dose caps");

    ASSERT_EQ(dm1_v1_projectile_champion_poison_plan_pc34(
                  &impact, &p, 12, 30, 0, 0, &poison), 1,
              "champion poison random fail builds");
    ASSERT_EQ(poison.shouldApply, 0, "champion poison random gate blocks");

    ASSERT_EQ(dm1_v1_projectile_champion_party_death_check_pc34(0, 1), 0,
              "living champion no party death check");
    ASSERT_EQ(dm1_v1_projectile_champion_party_death_check_pc34(0, 0), 1,
              "zero hp party death check");
    ASSERT_EQ(dm1_v1_projectile_champion_party_death_check_pc34(1, 9), 1,
              "killed flag party death check");
}

int main(void) {
    test_throw_weight_and_stamina();
    test_throw_runtime_math();
    test_projectile_shapes_and_launch();
    test_shoot_runtime_math();
    test_projectile_create_input_model();
    test_projectile_impact_model();
    test_projectile_associated_thing_disposition();
    test_black_flame_heal_and_group_cell();
    test_projectile_creature_impact_plan();
    test_projectile_champion_impact_plan();
    if (g_failures) {
        fprintf(stderr, "test_dm1_v1_throw_shoot_pc34_compat: %d failures\n",
                g_failures);
        return 1;
    }
    printf("test_dm1_v1_throw_shoot_pc34_compat: PASS\n");
    return 0;
}

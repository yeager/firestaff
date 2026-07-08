#include "dm1_v1_throw_shoot_pc34_compat.h"

#include "dm1_v1_combat_pc34_compat.h"
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

int main(void) {
    test_throw_weight_and_stamina();
    test_throw_runtime_math();
    test_projectile_shapes_and_launch();
    test_shoot_runtime_math();
    test_projectile_create_input_model();
    if (g_failures) {
        fprintf(stderr, "test_dm1_v1_throw_shoot_pc34_compat: %d failures\n",
                g_failures);
        return 1;
    }
    printf("test_dm1_v1_throw_shoot_pc34_compat: PASS\n");
    return 0;
}

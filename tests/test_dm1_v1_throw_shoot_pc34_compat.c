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
    DM1_ThrowF0328ProjectileInputPc34 throwIn;
    DM1_ThrowF0328ProjectilePlanPc34 throwOut;
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

    memset(&throwIn, 0, sizeof(throwIn));
    throwIn.objectWeight = 20;
    throwIn.championStrength = 30;
    throwIn.championMaxLoad = 340;
    throwIn.championCurrentStamina = 100;
    throwIn.championMaximumStamina = 100;
    throwIn.isWeapon = 1;
    throwIn.hasWeaponInfo = 1;
    throwIn.weaponClass = 10;
    throwIn.weaponStrength = 7;
    throwIn.weaponKineticEnergy = 20;
    throwIn.f0312SkillBonus = 2;
    throwIn.throwSkillLevel = 3;
    throwIn.rngStrength16 = 5;
    throwIn.rngKinetic16 = 5;
    throwIn.rngAttack32 = 31;
    throwIn.thrownThing = (unsigned short)((THING_TYPE_WEAPON << 10) | 7);
    throwIn.thingType = THING_TYPE_WEAPON;
    throwIn.partyDirection = 2;
    throwIn.throwSide = 1;
    ASSERT_EQ(dm1_v1_throw_projectile_plan_f0328_pc34(
                  &throwIn, &throwOut), 1,
              "F0328 throw projectile plan builds");
    ASSERT_EQ(throwOut.valid, 1, "F0328 plan valid");
    ASSERT_EQ(throwOut.staminaCost, 10, "F0328 stamina cost");
    ASSERT_EQ(throwOut.throwExperience, 17, "F0328 throw xp");
    ASSERT_EQ(throwOut.throwStrength, 27, "F0312 throw strength");
    ASSERT_EQ(throwOut.kineticEnergy, 78, "F0328 kinetic energy");
    ASSERT_EQ(throwOut.attack, 55, "F0328 attack");
    ASSERT_EQ(throwOut.stepEnergy, 8, "F0328 step energy");
    ASSERT_EQ(throwOut.actionDisableTicks, 4, "F0328 disable ticks");
    ASSERT_EQ(throwOut.combatSoundIndex, 13, "F0328 combat sound");
    ASSERT_EQ(throwOut.projectileSubtype, PROJECTILE_SUBTYPE_KINETIC_ARROW,
              "F0328 weapon projectile subtype");
    ASSERT_EQ(throwOut.launchDirection, 3, "F0328 launch side direction");
    ASSERT_EQ(throwOut.projectileDisabledMovementTicks, 4,
              "F0328 projectile movement disable");
    ASSERT_EQ(throwOut.lastProjectileDisabledMovementDirection, 2,
              "F0328 last projectile direction");

    throwIn.championCurrentStamina = 20;
    throwIn.actionHandWounded = 1;
    ASSERT_EQ(dm1_v1_throw_projectile_plan_f0328_pc34(
                  &throwIn, &throwOut), 1,
              "F0328 wounded low-stamina plan builds");
    ASSERT_EQ(throwOut.throwStrength, 9,
              "F0312 low stamina and wound reduce strength");
    ASSERT_EQ(throwOut.kineticEnergy, 51,
              "F0328 kinetic uses reduced strength");

    throwIn.actionHandWounded = 0;
    throwIn.championCurrentStamina = 100;
    throwIn.isWeapon = 0;
    throwIn.hasWeaponInfo = 0;
    throwIn.thingType = THING_TYPE_POTION;
    throwIn.potionType = DM1_POTION_FUL_BOMB_PC34;
    throwIn.potionPower = 44;
    ASSERT_EQ(dm1_v1_throw_projectile_plan_f0328_pc34(
                  &throwIn, &throwOut), 1,
              "F0328 potion throw projectile plan builds");
    ASSERT_EQ(throwOut.throwExperience, 8, "F0328 potion throw xp");
    ASSERT_EQ(throwOut.projectileSubtype, PROJECTILE_SUBTYPE_FIREBALL,
              "F0328 ful bomb projectile subtype");
    ASSERT_EQ(throwOut.projectilePotionPower, 44,
              "F0328 potion power transported");
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

static void test_spell_projectile_f0412_to_f0327_create_input(void) {
    DM1_SpellF0412RuntimeReceipt receipt;
    DM1_SpellF0327ProjectileContextPc34 context;
    DM1_SpellF0327ProjectileLaunchPlanPc34 plan;
    struct ProjectileCreateInput_Compat input;

    memset(&receipt, 0, sizeof(receipt));
    memset(&context, 0, sizeof(context));
    receipt.castResult = DM1_SPELL_CAST_SUCCESS;
    receipt.createsProjectile = 1;
    receipt.projectileThing = DM1_PROJECTILE_THING_FIREBALL;
    receipt.projectileKineticEnergy = 21;
    receipt.projectileStepEnergy = 10;
    receipt.championDirectionAfter = 2;
    context.championIndex = 1;
    context.championCell = 1;
    context.partyMapIndex = 0;
    context.partyMapX = 4;
    context.partyMapY = 5;
    context.gameTick = 1234;

    ASSERT_EQ(dm1_v1_spell_projectile_launch_plan_f0327_pc34(
                  &receipt, &context, &plan), 1,
              "F0412 projectile launch plan builds");
    ASSERT_EQ(plan.valid, 1, "F0412 projectile plan valid");
    ASSERT_EQ(plan.shouldCreateProjectile, 1, "F0412 projectile should create");
    ASSERT_EQ(plan.projectileSubtype, PROJECTILE_SUBTYPE_FIREBALL,
              "F0412 fireball subtype");
    ASSERT_EQ(plan.attackTypeCode, COMBAT_ATTACK_FIRE,
              "F0412 fireball attack type");
    ASSERT_EQ(plan.kineticEnergyBeforeF0327, 21,
              "F0412 kinetic before F0327");
    ASSERT_EQ(plan.kineticEnergyAfterF0327, 24,
              "F0327 weak spell projectile kinetic adjustment");
    ASSERT_EQ(plan.stepEnergyBeforeF0327, 10,
              "F0412 step before F0327");
    ASSERT_EQ(plan.stepEnergyAfterF0327, 9,
              "F0327 weak spell projectile step adjustment");
    ASSERT_EQ(plan.attack, 90, "F0327 spell projectile attack");
    ASSERT_EQ(plan.launchDirection, 2, "F0327 launch direction");
    ASSERT_EQ(plan.launchCell, 2, "F0326 launch cell");
    ASSERT_EQ(plan.movementDisabledTicks, 0,
              "spell projectile does not use F0328 movement-disable gate");

    ASSERT_EQ(dm1_v1_build_spell_projectile_create_input_f0327_pc34(
                  &receipt, &context, &input), 1,
              "F0412/F0327 projectile create input builds");
    ASSERT_EQ(input.category, PROJECTILE_CATEGORY_MAGICAL,
              "spell projectile category");
    ASSERT_EQ(input.subtype, PROJECTILE_SUBTYPE_FIREBALL,
              "spell projectile create subtype");
    ASSERT_EQ(input.ownerKind, PROJECTILE_OWNER_CHAMPION,
              "spell projectile owner kind");
    ASSERT_EQ(input.ownerIndex, 1, "spell projectile owner index");
    ASSERT_EQ(input.mapX, 4, "spell projectile map x");
    ASSERT_EQ(input.mapY, 5, "spell projectile map y");
    ASSERT_EQ(input.cell, 2, "spell projectile create cell");
    ASSERT_EQ(input.direction, 2, "spell projectile create direction");
    ASSERT_EQ(input.kineticEnergy, 24, "spell projectile create kinetic");
    ASSERT_EQ(input.attack, 90, "spell projectile create attack");
    ASSERT_EQ(input.launcherStrength, 90,
              "spell projectile launcher strength follows F0327 attack");
    ASSERT_EQ(input.stepEnergy, 9, "spell projectile create step");
    ASSERT_EQ(input.currentTick, 1234, "spell projectile create tick");
    ASSERT_EQ(input.attackTypeCode, COMBAT_ATTACK_FIRE,
              "spell projectile create attack type");
    ASSERT_EQ(input.associatedThing, THING_NONE,
              "spell projectile carries no thrown item");
    ASSERT_EQ(input.firstMoveGraceFlag, 1, "spell projectile first move grace");

    receipt.projectileThing = DM1_PROJECTILE_THING_OPEN_DOOR;
    receipt.projectileKineticEnergy = 120;
    receipt.projectileStepEnergy = 7;
    receipt.championDirectionAfter = 3;
    context.championCell = 0;
    ASSERT_EQ(dm1_v1_build_spell_projectile_create_input_f0327_pc34(
                  &receipt, &context, &input), 1,
              "open-door spell projectile create input builds");
    ASSERT_EQ(input.subtype, PROJECTILE_SUBTYPE_OPEN_DOOR,
              "open-door spell subtype");
    ASSERT_EQ(input.attackTypeCode, COMBAT_ATTACK_MAGIC,
              "open-door spell attack type");
    ASSERT_EQ(input.kineticEnergy, 120,
              "open-door spell kinetic not adjusted");
    ASSERT_EQ(input.stepEnergy, 7, "open-door spell step not adjusted");
    ASSERT_EQ(input.direction, 3, "open-door spell direction");
    ASSERT_EQ(input.cell, 0, "open-door spell launch cell");

    receipt.castResult = DM1_SPELL_CAST_FAILURE;
    receipt.createsProjectile = 0;
    ASSERT_EQ(dm1_v1_spell_projectile_launch_plan_f0327_pc34(
                  &receipt, &context, &plan), 1,
              "failed spell launch plan is handled");
    ASSERT_EQ(plan.valid, 0, "failed spell has no launch plan");
    ASSERT_EQ(plan.shouldCreateProjectile, 0,
              "failed spell creates no projectile");
}

static void test_projectile_create_input_model(void) {
    DM1_ProjectileCreateRequestPc34 req;
    DM1_CreatureProjectileCreateRequestPc34 creatureReq;
    struct ProjectileCreateInput_Compat input;
    int subtype;
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

    ASSERT_EQ(dm1_v1_projectile_subtype_from_thing_pc34(
                  DM1_PROJECTILE_THING_LIGHTNING_BOLT, &subtype), 1,
              "lightning thing maps");
    ASSERT_EQ(subtype, PROJECTILE_SUBTYPE_LIGHTNING_BOLT,
              "lightning thing subtype");
    ASSERT_EQ(dm1_v1_projectile_attack_type_for_subtype_pc34(
                  PROJECTILE_SUBTYPE_LIGHTNING_BOLT),
              COMBAT_ATTACK_LIGHTNING, "lightning attack type");
    ASSERT_EQ(dm1_v1_projectile_subtype_from_thing_pc34(0x1234, &subtype), 0,
              "unknown projectile thing rejected");
    ASSERT_EQ(subtype, -1, "unknown projectile subtype reset");

    memset(&creatureReq, 0, sizeof(creatureReq));
    memset(&input, 0, sizeof(input));
    creatureReq.creatureGroupIndex = 7;
    creatureReq.partyMapIndex = 4;
    creatureReq.groupMapX = 8;
    creatureReq.groupMapY = 9;
    creatureReq.projectileThing = DM1_PROJECTILE_THING_POISON_CLOUD;
    creatureReq.targetCell = 6;
    creatureReq.direction = 5;
    creatureReq.kineticEnergy = 77;
    creatureReq.attack = 55;
    creatureReq.stepEnergy = 0;
    creatureReq.gameTick = 4321;
    ASSERT_EQ(dm1_v1_build_creature_projectile_create_input_pc34(
                  &creatureReq, &input), 1,
              "creature projectile input builds");
    ASSERT_EQ(input.ownerKind, PROJECTILE_OWNER_CREATURE,
              "creature projectile owner kind");
    ASSERT_EQ(input.ownerIndex, 7, "creature projectile owner index");
    ASSERT_EQ(input.mapX, 8, "creature projectile map x");
    ASSERT_EQ(input.mapY, 9, "creature projectile map y");
    ASSERT_EQ(input.subtype, PROJECTILE_SUBTYPE_POISON_CLOUD,
              "creature poison cloud subtype");
    ASSERT_EQ(input.cell, 2, "creature projectile cell masked");
    ASSERT_EQ(input.direction, 1, "creature projectile direction masked");
    ASSERT_EQ(input.stepEnergy, 1, "creature projectile step clamp");
    ASSERT_EQ(input.poisonAttack, 55, "creature poison attack");
    ASSERT_EQ(input.attackTypeCode, COMBAT_ATTACK_NORMAL,
              "creature poison attack type");
    ASSERT_EQ(input.associatedThing, THING_NONE,
              "creature projectile no associated thing");
    ASSERT_EQ(input.firstMoveGraceFlag, 1,
              "creature projectile first move grace");

    creatureReq.projectileThing = DM1_PROJECTILE_THING_OPEN_DOOR;
    creatureReq.attack = 66;
    creatureReq.stepEnergy = 4;
    ASSERT_EQ(dm1_v1_build_creature_projectile_create_input_pc34(
                  &creatureReq, &input), 1,
              "creature open door projectile input builds");
    ASSERT_EQ(input.subtype, PROJECTILE_SUBTYPE_OPEN_DOOR,
              "creature open door subtype");
    ASSERT_EQ(input.poisonAttack, 0, "open door no poison attack");
    ASSERT_EQ(input.attackTypeCode, COMBAT_ATTACK_MAGIC,
              "open door magic attack type");
}

static void test_projectile_impact_model(void) {
    struct ProjectileInstance_Compat p;
    struct ProjectileTickResult_Compat r;
    struct ExplosionCreateInput_Compat e;
    DM1_ProjectileImpactLogPlanPc34 logPlan;
    memset(&p, 0, sizeof(p));
    memset(&r, 0, sizeof(r));
    memset(&e, 0, sizeof(e));
    memset(&logPlan, 0, sizeof(logPlan));
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

    r.outExplosion.explosionType = C002_EXPLOSION_LIGHTNING_BOLT;
    r.outExplosion.attack = 37;
    r.outExplosion.mapIndex = 4;
    r.outExplosion.mapX = 12;
    r.outExplosion.mapY = 13;
    r.outExplosion.cell = 2;
    r.outExplosion.centered = 1;
    r.outExplosion.poisonAttack = 17;
    r.outExplosion.ownerKind = PROJECTILE_OWNER_CHAMPION;
    r.outExplosion.ownerIndex = 3;
    r.outExplosion.creatorProjectileSlot = 9;
    ASSERT_EQ(dm1_v1_projectile_explosion_create_input_pc34(&r, 456, &e), 1,
              "explosion input builds");
    ASSERT_EQ(e.explosionType, C002_EXPLOSION_LIGHTNING_BOLT,
              "explosion type copied");
    ASSERT_EQ(e.attack, 37, "explosion attack copied");
    ASSERT_EQ(e.mapIndex, 4, "explosion map copied");
    ASSERT_EQ(e.mapX, 12, "explosion x copied");
    ASSERT_EQ(e.mapY, 13, "explosion y copied");
    ASSERT_EQ(e.cell, 2, "explosion cell copied");
    ASSERT_EQ(e.centered, 1, "explosion centered copied");
    ASSERT_EQ(e.poisonAttack, 17, "explosion poison copied");
    ASSERT_EQ(e.currentTick, 456, "explosion tick supplied");
    ASSERT_EQ(e.ownerKind, PROJECTILE_OWNER_CHAMPION,
              "explosion owner kind copied");
    ASSERT_EQ(e.ownerIndex, 3, "explosion owner index copied");
    ASSERT_EQ(e.creatorProjectileSlot, 9,
              "explosion creator slot copied");
    r.emittedExplosion = 0;
    ASSERT_EQ(dm1_v1_projectile_explosion_create_input_pc34(&r, 456, &e), 0,
              "no explosion input without emission");

    r.resultKind = PROJECTILE_RESULT_HIT_WALL;
    ASSERT_EQ(dm1_v1_projectile_impact_log_plan_pc34(&r, &logPlan), 1,
              "wall log plan builds");
    ASSERT_EQ(logPlan.handled, 1, "wall log handled");
    ASSERT_EQ(logPlan.logKind, DM1_PROJECTILE_IMPACT_LOG_HIT_WALL_PC34,
              "wall log kind");
    r.resultKind = PROJECTILE_RESULT_HIT_DOOR;
    (void)dm1_v1_projectile_impact_log_plan_pc34(&r, &logPlan);
    ASSERT_EQ(logPlan.logKind, DM1_PROJECTILE_IMPACT_LOG_HIT_DOOR_PC34,
              "door log kind");
    r.resultKind = PROJECTILE_RESULT_HIT_FLUXCAGE;
    (void)dm1_v1_projectile_impact_log_plan_pc34(&r, &logPlan);
    ASSERT_EQ(logPlan.logKind, DM1_PROJECTILE_IMPACT_LOG_HIT_FLUXCAGE_PC34,
              "fluxcage log kind");
    r.resultKind = PROJECTILE_RESULT_HIT_OTHER_PROJECTILE;
    (void)dm1_v1_projectile_impact_log_plan_pc34(&r, &logPlan);
    ASSERT_EQ(logPlan.logKind,
              DM1_PROJECTILE_IMPACT_LOG_HIT_OTHER_PROJECTILE_PC34,
              "other projectile log kind");
    r.resultKind = PROJECTILE_RESULT_DESPAWN_ENERGY;
    (void)dm1_v1_projectile_impact_log_plan_pc34(&r, &logPlan);
    ASSERT_EQ(logPlan.logKind, DM1_PROJECTILE_IMPACT_LOG_DESPAWN_ENERGY_PC34,
              "energy despawn log kind");
    r.resultKind = PROJECTILE_RESULT_DESPAWN_BOUNDS;
    (void)dm1_v1_projectile_impact_log_plan_pc34(&r, &logPlan);
    ASSERT_EQ(logPlan.logKind, DM1_PROJECTILE_IMPACT_LOG_DESPAWN_BOUNDS_PC34,
              "bounds despawn log kind");
    r.resultKind = PROJECTILE_RESULT_FLEW;
    (void)dm1_v1_projectile_impact_log_plan_pc34(&r, &logPlan);
    ASSERT_EQ(logPlan.handled, 0, "flew log not handled");
    ASSERT_EQ(logPlan.logKind, DM1_PROJECTILE_IMPACT_LOG_NONE_PC34,
              "flew log none");

    ASSERT_EQ(dm1_v1_thrown_sharp_weapon_type_kept_by_creature_pc34(8), 1,
              "dagger kept");
    ASSERT_EQ(dm1_v1_thrown_sharp_weapon_type_kept_by_creature_pc34(32), 1,
              "throwing star kept");
    ASSERT_EQ(dm1_v1_thrown_sharp_weapon_type_kept_by_creature_pc34(9), 0,
              "ordinary weapon not kept");
}

static void test_projectile_group_slot_materialization_plan(void) {
    struct ProjectileInstance_Compat p;
    DM1_ProjectileGroupSlotMaterializationPlanPc34 plan;
    DM1_ProjectileGroupSlotAttachPlanPc34 attach;
    DM1_ProjectileGroupSlotAttachReceiptPc34 receipt;
    unsigned short chain[66];
    unsigned short weaponThing =
        (unsigned short)((THING_TYPE_WEAPON << 10) | 7);
    unsigned short tailThing =
        (unsigned short)((THING_TYPE_JUNK << 10) | 3);
    memset(&p, 0, sizeof(p));
    memset(&plan, 0, sizeof(plan));
    memset(&attach, 0, sizeof(attach));

    p.reserved1 = weaponThing;
    ASSERT_EQ(dm1_v1_projectile_group_slot_materialization_plan_pc34(
                  &p, COMBAT_OUTCOME_KILLED_NO_CREATURES,
                  DM1_PROJECTILE_ATTR_KEEP_THROWN_SHARP_WEAPONS_PC34,
                  8, &plan), 1,
              "group slot materialization plan builds");
    ASSERT_EQ(plan.valid, 1, "group slot materialization valid");
    ASSERT_EQ(plan.shouldAttachToGroupSlot, 1,
              "dagger attaches to keep-sharp group slot");
    ASSERT_EQ(plan.associatedThing, weaponThing,
              "group slot materialization keeps thing");
    ASSERT_EQ(plan.weaponType, 8, "group slot materialization weapon type");

    ASSERT_EQ(dm1_v1_projectile_group_slot_materialization_plan_pc34(
                  &p, COMBAT_OUTCOME_KILLED_SOME_CREATURES,
                  DM1_PROJECTILE_ATTR_KEEP_THROWN_SHARP_WEAPONS_PC34,
                  8, &plan), 1,
              "killed creature group slot materialization builds");
    ASSERT_EQ(plan.shouldAttachToGroupSlot, 0,
              "killed creature does not keep projectile weapon");

    ASSERT_EQ(dm1_v1_projectile_group_slot_materialization_plan_pc34(
                  &p, COMBAT_OUTCOME_KILLED_NO_CREATURES,
                  DM1_PROJECTILE_ATTR_KEEP_THROWN_SHARP_WEAPONS_PC34,
                  9, &plan), 1,
              "ordinary weapon group slot materialization builds");
    ASSERT_EQ(plan.shouldAttachToGroupSlot, 0,
              "ordinary weapon does not attach to group slot");

    ASSERT_EQ(dm1_v1_projectile_group_slot_materialization_plan_pc34(
                  &p, COMBAT_OUTCOME_KILLED_NO_CREATURES, 0, 8, &plan), 1,
              "no keep attr group slot materialization builds");
    ASSERT_EQ(plan.shouldAttachToGroupSlot, 0,
              "creature without keep attr does not attach");

    ASSERT_EQ(dm1_v1_projectile_group_slot_attach_plan_f0215_pc34(
                  weaponThing, THING_ENDOFLIST, THING_NONE, &attach), 1,
              "F0215 empty group slot attach plan builds");
    ASSERT_EQ(attach.valid, 1, "F0215 empty attach valid");
    ASSERT_EQ(attach.shouldSetAssociatedNextEnd, 1,
              "F0215 empty attach terminates associated thing");
    ASSERT_EQ(attach.shouldSetGroupSlotHead, 1,
              "F0215 empty attach sets group slot head");
    ASSERT_EQ(attach.shouldAppendAfterTail, 0,
              "F0215 empty attach skips tail append");

    memset(&receipt, 0, sizeof(receipt));
    chain[0] = THING_ENDOFLIST;
    ASSERT_EQ(dm1_v1_projectile_group_slot_attach_receipt_f0215_pc34(
                  weaponThing, THING_ENDOFLIST, chain, 1, &receipt), 1,
              "F0215 empty group slot attach receipt builds");
    ASSERT_EQ(receipt.valid, 1, "F0215 empty group receipt valid");
    ASSERT_EQ(receipt.shouldSetAssociatedNextEnd, 1,
              "F0215 empty group receipt terminates associated thing");
    ASSERT_EQ(receipt.shouldSetGroupSlotHead, 1,
              "F0215 empty group receipt sets group slot head");
    ASSERT_EQ(receipt.shouldAppendAfterTail, 0,
              "F0215 empty group receipt skips append");

    ASSERT_EQ(dm1_v1_projectile_group_slot_attach_plan_f0215_pc34(
                  weaponThing, tailThing, tailThing, &attach), 1,
              "F0215 occupied group slot attach plan builds");
    ASSERT_EQ(attach.valid, 1, "F0215 occupied attach valid");
    ASSERT_EQ(attach.shouldSetGroupSlotHead, 0,
              "F0215 occupied attach keeps head");
    ASSERT_EQ(attach.shouldAppendAfterTail, 1,
              "F0215 occupied attach appends after tail");
    ASSERT_EQ(attach.tailThing, tailThing, "F0215 occupied attach tail");

    chain[0] = tailThing;
    chain[1] = THING_ENDOFLIST;
    memset(&receipt, 0, sizeof(receipt));
    ASSERT_EQ(dm1_v1_projectile_group_slot_attach_receipt_f0215_pc34(
                  weaponThing, tailThing, chain, 2, &receipt), 1,
              "F0215 occupied group slot attach receipt builds");
    ASSERT_EQ(receipt.valid, 1, "F0215 occupied group receipt valid");
    ASSERT_EQ(receipt.shouldSetAssociatedNextEnd, 1,
              "F0215 occupied group receipt terminates associated thing");
    ASSERT_EQ(receipt.shouldSetGroupSlotHead, 0,
              "F0215 occupied group receipt keeps group slot head");
    ASSERT_EQ(receipt.shouldAppendAfterTail, 1,
              "F0215 occupied group receipt appends after tail");
    ASSERT_EQ(receipt.foundTail, 1,
              "F0215 occupied group receipt finds tail");
    ASSERT_EQ(receipt.tailThing, tailThing,
              "F0215 occupied group receipt tail thing");

    memset(chain, 0, sizeof(chain));
    for (int i = 0; i < 66; ++i) {
        chain[i] = (unsigned short)((THING_TYPE_JUNK << 10) | (i & 0x03ff));
    }
    memset(&receipt, 0, sizeof(receipt));
    ASSERT_EQ(dm1_v1_projectile_group_slot_attach_receipt_f0215_pc34(
                  weaponThing, tailThing, chain, 66, &receipt), 1,
              "F0215 overflow group slot attach receipt builds");
    ASSERT_EQ(receipt.valid, 1, "F0215 overflow group receipt valid");
    ASSERT_EQ(receipt.chainOverflow, 1,
              "F0215 overflow group receipt reports overflow");

    ASSERT_EQ(dm1_v1_projectile_group_slot_attach_plan_f0215_pc34(
                  (unsigned short)((THING_TYPE_EXPLOSION << 10) | 1),
                  THING_ENDOFLIST, THING_NONE, &attach), 0,
              "F0215 explosion slot is not attached");
    ASSERT_EQ(dm1_v1_projectile_group_slot_attach_receipt_f0215_pc34(
                  (unsigned short)((THING_TYPE_EXPLOSION << 10) | 1),
                  THING_ENDOFLIST, chain, 1, &receipt), 0,
              "F0215 explosion group receipt is rejected");
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

    p.reserved1 = (unsigned short)((THING_TYPE_PROJECTILE << 10) | 2);
    ASSERT_EQ(dm1_v1_projectile_associated_thing_disposition_pc34(
                  &p, NULL, 0, 0, &d), 1,
              "projectile-associated disposition builds");
    ASSERT_EQ(d.shouldMaterialize, 0,
              "projectile-associated thing is not materialized as floor item");

    p.reserved1 = (unsigned short)((THING_TYPE_GROUP << 10) | 1);
    ASSERT_EQ(dm1_v1_projectile_associated_thing_disposition_pc34(
                  &p, NULL, 0, 0, &d), 1,
              "group-associated disposition builds");
    ASSERT_EQ(d.shouldMaterialize, 0,
              "group-associated thing is not materialized as floor item");
}

static void test_projectile_materialization_plan(void) {
    struct ProjectileInstance_Compat p;
    struct ProjectileTickResult_Compat r;
    DM1_ProjectileMaterializationPlanPc34 plan;
    DM1_ProjectileMaterializationReceiptPc34 materialReceipt;
    DM1_ProjectileSquareAttachPlanPc34 attach;
    DM1_ProjectileSquareAttachReceiptPc34 receipt;
    unsigned short chain[66];
    unsigned short weaponThing =
        (unsigned short)((THING_TYPE_WEAPON << 10) | 7);
    unsigned short droppedThing =
        (unsigned short)(weaponThing | (unsigned short)(2u << 14));
    unsigned short tailThing =
        (unsigned short)((THING_TYPE_JUNK << 10) | 3);
    memset(&p, 0, sizeof(p));
    memset(&r, 0, sizeof(r));
    memset(&plan, 0, sizeof(plan));

    p.reserved1 = weaponThing;
    p.mapIndex = 2;
    p.mapX = 10;
    p.mapY = 11;
    p.cell = 1;
    r.resultKind = PROJECTILE_RESULT_HIT_WALL;
    r.despawn = 1;
    ASSERT_EQ(dm1_v1_projectile_materialization_plan_pc34(
                  &p, &r, 0, 0, &plan), 1,
              "wall materialization plan builds");
    ASSERT_EQ(plan.handled, 1, "wall materialization handled");
    ASSERT_EQ(plan.shouldMaterialize, 1, "wall materializes weapon");
    ASSERT_EQ(plan.mapIndex, 2, "wall materialize source map");
    ASSERT_EQ(plan.mapX, 10, "wall materialize source x");
    ASSERT_EQ(plan.mapY, 11, "wall materialize source y");
    ASSERT_EQ(plan.cell, 1, "wall materialize source cell");
    ASSERT_EQ(plan.droppedThing,
              (unsigned short)(weaponThing | (unsigned short)(1u << 14)),
              "wall materialize source-cell thing");
    chain[0] = THING_ENDOFLIST;
    memset(&materialReceipt, 0, sizeof(materialReceipt));
    ASSERT_EQ(dm1_v1_projectile_materialization_receipt_f0215_pc34(
                  &p, &r, 0, 0, THING_ENDOFLIST, chain, 1,
                  &materialReceipt), 1,
              "wall materialization receipt builds");
    ASSERT_EQ(materialReceipt.valid, 1,
              "wall materialization receipt valid");
    ASSERT_EQ(materialReceipt.shouldDeleteProjectile, 1,
              "wall materialization receipt deletes projectile");
    ASSERT_EQ(materialReceipt.shouldClearProjectileNext, 1,
              "wall materialization receipt clears projectile next");
    ASSERT_EQ(materialReceipt.projectileThing,
              (unsigned short)((THING_TYPE_PROJECTILE << 10) |
                               (unsigned short)(1u << 14)),
              "wall materialization receipt carries projectile thing");
    ASSERT_EQ(materialReceipt.projectileNextAfterDelete, THING_NONE,
              "wall materialization receipt clears projectile list link");
    ASSERT_EQ(materialReceipt.shouldMaterialize, 1,
              "wall materialization receipt materializes");
    ASSERT_EQ(materialReceipt.mapX, 10,
              "wall materialization receipt map x");
    ASSERT_EQ(materialReceipt.squareAttach.shouldSetSquareFirstThing, 1,
              "wall materialization receipt owns empty square attach");
    ASSERT_EQ(materialReceipt.squareAttach.droppedThing,
              (unsigned short)(weaponThing | (unsigned short)(1u << 14)),
              "wall materialization receipt dropped thing");

    r.resultKind = PROJECTILE_RESULT_HIT_CHAMPION;
    r.emittedCombatAction = 1;
    r.newMapIndex = 3;
    r.newMapX = 20;
    r.newMapY = 21;
    r.newCell = 2;
    ASSERT_EQ(dm1_v1_projectile_materialization_plan_pc34(
                  &p, &r, 0, 0, &plan), 1,
              "champion materialization plan builds");
    ASSERT_EQ(plan.mapIndex, 3, "champion materialize impact map");
    ASSERT_EQ(plan.mapX, 20, "champion materialize impact x");
    ASSERT_EQ(plan.mapY, 21, "champion materialize impact y");
    ASSERT_EQ(plan.cell, 2, "champion materialize impact cell");
    ASSERT_EQ(plan.droppedThing,
              (unsigned short)(weaponThing | (unsigned short)(2u << 14)),
              "champion materialize impact-cell thing");
    chain[0] = tailThing;
    chain[1] = THING_ENDOFLIST;
    memset(&materialReceipt, 0, sizeof(materialReceipt));
    ASSERT_EQ(dm1_v1_projectile_materialization_receipt_f0215_pc34(
                  &p, &r, 0, 0, tailThing, chain, 2, &materialReceipt), 1,
              "champion materialization receipt builds");
    ASSERT_EQ(materialReceipt.mapIndex, 3,
              "champion materialization receipt impact map");
    ASSERT_EQ(materialReceipt.cell, 2,
              "champion materialization receipt impact cell");
    ASSERT_EQ(materialReceipt.squareAttach.shouldAppendAfterTail, 1,
              "champion materialization receipt appends to occupied square");
    ASSERT_EQ(materialReceipt.squareAttach.tailThing, tailThing,
              "champion materialization receipt tail thing");

    p.flags = PROJECTILE_FLAG_REMOVE_POTION_ON_IMPACT;
    p.reserved1 = (unsigned short)((THING_TYPE_POTION << 10) | 1);
    r.despawn = 1;
    ASSERT_EQ(dm1_v1_projectile_materialization_plan_pc34(
                  &p, &r, 0, 4, &plan), 1,
              "potion materialization plan builds");
    ASSERT_EQ(plan.shouldConsumePotion, 1, "potion materialization consumes");
    ASSERT_EQ(plan.shouldMaterialize, 0, "potion materialization blocked");
    memset(&materialReceipt, 0, sizeof(materialReceipt));
    ASSERT_EQ(dm1_v1_projectile_materialization_receipt_f0215_pc34(
                  &p, &r, 0, 4, THING_ENDOFLIST, chain, 1,
                  &materialReceipt), 1,
              "potion materialization receipt builds");
    ASSERT_EQ(materialReceipt.shouldConsumePotion, 1,
              "potion materialization receipt consumes");
    ASSERT_EQ(materialReceipt.shouldDeleteProjectile, 1,
              "potion materialization receipt still deletes projectile");
    ASSERT_EQ(materialReceipt.shouldClearProjectileNext, 1,
              "potion materialization receipt still clears projectile next");
    ASSERT_EQ(materialReceipt.shouldMaterialize, 0,
              "potion materialization receipt skips attach");
    ASSERT_EQ(materialReceipt.squareAttach.valid, 0,
              "potion materialization receipt has no square attach");

    p.flags = 0;
    p.reserved1 = (unsigned short)((THING_TYPE_PROJECTILE << 10) | 2);
    ASSERT_EQ(dm1_v1_projectile_materialization_plan_pc34(
                  &p, &r, 0, 0, &plan), 1,
              "projectile thing materialization plan builds");
    ASSERT_EQ(plan.handled, 1, "projectile thing materialization handled");
    ASSERT_EQ(plan.shouldMaterialize, 0,
              "projectile thing is not materialized into a dungeon square");
    memset(&materialReceipt, 0, sizeof(materialReceipt));
    ASSERT_EQ(dm1_v1_projectile_materialization_receipt_f0215_pc34(
                  &p, &r, 0, 0, THING_ENDOFLIST, chain, 1,
                  &materialReceipt), 1,
              "projectile thing materialization receipt builds");
    ASSERT_EQ(materialReceipt.shouldMaterialize, 0,
              "projectile thing materialization receipt blocks attach");

    p.reserved1 = (unsigned short)((THING_TYPE_SENSOR << 10) | 1);
    ASSERT_EQ(dm1_v1_projectile_materialization_plan_pc34(
                  &p, &r, 0, 0, &plan), 1,
              "sensor thing materialization plan builds");
    ASSERT_EQ(plan.shouldMaterialize, 0,
              "sensor thing is not materialized into a dungeon square");

    ASSERT_EQ(dm1_v1_projectile_square_attach_plan_f0215_pc34(
                  droppedThing, THING_ENDOFLIST, THING_NONE, &attach), 1,
              "F0215 empty square attach plan builds");
    ASSERT_EQ(attach.valid, 1, "F0215 empty square attach valid");
    ASSERT_EQ(attach.baseThing, weaponThing,
              "F0215 empty square attach strips cell bits for next write");
    ASSERT_EQ(attach.shouldSetDroppedNextEnd, 1,
              "F0215 empty square attach terminates dropped thing");
    ASSERT_EQ(attach.shouldSetSquareFirstThing, 1,
              "F0215 empty square attach sets first thing");
    ASSERT_EQ(attach.shouldAppendAfterTail, 0,
              "F0215 empty square attach skips tail append");
    memset(&receipt, 0, sizeof(receipt));
    chain[0] = THING_ENDOFLIST;
    ASSERT_EQ(dm1_v1_projectile_square_attach_receipt_f0215_pc34(
                  droppedThing, THING_ENDOFLIST, chain, 1, &receipt), 1,
              "F0215 empty square attach receipt builds");
    ASSERT_EQ(receipt.valid, 1, "F0215 empty square receipt valid");
    ASSERT_EQ(receipt.shouldSetDroppedNextEnd, 1,
              "F0215 empty square receipt terminates dropped thing");
    ASSERT_EQ(receipt.shouldSetSquareFirstThing, 1,
              "F0215 empty square receipt sets first thing");
    ASSERT_EQ(receipt.shouldAppendAfterTail, 0,
              "F0215 empty square receipt skips append");

    ASSERT_EQ(dm1_v1_projectile_square_attach_plan_f0215_pc34(
                  droppedThing, tailThing, tailThing, &attach), 1,
              "F0215 occupied square attach plan builds");
    ASSERT_EQ(attach.valid, 1, "F0215 occupied square attach valid");
    ASSERT_EQ(attach.shouldSetSquareFirstThing, 0,
              "F0215 occupied square attach keeps first thing");
    ASSERT_EQ(attach.shouldAppendAfterTail, 1,
              "F0215 occupied square attach appends after tail");
    ASSERT_EQ(attach.tailThing, tailThing,
              "F0215 occupied square attach tail");
    chain[0] = tailThing;
    chain[1] = THING_ENDOFLIST;
    memset(&receipt, 0, sizeof(receipt));
    ASSERT_EQ(dm1_v1_projectile_square_attach_receipt_f0215_pc34(
                  droppedThing, tailThing, chain, 2, &receipt), 1,
              "F0215 occupied square attach receipt builds");
    ASSERT_EQ(receipt.valid, 1, "F0215 occupied square receipt valid");
    ASSERT_EQ(receipt.shouldSetDroppedNextEnd, 1,
              "F0215 occupied square receipt terminates dropped thing");
    ASSERT_EQ(receipt.shouldSetSquareFirstThing, 0,
              "F0215 occupied square receipt keeps first thing");
    ASSERT_EQ(receipt.shouldAppendAfterTail, 1,
              "F0215 occupied square receipt appends after tail");
    ASSERT_EQ(receipt.foundTail, 1,
              "F0215 occupied square receipt finds tail");
    ASSERT_EQ(receipt.tailThing, tailThing,
              "F0215 occupied square receipt tail thing");

    memset(chain, 0, sizeof(chain));
    for (int i = 0; i < 66; ++i) {
        chain[i] = (unsigned short)((THING_TYPE_JUNK << 10) | (i & 0x03ff));
    }
    memset(&receipt, 0, sizeof(receipt));
    ASSERT_EQ(dm1_v1_projectile_square_attach_receipt_f0215_pc34(
                  droppedThing, tailThing, chain, 66, &receipt), 1,
              "F0215 overflow square attach receipt builds");
    ASSERT_EQ(receipt.valid, 1, "F0215 overflow square receipt valid");
    ASSERT_EQ(receipt.chainOverflow, 1,
              "F0215 overflow square receipt reports overflow");

    ASSERT_EQ(dm1_v1_projectile_square_attach_plan_f0215_pc34(
                  (unsigned short)((THING_TYPE_EXPLOSION << 10) | 1),
                  THING_ENDOFLIST, THING_NONE, &attach), 0,
              "F0215 explosion square attach is rejected");
    ASSERT_EQ(dm1_v1_projectile_square_attach_receipt_f0215_pc34(
                  (unsigned short)((THING_TYPE_EXPLOSION << 10) | 1),
                  THING_ENDOFLIST, chain, 1, &receipt), 0,
              "F0215 explosion square receipt is rejected");
}

static void test_projectile_flight_relink_receipt(void) {
    struct ProjectileInstance_Compat before;
    struct ProjectileInstance_Compat after;
    struct ProjectileTickResult_Compat result;
    DM1_ProjectileFlightRelinkReceiptPc34 receipt;

    memset(&before, 0, sizeof(before));
    memset(&after, 0, sizeof(after));
    memset(&result, 0, sizeof(result));
    before.slotIndex = 6;
    before.mapIndex = 1;
    before.mapX = 4;
    before.mapY = 5;
    before.cell = 1;
    after = before;
    after.mapX = 5;
    after.cell = 2;
    after.direction = 3;
    after.kineticEnergy = 14;
    after.attack = 9;
    result.resultKind = PROJECTILE_RESULT_FLEW;
    result.newMapIndex = after.mapIndex;
    result.newMapX = after.mapX;
    result.newMapY = after.mapY;
    result.newCell = after.cell;
    result.newDirection = after.direction;
    result.newKineticEnergy = after.kineticEnergy;
    result.newAttack = after.attack;
    result.newFirstMoveGraceFlag = 0;

    ASSERT_EQ(dm1_v1_projectile_flight_relink_receipt_f0219_pc34(
                  &before, &after, &result, &receipt), 1,
              "F0219 flight relink receipt builds");
    ASSERT_EQ(receipt.valid, 1, "F0219 flight relink receipt valid");
    ASSERT_EQ(receipt.shouldApply, 1, "F0219 flight relink applies");
    ASSERT_EQ(receipt.shouldUnlinkSourceSquare, 1,
              "F0219 relink unlinks source square");
    ASSERT_EQ(receipt.shouldLinkDestinationSquare, 1,
              "F0219 relink links destination square");
    ASSERT_EQ(receipt.shouldWriteProjectileState, 1,
              "F0219 relink writes projectile state");
    ASSERT_EQ(receipt.shouldScheduleNextMove, 1,
              "F0219 relink schedules next move");
    ASSERT_EQ(receipt.sourceProjectileThing,
              (unsigned short)((THING_TYPE_PROJECTILE << 10) |
                               6u | (unsigned short)(1u << 14)),
              "F0219 source projectile thing keeps source cell");
    ASSERT_EQ(receipt.destinationProjectileThing,
              (unsigned short)((THING_TYPE_PROJECTILE << 10) |
                               6u | (unsigned short)(2u << 14)),
              "F0219 destination projectile thing keeps destination cell");
    ASSERT_EQ(receipt.destinationMapX, 5,
              "F0219 destination x carried");
    ASSERT_EQ(receipt.destinationCell, 2,
              "F0219 destination cell carried");

    before.mapX = 4;
    before.cell = 2;
    after = before;
    after.kineticEnergy = 7;
    result.newMapX = before.mapX;
    result.newCell = before.cell;
    result.newDirection = after.direction;
    result.newKineticEnergy = after.kineticEnergy;
    result.newAttack = after.attack;
    result.newFirstMoveGraceFlag = after.firstMoveGraceFlag;
    ASSERT_EQ(dm1_v1_projectile_flight_relink_receipt_f0219_pc34(
                  &before, &after, &result, &receipt), 1,
              "F0219 same-cell flight relink receipt builds");
    ASSERT_EQ(receipt.shouldUnlinkSourceSquare, 0,
              "F0219 same-cell flight skips source unlink");
    ASSERT_EQ(receipt.shouldLinkDestinationSquare, 0,
              "F0219 same-cell flight skips destination link");

    result.despawn = 1;
    ASSERT_EQ(dm1_v1_projectile_flight_relink_receipt_f0219_pc34(
                  &before, &after, &result, &receipt), 0,
              "F0219 despawn does not build flight relink receipt");
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
    struct CombatAction_Compat action;
    DM1_ProjectileCreatureImpactPlanPc34 plan;
    DM1_ProjectileCreatureActionPlanPc34 actionPlan;
    DM1_ProjectileCreatureActionApplyPlanPc34 actionApply;
    DM1_ProjectileCreatureImpactAftermathPc34 aftermath;
    DM1_ProjectileCreaturePrecheckDamagePlanPc34 precheck;
    unsigned short weaponThing =
        (unsigned short)((THING_TYPE_WEAPON << 10) | 1);
    memset(&group, 0, sizeof(group));
    memset(&p, 0, sizeof(p));
    memset(&r, 0, sizeof(r));
    memset(&action, 0, sizeof(action));
    memset(&plan, 0, sizeof(plan));
    memset(&actionPlan, 0, sizeof(actionPlan));
    memset(&actionApply, 0, sizeof(actionApply));
    memset(&aftermath, 0, sizeof(aftermath));
    memset(&precheck, 0, sizeof(precheck));

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

    action.kind = COMBAT_ACTION_APPLY_DAMAGE_GROUP;
    action.targetCell = 3;
    action.rawAttackValue = 17;
    ASSERT_EQ(dm1_v1_projectile_creature_action_plan_pc34(
                  &p, &action, &group, 0, &actionPlan), 1,
              "creature action plan builds");
    ASSERT_EQ(actionPlan.handled, 1, "creature action handled");
    ASSERT_EQ(actionPlan.shouldApplyDamage, 1,
              "creature action applies damage");
    ASSERT_EQ(actionPlan.slotIndex, 1, "creature action target slot");
    ASSERT_EQ(actionPlan.damageApplied, 17, "creature action damage");
    ASSERT_EQ(actionPlan.killedCell, 3, "creature action killed cell");

    ASSERT_EQ(dm1_v1_projectile_creature_action_apply_pc34(
                  &actionPlan, &group, &actionApply), 1,
              "creature action apply builds");
    ASSERT_EQ(actionApply.valid, 1, "creature action apply valid");
    ASSERT_EQ(actionApply.handled, 1, "creature action apply handled");
    ASSERT_EQ(actionApply.creatureIndex, 1, "creature action apply slot");
    ASSERT_EQ(actionApply.damageApplied, 17, "creature action apply damage");
    ASSERT_EQ(actionApply.damage.damageApplied, 17,
              "creature action apply payload");
    ASSERT_EQ(actionApply.outcomeCode, COMBAT_OUTCOME_KILLED_NO_CREATURES,
              "creature action apply outcome");
    ASSERT_EQ(group.health[1], 3, "creature action apply mutates hp");
    group.health[1] = 20;

    ASSERT_EQ(dm1_v1_projectile_creature_action_aftermath_pc34(
                  &actionPlan, &p,
                  DM1_PROJECTILE_ATTR_KEEP_THROWN_SHARP_WEAPONS_PC34,
                  DM1_BEHAVIOR_ATTACK,
                  COMBAT_OUTCOME_KILLED_NO_CREATURES,
                  27,
                  &aftermath), 1,
              "creature action aftermath builds");
    ASSERT_EQ(aftermath.scheduleReaction, 1,
              "creature action surviving group reacts");
    ASSERT_EQ(aftermath.keepSharpWeaponInGroup, 1,
              "creature action sharp weapon kept");
    ASSERT_EQ(aftermath.spawnDeathSmoke, 0,
              "creature action no death smoke without kill");

    ASSERT_EQ(dm1_v1_projectile_creature_action_aftermath_pc34(
                  &actionPlan, &p,
                  DM1_PROJECTILE_ATTR_DROP_FIXED_POSSESSION_PC34,
                  DM1_BEHAVIOR_ATTACK,
                  COMBAT_OUTCOME_KILLED_SOME_CREATURES,
                  27,
                  &aftermath), 1,
              "creature action killed-some aftermath builds");
    ASSERT_EQ(aftermath.cleanupEventsAndFear, 1,
              "creature action partial kill cleans attack events");
    ASSERT_EQ(aftermath.dropFixedPossessions, 1,
              "creature action partial kill drops fixed possessions");
    ASSERT_EQ(aftermath.spawnDeathSmoke, 1,
              "creature action partial kill smoke");
    ASSERT_EQ(aftermath.keepSharpWeaponInGroup, 0,
              "creature action killing hit does not keep sharp weapon");

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
    ASSERT_EQ(dm1_v1_projectile_creature_action_plan_pc34(
                  &p, &action, &group,
                  DM1_PROJECTILE_ATTR_NON_MATERIAL_PC34, &actionPlan), 1,
              "non-material action plan builds");
    ASSERT_EQ(actionPlan.blockedByNonMaterial, 1,
              "non-material action blocks ordinary projectile");
    ASSERT_EQ(actionPlan.shouldApplyDamage, 0,
              "non-material action no damage");

    p.projectileSubtype = PROJECTILE_SUBTYPE_HARM_NON_MATERIAL;
    ASSERT_EQ(dm1_v1_projectile_creature_impact_plan_pc34(
                  &p, &r, &group,
                  DM1_PROJECTILE_ATTR_NON_MATERIAL_PC34, &plan), 1,
              "harm non-material plan builds");
    ASSERT_EQ(plan.blockedByNonMaterial, 0, "harm non-material bypasses block");
    ASSERT_EQ(plan.shouldApplyDamage, 1, "harm non-material damages");
    ASSERT_EQ(dm1_v1_projectile_creature_action_plan_pc34(
                  &p, &action, &group,
                  DM1_PROJECTILE_ATTR_NON_MATERIAL_PC34, &actionPlan), 1,
              "harm non-material action plan builds");
    ASSERT_EQ(actionPlan.blockedByNonMaterial, 0,
              "harm non-material action bypasses block");
    ASSERT_EQ(actionPlan.shouldApplyDamage, 1,
              "harm non-material action damages");

    memset(&group, 0, sizeof(group));
    memset(&p, 0, sizeof(p));
    group.creatureType = 6;
    group.count = 1;
    group.cells = (1 << 0) | (3 << 2);
    group.health[0] = 4;
    group.health[1] = 20;
    p.projectileCategory = PROJECTILE_CATEGORY_KINETIC;
    p.projectileSubtype = PROJECTILE_SUBTYPE_KINETIC_ARROW;
    p.attack = 64;
    ASSERT_EQ(dm1_v1_projectile_creature_precheck_damage_plan_pc34(
                  &p, &group, 0, 64, 0, &precheck), 1,
              "F0266 precheck damage plan builds");
    ASSERT_EQ(precheck.valid, 1, "F0266 precheck valid");
    ASSERT_EQ(precheck.shouldWriteGroup, 1, "F0266 precheck writes group");
    ASSERT_EQ(precheck.outcomeCode, 1, "F0266 precheck killed some");
    ASSERT_EQ(precheck.damageApplied, 64, "F0266 precheck damage");
    ASSERT_EQ(precheck.killedCell, 1, "F0266 precheck killed cell");
    ASSERT_EQ(precheck.newCount, 0, "F0266 precheck compacts count");
    ASSERT_EQ(precheck.newHealth[0], 20, "F0266 precheck shifts health");
    ASSERT_EQ(precheck.newHealth[1], 0, "F0266 precheck clears tail");
    ASSERT_EQ(dm1_v1_projectile_creature_precheck_aftermath_pc34(
                  &precheck, &p,
                  DM1_PROJECTILE_ATTR_KEEP_THROWN_SHARP_WEAPONS_PC34,
                  27, &aftermath), 1,
              "F0266 killed-some aftermath builds");
    ASSERT_EQ(aftermath.keepSharpWeaponInGroup, 0,
              "F0266 killed-some does not keep sharp weapon");

    group.health[0] = 100;
    ASSERT_EQ(dm1_v1_projectile_creature_precheck_damage_plan_pc34(
                  &p, &group, 0, 64, 0, &precheck), 1,
              "F0266 no-kill precheck damage plan builds");
    ASSERT_EQ(precheck.outcomeCode, 0,
              "F0266 no-kill outcome");
    ASSERT_EQ(dm1_v1_projectile_creature_precheck_aftermath_pc34(
                  &precheck, &p,
                  DM1_PROJECTILE_ATTR_KEEP_THROWN_SHARP_WEAPONS_PC34,
                  27, &aftermath), 1,
              "F0266 no-kill aftermath builds");
    ASSERT_EQ(aftermath.keepSharpWeaponInGroup, 1,
              "F0266 no-kill keeps sharp weapon");

    ASSERT_EQ(dm1_v1_projectile_creature_precheck_damage_plan_pc34(
                  &p, &group, 0, 64,
                  DM1_PROJECTILE_ATTR_NON_MATERIAL_PC34, &precheck), 1,
              "F0266 precheck non-material builds");
    ASSERT_EQ(precheck.shouldWriteGroup, 0,
              "F0266 precheck non-material blocks ordinary projectile");

    group.creatureType = DM1_PROJECTILE_BLACK_FLAME_CREATURE_PC34;
    group.count = 0;
    group.cells = DM1_PROJECTILE_SINGLE_CENTERED_CREATURE_CELL_PC34;
    group.health[0] = 990;
    p.projectileSubtype = PROJECTILE_SUBTYPE_FIREBALL;
    p.attack = 80;
    ASSERT_EQ(dm1_v1_projectile_creature_precheck_damage_plan_pc34(
                  &p, &group, 0, 64, 0, &precheck), 1,
              "F0266 black flame precheck builds");
    ASSERT_EQ(precheck.shouldWriteGroup, 1,
              "F0266 black flame precheck writes group");
    ASSERT_EQ(precheck.newHealth[0], DM1_PROJECTILE_BLACK_FLAME_MAX_HEALTH_PC34,
              "F0266 black flame heal caps");
    ASSERT_EQ(precheck.outcomeCode, 0, "F0266 black flame no kill outcome");

    memset(&action, 0, sizeof(action));
    action.kind = COMBAT_ACTION_APPLY_DAMAGE_GROUP;
    action.targetCell = 0;
    action.rawAttackValue = 80;
    ASSERT_EQ(dm1_v1_projectile_creature_action_plan_pc34(
                  &p, &action, &group, 0, &actionPlan), 1,
              "black flame action plan builds");
    ASSERT_EQ(actionPlan.healsBlackFlame, 1,
              "black flame action heals");
    ASSERT_EQ(actionPlan.shouldApplyDamage, 0,
              "black flame action skips damage");
    ASSERT_EQ(actionPlan.slotIndex, 0, "black flame action slot");
    ASSERT_EQ(actionPlan.newHealth, DM1_PROJECTILE_BLACK_FLAME_MAX_HEALTH_PC34,
              "black flame action heal cap");
}

static void test_projectile_champion_impact_plan(void) {
    struct ProjectileInstance_Compat p;
    struct ProjectileTickResult_Compat r;
    struct CombatAction_Compat action;
    struct CombatantChampionSnapshot_Compat defender;
    struct ChampionState_Compat championState;
    struct RngState_Compat rng;
    DM1_ProjectileChampionImpactPlanPc34 impact;
    DM1_ProjectileChampionDamageApplyPlanPc34 damagePlan;
    DM1_ProjectileChampionPoisonPlanPc34 poison;
    DM1_ProjectileChampionPoisonApplyPlanPc34 poisonApply;
    memset(&p, 0, sizeof(p));
    memset(&r, 0, sizeof(r));
    memset(&action, 0, sizeof(action));
    memset(&defender, 0, sizeof(defender));
    memset(&championState, 0, sizeof(championState));
    memset(&rng, 0, sizeof(rng));
    memset(&impact, 0, sizeof(impact));
    memset(&damagePlan, 0, sizeof(damagePlan));
    memset(&poison, 0, sizeof(poison));
    memset(&poisonApply, 0, sizeof(poisonApply));

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

    action.kind = COMBAT_ACTION_APPLY_DAMAGE_CHAMPION;
    action.defenderSlotOrCreatureIndex = 2;
    action.targetMapIndex = 4;
    action.targetMapX = 9;
    action.targetMapY = 10;
    action.targetCell = 1;
    action.attackTypeCode = COMBAT_ATTACK_FIRE;
    action.rawAttackValue = 77;
    action.allowedWounds = COMBAT_WOUND_LEGS;
    ASSERT_EQ(dm1_v1_projectile_champion_action_plan_pc34(
                  &p, &action, 1, &impact), 1,
              "champion action plan builds");
    ASSERT_EQ(impact.handled, 1, "champion action handled");
    ASSERT_EQ(impact.championPresent, 1, "champion action present");
    ASSERT_EQ(impact.championIndex, 2, "champion action index");
    ASSERT_EQ(impact.impactMapIndex, 4, "champion action map");
    ASSERT_EQ(impact.impactMapX, 9, "champion action x");
    ASSERT_EQ(impact.impactMapY, 10, "champion action y");
    ASSERT_EQ(impact.impactCell, 1, "champion action cell");
    ASSERT_EQ(impact.attackTypeCode, COMBAT_ATTACK_FIRE,
              "champion action attack type");
    ASSERT_EQ(impact.rawAttackValue, 77, "champion action raw attack");
    ASSERT_EQ(impact.allowedWounds, COMBAT_WOUND_LEGS,
              "champion action allowed wounds");
    ASSERT_EQ(dm1_v1_projectile_champion_action_plan_pc34(
                  &p, &action, 0, &impact), 1,
              "champion absent action plan builds");
    ASSERT_EQ(impact.handled, 1, "champion absent action handled");
    ASSERT_EQ(impact.championPresent, 0, "champion absent action present");
    action.kind = 0;
    ASSERT_EQ(dm1_v1_projectile_champion_action_plan_pc34(
                  &p, &action, 1, &impact), 0,
              "non-champion action rejected");
    ASSERT_EQ(dm1_v1_projectile_champion_impact_plan_pc34(
                  &p, &r, 1, &impact), 1,
              "champion impact plan rebuilds for poison");

    defender.currentHealth = 100;
    defender.statisticVitality = 64;
    championState.present = 1;
    championState.hp.current = 100;
    rng.seed = 1u;
    impact.rawAttackValue = 40;
    impact.attackTypeCode = COMBAT_ATTACK_NORMAL;
    impact.allowedWounds = COMBAT_WOUND_NONE;
    ASSERT_EQ(dm1_v1_projectile_champion_damage_apply_pc34(
                  &impact, &defender, &rng, &championState, &damagePlan), 1,
              "champion damage apply builds");
    ASSERT_EQ(damagePlan.valid, 1, "champion damage valid");
    ASSERT_EQ(damagePlan.championIndex, 1, "champion damage index");
    ASSERT_EQ(damagePlan.scaledAttack, 40, "champion damage scaled");
    ASSERT_EQ(damagePlan.selectedWounds, 0, "champion damage wounds");
    ASSERT_EQ(damagePlan.killed, 0, "champion damage killed");
    ASSERT_EQ(championState.hp.current, 60, "champion damage hp applied");

    ASSERT_EQ(dm1_v1_projectile_champion_poison_plan_pc34(
                  &impact, &p, 12, 30, 65000, 1, &poison), 1,
              "champion poison plan builds");
    ASSERT_EQ(poison.shouldApply, 1, "champion poison applies");
    ASSERT_EQ(poison.championIndex, 1, "champion poison index");
    ASSERT_EQ(poison.poisonDamage, 2, "champion poison damage");
    ASSERT_EQ(poison.newPoisonDose, 65130, "champion poison dose");
    ASSERT_EQ(poison.nextAttack, 129, "champion poison next attack");
    ASSERT_EQ(poison.scheduleDelayTicks, 36, "champion poison delay");

    championState.hp.current = 30;
    championState.poisonDose = 65000;
    ASSERT_EQ(dm1_v1_projectile_champion_poison_apply_pc34(
                  &impact, &p, 12, 1, 100, 2, 7, 8,
                  &championState, &poisonApply), 1,
              "champion poison apply builds");
    ASSERT_EQ(poisonApply.valid, 1, "champion poison apply valid");
    ASSERT_EQ(poisonApply.shouldApply, 1, "champion poison apply mutates");
    ASSERT_EQ(poisonApply.championIndex, 1, "champion poison apply index");
    ASSERT_EQ(championState.hp.current, 28, "champion poison apply hp");
    ASSERT_EQ(championState.poisonDose, 65130,
              "champion poison apply dose");
    ASSERT_EQ(poisonApply.schedulePoisonEvent, 1,
              "champion poison apply schedules");
    ASSERT_EQ(poisonApply.incrementPoisonEventCount, 1,
              "champion poison apply count");
    ASSERT_EQ(poisonApply.poisonEvent.kind, TIMELINE_EVENT_STATUS_TIMEOUT,
              "champion poison event kind");
    ASSERT_EQ(poisonApply.poisonEvent.fireAtTick, 136,
              "champion poison event tick");
    ASSERT_EQ(poisonApply.poisonEvent.mapIndex, 2,
              "champion poison event map");
    ASSERT_EQ(poisonApply.poisonEvent.mapX, 7,
              "champion poison event x");
    ASSERT_EQ(poisonApply.poisonEvent.mapY, 8,
              "champion poison event y");
    ASSERT_EQ(poisonApply.poisonEvent.aux1, 129,
              "champion poison event attack");
    ASSERT_EQ(poisonApply.poisonEvent.aux4, 1,
              "champion poison event champion");
    {
        int nextCount = -1;
        ASSERT_EQ(dm1_v1_projectile_champion_poison_event_count_after_pc34(
                      &poisonApply, 0, &nextCount), 1,
                  "champion poison event count helper builds");
        ASSERT_EQ(nextCount, 1, "champion poison event count increments");
        ASSERT_EQ(dm1_v1_projectile_champion_poison_event_count_after_pc34(
                      &poisonApply, 255, &nextCount), 1,
                  "champion poison event count helper clamps");
        ASSERT_EQ(nextCount, 255,
                  "champion poison event count saturates at byte max");
    }

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

static void test_explosion_party_damage_plan(void) {
    DM1_ExplosionPartyDamageFanoutPlanPc34 fanout;
    DM1_ExplosionPartyChampionDamagePlanPc34 champion;
    DM1_ExplosionPartyChampionApplyPlanPc34 apply;
    DM1_ExplosionGroupApplyPlanPc34 groupApply;
    struct CombatantChampionSnapshot_Compat defender;
    struct ChampionState_Compat championState;
    struct CombatAction_Compat groupAction;
    struct DungeonGroup_Compat group;
    struct RngState_Compat rng;
    memset(&fanout, 0, sizeof(fanout));
    memset(&champion, 0, sizeof(champion));
    memset(&apply, 0, sizeof(apply));
    memset(&groupApply, 0, sizeof(groupApply));
    memset(&defender, 0, sizeof(defender));
    memset(&championState, 0, sizeof(championState));
    memset(&groupAction, 0, sizeof(groupAction));
    memset(&group, 0, sizeof(group));
    memset(&rng, 0, sizeof(rng));

    ASSERT_EQ(dm1_v1_explosion_party_damage_fanout_plan_pc34(
                  40, COMBAT_ATTACK_FIRE,
                  COMBAT_WOUND_READY_HAND | COMBAT_WOUND_HEAD, &fanout), 1,
              "explosion party fanout builds");
    ASSERT_EQ(fanout.handled, 1, "explosion party fanout handled");
    ASSERT_EQ(fanout.baseAttack, 34, "F0324 base attack");
    ASSERT_EQ(fanout.rngModulus, 12, "F0324 rng modulus");
    ASSERT_EQ(fanout.attackTypeCode, COMBAT_ATTACK_FIRE,
              "F0324 attack type");
    ASSERT_EQ(fanout.allowedWounds,
              COMBAT_WOUND_READY_HAND | COMBAT_WOUND_HEAD,
              "F0324 wounds");

    ASSERT_EQ(dm1_v1_explosion_party_champion_damage_plan_pc34(
                  &fanout, 2, 1, 80, 7, &champion), 1,
              "explosion champion plan builds");
    ASSERT_EQ(champion.shouldAttemptDamage, 1,
              "explosion champion should damage");
    ASSERT_EQ(champion.championIndex, 2, "explosion champion index");
    ASSERT_EQ(champion.randomizedAttack, 41, "explosion randomized attack");
    ASSERT_EQ(champion.attackTypeCode, COMBAT_ATTACK_FIRE,
              "explosion champion attack type");
    ASSERT_EQ(champion.allowedWounds,
              COMBAT_WOUND_READY_HAND | COMBAT_WOUND_HEAD,
              "explosion champion wounds");

    champion.attackTypeCode = COMBAT_ATTACK_NORMAL;
    champion.allowedWounds = COMBAT_WOUND_NONE;
    defender.currentHealth = 100;
    defender.statisticVitality = 64;
    championState.present = 1;
    championState.hp.current = 100;
    rng.seed = 1u;
    ASSERT_EQ(dm1_v1_explosion_party_champion_apply_pc34(
                  &champion, &defender, &rng, &championState, &apply), 1,
              "explosion champion apply builds");
    ASSERT_EQ(apply.valid, 1, "explosion champion apply valid");
    ASSERT_EQ(apply.championIndex, 2, "explosion champion apply index");
    ASSERT_EQ(apply.scaledAttack, 41, "explosion champion apply damage");
    ASSERT_EQ(apply.selectedWounds, 0, "explosion champion apply wounds");
    ASSERT_EQ(apply.killed, 0, "explosion champion apply killed");
    ASSERT_EQ(championState.hp.current, 59, "explosion champion hp applied");

    ASSERT_EQ(dm1_v1_explosion_party_champion_damage_plan_pc34(
                  &fanout, 1, 0, 80, 7, &champion), 1,
              "explosion absent champion plan builds");
    ASSERT_EQ(champion.shouldAttemptDamage, 0,
              "absent champion is skipped");
    ASSERT_EQ(dm1_v1_explosion_party_champion_damage_plan_pc34(
                  &fanout, 1, 1, 0, 7, &champion), 1,
              "explosion dead champion plan builds");
    ASSERT_EQ(champion.shouldAttemptDamage, 0,
              "dead champion is skipped");

    ASSERT_EQ(dm1_v1_explosion_party_damage_fanout_plan_pc34(
                  0, COMBAT_ATTACK_FIRE, COMBAT_WOUND_NONE, &fanout), 1,
              "zero explosion fanout builds");
    ASSERT_EQ(fanout.handled, 0, "zero attack is not handled");

    groupAction.kind = COMBAT_ACTION_APPLY_DAMAGE_GROUP;
    groupAction.rawAttackValue = 15;
    group.count = 1;
    group.cells = (0 << 0) | (1 << 2);
    group.health[0] = 10;
    group.health[1] = 20;
    ASSERT_EQ(dm1_v1_explosion_group_apply_pc34(
                  &groupAction, &group, &groupApply), 1,
              "explosion group apply builds");
    ASSERT_EQ(groupApply.valid, 1, "explosion group apply valid");
    ASSERT_EQ(groupApply.handled, 1, "explosion group handled");
    ASSERT_EQ(groupApply.appliedCount, 2, "explosion group applies twice");
    ASSERT_EQ(groupApply.finalOutcomeCode, COMBAT_OUTCOME_KILLED_NO_CREATURES,
              "explosion group final outcome carried");
    ASSERT_EQ(groupApply.finalGroupCount, 0,
              "explosion group final count carried");
    ASSERT_EQ(groupApply.finalGroupCells, group.cells,
              "explosion group final cells carried");
    ASSERT_EQ(groupApply.finalHealth[0], 5,
              "explosion group final health[0] carried");
    ASSERT_EQ(groupApply.finalHealth[1], 20,
              "explosion group final health[1] carried");
    ASSERT_EQ(groupApply.damage.damageApplied, 15,
              "explosion group damage payload");
    ASSERT_EQ(group.count, 0, "explosion group compacts count");
    ASSERT_EQ(group.health[0], 5, "explosion group shifted health");
    ASSERT_EQ(group.health[1], 20, "explosion group preserves tail health");
}

int main(void) {
    test_throw_weight_and_stamina();
    test_throw_runtime_math();
    test_projectile_shapes_and_launch();
    test_shoot_runtime_math();
    test_spell_projectile_f0412_to_f0327_create_input();
    test_projectile_create_input_model();
    test_projectile_impact_model();
    test_projectile_group_slot_materialization_plan();
    test_projectile_associated_thing_disposition();
    test_projectile_materialization_plan();
    test_projectile_flight_relink_receipt();
    test_black_flame_heal_and_group_cell();
    test_projectile_creature_impact_plan();
    test_projectile_champion_impact_plan();
    test_explosion_party_damage_plan();
    if (g_failures) {
        fprintf(stderr, "test_dm1_v1_throw_shoot_pc34_compat: %d failures\n",
                g_failures);
        return 1;
    }
    printf("test_dm1_v1_throw_shoot_pc34_compat: PASS\n");
    return 0;
}

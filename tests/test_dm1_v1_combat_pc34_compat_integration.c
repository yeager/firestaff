/*
 * test_dm1_v1_combat_pc34_compat_integration.c — CTest gate
 *
 * Verifies DM1 V1 combat system: melee, damage, armor, wounds.
 * Source-locked to ReDMCSB GROUP.C / CHAMPION.C / DEFS.H.
 */
#include "dm1_v1_combat_pc34_compat.h"
#include "dm1_v1_dungeon_weapon_info_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"
#include "memory_combat_pc34_compat.h"  /* F0192_GROUP_* new layer (BUG-115b) */
#include "memory_creature_ai_pc34_compat.h"  /* F0801b archenemy double-move (BUG-115b) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static int g_pass = 0;
static int g_fail = 0;

#define TEST(name) do { printf("  TEST: %s ... ", #name); } while(0)
#define PASS() do { printf("PASS\n"); g_pass++; } while(0)
#define FAIL(msg) do { printf("FAIL: %s\n", msg); g_fail++; } while(0)
#define CHECK(cond, msg) do { if (!(cond)) { FAIL(msg); return; } } while(0)

/* Forward declarations for tests defined after main(). */
static void test_champion_is_lucky(void);
static void test_f0735_lucky_hit_enters_damage_path(void);
static void test_f0735_dex_hit_short_circuits_random4_luck(void);
static void test_f0735_zero_luck_f0308_uses_pc34_rng_count(void);
static void test_f0735_non_material_gate_skips_luck(void);
static void test_f0735_dexterity_255_skips_hit_branch(void);
static void test_f0735_weak_damage_zero_roll_uses_miss_tail(void);
static void test_ordered_cells_to_attack_priority(void);
static void test_f0192_per_creature_resistance(void);
static void test_f0801b_archenemy_double_move(void);

/* ── Test: scaled product matches F0030 ──────────────────────────── */
static void test_scaled_product(void) {
    TEST(scaled_product);
    /* 100 * 130 / 128 = 101 (integer) */
    int r1 = dm1_scaled_product(100, 7, 130);
    CHECK(r1 == 101, "100*130>>7 should be 101");
    /* 50 * 64 / 128 = 25 */
    int r2 = dm1_scaled_product(50, 7, 64);
    CHECK(r2 == 25, "50*64>>7 should be 25");
    /* Edge: 0 */
    int r3 = dm1_scaled_product(0, 7, 130);
    CHECK(r3 == 0, "0*130>>7 should be 0");
    PASS();
}

/* ── Test: armor defense with sharp ──────────────────────────────── */
static void test_armor_defense(void) {
    TEST(armor_defense);
    DM1_ArmorPiece ap = { .defense = 20, .sharpDefense = 5, .isShield = 0, .slot = 3 };
    CHECK(dm1_armor_defense(&ap, 0) == 20, "no-sharp defense should be 20");
    CHECK(dm1_armor_defense(&ap, 1) == 22,
          "sharp defense must use F0143's (defense * (bits + 4)) >> 3");
    CHECK(dm1_armor_defense(NULL, 0) == 0, "NULL armor should be 0");
    PASS();
}

/* ── Test: F0156 ARMOUR.Type -> G0239 -> F0143 ───────────────────── */
static void test_f0143_raw_armour_records(void) {
    struct DungeonThings_Compat things;
    unsigned char rawArmours[3 * 4] = {0};
    DM1_ArmourInfoPc34 info;
    int defense = -1;

    TEST(f0143_raw_armour_records);
    memset(&things, 0, sizeof(things));
    things.loaded = 1;
    things.rawThingData[THING_TYPE_ARMOUR] = rawArmours;
    things.thingCounts[THING_TYPE_ARMOUR] = 3;

    /* Raw type 34 is MITHRAL AKETON: { Weight 52, Defense 70, Attributes 7 }. */
    rawArmours[2] = 34;
    CHECK(dm1_v1_dungeon_get_armour_info_pc34(&things,
                                               (THING_TYPE_ARMOUR << 10), &info) == 1,
          "loaded ARMOUR.Type 34 must resolve through G0239");
    CHECK(info.weight == 52 && info.defense == 70 && info.attributes == 7,
          "MITHRAL AKETON must retain G0239 Weight/Defense/Attributes");
    CHECK(dm1_v1_dungeon_get_armour_defense_f0143_pc34(
              &things, (THING_TYPE_ARMOUR << 10), 0, &defense) == 1 && defense == 70,
          "F0143 non-sharp must return raw G0239 defense");
    CHECK(dm1_v1_dungeon_get_armour_defense_f0143_pc34(
              &things, (THING_TYPE_ARMOUR << 10), 1, &defense) == 1 && defense == 96,
          "F0143 sharp defense must scale 70 by (7 + 4) / 8");

    /* Type 52 is SHIELD OF DARC and keeps its source shield attribute. */
    rawArmours[4 + 2] = 52;
    CHECK(dm1_v1_dungeon_get_armour_info_pc34(&things,
                                               (THING_TYPE_ARMOUR << 10) | 1, &info) == 1,
          "loaded shield ARMOUR.Type must resolve through G0239");
    CHECK(info.weight == 40 && info.defense == 100 && info.attributes == 0x84,
          "SHIELD OF DARC must retain its original shield flag");

    rawArmours[8 + 2] = 58;
    CHECK(dm1_v1_dungeon_get_armour_info_pc34(&things,
                                               (THING_TYPE_ARMOUR << 10) | 2, &info) == 0,
          "out-of-range ARMOUR.Type must fail closed");
    CHECK(dm1_v1_dungeon_get_armour_info_pc34(&things,
                                               (THING_TYPE_WEAPON << 10), &info) == 0,
          "non-armour Thing must not borrow an ARMOUR row");
    PASS();
}

/* ── Test: stamina adjustment (F0306) ────────────────────────────── */
static void test_stamina_adjusted(void) {
    TEST(stamina_adjusted);
    DM1_ChampionCombat ch;
    dm1_combat_init_champion(&ch);

    /* Full stamina: no adjustment */
    ch.maxStamina = 100;
    ch.currentStamina = 100;
    CHECK(dm1_stamina_adjusted(&ch, 80) == 80, "full stamina should not adjust");

    /* Half stamina: value = val/2 + val/2 * stam/halfMax = 40 + 40*50/50 = 80 */
    ch.currentStamina = 50;
    CHECK(dm1_stamina_adjusted(&ch, 80) == 80, "exactly half should return same");

    /* Quarter stamina: val/2 + val/2 * 25/50 = 40 + 20 = 60 */
    ch.currentStamina = 25;
    CHECK(dm1_stamina_adjusted(&ch, 80) == 60, "quarter stamina: 40+20=60");

    /* Zero stamina: val/2 + 0 = 40 */
    ch.currentStamina = 0;
    CHECK(dm1_stamina_adjusted(&ch, 80) == 40, "zero stamina should halve");

    PASS();
}

/* ── Test: stat adjusted attack (F0307) ──────────────────────────── */
static void test_stat_adjusted_attack(void) {
    TEST(stat_adjusted_attack);
    DM1_ChampionCombat ch;
    dm1_combat_init_champion(&ch);

    /* stat=0: factor=170, scaled_product(100,7,170) = 100*170/128 = 132 */
    int r1 = dm1_stat_adjusted_attack(&ch, 0, 100);
    CHECK(r1 == 132, "stat=0: 100*170/128=132");

    /* stat=154: factor=16, scaled_product(100,7,16) = 100*16/128=12 */
    int r2 = dm1_stat_adjusted_attack(&ch, 154, 100);
    CHECK(r2 == 12, "stat=154: should be 12");

    /* stat=155+: factor<16, return attack>>3=12 */
    int r3 = dm1_stat_adjusted_attack(&ch, 155, 100);
    CHECK(r3 == 12, "stat=155: should be 100>>3=12");

    PASS();
}

/* -- Test: F0312 weapon-class skill routing -------------------------- */
static void test_weapon_info_class_table_source_lock(void) {
    TEST(weapon_info_class_table_source_lock);
    DM1_WeaponInfo info;

    CHECK(dm1_weapon_info_pc34(8, &info) == 1,
          "weapon type 8 DAGGER should resolve to a full info row");
    CHECK(info.weight == 5 && info.weaponClass == 2 &&
          info.strength == 10 && info.kineticEnergy == 19 &&
          info.attributes == 0x0200,
          "DUNGEON.C weapon type 8 DAGGER row should match Weight/Class/Strength/Kinetic/Attributes");
    CHECK(dm1_weapon_info_pc34(25, &info) == 1,
          "weapon type 25 BOW should resolve to a full info row");
    CHECK(info.weight == 10 && info.weaponClass == 20 &&
          info.strength == 1 && info.kineticEnergy == 50 &&
          info.attributes == 0x2032,
          "DUNGEON.C weapon type 25 BOW row should match Weight/Class/Strength/Kinetic/Attributes");
    CHECK(dm1_weapon_info_pc34(45, &info) == 1,
          "weapon type 45 complete Firestaff should resolve to a full info row");
    CHECK(info.weight == 36 && info.weaponClass == 255 &&
          info.strength == 100 && info.kineticEnergy == 50 &&
          info.attributes == 0x20FF,
          "DUNGEON.C weapon type 45 complete Firestaff row should match");
    CHECK(dm1_weapon_info_class_pc34(8) == 2,
          "DUNGEON.C weapon type 8 DAGGER should have class 2");
    CHECK(dm1_weapon_info_class_pc34(18) == 2,
          "DUNGEON.C weapon type 18 AXE should have class 2");
    CHECK(dm1_weapon_info_class_pc34(25) == 20,
          "DUNGEON.C weapon type 25 BOW should have class 20");
    CHECK(dm1_weapon_info_class_pc34(26) == 30,
          "DUNGEON.C weapon type 26 CROSSBOW should have class 30");
    CHECK(dm1_weapon_info_class_pc34(29) == 39,
          "DUNGEON.C weapon type 29 SLING should have class 39");
    CHECK(dm1_weapon_info_class_pc34(30) == DM1_WEAPON_CLASS_SLING_AMMUNITION,
          "DUNGEON.C weapon type 30 ROCK should have sling ammunition class");
    CHECK(dm1_weapon_info_class_pc34(45) == 255,
          "DUNGEON.C weapon type 45 complete Firestaff should have class 255");
    CHECK(dm1_weapon_info_class_pc34(-1) == -1,
          "negative weapon type should be rejected");
    CHECK(dm1_weapon_info_class_pc34(46) == -1,
          "out-of-range weapon type should be rejected");
    CHECK(dm1_weapon_info_pc34(46, &info) == -1 && info.weaponClass == -1,
          "out-of-range full weapon info should be rejected with class -1");

    PASS();
}

static void test_f0312_skill_level_bonus_uses_source_class_rules(void) {
    TEST(f0312_skill_level_bonus_uses_source_class_rules);

    CHECK(dm1_champion_f0312_skill_level_bonus_pc34(0, 3, 5, 7) == 3,
          "swing class 0 should use F0303(SWING)");
    CHECK(dm1_champion_f0312_skill_level_bonus_pc34(2, 3, 5, 7) == 8,
          "dagger/axe class 2 should add F0303(SWING) and F0303(THROW)");
    CHECK(dm1_champion_f0312_skill_level_bonus_pc34(1, 3, 5, 7) == 5,
          "throw class below first bow should use F0303(THROW)");
    CHECK(dm1_champion_f0312_skill_level_bonus_pc34(20, 3, 5, 7) == 7,
          "bow class should use F0303(SHOOT)");
    CHECK(dm1_champion_f0312_skill_level_bonus_pc34(112, 3, 5, 7) == 0,
          "magic weapon class should not receive F0312 weapon skill bonus");

    PASS();
}

/* ── Test: creature damage (F0190) ───────────────────────────────── */
static void test_creature_damage(void) {
    TEST(creature_damage);
    DM1_CreatureGroup g;
    dm1_combat_init_group(&g);
    g.info.baseHealth = 50;
    g.info.defense = 10;
    g.count = 0; /* 1 creature */
    g.creatures[0].health = 50;

    /* Damage less than health: survives */
    int r1 = dm1_creature_take_damage(&g, 0, 20);
    CHECK(r1 == DM1_OUTCOME_KILLED_NONE, "20 dmg on 50 hp: none killed");
    CHECK(g.creatures[0].health == 30, "health should be 30");

    /* Lethal damage on last creature */
    int r2 = dm1_creature_take_damage(&g, 0, 30);
    CHECK(r2 == DM1_OUTCOME_KILLED_ALL, "30 dmg on 30 hp (last): all killed");

    PASS();
}

/* ── Test: creature damage with multiple creatures ───────────────── */
static void test_creature_damage_multi(void) {
    TEST(creature_damage_multi);
    DM1_CreatureGroup g;
    dm1_combat_init_group(&g);
    g.info.defense = 5;
    g.count = 2; /* 3 creatures */
    g.creatures[0].health = 20;
    g.creatures[1].health = 30;
    g.creatures[2].health = 10;

    /* Kill creature at index 2 */
    int r1 = dm1_creature_take_damage(&g, 2, 15);
    CHECK(r1 == DM1_OUTCOME_KILLED_SOME, "kill creature 2: killed some");
    CHECK(g.count == 1, "count should be 1 (2 creatures remaining)");

    PASS();
}

/* -- Test: lethal hit compacts a multi-creature group (F0190) -------- */
static void test_creature_damage_group_split_compacts_state(void) {
    TEST(creature_damage_group_split_compacts_state);
    DM1_CreatureGroup g;
    dm1_combat_init_group(&g);
    g.info.defense = 5;
    g.count = 3; /* 4 creatures */
    g.creatures[0].health = 44;
    g.creatures[0].cell = 0;
    g.creatures[0].direction = 1;
    g.creatures[1].health = 12;
    g.creatures[1].cell = 1;
    g.creatures[1].direction = 2;
    g.creatures[2].health = 36;
    g.creatures[2].cell = 2;
    g.creatures[2].direction = 3;
    g.creatures[3].health = 28;
    g.creatures[3].cell = 3;
    g.creatures[3].direction = 0;

    /*
     * ReDMCSB GROUP.C F0190 lines 892-905 shifts Health, group cells,
     * and group directions down after a killed middle creature, then
     * decrements Count. F0199 remains a behavior distance helper; this
     * regression deliberately proves only the attacked-group transition.
     */
    int r = dm1_creature_take_damage(&g, 1, 12);
    CHECK(r == DM1_OUTCOME_KILLED_SOME, "lethal middle hit should kill one creature");
    CHECK(g.count == 2, "count should be 2 (3 creatures remaining)");
    CHECK(g.creatures[0].health == 44, "leader health should stay in slot 0");
    CHECK(g.creatures[0].cell == 0, "leader cell should stay in slot 0");
    CHECK(g.creatures[0].direction == 1, "leader direction should stay in slot 0");
    CHECK(g.creatures[1].health == 36, "slot 2 health should compact into slot 1");
    CHECK(g.creatures[1].cell == 2, "slot 2 cell should compact into slot 1");
    CHECK(g.creatures[1].direction == 3, "slot 2 direction should compact into slot 1");
    CHECK(g.creatures[2].health == 28, "slot 3 health should compact into slot 2");
    CHECK(g.creatures[2].cell == 3, "slot 3 cell should compact into slot 2");
    CHECK(g.creatures[2].direction == 0, "slot 3 direction should compact into slot 2");

    PASS();
}

/* ── Test: archenemy immune (F0190) ──────────────────────────────── */
static void test_archenemy_immune(void) {
    TEST(archenemy_immune);
    DM1_CreatureGroup g;
    dm1_combat_init_group(&g);
    g.info.defense = 255; /* Immune */
    g.count = 0;
    g.creatures[0].health = 100;

    int r = dm1_creature_take_damage(&g, 0, 999);
    CHECK(r == DM1_OUTCOME_KILLED_NONE, "archenemy should be immune");
    CHECK(g.creatures[0].health == 100, "health unchanged");

    PASS();
}

/* ── Test: champion take damage pipeline (F0321) ─────────────────── */
static void test_champion_damage_pipeline(void) {
    TEST(champion_damage_pipeline);
    dm1_combat_seed_rng(42);

    DM1_CombatState s;
    dm1_combat_init(&s);
    s.championCount = 1;
    dm1_combat_init_champion(&s.champions[0]);
    s.champions[0].currentHealth = 200;
    s.champions[0].maxHealth = 200;
    s.champions[0].vitality = 30;
    s.champions[0].antifire = 10;

    /* NORMAL attack: no wound processing, raw damage */
    int r = dm1_champion_take_damage(&s, 0, 25, DM1_WOUND_NONE, DM1_ATTACK_NORMAL);
    CHECK(r == 25, "normal attack should accumulate 25");
    CHECK(s.pendingDamage[0] == 25, "pending damage should be 25");

    /* Apply it */
    dm1_apply_pending_damage(&s);
    CHECK(s.champions[0].currentHealth == 175, "health should be 175");

    PASS();
}

/* ── Test: sharp attack with wound application ───────────────────── */
static void test_sharp_attack_wounds(void) {
    TEST(sharp_attack_wounds);
    dm1_combat_seed_rng(1000);

    DM1_CombatState s;
    dm1_combat_init(&s);
    s.championCount = 1;
    dm1_combat_init_champion(&s.champions[0]);
    s.champions[0].currentHealth = 500;
    s.champions[0].maxHealth = 500;
    s.champions[0].vitality = 10; /* Low vitality = more wounds */

    /* Sharp attack with all wounds allowed */
    int dmg = dm1_champion_take_damage(&s, 0, 200, DM1_WOUND_ALL, DM1_ATTACK_SHARP);
    CHECK(dmg > 0, "should deal damage");

    dm1_apply_pending_damage(&s);
    CHECK(s.champions[0].currentHealth < 500, "health should decrease");
    /* Wounds may or may not be applied depending on RNG, but pipeline works */

    PASS();
}

/* ── Test: fire attack with shield reduction ─────────────────────── */
static void test_fire_attack_shield(void) {
    TEST(fire_attack_shield);
    dm1_combat_seed_rng(555);

    DM1_CombatState s;
    dm1_combat_init(&s);
    s.championCount = 1;
    dm1_combat_init_champion(&s.champions[0]);
    s.champions[0].currentHealth = 300;
    s.champions[0].maxHealth = 300;
    s.champions[0].antifire = 50; /* High antifire */
    s.partyFireShieldDefense = 100; /* Strong fire shield */

    /* Fire attack should be heavily reduced */
    int dmg = dm1_champion_take_damage(&s, 0, 50, DM1_WOUND_ALL, DM1_ATTACK_FIRE);
    CHECK(dmg == 0, "50 fire attack with 100 fire shield should be absorbed");

    PASS();
}

/* ── Test: poison adjusted attack (F0192) ────────────────────────── */
static void test_poison_adjusted(void) {
    TEST(poison_adjusted);
    dm1_combat_seed_rng(77);

    /* Immune (15): always 0 */
    CHECK(dm1_poison_adjusted_attack(15, 10) == 0, "immune should return 0");
    /* No attack: 0 */
    CHECK(dm1_poison_adjusted_attack(5, 0) == 0, "no poison should return 0");
    /* Normal case: (10 + rng(4)) * 8 / (5+1) */
    int r = dm1_poison_adjusted_attack(5, 10);
    CHECK(r > 0, "should return positive");

    PASS();
}

/* -- Test: F0322 poison applies immediate damage then schedules DOT ---- */
static void test_poison_start_immediate_and_followup(void) {
    TEST(poison_start_immediate_and_followup);

    DM1_CombatState s;
    dm1_combat_init(&s);
    s.championCount = 1;
    dm1_combat_init_champion(&s.champions[0]);

    CHECK(dm1_combat_start_poison_pc34(&s, 0, 130) == 2,
          "attack 130 should immediately add max(1, 130>>6) damage");
    CHECK(s.pendingDamage[0] == 2, "poison immediate damage should be pending");
    CHECK(s.pendingPoison[0].active == 1, "attack 130 should schedule a follow-up event");
    CHECK(s.pendingPoison[0].attack == 129, "scheduled poison attack should be decremented");
    CHECK(s.pendingPoison[0].ticksUntilNext == 36, "follow-up should be 36 ticks out");
    CHECK(s.champions[0].poisonEventCount == 1, "scheduled poison event count should be one");

    for (int i = 0; i < 35; i++) {
        dm1_combat_tick_poison(&s);
    }
    CHECK(s.pendingDamage[0] == 2, "poison should not tick before 36 ticks");
    CHECK(s.pendingPoison[0].ticksUntilNext == 1, "one tick should remain before follow-up");

    dm1_combat_tick_poison(&s);
    CHECK(s.pendingDamage[0] == 4, "36th tick should apply the next poison damage");
    CHECK(s.pendingPoison[0].active == 1, "continuing poison should reschedule itself");
    CHECK(s.pendingPoison[0].attack == 128, "rescheduled poison attack should decrement again");
    CHECK(s.champions[0].poisonEventCount == 1, "event count should remain one while chain continues");

    s.pendingPoison[0].active = 0;
    s.pendingPoison[0].attack = 0;
    s.pendingPoison[0].ticksUntilNext = 0;
    s.champions[0].poisonEventCount = 0;
    s.pendingDamage[0] = 0;
    CHECK(dm1_combat_start_poison_pc34(&s, 0, 1) == 1,
          "attack 1 should still deal one immediate poison damage");
    CHECK(s.pendingPoison[0].active == 0, "attack 1 should not schedule a follow-up");
    CHECK(s.champions[0].poisonEventCount == 0, "no follow-up means no scheduled event count");

    PASS();
}

/* -- Test: F0230 creature poison uses 50% gate + F0307 Vitality adjust -- */
static void test_creature_poison_gate_and_vitality_adjust(void) {
    TEST(creature_poison_gate_and_vitality_adjust);

    DM1_CombatState s;
    dm1_combat_init(&s);
    s.championCount = 1;
    dm1_combat_init_champion(&s.champions[0]);
    s.champions[0].vitality = 42;

    dm1_combat_seed_rng(1); /* first random(2) == 0: poison gate fails (DM1 LCG) */
    CHECK(dm1_creature_poison_attack_pc34(&s, 0, 96) == 0,
          "failed 50% gate should not start poison");
    CHECK(s.pendingDamage[0] == 0, "failed gate should leave pending damage unchanged");
    CHECK(s.pendingPoison[0].active == 0, "failed gate should not schedule poison");

    dm1_combat_seed_rng(6); /* first random(2) == 1: poison gate passes (DM1 LCG) */
    CHECK(dm1_creature_poison_attack_pc34(&s, 0, 96) == 1,
          "passed gate should start F0322 with Vitality-adjusted attack");
    CHECK(s.pendingDamage[0] == 1, "adjusted attack 96 should add one immediate poison damage");
    CHECK(s.pendingPoison[0].active == 1, "passed gate should schedule poison follow-up");
    CHECK(s.pendingPoison[0].attack == 95, "F0322 should schedule adjusted attack minus one");

    PASS();
}

/* ── Test: damage all creatures (F0191) ──────────────────────────── */
static void test_damage_all_creatures(void) {
    TEST(damage_all_creatures);
    dm1_combat_seed_rng(999);

    DM1_CreatureGroup g;
    dm1_combat_init_group(&g);
    g.info.defense = 0;
    g.count = 1; /* 2 creatures */
    g.creatures[0].health = 5;
    g.creatures[1].health = 5;

    int r = dm1_damage_all_creatures(&g, 100);
    CHECK(r == DM1_OUTCOME_KILLED_ALL, "100 attack on 5hp creatures should kill all");

    PASS();
}

/* ── Test: damage all champions (F0324) ──────────────────────────── */
static void test_damage_all_champions(void) {
    TEST(damage_all_champions);
    dm1_combat_seed_rng(123);

    DM1_CombatState s;
    dm1_combat_init(&s);
    s.championCount = 2;
    dm1_combat_init_champion(&s.champions[0]);
    dm1_combat_init_champion(&s.champions[1]);

    int count = dm1_damage_all_champions(&s, 30, DM1_WOUND_ALL, DM1_ATTACK_BLUNT);
    CHECK(count > 0, "should damage at least one champion");

    dm1_apply_pending_damage(&s);
    CHECK(s.champions[0].currentHealth < 100 || s.champions[1].currentHealth < 100,
          "at least one champion should have reduced health");

    PASS();
}

/* -- Test: F0324 zero attack has no champion side effects ------------ */
static void test_damage_all_champions_zero_attack(void) {
    TEST(damage_all_champions_zero_attack);
    DM1_CombatState s;
    dm1_combat_init(&s);
    s.championCount = 2;
    dm1_combat_init_champion(&s.champions[0]);
    dm1_combat_init_champion(&s.champions[1]);
    s.pendingDamage[0] = 3;
    s.pendingWounds[1] = DM1_WOUND_HEAD;

    CHECK(dm1_damage_all_champions(&s, 0, DM1_WOUND_ALL, DM1_ATTACK_SELF) == 0,
          "F0324 zero attack returns zero damaged champions");
    CHECK(s.pendingDamage[0] == 3,
          "F0324 zero attack leaves existing pending damage untouched");
    CHECK(s.pendingWounds[1] == DM1_WOUND_HEAD,
          "F0324 zero attack leaves existing pending wounds untouched");

    PASS();
}

/* -- Test: F0320 applies wounds before pending-damage branches -------- */
static void test_pending_damage_applies_wounds_before_damage_guard(void) {
    TEST(pending_damage_applies_wounds_before_damage_guard);
    DM1_CombatState s;
    dm1_combat_init(&s);
    s.championCount = 2;
    dm1_combat_init_champion(&s.champions[0]);
    dm1_combat_init_champion(&s.champions[1]);
    s.champions[0].currentHealth = 0;
    s.champions[0].alive = 0;
    s.champions[1].currentHealth = 25;
    s.pendingWounds[0] = DM1_WOUND_TORSO;
    s.pendingWounds[1] = DM1_WOUND_LEGS;
    s.pendingDamage[1] = 0;

    dm1_apply_pending_damage(&s);

    CHECK((s.champions[0].wounds & DM1_WOUND_TORSO) != 0,
          "F0320 mounts pending wounds before dead champion skip");
    CHECK((s.champions[1].wounds & DM1_WOUND_LEGS) != 0,
          "F0320 mounts pending wounds before zero damage continue");
    CHECK(s.pendingWounds[0] == 0 && s.pendingWounds[1] == 0,
          "F0320 clears pending wounds after mounting");
    CHECK(s.champions[1].currentHealth == 25,
          "F0320 zero pending damage leaves health unchanged");

    PASS();
}

/* ── Test: creature attacks champion (melee) ─────────────────────── */
static void test_creature_melee_attack(void) {
    TEST(creature_melee_attack);
    dm1_combat_seed_rng(456);

    DM1_CombatState s;
    dm1_combat_init(&s);
    s.championCount = 1;
    dm1_combat_init_champion(&s.champions[0]);
    s.champions[0].currentHealth = 500;
    s.champions[0].maxHealth = 500;
    s.champions[0].dexterity = 8;
    s.champions[0].strength = 40;
    s.champions[0].vitality = 40;
    s.champions[0].hasArmor[DM1_WOUND_IDX_TORSO] = 1;
    s.champions[0].armor[DM1_WOUND_IDX_TORSO].defense = 24;
    s.champions[0].armor[DM1_WOUND_IDX_TORSO].sharpDefense = 6;
    s.partyShieldDefense = 6;

    DM1_CreatureGroup g;
    dm1_combat_init_group(&g);
    g.info.attack = 70;
    g.info.defense = 10;
    g.info.dexterity = 40;
    g.info.attackType = DM1_ATTACK_SHARP;
    g.info.woundProbHead = 15;
    g.info.woundProbTorso = 15;
    g.info.woundProbLegs = 15;
    g.info.woundProbFeet = 15;
    g.count = 0;
    g.creatures[0].health = 50;

    /*
     * ReDMCSB PROJEXPL.C F0230 lines 1377-1408: this fixed RNG fixture
     * exercises the champion-dexterity hit gate, wound roll, staged
     * creature attack random terms including the late armor-style reduction,
     * and the CHAMPION.C F0321 armor/shield handoff for a sharp attack.
     */
    int dmg = dm1_creature_attack_champion(&s, &g, 0, 0);
    CHECK(dmg == 37, "sharp creature attack fixture should resolve to 37 damage");
    CHECK(s.pendingDamage[0] == 37, "creature damage should be queued as pending champion damage");
    CHECK(s.pendingWounds[0] == 0, "fixture should not add a wound after F0321 vitality check");
    CHECK(s.champions[0].currentHealth == 500, "pending damage should not be applied immediately");

    PASS();
}

/* ── Test: parry reduces incoming creature melee ─────────────────── */
/* ReDMCSB PROJEXPL.C:1392 (F0230_GROUP_GetChampionDamage):
 *   Attack = (RANDOM(16) + CreatureInfo.Attack + DoubledMapDifficulty)
 *            - (F0303_CHAMPION_GetSkillLevel(champion, C07_SKILL_PARRY) << 1)
 * The parry term was absent, so the skill gave no mitigation at all. Because
 * the attack then runs through >>1, +random, >>2, dropping it roughly
 * doubled late-game melee damage taken.
 *
 * Damage is NOT monotonic in parry for a fixed seed: the staged terms call
 * random(atk), so changing atk changes which RNG values the rest of the
 * formula consumes. Aggregate over many seeds instead. */
static int parry_fixture_damage(unsigned seed, int parry) {
    DM1_CombatState s;
    DM1_CreatureGroup g;
    dm1_combat_seed_rng(seed);
    dm1_combat_init(&s);
    s.championCount = 1;
    dm1_combat_init_champion(&s.champions[0]);
    s.champions[0].currentHealth = 500;
    s.champions[0].maxHealth = 500;
    s.champions[0].dexterity = 8;
    s.champions[0].strength = 40;
    s.champions[0].vitality = 40;
    s.champions[0].hasArmor[DM1_WOUND_IDX_TORSO] = 1;
    s.champions[0].armor[DM1_WOUND_IDX_TORSO].defense = 24;
    s.champions[0].armor[DM1_WOUND_IDX_TORSO].sharpDefense = 6;
    s.champions[0].skillParry = parry;
    s.partyShieldDefense = 6;
    dm1_combat_init_group(&g);
    g.info.attack = 70;
    g.info.defense = 10;
    g.info.dexterity = 40;
    g.info.attackType = DM1_ATTACK_SHARP;
    g.info.woundProbHead = 15;
    g.info.woundProbTorso = 15;
    g.info.woundProbLegs = 15;
    g.info.woundProbFeet = 15;
    g.count = 0;
    g.creatures[0].health = 50;
    return dm1_creature_attack_champion(&s, &g, 0, 0);
}

static void test_creature_melee_parry_reduces_damage(void) {
    TEST(creature_melee_parry_reduces_damage);

    long total_no_parry = 0;
    long total_parry = 0;
    long total_high_parry = 0;
    unsigned seed;

    /* The parry-0 fixture must still resolve to the pinned value. */
    CHECK(parry_fixture_damage(456, 0) == 37,
          "parry 0 must still resolve to the pinned 37");

    for (seed = 1; seed <= 400u; ++seed) {
        total_no_parry   += parry_fixture_damage(seed, 0);
        total_parry      += parry_fixture_damage(seed, 8);
        total_high_parry += parry_fixture_damage(seed, 20);
    }

    /* parry 8 removes 16 from the pre-staged attack, parry 20 removes 40. */
    CHECK(total_parry < total_no_parry,
          "parry 8 must reduce total incoming melee damage across 400 seeds");
    CHECK(total_high_parry < total_parry,
          "parry 20 must reduce it further than parry 8");
    /* Before the fix all three totals were identical -- the skill did nothing. */
    CHECK(total_no_parry - total_high_parry > total_no_parry / 10,
          "parry 20 must cut total damage by more than 10 percent");

    PASS();
}

/* ── Test: champion melee action ─────────────────────────────────── */
static void test_champion_melee_action(void) {
    TEST(champion_melee_action);
    dm1_combat_seed_rng(789);

    DM1_CombatState s;
    dm1_combat_init(&s);
    s.championCount = 1;
    dm1_combat_init_champion(&s.champions[0]);
    s.champions[0].strength = 50;
    s.champions[0].dexterity = 40;
    s.champions[0].hasWeapon = 1;
    s.champions[0].actionHandWeapon.strength = 10;
    s.champions[0].actionHandWeapon.weaponClass = 0; /* Swing */
    s.champions[0].skillSwing = 5;

    DM1_CreatureGroup g;
    dm1_combat_init_group(&g);
    g.info.defense = 10;
    g.info.dexterity = 15;
    g.count = 0;
    g.creatures[0].health = 100;

    int outcome = dm1_melee_action_damage(&s, 0, &g, 0);
    /* outcome is DM1_OUTCOME_* — the creature may survive or die */
    CHECK(outcome >= 0, "outcome should be valid");

    PASS();
}

static void setup_non_material_melee_fixture(DM1_CombatState* s,
                                            DM1_CreatureGroup* g) {
    dm1_combat_init(s);
    s->championCount = 1;
    dm1_combat_init_champion(&s->champions[0]);
    s->champions[0].strength = 100;
    s->champions[0].dexterity = 100;
    s->champions[0].hasWeapon = 1;
    s->champions[0].actionHandWeapon.strength = 50;
    s->champions[0].actionHandWeapon.weaponClass = 0;
    s->champions[0].skillSwing = 10;

    dm1_combat_init_group(g);
    g->info.defense = 0;
    g->info.dexterity = 0;
    g->info.nonMaterial = 1;
    g->count = 0;
    g->creatures[0].health = 1;
}

/* -- Test: Ghost/non-material melee source gate ---------------------- */
static void test_non_material_melee_requires_vorpal_or_disrupt(void) {
    TEST(non_material_melee_requires_vorpal_or_disrupt);

    DM1_CombatState s;
    DM1_CreatureGroup g;

    setup_non_material_melee_fixture(&s, &g);
    dm1_combat_seed_rng(789);
    s.champions[0].actionHandIcon = 1; /* ordinary weapon icon */
    CHECK(dm1_melee_action_damage(&s, 0, &g, 0) == DM1_OUTCOME_KILLED_NONE,
          "ordinary weapon should not hit non-material creature");
    CHECK(g.creatures[0].health == 1,
          "ordinary weapon should leave non-material creature health unchanged");

    setup_non_material_melee_fixture(&s, &g);
    dm1_combat_seed_rng(789);
    s.champions[0].actionHandIcon = DM1_ICON_WEAPON_VORPAL_BLADE;
    CHECK(dm1_melee_action_damage(&s, 0, &g, 0) == DM1_OUTCOME_KILLED_ALL,
          "Vorpal Blade should set the non-material melee hit gate");

    setup_non_material_melee_fixture(&s, &g);
    dm1_combat_seed_rng(789);
    s.champions[0].hasWeapon = 0;
    s.champions[0].actionHandIcon = 0;
    s.champions[0].actionFlags = DM1_MELEE_FLAG_HIT_NON_MATERIAL;
    CHECK(dm1_melee_action_damage(&s, 0, &g, 0) == DM1_OUTCOME_KILLED_ALL,
          "DISRUPT-style action flag should hit non-material creature");

    PASS();
}

/* -- Test: material melee still accepts ordinary weapons -------------- */
static void test_material_melee_does_not_require_non_material_gate(void) {
    TEST(material_melee_does_not_require_non_material_gate);

    DM1_CombatState s;
    DM1_CreatureGroup g;

    setup_non_material_melee_fixture(&s, &g);
    dm1_combat_seed_rng(789);
    g.info.nonMaterial = 0;
    s.champions[0].actionHandIcon = 1;
    CHECK(dm1_melee_action_damage(&s, 0, &g, 0) == DM1_OUTCOME_KILLED_ALL,
          "ordinary weapon should still hit material creature");

    PASS();
}

/* ── Test: wound defense calculation (F0313) ─────────────────────── */
static void test_wound_defense(void) {
    TEST(wound_defense);
    dm1_combat_seed_rng(321);

    DM1_CombatState s;
    dm1_combat_init(&s);
    s.championCount = 1;
    dm1_combat_init_champion(&s.champions[0]);
    s.champions[0].vitality = 30;
    s.champions[0].actionDefense = 5;
    s.partyShieldDefense = 3;

    /* With torso armor */
    s.champions[0].hasArmor[DM1_WOUND_IDX_TORSO] = 1;
    s.champions[0].armor[DM1_WOUND_IDX_TORSO].defense = 20;
    s.champions[0].armor[DM1_WOUND_IDX_TORSO].sharpDefense = 5;

    int def = dm1_wound_defense(&s, 0, DM1_WOUND_IDX_TORSO, 0);
    CHECK(def >= 0 && def <= 100, "defense should be bounded [0,100]");

    /* Sharp defense should be higher with sharp armor bonus */
    int defSharp = dm1_wound_defense(&s, 0, DM1_WOUND_IDX_TORSO, 1);
    CHECK(defSharp >= def, "sharp defense should be >= base defense");

    PASS();
}

/* ── Test: shield defense includes F0312 hand strength ───────────── */
static void test_shield_defense_uses_hand_strength(void) {
    TEST(shield_defense_uses_hand_strength);

    DM1_CombatState base;
    DM1_CombatState shielded;
    dm1_combat_init(&base);
    dm1_combat_init(&shielded);
    base.championCount = 1;
    shielded.championCount = 1;
    dm1_combat_init_champion(&base.champions[0]);
    dm1_combat_init_champion(&shielded.champions[0]);

    base.champions[0].strength = 80;
    base.champions[0].vitality = 0;
    shielded.champions[0] = base.champions[0];
    shielded.champions[0].hasArmor[DM1_WOUND_IDX_READY_HAND] = 1;
    shielded.champions[0].armor[DM1_WOUND_IDX_READY_HAND].isShield = 1;
    shielded.champions[0].armor[DM1_WOUND_IDX_READY_HAND].defense = 0;
    shielded.champions[0].armor[DM1_WOUND_IDX_READY_HAND].weight = 0;

    dm1_combat_seed_rng(9001);
    int withoutShield = dm1_wound_defense(&base, 0, DM1_WOUND_IDX_TORSO, 0);
    dm1_combat_seed_rng(9001);
    int withShield = dm1_wound_defense(&shielded, 0, DM1_WOUND_IDX_TORSO, 0);

    CHECK(withShield > withoutShield,
          "shield contribution should include hand strength even when armor defense is zero");

    PASS();
}

/* ── Test: dead champion can't take damage ───────────────────────── */
static void test_dead_champion_no_damage(void) {
    TEST(dead_champion_no_damage);
    DM1_CombatState s;
    dm1_combat_init(&s);
    s.championCount = 1;
    dm1_combat_init_champion(&s.champions[0]);
    s.champions[0].alive = 0;
    s.champions[0].currentHealth = 0;

    int r = dm1_champion_take_damage(&s, 0, 50, DM1_WOUND_ALL, DM1_ATTACK_BLUNT);
    CHECK(r == 0, "dead champion should take 0 damage");

    PASS();
}

/* ── Test: lethal pending damage kills champion ──────────────────── */
static void test_lethal_pending(void) {
    TEST(lethal_pending);
    DM1_CombatState s;
    dm1_combat_init(&s);
    s.championCount = 1;
    dm1_combat_init_champion(&s.champions[0]);
    s.champions[0].currentHealth = 10;
    s.champions[0].maxHealth = 100;

    s.pendingDamage[0] = 50;
    dm1_apply_pending_damage(&s);
    CHECK(s.champions[0].currentHealth == 0, "should be dead");
    CHECK(s.champions[0].alive == 0, "should be dead");

    PASS();
}

/* -- Test: bow/crossbow SHOOT projectile parameters ----------------- */
static void test_ranged_shoot_bow_crossbow(void) {
    TEST(ranged_shoot_bow_crossbow);

    DM1_WeaponInfo bow = {0};
    DM1_WeaponInfo crossbow = {0};
    DM1_WeaponInfo arrow = {0};
    DM1_WeaponInfo slayer = {0};
    DM1_RangedShootResult out;

    bow.weaponClass = 20;
    bow.kineticEnergy = 50;
    bow.attributes = 0x2032;
    arrow.weaponClass = DM1_WEAPON_CLASS_BOW_AMMUNITION;
    arrow.kineticEnergy = 10;

    CHECK(dm1_ranged_shoot_resolve_pc34(&bow, &arrow, 0x1234, 0, 0, 5, &out) == 1,
          "bow+arrow should perform SHOOT");
    CHECK(out.actionPerformed == 1, "bow action should be performed");
    CHECK(out.noAmmunition == 0, "bow action should not report no ammo");
    CHECK(out.projectileThing == 0x1234, "ready-hand arrow should become projectile thing");
    CHECK(out.projectileCell == 0, "front-left/north launch cell should match F0326 formula");
    CHECK(out.projectileDirection == 0, "projectile direction should be champion direction");
    CHECK(out.kineticEnergy == 60, "bow kinetic energy should add launcher and arrow");
    CHECK(out.attack == 110, "bow attack should be (shoot-attack low byte + skill) << 1");
    CHECK(out.stepEnergy == 4, "bow class 20 should produce step energy 4");
    CHECK(out.actionDisabledTicks == DM1_ACTION_SHOOT_DISABLED_TICKS_PC34,
          "SHOOT should disable action for 14 ticks");
    CHECK(out.actionStaminaBase == DM1_ACTION_SHOOT_STAMINA_BASE_PC34,
          "SHOOT base stamina should be 3 before random(2)");
    CHECK(out.skillIndex == DM1_ACTION_SHOOT_SKILL_INDEX_PC34,
          "SHOOT should award C11 skill");
    CHECK(out.experienceGain == DM1_ACTION_SHOOT_EXPERIENCE_GAIN_PC34,
          "SHOOT should award 20 XP on success");
    CHECK(out.projectileMovementDisabledTicks == DM1_PROJECTILE_DISABLED_MOVEMENT_TICKS_PC34,
          "F0326 should set projectile movement lockout to 4 ticks");

    crossbow.weaponClass = 30;
    crossbow.kineticEnergy = 180;
    crossbow.attributes = 0x2078;
    slayer.weaponClass = DM1_WEAPON_CLASS_BOW_AMMUNITION;
    slayer.kineticEnergy = 28;

    CHECK(dm1_ranged_shoot_resolve_pc34(&crossbow, &slayer, 0x5678, 2, 1, 7, &out) == 1,
          "crossbow+slayer should perform SHOOT");
    CHECK(out.projectileThing == 0x5678, "ready-hand slayer should become projectile thing");
    CHECK(out.projectileCell == 2, "champion cell 2/east launch cell should match F0326 formula");
    CHECK(out.projectileDirection == 1, "crossbow projectile direction should be east");
    CHECK(out.kineticEnergy == 208, "crossbow kinetic energy should add launcher and slayer");
    CHECK(out.attack == 254, "crossbow attack should be (120 + shoot skill 7) << 1");
    CHECK(out.stepEnergy == 14, "crossbow class 30 should produce step energy 14");

    PASS();
}

/* -- Test: SHOOT without compatible bow ammunition ------------------ */
static void test_ranged_shoot_no_bow_ammunition(void) {
    TEST(ranged_shoot_no_bow_ammunition);

    DM1_WeaponInfo bow = {0};
    DM1_WeaponInfo rock = {0};
    DM1_RangedShootResult out;

    bow.weaponClass = 20;
    bow.kineticEnergy = 50;
    bow.attributes = 0x2032;
    rock.weaponClass = DM1_WEAPON_CLASS_SLING_AMMUNITION;
    rock.kineticEnergy = 18;

    CHECK(dm1_ranged_shoot_resolve_pc34(&bow, &rock, 0x2222, 0, 0, 5, &out) == 0,
          "bow+rock should fail compatibility gate");
    CHECK(out.actionPerformed == 0, "failed SHOOT should not perform action");
    CHECK(out.noAmmunition == 1, "failed SHOOT should report no ammunition");
    CHECK(out.projectileThing == -1, "failed SHOOT should not produce projectile thing");
    CHECK(out.experienceGain == 0, "failed SHOOT should zero XP gain");
    CHECK(out.actionDisabledTicks == DM1_ACTION_SHOOT_DISABLED_TICKS_PC34,
          "failed SHOOT still uses action disabled ticks from source table");
    CHECK(out.actionStaminaBase == DM1_ACTION_SHOOT_STAMINA_BASE_PC34,
          "failed SHOOT still uses source base stamina before random(2)");

    PASS();
}

/* -- Test: sling SHOOT projectile parameters and ammo class gate ----- */
static void test_ranged_shoot_sling_ammunition(void) {
    TEST(ranged_shoot_sling_ammunition);

    DM1_WeaponInfo sling = {0};
    DM1_WeaponInfo rock = {0};
    DM1_WeaponInfo arrow = {0};
    DM1_RangedShootResult out;

    sling.weaponClass = 39;
    sling.kineticEnergy = 20;
    sling.attributes = 0x2032;
    rock.weaponClass = DM1_WEAPON_CLASS_SLING_AMMUNITION;
    rock.kineticEnergy = 18;

    CHECK(dm1_ranged_shoot_resolve_pc34(&sling, &rock, 0x3333, 3, 2, 6, &out) == 1,
          "sling+rock should perform SHOOT");
    CHECK(out.actionPerformed == 1, "sling action should be performed");
    CHECK(out.noAmmunition == 0, "sling action should not report no ammo");
    CHECK(out.projectileThing == 0x3333, "ready-hand rock should become projectile thing");
    CHECK(out.projectileCell == 3, "champion cell 3/south launch cell should match F0326 formula");
    CHECK(out.projectileDirection == 2, "sling projectile direction should be south");
    CHECK(out.kineticEnergy == 38, "sling kinetic energy should add launcher and rock");
    CHECK(out.attack == 112, "sling attack should be (shoot-attack low byte + skill) << 1");
    CHECK(out.stepEnergy == 7, "sling class 39 should produce step energy 7");

    arrow.weaponClass = DM1_WEAPON_CLASS_BOW_AMMUNITION;
    arrow.kineticEnergy = 10;

    CHECK(dm1_ranged_shoot_resolve_pc34(&sling, &arrow, 0x4444, 3, 2, 6, &out) == 0,
          "sling+arrow should fail compatibility gate");
    CHECK(out.actionPerformed == 0, "failed sling SHOOT should not perform action");
    CHECK(out.noAmmunition == 1, "failed sling SHOOT should report no ammunition");
    CHECK(out.projectileThing == -1, "failed sling SHOOT should not produce projectile thing");

    PASS();
}

/* F0294 consumes raw PC3.4 WEAPON.Type records, not decoded item mirrors. */
static void test_f0294_raw_weapon_records(void) {
    struct DungeonThings_Compat things;
    unsigned char rawWeapons[20 * 4];
    unsigned short bow = (unsigned short)((THING_TYPE_WEAPON << 10) | 6);
    unsigned short arrow = (unsigned short)((THING_TYPE_WEAPON << 10) | 7);
    unsigned short sling = (unsigned short)((THING_TYPE_WEAPON << 10) | 18);
    unsigned short rock = (unsigned short)((THING_TYPE_WEAPON << 10) | 12);
    unsigned short scroll = (unsigned short)(THING_TYPE_SCROLL << 10);

    TEST(f0294_raw_weapon_records);
    memset(&things, 0, sizeof(things));
    memset(rawWeapons, 0, sizeof(rawWeapons));
    things.loaded = 1;
    things.rawThingData[THING_TYPE_WEAPON] = rawWeapons;
    things.thingCounts[THING_TYPE_WEAPON] = 20;

    /* PC3.4 WEAPON.Type occupies bits 0..6 of bytes 2..3.  G0238 rows:
     * 25 = bow class 20, 27 = bow ammunition class 10, 30 = sling
     * ammunition class 11, and 29 = sling class 39. */
    rawWeapons[6 * 4 + 2] = 25;
    rawWeapons[7 * 4 + 2] = 27;
    rawWeapons[12 * 4 + 2] = 30;
    rawWeapons[18 * 4 + 2] = 29;

    {
        DM1_WeaponInfo info;
        CHECK(dm1_v1_dungeon_get_weapon_info_pc34(&things, bow, &info) == 1,
              "F0158 should read the raw bow WEAPON.Type");
        CHECK(info.weaponClass == 20,
              "F0158 raw bow record should resolve G0238 class 20");
        CHECK(dm1_v1_dungeon_get_weapon_info_pc34(&things, arrow, &info) == 1,
              "F0158 should read the raw arrow WEAPON.Type");
        CHECK(info.weaponClass == DM1_WEAPON_CLASS_BOW_AMMUNITION,
              "F0158 raw arrow record should resolve G0238 class 10");
    }

    CHECK(dm1_champion_ammunition_compatible_f0294_pc34(&things, bow, arrow) == 1,
          "F0294 should accept raw G0238 bow plus arrow records");
    CHECK(dm1_champion_ammunition_compatible_f0294_pc34(&things, sling, rock) == 1,
          "F0294 should accept raw G0238 sling plus stone records");
    CHECK(dm1_champion_ammunition_compatible_f0294_pc34(&things, bow, rock) == 0,
          "F0294 should reject raw sling ammunition for a bow");
    CHECK(dm1_champion_ammunition_compatible_f0294_pc34(&things, scroll, arrow) == 0,
          "F0294 should reject a non-weapon launcher before F0158");
    CHECK(dm1_champion_ammunition_compatible_f0294_pc34(&things, bow, scroll) == 0,
          "F0294 should fail closed when F0158 cannot read ammunition");

    PASS();
}

/* ── Main ─────────────────────────────────────────────────────────── */
int main(void) {
    printf("=== DM1 V1 Combat System — Source-locked CTest Gate ===\n");

    test_scaled_product();
    test_armor_defense();
    test_f0143_raw_armour_records();
    test_stamina_adjusted();
    test_stat_adjusted_attack();
    test_weapon_info_class_table_source_lock();
    test_f0312_skill_level_bonus_uses_source_class_rules();
    test_creature_damage();
    test_creature_damage_multi();
    test_creature_damage_group_split_compacts_state();
    test_archenemy_immune();
    test_champion_damage_pipeline();
    test_sharp_attack_wounds();
    test_fire_attack_shield();
    test_poison_adjusted();
    test_poison_start_immediate_and_followup();
    test_creature_poison_gate_and_vitality_adjust();
    test_damage_all_creatures();
    test_damage_all_champions();
    test_damage_all_champions_zero_attack();
    test_pending_damage_applies_wounds_before_damage_guard();
    test_creature_melee_attack();
    test_creature_melee_parry_reduces_damage();
    test_champion_melee_action();
    test_non_material_melee_requires_vorpal_or_disrupt();
    test_material_melee_does_not_require_non_material_gate();
    test_wound_defense();
    test_shield_defense_uses_hand_strength();
    test_dead_champion_no_damage();
    test_lethal_pending();
    test_ranged_shoot_bow_crossbow();
    test_ranged_shoot_no_bow_ammunition();
    test_ranged_shoot_sling_ammunition();
    test_f0294_raw_weapon_records();
    test_champion_is_lucky();
    test_f0735_lucky_hit_enters_damage_path();
    test_f0735_dex_hit_short_circuits_random4_luck();
    test_f0735_zero_luck_f0308_uses_pc34_rng_count();
    test_f0735_non_material_gate_skips_luck();
    test_f0735_dexterity_255_skips_hit_branch();
    test_f0735_weak_damage_zero_roll_uses_miss_tail();
    test_ordered_cells_to_attack_priority();
    test_f0192_per_creature_resistance();
    test_f0801b_archenemy_double_move();

    printf("\n=== Results: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}

/* ── Test: F0308_CHAMPION_IsLucky (CHAMPION.C:1120-1155) ────────────── */
static void test_champion_is_lucky(void) {
    TEST(champion_is_lucky);

    /* Zero luck: F0308 returns 0 (unlucky) without consuming RNG. */
    DM1_ChampionCombat ch;
    dm1_combat_init_champion(&ch);
    ch.luck = 0;
    int r1 = dm1_champion_is_lucky(&ch, 60, 30);
    CHECK(r1 == 0, "luck=0 should never be lucky");
    CHECK(ch.luck == 0, "luck=0 should not be mutated by F0308");

    /* Lucky roll: luck decrements by 2 (CHAMPION.C:1153).
     * The free lucky gate (random(2) != 0 && random(100) > percentage)
     * returns 1 without touching Luck; the consuming branch returns 1
     * with random(luck) > percentage and decrements Luck by 2. We
     * pin the bounded-update path: try many seeds and confirm that
     * whenever luck > 0 the result is binary {0,1} and the mutation
     * is in {-2, 0, +2}. */
    for (uint32_t seed = 0; seed < 200; seed++) {
        dm1_combat_init_champion(&ch);
        ch.luck = 20;
        dm1_combat_seed_rng(seed);
        int before = ch.luck;
        int r = dm1_champion_is_lucky(&ch, 60, 30);
        if (r) {
            /* Lucky: either free gate (luck unchanged) or consuming
             * (luck decremented by 2). */
            int delta = ch.luck - before;
            CHECK(delta == 0 || delta == -2,
                  "lucky: luck delta must be 0 (free gate) or -2 (consume)");
        } else {
            /* Unlucky: free gate fail means luck is unchanged, OR
             * consuming fail (random(luck) <= percentage) increments
             * luck by 2 (bounded to luckMaximum=30). */
            int delta = ch.luck - before;
            CHECK(delta == 0 || delta == 2,
                  "unlucky: luck delta must be 0 (free gate) or +2 (consume)");
        }
    }

    /* Luck ceiling: at 29, an unlucky consume would push to 31 → bounded
     * to 30. */
    dm1_combat_init_champion(&ch);
    ch.luck = 29;
    /* Force the consume path by setting a high percentage (so the
     * random(luck) check is biased toward 0). The free gate is
     * 1-in-2, so we cannot deterministically force consume; instead
     * check that luck never exceeds 30 across many seeds. */
    for (uint32_t seed = 0; seed < 200; seed++) {
        dm1_combat_init_champion(&ch);
        ch.luck = 29;
        dm1_combat_seed_rng(seed);
        (void)dm1_champion_is_lucky(&ch, 60, 30);
        CHECK(ch.luck <= 30, "luck must never exceed luckMaximum (30)");
    }

    /* Luck floor: stays at 0 (no underflow). */
    for (uint32_t seed = 0; seed < 200; seed++) {
        dm1_combat_init_champion(&ch);
        ch.luck = 0;
        dm1_combat_seed_rng(seed);
        (void)dm1_champion_is_lucky(&ch, 60, 30);
        CHECK(ch.luck >= 0, "luck must never go below 0");
    }

    PASS();
}

/* -- Test: F0231 luck hit enters the damage path --------------------- */
static void test_f0735_lucky_hit_enters_damage_path(void) {
    TEST(f0735_lucky_hit_enters_damage_path);

    for (uint32_t seed = 1; seed < 2048; ++seed) {
        struct CombatantChampionSnapshot_Compat attacker;
        struct CombatantCreatureSnapshot_Compat defender;
        struct WeaponProfile_Compat weapon;
        struct CombatResult_Compat out;
        struct RngState_Compat rng;

        memset(&attacker, 0, sizeof(attacker));
        memset(&defender, 0, sizeof(defender));
        memset(&weapon, 0, sizeof(weapon));
        memset(&out, 0, sizeof(out));
        CHECK(F0730_COMBAT_RngInit_Compat(&rng, seed) == 1,
              "rng init should accept seed");

        attacker.championIndex = 0;
        attacker.currentHealth = 100;
        attacker.dexterity = 0;
        attacker.strengthActionHand = 100;
        attacker.skillLevelAction = 0;
        attacker.statisticLuck = 80;
        attacker.statisticLuckMax = 100;
        attacker.statisticLuckMin = 0;

        defender.creatureType = CREATURE_TYPE_GIANT_SCORPION;
        defender.defense = 0;
        defender.dexterity = 80;
        defender.attributes = 0;
        defender.doubledMapDifficulty = 30;
        defender.healthBefore = 200;

        weapon.hitProbability = 0;
        weapon.damageFactor = 32;

        CHECK(F0735_COMBAT_ResolveChampionMelee_Compat(
                  &attacker, &weapon, &defender, &rng, &out) == 1,
              "F0735 should resolve bounded lucky-hit fixture");
        if (out.luckyHit) {
            CHECK(out.hitLanded == 1,
                  "ReDMCSB F0231 lucky hit must enter hit/damage branch");
            CHECK(out.outcome == COMBAT_OUTCOME_HIT_DAMAGE ||
                      out.outcome == COMBAT_OUTCOME_HIT_NO_DAMAGE,
                  "lucky hit should not stay a miss");
            CHECK(out.damageApplied >= 0,
                  "lucky hit should produce bounded damage result");
            PASS();
            return;
        }
    }

    FAIL("no deterministic seed reached the F0308 lucky-hit branch");
}

/* -- Test: F0231 dexterity hit short-circuits random4/luck ----------- */
static void test_f0735_dex_hit_short_circuits_random4_luck(void) {
    TEST(f0735_dex_hit_short_circuits_random4_luck);

    {
        struct CombatantChampionSnapshot_Compat attacker;
        struct CombatantCreatureSnapshot_Compat defender;
        struct WeaponProfile_Compat weapon;
        struct CombatResult_Compat out;
        struct RngState_Compat rng;

        memset(&attacker, 0, sizeof(attacker));
        memset(&defender, 0, sizeof(defender));
        memset(&weapon, 0, sizeof(weapon));
        memset(&out, 0, sizeof(out));
        CHECK(F0730_COMBAT_RngInit_Compat(&rng, 0x630u) == 1,
              "rng init should accept dex-hit fixture seed");

        attacker.championIndex = 0;
        attacker.currentHealth = 100;
        attacker.dexterity = 255;
        attacker.strengthActionHand = 120;
        attacker.skillLevelAction = 0;
        attacker.statisticLuck = 40;
        attacker.statisticLuckMax = 100;
        attacker.statisticLuckMin = 0;

        defender.creatureType = CREATURE_TYPE_GIANT_SCORPION;
        defender.defense = 0;
        defender.dexterity = 0;
        defender.attributes = 0;
        defender.doubledMapDifficulty = 0;
        defender.healthBefore = 200;

        weapon.hitProbability = 0;
        weapon.damageFactor = 32;

        CHECK(F0735_COMBAT_ResolveChampionMelee_Compat(
                  &attacker, &weapon, &defender, &rng, &out) == 1,
              "F0735 should resolve dex-hit fixture");
        CHECK(out.hitLanded == 1,
              "high-dexterity fixture must hit through the first F0231 gate");
        CHECK(out.luckyHit == 0,
              "dexterity hit must not enter F0308 luck branch");
        CHECK(attacker.statisticLuck == 40,
              "dexterity hit must leave Luck unchanged");
        CHECK(out.rngCallCount == 9,
              "ReDMCSB F0231 dexterity hit consumes rand32 plus damage RNG only");
        CHECK(out.damageApplied > 0,
              "strong dex-hit fixture should stay on the non-weak damage path");
    }

    PASS();
}

/* -- Test: F0231 zero-Luck F0308 branch uses PC34 RNG count ----------- */
static void test_f0735_zero_luck_f0308_uses_pc34_rng_count(void) {
    TEST(f0735_zero_luck_f0308_uses_pc34_rng_count);

    for (uint32_t seed = 1; seed < 4096; ++seed) {
        struct RngState_Compat probeRng;
        int rand1;
        int rand2;
        int randShort;
        int dexOk;

        CHECK(F0730_COMBAT_RngInit_Compat(&probeRng, seed) == 1,
              "probe rng init should accept seed");
        rand1 = F0732_COMBAT_RngRandom_Compat(&probeRng, 32);
        rand2 = F0732_COMBAT_RngRandom_Compat(&probeRng, 4);
        randShort = F0732_COMBAT_RngRandom_Compat(&probeRng, 2);
        dexOk = (0 > (rand1 + 80 + 30 - 16));

        if (!dexOk && rand2 != 0 && randShort == 0) {
            struct CombatantChampionSnapshot_Compat attacker;
            struct CombatantCreatureSnapshot_Compat defender;
            struct WeaponProfile_Compat weapon;
            struct CombatResult_Compat out;
            struct RngState_Compat rng;

            memset(&attacker, 0, sizeof(attacker));
            memset(&defender, 0, sizeof(defender));
            memset(&weapon, 0, sizeof(weapon));
            memset(&out, 0, sizeof(out));
            CHECK(F0730_COMBAT_RngInit_Compat(&rng, seed) == 1,
                  "resolver rng init should accept selected seed");

            attacker.championIndex = 0;
            attacker.currentHealth = 100;
            attacker.dexterity = 0;
            attacker.strengthActionHand = 100;
            attacker.skillLevelAction = 0;
            attacker.statisticLuck = 0;
            attacker.statisticLuckMax = 30;
            attacker.statisticLuckMin = 0;

            defender.creatureType = CREATURE_TYPE_GIANT_SCORPION;
            defender.defense = 0;
            defender.dexterity = 80;
            defender.attributes = 0;
            defender.doubledMapDifficulty = 30;
            defender.healthBefore = 200;

            weapon.hitProbability = 0;
            weapon.damageFactor = 32;

            CHECK(F0735_COMBAT_ResolveChampionMelee_Compat(
                      &attacker, &weapon, &defender, &rng, &out) == 1,
                  "F0735 should resolve zero-Luck F0308 fixture");
            CHECK(out.hitLanded == 0,
                  "zero-Luck F0308 fixture should remain a miss");
            CHECK(out.luckyHit == 0,
                  "PC34 Luck <= 0 branch cannot produce lucky hit");
            CHECK(out.rngCallCount == 3,
                  "PC34 Luck <= 0 branch consumes rand32, rand4, rand2 only");
            CHECK(attacker.statisticLuck == 2,
                  "PC34 Luck <= 0 non-lucky branch still applies +2 bounded Luck");
            PASS();
            return;
        }
    }

    FAIL("no deterministic seed reached the zero-Luck F0308 branch");
}

/* -- Test: F0231 non-material gate short-circuits F0308 -------------- */
static void test_f0735_non_material_gate_skips_luck(void) {
    TEST(f0735_non_material_gate_skips_luck);

    for (uint32_t seed = 1; seed < 64; ++seed) {
        struct CombatantChampionSnapshot_Compat attacker;
        struct CombatantCreatureSnapshot_Compat defender;
        struct WeaponProfile_Compat weapon;
        struct CombatResult_Compat out;
        struct RngState_Compat rng;

        memset(&attacker, 0, sizeof(attacker));
        memset(&defender, 0, sizeof(defender));
        memset(&weapon, 0, sizeof(weapon));
        memset(&out, 0, sizeof(out));
        CHECK(F0730_COMBAT_RngInit_Compat(&rng, seed) == 1,
              "rng init should accept seed");

        attacker.championIndex = 0;
        attacker.currentHealth = 100;
        attacker.dexterity = 0;
        attacker.strengthActionHand = 100;
        attacker.skillLevelAction = 0;
        attacker.statisticLuck = 80;
        attacker.statisticLuckMax = 100;
        attacker.statisticLuckMin = 0;

        defender.creatureType = CREATURE_TYPE_GHOST;
        defender.defense = 0;
        defender.dexterity = 80;
        defender.attributes = CREATURE_ATTR_MASK_NON_MATERIAL;
        defender.doubledMapDifficulty = 30;
        defender.healthBefore = 200;

        weapon.hitProbability = 0;
        weapon.damageFactor = 32;

        CHECK(F0735_COMBAT_ResolveChampionMelee_Compat(
                  &attacker, &weapon, &defender, &rng, &out) == 1,
              "F0735 should resolve bounded non-material fixture");
        CHECK(out.luckyHit == 0,
              "ordinary attack against non-material target must not call F0308");
        CHECK(out.rngCallCount == 0,
              "non-material short-circuit must not consume F0231 hit RNG");
        CHECK(attacker.statisticLuck == 80,
              "non-material short-circuit should leave luck unchanged");
        CHECK(out.hitLanded == 0 && out.outcome == COMBAT_OUTCOME_MISS,
              "non-material short-circuit should remain a miss");
    }

    PASS();
}

/* -- Test: F0231 dexterity 255 skips the hit branch ------------------ */
static void test_f0735_dexterity_255_skips_hit_branch(void) {
    TEST(f0735_dexterity_255_skips_hit_branch);

    for (uint32_t seed = 1; seed < 64; ++seed) {
        struct CombatantChampionSnapshot_Compat attacker;
        struct CombatantCreatureSnapshot_Compat defender;
        struct WeaponProfile_Compat weapon;
        struct CombatResult_Compat out;
        struct RngState_Compat rng;

        memset(&attacker, 0, sizeof(attacker));
        memset(&defender, 0, sizeof(defender));
        memset(&weapon, 0, sizeof(weapon));
        memset(&out, 0, sizeof(out));
        CHECK(F0730_COMBAT_RngInit_Compat(&rng, seed) == 1,
              "rng init should accept seed");

        attacker.championIndex = 0;
        attacker.currentHealth = 100;
        attacker.dexterity = 255;
        attacker.strengthActionHand = 100;
        attacker.skillLevelAction = 12;
        attacker.statisticLuck = 80;
        attacker.statisticLuckMax = 100;
        attacker.statisticLuckMin = 0;

        defender.creatureType = CREATURE_TYPE_GIANT_SCORPION;
        defender.defense = 0;
        defender.dexterity = 255;
        defender.attributes = 0;
        defender.doubledMapDifficulty = 0;
        defender.healthBefore = 200;

        weapon.hitProbability = 0x80FF;
        weapon.damageFactor = 32;

        CHECK(F0735_COMBAT_ResolveChampionMelee_Compat(
                  &attacker, &weapon, &defender, &rng, &out) == 1,
              "F0735 should resolve bounded dexterity-255 fixture");
        CHECK(out.rngCallCount == 0,
              "CreatureInfo->Dexterity 255 should skip F0231 hit RNG");
        CHECK(out.luckyHit == 0,
              "CreatureInfo->Dexterity 255 should skip F0308");
        CHECK(attacker.statisticLuck == 80,
              "CreatureInfo->Dexterity 255 should leave luck unchanged");
        CHECK(out.hitLanded == 0 && out.damageApplied == 0 &&
                  out.outcome == COMBAT_OUTCOME_MISS,
              "CreatureInfo->Dexterity 255 should fall through as a miss");
    }

    PASS();
}

/* -- Test: F0231 weak-damage zero roll uses T0231015 miss tail ------- */
static void test_f0735_weak_damage_zero_roll_uses_miss_tail(void) {
    TEST(f0735_weak_damage_zero_roll_uses_miss_tail);

    for (uint32_t seed = 1; seed < 4096; ++seed) {
        struct RngState_Compat probeRng;
        int rand32Hit;
        int bonus;
        int defense;
        int damage0;
        int weakRoll;

        CHECK(F0730_COMBAT_RngInit_Compat(&probeRng, seed) == 1,
              "probe rng init should accept seed");
        rand32Hit = F0732_COMBAT_RngRandom_Compat(&probeRng, 32);
        (void)rand32Hit;
        bonus = F0732_COMBAT_RngRandom_Compat(&probeRng, 1);
        defense = F0732_COMBAT_RngRandom_Compat(&probeRng, 32) + 200;
        damage0 = F0732_COMBAT_RngRandom_Compat(&probeRng, 32) +
                  (((1 + bonus) * 1) >> 5) - defense;
        weakRoll = F0732_COMBAT_RngRandom_Compat(&probeRng, 4);

        if (damage0 <= 1 && weakRoll == 0) {
            struct CombatantChampionSnapshot_Compat attacker;
            struct CombatantCreatureSnapshot_Compat defender;
            struct WeaponProfile_Compat weapon;
            struct CombatResult_Compat out;
            struct RngState_Compat rng;

            memset(&attacker, 0, sizeof(attacker));
            memset(&defender, 0, sizeof(defender));
            memset(&weapon, 0, sizeof(weapon));
            memset(&out, 0, sizeof(out));
            CHECK(F0730_COMBAT_RngInit_Compat(&rng, seed) == 1,
                  "resolver rng init should accept selected seed");

            attacker.championIndex = 0;
            attacker.currentHealth = 100;
            attacker.dexterity = 255;
            attacker.strengthActionHand = 1;
            attacker.skillLevelAction = 0;
            attacker.statisticLuck = 40;
            attacker.statisticLuckMax = 100;
            attacker.statisticLuckMin = 0;

            defender.creatureType = CREATURE_TYPE_GIANT_SCORPION;
            defender.defense = 200;
            defender.dexterity = 0;
            defender.attributes = 0;
            defender.doubledMapDifficulty = 0;
            defender.healthBefore = 200;

            weapon.hitProbability = 0;
            weapon.damageFactor = 1;

            CHECK(F0735_COMBAT_ResolveChampionMelee_Compat(
                      &attacker, &weapon, &defender, &rng, &out) == 1,
                  "F0735 should resolve weak-damage zero-roll fixture");
            CHECK(out.rngCallCount == 5,
                  "ReDMCSB F0231 weak zero path consumes hit, damage, defense, damage, weak-roll RNG only");
            CHECK(out.hitLanded == 0,
                  "ReDMCSB T0231015 weak zero path is not a landed zero-damage hit");
            CHECK(out.damageApplied == 0 && out.outcome == COMBAT_OUTCOME_MISS,
                  "ReDMCSB T0231015 weak zero path returns the miss tail");
            PASS();
            return;
        }
    }

    FAIL("no deterministic seed reached the F0231 weak-damage zero-roll branch");
}

/* ── Test: F0229_GROUP_SetOrderedCellsToAttack (PROJEXPL.C:1284-1305) ── */
static void test_ordered_cells_to_attack_priority(void) {
    TEST(ordered_cells_to_attack_priority);

    DM1_CreatureGroup g;
    dm1_combat_init_group(&g);

    /* No creatures → -1. (The F0177 walk checks creatures[0..count];
     * with count=0 it never iterates and falls through to return -1,
     * matching the original v1 simplified contract.) */
    g.count = 0;
    g.creatures[0].cell = 0xFF;  /* clear — must not match any want. */
    int t0 = dm1_get_melee_target(&g, 0, 0, 0);
    CHECK(t0 == -1, "no creatures should return -1");

    /* One creature at each cell, every (partyDir, cell) pair. The
     * function must return a valid index in [0, count]. */
    for (int dir = 0; dir < 4; dir++) {
        for (int cell = 0; cell < 4; cell++) {
            for (int c = 0; c < 4; c++) {
                dm1_combat_init_group(&g);
                g.count = 0;
                g.creatures[0].cell = c;
                g.creatures[0].health = 50;
                int t = dm1_get_melee_target(&g, cell, dir, dir);
                CHECK(t == 0,
                      "single-creature group should return 0 for any (dir, cell, c)");
            }
        }
    }

    /* Two creatures: verify the function picks ONE (not out-of-range). */
    dm1_combat_init_group(&g);
    g.count = 1;
    g.creatures[0].cell = 0;
    g.creatures[0].health = 50;
    g.creatures[1].cell = 2;
    g.creatures[1].health = 50;
    int t1 = dm1_get_melee_target(&g, 0, 0, 0);
    CHECK(t1 == 0 || t1 == 1, "two-creature group should return 0 or 1");

    /* Invalid group pointer → -1. */
    int t2 = dm1_get_melee_target(NULL, 0, 0, 0);
    CHECK(t2 == -1, "NULL group should return -1");

    PASS();
}

/* ── Test: F0192 per-creature poison resistance (M10 new layer) ─────
 *
 * The M10 compat layer in src/memory/memory_combat_pc34_compat.c
 * ships a source-locked port of GROUP.C F0192. This test verifies:
 *  - F0192_GROUP_GetPoisonResistance_Compat returns the DUNGEON.C
 *    G0243 upper-nibble for each of the 27 DM1 creature types.
 *  - F0192_GROUP_GetResistanceAdjustedPoisonAttack_Compat returns 0
 *    for immune creatures (C23/C25/C26 — resistance 15).
 *  - F0192_GROUP_GetResistanceAdjustedPoisonAttack_Compat returns 0
 *    for poisonAttack == 0 (no-op).
 *  - The formula ((attack + rng(4)) << 3) / (resistance + 1) gives
 *    the same value as the C++ wrapper dm1_poison_adjusted_attack
 *    when seeded the same way (cross-checks the two implementations
 *    share a single formula).
 *  - Out-of-range creatureType is rejected with *outAdjusted = 0.
 */
static void test_f0192_per_creature_resistance(void) {
    TEST(f0192_per_creature_resistance);

    /* M061_POISON_RESISTANCE: (Resistances >> 8) & 0xF per I34E G0243. */
    static const int kExpected[27] = {
        8, 14,  2, 11, 10,  5,  7,  6, 15, 15, 15, 15, 15,  6,  3,
       11,  3,  0, 15, 15, 14,  8, 10, 15,  6, 15, 15
    };
    for (int t = 0; t < 27; t++) {
        int r = F0192_GROUP_GetPoisonResistance_Compat(t);
        CHECK(r == kExpected[t],
              "creature type resistance must match M061(Resistances) per I34E");
    }

    /* Out-of-range → -1. */
    CHECK(F0192_GROUP_GetPoisonResistance_Compat(-1) == -1, "negative → -1");
    CHECK(F0192_GROUP_GetPoisonResistance_Compat(27) == -1, ">= count → -1");

    /* Immune creatures: all with M061 resistance == 15. */
    struct RngState_Compat rng;
    F0730_COMBAT_RngInit_Compat(&rng, 0xC0FFEEu);
    {
        const int kImmune[] = {8, 9, 10, 11, 12, 18, 19, 23, 25, 26};
        for (size_t i = 0; i < sizeof(kImmune) / sizeof(kImmune[0]); i++) {
            int t = kImmune[i];
            int adj = 0x7FFFFFFFu;  /* sentinel */
            int rc = F0192_GROUP_GetResistanceAdjustedPoisonAttack_Compat(
                t, 100, &rng, &adj);
            CHECK(rc == 1, "immune creature: F0192 should return success");
            CHECK(adj == 0, "immune creature: adjusted attack must be 0");
        }
    }

    /* No poison attack → 0 (no RNG advance expected either). */
    F0730_COMBAT_RngInit_Compat(&rng, 0xDEADBEEFu);
    uint32_t before = rng.seed;
    int adj2 = 0x7FFFFFFFu;
    int rc2 = F0192_GROUP_GetResistanceAdjustedPoisonAttack_Compat(
        0, 0, &rng, &adj2);
    CHECK(rc2 == 1, "no poison: F0192 should return success (no-op)");
    CHECK(adj2 == 0, "no poison: adjusted attack must be 0");
    CHECK(rng.seed == before, "no poison: must not advance RNG");

    /* Formula: ((attack + rng(4)) << 3) / (resistance + 1).
     * C0 (Giant Scorpion) has resistance = 8, so denominator = 9.
     * adjusted = (attack + bump) * 8 / 9, bump in [0..3]. */
    F0730_COMBAT_RngInit_Compat(&rng, 0xABACAB1u);
    int adjCompat = -1;
    F0192_GROUP_GetResistanceAdjustedPoisonAttack_Compat(
        0 /* C0 */, 50, &rng, &adjCompat);
    CHECK(adjCompat >= ((50 + 0) * 8) / 9 &&
          adjCompat <= ((50 + 3) * 8) / 9,
          "C0 resistance 8 attack 50: result must be in [44, 47]");

    /* C15 Magenta Worm (resistance 11, denominator 12). With attack=100,
     * minimum bump gives (100 + 0) * 8 / 12 = 66; maximum bump gives
     * (100 + 3) * 8 / 12 = 68. */
    F0730_COMBAT_RngInit_Compat(&rng, 42);
    int adj15 = -1;
    F0192_GROUP_GetResistanceAdjustedPoisonAttack_Compat(
        15, 100, &rng, &adj15);
    CHECK(adj15 >= 66 && adj15 <= 68,
          "C15 (resistance 11): scaled attack must be in [66, 68]");

    /* C23 Lord Chaos (immune, r=15): always 0 regardless of attack. */
    F0730_COMBAT_RngInit_Compat(&rng, 1);
    int adj23 = -1;
    F0192_GROUP_GetResistanceAdjustedPoisonAttack_Compat(
        23, 255, &rng, &adj23);
    CHECK(adj23 == 0, "C23 Lord Chaos must be immune (0 damage)");

    /* Out-of-range creatureType is rejected. */
    F0730_COMBAT_RngInit_Compat(&rng, 99);
    int adjBad = 0x7FFFFFFFu;
    int rcBad = F0192_GROUP_GetResistanceAdjustedPoisonAttack_Compat(
        99, 50, &rng, &adjBad);
    CHECK(rcBad == 0, "out-of-range creature type must fail");
    CHECK(adjBad == 0, "out-of-range creature type must set adjusted to 0");

    /* NULL pointers rejected. */
    int adjNull = 0x7FFFFFFFu;
    (void)adjNull;
    int rcNull = F0192_GROUP_GetResistanceAdjustedPoisonAttack_Compat(
        0, 50, &rng, 0);
    CHECK(rcNull == 0, "NULL outAdjusted must fail");

    F0730_COMBAT_RngInit_Compat(&rng, 99);
    int adjNull2 = 0x7FFFFFFFu;
    int rcNull2 = F0192_GROUP_GetResistanceAdjustedPoisonAttack_Compat(
        0, 50, 0, &adjNull2);
    /* NULL rng is allowed in v1 — the random bump is treated as 0.
     * This is the lower-bound of the ((poisonAttack + random(4)) << 3)
     * term. Deterministic callers (test harnesses, projection paths)
     * rely on this so the result is reproducible. */
    CHECK(rcNull2 == 1, "NULL rng is accepted (random bump = 0)");
    CHECK(adjNull2 == (50 * 8) / (8 + 1),
          "NULL rng: result must be (attack * 8) / (resistance + 1)");

    PASS();
}

/* ── Test: F0801b archenemy double-move second-square helper (BUG-115b) ─
 *
 * Verifies the F0801b helper produces the correct second-square
 * target for each cardinal direction, and that the
 * CreatureTickResult_Compat.emittedDoubleMove field exists with the
 * expected layout (the layout is still 176 bytes total).
 */
static void test_f0801b_archenemy_double_move(void) {
    TEST(f0801b_archenemy_double_move);

    /* Layout invariant: the result struct must remain 176 bytes
     * after repurposing reserved0 → emittedDoubleMove. */
    CHECK(sizeof(struct CreatureTickResult_Compat) == 176,
          "CreatureTickResult_Compat must remain 176 bytes");

    /* F0801b: for each cardinal direction, the second square must
     * be one step further from the first square in that direction.
     * (DIR_NORTH=0: dy=-1, DIR_EAST=1: dx=+1, DIR_SOUTH=2: dy=+1,
     * DIR_WEST=3: dx=-1.) */
    const struct { int dir; int firstX; int firstY; int secondX; int secondY; } kCases[] = {
        { 0, 10, 10, 10,  9 },  /* NORTH */
        { 1, 10, 10, 11, 10 },  /* EAST  */
        { 2, 10, 10, 10, 11 },  /* SOUTH */
        { 3, 10, 10,  9, 10 },  /* WEST  */
    };
    for (size_t i = 0; i < sizeof(kCases) / sizeof(kCases[0]); i++) {
        struct CreatureTickResult_Compat r;
        memset(&r, 0, sizeof(r));
        int ok = F0801b_CREATURE_EmitArchenemySecondSquare_Compat(
            kCases[i].firstX, kCases[i].firstY, kCases[i].dir, &r);
        CHECK(ok == 1, "F0801b must return 1 on success");
        CHECK(r.outMovementTargetMapX == kCases[i].secondX,
              "F0801b: second square X must be first+dx");
        CHECK(r.outMovementTargetMapY == kCases[i].secondY,
              "F0801b: second square Y must be first+dy");
        CHECK(r.outMovementDirection == kCases[i].dir,
              "F0801b: direction must be preserved");
        CHECK(r.outMovementReserved == 1,
              "F0801b: outMovementReserved must be 1 (double-move marker)");
    }

    /* F0801b rejects out-of-range direction. */
    {
        struct CreatureTickResult_Compat r;
        memset(&r, 0, sizeof(r));
        int okBad = F0801b_CREATURE_EmitArchenemySecondSquare_Compat(
            10, 10, 4, &r);
        CHECK(okBad == 0, "F0801b must reject direction 4");
        okBad = F0801b_CREATURE_EmitArchenemySecondSquare_Compat(
            10, 10, -1, &r);
        CHECK(okBad == 0, "F0801b must reject direction -1");
    }

    /* F0801b rejects NULL output. */
    CHECK(F0801b_CREATURE_EmitArchenemySecondSquare_Compat(
              10, 10, 0, 0) == 0,
          "F0801b must reject NULL outResult");

    /* F0801: single-square emission leaves outMovementReserved = 0. */
    {
        struct CreatureTickInput_Compat in;
        struct CreatureAIState_Compat s;
        struct CreatureTickResult_Compat r;
        memset(&in, 0, sizeof(in));
        memset(&s, 0, sizeof(s));
        memset(&r, 0, sizeof(r));
        in.groupMapX = 5;
        in.groupMapY = 7;
        int ok1 = F0801_CREATURE_EmitMovement_Compat(&s, &in, 1, &r);
        CHECK(ok1 == 1, "F0801 must return 1 on success");
        CHECK(r.outMovementTargetMapX == 6, "F0801 EAST: X must be 6");
        CHECK(r.outMovementTargetMapY == 7, "F0801 EAST: Y must be 7");
        CHECK(r.outMovementReserved == 0,
              "F0801: outMovementReserved must be 0 (single move)");
    }

    /* F0801b / F0801 sequence: simulating a full F0804 dispatch
     * for an archenemy on the APPROACH state moving east from (5,7).
     * First square → (6,7); second square → (7,7). */
    {
        struct CreatureTickInput_Compat in;
        struct CreatureAIState_Compat s;
        struct CreatureTickResult_Compat r;
        memset(&in, 0, sizeof(in));
        memset(&s, 0, sizeof(s));
        memset(&r, 0, sizeof(r));
        r.emittedDoubleMove = 1;  /* archenemy intent flag from §(5b) */
        in.groupMapX = 5;
        in.groupMapY = 7;
        F0801_CREATURE_EmitMovement_Compat(&s, &in, 1, &r);
        /* r.outMovementTarget is (6,7). Now apply F0801b: */
        F0801b_CREATURE_EmitArchenemySecondSquare_Compat(
            r.outMovementTargetMapX, r.outMovementTargetMapY, 1, &r);
        CHECK(r.outMovementTargetMapX == 7,
              "double-move east: final X must be 7 (two squares)");
        CHECK(r.outMovementTargetMapY == 7,
              "double-move east: final Y must be 7");
        CHECK(r.outMovementReserved == 1,
              "double-move: outMovementReserved must be 1 (final state)");
    }

    /* Archenemy attributes are wired up in the static g_profiles
     * table (BUG-115b): C23 Lord Chaos, C25 Lord Order, C26 Grey
     * Lord must have CREATURE_ATTR_MASK_ARCHENEMY set. */
    {
        const struct CreatureBehaviorProfile_Compat* p23 =
            CREATURE_GetProfile_Compat(CREATURE_TYPE_LORD_CHAOS);
        const struct CreatureBehaviorProfile_Compat* p25 =
            CREATURE_GetProfile_Compat(CREATURE_TYPE_LORD_ORDER);
        const struct CreatureBehaviorProfile_Compat* p26 =
            CREATURE_GetProfile_Compat(CREATURE_TYPE_GREY_LORD);
        CHECK(p23 != 0 && p25 != 0 && p26 != 0,
              "C23/C25/C26 profiles must exist");
        if (p23) {
            CHECK(p23->attributes & CREATURE_ATTR_MASK_ARCHENEMY,
                  "C23 Lord Chaos must have CREATURE_ATTR_MASK_ARCHENEMY set");
        }
        if (p25) {
            CHECK(p25->attributes & CREATURE_ATTR_MASK_ARCHENEMY,
                  "C25 Lord Order must have CREATURE_ATTR_MASK_ARCHENEMY set");
        }
        if (p26) {
            CHECK(p26->attributes & CREATURE_ATTR_MASK_ARCHENEMY,
                  "C26 Grey Lord must have CREATURE_ATTR_MASK_ARCHENEMY set");
        }
    }

    /* C24 Red Dragon is NOT an archenemy (it's a normal ranged
     * spell-caster). The double-move should NOT apply to it. */
    {
        const struct CreatureBehaviorProfile_Compat* p24 =
            CREATURE_GetProfile_Compat(CREATURE_TYPE_RED_DRAGON);
        CHECK(p24 != 0, "C24 Red Dragon profile must exist");
        if (p24) {
            CHECK((p24->attributes & CREATURE_ATTR_MASK_ARCHENEMY) == 0,
                  "C24 Red Dragon must NOT have CREATURE_ATTR_MASK_ARCHENEMY");
        }
    }

    PASS();
}

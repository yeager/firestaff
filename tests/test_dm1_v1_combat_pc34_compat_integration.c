/*
 * test_dm1_v1_combat_pc34_compat_integration.c — CTest gate
 *
 * Verifies DM1 V1 combat system: melee, damage, armor, wounds.
 * Source-locked to ReDMCSB GROUP.C / CHAMPION.C / DEFS.H.
 */
#include "dm1_v1_combat_pc34_compat.h"
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
    CHECK(dm1_armor_defense(&ap, 1) == 25, "sharp defense should be 25");
    CHECK(dm1_armor_defense(NULL, 0) == 0, "NULL armor should be 0");
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

    dm1_combat_seed_rng(1); /* first random(2) == 0: poison gate fails */
    CHECK(dm1_creature_poison_attack_pc34(&s, 0, 96) == 0,
          "failed 50% gate should not start poison");
    CHECK(s.pendingDamage[0] == 0, "failed gate should leave pending damage unchanged");
    CHECK(s.pendingPoison[0].active == 0, "failed gate should not schedule poison");

    dm1_combat_seed_rng(3); /* first random(2) == 1: poison gate passes */
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

    DM1_CreatureGroup g;
    dm1_combat_init_group(&g);
    g.info.attack = 40;
    g.info.defense = 10;
    g.info.dexterity = 20;
    g.info.attackType = DM1_ATTACK_SHARP;
    g.info.woundProbHead = 8;
    g.info.woundProbTorso = 12;
    g.info.woundProbLegs = 6;
    g.info.woundProbFeet = 4;
    g.count = 0;
    g.creatures[0].health = 50;

    int dmg = dm1_creature_attack_champion(&s, &g, 0, 0);
    /* Damage may be 0 (miss) or positive, but function should not crash */
    CHECK(dmg >= 0, "damage should be non-negative");

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

/* ── Main ─────────────────────────────────────────────────────────── */
int main(void) {
    printf("=== DM1 V1 Combat System — Source-locked CTest Gate ===\n");

    test_scaled_product();
    test_armor_defense();
    test_stamina_adjusted();
    test_stat_adjusted_attack();
    test_creature_damage();
    test_creature_damage_multi();
    test_archenemy_immune();
    test_champion_damage_pipeline();
    test_sharp_attack_wounds();
    test_fire_attack_shield();
    test_poison_adjusted();
    test_poison_start_immediate_and_followup();
    test_creature_poison_gate_and_vitality_adjust();
    test_damage_all_creatures();
    test_damage_all_champions();
    test_creature_melee_attack();
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

    printf("\n=== Results: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}

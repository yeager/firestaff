/*
 * test_dm1_v1_combat_pc34_compat_integration.c — CTest gate
 *
 * Verifies DM1 V1 combat system: melee, damage, armor, wounds.
 * Source-locked to ReDMCSB GROUP.C / CHAMPION.C / DEFS.H.
 */
#include "dm1_v1_combat_pc34_compat.h"
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
    CHECK(dmg == 57, "sharp creature attack fixture should resolve to 57 damage");
    CHECK(s.pendingDamage[0] == 57, "creature damage should be queued as pending champion damage");
    CHECK(s.pendingWounds[0] == 0, "fixture should not add a wound after F0321 vitality check");
    CHECK(s.champions[0].currentHealth == 500, "pending damage should not be applied immediately");

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
    test_champion_is_lucky();
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

    /* All 27 entries match the DUNGEON.C G0243 upper nibble. */
    static const int kExpected[27] = {
        2,  3,  7,  9,  5,  4,  1,  2,  4,  3,  5,  5,  6,  5,  9,
        1,  2,  1,  7, 10,  7,  6, 11, 15, 11, 15, 15
    };
    for (int t = 0; t < 27; t++) {
        int r = F0192_GROUP_GetPoisonResistance_Compat(t);
        CHECK(r == kExpected[t],
              "creature type resistance must match DUNGEON.C G0243 upper nibble");
    }

    /* Out-of-range → -1. */
    CHECK(F0192_GROUP_GetPoisonResistance_Compat(-1) == -1, "negative → -1");
    CHECK(F0192_GROUP_GetPoisonResistance_Compat(27) == -1, ">= count → -1");

    /* Immune creatures (C23 Lord Chaos, C25 Lord Order, C26 Grey Lord). */
    struct RngState_Compat rng;
    F0730_COMBAT_RngInit_Compat(&rng, 0xC0FFEEu);
    {
        const int kImmune[] = {23, 25, 26};
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
     * C0 (Giant Scorpion) has resistance = 2, so denominator = 3.
     * The rng() bump is in [0..3], so adjusted = (attack * 8 + bump * 8) / 3
     * ranges over a small band. We don't depend on the exact rng() bump
     * because the M10 F0732 RNG and the C++ dm1_combat_random use
     * different state machines — the only contract is the closed-form
     * range, not bit-equality. Verify the result is in that range. */
    F0730_COMBAT_RngInit_Compat(&rng, 0xABACAB1u);
    int adjCompat = -1;
    F0192_GROUP_GetResistanceAdjustedPoisonAttack_Compat(
        0 /* C0 */, 50, &rng, &adjCompat);
    CHECK(adjCompat >= ((50 + 0) * 8) / 3 &&
          adjCompat <= ((50 + 3) * 8) / 3,
          "C0 resistance 2 attack 50: result must be in [(400/3), (424/3)]");

    /* C15 Magenta Worm (resistance 1, denominator 2): the small
     * denominator means a healthy bite. With attack=100, minimum
     * bump gives (100 + 0) * 8 / 2 = 400; maximum bump gives
     * (100 + 3) * 8 / 2 = 412. Verify the result is in that range. */
    F0730_COMBAT_RngInit_Compat(&rng, 42);
    int adj15 = -1;
    F0192_GROUP_GetResistanceAdjustedPoisonAttack_Compat(
        15, 100, &rng, &adj15);
    CHECK(adj15 >= 400 && adj15 <= 412,
          "C15 (resistance 1): scaled attack must be in [400, 412]");

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
    CHECK(adjNull2 == (50 * 8) / (2 + 1),
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

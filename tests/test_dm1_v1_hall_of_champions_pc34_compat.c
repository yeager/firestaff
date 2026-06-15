/*
 * test_dm1_v1_hall_of_champions_pc34_compat.c — BUG-119/120/121 regression gate
 *
 * Verifies Hall of Champions fixes:
 *   BUG-119 (Major): Champions die in Hall of Champions
 *     F0735_COMBAT_ResolveChampionMelee_Compat must return
 *     COMBAT_OUTCOME_NO_ACTION when the defender is the C040
 *     candidate (isCandidateInvulnerable=1), not apply damage.
 *   BUG-120 (Major): Slow after selection
 *     The C040 panel render path in m11_game_view.c has the
 *     early-return guard: if candidateMirrorPanelActive is set
 *     the wall-ornament blit is skipped, preventing per-frame
 *     re-rendering while the panel is open. Real pixel-level
 *     verification lives in
 *     probes/m11/firestaff_dm1_v1_hall_of_champions_panel_guard_probe.c
 *     (registered as CTest test dm1_v1_hall_of_champions_panel_guard).
 *     This unit test only pins the contract surface of the
 *     isCandidateInvulnerable field.
 *   BUG-121 (Major): Graphical artifacts (orange box floating)
 *     Same as BUG-120 — the C040 panel state suppresses the
 *     wall-ornament blit so the peach placeholder is hidden.
 *     Pixel-verified by the same probe.
 *
 * Source-locked to ReDMCSB CLIKCHAM.C F0367, MOVESENS.C:1501-1503,
 * REVIVE.C F0280 candidate selection.
 */
#include "memory_combat_pc34_compat.h"
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

/* ── BUG-119: F0735 must return NO_ACTION when defender is candidate ── */
static void test_candidate_invulnerable_attack_bounce(void) {
    TEST(candidate_invulnerable_attack_bounce);
    struct CombatantChampionSnapshot_Compat attacker;
    struct WeaponProfile_Compat weapon;
    struct CombatantCreatureSnapshot_Compat defender;
    struct RngState_Compat rng;
    struct CombatResult_Compat result;

    memset(&attacker, 0, sizeof(attacker));
    memset(&weapon, 0, sizeof(weapon));
    memset(&defender, 0, sizeof(defender));
    memset(&result, 0, sizeof(result));
    memset(&rng, 0, sizeof(rng));

    /* A healthy party champion attacking a candidate in D1C. */
    attacker.championIndex = 0;
    attacker.currentHealth = 100;
    attacker.dexterity = 50;
    attacker.strengthActionHand = 50;
    attacker.skillLevelParry = 5;
    attacker.skillLevelAction = 5;
    attacker.statisticVitality = 10;
    attacker.statisticAntifire = 0;
    attacker.statisticAntimagic = 0;
    attacker.actionHandIcon = 0;
    attacker.wounds = 0;
    for (int i = 0; i < 6; ++i) attacker.woundDefense[i] = 0;
    attacker.isResting = 0;
    attacker.partyShieldDefense = 0;

    weapon.weaponType = 0;
    weapon.weaponClass = 0;
    weapon.weaponStrength = 20;
    weapon.kineticEnergy = 10;
    weapon.hitProbability = 64;
    weapon.damageFactor = 8;
    weapon.skillIndex = 0;
    weapon.attributes = 0;

    /* The candidate creature — the one shown in the C040 panel. */
    defender.creatureType = 16;        /* Trolin / champion-mirror candidate */
    defender.creatureIndex = 99;
    defender.healthBefore = 100;
    defender.baseHealth = 100;
    defender.attack = 25;
    defender.defense = 28;
    defender.dexterity = 41;
    defender.poisonAttack = 0;
    defender.attackType = COMBAT_ATTACK_BLUNT;
    defender.attributes = 0;
    defender.woundProbabilities = 0xFC30;
    defender.properties = 0;
    defender.doubledMapDifficulty = 0;
    /* This is the fix: when the C040 panel is open, the candidate
     * is invulnerable to party attacks. */
    defender.isCandidateInvulnerable = 1;

    int rc = F0735_COMBAT_ResolveChampionMelee_Compat(
        &attacker, &weapon, &defender, &rng, &result);
    CHECK(rc == 1, "F0735 must return 1 (handled) when candidate is invulnerable");
    CHECK(result.outcome == COMBAT_OUTCOME_NO_ACTION,
          "outcome must be NO_ACTION, not MISS or HIT_DAMAGE");

    /* BUG-119 negative case: same attack without invulnerability
     * flag must NOT bounce to NO_ACTION. */
    defender.isCandidateInvulnerable = 0;
    memset(&result, 0, sizeof(result));
    rc = F0735_COMBAT_ResolveChampionMelee_Compat(
        &attacker, &weapon, &defender, &rng, &result);
    CHECK(rc == 1, "F0735 must return 1 even without invulnerable flag");
    CHECK(result.outcome != COMBAT_OUTCOME_NO_ACTION,
          "without invulnerable flag outcome must NOT be NO_ACTION");
    PASS();
}

/* ── BUG-120/121: contract surface for the panel-active guard ── */
/* Real pixel-level verification of the m11_draw_dm1_front_mirror_route
 * early-return now lives in
 * probes/m11/firestaff_dm1_v1_hall_of_champions_panel_guard_probe.c
 * (CTest: dm1_v1_hall_of_champions_panel_guard).  This unit test
 * pins the field-level contract so the engine-side fix has a
 * deterministic surface to bind to. */
static void test_m11_panel_active_guard_contract(void) {
    TEST(m11_panel_active_guard_contract);
    /* The isCandidateInvulnerable field is the F0735-side
     * contract surface that triggers the BUG-119 bounce, and
     * the candidateMirrorPanelActive flag is the BUG-120/121
     * render-side flag.  Both fields must be readable.  We
     * roundtrip the F0735 field here; the M11 side is round-
     * tripped by the panel_guard_probe (which links the
     * engine). */
    struct CombatantCreatureSnapshot_Compat s;
    memset(&s, 0, sizeof(s));
    s.isCandidateInvulnerable = 1;
    CHECK(s.isCandidateInvulnerable == 1,
          "isCandidateInvulnerable must be readable after set");
    s.isCandidateInvulnerable = 0;
    CHECK(s.isCandidateInvulnerable == 0,
          "isCandidateInvulnerable must clear on set 0");
    PASS();
}

/* ── Driver ──────────────────────────────────────────────────── */
int main(void) {
    printf("=== BUG-119/120/121 Hall of Champions regression gate ===\n");
    test_candidate_invulnerable_attack_bounce();
    test_m11_panel_active_guard_contract();
    printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}

/* test_dm2_v1_projectile_step_pc34_compat.c
 *
 * Phase 5 narrow source-faithful slice — DM2 V1 per-tick missile-step
 * helper regression gate.  This test pins one deterministic outcome
 * per branch of the STEP_MISSILE energy-decay + despawn boundary:
 *
 *   1. EMPTY_LIST                — empty input list, no slots alive.
 *   2. SURVIVE_KIN_GT_STEP       — kineticEnergy > stepEnergy → survive, decrement.
 *   3. DESPAWN_KIN_EQ_STEP       — kineticEnergy == stepEnergy → despawn (m_7CE0 boundary).
 *   4. DESPAWN_KIN_LT_STEP       — kineticEnergy < stepEnergy → despawn.
 *   5. DESPAWN_KIN_ZERO          — kineticEnergy == 0 → despawn (safety net).
 *   6. FIRST_GRACE_HONORED       — firstMoveGraceFlag == 1 → survive, no decrement.
 *   7. GRACE_CLEARS_ON_NEXT      — after grace, next step decrements.
 *   8. MULTIPLE_SLOTS_MIXED      — mix of survive/despawn/grace in one tick.
 *   9. STEP_AND_DRAIN_COUNT      — atomic step+returns survivor count = drained count.
 *  10. STEP_AND_DRAIN_PRESERVES  — survivors remain drainable; despawned don't reappear.
 *  11. COUNTERS_MONOTONIC        — step_total, step_despawned_total, step_survived_total.
 *  12. RESET_COUNTERS            — reset clears all step counters.
 *  13. STEP_RESULT_INVARIANTS    — slots_alive_before == slots_alive_after + slots_despawned.
 *  14. ENERGY_INVARIANT          — energy_total_after <= energy_total_before.
 *  15. SOURCE_EVIDENCE           — source-evidence string contains key anchors.
 *
 * Source: dm2_v1_projectile_step_pc34_compat.c (Phase 5 source-lock)
 *   skproject/SKULLWIN/c_tim_proc.cpp:442-563   (DM2_STEP_MISSILE)
 *       m_7CE0: kineticEnergy <= stepEnergy → CUT_RECORD_FROM + DELETE_MISSILE_RECORD
 *       m_7D2A: RG4Blo -= RG2Blo (else)
 *   skproject/SKWIN/SkWinCore.cpp:60920-61116   (STEP_MISSILE)
 *   ReDMCSB PROJEXPL.C:76-92 (F0212)
 *   ReDMCSB PROJEXPL.C:689-690 (F0219 C48 grace)
 *   ReDMCSB GROUP.C:1695-1770 (F0207 creature attack)
 *   memory_projectile_pc34_compat.h (F0813 despawn)
 */

#include "dm2_v1_projectile_pc34_compat.h"
#include "dm2_v1_projectile_step_pc34_compat.h"
#include "memory_projectile_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name_) do { \
    printf("  %s...\n", #name_); \
    tests_run++; \
    if (test_##name_()) { \
        tests_passed++; \
        printf("    PASS\n"); \
    } else { \
        printf("    FAIL\n"); \
    } \
} while (0)

/* Reset all relevant state for a clean test. */
static void reset_state(void) {
    /* Full-list reset (test-only) clears all entries + counters. */
    dm2_v1_projectile_test_reset_list();
    dm2_v1_projectile_step_reset_counters();
}

/* Forward decl: test-only energy override. */
extern int dm2_v1_projectile_test_set_slot_energy(int slot_index,
                                                    int kinetic_energy,
                                                    int step_energy,
                                                    int first_grace);

/* Helper: dispatch via the synthetic API then override energy.
 * The public synthetic dispatch sets kineticEnergy=100, stepEnergy=8
 * at F0810 time, so we override post-create via the test-only helper
 * (gated on FIRESTAFF_DM2_PROJECTILE_TESTING=1).  That gives us the
 * deterministic boundaries the source-locked rule needs:
 *   ke > se  → survive, decrement
 *   ke == se → despawn (m_7CE0 boundary)
 *   ke <  se → despawn
 *   ke == 0  → safety-net despawn
 *   grace=1  → survive, no decrement. */
static int dispatch_with_energy_test(int category, int subtype,
                                       int map_x, int map_y, int map_index,
                                       int direction,
                                       int kinetic_energy, int step_energy,
                                       int first_grace) {
    int slot = dm2_v1_projectile_dispatch_synthetic(category, subtype,
                                                      map_x, map_y,
                                                      map_index, direction);
    if (slot < 0) return -1;
    if (!dm2_v1_projectile_test_set_slot_energy(slot, kinetic_energy,
                                                  step_energy, first_grace)) {
        return -1;
    }
    return slot;
}

/* ── 1. EMPTY_LIST ───────────────────────────────────────────── */

static int test_empty_list(void) {
    reset_state();
    DM2_V1_ProjectileStepResult r = dm2_v1_projectile_step_tick();
    return r.slots_alive_before == 0
        && r.slots_alive_after == 0
        && r.slots_despawned == 0
        && r.slots_survived == 0
        && r.slots_graced == 0
        && r.energy_total_before == 0
        && r.energy_total_after == 0;
}

/* ── 2. SURVIVE_KIN_GT_STEP ────────────────────────────────────
 *
 * kineticEnergy=20, stepEnergy=5 → ke > se → survive, decrement to 15.
 *
 * Source-locked: skproject m_7D2A (RG4Blo -= RG2Blo).
 */
static int test_survive_kinetic_gt_step(void) {
    reset_state();
    int slot = dispatch_with_energy_test(PROJECTILE_CATEGORY_KINETIC,
                                           DM2_PROJ_SUBTYPE_KINETIC_ARROW,
                                           5, 5, 0, 0,
                                           20, 5, 0);
    if (slot < 0) return 0;
    DM2_V1_ProjectileStepResult r = dm2_v1_projectile_step_tick();
    if (r.slots_alive_before != 1) return 0;
    if (r.slots_alive_after  != 1) return 0;
    if (r.slots_despawned     != 0) return 0;
    if (r.slots_survived      != 1) return 0;
    if (r.energy_total_before != 20) return 0;
    if (r.energy_total_after  != 15) return 0;
    /* Slot is still in the list. */
    if (dm2_v1_projectile_active_count() != 1) return 0;
    /* Snapshot shows post-step energy 15. */
    DM2_V1_ProjectileSlotSnapshot s;
    if (!dm2_v1_projectile_get_slot(slot, &s)) return 0;
    return s.kineticEnergy == 15;
}

/* ── 3. DESPAWN_KIN_EQ_STEP ────────────────────────────────────
 *
 * kineticEnergy=5, stepEnergy=5 → ke <= se → despawn (m_7CE0 boundary).
 *
 * Source-locked: skproject m_7CE0 (RG4L <= RG1L → despawn).
 */
static int test_despawn_kinetic_eq_step(void) {
    reset_state();
    int slot = dispatch_with_energy_test(PROJECTILE_CATEGORY_MAGICAL,
                                           DM2_PROJ_SUBTYPE_MAGICAL_FIREBALL,
                                           5, 5, 0, 0,
                                           5, 5, 0);
    if (slot < 0) return 0;
    DM2_V1_ProjectileStepResult r = dm2_v1_projectile_step_tick();
    if (r.slots_alive_before != 1) return 0;
    if (r.slots_alive_after  != 0) return 0;
    if (r.slots_despawned     != 1) return 0;
    if (r.slots_survived      != 0) return 0;
    /* Slot is gone from the list. */
    if (dm2_v1_projectile_active_count() != 0) return 0;
    /* Snapshot returns 0 (slot empty). */
    DM2_V1_ProjectileSlotSnapshot s;
    return !dm2_v1_projectile_get_slot(slot, &s);
}

/* ── 4. DESPAWN_KIN_LT_STEP ────────────────────────────────────
 *
 * kineticEnergy=3, stepEnergy=10 → ke < se → despawn.
 *
 * Source-locked: skproject m_7CE0 (RG4L <= RG1L → despawn).
 */
static int test_despawn_kinetic_lt_step(void) {
    reset_state();
    int slot = dispatch_with_energy_test(PROJECTILE_CATEGORY_KINETIC,
                                           DM2_PROJ_SUBTYPE_KINETIC_ARROW,
                                           6, 6, 0, 1,
                                           3, 10, 0);
    if (slot < 0) return 0;
    DM2_V1_ProjectileStepResult r = dm2_v1_projectile_step_tick();
    return r.slots_alive_before == 1
        && r.slots_alive_after  == 0
        && r.slots_despawned     == 1
        && r.slots_survived      == 0
        && dm2_v1_projectile_active_count() == 0;
}

/* ── 5. DESPAWN_KIN_ZERO ───────────────────────────────────────
 *
 * kineticEnergy=0 → safety-net despawn (F0813 should have caught it
 * earlier but we make the step robust).
 */
static int test_despawn_kinetic_zero(void) {
    reset_state();
    int slot = dispatch_with_energy_test(PROJECTILE_CATEGORY_MAGICAL,
                                           DM2_PROJ_SUBTYPE_MAGICAL_LIGHTNING,
                                           7, 7, 0, 2,
                                           0, 8, 0);
    if (slot < 0) return 0;
    DM2_V1_ProjectileStepResult r = dm2_v1_projectile_step_tick();
    return r.slots_alive_before == 1
        && r.slots_despawned     == 1
        && r.slots_alive_after   == 0
        && dm2_v1_projectile_active_count() == 0;
}

/* ── 6. FIRST_GRACE_HONORED ─────────────────────────────────────
 *
 * firstMoveGraceFlag=1 → survive with unchanged kineticEnergy.
 *
 * Source-locked: ReDMCSB PROJEXPL.C:689-690 (F0219 C48 first-move).
 */
static int test_first_grace_honored(void) {
    reset_state();
    int slot = dispatch_with_energy_test(PROJECTILE_CATEGORY_KINETIC,
                                           DM2_PROJ_SUBTYPE_KINETIC_ARROW,
                                           8, 8, 0, 3,
                                           50, 8, 1);
    if (slot < 0) return 0;
    DM2_V1_ProjectileStepResult r = dm2_v1_projectile_step_tick();
    if (r.slots_alive_before != 1) return 0;
    if (r.slots_alive_after  != 1) return 0;
    if (r.slots_survived      != 1) return 0;
    if (r.slots_graced        != 1) return 0;
    if (r.energy_total_before != 50) return 0;
    if (r.energy_total_after  != 50) return 0;  /* unchanged */
    /* Grace flag is now cleared. */
    DM2_V1_ProjectileSlotSnapshot s;
    if (!dm2_v1_projectile_get_slot(slot, &s)) return 0;
    return s.firstMoveGraceFlag == 0;
}

/* ── 7. GRACE_CLEARS_ON_NEXT ────────────────────────────────────
 *
 * Two ticks: first tick honors grace (no decrement), second tick
 * decrements normally.
 */
static int test_grace_clears_on_next(void) {
    reset_state();
    int slot = dispatch_with_energy_test(PROJECTILE_CATEGORY_KINETIC,
                                           DM2_PROJ_SUBTYPE_KINETIC_ARROW,
                                           9, 9, 0, 0,
                                           50, 8, 1);
    if (slot < 0) return 0;
    /* Tick 1: grace honored. */
    DM2_V1_ProjectileStepResult r1 = dm2_v1_projectile_step_tick();
    if (r1.slots_graced != 1 || r1.energy_total_after != 50) return 0;
    /* Tick 2: real decrement. */
    DM2_V1_ProjectileStepResult r2 = dm2_v1_projectile_step_tick();
    if (r2.slots_graced        != 0) return 0;
    if (r2.slots_survived      != 1) return 0;
    if (r2.energy_total_before != 50) return 0;
    if (r2.energy_total_after  != 42) return 0;  /* 50 - 8 */
    DM2_V1_ProjectileSlotSnapshot s;
    if (!dm2_v1_projectile_get_slot(slot, &s)) return 0;
    return s.kineticEnergy == 42 && s.firstMoveGraceFlag == 0;
}

/* ── 8. MULTIPLE_SLOTS_MIXED ────────────────────────────────────
 *
 * 3 slots: A survives, B despawns, C grace-honored.
 */
static int test_multiple_slots_mixed(void) {
    reset_state();
    /* A: ke=20, se=5 → survives (post=15) */
    int sA = dispatch_with_energy_test(PROJECTILE_CATEGORY_KINETIC,
                                         DM2_PROJ_SUBTYPE_KINETIC_ARROW,
                                         1, 1, 0, 0,
                                         20, 5, 0);
    /* B: ke=4, se=10 → despawns */
    int sB = dispatch_with_energy_test(PROJECTILE_CATEGORY_MAGICAL,
                                         DM2_PROJ_SUBTYPE_MAGICAL_FIREBALL,
                                         2, 2, 0, 1,
                                         4, 10, 0);
    /* C: ke=30, se=6, grace=1 → survives (unchanged) */
    int sC = dispatch_with_energy_test(PROJECTILE_CATEGORY_MAGICAL,
                                         DM2_PROJ_SUBTYPE_MAGICAL_LIGHTNING,
                                         3, 3, 0, 2,
                                         30, 6, 1);
    if (sA < 0 || sB < 0 || sC < 0) return 0;

    DM2_V1_ProjectileStepResult r = dm2_v1_projectile_step_tick();
    if (r.slots_alive_before != 3) return 0;
    if (r.slots_alive_after  != 2) return 0;
    if (r.slots_despawned     != 1) return 0;
    if (r.slots_survived      != 2) return 0;
    if (r.slots_graced        != 1) return 0;
    if (r.energy_total_before != 54) return 0;  /* 20+4+30 */
    if (r.energy_total_after  != 45) return 0;  /* 15+30 */

    /* Verify slot identities: A survives with 15, B gone, C survives with 30. */
    DM2_V1_ProjectileSlotSnapshot sA_snap, sB_snap, sC_snap;
    int gotA = dm2_v1_projectile_get_slot(sA, &sA_snap);
    int gotB = dm2_v1_projectile_get_slot(sB, &sB_snap);
    int gotC = dm2_v1_projectile_get_slot(sC, &sC_snap);
    return gotA == 1 && gotC == 1 && gotB == 0
        && sA_snap.kineticEnergy == 15
        && sC_snap.kineticEnergy == 30;
}

/* ── 9. STEP_AND_DRAIN_COUNT ───────────────────────────────────
 *
 * Atomic step+returns survivor count = drained count.
 */
static int test_step_and_drain_count(void) {
    reset_state();
    /* 3 slots: A survives, B despawns, C survives. */
    dispatch_with_energy_test(PROJECTILE_CATEGORY_KINETIC,
                                DM2_PROJ_SUBTYPE_KINETIC_ARROW,
                                10, 10, 0, 0, 50, 5, 0);
    dispatch_with_energy_test(PROJECTILE_CATEGORY_MAGICAL,
                                DM2_PROJ_SUBTYPE_MAGICAL_FIREBALL,
                                11, 11, 0, 1, 2, 10, 0);
    dispatch_with_energy_test(PROJECTILE_CATEGORY_MAGICAL,
                                DM2_PROJ_SUBTYPE_MAGICAL_LIGHTNING,
                                12, 12, 0, 2, 30, 6, 0);

    DM2_V1_DrainedProjectile drained[8];
    DM2_V1_ProjectileStepResult step;
    int n = dm2_v1_projectile_step_and_drain(drained, 8, &step);
    return n == 2 && step.slots_alive_after == 2;
}

/* ── 10. STEP_AND_DRAIN_PRESERVES ──────────────────────────────
 *
 * Survivors remain drainable; despawned don't reappear.
 */
static int test_step_and_drain_preserves(void) {
    reset_state();
    int sA = dispatch_with_energy_test(PROJECTILE_CATEGORY_KINETIC,
                                         DM2_PROJ_SUBTYPE_KINETIC_ARROW,
                                         13, 13, 0, 0, 50, 5, 0);
    int sB = dispatch_with_energy_test(PROJECTILE_CATEGORY_MAGICAL,
                                         DM2_PROJ_SUBTYPE_MAGICAL_FIREBALL,
                                         14, 14, 0, 1, 1, 10, 0);

    DM2_V1_DrainedProjectile drained[8];
    int n1 = dm2_v1_projectile_step_and_drain(drained, 8, NULL);
    /* Only A survives; B is gone. */
    if (n1 != 1) return 0;

    /* Second call: A still survives (50→45), drain still 1. */
    int n2 = dm2_v1_projectile_step_and_drain(drained, 8, NULL);
    if (n2 != 1) return 0;

    /* Verify A's energy after two steps: 50 → 45 → 40. */
    DM2_V1_ProjectileSlotSnapshot sA_snap;
    if (!dm2_v1_projectile_get_slot(sA, &sA_snap)) return 0;
    if (sA_snap.kineticEnergy != 40) return 0;
    /* Verify B's slot is gone. */
    DM2_V1_ProjectileSlotSnapshot sB_snap;
    return !dm2_v1_projectile_get_slot(sB, &sB_snap);
}

/* ── 11. COUNTERS_MONOTONIC ─────────────────────────────────────
 *
 * step_total, step_despawned_total, step_survived_total monotonically
 * increment across calls and are not affected by repeated calls.
 */
static int test_counters_monotonic(void) {
    reset_state();
    if (dm2_v1_projectile_step_total() != 0) return 0;
    if (dm2_v1_projectile_step_despawned_total() != 0) return 0;
    if (dm2_v1_projectile_step_survived_total() != 0) return 0;

    /* Tick 1: dispatch 1 survive + 1 despawn, then step. */
    dispatch_with_energy_test(PROJECTILE_CATEGORY_KINETIC,
                                DM2_PROJ_SUBTYPE_KINETIC_ARROW,
                                20, 20, 0, 0, 50, 5, 0);
    dispatch_with_energy_test(PROJECTILE_CATEGORY_MAGICAL,
                                DM2_PROJ_SUBTYPE_MAGICAL_FIREBALL,
                                21, 21, 0, 1, 1, 10, 0);
    dm2_v1_projectile_step_tick();
    if (dm2_v1_projectile_step_total()          != 1) return 0;
    if (dm2_v1_projectile_step_despawned_total() != 1) return 0;
    if (dm2_v1_projectile_step_survived_total()  != 1) return 0;

    /* Tick 2: step again (1 survivor from before, no new dispatches). */
    dm2_v1_projectile_step_tick();
    if (dm2_v1_projectile_step_total()          != 2) return 0;
    if (dm2_v1_projectile_step_despawned_total() != 1) return 0;
    if (dm2_v1_projectile_step_survived_total()  != 2) return 0;
    return 1;
}

/* ── 12. RESET_COUNTERS ─────────────────────────────────────────
 *
 * Reset clears all step counters.
 */
static int test_reset_counters(void) {
    reset_state();
    dispatch_with_energy_test(PROJECTILE_CATEGORY_KINETIC,
                                DM2_PROJ_SUBTYPE_KINETIC_ARROW,
                                30, 30, 0, 0, 50, 5, 0);
    dm2_v1_projectile_step_tick();
    if (dm2_v1_projectile_step_total() == 0) return 0;
    dm2_v1_projectile_step_reset_counters();
    return dm2_v1_projectile_step_total() == 0
        && dm2_v1_projectile_step_despawned_total() == 0
        && dm2_v1_projectile_step_survived_total() == 0
        && dm2_v1_projectile_step_graced_total() == 0;
}

/* ── 13. STEP_RESULT_INVARIANTS ─────────────────────────────────
 *
 * Identity invariant: slots_alive_before == slots_alive_after + slots_despawned
 */
static int test_step_result_invariants(void) {
    reset_state();
    /* Mix of 5 slots. */
    dispatch_with_energy_test(PROJECTILE_CATEGORY_KINETIC,
                                DM2_PROJ_SUBTYPE_KINETIC_ARROW,
                                40, 40, 0, 0, 50, 5, 0);   /* survive */
    dispatch_with_energy_test(PROJECTILE_CATEGORY_MAGICAL,
                                DM2_PROJ_SUBTYPE_MAGICAL_FIREBALL,
                                41, 41, 0, 1, 5, 5, 0);    /* despawn (eq) */
    dispatch_with_energy_test(PROJECTILE_CATEGORY_MAGICAL,
                                DM2_PROJ_SUBTYPE_MAGICAL_LIGHTNING,
                                42, 42, 0, 2, 1, 10, 0);   /* despawn (lt) */
    dispatch_with_energy_test(PROJECTILE_CATEGORY_KINETIC,
                                DM2_PROJ_SUBTYPE_KINETIC_ARROW,
                                43, 43, 0, 3, 30, 6, 1);   /* grace */
    dispatch_with_energy_test(PROJECTILE_CATEGORY_MAGICAL,
                                DM2_PROJ_SUBTYPE_MAGICAL_POISON_CLOUD,
                                44, 44, 0, 0, 0, 8, 0);    /* despawn (zero) */
    DM2_V1_ProjectileStepResult r = dm2_v1_projectile_step_tick();
    return r.slots_alive_before == 5
        && r.slots_alive_after == 2
        && r.slots_despawned == 3
        && r.slots_alive_before == (r.slots_alive_after + r.slots_despawned)
        && r.slots_survived == 2  /* 1 real + 1 grace */
        && r.slots_graced == 1;
}

/* ── 14. ENERGY_INVARIANT ───────────────────────────────────────
 *
 * energy_total_after <= energy_total_before.
 */
static int test_energy_invariant(void) {
    reset_state();
    dispatch_with_energy_test(PROJECTILE_CATEGORY_KINETIC,
                                DM2_PROJ_SUBTYPE_KINETIC_ARROW,
                                50, 50, 0, 0, 50, 5, 0);
    dispatch_with_energy_test(PROJECTILE_CATEGORY_MAGICAL,
                                DM2_PROJ_SUBTYPE_MAGICAL_FIREBALL,
                                51, 51, 0, 1, 30, 6, 1);
    dispatch_with_energy_test(PROJECTILE_CATEGORY_MAGICAL,
                                DM2_PROJ_SUBTYPE_MAGICAL_LIGHTNING,
                                52, 52, 0, 2, 1, 10, 0);
    DM2_V1_ProjectileStepResult r = dm2_v1_projectile_step_tick();
    /* 50+30+1 = 81 before, 45+30+0 = 75 after (grace honored, ke unchanged). */
    return r.energy_total_before == 81
        && r.energy_total_after == 75
        && r.energy_total_after <= r.energy_total_before;
}

/* ── 15. SOURCE_EVIDENCE ──────────────────────────────────────── */

static int test_source_evidence(void) {
    const char *e = dm2_v1_projectile_step_source_evidence();
    return e != NULL && e[0] != '\0'
        && strstr(e, "DM2_STEP_MISSILE") != NULL
        && strstr(e, "m_7CE0") != NULL
        && strstr(e, "F0813") != NULL
        && strstr(e, "PROJEXPL.C") != NULL;
}

int main(void) {
    printf("DM2 V1 Projectile Step (Drain Runtime / M11 Cache Boundary) — Phase 5 source-lock tests\n");
    printf("Source: skproject/SKULLWIN/c_tim_proc.cpp:442-563   (DM2_STEP_MISSILE)\n"
           "        skproject/SKWIN/SkWinCore.cpp:60920-61116   (STEP_MISSILE wrapper)\n"
           "        ReDMCSB PROJEXPL.C:76-92 (F0212), 689-690 (F0219 C48 grace)\n"
           "        ReDMCSB GROUP.C:1695-1770 (F0207)\n");

    TEST(empty_list);
    TEST(survive_kinetic_gt_step);
    TEST(despawn_kinetic_eq_step);
    TEST(despawn_kinetic_lt_step);
    TEST(despawn_kinetic_zero);
    TEST(first_grace_honored);
    TEST(grace_clears_on_next);
    TEST(multiple_slots_mixed);
    TEST(step_and_drain_count);
    TEST(step_and_drain_preserves);
    TEST(counters_monotonic);
    TEST(reset_counters);
    TEST(step_result_invariants);
    TEST(energy_invariant);
    TEST(source_evidence);

    printf("\n%d/%d tests passed\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}

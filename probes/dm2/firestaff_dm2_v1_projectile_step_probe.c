/*
 * firestaff_dm2_v1_projectile_step_probe.c — DM2 V1 Projectile Step Probe
 *
 * Headless smoke probe for the per-tick missile-step helper.  Exercises
 * one invariant per branch of the energy-decay + despawn boundary and
 * verifies the runtime/drain-cache handoff end-to-end.
 *
 * Source-lock:
 *   skproject/SKULLWIN/c_tim_proc.cpp:442-563   (DM2_STEP_MISSILE)
 *       m_7CE0: kineticEnergy <= stepEnergy → despawn
 *       m_7D2A: else kineticEnergy -= stepEnergy
 *   skproject/SKWIN/SkWinCore.cpp:60920-61116   (STEP_MISSILE wrapper)
 *   ReDMCSB PROJEXPL.C:76-92 (F0212)
 *   ReDMCSB PROJEXPL.C:689-690 (F0219 C48 grace)
 *   ReDMCSB PROJEXPL.C:692-698 (F0219 creature impact before energy)
 *   ReDMCSB GROUP.C:1695-1770 (F0207)
 *   memory_projectile_pc34_compat.h (F0813 despawn)
 */

#define FIRESTAFF_DM2_CREATURE_TESTING 1
#define FIRESTAFF_DM2_PROJECTILE_TESTING 1

#include "dm2_v1_creature.h"
#include "dm2_v1_projectile_creature_collision_pc34_compat.h"
#include "dm2_v1_projectile_pc34_compat.h"
#include "dm2_v1_projectile_step_pc34_compat.h"
#include "memory_projectile_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int errors = 0;
static int passed = 0;

#define PROBE_ASSERT(cond, fmt, ...) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL: " fmt "\n", ##__VA_ARGS__); \
        errors++; \
    } else { \
        fprintf(stderr, "PASS: " fmt "\n", ##__VA_ARGS__); \
        passed++; \
    } \
} while (0)

/* Test-only helpers defined in dispatch module (gated on FIRESTAFF_DM2_PROJECTILE_TESTING). */
extern int dm2_v1_projectile_test_set_slot_energy(int slot_index,
                                                   int kinetic_energy,
                                                   int step_energy,
                                                   int first_grace);
extern void dm2_v1_projectile_test_reset_list(void);

/* Full-list reset helper.  Clears all entries + counters so each
 * invariant block starts from a known empty state. */
static void reset_all(void) {
    dm2_v1_projectile_test_reset_list();
    dm2_v1_projectile_step_reset_counters();
    dm2_v1_projectile_creature_collision_reset_counters();
    dm2_v1_creature_test_clear_ai_overrides();
}

/* Dispatch a synthetic projectile then override its energy fields.
 * Returns the slot index or -1. */
static int dispatch_with_energy(int category, int subtype,
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

static void build_ai(DM2_AIDefinition *out, uint16_t flags, uint8_t armor,
                     uint16_t hp) {
    memset(out, 0, sizeof(*out));
    out->w0AIFlags = flags;
    out->ArmorClass = armor;
    out->BaseHP = hp;
    out->AttackStrength = 5;
    out->Defense = 100;
    out->Weight = 100;
}

int main(int argc, char **argv) {
    (void)argc; (void)argv;

    fprintf(stderr, "=== DM2 V1 Projectile Step (Drain Runtime / M11 Cache Boundary) Probe ===\n");
    fprintf(stderr, "Source: skproject/SKULLWIN/c_tim_proc.cpp:442-563   (DM2_STEP_MISSILE)\n");
    fprintf(stderr, "        skproject/SKWIN/SkWinCore.cpp:60920-61116   (STEP_MISSILE)\n");
    fprintf(stderr, "        ReDMCSB PROJEXPL.C:76-92 (F0212), 689-690 (F0219 C48 grace)\n");
    fprintf(stderr, "        ReDMCSB GROUP.C:1695-1770 (F0207)\n\n");

    /* ── Reset ── */
    reset_all();

    /* ── Invariant 1: empty step ── */
    {
        DM2_V1_ProjectileStepResult r = dm2_v1_projectile_step_tick();
        PROBE_ASSERT(r.slots_alive_before == 0
                  && r.slots_alive_after == 0
                  && r.slots_despawned == 0
                  && r.slots_survived == 0,
                     "empty step (alive_before=%d alive_after=%d despawned=%d survived=%d)",
                     r.slots_alive_before, r.slots_alive_after,
                     r.slots_despawned, r.slots_survived);
    }

    /* ── Invariant 2: ke > se → survive, decrement ── */
    {
        reset_all();

        int slot = dispatch_with_energy(PROJECTILE_CATEGORY_KINETIC,
                                          DM2_PROJ_SUBTYPE_KINETIC_ARROW,
                                          5, 5, 0, 0, 20, 5, 0);
        PROBE_ASSERT(slot >= 0, "dispatched projectile (slot=%d)", slot);
        DM2_V1_ProjectileStepResult r = dm2_v1_projectile_step_tick();
        PROBE_ASSERT(r.slots_survived == 1 && r.slots_despawned == 0,
                     "ke=20 se=5: survive+decrement (survived=%d despawned=%d)",
                     r.slots_survived, r.slots_despawned);
        PROBE_ASSERT(r.energy_total_after == 15,
                     "ke=20 se=5: post-energy=15 (got %d)", r.energy_total_after);
        DM2_V1_ProjectileSlotSnapshot s;
        dm2_v1_projectile_get_slot(slot, &s);
        PROBE_ASSERT(s.kineticEnergy == 15,
                     "slot snapshot ke=15 (got %d)", s.kineticEnergy);
    }

    /* ── Invariant 3: ke == se → despawn (m_7CE0 boundary) ── */
    {
        reset_all();

        int slot = dispatch_with_energy(PROJECTILE_CATEGORY_MAGICAL,
                                          DM2_PROJ_SUBTYPE_MAGICAL_FIREBALL,
                                          6, 6, 0, 0, 5, 5, 0);
        PROBE_ASSERT(slot >= 0, "dispatched projectile (slot=%d)", slot);
        DM2_V1_ProjectileStepResult r = dm2_v1_projectile_step_tick();
        PROBE_ASSERT(r.slots_despawned == 1 && r.slots_survived == 0,
                     "ke=5 se=5: despawn at boundary (despawned=%d survived=%d)",
                     r.slots_despawned, r.slots_survived);
        DM2_V1_ProjectileSlotSnapshot s;
        PROBE_ASSERT(!dm2_v1_projectile_get_slot(slot, &s),
                     "slot is gone after despawn");
    }

    /* ── Invariant 4: ke < se → despawn ── */
    {
        reset_all();

        dispatch_with_energy(PROJECTILE_CATEGORY_KINETIC,
                              DM2_PROJ_SUBTYPE_KINETIC_ARROW,
                              7, 7, 0, 0, 3, 10, 0);
        DM2_V1_ProjectileStepResult r = dm2_v1_projectile_step_tick();
        PROBE_ASSERT(r.slots_despawned == 1,
                     "ke=3 se=10: despawn (despawned=%d)", r.slots_despawned);
    }

    /* ── Invariant 5: first-move grace honored, no decrement ── */
    {
        reset_all();

        int slot = dispatch_with_energy(PROJECTILE_CATEGORY_KINETIC,
                                          DM2_PROJ_SUBTYPE_KINETIC_ARROW,
                                          8, 8, 0, 0, 50, 8, 1);
        DM2_V1_ProjectileStepResult r = dm2_v1_projectile_step_tick();
        PROBE_ASSERT(r.slots_graced == 1 && r.slots_survived == 1,
                     "grace=1: graced+survived (graced=%d survived=%d)",
                     r.slots_graced, r.slots_survived);
        PROBE_ASSERT(r.energy_total_after == 50,
                     "grace: energy unchanged (after=%d)", r.energy_total_after);
        DM2_V1_ProjectileSlotSnapshot s;
        dm2_v1_projectile_get_slot(slot, &s);
        PROBE_ASSERT(s.firstMoveGraceFlag == 0,
                     "grace flag cleared after first step (got %d)",
                     s.firstMoveGraceFlag);
    }

    /* ── Invariant 6: step + drain atomic handoff ── */
    {
        reset_all();

        /* 3 slots: A survives, B despawns, C survives */
        dispatch_with_energy(PROJECTILE_CATEGORY_KINETIC,
                              DM2_PROJ_SUBTYPE_KINETIC_ARROW,
                              10, 10, 0, 0, 50, 5, 0);
        dispatch_with_energy(PROJECTILE_CATEGORY_MAGICAL,
                              DM2_PROJ_SUBTYPE_MAGICAL_FIREBALL,
                              11, 11, 0, 1, 2, 10, 0);
        dispatch_with_energy(PROJECTILE_CATEGORY_MAGICAL,
                              DM2_PROJ_SUBTYPE_MAGICAL_LIGHTNING,
                              12, 12, 0, 2, 30, 6, 0);
        DM2_V1_DrainedProjectile drained[8];
        DM2_V1_ProjectileStepResult step;
        int n = dm2_v1_projectile_step_and_drain(drained, 8, &step);
        PROBE_ASSERT(n == 2 && step.slots_alive_after == 2,
                     "step+returns 2 survivors drained=%d alive_after=%d",
                     n, step.slots_alive_after);
        /* Verify coordinates match survivors (10,10) and (12,12). */
        int found_10_10 = 0, found_12_12 = 0;
        for (int i = 0; i < n; i++) {
            if (drained[i].map_x == 10 && drained[i].map_y == 10) found_10_10 = 1;
            if (drained[i].map_x == 12 && drained[i].map_y == 12) found_12_12 = 1;
        }
        PROBE_ASSERT(found_10_10 && found_12_12,
                     "drain coords match survivors (10,10)=%d (12,12)=%d",
                     found_10_10, found_12_12);
    }

    /* ── Invariant 7: counters monotonic ── */
    {
        reset_all();

        int total_before = dm2_v1_projectile_step_total();
        dispatch_with_energy(PROJECTILE_CATEGORY_KINETIC,
                              DM2_PROJ_SUBTYPE_KINETIC_ARROW,
                              20, 20, 0, 0, 50, 5, 0);
        dispatch_with_energy(PROJECTILE_CATEGORY_MAGICAL,
                              DM2_PROJ_SUBTYPE_MAGICAL_FIREBALL,
                              21, 21, 0, 1, 1, 10, 0);
        dm2_v1_projectile_step_tick();
        PROBE_ASSERT(dm2_v1_projectile_step_total() == total_before + 1,
                     "step_total=%d (expected %d)",
                     dm2_v1_projectile_step_total(), total_before + 1);
        PROBE_ASSERT(dm2_v1_projectile_step_despawned_total() == 1,
                     "step_despawned_total=1 (got %d)",
                     dm2_v1_projectile_step_despawned_total());
        PROBE_ASSERT(dm2_v1_projectile_step_survived_total() == 1,
                     "step_survived_total=1 (got %d)",
                     dm2_v1_projectile_step_survived_total());
    }

    /* ── Invariant 8: creature collision resolves before energy despawn ── */
    {
        reset_all();

        DM2_AIDefinition spec;
        build_ai(&spec, 0, 0, 20);
        dm2_v1_creature_test_set_ai_spec(60, &spec);
        int cid = dm2_v1_creature_spawn(60, 62, 62, 0, 0, 8);
        int slot = dispatch_with_energy(PROJECTILE_CATEGORY_KINETIC,
                                          DM2_PROJ_SUBTYPE_KINETIC_ARROW,
                                          62, 62, 0, 0, 1, 10, 0);
        PROBE_ASSERT(cid >= 0 && slot >= 0,
                     "creature collision fixture spawned cid=%d slot=%d",
                     cid, slot);
        DM2_V1_ProjectileStepResult r = dm2_v1_projectile_step_tick();
        PROBE_ASSERT(r.slots_creature_collisions == 1
                  && r.slots_despawned == 1
                  && r.slots_survived == 0
                  && dm2_v1_creature_instance_hp(cid) == 10,
                     "creature impact before energy (collisions=%d despawned=%d survived=%d hp=%d)",
                     r.slots_creature_collisions, r.slots_despawned,
                     r.slots_survived, dm2_v1_creature_instance_hp(cid));
        PROBE_ASSERT(dm2_v1_projectile_step_creature_collision_total() == 1
                  && dm2_v1_projectile_creature_collision_count() == 1,
                     "creature collision counters step=%d resolver=%d",
                     dm2_v1_projectile_step_creature_collision_total(),
                     dm2_v1_projectile_creature_collision_count());
    }

    /* ── Invariant 9: source evidence ── */
    {
        const char *e = dm2_v1_projectile_step_source_evidence();
        PROBE_ASSERT(e != NULL
                  && strstr(e, "DM2_STEP_MISSILE") != NULL
                  && strstr(e, "m_7CE0") != NULL
                  && strstr(e, "PROJEXPL.C:692-698") != NULL,
                     "source evidence contains DM2_STEP_MISSILE + m_7CE0 + creature-impact ordering");
    }

    reset_all();

    fprintf(stderr, "\n=== Summary ===\n");
    fprintf(stderr, "Step total: %d, despawned_total: %d, survived_total: %d, graced_total: %d, creature_collisions=%d\n",
            dm2_v1_projectile_step_total(),
            dm2_v1_projectile_step_despawned_total(),
            dm2_v1_projectile_step_survived_total(),
            dm2_v1_projectile_step_graced_total(),
            dm2_v1_projectile_step_creature_collision_total());
    fprintf(stderr, "\n%d/%d invariants PASS\n", passed, passed + errors);
    return (errors == 0) ? 0 : 1;
}

/*
 * firestaff_dm2_v1_projectile_creature_collision_probe.c
 *
 * Headless DM2 V1 projectile-vs-creature collision smoke probe.
 * Exercises one deterministic outcome per branch of the missile-
 * redirect dispatch and reports pass/fail counts.  Used by CI to
 * verify the source-locked contract without running CTest.
 *
 * Source-lock:
 *   SKULL.ASM:10620-10710  (SKULL_COMBAT_ResolveRanged)
 *   SKULL.ASM:10800-10890  (SKULL_COMBAT_DoorAttack + REFLECTOR)
 *   ReDMCSB PROJEXPL.C:76-92       (F0212)
 *   ReDMCSB GROUP.C:1695-1770      (F0207)
 *   skproject/SKULLWIN/c_combat.cpp:401-420 (kill threshold)
 *   skproject/SKWIN/DME.h:1545-1560 (w0AIFlags)
 */

#define FIRESTAFF_DM2_CREATURE_TESTING 1

#include "dm2_v1_projectile_creature_collision_pc34_compat.h"
#include "dm2_v1_projectile_pc34_compat.h"
#include "dm2_v1_creature.h"
#include "memory_projectile_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int passed = 0;
static int errors = 0;

#define PROBE(cond, fmt, ...) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL: " fmt "\n", ##__VA_ARGS__); \
        errors++; \
    } else { \
        passed++; \
    } \
} while (0)

static void build_ai(DM2_AIDefinition *out, uint16_t flags, uint8_t armor,
                     uint16_t hp) {
    memset(out, 0, sizeof(*out));
    out->w0AIFlags   = flags;
    out->ArmorClass  = armor;
    out->BaseHP      = hp;
    out->AttackStrength = 5;
    out->Defense = 100;
    out->Weight = 100;
}

int main(int argc, char **argv) {
    (void)argc; (void)argv;

    fprintf(stderr, "=== DM2 V1 Projectile-vs-Creature Collision Probe ===\n");
    fprintf(stderr, "Source: SKULL.ASM:10620-10710, PROJEXPL.C:76-92,\n"
                    "        GROUP.C:1695-1770, DME.h:1545-1560\n\n");

    /* Use a synthetic AI index above the natural table to keep this
     * probe independent of any other test that mutates real AI specs. */
    const int SYNTH_AI = 60;
    int slot;

    /* 1. HIT that kills. */
    {
        dm2_v1_projectile_reset_counters();
        dm2_v1_projectile_creature_collision_reset_counters();
        dm2_v1_creature_test_clear_ai_overrides();
        DM2_AIDefinition spec; build_ai(&spec, 0, 0, 10);
        dm2_v1_creature_test_set_ai_spec(SYNTH_AI, &spec);
        (void)dm2_v1_creature_spawn(SYNTH_AI, 1, 1, 0, 0, 8);
        slot = dm2_v1_projectile_dispatch_synthetic(
            PROJECTILE_CATEGORY_KINETIC,
            DM2_PROJ_SUBTYPE_KINETIC_ARROW, 1, 1, 0, 0);
        DM2_V1_ProjectileCreatureCollisionResult r =
            dm2_v1_projectile_creature_collision_resolve(slot, 20);
        PROBE(r.outcome == DM2_PROJ_CREATURE_OUTCOME_HIT
              && r.damage_dealt == 20 && r.hp_after == 0
              && r.kill_event_emitted == 1 && r.projectile_despawned == 1,
              "HIT kills (outcome=%d dmg=%d hp_after=%d kill=%d despawn=%d)",
              r.outcome, r.damage_dealt, r.hp_after,
              r.kill_event_emitted, r.projectile_despawned);
    }

    /* 2. HIT that survives (armor floor). */
    {
        dm2_v1_projectile_reset_counters();
        dm2_v1_projectile_creature_collision_reset_counters();
        dm2_v1_creature_test_clear_ai_overrides();
        DM2_AIDefinition spec; build_ai(&spec, 0, 20, 50);
        dm2_v1_creature_test_set_ai_spec(SYNTH_AI, &spec);
        (void)dm2_v1_creature_spawn(SYNTH_AI, 2, 2, 0, 0, 8);
        slot = dm2_v1_projectile_dispatch_synthetic(
            PROJECTILE_CATEGORY_KINETIC,
            DM2_PROJ_SUBTYPE_KINETIC_ARROW, 2, 2, 0, 0);
        DM2_V1_ProjectileCreatureCollisionResult r =
            dm2_v1_projectile_creature_collision_resolve(slot, 8);
        PROBE(r.damage_dealt == 1 && r.hp_after == 49
              && r.kill_event_emitted == 0,
              "HIT survives armor floor (dmg=%d hp_after=%d kill=%d)",
              r.damage_dealt, r.hp_after, r.kill_event_emitted);
    }

    /* 3. NONMATERIAL passthrough. */
    {
        dm2_v1_projectile_reset_counters();
        dm2_v1_projectile_creature_collision_reset_counters();
        dm2_v1_creature_test_clear_ai_overrides();
        DM2_AIDefinition spec; build_ai(&spec, DM2_AIFLAG_NONMATERIAL, 0, 100);
        dm2_v1_creature_test_set_ai_spec(SYNTH_AI, &spec);
        (void)dm2_v1_creature_spawn(SYNTH_AI, 3, 3, 0, 0, 8);
        slot = dm2_v1_projectile_dispatch_synthetic(
            PROJECTILE_CATEGORY_MAGICAL,
            DM2_PROJ_SUBTYPE_MAGICAL_FIREBALL, 3, 3, 0, 0);
        DM2_V1_ProjectileCreatureCollisionResult r =
            dm2_v1_projectile_creature_collision_resolve(slot, 50);
        PROBE(r.outcome == DM2_PROJ_CREATURE_OUTCOME_NONMATERIAL
              && r.damage_dealt == 0
              && r.projectile_despawned == 0,
              "NONMATERIAL (outcome=%d dmg=%d despawn=%d)",
              r.outcome, r.damage_dealt, r.projectile_despawned);
    }

    /* 4. ABSORBS_MISSILE. */
    {
        dm2_v1_projectile_reset_counters();
        dm2_v1_projectile_creature_collision_reset_counters();
        dm2_v1_creature_test_clear_ai_overrides();
        DM2_AIDefinition spec; build_ai(&spec, DM2_AIFLAG_ABSORBS_MISSILE, 0, 30);
        dm2_v1_creature_test_set_ai_spec(SYNTH_AI, &spec);
        (void)dm2_v1_creature_spawn(SYNTH_AI, 4, 4, 0, 0, 8);
        slot = dm2_v1_projectile_dispatch_synthetic(
            PROJECTILE_CATEGORY_KINETIC,
            DM2_PROJ_SUBTYPE_KINETIC_ARROW, 4, 4, 0, 0);
        DM2_V1_ProjectileCreatureCollisionResult r =
            dm2_v1_projectile_creature_collision_resolve(slot, 15);
        PROBE(r.outcome == DM2_PROJ_CREATURE_OUTCOME_ABSORBED
              && r.damage_dealt == 0
              && r.projectile_despawned == 1,
              "ABSORBS_MISSILE (outcome=%d dmg=%d despawn=%d)",
              r.outcome, r.damage_dealt, r.projectile_despawned);
    }

    /* 5. REFLECTOR. */
    {
        dm2_v1_projectile_reset_counters();
        dm2_v1_projectile_creature_collision_reset_counters();
        dm2_v1_creature_test_clear_ai_overrides();
        DM2_AIDefinition spec; build_ai(&spec, DM2_AIFLAG_REFLECTOR, 5, 40);
        dm2_v1_creature_test_set_ai_spec(SYNTH_AI, &spec);
        (void)dm2_v1_creature_spawn(SYNTH_AI, 5, 5, 0, 0, 8);
        slot = dm2_v1_projectile_dispatch_synthetic(
            PROJECTILE_CATEGORY_MAGICAL,
            DM2_PROJ_SUBTYPE_MAGICAL_FIREBALL, 5, 5, 0, 0);
        DM2_V1_ProjectileCreatureCollisionResult r =
            dm2_v1_projectile_creature_collision_resolve(slot, 25);
        PROBE(r.outcome == DM2_PROJ_CREATURE_OUTCOME_REFLECTED
              && r.damage_dealt == 0
              && r.projectile_despawned == 1,
              "REFLECTOR (outcome=%d dmg=%d despawn=%d)",
              r.outcome, r.damage_dealt, r.projectile_despawned);
    }

    /* 6. REDIRECTED (TURNS_MISSILE). */
    {
        dm2_v1_projectile_reset_counters();
        dm2_v1_projectile_creature_collision_reset_counters();
        dm2_v1_creature_test_clear_ai_overrides();
        DM2_AIDefinition spec; build_ai(&spec, DM2_AI_W30_TURNS_MISSILE, 4, 60);
        dm2_v1_creature_test_set_ai_spec(SYNTH_AI, &spec);
        (void)dm2_v1_creature_spawn(SYNTH_AI, 6, 6, 0, 0, 8);
        slot = dm2_v1_projectile_dispatch_synthetic(
            PROJECTILE_CATEGORY_MAGICAL,
            DM2_PROJ_SUBTYPE_MAGICAL_LIGHTNING, 6, 6, 0, 0);
        DM2_V1_ProjectileCreatureCollisionResult r =
            dm2_v1_projectile_creature_collision_resolve(slot, 12);
        PROBE(r.outcome == DM2_PROJ_CREATURE_OUTCOME_REDIRECTED
              && r.damage_dealt == 10
              && r.hp_after == 50
              && r.projectile_despawned == 1,
              "REDIRECTED (outcome=%d dmg=%d hp_after=%d despawn=%d)",
              r.outcome, r.damage_dealt, r.hp_after, r.projectile_despawned);
    }

    /* 7. Invalid slot rejected. */
    {
        dm2_v1_projectile_creature_collision_reset_counters();
        DM2_V1_ProjectileCreatureCollisionResult r =
            dm2_v1_projectile_creature_collision_resolve(-1, 10);
        PROBE(r.accepted == 0
              && r.outcome == DM2_PROJ_CREATURE_OUTCOME_INVALID,
              "Invalid slot rejected (accepted=%d outcome=%d)",
              r.accepted, r.outcome);
    }

    /* 8. Source evidence citation. */
    {
        const char *e = dm2_v1_projectile_creature_collision_source_evidence();
        PROBE(e != NULL && strstr(e, "SKULL.ASM:10620-10710") != NULL
              && strstr(e, "deterministic") != NULL,
              "Source evidence present (len=%lu)",
              (unsigned long)(e ? strlen(e) : 0));
    }

    fprintf(stderr, "\n=== Summary ===\n");
    fprintf(stderr, "%d invariants PASS, %d FAIL\n", passed, errors);
    return errors == 0 ? 0 : 1;
}

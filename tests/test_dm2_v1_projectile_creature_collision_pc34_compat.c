/* test_dm2_v1_projectile_creature_collision_pc34_compat.c
 *
 * Phase 5 (creature/combat parity) — DM2 V1 projectile-vs-creature
 * collision regression gate.  This test pins one deterministic
 * damage/event outcome per branch of the missile-redirect dispatch:
 *
 *   1. NO_CREATURE_AT_TARGET         — projectile continues (no event)
 *   2. HIT, kills creature           — deterministic damage + kill event
 *   3. HIT, creature survives        — deterministic damage, no kill
 *   4. NONMATERIAL                   — projectile passes through, no damage
 *   5. ABSORBS_MISSILE               — projectile despawned, no damage
 *   6. REFLECTOR                     — projectile despawned, no damage
 *   7. REDIRECTED (TURNS_MISSILE)    — same damage path as HIT
 *   8. INVALID projectile slot       — rejected
 *   9. Damage formula pinning        — exact damage values for fixed inputs
 *
 * Source: dm2_v1_projectile_creature_collision_pc34_compat.c (Phase 5)
 *   SKULL.ASM:10620-10710  (SKULL_COMBAT_ResolveRanged)
 *   SKULL.ASM:10800-10890  (SKULL_COMBAT_DoorAttack + REFLECTOR)
 *   ReDMCSB PROJEXPL.C:76-92 (F0212)
 *   ReDMCSB GROUP.C:1695-1770 (F0207)
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

/* Build a synthetic AIDefinition with the given flags and armor. */
static void build_ai_spec(DM2_AIDefinition *out, uint16_t flags,
                           uint8_t armor, uint16_t base_hp) {
    memset(out, 0, sizeof(*out));
    out->w0AIFlags   = flags;
    out->ArmorClass  = armor;
    out->BaseHP      = base_hp;
    out->AttackStrength = 5;
    out->Defense     = 100;
    out->Weight      = 100;
}

/* Helper: spawn a creature and place a synthetic projectile at the
 * same square.  Returns the creature instance id; *out_proj_slot
 * receives the projectile slot index. */
static int setup_creature_with_projectile(int creature_ai_index,
                                            const DM2_AIDefinition *creature_spec,
                                            int world_x, int world_y,
                                            int map_index,
                                            int proj_category, int proj_subtype,
                                            int *out_proj_slot) {
    dm2_v1_creature_test_set_ai_spec(creature_ai_index, creature_spec);
    int cid = dm2_v1_creature_spawn(creature_ai_index, world_x, world_y,
                                     map_index, 0, 8);
    if (cid < 0) return -1;
    int slot = dm2_v1_projectile_dispatch_synthetic(proj_category, proj_subtype,
                                                      world_x, world_y,
                                                      map_index, 0);
    if (slot < 0) return -1;
    if (out_proj_slot) *out_proj_slot = slot;
    return cid;
}

/* ── 1. NO_CREATURE_AT_TARGET ───────────────────────────────────── */

static int test_no_creature_at_target(void) {
    dm2_v1_projectile_reset_counters();
    dm2_v1_projectile_creature_collision_reset_counters();
    dm2_v1_creature_test_clear_ai_overrides();

    /* Spawn a projectile at (3, 3) but no creature at that square. */
    int slot = dm2_v1_projectile_dispatch_synthetic(
        PROJECTILE_CATEGORY_MAGICAL,
        DM2_PROJ_SUBTYPE_MAGICAL_FIREBALL,
        3, 3, 0, 0);
    if (slot < 0) return 0;

    DM2_V1_ProjectileCreatureCollisionResult r =
        dm2_v1_projectile_creature_collision_resolve(slot, 10);

    /* No creature → INVALID outcome, accepted=0, no despawn. */
    return r.outcome == DM2_PROJ_CREATURE_OUTCOME_INVALID
        && r.accepted == 0
        && r.damage_dealt == 0
        && r.projectile_despawned == 0
        && r.creature_instance_id == -1;
}

/* ── 2. HIT, kills creature ───────────────────────────────────────
 *
 * Deterministic outcome: Archer Guard (AI 36, default armor=0 in our
 * zero-init table) at (5,5) with HP 10 takes impact_attack=20.
 *   damage = max(1, 20 - 0/2) = 20
 *   new HP = 10 - 20 = -10 → clamped to 0
 *   kill_event_emitted = 1
 *   projectile_despawned = 1
 *   counters: collision=1, kill=1
 *
 * Source-locked formula: SKULL.ASM:10620-10710. */

static int test_hit_kills_creature(void) {
    dm2_v1_projectile_reset_counters();
    dm2_v1_projectile_creature_collision_reset_counters();
    dm2_v1_creature_test_clear_ai_overrides();

    DM2_AIDefinition spec;
    build_ai_spec(&spec, 0 /* no flags */, 0 /* armor */, 10 /* HP */);

    int proj_slot = -1;
    int cid = setup_creature_with_projectile(36 /* Archer Guard */, &spec,
                                               5, 5, 0,
                                               PROJECTILE_CATEGORY_KINETIC,
                                               DM2_PROJ_SUBTYPE_KINETIC_ARROW,
                                               &proj_slot);
    if (cid < 0 || proj_slot < 0) return 0;

    /* Confirm pre-collision HP. */
    if (dm2_v1_creature_instance_hp(cid) != 10) return 0;

    DM2_V1_ProjectileCreatureCollisionResult r =
        dm2_v1_projectile_creature_collision_resolve(proj_slot, 20);

    /* Verify the deterministic outcome. */
    int ok = r.outcome == DM2_PROJ_CREATURE_OUTCOME_HIT
        && r.accepted == 1
        && r.creature_instance_id == cid
        && r.creature_ai_index == 36
        && r.damage_dealt == 20
        && r.hp_before == 10
        && r.hp_after == 0
        && r.kill_event_emitted == 1
        && r.projectile_despawned == 1
        && r.projectile_slot == proj_slot
        && r.impact_attack_input == 20
        && r.impact_attack_effective == 20
        && r.armor_class == 0;
    if (!ok) return 0;

    /* Verify post-collision state: creature dead (alive=0), slot
     * despawned (F0813 should have set slotIndex=-1). */
    const DM2_V1_CreatureInstance *c = dm2_v1_creature_get_instance(cid);
    if (!c || c->alive != 0) return 0;

    DM2_V1_ProjectileSlotSnapshot snap;
    if (dm2_v1_projectile_get_slot(proj_slot, &snap)) {
        /* slot should be empty now */
        return 0;
    }
    /* slot get returns 0 → empty, which is what we want */

    /* Verify counters. */
    if (dm2_v1_projectile_creature_collision_count() != 1) return 0;
    if (dm2_v1_projectile_creature_kill_event_count() != 1) return 0;
    return 1;
}

/* ── 3. HIT, creature survives (impact_attack < armor/2 floor) ────
 *
 * Deterministic outcome: creature at (6,6) with armor=20 and HP 50
 * takes impact_attack=8.
 *   damage = max(1, 8 - 20/2) = max(1, 8 - 10) = max(1, -2) = 1
 *   new HP = 50 - 1 = 49
 *   kill_event_emitted = 0
 *   projectile_despawned = 1
 *
 * The "floor at 1" matches the SKULL.ASM rule that any connected
 * projectile always does ≥ 1 damage (no 0-damage no-op on HIT). */

static int test_hit_creature_survives(void) {
    dm2_v1_projectile_reset_counters();
    dm2_v1_projectile_creature_collision_reset_counters();
    dm2_v1_creature_test_clear_ai_overrides();

    DM2_AIDefinition spec;
    build_ai_spec(&spec, 0, 20 /* armor */, 50 /* HP */);

    int proj_slot = -1;
    int cid = setup_creature_with_projectile(36, &spec, 6, 6, 0,
                                               PROJECTILE_CATEGORY_KINETIC,
                                               DM2_PROJ_SUBTYPE_KINETIC_ARROW,
                                               &proj_slot);
    if (cid < 0 || proj_slot < 0) return 0;

    DM2_V1_ProjectileCreatureCollisionResult r =
        dm2_v1_projectile_creature_collision_resolve(proj_slot, 8);

    int ok = r.outcome == DM2_PROJ_CREATURE_OUTCOME_HIT
        && r.accepted == 1
        && r.damage_dealt == 1
        && r.hp_before == 50
        && r.hp_after == 49
        && r.kill_event_emitted == 0
        && r.projectile_despawned == 1
        && r.armor_class == 20;
    if (!ok) return 0;

    /* Creature still alive after the hit. */
    const DM2_V1_CreatureInstance *c = dm2_v1_creature_get_instance(cid);
    return c && c->alive == 1 && c->hp_current == 49;
}

/* ── 4. NONMATERIAL ───────────────────────────────────────────────
 *
 * Source: skproject/SKWIN/DME.h:1545-1560 (w0AIFlags NONMATERIAL bit)
 * and skproject/SKULLWIN/c_creature.cpp.
 *
 * Behavior: NONMATERIAL creature (e.g., a ghost) lets the projectile
 * pass through without damage.  Projectile is NOT despawned (it
 * continues moving on the next F0811 advance tick). */

static int test_nonmaterial_passthrough(void) {
    dm2_v1_projectile_reset_counters();
    dm2_v1_projectile_creature_collision_reset_counters();
    dm2_v1_creature_test_clear_ai_overrides();

    DM2_AIDefinition spec;
    build_ai_spec(&spec, DM2_AIFLAG_NONMATERIAL, 0, 100 /* HP */);

    int proj_slot = -1;
    int cid = setup_creature_with_projectile(36, &spec, 7, 7, 0,
                                               PROJECTILE_CATEGORY_MAGICAL,
                                               DM2_PROJ_SUBTYPE_MAGICAL_FIREBALL,
                                               &proj_slot);
    if (cid < 0 || proj_slot < 0) return 0;

    DM2_V1_ProjectileCreatureCollisionResult r =
        dm2_v1_projectile_creature_collision_resolve(proj_slot, 50);

    int ok = r.outcome == DM2_PROJ_CREATURE_OUTCOME_NONMATERIAL
        && r.accepted == 1
        && r.damage_dealt == 0
        && r.hp_before == 100
        && r.hp_after == 100
        && r.kill_event_emitted == 0
        && r.projectile_despawned == 0;
    if (!ok) return 0;

    /* Projectile still alive (NOT despawned) — slot still readable. */
    DM2_V1_ProjectileSlotSnapshot snap;
    if (!dm2_v1_projectile_get_slot(proj_slot, &snap)) return 0;

    /* Counters: collision + nonmaterial, NO kill/absorb/reflect. */
    if (dm2_v1_projectile_creature_collision_count() != 1) return 0;
    if (dm2_v1_projectile_creature_nonmaterial_count() != 1) return 0;
    if (dm2_v1_projectile_creature_kill_event_count() != 0) return 0;
    if (dm2_v1_projectile_creature_absorb_count() != 0) return 0;
    if (dm2_v1_projectile_creature_reflect_count() != 0) return 0;
    return 1;
}

/* ── 5. ABSORBS_MISSILE ────────────────────────────────────────────
 *
 * Source: skproject/SKWIN/DME.h:1545-1560 (AbsorbsMissile bit).
 * Behavior: projectile is consumed (F0813 despawn) and no damage is
 * applied.  Counter: absorb_count++. */

static int test_absorbs_missile(void) {
    dm2_v1_projectile_reset_counters();
    dm2_v1_projectile_creature_collision_reset_counters();
    dm2_v1_creature_test_clear_ai_overrides();

    DM2_AIDefinition spec;
    build_ai_spec(&spec, DM2_AIFLAG_ABSORBS_MISSILE, 0, 30);

    int proj_slot = -1;
    int cid = setup_creature_with_projectile(36, &spec, 8, 8, 0,
                                               PROJECTILE_CATEGORY_KINETIC,
                                               DM2_PROJ_SUBTYPE_KINETIC_ARROW,
                                               &proj_slot);
    if (cid < 0 || proj_slot < 0) return 0;

    DM2_V1_ProjectileCreatureCollisionResult r =
        dm2_v1_projectile_creature_collision_resolve(proj_slot, 15);

    int ok = r.outcome == DM2_PROJ_CREATURE_OUTCOME_ABSORBED
        && r.accepted == 1
        && r.damage_dealt == 0
        && r.hp_before == 30
        && r.hp_after == 30
        && r.kill_event_emitted == 0
        && r.projectile_despawned == 1;
    if (!ok) return 0;

    /* Projectile despawned. */
    DM2_V1_ProjectileSlotSnapshot snap;
    if (dm2_v1_projectile_get_slot(proj_slot, &snap)) return 0;

    /* Counters. */
    if (dm2_v1_projectile_creature_absorb_count() != 1) return 0;
    if (dm2_v1_projectile_creature_kill_event_count() != 0) return 0;
    if (dm2_v1_projectile_creature_reflect_count() != 0) return 0;
    return 1;
}

/* ── 6. REFLECTOR ──────────────────────────────────────────────────
 *
 * Source: skproject/SKWIN/DME.h:1545-1560 (REFLECTOR bit) +
 * skproject/SKULLWIN/c_creature.cpp (Magick Reflector, AI 37).
 * Behavior: projectile is consumed (no damage, no event).
 *
 * Note: REFLECTOR in DM2 reflects spells back at the caster; the
 * `redirect` semantics are owned by the creature AI / spell module,
 * not this gate.  For the projectile-vs-creature collision path,
 * the projectile is consumed and the creature takes no damage. */

static int test_reflector(void) {
    dm2_v1_projectile_reset_counters();
    dm2_v1_projectile_creature_collision_reset_counters();
    dm2_v1_creature_test_clear_ai_overrides();

    DM2_AIDefinition spec;
    build_ai_spec(&spec, DM2_AIFLAG_REFLECTOR, 5, 40);

    int proj_slot = -1;
    int cid = setup_creature_with_projectile(36, &spec, 9, 9, 0,
                                               PROJECTILE_CATEGORY_MAGICAL,
                                               DM2_PROJ_SUBTYPE_MAGICAL_FIREBALL,
                                               &proj_slot);
    if (cid < 0 || proj_slot < 0) return 0;

    DM2_V1_ProjectileCreatureCollisionResult r =
        dm2_v1_projectile_creature_collision_resolve(proj_slot, 25);

    int ok = r.outcome == DM2_PROJ_CREATURE_OUTCOME_REFLECTED
        && r.accepted == 1
        && r.damage_dealt == 0
        && r.hp_before == 40
        && r.hp_after == 40
        && r.kill_event_emitted == 0
        && r.projectile_despawned == 1;
    if (!ok) return 0;

    if (dm2_v1_projectile_creature_reflect_count() != 1) return 0;
    if (dm2_v1_projectile_creature_kill_event_count() != 0) return 0;
    return 1;
}

/* ── 7. REDIRECTED (TURNS_MISSILE) ────────────────────────────────
 *
 * Source: skproject/SKWIN/SkWinCore.cpp:10479-10561 (AI_W30_TURNS_MISSILE
 * check).  Behavior in DM2: creature re-targets the projectile.  This
 * module treats REDIRECTED as the same damage path as HIT (the actual
 * redirect routing is owned by the AI module, not this gate).
 *
 * Verifies that the collision still applies damage + despawns the
 * projectile, marking the slot for the next F0811 advance tick to
 * pick up the redirect target. */

static int test_redirected_turns_missile(void) {
    dm2_v1_projectile_reset_counters();
    dm2_v1_projectile_creature_collision_reset_counters();
    dm2_v1_creature_test_clear_ai_overrides();

    DM2_AIDefinition spec;
    build_ai_spec(&spec, DM2_AI_W30_TURNS_MISSILE, 4 /* armor */, 60);

    int proj_slot = -1;
    int cid = setup_creature_with_projectile(36, &spec, 10, 10, 0,
                                               PROJECTILE_CATEGORY_MAGICAL,
                                               DM2_PROJ_SUBTYPE_MAGICAL_LIGHTNING,
                                               &proj_slot);
    if (cid < 0 || proj_slot < 0) return 0;

    DM2_V1_ProjectileCreatureCollisionResult r =
        dm2_v1_projectile_creature_collision_resolve(proj_slot, 12);

    /* damage = max(1, 12 - 4/2) = max(1, 10) = 10
     * HP = 60 - 10 = 50 */
    int ok = r.outcome == DM2_PROJ_CREATURE_OUTCOME_REDIRECTED
        && r.accepted == 1
        && r.damage_dealt == 10
        && r.hp_after == 50
        && r.kill_event_emitted == 0
        && r.projectile_despawned == 1;
    return ok;
}

/* ── 8. INVALID projectile slot ─────────────────────────────────── */

static int test_invalid_projectile_slot(void) {
    dm2_v1_projectile_creature_collision_reset_counters();

    /* Slot index -1: rejected. */
    DM2_V1_ProjectileCreatureCollisionResult r1 =
        dm2_v1_projectile_creature_collision_resolve(-1, 10);
    if (r1.accepted != 0 || r1.outcome != DM2_PROJ_CREATURE_OUTCOME_INVALID) return 0;

    /* Out-of-range slot index: rejected. */
    DM2_V1_ProjectileCreatureCollisionResult r2 =
        dm2_v1_projectile_creature_collision_resolve(9999, 10);
    if (r2.accepted != 0) return 0;

    /* Empty slot: rejected. */
    DM2_V1_ProjectileCreatureCollisionResult r3 =
        dm2_v1_projectile_creature_collision_resolve(50, 10);
    if (r3.accepted != 0) return 0;

    /* Counters must remain at zero (no collision resolved). */
    if (dm2_v1_projectile_creature_collision_count() != 0) return 0;
    return 1;
}

/* ── 9. Damage formula pinning ────────────────────────────────────
 *
 * Pin exact damage values for the deterministic formula:
 *   damage = max(1, impact_attack - armor_class / 2)
 *
 * Cases:
 *   (impact=20, armor=0)  → 20
 *   (impact=20, armor=10) → 15
 *   (impact=20, armor=20) → 10
 *   (impact=20, armor=40) → 1   (floored)
 *   (impact=1,  armor=0)  → 1
 *   (impact=1,  armor=10) → 1   (would be -4, floored to 1)
 *   (impact=0,  armor=0)  → 1   (clamped impact_attack)
 *   (impact=-5, armor=0)  → 1   (clamped impact_attack)
 *
 * Each case: spawn a creature, run the collision, assert damage_dealt.
 * No creature dies (HP is set high), so no kill event is checked. */

static int run_damage_case(int armor, int impact, int expected_damage) {
    dm2_v1_projectile_reset_counters();
    dm2_v1_projectile_creature_collision_reset_counters();
    dm2_v1_creature_test_clear_ai_overrides();

    DM2_AIDefinition spec;
    /* Use a synthetic AI index well above the real table so this
     * never collides with an existing spec.  Pick 60 (just under 64). */
    build_ai_spec(&spec, 0, (uint8_t)armor, 200);

    int proj_slot = -1;
    int cid = setup_creature_with_projectile(60, &spec,
                                               12 + (armor & 0x7),
                                               12, 0,
                                               PROJECTILE_CATEGORY_KINETIC,
                                               DM2_PROJ_SUBTYPE_KINETIC_ARROW,
                                               &proj_slot);
    if (cid < 0 || proj_slot < 0) return 0;

    DM2_V1_ProjectileCreatureCollisionResult r =
        dm2_v1_projectile_creature_collision_resolve(proj_slot, impact);

    return r.outcome == DM2_PROJ_CREATURE_OUTCOME_HIT
        && r.damage_dealt == expected_damage
        && r.armor_class == armor
        && r.kill_event_emitted == 0;
}

static int test_damage_formula_case_20_0(void) { return run_damage_case(0, 20, 20); }
static int test_damage_formula_case_20_10(void) { return run_damage_case(10, 20, 15); }
static int test_damage_formula_case_20_20(void) { return run_damage_case(20, 20, 10); }
static int test_damage_formula_case_20_40(void) { return run_damage_case(40, 20, 1); }
static int test_damage_formula_case_1_0(void)  { return run_damage_case(0, 1, 1); }
static int test_damage_formula_case_1_10(void) { return run_damage_case(10, 1, 1); }
static int test_damage_formula_case_0_0(void)  { return run_damage_case(0, 0, 1); }
static int test_damage_formula_case_neg(void)  { return run_damage_case(0, -5, 1); }

/* ── 10. Source evidence ─────────────────────────────────────────── */

static int test_source_evidence(void) {
    const char *e = dm2_v1_projectile_creature_collision_source_evidence();
    if (!e || e[0] == '\0') return 0;
    /* Must cite the key source anchors. */
    return strstr(e, "SKULL.ASM:10620-10710") != NULL
        && strstr(e, "PROJEXPL.C:76-92") != NULL
        && strstr(e, "GROUP.C:1695-1770") != NULL
        && strstr(e, "TURNS_MISSILE") != NULL
        && strstr(e, "deterministic") != NULL;
}

/* ── 11. Reset counters ─────────────────────────────────────────── */

static int test_reset_counters(void) {
    dm2_v1_projectile_creature_collision_reset_counters();
    /* Generate one of each event. */
    DM2_AIDefinition spec;
    int slot, cid;

    /* HIT (non-kill). */
    build_ai_spec(&spec, 0, 0, 50);
    cid = setup_creature_with_projectile(60, &spec, 1, 1, 0,
                                          PROJECTILE_CATEGORY_KINETIC,
                                          DM2_PROJ_SUBTYPE_KINETIC_ARROW,
                                          &slot);
    if (cid >= 0) dm2_v1_projectile_creature_collision_resolve(slot, 5);

    /* NONMATERIAL. */
    build_ai_spec(&spec, DM2_AIFLAG_NONMATERIAL, 0, 50);
    cid = setup_creature_with_projectile(60, &spec, 2, 2, 0,
                                          PROJECTILE_CATEGORY_MAGICAL,
                                          DM2_PROJ_SUBTYPE_MAGICAL_FIREBALL,
                                          &slot);
    if (cid >= 0) dm2_v1_projectile_creature_collision_resolve(slot, 5);

    /* ABSORB. */
    build_ai_spec(&spec, DM2_AIFLAG_ABSORBS_MISSILE, 0, 50);
    cid = setup_creature_with_projectile(60, &spec, 3, 3, 0,
                                          PROJECTILE_CATEGORY_KINETIC,
                                          DM2_PROJ_SUBTYPE_KINETIC_ARROW,
                                          &slot);
    if (cid >= 0) dm2_v1_projectile_creature_collision_resolve(slot, 5);

    /* REFLECT. */
    build_ai_spec(&spec, DM2_AIFLAG_REFLECTOR, 0, 50);
    cid = setup_creature_with_projectile(60, &spec, 4, 4, 0,
                                          PROJECTILE_CATEGORY_MAGICAL,
                                          DM2_PROJ_SUBTYPE_MAGICAL_FIREBALL,
                                          &slot);
    if (cid >= 0) dm2_v1_projectile_creature_collision_resolve(slot, 5);

    /* Now reset and verify all counters are zero. */
    dm2_v1_projectile_creature_collision_reset_counters();
    return dm2_v1_projectile_creature_collision_count() == 0
        && dm2_v1_projectile_creature_kill_event_count() == 0
        && dm2_v1_projectile_creature_absorb_count() == 0
        && dm2_v1_projectile_creature_reflect_count() == 0
        && dm2_v1_projectile_creature_nonmaterial_count() == 0;
}

int main(void) {
    printf("DM2 V1 Projectile-vs-Creature Collision — Phase 5 source-lock tests\n");
    printf("Source: SKULL.ASM:10620-10710, ReDMCSB PROJEXPL.C:76-92,\n"
           "        skproject/SKULLWIN/c_combat.cpp:401-420,\n"
           "        skproject/SKWIN/DME.h:1545-1560 (w0AIFlags)\n");

    /* Branches. */
    TEST(no_creature_at_target);
    TEST(hit_kills_creature);
    TEST(hit_creature_survives);
    TEST(nonmaterial_passthrough);
    TEST(absorbs_missile);
    TEST(reflector);
    TEST(redirected_turns_missile);
    TEST(invalid_projectile_slot);

    /* Damage formula pinning. */
    TEST(damage_formula_case_20_0);
    TEST(damage_formula_case_20_10);
    TEST(damage_formula_case_20_20);
    TEST(damage_formula_case_20_40);
    TEST(damage_formula_case_1_0);
    TEST(damage_formula_case_1_10);
    TEST(damage_formula_case_0_0);
    TEST(damage_formula_case_neg);

    /* Evidence + counters. */
    TEST(source_evidence);
    TEST(reset_counters);

    printf("\n%d/%d tests passed\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}

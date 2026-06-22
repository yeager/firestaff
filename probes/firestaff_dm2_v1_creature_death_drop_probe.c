/* firestaff_dm2_v1_creature_death_drop_probe.c — DM2 V1 Creature Death/Drop Probe
 *
 * Phase 5 (creature/combat parity) followup — 2026-06-22.
 * Headless, data-free probe that exercises the deterministic creature
 * death → drop state contract:
 *
 *   - dm2_v1_creature_death_check populates the death/drop observer with
 *     the killed creature's identity (slot, AI, world coords, map) and the
 *     resulting loot state (item_id, count, dropped).
 *   - Thorn Demon (AI 19) drops DM2_DROP_THORN_DEMON_WORM_FOOD count=1.
 *   - Other AIs drop nothing (dropped=0) but still record the death.
 *   - Out-of-range instance ids, dead-creature re-checks, and positive-HP
 *     calls are all rejected without polluting the observer.
 *
 * The probe is a sibling of the CTest gate
 * `test_dm2_v1_creature_death_drop_pc34_compat` — same coverage, no
 * CTest harness, suitable for the firestaff pool workers.
 *
 * Source-lock anchors (see test_dm2_v1_creature_death_drop_pc34_compat.c
 * for the full citation list):
 *   SKULL.ASM (sha256 a2a04b0e...)
 *   SKULLWIN/c_creature.cpp DM2_PROCEED_CCM (CCM b_1a dispatch)
 *   SKULLWIN/c_creature.h b_1a / b_17 fields
 *   skproject/SKWIN/SkWinCore.cpp:16815-16936 ALLOC_NEW_CREATURE
 *   SKWin.GDAT2.InternalCodes.txt creature category 0x0A (11 drop slots)
 *   docs/dm2_characters.md Thorn Demon worm food
 */

#include "dm2_v1_creature.h"
#include "dm2_v1_drops.h"
#include "dm2_v1_sound.h"

#include <stdio.h>
#include <string.h>

static int g_assertions = 0;
static int g_failures   = 0;

#define CHECK(cond_) do { \
    g_assertions++; \
    if (!(cond_)) { \
        printf("  FAIL  %s:%d  %s\n", __FILE__, __LINE__, #cond_); \
        g_failures++; \
    } \
} while (0)

#define CHECK_EQ(actual_, expected_) do { \
    g_assertions++; \
    int _a = (int)(actual_); \
    int _e = (int)(expected_); \
    if (_a != _e) { \
        printf("  FAIL  %s:%d  expected %d got %d\n", \
               __FILE__, __LINE__, _e, _a); \
        g_failures++; \
    } \
} while (0)

/* ── Thorn Demon → worm food ─────────────────────────────────────── */

static void test_thorn_demon_worm_food(void)
{
    printf("--- Thorn Demon → worm food ---\n");
    dm2_v1_creature_reset_death_observer();

    int slot = dm2_v1_creature_spawn(DM2_AI_THORN_DEMON, 5, 10, 0, 1, 0);
    CHECK(slot >= 0);

    dm2_v1_creature_deal_damage(slot, 9999);
    dm2_v1_creature_tick();

    DM2_V1_CreatureDeathDropObserver obs;
    CHECK_EQ(dm2_v1_creature_last_death_drop(&obs), 1);
    CHECK_EQ(obs.dropped, 1);
    CHECK_EQ(obs.item_id, DM2_DROP_THORN_DEMON_WORM_FOOD);
    CHECK_EQ(obs.count, 1);
    CHECK_EQ(obs.instance_id, slot);
    CHECK_EQ(obs.ai_index, DM2_AI_THORN_DEMON);
    CHECK_EQ(obs.world_x, 5);
    CHECK_EQ(obs.world_y, 10);
    CHECK_EQ(obs.map_index, 0);
    CHECK_EQ(dm2_v1_creature_death_observer_count(), 1);
}

/* ── Cavern Bat → no drop ─────────────────────────────────────────── */

static void test_non_thorn_demon_no_drop(void)
{
    printf("--- Cavern Bat → no drop ---\n");
    dm2_v1_creature_reset_death_observer();

    int slot = dm2_v1_creature_spawn(DM2_AI_CAVE_BAT, 3, 4, 1, 2, 0);
    CHECK(slot >= 0);

    dm2_v1_creature_deal_damage(slot, 9999);
    dm2_v1_creature_tick();

    DM2_V1_CreatureDeathDropObserver obs;
    CHECK_EQ(dm2_v1_creature_last_death_drop(&obs), 1);
    CHECK_EQ(obs.dropped, 0);
    CHECK_EQ(obs.item_id, 0);
    CHECK_EQ(obs.count, 0);
    CHECK_EQ(obs.instance_id, slot);
    CHECK_EQ(obs.ai_index, DM2_AI_CAVE_BAT);
    CHECK_EQ(obs.map_index, 1);
    CHECK_EQ(dm2_v1_creature_death_observer_count(), 1);
}

/* ── Out-of-range + dead re-check + alive re-check are no-ops ─────── */

static void test_rejection_paths(void)
{
    printf("--- rejection paths (no observer pollution) ---\n");
    dm2_v1_creature_reset_death_observer();

    /* Out-of-range ids. */
    dm2_v1_creature_death_check(-1);
    dm2_v1_creature_death_check(DM2_MAX_CREATURE_INSTANCES);
    dm2_v1_creature_death_check(9999);

    DM2_V1_CreatureDeathDropObserver obs;
    CHECK_EQ(dm2_v1_creature_last_death_drop(&obs), 0);
    CHECK_EQ(dm2_v1_creature_death_observer_count(), 0);

    /* Alive creature with HP > 0 — must not fire. */
    int slot = dm2_v1_creature_spawn(DM2_AI_THORN_DEMON, 0, 0, 0, 0, 0);
    CHECK(slot >= 0);
    dm2_v1_creature_death_check(slot);
    CHECK_EQ(dm2_v1_creature_death_observer_count(), 0);

    /* Kill via tick → death_check fires once. */
    dm2_v1_creature_deal_damage(slot, 9999);
    dm2_v1_creature_tick();
    CHECK_EQ(dm2_v1_creature_death_observer_count(), 1);

    /* Re-call on already-dead slot must NOT increment. */
    dm2_v1_creature_death_check(slot);
    dm2_v1_creature_death_check(slot);
    CHECK_EQ(dm2_v1_creature_death_observer_count(), 1);
}

/* ── Reset clears observer + count ────────────────────────────────── */

static void test_reset(void)
{
    printf("--- reset clears observer + count ---\n");

    int slot = dm2_v1_creature_spawn(DM2_AI_THORN_DEMON, 7, 8, 0, 0, 0);
    CHECK(slot >= 0);
    dm2_v1_creature_deal_damage(slot, 9999);
    dm2_v1_creature_tick();
    CHECK(dm2_v1_creature_death_observer_count() > 0);

    dm2_v1_creature_reset_death_observer();

    DM2_V1_CreatureDeathDropObserver obs;
    /* Pre-fill with garbage to verify zero-init. */
    memset(&obs, 0xAA, sizeof(obs));

    CHECK_EQ(dm2_v1_creature_last_death_drop(&obs), 0);
    CHECK_EQ(obs.instance_id, 0);
    CHECK_EQ(obs.ai_index, 0);
    CHECK_EQ(obs.world_x, 0);
    CHECK_EQ(obs.world_y, 0);
    CHECK_EQ(obs.map_index, 0);
    CHECK_EQ(obs.dropped, 0);
    CHECK_EQ(obs.item_id, 0);
    CHECK_EQ(obs.count, 0);
    CHECK_EQ(dm2_v1_creature_death_observer_count(), 0);
}

/* ── Monotonic counter across multiple deaths ─────────────────────── */

static void test_monotonic_count(void)
{
    printf("--- monotonic count across multiple deaths ---\n");
    dm2_v1_creature_reset_death_observer();

    int s1 = dm2_v1_creature_spawn(DM2_AI_THORN_DEMON, 1, 1, 0, 0, 0);
    int s2 = dm2_v1_creature_spawn(DM2_AI_CAVE_BAT,    2, 2, 0, 1, 0);
    CHECK(s1 >= 0);
    CHECK(s2 >= 0);
    CHECK_EQ(dm2_v1_creature_death_observer_count(), 0);

    dm2_v1_creature_deal_damage(s1, 9999);
    dm2_v1_creature_tick();
    CHECK_EQ(dm2_v1_creature_death_observer_count(), 1);

    dm2_v1_creature_deal_damage(s2, 9999);
    dm2_v1_creature_tick();
    CHECK_EQ(dm2_v1_creature_death_observer_count(), 2);

    /* Observer holds the most recent death (Cavern Bat). */
    DM2_V1_CreatureDeathDropObserver obs;
    CHECK_EQ(dm2_v1_creature_last_death_drop(&obs), 1);
    CHECK_EQ(obs.ai_index, DM2_AI_CAVE_BAT);
    CHECK_EQ(obs.instance_id, s2);
}

/* ── Source-evidence + magic-number sanity ────────────────────────── */

static void test_source_and_constants(void)
{
    printf("--- source evidence + constants ---\n");
    const char *e = dm2_v1_creature_source_evidence();
    CHECK(e != NULL && e[0] != '\0' && strlen(e) > 10);
    CHECK_EQ(DM2_SOUND_CREATURE_DEATH, 0x11);
    CHECK_EQ(DM2_DROP_SLOT_COUNT, 11);
    CHECK_EQ(DM2_DROP_SLOT_FIRST, 10);
    CHECK_EQ(DM2_DROP_SLOT_LAST, 20);
    CHECK_EQ(DM2_DROP_THORN_DEMON_WORM_FOOD, 1);
    CHECK_EQ(DM2_AI_THORN_DEMON, 19);
    CHECK_EQ(DM2_AI_CAVE_BAT, 23);
}

int main(void)
{
    printf("=== DM2 V1 creature death/drop probe ===\n");
    printf("Source: SKULL.ASM + SKWin.GDAT2.InternalCodes.txt +\n"
           "        skproject/SKWIN/SkWinCore.cpp:16815-16936 +\n"
           "        SKULLWIN/c_creature.cpp DM2_PROCEED_CCM +\n"
           "        docs/dm2_characters.md Thorn Demon worm food.\n\n");

    test_thorn_demon_worm_food();
    test_non_thorn_demon_no_drop();
    test_rejection_paths();
    test_reset();
    test_monotonic_count();
    test_source_and_constants();

    printf("\n=== %d/%d assertions passed (%d failures) ===\n",
           g_assertions - g_failures, g_assertions, g_failures);
    if (g_failures > 0) {
        printf("PROBE FAILED\n");
        return 1;
    }
    printf("PROBE PASSED\n");
    return 0;
}

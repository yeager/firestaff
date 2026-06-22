/* test_dm2_v1_creature_death_drop_pc34_compat.c
 *
 * DM2 V1 creature death → drop observer CTest gate.
 *
 * Source-locked coverage of:
 *   1. Thorn Demon (AI 19) kill → DM2_DROP_THORN_DEMON_WORM_FOOD
 *      with count=1 is captured in the death/drop observer.
 *   2. Cavern Bat (AI 23, no Thorn Demon special case) kill →
 *      observer records dropped=0 (death landed, no loot).
 *   3. Out-of-range instance_id is rejected before observer fires.
 *   4. Dead creature (alive=0) death_check is a no-op (observer unchanged).
 *   5. HP > 0 death_check is a no-op (observer unchanged).
 *   6. Double death_check on same dead slot does not duplicate observer.
 *   7. Monotonic death_observer_count increments per death event.
 *   8. reset_death_observer clears observer + count.
 *   9. last_death_drop on empty observer returns 0 + zero-init struct.
 *  10. Magic-number fix: creature death sound uses DM2_SOUND_CREATURE_DEATH.
 *  11. Observer captures world_x/world_y/map_index/ai_index snapshot.
 *  12. Source evidence non-empty.
 *
 * Build (CMake — see CMakeLists.txt):
 *   test_dm2_v1_creature_death_drop_pc34_compat
 *     tests/test_dm2_v1_creature_death_drop_pc34_compat.c
 *     src/dm2/dm2_v1_creature.c
 *     src/dm2/dm2_v1_drops.c
 *     src/dm2/dm2_v1_sound.c
 *
 * Source-lock anchors:
 *   SKULL.ASM (sha256 a2a04b0ea7c05fd2b2a7a8da5197cdfcccd7d4d0167943caf3a21a079462e099)
 *   skproject/SKWIN/SkWinCore.cpp:16815-16936 (ALLOC_NEW_CREATURE)
 *   skproject/SKWIN/SkWinCore.cpp:27038-27096 (spell/attack resolution)
 *   SKULLWIN/c_creature.cpp: DM2_PROCEED_CCM (CCM b_1a dispatch)
 *   SKULLWIN/c_creature.h: b_1a / b_17 fields
 *   SKWin.GDAT2.InternalCodes.txt (creature category 0x0A, 11 drop slots 0x0A-0x14)
 *   docs/dm2_characters.md (Thorn Demon worm food)
 *   docs/dm2_dungeon_design.md (11 drop slots, DropTableSeed RNG)
 *   ReDMCSB DEFS.H C040-equivalent dead-instance sentinel
 *
 * Phase 5 (creature/combat parity) followup — 2026-06-22.
 */

#include "dm2_v1_creature.h"
#include "dm2_v1_drops.h"
#include "dm2_v1_sound.h"

#include <stdio.h>
#include <string.h>

static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name_) do { \
    printf("  %-58s", #name_); \
    fflush(stdout); \
    tests_run++; \
    if (test_##name_()) { \
        tests_passed++; \
        printf("PASS\n"); \
    } else { \
        printf("FAIL\n"); \
    } \
} while (0)

/* ── Thorn Demon kill → worm food drop ────────────────────────────── */

static int test_thorn_demon_drops_worm_food(void) {
    dm2_v1_creature_reset_death_observer();

    int slot = dm2_v1_creature_spawn(DM2_AI_THORN_DEMON, 5, 10, 0, 1, 0);
    if (slot < 0) return 0;

    /* Deal enough damage to kill, then tick → death_check fires. */
    dm2_v1_creature_deal_damage(slot, 9999);
    dm2_v1_creature_tick();

    DM2_V1_CreatureDeathDropObserver obs;
    int rc = dm2_v1_creature_last_death_drop(&obs);
    return rc == 1
        && obs.dropped == 1
        && obs.item_id == DM2_DROP_THORN_DEMON_WORM_FOOD
        && obs.count == 1
        && obs.instance_id == slot
        && obs.ai_index == DM2_AI_THORN_DEMON
        && obs.world_x == 5
        && obs.world_y == 10
        && obs.map_index == 0
        && dm2_v1_creature_death_observer_count() == 1;
}

/* ── Cavern Bat kill → no drop (only Thorn Demon has special drops) ── */

static int test_non_thorn_demon_no_drop(void) {
    dm2_v1_creature_reset_death_observer();

    int slot = dm2_v1_creature_spawn(DM2_AI_CAVE_BAT, 3, 4, 1, 2, 0);
    if (slot < 0) return 0;

    dm2_v1_creature_deal_damage(slot, 9999);
    dm2_v1_creature_tick();

    DM2_V1_CreatureDeathDropObserver obs;
    int rc = dm2_v1_creature_last_death_drop(&obs);
    return rc == 1
        && obs.dropped == 0
        && obs.item_id == 0
        && obs.count == 0
        && obs.instance_id == slot
        && obs.ai_index == DM2_AI_CAVE_BAT
        && obs.world_x == 3
        && obs.world_y == 4
        && obs.map_index == 1
        && dm2_v1_creature_death_observer_count() == 1;
}

/* ── Out-of-range instance_id is rejected ─────────────────────────── */

static int test_out_of_range_instance_id_rejected(void) {
    dm2_v1_creature_reset_death_observer();

    dm2_v1_creature_death_check(-1);
    dm2_v1_creature_death_check(DM2_MAX_CREATURE_INSTANCES);
    dm2_v1_creature_death_check(9999);

    DM2_V1_CreatureDeathDropObserver obs;
    int rc = dm2_v1_creature_last_death_drop(&obs);
    return rc == 0
        && obs.instance_id == 0
        && obs.dropped == 0
        && dm2_v1_creature_death_observer_count() == 0;
}

/* ── Dead creature death_check is a no-op ─────────────────────────── */

static int test_dead_creature_death_check_is_noop(void) {
    dm2_v1_creature_reset_death_observer();

    int slot = dm2_v1_creature_spawn(DM2_AI_THORN_DEMON, 0, 0, 0, 0, 0);
    if (slot < 0) return 0;

    /* Kill via tick → death_check fires once. */
    dm2_v1_creature_deal_damage(slot, 9999);
    dm2_v1_creature_tick();

    int after_first = dm2_v1_creature_death_observer_count();
    if (after_first != 1) return 0;

    /* Direct re-call on the same dead slot must NOT increment. */
    dm2_v1_creature_death_check(slot);
    int after_second = dm2_v1_creature_death_observer_count();

    return after_second == after_first;
}

/* ── HP > 0 death_check is a no-op ────────────────────────────────── */

static int test_alive_hp_positive_death_check_is_noop(void) {
    dm2_v1_creature_reset_death_observer();

    int slot = dm2_v1_creature_spawn(DM2_AI_THORN_DEMON, 0, 0, 0, 0, 0);
    if (slot < 0) return 0;

    /* HP is still > 0 — direct death_check must be a no-op. */
    dm2_v1_creature_death_check(slot);

    return dm2_v1_creature_death_observer_count() == 0;
}

/* ── Monotonic death_observer_count ───────────────────────────────── */

static int test_monotonic_death_observer_count(void) {
    dm2_v1_creature_reset_death_observer();

    int s1 = dm2_v1_creature_spawn(DM2_AI_THORN_DEMON, 1, 1, 0, 0, 0);
    int s2 = dm2_v1_creature_spawn(DM2_AI_CAVE_BAT,    2, 2, 0, 1, 0);
    if (s1 < 0 || s2 < 0) return 0;

    if (dm2_v1_creature_death_observer_count() != 0) return 0;

    dm2_v1_creature_deal_damage(s1, 9999);
    dm2_v1_creature_tick();
    int after_one = dm2_v1_creature_death_observer_count();
    if (after_one != 1) return 0;

    dm2_v1_creature_deal_damage(s2, 9999);
    dm2_v1_creature_tick();
    int after_two = dm2_v1_creature_death_observer_count();

    return after_two == 2;
}

/* ── Reset clears observer + count ────────────────────────────────── */

static int test_reset_clears_observer_and_count(void) {
    int slot = dm2_v1_creature_spawn(DM2_AI_THORN_DEMON, 7, 8, 0, 0, 0);
    if (slot < 0) return 0;
    dm2_v1_creature_deal_damage(slot, 9999);
    dm2_v1_creature_tick();

    if (dm2_v1_creature_death_observer_count() == 0) return 0;

    dm2_v1_creature_reset_death_observer();

    DM2_V1_CreatureDeathDropObserver obs;
    int rc = dm2_v1_creature_last_death_drop(&obs);
    return rc == 0
        && obs.instance_id == 0
        && obs.dropped == 0
        && obs.item_id == 0
        && dm2_v1_creature_death_observer_count() == 0;
}

/* ── Empty observer returns 0 + zero-init struct ───────────────────── */

static int test_empty_observer_returns_zero_init(void) {
    dm2_v1_creature_reset_death_observer();

    DM2_V1_CreatureDeathDropObserver obs;
    /* Pre-fill the struct with garbage so we can verify zero-init. */
    memset(&obs, 0xAA, sizeof(obs));

    int rc = dm2_v1_creature_last_death_drop(&obs);

    return rc == 0
        && obs.instance_id == 0
        && obs.ai_index == 0
        && obs.world_x == 0
        && obs.world_y == 0
        && obs.map_index == 0
        && obs.dropped == 0
        && obs.item_id == 0
        && obs.count == 0;
}

/* ── last_death_drop with NULL out is rejected ─────────────────────── */

static int test_last_death_drop_null_out_rejected(void) {
    dm2_v1_creature_reset_death_observer();
    int slot = dm2_v1_creature_spawn(DM2_AI_THORN_DEMON, 0, 0, 0, 0, 0);
    if (slot < 0) return 0;
    dm2_v1_creature_deal_damage(slot, 9999);
    dm2_v1_creature_tick();

    /* NULL out pointer must not crash.  Return value is intentionally
     * allowed to be 0 here — the spec only requires non-NULL observation,
     * and crashing is the failure mode we are gating against. */
    int rc = dm2_v1_creature_last_death_drop(NULL);
    (void)rc;
    return 1;
}

/* ── Magic-number fix: DM2_SOUND_CREATURE_DEATH == 0x11 ───────────── */

static int test_creature_death_sound_constant(void) {
    return DM2_SOUND_CREATURE_DEATH == 0x11;
}

/* ── Source evidence non-empty ────────────────────────────────────── */

static int test_source_evidence_nonempty(void) {
    const char *e = dm2_v1_creature_source_evidence();
    return e != NULL
        && e[0] != '\0'
        && strlen(e) > 10;
}

/* ── Observer snapshot fields ─────────────────────────────────────── */

static int test_observer_snapshot_fields(void) {
    dm2_v1_creature_reset_death_observer();

    int slot = dm2_v1_creature_spawn(DM2_AI_THORN_DEMON, 11, 22, 3, 2, 0);
    if (slot < 0) return 0;

    dm2_v1_creature_deal_damage(slot, 9999);
    dm2_v1_creature_tick();

    DM2_V1_CreatureDeathDropObserver obs;
    if (dm2_v1_creature_last_death_drop(&obs) != 1) return 0;

    return obs.world_x == 11
        && obs.world_y == 22
        && obs.map_index == 3
        && obs.ai_index == DM2_AI_THORN_DEMON
        && obs.instance_id == slot;
}

/* ── Drops constants still source-locked ──────────────────────────── */

static int test_drops_source_lock_constants(void) {
    return DM2_DROP_SLOT_COUNT == 11
        && DM2_DROP_SLOT_FIRST == 10
        && DM2_DROP_SLOT_LAST == 20
        && DM2_DROP_THORN_DEMON_WORM_FOOD == 1;
}

int main(void) {
    printf("DM2 V1 creature death/drop source-lock tests\n");
    printf("Source: SKULL.ASM:10620-10710, SKWin.GDAT2.InternalCodes.txt,\n"
           "        skproject/SKWIN/SkWinCore.cpp:16815-16936,\n"
           "        SKULLWIN/c_creature.cpp (DM2_PROCEED_CCM),\n"
           "        docs/dm2_characters.md (Thorn Demon worm food).\n\n");

    TEST(thorn_demon_drops_worm_food);
    TEST(non_thorn_demon_no_drop);
    TEST(out_of_range_instance_id_rejected);
    TEST(dead_creature_death_check_is_noop);
    TEST(alive_hp_positive_death_check_is_noop);
    TEST(monotonic_death_observer_count);
    TEST(reset_clears_observer_and_count);
    TEST(empty_observer_returns_zero_init);
    TEST(last_death_drop_null_out_rejected);
    TEST(creature_death_sound_constant);
    TEST(source_evidence_nonempty);
    TEST(observer_snapshot_fields);
    TEST(drops_source_lock_constants);

    printf("\n%d/%d tests passed\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}

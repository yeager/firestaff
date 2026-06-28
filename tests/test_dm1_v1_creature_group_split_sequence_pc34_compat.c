/*
 * DM1 V1 creature group split sequence gate.
 *
 * Multi-step regression for the source-locked F0190 group transitions:
 * a 4-creature group hit by three consecutive lethal attacks compacts
 * deterministically to a single creature (and finally to KILLED_ALL).
 *
 * Source: ReDMCSB Toolchains/Common/Source/GROUP.C F0190 lines 892-940.
 *
 * This test pins every distinct F0738_COMBAT_ApplyDamageToGroup_Compat
 * transition path on a 4-creature group without widening creature AI:
 *   1. Middle-slot kill  → loop compacts slots 1..count-1 down.
 *   2. Tail-slot kill    → compact loop body never executes; cells is
 *                            masked with 0x3F but slot data at count is
 *                            already discarded.
 *   3. Head-slot kill    → compact loop body never executes; cells is
 *                            masked with 0x3F and the previous slot 0
 *                            remains the surviving creature.
 *   4. Final creature    → count == 0 branch returns KILLED_ALL without
 *                            touching Health, Cells, or Count.
 *
 * The four transitions together prove the deterministic group-state
 * machine F0738 exposes, with the explicit non-goal that F0738 must
 * not change group->Behavior (fear/flee lives in F0190/F0821 outside
 * this helper).
 */

#include <stdio.h>
#include <string.h>

#include "dm1_v1_creature_ai_behavior_pc34_compat.h"
#include "memory_combat_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"

static int fail_count = 0;

#define CHECK(cond, msg) do { \
    if (!(cond)) { \
        printf("FAIL: %s\n", msg); \
        fail_count++; \
    } \
} while (0)

/*
 * Helper: 4-cell byte layout used by DungeonGroup_Compat.cells.
 * Each cell occupies 2 bits; ReDMCSB M050_CREATURE_VALUE reads
 * (packed >> (idx << 1)) & 0x03.
 */
static unsigned char pack4(int c0, int c1, int c2, int c3) {
    return (unsigned char)((c0 << 0) | (c1 << 2) | (c2 << 4) | (c3 << 6));
}

static void init_group(struct DungeonGroup_Compat* group) {
    memset(group, 0, sizeof(*group));
    /* ReDMCSB stores Count as actual count - 1; a 4-creature group
     * has Count == 3. */
    group->count = 3;
    group->cells = pack4(0, 1, 2, 3);
    group->health[0] = 44;
    group->health[1] = 12;
    group->health[2] = 36;
    group->health[3] = 28;
    group->behavior = 6;  /* C6_BEHAVIOR_ATTACK in ReDMCSB GROUP.C */
    group->direction = 2;
}

int main(void) {
    struct DungeonGroup_Compat group;
    struct CombatResult_Compat result;
    int outcome = -1;
    int rc = 0;

    /* ============================================================
     * Step 1: 4-creature group, kill middle slot (index 1, HP 12).
     *
     * Expected outcome: KILLED_SOME_CREATURES. The compact loop
     * `for (i = creatureIndex; i < group.count; i++)` runs for
     * i = 1, 2. Health[1]=36, Health[2]=28; cells slot 1 ← slot 2,
     * cells slot 2 ← slot 3 (which is 3). The mask `cells & 0x3F`
     * then zeroes the surviving top 2 bits. Count drops 3 → 2.
     * ============================================================ */
    init_group(&group);
    memset(&result, 0, sizeof(result));
    result.damageApplied = 12;

    rc = F0738_COMBAT_ApplyDamageToGroup_Compat(&result, &group, 1, &outcome);
    CHECK(rc == 1, "step1: F0738 should accept the middle-slot hit");
    CHECK(outcome == COMBAT_OUTCOME_KILLED_SOME_CREATURES,
          "step1: middle-slot kill should report KILLED_SOME_CREATURES");
    CHECK(group.count == 2,
          "step1: 4-creature group should compact to 3 creatures");
    CHECK(group.health[0] == 44 && group.health[1] == 36 && group.health[2] == 28,
          "step1: surviving health should compact to slots 0..2");
    CHECK(group.cells == pack4(0, 2, 3, 0),
          "step1: cells should compact to live cells 0,2,3 with slot 3 zeroed");
    CHECK(group.behavior == 6,
          "step1: F0738 must not change group behavior (fear/flee lives in F0190)");
    CHECK(group.direction == 2,
          "step1: F0738 must not change base group direction (single-direction field)");

    /* ============================================================
     * Step 2: 3-creature group, kill tail slot (index 2, HP 28).
     *
     * Expected outcome: KILLED_SOME_CREATURES. The compact loop
     * `for (i = 2; i < 2; i++)` never executes — slot 2 is the
     * last valid slot. cells = original cells (0x38), masked to
     * 0x3F → 0x38 (slot 3 was already zero). Count drops 2 → 1.
     * ============================================================ */
    memset(&result, 0, sizeof(result));
    result.damageApplied = 100;

    rc = F0738_COMBAT_ApplyDamageToGroup_Compat(&result, &group, 2, &outcome);
    CHECK(rc == 1, "step2: F0738 should accept the tail-slot hit");
    CHECK(outcome == COMBAT_OUTCOME_KILLED_SOME_CREATURES,
          "step2: tail-slot kill should report KILLED_SOME_CREATURES");
    CHECK(group.count == 1,
          "step2: 3-creature group should compact to 2 creatures");
    CHECK(group.health[0] == 44 && group.health[1] == 36,
          "step2: surviving health should remain slots 0,1");
    CHECK(group.cells == pack4(0, 2, 3, 0),
          "step2: tail-slot kill should leave cells 0x3F-masked unchanged");
    CHECK(group.behavior == 6,
          "step2: F0738 must not change group behavior");

    /* ============================================================
     * Step 3: 2-creature group, kill head slot (index 0, HP 44).
     *
     * Expected outcome: KILLED_SOME_CREATURES. The compact loop
     * `for (i = 0; i < 1; i++)` runs for i = 0; Health[0] ← Health[1]
     * (= 36); cells slot 0 ← cells slot 1 (= 2). Cells becomes
     * pack4(2, 2, 3, 0) = 0x3A; mask `& 0x3F` keeps slots 0..2
     * (slot 3 sits at bits 6-7, which fall off the 0x3F mask).
     * Count drops 1 → 0.
     * ============================================================ */
    memset(&result, 0, sizeof(result));
    result.damageApplied = 100;

    rc = F0738_COMBAT_ApplyDamageToGroup_Compat(&result, &group, 0, &outcome);
    CHECK(rc == 1, "step3: F0738 should accept the head-slot hit");
    CHECK(outcome == COMBAT_OUTCOME_KILLED_SOME_CREATURES,
          "step3: head-slot kill should report KILLED_SOME_CREATURES");
    CHECK(group.count == 0,
          "step3: 2-creature group should compact to 1 creature");
    CHECK(group.health[0] == 36 && group.health[1] == 36,
          "step3: surviving creature's health should be slot 1's 36");
    CHECK(group.cells == pack4(2, 2, 3, 0),
          "step3: cells should compact slot 1's value into slot 0");
    CHECK(group.behavior == 6,
          "step3: F0738 must not change group behavior");

    /* ============================================================
     * Step 4: 1-creature group, kill the last slot (index 0, HP 36).
     *
     * Expected outcome: KILLED_ALL_CREATURES. The `count == 0` branch
     * short-circuits — no compact loop, no cells mask. Health[0] is
     * set to 0; group->behavior is unchanged.
     * ============================================================ */
    memset(&result, 0, sizeof(result));
    result.damageApplied = 100;

    rc = F0738_COMBAT_ApplyDamageToGroup_Compat(&result, &group, 0, &outcome);
    CHECK(rc == 1, "step4: F0738 should accept the final-creature hit");
    CHECK(outcome == COMBAT_OUTCOME_KILLED_ALL_CREATURES,
          "step4: last-creature kill should report KILLED_ALL_CREATURES");
    CHECK(group.count == 0,
          "step4: count should remain 0 (single creature already)");
    CHECK(group.health[0] == 0,
          "step4: last surviving creature's health should be zeroed");
    CHECK(group.cells == pack4(2, 2, 3, 0),
          "step4: KILLED_ALL branch must not mutate cells");
    CHECK(group.behavior == 6,
          "step4: KILLED_ALL branch must not mutate behavior");

    /* ============================================================
     * Non-lethal follow-up: a 0-damage hit on a 0-HP slot does NOT
     * take the `if (slotHp > damage)` true branch (0 > 0 is false),
     * so the else branch runs. With count == 0, F0738 returns
     * KILLED_ALL_CREATURES without touching cells or count.
     *
     * This pins the negative path: a refactor that re-routed an
     * empty group back into the compact branch would silently
     * underflow count.
     * ============================================================ */
    memset(&result, 0, sizeof(result));
    result.damageApplied = 0;

    rc = F0738_COMBAT_ApplyDamageToGroup_Compat(&result, &group, 0, &outcome);
    CHECK(rc == 1, "step5: zero-damage hit on empty slot should still return 1");
    CHECK(outcome == COMBAT_OUTCOME_KILLED_ALL_CREATURES,
          "step5: zero damage on a 0-HP slot of an empty group is KILLED_ALL");
    CHECK(group.count == 0,
          "step5: empty group must not underflow count");

    if (fail_count == 0) {
        printf("PASS: DM1 V1 creature group split sequence gate\n");
    }
    return fail_count == 0 ? 0 : 1;
}

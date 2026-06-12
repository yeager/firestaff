/*
 * DM1 V1 creature group split gate.
 *
 * Regression for the source-locked F0190 group transition: a lethal hit on
 * one creature in a multi-creature group compacts the remaining group state.
 */

#include <stdio.h>
#include <string.h>

#include "memory_combat_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"

static int fail_count = 0;

#define CHECK(cond, msg) do { \
    if (!(cond)) { \
        printf("FAIL: %s\n", msg); \
        fail_count++; \
    } \
} while (0)

int main(void) {
    struct DungeonGroup_Compat group;
    struct CombatResult_Compat result;
    int outcome = -1;

    memset(&group, 0, sizeof(group));
    memset(&result, 0, sizeof(result));

    group.count = 3; /* ReDMCSB Count stores actual creature count minus one. */
    group.cells = (unsigned char)((0 << 0) | (1 << 2) | (2 << 4) | (3 << 6));
    group.health[0] = 44;
    group.health[1] = 12;
    group.health[2] = 36;
    group.health[3] = 28;
    group.behavior = 6;  /* C6_BEHAVIOR_ATTACK in ReDMCSB GROUP.C. */
    group.direction = 2;
    result.damageApplied = 12;

    CHECK(F0738_COMBAT_ApplyDamageToGroup_Compat(&result, &group, 1, &outcome) == 1,
          "F0738 should accept the middle creature hit");

    /*
     * ReDMCSB GROUP.C F0190 lines 892-905 shifts Health, cells, and active
     * directions down after a killed middle creature, masks cells with
     * 0x003F, then decrements Count. DungeonGroup_Compat only owns health
     * and packed cells; ACTIVE_GROUP directions remain outside this helper.
     * GROUP.C F0199 lines 1238-1281 is the separate unblocked-distance path
     * used by visibility/smell logic, so this split gate also proves no
     * behavior or base-direction AI state is widened by F0738.
     */
    CHECK(outcome == COMBAT_OUTCOME_KILLED_SOME_CREATURES,
          "lethal hit should report KILLED_SOME_CREATURES");
    CHECK(group.count == 2, "four-creature group should become three creatures");
    CHECK(group.health[0] == 44, "slot 0 health should remain unchanged");
    CHECK(group.health[1] == 36, "slot 2 health should compact into slot 1");
    CHECK(group.health[2] == 28, "slot 3 health should compact into slot 2");
    CHECK(group.cells == (unsigned char)((0 << 0) | (2 << 2) | (3 << 4)),
          "packed cells should compact to live cells 0,2,3");
    CHECK(group.behavior == 6, "split compaction should not change group behavior");
    CHECK(group.direction == 2, "split compaction should not change base group direction");

    if (fail_count == 0) {
        printf("PASS: DM1 V1 creature group split gate\n");
    }
    return fail_count == 0 ? 0 : 1;
}

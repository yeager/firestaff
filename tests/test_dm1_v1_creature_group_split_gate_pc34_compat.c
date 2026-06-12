/*
 * DM1 V1 creature group split gate.
 *
 * Regression for the source-locked F0190 group transition: a lethal hit on
 * one creature in a multi-creature group compacts the remaining group state.
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

int main(void) {
    struct DungeonGroup_Compat group;
    struct CombatResult_Compat result;
    struct DM1GroupBehaviorContext_Compat behaviorCtx;
    int visibleDistance = -1;
    int smelledDirOrdinal = -1;
    int outcome = -1;

    memset(&group, 0, sizeof(group));
    memset(&result, 0, sizeof(result));
    memset(&behaviorCtx, 0, sizeof(behaviorCtx));

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

    /*
     * ReDMCSB GROUP.C F0199 lines 1238-1281 returns distance 1 immediately
     * for adjacent source/destination squares. F0200/F0201 then consume that
     * visibility/smell distance; after an attacked group loses one member,
     * this must remain a deterministic group-state input rather than a widened
     * creature-AI transition.
     */
    behaviorCtx.currentGroupDistanceToParty = 1;
    behaviorCtx.currentGroupPrimaryDirToParty = 1;
    behaviorCtx.distanceToVisibleParty = 1;
    behaviorCtx.creatureInfo.ranges = 0x1403; /* sight=3, smell=4, attack=1 */
    behaviorCtx.groupBehavior = group.behavior;
    behaviorCtx.creatureCount = group.count;
    CHECK(F0818_DM1_GROUP_GetDistanceToVisibleParty_Compat(
              &behaviorCtx, -1, &visibleDistance) == 1,
          "F0818 should accept the compacted group visibility context");
    CHECK(F0819_DM1_GROUP_GetSmelledPartyDirOrdinal_Compat(
              &behaviorCtx, &smelledDirOrdinal) == 1,
          "F0819 should accept the compacted group smell context");
    CHECK(visibleDistance == 1,
          "adjacent attacked group should keep F0199/F0200 visible distance 1");
    CHECK(smelledDirOrdinal == 2,
          "adjacent attacked group should smell toward primary direction ordinal");
    CHECK(behaviorCtx.groupBehavior == group.behavior,
          "F0199/F0200/F0201 distance checks should not mutate behavior");
    CHECK(behaviorCtx.creatureCount == group.count,
          "F0199/F0200/F0201 distance checks should not mutate split count");

    if (fail_count == 0) {
        printf("PASS: DM1 V1 creature group split gate\n");
    }
    return fail_count == 0 ? 0 : 1;
}

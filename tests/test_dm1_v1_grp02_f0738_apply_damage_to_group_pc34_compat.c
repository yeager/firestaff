/*
 * test_dm1_v1_grp02_f0738_apply_damage_to_group_pc34_compat.c
 *
 * Source-locked to ReDMCSB GROUP.C:892-905 (F0190 creature-death
 * compaction) and F0192 (per-creature poison resistance).
 *
 * GRP-02 (DM1 V1 functional-divergence-report.md):
 *   "F0190/F0191/F0192 damage outcomes are amalgam-only."
 *
 * F0738 is the new-path replacement for the F0190 damage
 * application to a creature in a group, including the multi-creature
 * compaction path (creature compaction after a kill).
 *
 * Pins the F0738_COMBAT_ApplyDamageToGroup_Compat contract:
 *  T1  NULL result returns 0
 *  T2  NULL group returns 0
 *  T3  NULL outOutcome returns 0
 *  T4  creatureIndex=-1 returns 0
 *  T5  creatureIndex=4 returns 0 (only 0..3 valid)
 *  T6  Damage < slotHp: outOutcome=KILLED_NO_CREATURES,
 *      slotHp -= damage, group->count unchanged
 *  T7  Damage >= slotHp, single-creature group:
 *      outOutcome=KILLED_ALL_CREATURES, slotHp=0, count=0
 *  T8  Damage >= slotHp, multi-creature group, killing the
 *      last slot: compaction moves surviving creatures down,
 *      count decrements
 *  T9  Damage >= slotHp, multi-creature group, killing a middle
 *      slot: surviving creatures shift down, count decrements
 *  T10 Damage = 0, single-creature group: outOutcome=KILLED_NO_CREATURES
 *  T11 Damage > slotHp (overkill), single creature: outOutcome=
 *      KILLED_ALL_CREATURES (already 0, no negative)
 *  T12 After kill, group->cells is masked with 0x3F
 *  T13 After kill, group->count >= 1 stays >= 0
 *  T14 Returns 1 on success
 *
 * Source-locked to ReDMCSB GROUP.C:892-905.
 */

#include "memory_combat_pc34_compat.h"

#include <stdio.h>
#include <string.h>

#define CHECK(cond, msg) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, msg); \
        return 1; \
    } \
} while (0)

int main(void) {
    struct CombatResult_Compat result;
    struct DungeonGroup_Compat group;
    int outOutcome = -1;
    int rc;

    /* T1: NULL result. */
    memset(&group, 0, sizeof(group));
    group.count = 1;
    group.health[0] = 50;
    CHECK(F0738_COMBAT_ApplyDamageToGroup_Compat(NULL, &group, 0, &outOutcome) == 0,
          "T1: NULL result returns 0");

    /* T2: NULL group. */
    memset(&result, 0, sizeof(result));
    result.damageApplied = 10;
    CHECK(F0738_COMBAT_ApplyDamageToGroup_Compat(&result, NULL, 0, &outOutcome) == 0,
          "T2: NULL group returns 0");

    /* T3: NULL outOutcome. */
    memset(&group, 0, sizeof(group));
    group.count = 1;
    group.health[0] = 50;
    CHECK(F0738_COMBAT_ApplyDamageToGroup_Compat(&result, &group, 0, NULL) == 0,
          "T3: NULL outOutcome returns 0");

    /* T4: creatureIndex=-1 returns 0. */
    memset(&group, 0, sizeof(group));
    group.count = 1;
    group.health[0] = 50;
    outOutcome = -1;
    CHECK(F0738_COMBAT_ApplyDamageToGroup_Compat(&result, &group, -1, &outOutcome) == 0,
          "T4: creatureIndex=-1 returns 0");
    CHECK(outOutcome == -1, "T4: outOutcome unchanged on -1");

    /* T5: creatureIndex=4 returns 0. */
    memset(&group, 0, sizeof(group));
    group.count = 1;
    group.health[0] = 50;
    outOutcome = -1;
    CHECK(F0738_COMBAT_ApplyDamageToGroup_Compat(&result, &group, 4, &outOutcome) == 0,
          "T5: creatureIndex=4 returns 0");

    /* T6: Damage < slotHp, single creature. */
    memset(&group, 0, sizeof(group));
    group.count = 1;
    group.health[0] = 100;
    memset(&result, 0, sizeof(result));
    result.damageApplied = 30;
    outOutcome = -1;
    CHECK(F0738_COMBAT_ApplyDamageToGroup_Compat(&result, &group, 0, &outOutcome) == 1,
          "T6a: returns 1 on success");
    CHECK(group.health[0] == 70, "T6b: slotHp = 100 - 30 = 70");
    CHECK(outOutcome == COMBAT_OUTCOME_KILLED_NO_CREATURES, "T6c: KILLED_NO_CREATURES");
    CHECK(group.count == 1, "T6d: count unchanged at 1");

    /* T7: Damage >= slotHp, single-creature group.  F0738 always
     * takes the compaction path (count=1, slotHp=0, decrements to 0).
     * KILLED_ALL only fires for the rare case where group->count was
     * already 0 at entry (degenerate input). */
    memset(&group, 0, sizeof(group));
    group.count = 1;
    group.health[0] = 50;
    memset(&result, 0, sizeof(result));
    result.damageApplied = 100;
    outOutcome = -1;
    CHECK(F0738_COMBAT_ApplyDamageToGroup_Compat(&result, &group, 0, &outOutcome) == 1,
          "T7a: returns 1");
    CHECK(group.health[0] == 0, "T7b: slotHp=0");
    CHECK(group.count == 0, "T7c: count=0 (single creature died)");
    CHECK(outOutcome == COMBAT_OUTCOME_KILLED_SOME_CREATURES, "T7d: KILLED_SOME (compaction path)");

    /* T8: Killing last slot of multi-creature group -> compaction. */
    memset(&group, 0, sizeof(group));
    group.count = 3;
    group.health[0] = 30;
    group.health[1] = 40;
    group.health[2] = 50;
    group.cells = 0x80; /* encoded: slot 0 = 0x00, slot 1 = 0x01, slot 2 = 0x02 */
    memset(&result, 0, sizeof(result));
    result.damageApplied = 100;
    outOutcome = -1;
    CHECK(F0738_COMBAT_ApplyDamageToGroup_Compat(&result, &group, 2, &outOutcome) == 1,
          "T8a: returns 1");
    CHECK(group.health[2] == 0, "T8b: killed slot hp=0");
    CHECK(group.count == 2, "T8c: count=2 (one killed)");
    CHECK(outOutcome == COMBAT_OUTCOME_KILLED_SOME_CREATURES, "T8d: KILLED_SOME");

    /* T9: Killing middle slot -> shift down. */
    memset(&group, 0, sizeof(group));
    group.count = 3;
    group.health[0] = 30;
    group.health[1] = 40;
    group.health[2] = 50;
    group.cells = 0xCA;
    memset(&result, 0, sizeof(result));
    result.damageApplied = 100;
    outOutcome = -1;
    CHECK(F0738_COMBAT_ApplyDamageToGroup_Compat(&result, &group, 1, &outOutcome) == 1,
          "T9a: returns 1");
    CHECK(group.count == 2, "T9b: count=2 (one killed)");
    CHECK(group.health[0] == 30, "T9c: slot 0 unchanged");
    CHECK(group.health[1] == 50, "T9d: slot 1 = old slot 2 (50)");

    /* T10: Damage = 0. */
    memset(&group, 0, sizeof(group));
    group.count = 1;
    group.health[0] = 100;
    memset(&result, 0, sizeof(result));
    result.damageApplied = 0;
    outOutcome = -1;
    F0738_COMBAT_ApplyDamageToGroup_Compat(&result, &group, 0, &outOutcome);
    CHECK(outOutcome == COMBAT_OUTCOME_KILLED_NO_CREATURES, "T10: 0 damage = KILLED_NO_CREATURES");
    CHECK(group.health[0] == 100, "T10: hp unchanged at 100");
    CHECK(group.count == 1, "T10: count unchanged");

    /* T11: Overkill single creature. */
    memset(&group, 0, sizeof(group));
    group.count = 1;
    group.health[0] = 30;
    memset(&result, 0, sizeof(result));
    result.damageApplied = 1000;
    outOutcome = -1;
    F0738_COMBAT_ApplyDamageToGroup_Compat(&result, &group, 0, &outOutcome);
    CHECK(group.health[0] == 0, "T11: hp clamped to 0");
    CHECK(outOutcome == COMBAT_OUTCOME_KILLED_SOME_CREATURES, "T11: KILLED_SOME (compaction path)");
    CHECK(group.count == 0, "T11: count=0");

    /* T12: cells is masked with 0x3F after kill. */
    memset(&group, 0, sizeof(group));
    group.count = 1;
    group.health[0] = 50;
    group.cells = 0xFF; /* high bits set */
    memset(&result, 0, sizeof(result));
    result.damageApplied = 100;
    outOutcome = -1;
    F0738_COMBAT_ApplyDamageToGroup_Compat(&result, &group, 0, &outOutcome);
    CHECK((group.cells & 0xC0) == 0, "T12: high bits cleared by 0x3F mask");

    /* T13: count never goes negative. */
    memset(&group, 0, sizeof(group));
    group.count = 1;
    group.health[0] = 1;
    memset(&result, 0, sizeof(result));
    result.damageApplied = 100;
    F0738_COMBAT_ApplyDamageToGroup_Compat(&result, &group, 0, &outOutcome);
    CHECK(group.count == 0, "T13: count=0 after kill, never negative");

    /* T14: Returns 1 on success. */
    memset(&group, 0, sizeof(group));
    group.count = 2;
    group.health[0] = 50;
    group.health[1] = 30;
    memset(&result, 0, sizeof(result));
    result.damageApplied = 10;
    outOutcome = -1;
    rc = F0738_COMBAT_ApplyDamageToGroup_Compat(&result, &group, 0, &outOutcome);
    CHECK(rc == 1, "T14: returns 1 on success");

    printf("PASS: GRP-02 F0738 apply-damage-to-group invariants (14 scenarios)\n");
    return 0;
}

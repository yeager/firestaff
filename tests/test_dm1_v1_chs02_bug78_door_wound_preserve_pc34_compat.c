/*
 * test_dm1_v1_chs02_bug78_door_wound_preserve_pc34_compat.c
 *
 * Source-locked to ReDMCSB TIMELINE.C:756-764 (BUG0_78):
 *   The condition
 *     MASK0x0008_WOUND_TORSO | AL0602_ui_VerticalDoor ?
 *     MASK0x0004_WOUND_HEAD : MASK0x0001_WOUND_READY_HAND |
 *     MASK0x0002_WOUND_ACTION_HAND
 *   has a missing parenthesis, so MASK0x0008_WOUND_TORSO |
 *   AL0602_ui_VerticalDoor is always non-zero, causing all
 *   doors to wound HEAD+TORSO instead of vertical/horizontal-
 *   conditional wounds.
 *
 * CHS-02 (DM1 V1 functional-divergence-report.md):
 *   "BUG0_78 (door-wound missing-parens) is intentionally
 *    preserved in new path."  Severity: Cosmetic.
 *
 * The intent is that BUG0_78 is preserved: doors wound HEAD
 * (and TORSO via the `|` chain) for all orientations.  The
 * Firestaff compat layer hardcodes woundMask =
 * DOOR_OBSTRUCTION_WOUND_HEAD in F0717_DOOR_ResolveClosingObstruction_Compat,
 * matching the broken-but-source-locked behavior.
 *
 * Pins:
 *  T1  F0717 with partyOnDoorSquare + doorState=1 (closing)
 *      returns woundMask == DOOR_OBSTRUCTION_WOUND_HEAD
 *  T2  doorVertical=1 (vertical door) does NOT change woundMask
 *      (BUG0_78: missing parens means vertical is irrelevant)
 *  T3  doorVertical=0 (horizontal door) does NOT change woundMask
 *      (BUG0_78: same broken precedence)
 *  T4  doorState=0 (open) -> no obstruction, no wound
 *  T5  doorState=5 (destroyed) -> no obstruction, no wound
 *  T6  partyChampionCount=0 -> no obstruction (no one to wound)
 *  T7  partyOnDoorSquare=0 -> no obstruction (party not in door)
 *  T8  Closing door schedule delay is 2 ticks (ReDMCSB F0241:772)
 *  T9  Damage amount is 5 (ReDMCSB standard door damage)
 *
 * Source-locked to ReDMCSB TIMELINE.C:756-764.
 */

#include "memory_door_action_pc34_compat.h"

#include <stdio.h>
#include <string.h>

#define CHECK(cond, msg) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, msg); \
        return 1; \
    } \
} while (0)

int main(void) {
    struct DoorClosingObstruction_Compat r;

    /* T1: Closing door + party on door -> wound HEAD. */
    memset(&r, 0, sizeof(r));
    F0717_DOOR_ResolveClosingObstruction_Compat(
        1, /* doorState=1 closing */
        0, /* doorVertical=0 (horizontal) */
        1, /* partyOnDoorSquare */
        4, /* partyChampionCount=4 */
        0, /* materialCreatureOnDoorSquare=0 */
        0, /* creatureHeight=0 */
        &r);
    CHECK(r.kind == DOOR_OBSTRUCTION_PARTY,
          "T1: party obstruction detected");
    CHECK(r.woundMask == DOOR_OBSTRUCTION_WOUND_HEAD,
          "T1: wound is HEAD (BUG0_78 preserve)");

    /* T2: doorVertical=1 does NOT change woundMask. */
    memset(&r, 0, sizeof(r));
    F0717_DOOR_ResolveClosingObstruction_Compat(1, 1, 1, 4, 0, 0, &r);
    CHECK(r.woundMask == DOOR_OBSTRUCTION_WOUND_HEAD,
          "T2: vertical door still wounds HEAD (BUG0_78: missing parens)");

    /* T3: doorVertical=0 does NOT change woundMask. */
    memset(&r, 0, sizeof(r));
    F0717_DOOR_ResolveClosingObstruction_Compat(1, 0, 1, 4, 0, 0, &r);
    CHECK(r.woundMask == DOOR_OBSTRUCTION_WOUND_HEAD,
          "T3: horizontal door wounds HEAD (BUG0_78)");

    /* T4: doorState=0 (open) -> no obstruction. */
    memset(&r, 0, sizeof(r));
    F0717_DOOR_ResolveClosingObstruction_Compat(0, 0, 1, 4, 0, 0, &r);
    CHECK(r.kind == DOOR_OBSTRUCTION_NONE,
          "T4: open door -> no obstruction");

    /* T5: doorState=5 (destroyed) -> no obstruction. */
    memset(&r, 0, sizeof(r));
    F0717_DOOR_ResolveClosingObstruction_Compat(5, 0, 1, 4, 0, 0, &r);
    CHECK(r.kind == DOOR_OBSTRUCTION_NONE,
          "T5: destroyed door -> no obstruction");

    /* T6: partyChampionCount=0 -> no obstruction (no one to wound). */
    memset(&r, 0, sizeof(r));
    F0717_DOOR_ResolveClosingObstruction_Compat(1, 0, 1, 0, 0, 0, &r);
    CHECK(r.kind == DOOR_OBSTRUCTION_NONE,
          "T6: empty party -> no obstruction");

    /* T7: party not on door -> no obstruction. */
    memset(&r, 0, sizeof(r));
    F0717_DOOR_ResolveClosingObstruction_Compat(1, 0, 0, 4, 0, 0, &r);
    CHECK(r.kind == DOOR_OBSTRUCTION_NONE,
          "T7: party not on door -> no obstruction");

    /* T8: Closing door reschedule delay is 2 ticks. */
    memset(&r, 0, sizeof(r));
    F0717_DOOR_ResolveClosingObstruction_Compat(1, 0, 1, 4, 0, 0, &r);
    CHECK(r.rescheduleDelayTicks == 2,
          "T8: closing door reschedule delay == 2 ticks (ReDMCSB F0241:772)");

    /* T9: Damage amount is 5. */
    memset(&r, 0, sizeof(r));
    F0717_DOOR_ResolveClosingObstruction_Compat(1, 0, 1, 4, 0, 0, &r);
    CHECK(r.damageAmount == 5,
          "T9: door damage == 5 (ReDMCSB standard door damage)");

    printf("PASS: CHS-02 BUG0_78 door-wound preserve (9 scenarios)\n");
    return 0;
}

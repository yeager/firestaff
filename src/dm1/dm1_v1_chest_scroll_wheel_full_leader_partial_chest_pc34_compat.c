#include "dm1_v1_chest_scroll_wheel_full_leader_partial_chest_pc34_compat.h"

static const DM1_V1_ChestScrollWheelCompositeCasePc34
    dm1_v1_chest_scroll_wheel_composite_table[] = {
        { 1, DM1_V1_CHEST_SCROLL_WHEEL_COMPOSITE_PARTIAL_SLOTS, 0,
          DM1_V1_CHEST_SCROLL_WHEEL_DIRECTION_UP, 0,
          DM1_V1_CHEST_SCROLL_WHEEL_SUPPRESS_LEADER_FULL },
        { 1, DM1_V1_CHEST_SCROLL_WHEEL_COMPOSITE_PARTIAL_SLOTS, 0,
          DM1_V1_CHEST_SCROLL_WHEEL_DIRECTION_UP, 1,
          DM1_V1_CHEST_SCROLL_WHEEL_SUPPRESS_LEADER_FULL },
        { 1, DM1_V1_CHEST_SCROLL_WHEEL_COMPOSITE_PARTIAL_SLOTS, 0,
          DM1_V1_CHEST_SCROLL_WHEEL_DIRECTION_DOWN, 0,
          DM1_V1_CHEST_SCROLL_WHEEL_SUPPRESS_LEADER_FULL },
        { 1, DM1_V1_CHEST_SCROLL_WHEEL_COMPOSITE_PARTIAL_SLOTS, 0,
          DM1_V1_CHEST_SCROLL_WHEEL_DIRECTION_DOWN, 1,
          DM1_V1_CHEST_SCROLL_WHEEL_SUPPRESS_LEADER_FULL },

        { 1, DM1_V1_CHEST_SCROLL_WHEEL_COMPOSITE_PARTIAL_SLOTS, 1,
          DM1_V1_CHEST_SCROLL_WHEEL_DIRECTION_UP, 0,
          DM1_V1_CHEST_SCROLL_WHEEL_SUPPRESS_LEADER_FULL },
        { 1, DM1_V1_CHEST_SCROLL_WHEEL_COMPOSITE_PARTIAL_SLOTS, 1,
          DM1_V1_CHEST_SCROLL_WHEEL_DIRECTION_UP, 1,
          DM1_V1_CHEST_SCROLL_WHEEL_SUPPRESS_LEADER_FULL },
        { 1, DM1_V1_CHEST_SCROLL_WHEEL_COMPOSITE_PARTIAL_SLOTS, 1,
          DM1_V1_CHEST_SCROLL_WHEEL_DIRECTION_DOWN, 0,
          DM1_V1_CHEST_SCROLL_WHEEL_SUPPRESS_LEADER_FULL },
        { 1, DM1_V1_CHEST_SCROLL_WHEEL_COMPOSITE_PARTIAL_SLOTS, 1,
          DM1_V1_CHEST_SCROLL_WHEEL_DIRECTION_DOWN, 1,
          DM1_V1_CHEST_SCROLL_WHEEL_SUPPRESS_LEADER_FULL },

        { 0, DM1_V1_CHEST_SCROLL_WHEEL_COMPOSITE_PARTIAL_SLOTS, 0,
          DM1_V1_CHEST_SCROLL_WHEEL_DIRECTION_UP, 0,
          DM1_V1_CHEST_SCROLL_WHEEL_SCROLL_TO_NEXT_SLOT },
        { 0, DM1_V1_CHEST_SCROLL_WHEEL_COMPOSITE_PARTIAL_SLOTS, 0,
          DM1_V1_CHEST_SCROLL_WHEEL_DIRECTION_UP, 1,
          DM1_V1_CHEST_SCROLL_WHEEL_SCROLL_TO_NEXT_SLOT },
        { 0, DM1_V1_CHEST_SCROLL_WHEEL_COMPOSITE_PARTIAL_SLOTS, 0,
          DM1_V1_CHEST_SCROLL_WHEEL_DIRECTION_DOWN, 0,
          DM1_V1_CHEST_SCROLL_WHEEL_SCROLL_TO_NEXT_SLOT },
        { 0, DM1_V1_CHEST_SCROLL_WHEEL_COMPOSITE_PARTIAL_SLOTS, 0,
          DM1_V1_CHEST_SCROLL_WHEEL_DIRECTION_DOWN, 1,
          DM1_V1_CHEST_SCROLL_WHEEL_SCROLL_TO_NEXT_SLOT },

        { 0, DM1_V1_CHEST_SCROLL_WHEEL_COMPOSITE_PARTIAL_SLOTS, 1,
          DM1_V1_CHEST_SCROLL_WHEEL_DIRECTION_UP, 0,
          DM1_V1_CHEST_SCROLL_WHEEL_DISPATCH_PICKUP },
        { 0, DM1_V1_CHEST_SCROLL_WHEEL_COMPOSITE_PARTIAL_SLOTS, 1,
          DM1_V1_CHEST_SCROLL_WHEEL_DIRECTION_UP, 1,
          DM1_V1_CHEST_SCROLL_WHEEL_DISPATCH_PICKUP },
        { 0, DM1_V1_CHEST_SCROLL_WHEEL_COMPOSITE_PARTIAL_SLOTS, 1,
          DM1_V1_CHEST_SCROLL_WHEEL_DIRECTION_DOWN, 0,
          DM1_V1_CHEST_SCROLL_WHEEL_DISPATCH_PICKUP },
        { 0, DM1_V1_CHEST_SCROLL_WHEEL_COMPOSITE_PARTIAL_SLOTS, 1,
          DM1_V1_CHEST_SCROLL_WHEEL_DIRECTION_DOWN, 1,
          DM1_V1_CHEST_SCROLL_WHEEL_DISPATCH_PICKUP }
    };

static const char s_source_evidence[] =
    "CHEST.C F0333:30-67 keeps same-open no-op stable, fills partial C537/C538 "
    "slots into G0425, and routes cross-champion open state without reshuffling; "
    "CHEST.C F0334:117-132 rewrites only non-empty slots on close. "
    "CHAMPION.C F0297:243-268 and F0298:270-298 own leader-hand state; "
    "CHAMPION.C F0302:662-710 suppresses empty/empty, snapshots the leader "
    "hand, and dispatches C30+ chest slots. PANEL.C F0344:1895-1944 + "
    "F0345:1946-1999 are the requested panel-click/highlight-rotation anchors; "
    "COMMAND.C F0359:1985-1990 is the M568/C040 dispatch anchor; MOUSE.C "
    "F0077:97-126 + F0078:128-168 are the requested wheel-queue anchors; "
    "OBJECT.C F0033:147-212 preserves icon identity; BLITMASK.C F0133:30-33 "
    "anchors partial-mask presentation; DEFS.H:810 and 3906-3913 define C30 "
    "and C537..C544.";

DM1_V1_ChestScrollWheelCompositeDecision
dm1_v1_chest_scroll_wheel_composite_decide(
    int leaderHandFull,
    int chestSlotCount,
    int chestSlotOccupied,
    DM1_V1_ChestScrollWheelCompositeDirection scrollDirection,
    int targetChampionIndex)
{
    (void)targetChampionIndex;

    if (leaderHandFull) {
        return DM1_V1_CHEST_SCROLL_WHEEL_SUPPRESS_LEADER_FULL;
    }
    if (chestSlotCount >= DM1_V1_CHEST_SCROLL_WHEEL_COMPOSITE_VISIBLE_SLOTS) {
        return DM1_V1_CHEST_SCROLL_WHEEL_FOCUS_CHEST_SLOT;
    }
    if (chestSlotOccupied) {
        return DM1_V1_CHEST_SCROLL_WHEEL_DISPATCH_PICKUP;
    }
    if (scrollDirection == DM1_V1_CHEST_SCROLL_WHEEL_DIRECTION_UP ||
        scrollDirection == DM1_V1_CHEST_SCROLL_WHEEL_DIRECTION_DOWN) {
        return DM1_V1_CHEST_SCROLL_WHEEL_SCROLL_TO_NEXT_SLOT;
    }
    return DM1_V1_CHEST_SCROLL_WHEEL_FOCUS_CHEST_SLOT;
}

const DM1_V1_ChestScrollWheelCompositeCasePc34*
dm1_v1_chest_scroll_wheel_composite_cases(size_t* count)
{
    if (count) {
        *count = sizeof(dm1_v1_chest_scroll_wheel_composite_table) /
                 sizeof(dm1_v1_chest_scroll_wheel_composite_table[0]);
    }
    return dm1_v1_chest_scroll_wheel_composite_table;
}

const char*
dm1_v1_chest_scroll_wheel_composite_source_evidence_pc34(void)
{
    return s_source_evidence;
}

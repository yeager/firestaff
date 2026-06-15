#ifndef FIRESTAFF_DM1_V1_CHEST_SCROLL_WHEEL_PICKUP_NON_LEADER_RUNTIME_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_SCROLL_WHEEL_PICKUP_NON_LEADER_RUNTIME_PC34_COMPAT_H

/*
 * DM1 V1 chest scroll-wheel pickup into a non-leader champion runtime gate.
 *
 * ReDMCSB anchors:
 * - CHEST.C F0333:30-67: open-chest early return, dispatch, and G0425
 *   materialization into C537..C544.
 * - CHEST.C F0334:113-132: close clears G0426, skips empty cells, and
 *   rewrites the visible chest chain.
 * - CHAMPION.C F0297:243-298 and F0298:270-298: leader-hand put/remove is
 *   intentionally skipped because the leader hand stack is already full.
 * - CHAMPION.C F0300:511-515: C30+ slot clear removes the picked C538 thing.
 * - CHAMPION.C F0301:606-614: C30+ slot write is the non-leader landing slot.
 * - CHAMPION.C F0302:662-714: occupied/empty slot dispatch and guards.
 * - CHAMPION.C F0284:93-131: party rotation moves the leader pointer.
 * - PANEL.C F0344:1895-1944 and F0345:1946-1999: panel click and per-cell
 *   highlight rotation.
 * - PANEL.C F0352: pressing-eye route must stay inactive for this pickup.
 * - COMMAND.C F0378:1973-1983 and F0359:1985-1990: M568/C040 dispatch
 *   lineage.
 * - MOUSE.C F0077:97-126 and F0078:128-168: scroll-wheel queue write/read.
 * - OBJECT.C F0033:147-212: icon identity for the picked object.
 * - BLITMASK.C F0133:30-33: partial-mask dispatch for the rotated highlight.
 * - DEFS.H:2088, 810-816, and 3906-3913: C30, M516/M070/G0425/G0426/G0423/
 *   G0305, C30..C36, and C537..C544 constants.
 */

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_PC34_NL_CHEST_SLOT_COUNT = 8,
    DM1_PC34_NL_LEADER_STACK_COUNT = 4,
    DM1_PC34_NL_CHAMPION_COUNT = 4,
    DM1_PC34_NL_NONE = 0xFFFF,
    DM1_PC34_NL_END_OF_LIST = 0xFFFE,
    DM1_PC34_NL_C30 = 30,
    DM1_PC34_NL_C31 = 31,
    DM1_PC34_NL_C36 = 36,
    DM1_PC34_NL_C537 = 537,
    DM1_PC34_NL_C538 = 538,
    DM1_PC34_NL_C539 = 539,
    DM1_PC34_NL_C540 = 540,
    DM1_PC34_NL_C541 = 541,
    DM1_PC34_NL_C542 = 542,
    DM1_PC34_NL_C543 = 543,
    DM1_PC34_NL_C544 = 544,
    DM1_PC34_NL_PANEL_M568 = 568,
    DM1_PC34_NL_COMMAND_C040 = 40,
    DM1_PC34_NL_SLOT_BOX_C39 = 39,
    DM1_PC34_NL_OPEN_CHEST = 0x7A00,
    DM1_PC34_NL_CHEST_ITEM0 = 0x7A10,
    DM1_PC34_NL_CHEST_ITEM1_PICKED = 0x7A11,
    DM1_PC34_NL_CHEST_ITEM2 = 0x7A12,
    DM1_PC34_NL_CHEST_ITEM3 = 0x7A13,
    DM1_PC34_NL_CHEST_ITEM4 = 0x7A14,
    DM1_PC34_NL_CHEST_ITEM5 = 0x7A15,
    DM1_PC34_NL_CHEST_ITEM6 = 0x7A16,
    DM1_PC34_NL_CHEST_ITEM7 = 0x7A17,
    DM1_PC34_NL_LEADER_ITEM0 = 0x7A40,
    DM1_PC34_NL_LEADER_ITEM1 = 0x7A41,
    DM1_PC34_NL_LEADER_ITEM2 = 0x7A42,
    DM1_PC34_NL_LEADER_ITEM3 = 0x7A43,
    DM1_PC34_NL_NEGATIVE_TARGET_ITEM = 0x7A60,
    DM1_PC34_NL_LOAD_MASK = 0x0200,
    DM1_PC34_NL_PANEL_MASK = 0x0800
};

typedef struct {
    int thing;
    int icon;
    int weight;
} DM1_V1_ChestScrollWheelPickupNonLeaderItemPc34;

typedef struct {
    const char* sourceEvidence;
    const char* f0333OpenAnchor;
    const char* f0334CloseAnchor;
    const char* f0297PutAnchor;
    const char* f0298RemoveAnchor;
    const char* f0300ClearAnchor;
    const char* f0301WriteAnchor;
    const char* f0302DispatchAnchor;
    const char* f0284RotationAnchor;
    const char* panelF0344Anchor;
    const char* panelF0345Anchor;
    const char* panelF0352Anchor;
    const char* commandF0378Anchor;
    const char* commandF0359Anchor;
    const char* mouseF0077Anchor;
    const char* mouseF0078Anchor;
    const char* objectAnchor;
    const char* blitMaskAnchor;
    const char* defsAnchor;
} DM1_V1_ChestScrollWheelPickupNonLeaderSpecPc34;

typedef struct {
    int openChestThing;
    int initialChest[DM1_PC34_NL_CHEST_SLOT_COUNT];
    int initialIcons[DM1_PC34_NL_CHEST_SLOT_COUNT];
    int initialLeaderStack[DM1_PC34_NL_LEADER_STACK_COUNT];
    int leaderStackAfterPickup[DM1_PC34_NL_LEADER_STACK_COUNT];
    int leaderStackAfterClose[DM1_PC34_NL_LEADER_STACK_COUNT];

    int partyChampionCountBefore;
    int partyChampionCountAfter;
    int inventoryChampionOrdinalBefore;
    int leaderIndexBeforeRotation;
    int leaderIndexAfterRotation;
    int targetChampionIndex;
    int targetSlotIndex;
    int targetSlotBefore;
    int targetSlotAfter;
    int targetSlotIconAfter;

    int wheelQueuedByF0077;
    int wheelReadByF0078;
    int wheelQueueDepthAfterRead;
    int wheelTargetZone;
    int panelF0344DispatchCount;
    int panelF0345HighlightCount;
    int highlightTrace[3];
    int partialMaskDispatches;
    int commandF0378Dispatch;
    int commandF0359Dispatch;
    int commandM568;
    int commandC040;
    int commandSlotBoxIndex;
    int championSlotIndexFromDispatch;
    int chestOffsetFromDispatch;
    int dispatchReadThing;
    int dispatchReadIcon;

    int leaderHandWasFull;
    int skippedOccupiedLeaderHand;
    int f0297PutCount;
    int f0298RemoveCount;
    int f0300ClearCount;
    int f0301WriteCount;
    int f0302DispatchCount;
    int pressingEyeRouteCount;
    int drawStateChampion;
    int targetLoadBefore;
    int targetLoadAfter;
    int targetAttributesAfter;

    int chestAfterPickup[DM1_PC34_NL_CHEST_SLOT_COUNT];
    int iconsAfterPickup[DM1_PC34_NL_CHEST_SLOT_COUNT];
    int closeCount;
    int closeSkippedNoneEntries;
    int closeHead;
    int closeEndSentinel;
    int closedSlots[DM1_PC34_NL_CHEST_SLOT_COUNT];
    int closedIcons[DM1_PC34_NL_CHEST_SLOT_COUNT];
    int pickedItemInClosedChain;
    int openChestAfterClose;
    int g0425ClearedAfterClose;

    int m570ChainLengthBefore;
    int m570ChainLengthAfter;
    int m516ThingCountBefore;
    int m516ThingCountAfter;
    int totalThingCountBefore;
    int totalThingCountAfter;
    int screenUpdateBalanced;

    int negativeWheelQueued;
    int negativeTargetBefore;
    int negativeTargetAfter;
    int negativeChestC538Before;
    int negativeChestC538After;
    int negativeMutated;
    int negativeF0301WriteCount;
} DM1_V1_ChestScrollWheelPickupNonLeaderProbePc34;

const DM1_V1_ChestScrollWheelPickupNonLeaderSpecPc34*
dm1_v1_chest_scroll_wheel_pickup_non_leader_runtime_spec_pc34(void);

const char*
dm1_v1_chest_scroll_wheel_pickup_non_leader_runtime_source_evidence_pc34(void);

int dm1_v1_chest_scroll_wheel_pickup_non_leader_runtime_pc34(
    DM1_V1_ChestScrollWheelPickupNonLeaderProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif

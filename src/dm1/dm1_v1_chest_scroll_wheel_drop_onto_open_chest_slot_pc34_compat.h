#ifndef FIRESTAFF_DM1_V1_CHEST_SCROLL_WHEEL_DROP_ONTO_OPEN_CHEST_SLOT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_SCROLL_WHEEL_DROP_ONTO_OPEN_CHEST_SLOT_PC34_COMPAT_H

/*
 * DM1 V1 chest scroll-wheel drop-onto-open-chest-slot regression gate.
 *
 * ReDMCSB anchors used by this asset-free probe:
 * - CHEST.C F0333:30-67: same-open guard and C537..C544 G0425 fill.
 * - CHEST.C F0334:113-132: close and relink non-empty chest cells.
 * - CHAMPION.C F0297:243-298: G4055 leader-hand put/load ownership.
 * - CHAMPION.C F0298:270-298: G4055 leader-hand remove/load ownership.
 * - CHAMPION.C F0300:511-515: C30+ G0425 slot clear, skipped here.
 * - CHAMPION.C F0301:606-614: C30+ G0425 empty-slot write.
 * - CHAMPION.C F0302:662-714: slot-box dispatch order and snapshots.
 * - PANEL.C F0352: chest panel redraw after slot mutation.
 * - COMMAND.C F0378:1973-1983: M569 chest panel dispatch to F0302.
 * - DEFS.H:810-817, 1876-1878, 2088, 3906-3913, 5862, 5878-5881:
 *   C30..C37, M070, C10, C537..C544, G4055, G0425, and G0426.
 */

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_PC34_C539_DROP_C10_COLOR_FLESH = 10,
    DM1_PC34_C539_DROP_C30 = 30,
    DM1_PC34_C539_DROP_C31 = 31,
    DM1_PC34_C539_DROP_C32 = 32,
    DM1_PC34_C539_DROP_C33 = 33,
    DM1_PC34_C539_DROP_C34 = 34,
    DM1_PC34_C539_DROP_C35 = 35,
    DM1_PC34_C539_DROP_C36 = 36,
    DM1_PC34_C539_DROP_C37 = 37,
    DM1_PC34_C539_DROP_C537 = 537,
    DM1_PC34_C539_DROP_C538 = 538,
    DM1_PC34_C539_DROP_C539 = 539,
    DM1_PC34_C539_DROP_C540 = 540,
    DM1_PC34_C539_DROP_C541 = 541,
    DM1_PC34_C539_DROP_C542 = 542,
    DM1_PC34_C539_DROP_C543 = 543,
    DM1_PC34_C539_DROP_C544 = 544,
    DM1_PC34_C539_DROP_C545_MOUTH = 545,
    DM1_PC34_C539_DROP_SLOT_COUNT = 8,
    DM1_PC34_C539_DROP_FOCUS_COUNT = 3,
    DM1_PC34_C539_DROP_NONE = 0xFFFF,
    DM1_PC34_C539_DROP_END = 0xFFFE,
    DM1_PC34_C539_DROP_OPEN_CHEST = 0x7B00,
    DM1_PC34_C539_DROP_CHEST_ITEM0 = 0x7B10,
    DM1_PC34_C539_DROP_CHEST_ITEM1 = 0x7B11,
    DM1_PC34_C539_DROP_CHEST_ITEM3 = 0x7B13,
    DM1_PC34_C539_DROP_CHEST_ITEM4 = 0x7B14,
    DM1_PC34_C539_DROP_HAND_ITEM = 0x7B80,
    DM1_PC34_C539_DROP_NEGATIVE_HAND = 0x7B81,
    DM1_PC34_C539_DROP_LOAD_MASK = 0x0200,
    DM1_PC34_C539_DROP_VIEWPORT_MASK = 0x4000,
    DM1_PC34_C539_DROP_PANEL_M569 = 569,
    DM1_PC34_C539_DROP_CLICK_C059 = 59
};

typedef struct {
    int thing;
    int icon;
    int weight;
    int allowedSlots;
} DM1_V1_ChestScrollWheelDropOntoOpenChestSlotItemPc34;

typedef struct {
    const char* sourceEvidence;
    const char* f0333OpenAnchor;
    const char* f0334CloseAnchor;
    const char* f0297PutAnchor;
    const char* f0298RemoveAnchor;
    const char* f0300ClearAnchor;
    const char* f0301WriteAnchor;
    const char* f0302DispatchAnchor;
    const char* panelF0352Anchor;
    const char* commandF0378Anchor;
    const char* mouseAnchor;
    const char* objectAnchor;
    const char* defsAnchor;
} DM1_V1_ChestScrollWheelDropOntoOpenChestSlotSpecPc34;

typedef struct {
    int openChestThing;
    int initialSlots[DM1_PC34_C539_DROP_SLOT_COUNT];
    int initialIcons[DM1_PC34_C539_DROP_SLOT_COUNT];
    int initialLeaderHand;
    int initialLeaderIcon;
    int initialLeaderLoad;
    int initialLeaderAttributes;
    int materializedVisibleCount;
    int sameOpenGuardKeptSlots;

    int focusTrace[4];
    int focusZoneTrace[4];
    int focusPreservedAfterDrop;
    int focusRotatesAfterDrop;
    int wheelTicks;
    int mouthFocusStartZone;
    int mouthFocusSurvivedDrop;

    int dropTargetZone;
    int dropTargetSlotIndex;
    int dropTargetSlotBox;
    int dropTargetCommand;
    int dropC30Offset;
    int commandF0378DispatchCount;
    int panelF0352RedrawCount;
    int f0302DispatchCount;
    int f0302LeaderSnapshot;
    int f0302SlotSnapshot;
    int f0302EmptyEmptyRejected;
    int f0302AllowedSlotGuardPassed;
    int f0298RemoveCount;
    int f0298RemovedThing;
    int f0298ClearedLeaderHand;
    int f0300ClearCount;
    int f0301WriteCount;
    int f0301WrittenThing;
    int f0301WrittenSlotValue;
    int f0297PutCount;

    int leaderHandAfterDrop;
    int leaderIconAfterDrop;
    int leaderLoadAfterDrop;
    int leaderAttributesAfterDrop;
    int chestAfterDrop[DM1_PC34_C539_DROP_SLOT_COUNT];
    int chestIconsAfterDrop[DM1_PC34_C539_DROP_SLOT_COUNT];
    int insertedSlotFilled;
    int insertedThingStoredOnce;
    int c30TailPreservedAfterDrop[3];
    int screenUpdateEnableCount;
    int screenUpdateDisableCount;
    int screenUpdateBalanced;

    int closeCount;
    int closeHead;
    int closeEndSentinel;
    int closedSlots[DM1_PC34_C539_DROP_SLOT_COUNT];
    int closedIcons[DM1_PC34_C539_DROP_SLOT_COUNT];
    int closedTailPreserved[3];
    int g0425ClearedAfterClose;
    int openChestAfterClose;

    int reopenCount;
    int reopenedOpenChest;
    int reopenedSlots[DM1_PC34_C539_DROP_SLOT_COUNT];
    int reopenedIcons[DM1_PC34_C539_DROP_SLOT_COUNT];
    int reopenedC539Thing;
    int reopenedChainContainsInserted;
    int reopenedChainOrderPreserved;

    int negativeEmptyHandBefore;
    int negativeSlotBefore;
    int negativeDropRejected;
    int negativeSlotAfter;
    int negativeLeaderAfter;
    int negativeWriteCount;
} DM1_V1_ChestScrollWheelDropOntoOpenChestSlotProbePc34;

const DM1_V1_ChestScrollWheelDropOntoOpenChestSlotSpecPc34*
dm1_v1_chest_scroll_wheel_drop_onto_open_chest_slot_spec_pc34(void);

int dm1_v1_chest_scroll_wheel_drop_onto_open_chest_slot_pc34(
    DM1_V1_ChestScrollWheelDropOntoOpenChestSlotProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif

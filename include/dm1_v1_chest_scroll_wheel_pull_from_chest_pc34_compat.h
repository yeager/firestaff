#ifndef FIRESTAFF_DM1_V1_CHEST_SCROLL_WHEEL_PULL_FROM_CHEST_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_SCROLL_WHEEL_PULL_FROM_CHEST_PC34_COMPAT_H

/*
 * DM1 V1 chest scroll-wheel pull-from-chest runtime regression gate.
 *
 * ReDMCSB anchors used by this asset-free probe:
 * - CHEST.C F0333:30-67: same-open guard and C537..C544 G0425 fill.
 * - CHEST.C F0334:117-132: close and relink non-empty chest cells.
 * - CHAMPION.C F0297:243-268: put visible chest thing in G4055 hand.
 * - CHAMPION.C F0298:270-298: leader-hand remove path stays unused.
 * - CHAMPION.C F0300:511-515: C30+ G0425 slot clear.
 * - CHAMPION.C F0301:606-614: C30+ G0425 slot write path is skipped.
 * - CHAMPION.C F0302:662-710: occupied chest-cell dispatch order.
 * - COMMAND.C F0378:1973-1983: chest panel dispatch to F0302.
 * - COMMAND.C F0359: scroll-wheel rotation input.
 * - PANEL.C F0344:1895-1944 and F0345:1946-1999: panel click/release.
 * - UTAMSCR.C F0077:147-151 and F0078:141-145: pointer wrappers.
 * - OBJECT.C F0033:147-212: icon identity.
 * - BLITMASK.C F0133:30-33: partial-mask presentation.
 * - DEFS.H:2088, 810-816, 3906-3913, G0425/G0426/G4055, M070, M516.
 */

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_PC34_PULL_C10_COLOR_FLESH = 10,
    DM1_PC34_PULL_C30 = 30,
    DM1_PC34_PULL_C31 = 31,
    DM1_PC34_PULL_C32 = 32,
    DM1_PC34_PULL_C33 = 33,
    DM1_PC34_PULL_C34 = 34,
    DM1_PC34_PULL_C35 = 35,
    DM1_PC34_PULL_C36 = 36,
    DM1_PC34_PULL_C37 = 37,
    DM1_PC34_PULL_C537 = 537,
    DM1_PC34_PULL_C538 = 538,
    DM1_PC34_PULL_C539 = 539,
    DM1_PC34_PULL_C540 = 540,
    DM1_PC34_PULL_C541 = 541,
    DM1_PC34_PULL_C542 = 542,
    DM1_PC34_PULL_C543 = 543,
    DM1_PC34_PULL_C544 = 544,
    DM1_PC34_PULL_SLOT_COUNT = 8,
    DM1_PC34_PULL_FOCUS_COUNT = 3,
    DM1_PC34_PULL_NONE = 0xFFFF,
    DM1_PC34_PULL_END = 0xFFFE,
    DM1_PC34_PULL_OPEN_CHEST = 0x7A00,
    DM1_PC34_PULL_CHEST_ITEM0 = 0x7A10,
    DM1_PC34_PULL_CHEST_ITEM1 = 0x7A11,
    DM1_PC34_PULL_CHEST_ITEM2 = 0x7A12,
    DM1_PC34_PULL_CHEST_ITEM3 = 0x7A13,
    DM1_PC34_PULL_LOAD_MASK = 0x0200,
    DM1_PC34_PULL_PANEL_M569 = 569,
    DM1_PC34_PULL_COMMAND_C040 = 40,
    DM1_PC34_PULL_CLICK_C059 = 59
};

typedef struct {
    int thing;
    int icon;
    int weight;
} DM1_V1_ChestScrollWheelPullItemPc34;

typedef struct {
    const char* sourceEvidence;
    const char* f0333OpenAnchor;
    const char* f0334CloseAnchor;
    const char* f0297PutAnchor;
    const char* f0298RemoveAnchor;
    const char* f0300ClearAnchor;
    const char* f0301WriteAnchor;
    const char* f0302DispatchAnchor;
    const char* commandF0378Anchor;
    const char* commandF0359Anchor;
    const char* panelF0344Anchor;
    const char* panelF0345Anchor;
    const char* mouseAnchor;
    const char* objectAnchor;
    const char* blitMaskAnchor;
    const char* defsAnchor;
} DM1_V1_ChestScrollWheelPullFromChestSpecPc34;

typedef struct {
    int openChestThing;
    int initialSlots[DM1_PC34_PULL_SLOT_COUNT];
    int initialIcons[DM1_PC34_PULL_SLOT_COUNT];
    int initialLeaderHand;
    int initialLeaderIcon;
    int initialLeaderLoad;
    int initialLeaderAttributes;
    int materializedVisibleCount;
    int sameOpenGuardKeptSlots;
    int focusTrace[4];
    int focusZoneTrace[4];
    int focusPreservedAfterPull;
    int focusRotatesAfterPull;
    int rotationWheelTicks;
    int rotationPartialMaskDispatches;
    int rotationLeaderHandStable;
    int pullWheelTick;
    int pullTargetZone;
    int pullTargetSlotIndex;
    int pullTargetCommand;
    int pullTargetSlotBox;
    int pullChampionSlotIndex;
    int pullC30Offset;
    int pullTargetNotFocusZone;
    int commandF0378DispatchCount;
    int commandF0359WheelCount;
    int panelClickCount;
    int panelReleaseCount;
    int f0302DispatchCount;
    int f0302LeaderSnapshot;
    int f0302SlotSnapshot;
    int f0302EmptyEmptyRejected;
    int f0302AllowedSlotGuardPassed;
    int f0300ClearCount;
    int f0300ClearedThing;
    int f0300ClearedSlotValue;
    int f0301WriteCount;
    int f0297PutCount;
    int f0298RemoveCount;
    int leaderHandAfterPull;
    int leaderIconAfterPull;
    int leaderLoadAfterPull;
    int leaderAttributesAfterPull;
    int chestAfterPull[DM1_PC34_PULL_SLOT_COUNT];
    int chestIconsAfterPull[DM1_PC34_PULL_SLOT_COUNT];
    int chainOrderPreservedAfterPull[3];
    int pulledSlotCleared;
    int pulledThingMissingFromChest;
    int screenUpdateEnableCount;
    int screenUpdateDisableCount;
    int screenUpdateBalanced;
    int closeCount;
    int closeHead;
    int closeEndSentinel;
    int closedSlots[DM1_PC34_PULL_SLOT_COUNT];
    int g0425ClearedAfterClose;
    int openChestAfterClose;
    int pulledThingNotClosedBackIntoChest;
} DM1_V1_ChestScrollWheelPullFromChestProbePc34;

const DM1_V1_ChestScrollWheelPullFromChestSpecPc34*
dm1_v1_chest_scroll_wheel_pull_from_chest_spec_pc34(void);

int dm1_v1_chest_scroll_wheel_pull_from_chest_pc34(
    DM1_V1_ChestScrollWheelPullFromChestProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif

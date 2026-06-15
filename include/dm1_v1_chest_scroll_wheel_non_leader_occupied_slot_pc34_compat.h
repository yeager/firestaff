#ifndef FIRESTAFF_DM1_V1_CHEST_SCROLL_WHEEL_NON_LEADER_OCCUPIED_SLOT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_SCROLL_WHEEL_NON_LEADER_OCCUPIED_SLOT_PC34_COMPAT_H

/*
 * DM1 V1 chest scroll-wheel non-leader occupied-slot runtime regression.
 *
 * This is a Firestaff runtime contract for the scroll-wheel extension, not an
 * original DOS parity claim.
 *
 * ReDMCSB anchors:
 * - CHEST.C F0333:30-67: open chest materializes visible C537..C544 slots.
 * - CHEST.C F0334:117-132: close rewrites non-empty visible C537..C544 cells.
 * - CHAMPION.C F0297:243-268 and F0298:270-298: hand put/remove ordering.
 * - CHAMPION.C F0300:511-515 and F0301:606-614: C30+ slot clear/write.
 * - CHAMPION.C F0302:662-710: occupied-slot click dispatch snapshot/order.
 * - PANEL.C F0344:1895-1944 and F0345:1946-1999: panel click/highlight.
 * - COMMAND.C F0359:1985-1990: M568/C040 chest-panel route.
 * - MOUSE.C F0077:97-126 and F0078:128-168: wheel queue write/read.
 * - OBJECT.C F0033:147-212: object icon identity.
 * - BLITMASK.C F0133:30-33: partial-mask highlight blit.
 * - DEFS.H:2088, 810-816, and 3906-3913: C30 and C537..C544 constants.
 */

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_PC34_NLO_SLOT_COUNT = 8,
    DM1_PC34_NLO_CHAMPION_COUNT = 4,
    DM1_PC34_NLO_NONE = 0xFFFF,
    DM1_PC34_NLO_END = 0xFFFE,
    DM1_PC34_NLO_C30 = 30,
    DM1_PC34_NLO_C31 = 31,
    DM1_PC34_NLO_C33 = 33,
    DM1_PC34_NLO_C537 = 537,
    DM1_PC34_NLO_C538 = 538,
    DM1_PC34_NLO_C540 = 540,
    DM1_PC34_NLO_C544 = 544,
    DM1_PC34_NLO_PANEL_M568 = 568,
    DM1_PC34_NLO_COMMAND_C040 = 40,
    DM1_PC34_NLO_OWNER_INDEX = 2,
    DM1_PC34_NLO_OWNER_ORDINAL = 3,
    DM1_PC34_NLO_LEADER_INDEX = 0,
    DM1_PC34_NLO_OPEN_CHEST = 0x6C00,
    DM1_PC34_NLO_CHEST_C537 = 0x6C10,
    DM1_PC34_NLO_HAND_INITIAL = 0x6C20,
    DM1_PC34_NLO_CHEST_C540_PICKED = 0x6C30,
    DM1_PC34_NLO_CHEST_C541 = 0x6C41,
    DM1_PC34_NLO_CHEST_C542 = 0x6C42,
    DM1_PC34_NLO_LOAD_MASK = 0x0200,
    DM1_PC34_NLO_PANEL_MASK = 0x0800,
    DM1_PC34_NLO_VIEWPORT_MASK = 0x4000
};

typedef struct {
    int thing;
    int icon;
    int weight;
} DM1_V1_ChestScrollWheelNonLeaderOccupiedItemPc34;

typedef struct {
    const char* sourceEvidence;
    const char* f0333OpenAnchor;
    const char* f0334CloseAnchor;
    const char* f0297PutAnchor;
    const char* f0298RemoveAnchor;
    const char* f0300ClearAnchor;
    const char* f0301WriteAnchor;
    const char* f0302DispatchAnchor;
    const char* panelF0344Anchor;
    const char* panelF0345Anchor;
    const char* commandF0359Anchor;
    const char* mouseF0077Anchor;
    const char* mouseF0078Anchor;
    const char* objectAnchor;
    const char* blitMaskAnchor;
    const char* defsAnchor;
} DM1_V1_ChestScrollWheelNonLeaderOccupiedSpecPc34;

typedef struct {
    int openChestBefore;
    int ownerChampionIndex;
    int ownerChampionOrdinal;
    int leaderIndex;
    int partyChampionCount;
    int initialHandThing;
    int initialHandIcon;
    int initialOwnerLoad;
    int initialChest[DM1_PC34_NLO_SLOT_COUNT];
    int initialIcons[DM1_PC34_NLO_SLOT_COUNT];

    int wheelUpQueuedByF0077;
    int wheelUpReadByF0078;
    int wheelUpQueueDepthAfterRead;
    int wheelUpTargetZone;
    int wheelUpTargetSlotIndex;
    int wheelUpCommandM568;
    int wheelUpCommandC040;
    int wheelUpPanelF0344Dispatch;
    int wheelUpPanelF0345Highlight;
    int wheelUpHighlightTrace[4];
    int wheelUpPartialMaskDispatches;
    int wheelUpF0302DispatchCount;
    int wheelUpHandSnapshot;
    int wheelUpSlotSnapshot;
    int wheelUpF0298RemoveHandCount;
    int wheelUpF0300ClearSlotCount;
    int wheelUpF0297PutHandCount;
    int wheelUpF0301WriteSlotCount;
    int handAfterWheelUp;
    int handIconAfterWheelUp;
    int c540AfterWheelUp;
    int c538AfterWheelUp;
    int ownerLoadAfterWheelUp;
    int ownerAttributesAfterWheelUp;
    int chestAfterWheelUp[DM1_PC34_NLO_SLOT_COUNT];
    int iconsAfterWheelUp[DM1_PC34_NLO_SLOT_COUNT];

    int wheelDownQueuedByF0077;
    int wheelDownReadByF0078;
    int wheelDownQueueDepthAfterRead;
    int wheelDownTargetZone;
    int wheelDownTargetSlotIndex;
    int wheelDownCommandM568;
    int wheelDownCommandC040;
    int wheelDownPanelF0344Dispatch;
    int wheelDownPanelF0345Highlight;
    int wheelDownHighlightTrace[3];
    int wheelDownPartialMaskDispatches;
    int wheelDownF0302DispatchCount;
    int wheelDownHandSnapshot;
    int wheelDownSlotSnapshot;
    int wheelDownF0298RemoveHandCount;
    int wheelDownF0300ClearSlotCount;
    int wheelDownF0297PutHandCount;
    int wheelDownF0301WriteSlotCount;
    int handAfterWheelDown;
    int c538AfterWheelDown;
    int c540AfterWheelDown;
    int ownerLoadAfterWheelDown;
    int ownerAttributesAfterWheelDown;
    int chestAfterWheelDown[DM1_PC34_NLO_SLOT_COUNT];
    int iconsAfterWheelDown[DM1_PC34_NLO_SLOT_COUNT];

    int closeCount;
    int closeSkippedEmptyEntries;
    int closeHead;
    int closeEndSentinel;
    int closedSlots[DM1_PC34_NLO_SLOT_COUNT];
    int closedIcons[DM1_PC34_NLO_SLOT_COUNT];
    int openChestAfterClose;
    int g0425ClearedAfterClose;
    int finalThingCount;
    int initialThingCount;
    int screenUpdateBalanced;

    int negativeWheelDownRejected;
    int negativeHandAfter;
    int negativeC538After;
} DM1_V1_ChestScrollWheelNonLeaderOccupiedProbePc34;

const DM1_V1_ChestScrollWheelNonLeaderOccupiedSpecPc34*
dm1_v1_chest_scroll_wheel_non_leader_occupied_slot_spec_pc34(void);

const char*
dm1_v1_chest_scroll_wheel_non_leader_occupied_slot_source_evidence_pc34(void);

int dm1_v1_chest_scroll_wheel_non_leader_occupied_slot_pc34(
    DM1_V1_ChestScrollWheelNonLeaderOccupiedProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif

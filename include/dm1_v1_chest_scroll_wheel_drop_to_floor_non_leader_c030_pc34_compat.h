#ifndef FIRESTAFF_DM1_V1_CHEST_SCROLL_WHEEL_DROP_TO_FLOOR_NON_LEADER_C030_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_SCROLL_WHEEL_DROP_TO_FLOOR_NON_LEADER_C030_PC34_COMPAT_H

/*
 * DM1 V1 chest scroll-wheel C545 drop-to-floor non-leader C030 gate.
 *
 * ReDMCSB anchors:
 * - CHEST.C F0333:30-67: open chest materializes visible C537..C544 in G0425.
 * - CHEST.C F0334:113-132: close rewrites the G0426 chest from G0425.
 * - CHAMPION.C F0297:243-298 and F0298:270-298: leader-hand put/remove
 *   must stay unused by this drop-to-floor path.
 * - CHAMPION.C F0300:511-515: C30+ slot removal clears the source cell.
 * - CHAMPION.C F0301:606-614: C30 landing-slot write is not the target here.
 * - CHAMPION.C F0302:662-714: slot command snapshots leader hand plus C30.
 * - CHAMPION.C F0284:93-131: champion ownership/non-leader read context.
 * - PANEL.C F0352: chest panel redraw after the visible slot becomes empty.
 * - COMMAND.C F0378:1973-1983: C545 panel event dispatch lineage.
 * - DEFS.H:2088, 810-816, 3906-3913: C30/G0425/G0426/G0423/G0305/M070/
 *   M516 and C537..C545 constants.
 */

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_PC34_C545_DROP_SLOT_COUNT = 8,
    DM1_PC34_C545_DROP_CHAMPION_COUNT = 4,
    DM1_PC34_C545_DROP_NONE = 0xFFFF,
    DM1_PC34_C545_DROP_END = 0xFFFE,
    DM1_PC34_C545_DROP_C30 = 30,
    DM1_PC34_C545_DROP_C31 = 31,
    DM1_PC34_C545_DROP_C36 = 36,
    DM1_PC34_C545_DROP_C37 = 37,
    DM1_PC34_C545_DROP_C537 = 537,
    DM1_PC34_C545_DROP_C538 = 538,
    DM1_PC34_C545_DROP_C539 = 539,
    DM1_PC34_C545_DROP_C540 = 540,
    DM1_PC34_C545_DROP_C541 = 541,
    DM1_PC34_C545_DROP_C542 = 542,
    DM1_PC34_C545_DROP_C543 = 543,
    DM1_PC34_C545_DROP_C544 = 544,
    DM1_PC34_C545_DROP_C545 = 545,
    DM1_PC34_C545_DROP_C070 = 70,
    DM1_PC34_C545_DROP_PANEL_M569 = 569,
    DM1_PC34_C545_DROP_LEADER_INDEX = 0,
    DM1_PC34_C545_DROP_OWNER_INDEX = 2,
    DM1_PC34_C545_DROP_OWNER_ORDINAL = 3,
    DM1_PC34_C545_DROP_OPEN_CHEST = 0x6B00,
    DM1_PC34_C545_DROP_ITEM = 0x6B21,
    DM1_PC34_C545_DROP_NEGATIVE_SENTINEL = 0x6B40,
    DM1_PC34_C545_DROP_FLOOR_X = 12,
    DM1_PC34_C545_DROP_FLOOR_Y = 18,
    DM1_PC34_C545_DROP_LOAD_MASK = 0x0200,
    DM1_PC34_C545_DROP_PANEL_MASK = 0x0800,
    DM1_PC34_C545_DROP_VIEWPORT_MASK = 0x4000
};

typedef struct {
    int thing;
    int icon;
    int weight;
} DM1_V1_ChestScrollWheelDropToFloorNonLeaderC030ItemPc34;

typedef struct {
    const char* sourceEvidence;
    const char* f0333OpenAnchor;
    const char* f0334CloseAnchor;
    const char* f0297PutAnchor;
    const char* f0298RemoveAnchor;
    const char* f0300ClearAnchor;
    const char* f0301WriteAnchor;
    const char* f0302DispatchAnchor;
    const char* f0284OwnershipAnchor;
    const char* panelF0352Anchor;
    const char* commandF0378Anchor;
    const char* defsAnchor;
} DM1_V1_ChestScrollWheelDropToFloorNonLeaderC030SpecPc34;

typedef struct {
    int partyChampionCount;
    int leaderIndex;
    int inventoryChampionOrdinal;
    int targetChampionIndex;
    int targetChampionOrdinal;
    int openChestThingBefore;
    int openChestOwnerBefore;
    int leaderHandBefore;
    int leaderC30Before;
    int nonLeaderC30Before;
    int nonLeaderC30IconBefore;
    int nonLeaderLoadBefore;
    int initialG0425[DM1_PC34_C545_DROP_SLOT_COUNT];
    int initialIcons[DM1_PC34_C545_DROP_SLOT_COUNT];
    int initialLeaderG0425[DM1_PC34_C545_DROP_SLOT_COUNT];

    int c545EventTriggered;
    int c545EventZone;
    int c545Command;
    int commandF0378DispatchCount;
    int f0302DispatchCount;
    int f0302LeaderSnapshot;
    int f0302SlotSnapshot;
    int f0302ChampionIndex;
    int f0302SlotIndex;
    int f0302C30Offset;
    int f0302EmptyEmptyRejected;
    int f0302LeaderHandBypass;
    int f0297PutCount;
    int f0298RemoveCount;
    int f0300ClearCount;
    int f0301WriteCount;

    int floorWriteCount;
    int floorThing;
    int floorIcon;
    int floorMapX;
    int floorMapY;
    int floorOwnerChampionIndex;
    int floorOwnerOrdinal;
    int floorOwnerG0426;
    int floorWroteSameChampion;
    int floorWroteNotLeader;

    int leaderHandAfter;
    int leaderC30After;
    int leaderC30Unchanged;
    int nonLeaderC30After;
    int nonLeaderLoadAfter;
    int nonLeaderAttributesAfter;
    int openChestThingAfter;
    int openChestOwnerAfter;
    int g0426StillOwnedByNonLeader;
    int g0425AfterDrop[DM1_PC34_C545_DROP_SLOT_COUNT];
    int iconsAfterDrop[DM1_PC34_C545_DROP_SLOT_COUNT];
    int leaderG0425AfterDrop[DM1_PC34_C545_DROP_SLOT_COUNT];
    int visibleSlotEmptyAfterDrop;
    int panelF0352RedrawCount;
    int panelRedrawSlots[DM1_PC34_C545_DROP_SLOT_COUNT];
    int screenUpdateBalanced;

    int negativeEventTriggered;
    int negativeFloorWriteCount;
    int negativeLeaderHandAfter;
    int negativeLeaderC30After;
    int negativeNonLeaderC30Before;
    int negativeNonLeaderC30After;
    int negativeOpenChestOwnerAfter;
    int negativeG0425After[DM1_PC34_C545_DROP_SLOT_COUNT];
    int negativePanelRedrawCount;
} DM1_V1_ChestScrollWheelDropToFloorNonLeaderC030ProbePc34;

const DM1_V1_ChestScrollWheelDropToFloorNonLeaderC030SpecPc34*
dm1_v1_chest_scroll_wheel_drop_to_floor_non_leader_c030_spec_pc34(void);

const char*
dm1_v1_chest_scroll_wheel_drop_to_floor_non_leader_c030_source_evidence_pc34(void);

int dm1_v1_chest_scroll_wheel_drop_to_floor_non_leader_c030_pc34(
    DM1_V1_ChestScrollWheelDropToFloorNonLeaderC030ProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif

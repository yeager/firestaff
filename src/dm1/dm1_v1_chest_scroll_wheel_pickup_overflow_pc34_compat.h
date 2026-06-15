#ifndef FIRESTAFF_DM1_V1_CHEST_SCROLL_WHEEL_PICKUP_OVERFLOW_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_SCROLL_WHEEL_PICKUP_OVERFLOW_PC34_COMPAT_H

/*
 * DM1 V1 chest scroll-wheel pickup-overflow runtime regression gate.
 *
 * ReDMCSB anchors:
 * - CHEST.C F0333:30-67: open-chest materialization into C537..C544.
 * - CHEST.C F0334:113-132: close rewrite skips C0xFFFF_THING_NONE cells
 *   and relinks only non-empty visible cells.
 * - CHAMPION.C F0297:243-268: leader-hand put and load/icon refresh.
 * - CHAMPION.C F0298:270-298: leader-hand remove and load/icon clear.
 * - CHAMPION.C F0302:662-710: occupied slot click dispatch, including the
 *   full-leader-hand replacement path for C30+ chest cells.
 * - PANEL.C F0344:1895-1944 + F0345:1946-1999: panel click/highlight
 *   lineage requested for scroll-wheel focus.
 * - COMMAND.C F0359:1985-1990: M568/C040 dispatch lineage.
 * - MOUSE.C F0077:97-126 + F0078:128-168: wheel queue lineage.
 * - OBJECT.C F0033:147-212: icon identity.
 * - BLITMASK.C F0133:30-33: partial-mask dispatch.
 * - DEFS.H:2088 and 3906-3913: requested C30/C537..C544 slot constants.
 */

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_PC34_PICKUP_OVERFLOW_SLOT_COUNT = 8,
    DM1_PC34_PICKUP_OVERFLOW_NONE = 0xFFFF,
    DM1_PC34_PICKUP_OVERFLOW_END_OF_LIST = 0xFFFE,
    DM1_PC34_PICKUP_OVERFLOW_C30 = 30,
    DM1_PC34_PICKUP_OVERFLOW_C37_CHEST_8 = 37,
    DM1_PC34_PICKUP_OVERFLOW_C537 = 537,
    DM1_PC34_PICKUP_OVERFLOW_C544 = 544,
    DM1_PC34_PICKUP_OVERFLOW_PANEL_M568 = 568,
    DM1_PC34_PICKUP_OVERFLOW_COMMAND_C040 = 40,
    DM1_PC34_PICKUP_OVERFLOW_CLICK_C065 = 65,
    DM1_PC34_PICKUP_OVERFLOW_SLOT_BOX_C45 = 45,
    DM1_PC34_PICKUP_OVERFLOW_OPEN_CHEST = 0x7700,
    DM1_PC34_PICKUP_OVERFLOW_LEADER_ITEM = 0x7720,
    DM1_PC34_PICKUP_OVERFLOW_CHEST_ITEM0 = 0x7730,
    DM1_PC34_PICKUP_OVERFLOW_CHEST_ITEM2 = 0x7732,
    DM1_PC34_PICKUP_OVERFLOW_CHEST_ITEM5 = 0x7735,
    DM1_PC34_PICKUP_OVERFLOW_CHEST_ITEM7 = 0x7737,
    DM1_PC34_PICKUP_OVERFLOW_BASE_LOAD = 80,
    DM1_PC34_PICKUP_OVERFLOW_LEADER_WEIGHT = 11,
    DM1_PC34_PICKUP_OVERFLOW_C544_WEIGHT = 7,
    DM1_PC34_PICKUP_OVERFLOW_LOAD_MASK = 0x0200,
    DM1_PC34_PICKUP_OVERFLOW_CHEST_SLOT_MASK = 0x0400
};

typedef enum {
    DM1_V1_PICKUP_OVERFLOW_REJECT_KEEP_LEADER = 1,
    DM1_V1_PICKUP_OVERFLOW_ROUTE_TO_FIRST_FREE = 2,
    DM1_V1_PICKUP_OVERFLOW_ROUTE_C544_REPLACEMENT = 3
} DM1_V1_ChestScrollWheelPickupOverflowRoutePc34;

typedef struct {
    int thing;
    int weight;
    int icon;
} DM1_V1_ChestScrollWheelPickupOverflowItemPc34;

typedef struct {
    int contractOnly;
    DM1_V1_ChestScrollWheelPickupOverflowRoutePc34 sourceLockedRoute;
    const char* routeLabel;
    const char* f0333OpenAnchor;
    const char* f0334CloseAnchor;
    const char* f0297PutAnchor;
    const char* f0298RemoveAnchor;
    const char* f0302DispatchAnchor;
    const char* panelAnchor;
    const char* commandAnchor;
    const char* mouseAnchor;
    const char* objectAnchor;
    const char* blitMaskAnchor;
    const char* defsAnchor;
} DM1_V1_ChestScrollWheelPickupOverflowSpecPc34;

typedef struct {
    int openChestThing;
    int initialSlots[DM1_PC34_PICKUP_OVERFLOW_SLOT_COUNT];
    int initialIcons[DM1_PC34_PICKUP_OVERFLOW_SLOT_COUNT];
    int initialLeaderThing;
    int initialLeaderIcon;
    int initialLeaderLoad;
    int initialLeaderAttributes;
    int initialChestItemCount;
    int firstFreeSlotBefore;
    int c544InitiallyOccupied;

    int wheelQueuedByF0078;
    int wheelQueueDepth;
    int wheelTargetZone;
    int panelDispatch;
    int commandDispatch;
    int commandClick;
    int commandSlotBoxIndex;
    int championSlotIndex;
    int chestSlotOffset;
    int dispatchReadC544Thing;
    int dispatchReadLeaderThing;
    int allowedSlotMaskMatched;

    DM1_V1_ChestScrollWheelPickupOverflowRoutePc34 routeTaken;
    int rejectedKeepLeaderPath;
    int routedToFirstFreePath;
    int routedToC544ReplacementPath;
    int firstFreeSlotUsed;
    int leaderRemovedCount;
    int leaderPutCount;
    int slotRemoveCount;
    int slotAddCount;
    int leaderThingAfter;
    int leaderIconAfter;
    int leaderLoadAfter;
    int leaderAttributesAfter;
    int c544ThingAfter;
    int c544IconAfter;
    int firstFreeSlotAfter;
    int leaderStackCountAfter;
    int totalThingsBefore;
    int totalThingsAfter;
    int screenUpdateBalanced;

    int closeCount;
    int closeSkippedNoneEntries;
    int closeHead;
    int closeEndSentinel;
    int closedSlots[DM1_PC34_PICKUP_OVERFLOW_SLOT_COUNT];
    int c544ReplacementRewrittenOnClose;
    int originalC544NowInLeader;
    int g0425ClearedAfterClose;
} DM1_V1_ChestScrollWheelPickupOverflowProbePc34;

const char*
dm1_v1_chest_scroll_wheel_pickup_overflow_source_evidence_pc34(void);
const DM1_V1_ChestScrollWheelPickupOverflowSpecPc34*
dm1_v1_chest_scroll_wheel_pickup_overflow_spec_pc34(void);
int dm1_v1_chest_scroll_wheel_pickup_overflow_pc34(
    DM1_V1_ChestScrollWheelPickupOverflowProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif

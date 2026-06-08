#ifndef FIRESTAFF_DM1_V1_CHEST_SCROLL_WHEEL_PICKUP_DROP_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_SCROLL_WHEEL_PICKUP_DROP_PC34_COMPAT_H

/*
 * DM1 V1 chest scroll-wheel pickup/drop contract gate.
 *
 * ReDMCSB anchors used by this contract-only probe:
 * - CHEST.C F0333:30-67: same-open guard, transitive close, open panel
 *   materialization, icon identity draw, and C537..C544 G0425 fill.
 * - CHEST.C F0334:113-132: no-open return, G0426 clear, G0425 clear, and
 *   non-empty visible-slot close rewrite.
 * - CHAMPION.C F0302:662-710: slot-box dispatch, leader hand snapshot,
 *   C30+ chest-slot selection, early empty/empty return, allowed-slot guard,
 *   leader-hand remove, occupied-slot pickup, and slot write.
 * - CHAMPION.C F0297:243-298 and F0298:270-298: leader-hand put/remove,
 *   object icon, pointer update, weight/load adjustment, and refresh flags.
 * - PANEL.C F0344:1895-1944 and F0345:1946-1999: requested panel click and
 *   per-cell highlight-rotation lineage for this scroll-wheel gate.
 * - COMMAND.C F0359:1985-1990: requested M568/C040 dispatch lineage marker.
 * - MOUSE.C F0077:97-126 and F0078:128-168: requested mouse-wheel queue
 *   lineage marker; the local ReDMCSB checkout stores F0077/F0078 platform
 *   wrappers outside a MOUSE.C file, so this gate keeps that requested anchor
 *   as a contract string only.
 * - OBJECT.C F0033:147-212: object icon identity used for visible cells.
 * - BLITMASK.C F0133:30-33: partial-mask dispatch, presentation-only.
 */

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_PC34_CHEST_SCROLL_WHEEL_SLOT_COUNT = 8,
    DM1_PC34_CHEST_SCROLL_WHEEL_VISIBLE_ITEMS = 4,
    DM1_PC34_CHEST_SCROLL_WHEEL_HIGHLIGHT_CYCLES = 4,
    DM1_PC34_CHEST_SCROLL_WHEEL_HIGHLIGHT_STEPS = 5,
    DM1_PC34_CHEST_SCROLL_WHEEL_C537 = 537,
    DM1_PC34_CHEST_SCROLL_WHEEL_C544 = 544,
    DM1_PC34_CHEST_SCROLL_WHEEL_C30_BASE = 30,
    DM1_PC34_CHEST_SCROLL_WHEEL_NONE = 0xFFFF,
    DM1_PC34_CHEST_SCROLL_WHEEL_END_OF_LIST = 0xFFFE,
    DM1_PC34_CHEST_SCROLL_WHEEL_OPEN_CHEST = 0x7600,
    DM1_PC34_CHEST_SCROLL_WHEEL_ITEM0 = 0x7610,
    DM1_PC34_CHEST_SCROLL_WHEEL_ITEM1 = 0x7611,
    DM1_PC34_CHEST_SCROLL_WHEEL_ITEM2 = 0x7612,
    DM1_PC34_CHEST_SCROLL_WHEEL_ITEM3 = 0x7613,
    DM1_PC34_CHEST_SCROLL_WHEEL_LEADER_ITEM = 0x7620,
    DM1_PC34_CHEST_SCROLL_WHEEL_LEADER_ITEM_WEIGHT = 9,
    DM1_PC34_CHEST_SCROLL_WHEEL_BASE_LOAD = 40,
    DM1_PC34_CHEST_SCROLL_WHEEL_LOAD_MASK = 0x0200,
    DM1_PC34_CHEST_SCROLL_WHEEL_ALLOWED_CONTAINER = 0x0400
};

typedef struct {
    int thing;
    int weight;
    int icon;
} DM1_V1_ChestScrollWheelPickupDropItemPc34;

typedef struct {
    int contractOnly;
    int c537SlotZone;
    int c544SlotZone;
    int c30BaseSlot;
    int thingNone;
    int endOfList;
    int visibleItemCount;
    int highlightCycleCount;
    int firstEmptySlotIndex;
    DM1_V1_ChestScrollWheelPickupDropItemPc34 leaderHandItem;
    const char* f0333OpenAnchor;
    const char* f0334CloseAnchor;
    const char* f0302DispatchAnchor;
    const char* f0297PutAnchor;
    const char* f0298RemoveAnchor;
    const char* panelHighlightAnchor;
    const char* commandDispatchAnchor;
    const char* mouseWheelAnchor;
    const char* objectIconAnchor;
    const char* blitMaskAnchor;
    const char* pixelParityMarker;
} DM1_V1_ChestScrollWheelPickupDropSpecPc34;

typedef struct {
    int openChestThing;
    int initialSlots[DM1_PC34_CHEST_SCROLL_WHEEL_SLOT_COUNT];
    int initialIcons[DM1_PC34_CHEST_SCROLL_WHEEL_SLOT_COUNT];
    int initialLeaderHand;
    int initialLeaderIcon;
    int initialLeaderLoad;
    int initialLeaderAttributes;
    int materializedVisibleCount;
    int firstEmptySlotBeforeDrop;
    int slotZones[DM1_PC34_CHEST_SCROLL_WHEEL_SLOT_COUNT];

    int highlightTrace
        [DM1_PC34_CHEST_SCROLL_WHEEL_HIGHLIGHT_CYCLES]
        [DM1_PC34_CHEST_SCROLL_WHEEL_HIGHLIGHT_STEPS];
    int unhighlightTrace
        [DM1_PC34_CHEST_SCROLL_WHEEL_HIGHLIGHT_CYCLES]
        [DM1_PC34_CHEST_SCROLL_WHEEL_HIGHLIGHT_STEPS];
    int highlightLeaderHandStable
        [DM1_PC34_CHEST_SCROLL_WHEEL_HIGHLIGHT_CYCLES];
    int highlightSlot4NeverSelected
        [DM1_PC34_CHEST_SCROLL_WHEEL_HIGHLIGHT_CYCLES];
    int highlightSlotsStable
        [DM1_PC34_CHEST_SCROLL_WHEEL_HIGHLIGHT_CYCLES];
    int highlightPointerQueueEvents;
    int highlightPartialMaskDispatches;

    int dropCommandSlotBox;
    int dropResolvedChestSlot;
    int dropFirstEmptySlot;
    int dropResult;
    int slot4AfterDrop;
    int leaderHandAfterDrop;
    int leaderLoadAfterDrop;
    int leaderAttributesAfterDrop;
    int leaderIdentityPreservedInChest;
    int screenUpdateBalancedAfterDrop;

    int closeCount;
    int closedHead;
    int closeEndSentinel;
    int closedSlots[DM1_PC34_CHEST_SCROLL_WHEEL_SLOT_COUNT];
    int closeVisibleOrderStable;
    int closeDropItemAppended;
    int g0425ClearedAfterClose;
    int noClaimPixelParity;
} DM1_V1_ChestScrollWheelPickupDropProbePc34;

extern const DM1_V1_ChestScrollWheelPickupDropSpecPc34
    dm1_v1_chest_scroll_wheel_pickup_drop_pc34_spec;

const char*
dm1_v1_chest_scroll_wheel_pickup_drop_source_evidence_pc34(void);
const DM1_V1_ChestScrollWheelPickupDropSpecPc34*
dm1_v1_chest_scroll_wheel_pickup_drop_spec_pc34(void);
int dm1_v1_chest_scroll_wheel_pickup_drop_pc34(
    DM1_V1_ChestScrollWheelPickupDropProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CHEST_SCROLL_WHEEL_PICKUP_DROP_PC34_COMPAT_H */

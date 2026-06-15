#ifndef FIRESTAFF_DM1_V1_CHEST_C540_PICKUP_C040_LIVE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_C540_PICKUP_C040_LIVE_PC34_COMPAT_H

/*
 * Contract-only DM1 V1 regression gate for a queued C540 chest-slot
 * scroll-wheel pickup while the M568/C040 mirror-candidate panel is live.
 *
 * ReDMCSB anchors:
 * - CHEST.C F0333:30-67 opens G0426 and materializes C537..C544/G0425.
 * - CHEST.C F0334:79-130 closes G0426 and rewrites the chest chain.
 * - CHAMPION.C F0297:243-268 puts the picked thing in the leader hand.
 * - CHAMPION.C F0298:270-298 removes/flags the leader hand path.
 * - CHAMPION.C F0300:485,564,575 uses MASK0x0800_PANEL around slot churn.
 * - CHAMPION.C F0301:606-660 writes C30..C37/G0425 slot state.
 * - CHAMPION.C F0302:662-714 dispatches C28..C65 slot-box commands.
 * - COMMAND.C F0359:1452-1662 queues mouse commands.
 * - COMMAND.C F0361:1709-1813 queues wheel-like keyboard commands.
 * - COMMAND.C F0378:1956-1993 routes panel clicks by panel content.
 * - COMMAND.C F0380:2045-2178 drains the queued command in order.
 * - MOUSE.C F0077:1-32 and F0078:33-64 bracket screen updates.
 * - PANEL.C F0344:1493-1561 reads food/water bar values.
 * - PANEL.C F0345:1563-1617 draws food/water, including close pressure.
 * - PANEL.C F0346:1619-1637 draws M568/C040 resurrect/reincarnate.
 * - PANEL.C F0347:1639-1693 routes live G0299 to the C040 panel.
 * - REVIVE.C F0280:124-132 publishes the mirror candidate.
 * - REVIVE.C F0282:744-806 clears only through the pending panel path.
 * - DEFS.H C040/C540/C537..C544/G0299/G0425/G0426/G0424/M568/M070/M516.
 *
 * No game data, no graphics, no pixel parity claim.
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_V1_CHEST_C540_PICKUP_C040_LIVE_SLOT_COUNT_PC34 = 8,
    DM1_V1_CHEST_C540_PICKUP_C040_LIVE_TRACE_CAPACITY_PC34 = 16,
    DM1_V1_CHEST_C540_PICKUP_C040_LIVE_TARGET_SLOT_PC34 = 3,
    DM1_V1_CHEST_C540_PICKUP_C040_LIVE_C537_BASE_PC34 = 537,
    DM1_V1_CHEST_C540_PICKUP_C040_LIVE_C540_ZONE_PC34 = 540,
    DM1_V1_CHEST_C540_PICKUP_C040_LIVE_C38_SLOT_BOX_PC34 = 38,
    DM1_V1_CHEST_C540_PICKUP_C040_LIVE_C41_SLOT_BOX_PC34 = 41,
    DM1_V1_CHEST_C540_PICKUP_C040_LIVE_C33_CHEST_SLOT_PC34 = 33,
    DM1_V1_CHEST_C540_PICKUP_C040_LIVE_C061_COMMAND_PC34 = 61,
    DM1_V1_CHEST_C540_PICKUP_C040_LIVE_C040_GRAPHIC_PC34 = 40,
    DM1_V1_CHEST_C540_PICKUP_C040_LIVE_M568_PANEL_PC34 = 5,
    DM1_V1_CHEST_C540_PICKUP_C040_LIVE_C040_COMMAND_PC34 = 568,
    DM1_V1_CHEST_C540_PICKUP_C040_LIVE_LEADER_PC34 = 0,
    DM1_V1_CHEST_C540_PICKUP_C040_LIVE_CANDIDATE_OWNER_PC34 = 2,
    DM1_V1_CHEST_C540_PICKUP_C040_LIVE_CANDIDATE_ORDINAL_PC34 = 3,
    DM1_V1_CHEST_C540_PICKUP_C040_LIVE_LEADER_FOOD_PC34 = 1500,
    DM1_V1_CHEST_C540_PICKUP_C040_LIVE_LEADER_WATER_PC34 = 1500,
    DM1_V1_CHEST_C540_PICKUP_C040_LIVE_NONE_PC34 = 0
};

typedef enum {
    DM1_V1_CHEST_C540_PICKUP_C040_LIVE_REJECT_NONE_PC34 = 0,
    DM1_V1_CHEST_C540_PICKUP_C040_LIVE_REJECT_NO_C040_PC34,
    DM1_V1_CHEST_C540_PICKUP_C040_LIVE_REJECT_ALIVE_CANDIDATE_PC34,
    DM1_V1_CHEST_C540_PICKUP_C040_LIVE_REJECT_CHEST_CLOSED_PC34,
    DM1_V1_CHEST_C540_PICKUP_C040_LIVE_REJECT_LEADER_HAND_FULL_PC34,
    DM1_V1_CHEST_C540_PICKUP_C040_LIVE_REJECT_QUEUE_EMPTY_PC34
} DM1_V1_ChestC540PickupC040LiveRejectPc34;

typedef enum {
    DM1_V1_CHEST_C540_PICKUP_C040_LIVE_TRACE_F0359_QUEUE_PC34 = 1,
    DM1_V1_CHEST_C540_PICKUP_C040_LIVE_TRACE_F0361_MARK_WHEEL_PC34 = 2,
    DM1_V1_CHEST_C540_PICKUP_C040_LIVE_TRACE_F0378_ROUTE_PC34 = 3,
    DM1_V1_CHEST_C540_PICKUP_C040_LIVE_TRACE_F0380_DRAIN_PC34 = 4,
    DM1_V1_CHEST_C540_PICKUP_C040_LIVE_TRACE_F0302_SLOT_HELPER_PC34 = 5,
    DM1_V1_CHEST_C540_PICKUP_C040_LIVE_TRACE_F0301_CHEST_SLOT_PC34 = 6,
    DM1_V1_CHEST_C540_PICKUP_C040_LIVE_TRACE_F0297_F0298_STAT_PC34 = 7,
    DM1_V1_CHEST_C540_PICKUP_C040_LIVE_TRACE_F0077_ENABLE_PC34 = 8,
    DM1_V1_CHEST_C540_PICKUP_C040_LIVE_TRACE_F0078_DISABLE_PC34 = 9,
    DM1_V1_CHEST_C540_PICKUP_C040_LIVE_TRACE_F0344_F0345_FOOD_WATER_PC34 = 10,
    DM1_V1_CHEST_C540_PICKUP_C040_LIVE_TRACE_F0346_RESURRECT_DRAW_PC34 = 11,
    DM1_V1_CHEST_C540_PICKUP_C040_LIVE_TRACE_F0347_PANEL_ROUTER_PC34 = 12,
    DM1_V1_CHEST_C540_PICKUP_C040_LIVE_TRACE_F0334_CLOSE_PC34 = 13,
    DM1_V1_CHEST_C540_PICKUP_C040_LIVE_TRACE_F0333_REOPEN_PC34 = 14
} DM1_V1_ChestC540PickupC040LiveOpcodePc34;

typedef struct {
    int itemType;
    int weight;
    int charges;
} DM1_V1_ChestC540PickupC040LiveItemBytesPc34;

typedef struct {
    int g0299CandidateOrdinal;
    int g0424PanelContent;
    int c040PanelOpen;
    int c040PanelGraphic;
    int c040PanelCommand;
    int c040PanelOwnerSlot;
    int c040PanelC038SlotBox;
    int resurrectPendingFlag;
    uint32_t c030ChainHash;
} DM1_V1_ChestC540PickupC040LivePanelStatePc34;

typedef struct {
    int opcode;
    int command;
    int zone;
    int slotBox;
    int chestSlot;
    int pickedThing;
    DM1_V1_ChestC540PickupC040LivePanelStatePc34 panel;
} DM1_V1_ChestC540PickupC040LiveTracePc34;

typedef struct {
    const char* contractName;
    const char* sourceEvidence;
    const char* opcodeTableName;
    const int* kTraceOpcodes;
    int traceOpcodeCount;
    int contractOnly;
    int noGameData;
    int c540Zone;
    int c040Graphic;
    int m568Panel;
    int c040PanelCommand;
    int c040PanelC038SlotBox;
    int c540Command;
    int c33ChestSlot;
    const char* chestOpenAnchor;
    const char* chestCloseAnchor;
    const char* championStatAnchor;
    const char* championFlagAnchor;
    const char* championPanelMaskAnchor;
    const char* championChestSlotAnchor;
    const char* championInventoryAnchor;
    const char* commandQueueAnchor;
    const char* commandWheelAnchor;
    const char* commandRouteAnchor;
    const char* commandDrainAnchor;
    const char* mouseBracketAnchor;
    const char* panelFoodReadAnchor;
    const char* panelFoodDrawAnchor;
    const char* panelResurrectDrawAnchor;
    const char* panelRouterAnchor;
    const char* revivePublishAnchor;
    const char* reviveClearAnchor;
    const char* defsAnchor;
    const char* disjointness;
} DM1_V1_ChestC540PickupC040LiveMetadataPc34;

typedef struct {
    int contractOnly;
    int noGameData;
    int queueHasC540;
    int c040PanelOpen;
    int candidateOrdinal;
    int candidateAlive;
    int resurrectPendingFlag;
    int panelContent;
    int c040PanelGraphic;
    int c040PanelCommand;
    int c040PanelOwnerSlot;
    int c040PanelC038SlotBox;
    int chestOpen;
    int leaderHandFull;
    int leaderHandStackable;
    int leaderFood;
    int leaderWater;
    int visibleSlots[DM1_V1_CHEST_C540_PICKUP_C040_LIVE_SLOT_COUNT_PC34];
    int chestChain[DM1_V1_CHEST_C540_PICKUP_C040_LIVE_SLOT_COUNT_PC34];
    int candidateChain[DM1_V1_CHEST_C540_PICKUP_C040_LIVE_SLOT_COUNT_PC34];
    DM1_V1_ChestC540PickupC040LiveItemBytesPc34 pickedBytes;
    DM1_V1_ChestC540PickupC040LiveItemBytesPc34 leaderHandBefore;
} DM1_V1_ChestC540PickupC040LiveScenarioPc34;

typedef struct {
    int accepted;
    int rejectReason;
    int traceCount;
    DM1_V1_ChestC540PickupC040LiveTracePc34 trace[
        DM1_V1_CHEST_C540_PICKUP_C040_LIVE_TRACE_CAPACITY_PC34];
    uint32_t deterministicHash;
    DM1_V1_ChestC540PickupC040LivePanelStatePc34 panelBefore;
    DM1_V1_ChestC540PickupC040LivePanelStatePc34 panelAfterC540;
    DM1_V1_ChestC540PickupC040LiveItemBytesPc34 leaderHandAfter;
    int visibleSlotsAfterC540[DM1_V1_CHEST_C540_PICKUP_C040_LIVE_SLOT_COUNT_PC34];
    int visibleSlotsAfterCloseReopen[
        DM1_V1_CHEST_C540_PICKUP_C040_LIVE_SLOT_COUNT_PC34];
    int chestChainAfterC540[DM1_V1_CHEST_C540_PICKUP_C040_LIVE_SLOT_COUNT_PC34];
    int chestChainAfterCloseReopen[
        DM1_V1_CHEST_C540_PICKUP_C040_LIVE_SLOT_COUNT_PC34];
    int leaderFoodAfter;
    int leaderWaterAfter;
    int f0077EnableCount;
    int f0078DisableCount;
    int f0282ClearCount;
    int f0344FoodWaterReadCount;
    int f0345FoodWaterDrawCount;
    int f0346ResurrectDrawCount;
    int f0347PanelRouterCount;
    int closeCount;
    int reopenCount;
} DM1_V1_ChestC540PickupC040LiveResultPc34;

const char*
dm1_v1_chest_c540_pickup_c040_live_source_evidence_pc34(void);
const DM1_V1_ChestC540PickupC040LiveMetadataPc34*
dm1_v1_chest_c540_pickup_c040_live_metadata_pc34(void);
void dm1_v1_chest_c540_pickup_c040_live_init_scenario_pc34(
    DM1_V1_ChestC540PickupC040LiveScenarioPc34* scenario);
int dm1_v1_chest_c540_pickup_c040_live_run_pc34(
    const DM1_V1_ChestC540PickupC040LiveScenarioPc34* scenario,
    DM1_V1_ChestC540PickupC040LiveResultPc34* out);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CHEST_C540_PICKUP_C040_LIVE_PC34_COMPAT_H */

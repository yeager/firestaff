#include "firestaff/dm1/v1/chest/c540_pickup_c040_live_pc34_compat.h"

#include <stddef.h>
#include <string.h>

/*
 * The model below is intentionally contract-only.  It pins the original
 * dispatch order named by ReDMCSB COMMAND.C F0359/F0361/F0378/F0380 and the
 * inventory side effects named by CHEST.C F0333/F0334 and CHAMPION.C
 * F0297/F0298/F0300/F0301/F0302 while a REVIVE.C F0280/F0282 M568/C040
 * candidate remains live through PANEL.C F0344/F0345/F0346/F0347 redraws and
 * a MOUSE.C F0077/F0078 screen-update bracket.
 */

static const int kTraceOpcodesPc34[] = {
    DM1_V1_CHEST_C540_PICKUP_C040_LIVE_TRACE_F0359_QUEUE_PC34,
    DM1_V1_CHEST_C540_PICKUP_C040_LIVE_TRACE_F0361_MARK_WHEEL_PC34,
    DM1_V1_CHEST_C540_PICKUP_C040_LIVE_TRACE_F0378_ROUTE_PC34,
    DM1_V1_CHEST_C540_PICKUP_C040_LIVE_TRACE_F0380_DRAIN_PC34,
    DM1_V1_CHEST_C540_PICKUP_C040_LIVE_TRACE_F0302_SLOT_HELPER_PC34,
    DM1_V1_CHEST_C540_PICKUP_C040_LIVE_TRACE_F0301_CHEST_SLOT_PC34,
    DM1_V1_CHEST_C540_PICKUP_C040_LIVE_TRACE_F0297_F0298_STAT_PC34,
    DM1_V1_CHEST_C540_PICKUP_C040_LIVE_TRACE_F0077_ENABLE_PC34,
    DM1_V1_CHEST_C540_PICKUP_C040_LIVE_TRACE_F0078_DISABLE_PC34,
    DM1_V1_CHEST_C540_PICKUP_C040_LIVE_TRACE_F0344_F0345_FOOD_WATER_PC34,
    DM1_V1_CHEST_C540_PICKUP_C040_LIVE_TRACE_F0346_RESURRECT_DRAW_PC34,
    DM1_V1_CHEST_C540_PICKUP_C040_LIVE_TRACE_F0347_PANEL_ROUTER_PC34,
    DM1_V1_CHEST_C540_PICKUP_C040_LIVE_TRACE_F0334_CLOSE_PC34,
    DM1_V1_CHEST_C540_PICKUP_C040_LIVE_TRACE_F0333_REOPEN_PC34
};

static const char kSourceEvidencePc34[] =
    "CHEST.C F0333:30-67 opens G0426 and fills C537..C544/G0425\n"
    "CHEST.C F0334:79-130 closes G0426 and rewrites the chest chain\n"
    "CHAMPION.C F0297:243-268 puts the picked thing in C01 leader hand\n"
    "CHAMPION.C F0298:270-298 owns leader-hand removal/flag bookkeeping\n"
    "CHAMPION.C F0300:485,564,575 is the MASK0x0800_PANEL slot churn anchor\n"
    "CHAMPION.C F0301:606-660 writes C30..C37/G0425 slot state\n"
    "CHAMPION.C F0302:662-714 dispatches C28..C65 slot-box clicks\n"
    "COMMAND.C F0359:1452-1662 queues the C540 mouse command\n"
    "COMMAND.C F0361:1709-1813 marks wheel-like queued commands\n"
    "COMMAND.C F0378:1956-1993 routes by panel content\n"
    "COMMAND.C F0380:2045-2178 drains queued slot commands\n"
    "MOUSE.C F0077:1-32 and F0078:33-64 balance screen updates\n"
    "PANEL.C F0344:1493-1561 reads food/water bar values\n"
    "PANEL.C F0345:1563-1617 draws food/water without changing bytes here\n"
    "PANEL.C F0346:1619-1637 redraws the M568/C040 panel\n"
    "PANEL.C F0347:1639-1693 routes live G0299 back to C040\n"
    "REVIVE.C F0280:124-132 publishes the mirror candidate\n"
    "REVIVE.C F0282:744-806 clears only through C160..C162 pending path\n"
    "DEFS.H C040/C540/C537..C544/G0299/G0425/G0426/G0424/M568/M070/M516\n"
    "Disjointness: this is the C540 chest-slot pickup with live C040 "
    "resurrect-pending panel, not the existing chest pickup/drop, non-leader, "
    "overflow, close race, C061 drop, partial-mask, or mirror-candidate pickup "
    "lanes.";

static const DM1_V1_ChestC540PickupC040LiveMetadataPc34 kMetadataPc34 = {
    "DM1 V1 chest C540 scroll-wheel pickup while C040 mirror candidate is live",
    kSourceEvidencePc34,
    "kTraceOpcodesPc34",
    kTraceOpcodesPc34,
    (int)(sizeof(kTraceOpcodesPc34) / sizeof(kTraceOpcodesPc34[0])),
    1,
    1,
    DM1_V1_CHEST_C540_PICKUP_C040_LIVE_C540_ZONE_PC34,
    DM1_V1_CHEST_C540_PICKUP_C040_LIVE_C040_GRAPHIC_PC34,
    DM1_V1_CHEST_C540_PICKUP_C040_LIVE_M568_PANEL_PC34,
    DM1_V1_CHEST_C540_PICKUP_C040_LIVE_C040_COMMAND_PC34,
    DM1_V1_CHEST_C540_PICKUP_C040_LIVE_C38_SLOT_BOX_PC34,
    DM1_V1_CHEST_C540_PICKUP_C040_LIVE_C061_COMMAND_PC34,
    DM1_V1_CHEST_C540_PICKUP_C040_LIVE_C33_CHEST_SLOT_PC34,
    "CHEST.C F0333:30-67",
    "CHEST.C F0334:79-130",
    "CHAMPION.C F0297:243-268",
    "CHAMPION.C F0298:270-298",
    "CHAMPION.C F0300:485,564,575 MASK0x0800_PANEL",
    "CHAMPION.C F0301:606-660",
    "CHAMPION.C F0302:662-714",
    "COMMAND.C F0359:1452-1662",
    "COMMAND.C F0361:1709-1813",
    "COMMAND.C F0378:1956-1993",
    "COMMAND.C F0380:2045-2178",
    "MOUSE.C F0077:1-32/F0078:33-64",
    "PANEL.C F0344:1493-1561",
    "PANEL.C F0345:1563-1617",
    "PANEL.C F0346:1619-1637",
    "PANEL.C F0347:1639-1693",
    "REVIVE.C F0280:124-132",
    "REVIVE.C F0282:744-806",
    "DEFS.H C040/C540/C537..C544/G0299/G0425/G0426/G0424/M568/M070/M516",
    "Non-duplicative C540 pickup while C040 resurrect-pending panel is live."
};

static uint32_t fnv1a_u32(uint32_t hash, uint32_t value)
{
    int i;

    for (i = 0; i < 4; ++i) {
        hash ^= (value >> (i * 8)) & 0xffu;
        hash *= 16777619u;
    }
    return hash;
}

static uint32_t hash_int_array(const int* values, int count)
{
    int i;
    uint32_t hash = 2166136261u;

    for (i = 0; i < count; ++i) {
        hash = fnv1a_u32(hash, (uint32_t)values[i]);
    }
    return hash;
}

static DM1_V1_ChestC540PickupC040LivePanelStatePc34 panel_from_scenario(
    const DM1_V1_ChestC540PickupC040LiveScenarioPc34* scenario)
{
    DM1_V1_ChestC540PickupC040LivePanelStatePc34 panel;

    memset(&panel, 0, sizeof(panel));
    panel.g0299CandidateOrdinal = scenario->candidateOrdinal;
    panel.g0424PanelContent = scenario->panelContent;
    panel.c040PanelOpen = scenario->c040PanelOpen;
    panel.c040PanelGraphic = scenario->c040PanelGraphic;
    panel.c040PanelCommand = scenario->c040PanelCommand;
    panel.c040PanelOwnerSlot = scenario->c040PanelOwnerSlot;
    panel.c040PanelC038SlotBox = scenario->c040PanelC038SlotBox;
    panel.resurrectPendingFlag = scenario->resurrectPendingFlag;
    panel.c030ChainHash = hash_int_array(
        scenario->candidateChain,
        DM1_V1_CHEST_C540_PICKUP_C040_LIVE_SLOT_COUNT_PC34);
    return panel;
}

static uint32_t hash_panel(uint32_t hash,
                           DM1_V1_ChestC540PickupC040LivePanelStatePc34 panel)
{
    hash = fnv1a_u32(hash, (uint32_t)panel.g0299CandidateOrdinal);
    hash = fnv1a_u32(hash, (uint32_t)panel.g0424PanelContent);
    hash = fnv1a_u32(hash, (uint32_t)panel.c040PanelOpen);
    hash = fnv1a_u32(hash, (uint32_t)panel.c040PanelGraphic);
    hash = fnv1a_u32(hash, (uint32_t)panel.c040PanelCommand);
    hash = fnv1a_u32(hash, (uint32_t)panel.c040PanelOwnerSlot);
    hash = fnv1a_u32(hash, (uint32_t)panel.c040PanelC038SlotBox);
    hash = fnv1a_u32(hash, (uint32_t)panel.resurrectPendingFlag);
    hash = fnv1a_u32(hash, panel.c030ChainHash);
    return hash;
}

static uint32_t hash_trace(
    const DM1_V1_ChestC540PickupC040LiveTracePc34* trace,
    int count)
{
    int i;
    uint32_t hash = 2166136261u;

    for (i = 0; i < count; ++i) {
        hash = fnv1a_u32(hash, (uint32_t)trace[i].opcode);
        hash = fnv1a_u32(hash, (uint32_t)trace[i].command);
        hash = fnv1a_u32(hash, (uint32_t)trace[i].zone);
        hash = fnv1a_u32(hash, (uint32_t)trace[i].slotBox);
        hash = fnv1a_u32(hash, (uint32_t)trace[i].chestSlot);
        hash = fnv1a_u32(hash, (uint32_t)trace[i].pickedThing);
        hash = hash_panel(hash, trace[i].panel);
    }
    return hash;
}

static int live_c040_ready(
    const DM1_V1_ChestC540PickupC040LiveScenarioPc34* scenario)
{
    return scenario->c040PanelOpen == 1 &&
           scenario->candidateOrdinal != 0 &&
           scenario->panelContent ==
               DM1_V1_CHEST_C540_PICKUP_C040_LIVE_M568_PANEL_PC34 &&
           scenario->c040PanelGraphic ==
               DM1_V1_CHEST_C540_PICKUP_C040_LIVE_C040_GRAPHIC_PC34 &&
           scenario->c040PanelCommand ==
               DM1_V1_CHEST_C540_PICKUP_C040_LIVE_C040_COMMAND_PC34 &&
           scenario->c040PanelC038SlotBox ==
               DM1_V1_CHEST_C540_PICKUP_C040_LIVE_C38_SLOT_BOX_PC34;
}

static int validate(
    const DM1_V1_ChestC540PickupC040LiveScenarioPc34* scenario)
{
    if (!live_c040_ready(scenario)) {
        return DM1_V1_CHEST_C540_PICKUP_C040_LIVE_REJECT_NO_C040_PC34;
    }
    if (scenario->candidateAlive || !scenario->resurrectPendingFlag) {
        return DM1_V1_CHEST_C540_PICKUP_C040_LIVE_REJECT_ALIVE_CANDIDATE_PC34;
    }
    if (!scenario->chestOpen ||
        scenario->visibleSlots[
            DM1_V1_CHEST_C540_PICKUP_C040_LIVE_TARGET_SLOT_PC34] ==
            DM1_V1_CHEST_C540_PICKUP_C040_LIVE_NONE_PC34) {
        return DM1_V1_CHEST_C540_PICKUP_C040_LIVE_REJECT_CHEST_CLOSED_PC34;
    }
    if (scenario->leaderHandFull && !scenario->leaderHandStackable) {
        return DM1_V1_CHEST_C540_PICKUP_C040_LIVE_REJECT_LEADER_HAND_FULL_PC34;
    }
    if (!scenario->queueHasC540) {
        return DM1_V1_CHEST_C540_PICKUP_C040_LIVE_REJECT_QUEUE_EMPTY_PC34;
    }
    return DM1_V1_CHEST_C540_PICKUP_C040_LIVE_REJECT_NONE_PC34;
}

static void add_trace(DM1_V1_ChestC540PickupC040LiveResultPc34* out,
                      int opcode,
                      const DM1_V1_ChestC540PickupC040LivePanelStatePc34* panel,
                      int pickedThing)
{
    DM1_V1_ChestC540PickupC040LiveTracePc34* trace;

    if (out->traceCount >=
        DM1_V1_CHEST_C540_PICKUP_C040_LIVE_TRACE_CAPACITY_PC34) {
        return;
    }
    trace = &out->trace[out->traceCount++];
    memset(trace, 0, sizeof(*trace));
    trace->opcode = opcode;
    trace->command = DM1_V1_CHEST_C540_PICKUP_C040_LIVE_C061_COMMAND_PC34;
    trace->zone = DM1_V1_CHEST_C540_PICKUP_C040_LIVE_C540_ZONE_PC34;
    trace->slotBox = DM1_V1_CHEST_C540_PICKUP_C040_LIVE_C41_SLOT_BOX_PC34;
    trace->chestSlot = DM1_V1_CHEST_C540_PICKUP_C040_LIVE_C33_CHEST_SLOT_PC34;
    trace->pickedThing = pickedThing;
    trace->panel = *panel;
}

const char*
dm1_v1_chest_c540_pickup_c040_live_source_evidence_pc34(void)
{
    return kSourceEvidencePc34;
}

const DM1_V1_ChestC540PickupC040LiveMetadataPc34*
dm1_v1_chest_c540_pickup_c040_live_metadata_pc34(void)
{
    return &kMetadataPc34;
}

void dm1_v1_chest_c540_pickup_c040_live_init_scenario_pc34(
    DM1_V1_ChestC540PickupC040LiveScenarioPc34* scenario)
{
    int i;
    static const int visible[
        DM1_V1_CHEST_C540_PICKUP_C040_LIVE_SLOT_COUNT_PC34] = {
        0x7101, 0x7102, 0x7103, 0x7540, 0, 0, 0, 0
    };
    static const int candidate[
        DM1_V1_CHEST_C540_PICKUP_C040_LIVE_SLOT_COUNT_PC34] = {
        0xC030, 0xC031, 0xC032, 0xC033, 0xC034, 0xC035, 0xC036, 0xC037
    };

    if (!scenario) {
        return;
    }
    memset(scenario, 0, sizeof(*scenario));
    scenario->contractOnly = 1;
    scenario->noGameData = 1;
    scenario->queueHasC540 = 1;
    scenario->c040PanelOpen = 1;
    scenario->candidateOrdinal =
        DM1_V1_CHEST_C540_PICKUP_C040_LIVE_CANDIDATE_ORDINAL_PC34;
    scenario->candidateAlive = 0;
    scenario->resurrectPendingFlag = 1;
    scenario->panelContent =
        DM1_V1_CHEST_C540_PICKUP_C040_LIVE_M568_PANEL_PC34;
    scenario->c040PanelGraphic =
        DM1_V1_CHEST_C540_PICKUP_C040_LIVE_C040_GRAPHIC_PC34;
    scenario->c040PanelCommand =
        DM1_V1_CHEST_C540_PICKUP_C040_LIVE_C040_COMMAND_PC34;
    scenario->c040PanelOwnerSlot =
        DM1_V1_CHEST_C540_PICKUP_C040_LIVE_CANDIDATE_OWNER_PC34;
    scenario->c040PanelC038SlotBox =
        DM1_V1_CHEST_C540_PICKUP_C040_LIVE_C38_SLOT_BOX_PC34;
    scenario->chestOpen = 1;
    scenario->leaderHandFull = 0;
    scenario->leaderHandStackable = 0;
    scenario->leaderFood =
        DM1_V1_CHEST_C540_PICKUP_C040_LIVE_LEADER_FOOD_PC34;
    scenario->leaderWater =
        DM1_V1_CHEST_C540_PICKUP_C040_LIVE_LEADER_WATER_PC34;
    scenario->pickedBytes.itemType = 0x54;
    scenario->pickedBytes.weight = 7;
    scenario->pickedBytes.charges = 3;
    scenario->leaderHandBefore.itemType = 0;
    scenario->leaderHandBefore.weight = 0;
    scenario->leaderHandBefore.charges = 0;
    for (i = 0; i < DM1_V1_CHEST_C540_PICKUP_C040_LIVE_SLOT_COUNT_PC34; ++i) {
        scenario->visibleSlots[i] = visible[i];
        scenario->chestChain[i] = visible[i];
        scenario->candidateChain[i] = candidate[i];
    }
}

int dm1_v1_chest_c540_pickup_c040_live_run_pc34(
    const DM1_V1_ChestC540PickupC040LiveScenarioPc34* scenario,
    DM1_V1_ChestC540PickupC040LiveResultPc34* out)
{
    int i;
    int reject;
    int pickedThing;
    DM1_V1_ChestC540PickupC040LivePanelStatePc34 panel;

    if (!scenario || !out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    panel = panel_from_scenario(scenario);
    out->panelBefore = panel;
    reject = validate(scenario);
    out->rejectReason = reject;
    if (reject != DM1_V1_CHEST_C540_PICKUP_C040_LIVE_REJECT_NONE_PC34) {
        out->deterministicHash = fnv1a_u32(2166136261u, (uint32_t)reject);
        out->panelAfterC540 = panel;
        return 0;
    }

    pickedThing =
        scenario->visibleSlots[
            DM1_V1_CHEST_C540_PICKUP_C040_LIVE_TARGET_SLOT_PC34];
    out->accepted = 1;

    /* ReDMCSB: COMMAND.C F0359:1452-1662 produces the queued C540 click. */
    add_trace(out,
              DM1_V1_CHEST_C540_PICKUP_C040_LIVE_TRACE_F0359_QUEUE_PC34,
              &panel,
              pickedThing);
    /* ReDMCSB: COMMAND.C F0361:1709-1813 marks the wheel-like queue leg. */
    add_trace(out,
              DM1_V1_CHEST_C540_PICKUP_C040_LIVE_TRACE_F0361_MARK_WHEEL_PC34,
              &panel,
              pickedThing);
    /* ReDMCSB: COMMAND.C F0378:1956-1993 keeps M568 routing distinct. */
    add_trace(out,
              DM1_V1_CHEST_C540_PICKUP_C040_LIVE_TRACE_F0378_ROUTE_PC34,
              &panel,
              pickedThing);
    /* ReDMCSB: COMMAND.C F0380:2045-2178 drains before slot dispatch. */
    add_trace(out,
              DM1_V1_CHEST_C540_PICKUP_C040_LIVE_TRACE_F0380_DRAIN_PC34,
              &panel,
              pickedThing);
    /* ReDMCSB: MOUSE.C F0077:1-32 opens the update bracket. */
    add_trace(out,
              DM1_V1_CHEST_C540_PICKUP_C040_LIVE_TRACE_F0077_ENABLE_PC34,
              &panel,
              pickedThing);
    out->f0077EnableCount = 1;
    /* ReDMCSB: CHAMPION.C F0302:662-714 dispatches slot box C41/C33. */
    add_trace(out,
              DM1_V1_CHEST_C540_PICKUP_C040_LIVE_TRACE_F0302_SLOT_HELPER_PC34,
              &panel,
              pickedThing);
    /* ReDMCSB: CHAMPION.C F0301:606-660 pins the C30..C37/G0425 write. */
    add_trace(out,
              DM1_V1_CHEST_C540_PICKUP_C040_LIVE_TRACE_F0301_CHEST_SLOT_PC34,
              &panel,
              pickedThing);
    /* ReDMCSB: CHAMPION.C F0297/F0298:243-298 pins hand/stat bytes. */
    add_trace(out,
              DM1_V1_CHEST_C540_PICKUP_C040_LIVE_TRACE_F0297_F0298_STAT_PC34,
              &panel,
              pickedThing);

    for (i = 0; i < DM1_V1_CHEST_C540_PICKUP_C040_LIVE_SLOT_COUNT_PC34; ++i) {
        out->visibleSlotsAfterC540[i] = scenario->visibleSlots[i];
        out->chestChainAfterC540[i] = scenario->chestChain[i];
    }
    out->visibleSlotsAfterC540[
        DM1_V1_CHEST_C540_PICKUP_C040_LIVE_TARGET_SLOT_PC34] =
        DM1_V1_CHEST_C540_PICKUP_C040_LIVE_NONE_PC34;
    out->chestChainAfterC540[
        DM1_V1_CHEST_C540_PICKUP_C040_LIVE_TARGET_SLOT_PC34] =
        DM1_V1_CHEST_C540_PICKUP_C040_LIVE_NONE_PC34;
    out->leaderHandAfter = scenario->pickedBytes;
    out->leaderFoodAfter = scenario->leaderFood;
    out->leaderWaterAfter = scenario->leaderWater;

    /* ReDMCSB: MOUSE.C F0078:33-64 closes the update bracket. */
    add_trace(out,
              DM1_V1_CHEST_C540_PICKUP_C040_LIVE_TRACE_F0078_DISABLE_PC34,
              &panel,
              pickedThing);
    out->f0078DisableCount = 1;
    /* ReDMCSB: PANEL.C F0344/F0345:1493-1617 food/water stay stable. */
    add_trace(out,
              DM1_V1_CHEST_C540_PICKUP_C040_LIVE_TRACE_F0344_F0345_FOOD_WATER_PC34,
              &panel,
              pickedThing);
    out->f0344FoodWaterReadCount = 1;
    out->f0345FoodWaterDrawCount = 1;
    /* ReDMCSB: PANEL.C F0346:1619-1637 redraws C040. */
    add_trace(out,
              DM1_V1_CHEST_C540_PICKUP_C040_LIVE_TRACE_F0346_RESURRECT_DRAW_PC34,
              &panel,
              pickedThing);
    out->f0346ResurrectDrawCount = 1;
    /* ReDMCSB: PANEL.C F0347:1639-1693 routes live G0299 to C040. */
    add_trace(out,
              DM1_V1_CHEST_C540_PICKUP_C040_LIVE_TRACE_F0347_PANEL_ROUTER_PC34,
              &panel,
              pickedThing);
    out->f0347PanelRouterCount = 1;
    out->panelAfterC540 = panel;

    /* ReDMCSB: CHEST.C F0334:79-130 rewrites the visible G0425 chain. */
    add_trace(out,
              DM1_V1_CHEST_C540_PICKUP_C040_LIVE_TRACE_F0334_CLOSE_PC34,
              &panel,
              pickedThing);
    out->closeCount = 1;
    /* ReDMCSB: CHEST.C F0333:30-67 reopens from the rewritten chain. */
    add_trace(out,
              DM1_V1_CHEST_C540_PICKUP_C040_LIVE_TRACE_F0333_REOPEN_PC34,
              &panel,
              pickedThing);
    out->reopenCount = 1;

    for (i = 0; i < DM1_V1_CHEST_C540_PICKUP_C040_LIVE_SLOT_COUNT_PC34; ++i) {
        out->chestChainAfterCloseReopen[i] = out->chestChainAfterC540[i];
        out->visibleSlotsAfterCloseReopen[i] = out->chestChainAfterC540[i];
    }
    out->f0282ClearCount = 0;
    out->deterministicHash = hash_trace(out->trace, out->traceCount);
    return 1;
}

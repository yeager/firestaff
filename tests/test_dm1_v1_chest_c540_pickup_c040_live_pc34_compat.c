#include "firestaff/dm1/v1/chest/c540_pickup_c040_live_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions;
static int g_failures;

static void check_true(int condition, const char* message, const char* anchor)
{
    ++g_assertions;
    if (!condition) {
        ++g_failures;
        printf("FAIL: %s [%s]\n", message, anchor ? anchor : "(null)");
    }
}

static void check_int(int actual, int expected, const char* message,
                      const char* anchor)
{
    ++g_assertions;
    if (actual != expected) {
        ++g_failures;
        printf("FAIL: %s actual=%d expected=%d [%s]\n",
               message,
               actual,
               expected,
               anchor ? anchor : "(null)");
    }
}

static void check_u32(unsigned int actual, unsigned int expected,
                      const char* message, const char* anchor)
{
    ++g_assertions;
    if (actual != expected) {
        ++g_failures;
        printf("FAIL: %s actual=0x%08X expected=0x%08X [%s]\n",
               message,
               actual,
               expected,
               anchor ? anchor : "(null)");
    }
}

static void check_contains(const char* text, const char* needle,
                           const char* message, const char* anchor)
{
    ++g_assertions;
    if (!text || !needle || !strstr(text, needle)) {
        ++g_failures;
        printf("FAIL: %s missing=%s [%s]\n",
               message,
               needle ? needle : "(null)",
               anchor ? anchor : "(null)");
    }
}

static void check_panel_stable(
    const DM1_V1_ChestC540PickupC040LivePanelStatePc34* before,
    const DM1_V1_ChestC540PickupC040LivePanelStatePc34* after,
    const DM1_V1_ChestC540PickupC040LiveMetadataPc34* meta)
{
    check_int(after->g0299CandidateOrdinal,
              before->g0299CandidateOrdinal,
              "G0299 candidate ordinal stays byte-stable",
              meta->revivePublishAnchor);
    check_true(after->g0299CandidateOrdinal != 0,
               "G0299 remains live",
               meta->revivePublishAnchor);
    check_int(after->g0424PanelContent,
              before->g0424PanelContent,
              "G0424/M568 panel content stays byte-stable",
              meta->panelRouterAnchor);
    check_int(after->c040PanelOpen, 1, "C040 panel stays open",
              meta->panelResurrectDrawAnchor);
    check_int(after->c040PanelGraphic,
              DM1_V1_CHEST_C540_PICKUP_C040_LIVE_C040_GRAPHIC_PC34,
              "C040 panel graphic stays C040",
              meta->defsAnchor);
    check_int(after->c040PanelCommand,
              DM1_V1_CHEST_C540_PICKUP_C040_LIVE_C040_COMMAND_PC34,
              "C040 panel command stays 568",
              meta->defsAnchor);
    check_int(after->c040PanelOwnerSlot,
              DM1_V1_CHEST_C540_PICKUP_C040_LIVE_CANDIDATE_OWNER_PC34,
              "C040 owner slot stays published candidate",
              meta->revivePublishAnchor);
    check_int(after->c040PanelC038SlotBox,
              DM1_V1_CHEST_C540_PICKUP_C040_LIVE_C38_SLOT_BOX_PC34,
              "C040 C038 slot box stays 38",
              meta->defsAnchor);
    check_int(after->resurrectPendingFlag,
              before->resurrectPendingFlag,
              "resurrect pending flag stays byte-stable",
              meta->reviveClearAnchor);
    check_u32(after->c030ChainHash,
              before->c030ChainHash,
              "C30..C37 mirror candidate chain stays byte-stable",
              meta->defsAnchor);
}

static void check_panel_equal(
    const DM1_V1_ChestC540PickupC040LivePanelStatePc34* before,
    const DM1_V1_ChestC540PickupC040LivePanelStatePc34* after,
    const DM1_V1_ChestC540PickupC040LiveMetadataPc34* meta)
{
    check_int(after->g0299CandidateOrdinal,
              before->g0299CandidateOrdinal,
              "rejected G0299 byte-stable",
              meta->revivePublishAnchor);
    check_int(after->g0424PanelContent,
              before->g0424PanelContent,
              "rejected G0424 byte-stable",
              meta->panelRouterAnchor);
    check_int(after->c040PanelOpen,
              before->c040PanelOpen,
              "rejected C040 open byte-stable",
              meta->panelResurrectDrawAnchor);
    check_int(after->c040PanelGraphic,
              before->c040PanelGraphic,
              "rejected C040 graphic byte-stable",
              meta->defsAnchor);
    check_int(after->c040PanelCommand,
              before->c040PanelCommand,
              "rejected C040 command byte-stable",
              meta->defsAnchor);
    check_int(after->c040PanelOwnerSlot,
              before->c040PanelOwnerSlot,
              "rejected C040 owner byte-stable",
              meta->revivePublishAnchor);
    check_int(after->c040PanelC038SlotBox,
              before->c040PanelC038SlotBox,
              "rejected C038 slot box byte-stable",
              meta->defsAnchor);
    check_int(after->resurrectPendingFlag,
              before->resurrectPendingFlag,
              "rejected resurrect flag byte-stable",
              meta->reviveClearAnchor);
    check_u32(after->c030ChainHash,
              before->c030ChainHash,
              "rejected C30..C37 chain byte-stable",
              meta->defsAnchor);
}

static void test_source_metadata(
    const DM1_V1_ChestC540PickupC040LiveMetadataPc34* meta)
{
    const char* source =
        dm1_v1_chest_c540_pickup_c040_live_source_evidence_pc34();

    check_true(meta != NULL, "metadata accessor returns data",
               "COMMAND.C F0380:2045-2178");
    check_int(meta->contractOnly, 1, "contract-only flag",
              meta->commandDrainAnchor);
    check_int(meta->noGameData, 1, "no-game-data flag",
              meta->chestOpenAnchor);
    check_int(meta->c540Zone,
              DM1_V1_CHEST_C540_PICKUP_C040_LIVE_C540_ZONE_PC34,
              "metadata C540 zone",
              meta->defsAnchor);
    check_int(meta->c040Graphic,
              DM1_V1_CHEST_C540_PICKUP_C040_LIVE_C040_GRAPHIC_PC34,
              "metadata C040 graphic",
              meta->defsAnchor);
    check_int(meta->m568Panel,
              DM1_V1_CHEST_C540_PICKUP_C040_LIVE_M568_PANEL_PC34,
              "metadata M568 panel",
              meta->defsAnchor);
    check_int(meta->c040PanelCommand,
              DM1_V1_CHEST_C540_PICKUP_C040_LIVE_C040_COMMAND_PC34,
              "metadata C040 command token",
              meta->defsAnchor);
    check_int(meta->c040PanelC038SlotBox,
              DM1_V1_CHEST_C540_PICKUP_C040_LIVE_C38_SLOT_BOX_PC34,
              "metadata C038 slot box",
              meta->defsAnchor);
    check_int(meta->c540Command,
              DM1_V1_CHEST_C540_PICKUP_C040_LIVE_C061_COMMAND_PC34,
              "metadata C540 command",
              meta->commandDrainAnchor);
    check_int(meta->traceOpcodeCount, 14, "metadata opcode count",
              meta->commandDrainAnchor);
    check_contains(source, "CHEST.C F0333:30-67",
                   "source cites F0333", meta->chestOpenAnchor);
    check_contains(source, "CHEST.C F0334:79-130",
                   "source cites F0334", meta->chestCloseAnchor);
    check_contains(source, "CHAMPION.C F0297:243-268",
                   "source cites F0297", meta->championStatAnchor);
    check_contains(source, "CHAMPION.C F0298:270-298",
                   "source cites F0298", meta->championFlagAnchor);
    check_contains(source, "CHAMPION.C F0300:485,564,575",
                   "source cites F0300 mask", meta->championPanelMaskAnchor);
    check_contains(source, "CHAMPION.C F0301:606-660",
                   "source cites F0301", meta->championChestSlotAnchor);
    check_contains(source, "CHAMPION.C F0302:662-714",
                   "source cites F0302", meta->championInventoryAnchor);
    check_contains(source, "COMMAND.C F0359:1452-1662",
                   "source cites F0359", meta->commandQueueAnchor);
    check_contains(source, "COMMAND.C F0361:1709-1813",
                   "source cites F0361", meta->commandWheelAnchor);
    check_contains(source, "COMMAND.C F0378:1956-1993",
                   "source cites F0378", meta->commandRouteAnchor);
    check_contains(source, "COMMAND.C F0380:2045-2178",
                   "source cites F0380", meta->commandDrainAnchor);
    check_contains(source, "MOUSE.C F0077:1-32",
                   "source cites F0077", meta->mouseBracketAnchor);
    check_contains(source, "F0078:33-64",
                   "source cites F0078", meta->mouseBracketAnchor);
    check_contains(source, "PANEL.C F0344:1493-1561",
                   "source cites F0344", meta->panelFoodReadAnchor);
    check_contains(source, "PANEL.C F0345:1563-1617",
                   "source cites F0345", meta->panelFoodDrawAnchor);
    check_contains(source, "PANEL.C F0346:1619-1637",
                   "source cites F0346", meta->panelResurrectDrawAnchor);
    check_contains(source, "PANEL.C F0347:1639-1693",
                   "source cites F0347", meta->panelRouterAnchor);
    check_contains(source, "REVIVE.C F0280:124-132",
                   "source cites F0280", meta->revivePublishAnchor);
    check_contains(source, "REVIVE.C F0282:744-806",
                   "source cites F0282", meta->reviveClearAnchor);
    check_contains(source, "DEFS.H C040/C540/C537..C544",
                   "source cites DEFS", meta->defsAnchor);
    check_contains(meta->disjointness, "C540 pickup while C040",
                   "metadata disjointness names lane", meta->disjointness);
}

static void test_live_scenario(
    const DM1_V1_ChestC540PickupC040LiveMetadataPc34* meta,
    unsigned int* out_hash)
{
    DM1_V1_ChestC540PickupC040LiveScenarioPc34 scenario;
    DM1_V1_ChestC540PickupC040LiveResultPc34 result;
    int i;
    int expectedOpcodes[] = {
        DM1_V1_CHEST_C540_PICKUP_C040_LIVE_TRACE_F0359_QUEUE_PC34,
        DM1_V1_CHEST_C540_PICKUP_C040_LIVE_TRACE_F0361_MARK_WHEEL_PC34,
        DM1_V1_CHEST_C540_PICKUP_C040_LIVE_TRACE_F0378_ROUTE_PC34,
        DM1_V1_CHEST_C540_PICKUP_C040_LIVE_TRACE_F0380_DRAIN_PC34,
        DM1_V1_CHEST_C540_PICKUP_C040_LIVE_TRACE_F0077_ENABLE_PC34,
        DM1_V1_CHEST_C540_PICKUP_C040_LIVE_TRACE_F0302_SLOT_HELPER_PC34,
        DM1_V1_CHEST_C540_PICKUP_C040_LIVE_TRACE_F0301_CHEST_SLOT_PC34,
        DM1_V1_CHEST_C540_PICKUP_C040_LIVE_TRACE_F0297_F0298_STAT_PC34,
        DM1_V1_CHEST_C540_PICKUP_C040_LIVE_TRACE_F0078_DISABLE_PC34,
        DM1_V1_CHEST_C540_PICKUP_C040_LIVE_TRACE_F0344_F0345_FOOD_WATER_PC34,
        DM1_V1_CHEST_C540_PICKUP_C040_LIVE_TRACE_F0346_RESURRECT_DRAW_PC34,
        DM1_V1_CHEST_C540_PICKUP_C040_LIVE_TRACE_F0347_PANEL_ROUTER_PC34,
        DM1_V1_CHEST_C540_PICKUP_C040_LIVE_TRACE_F0334_CLOSE_PC34,
        DM1_V1_CHEST_C540_PICKUP_C040_LIVE_TRACE_F0333_REOPEN_PC34
    };

    dm1_v1_chest_c540_pickup_c040_live_init_scenario_pc34(&scenario);
    check_int(dm1_v1_chest_c540_pickup_c040_live_run_pc34(&scenario, &result),
              1,
              "live C040 C540 pickup accepts",
              meta->commandDrainAnchor);
    check_int(result.accepted, 1, "result accepted",
              meta->commandDrainAnchor);
    check_int(result.rejectReason,
              DM1_V1_CHEST_C540_PICKUP_C040_LIVE_REJECT_NONE_PC34,
              "no reject reason",
              meta->commandDrainAnchor);
    check_int(result.traceCount, 14, "live trace count",
              meta->commandDrainAnchor);
    for (i = 0; i < result.traceCount; ++i) {
        char label[80];
        snprintf(label, sizeof(label), "trace opcode %d", i);
        check_int(result.trace[i].opcode, expectedOpcodes[i], label,
                  meta->commandDrainAnchor);
        check_panel_stable(&result.panelBefore, &result.trace[i].panel, meta);
    }
    check_panel_stable(&result.panelBefore, &result.panelAfterC540, meta);
    check_int(result.trace[0].opcode,
              DM1_V1_CHEST_C540_PICKUP_C040_LIVE_TRACE_F0359_QUEUE_PC34,
              "trace begins with F0359",
              meta->commandQueueAnchor);
    check_int(result.trace[1].opcode,
              DM1_V1_CHEST_C540_PICKUP_C040_LIVE_TRACE_F0361_MARK_WHEEL_PC34,
              "trace continues with F0361",
              meta->commandWheelAnchor);
    check_int(result.trace[2].opcode,
              DM1_V1_CHEST_C540_PICKUP_C040_LIVE_TRACE_F0378_ROUTE_PC34,
              "trace continues with F0378",
              meta->commandRouteAnchor);
    check_int(result.trace[3].opcode,
              DM1_V1_CHEST_C540_PICKUP_C040_LIVE_TRACE_F0380_DRAIN_PC34,
              "trace continues with F0380",
              meta->commandDrainAnchor);
    check_int(result.visibleSlotsAfterC540[
                  DM1_V1_CHEST_C540_PICKUP_C040_LIVE_TARGET_SLOT_PC34],
              DM1_V1_CHEST_C540_PICKUP_C040_LIVE_NONE_PC34,
              "C540 G0425 slot cleared",
              meta->championChestSlotAnchor);
    check_int(result.chestChainAfterC540[
                  DM1_V1_CHEST_C540_PICKUP_C040_LIVE_TARGET_SLOT_PC34],
              DM1_V1_CHEST_C540_PICKUP_C040_LIVE_NONE_PC34,
              "C540 chest chain cleared",
              meta->chestCloseAnchor);
    check_int(result.leaderHandAfter.itemType, scenario.pickedBytes.itemType,
              "leader C01 item type becomes picked thing",
              meta->championStatAnchor);
    check_int(result.leaderHandAfter.weight, scenario.pickedBytes.weight,
              "leader C01 weight becomes picked thing",
              meta->championStatAnchor);
    check_int(result.leaderHandAfter.charges, scenario.pickedBytes.charges,
              "leader C01 charges become picked thing",
              meta->championStatAnchor);
    check_int(result.leaderFoodAfter,
              DM1_V1_CHEST_C540_PICKUP_C040_LIVE_LEADER_FOOD_PC34,
              "leader M516 food byte-stable",
              meta->panelFoodReadAnchor);
    check_int(result.leaderWaterAfter,
              DM1_V1_CHEST_C540_PICKUP_C040_LIVE_LEADER_WATER_PC34,
              "leader M516 water byte-stable",
              meta->panelFoodReadAnchor);
    check_int(result.f0077EnableCount, 1, "F0077 enable count",
              meta->mouseBracketAnchor);
    check_int(result.f0078DisableCount, 1, "F0078 disable count",
              meta->mouseBracketAnchor);
    check_int(result.f0077EnableCount, result.f0078DisableCount,
              "mouse bracket balances",
              meta->mouseBracketAnchor);
    check_int(result.f0282ClearCount, 0, "F0282 candidate clear untouched",
              meta->reviveClearAnchor);
    check_int(result.f0344FoodWaterReadCount, 1, "F0344 food/water read",
              meta->panelFoodReadAnchor);
    check_int(result.f0345FoodWaterDrawCount, 1, "F0345 food/water draw",
              meta->panelFoodDrawAnchor);
    check_int(result.f0346ResurrectDrawCount, 1, "F0346 resurrect draw",
              meta->panelResurrectDrawAnchor);
    check_int(result.f0347PanelRouterCount, 1, "F0347 panel router",
              meta->panelRouterAnchor);
    check_int(result.closeCount, 1, "post-C540 close count",
              meta->chestCloseAnchor);
    check_int(result.reopenCount, 1, "post-C540 reopen count",
              meta->chestOpenAnchor);
    for (i = 0; i < DM1_V1_CHEST_C540_PICKUP_C040_LIVE_SLOT_COUNT_PC34; ++i) {
        char label[96];
        snprintf(label, sizeof(label), "close-reopen G0425 slot %d", i);
        check_int(result.visibleSlotsAfterCloseReopen[i],
                  result.chestChainAfterC540[i],
                  label,
                  meta->chestOpenAnchor);
    }
    check_true(result.deterministicHash != 0u, "deterministic hash populated",
               meta->commandDrainAnchor);
    *out_hash = result.deterministicHash;
}

static void expect_reject(
    const DM1_V1_ChestC540PickupC040LiveMetadataPc34* meta,
    DM1_V1_ChestC540PickupC040LiveScenarioPc34 scenario,
    int expectedReject,
    const char* message,
    const char* anchor)
{
    DM1_V1_ChestC540PickupC040LiveResultPc34 result;

    check_int(dm1_v1_chest_c540_pickup_c040_live_run_pc34(&scenario, &result),
              0,
              message,
              anchor);
    check_int(result.accepted, 0, "reject accepted flag stays zero",
              anchor);
    check_int(result.rejectReason, expectedReject, "reject reason", anchor);
    check_int(result.traceCount, 0, "reject has empty trace",
              meta->commandDrainAnchor);
    check_panel_equal(&result.panelBefore, &result.panelAfterC540, meta);
}

static void test_rejects(
    const DM1_V1_ChestC540PickupC040LiveMetadataPc34* meta)
{
    DM1_V1_ChestC540PickupC040LiveScenarioPc34 scenario;

    dm1_v1_chest_c540_pickup_c040_live_init_scenario_pc34(&scenario);
    scenario.c040PanelOpen = 0;
    expect_reject(meta,
                  scenario,
                  DM1_V1_CHEST_C540_PICKUP_C040_LIVE_REJECT_NO_C040_PC34,
                  "no-C040-panel state rejects",
                  meta->panelRouterAnchor);

    dm1_v1_chest_c540_pickup_c040_live_init_scenario_pc34(&scenario);
    scenario.candidateAlive = 1;
    expect_reject(meta,
                  scenario,
                  DM1_V1_CHEST_C540_PICKUP_C040_LIVE_REJECT_ALIVE_CANDIDATE_PC34,
                  "alive-only candidate rejects",
                  meta->revivePublishAnchor);

    dm1_v1_chest_c540_pickup_c040_live_init_scenario_pc34(&scenario);
    scenario.chestOpen = 0;
    expect_reject(meta,
                  scenario,
                  DM1_V1_CHEST_C540_PICKUP_C040_LIVE_REJECT_CHEST_CLOSED_PC34,
                  "closed chest rejects",
                  meta->chestOpenAnchor);

    dm1_v1_chest_c540_pickup_c040_live_init_scenario_pc34(&scenario);
    scenario.leaderHandFull = 1;
    scenario.leaderHandStackable = 0;
    expect_reject(meta,
                  scenario,
                  DM1_V1_CHEST_C540_PICKUP_C040_LIVE_REJECT_LEADER_HAND_FULL_PC34,
                  "full non-stackable leader hand rejects",
                  meta->championInventoryAnchor);

    dm1_v1_chest_c540_pickup_c040_live_init_scenario_pc34(&scenario);
    scenario.queueHasC540 = 0;
    expect_reject(meta,
                  scenario,
                  DM1_V1_CHEST_C540_PICKUP_C040_LIVE_REJECT_QUEUE_EMPTY_PC34,
                  "queue-empty input rejects",
                  meta->commandQueueAnchor);
}

int main(void)
{
    const DM1_V1_ChestC540PickupC040LiveMetadataPc34* meta;
    unsigned int deterministicHash = 0u;
    int passed;

    printf("probe=firestaff_dm1_v1_chest_c540_pickup_c040_live\n");
    meta = dm1_v1_chest_c540_pickup_c040_live_metadata_pc34();
    test_source_metadata(meta);
    test_live_scenario(meta, &deterministicHash);
    test_rejects(meta);
    passed = g_assertions - g_failures;
    printf("deterministic_hash=0x%08X\n", deterministicHash);
    printf("summary: %d/%d assertions passed\n", passed, g_assertions);
    if (g_failures) {
        printf("FAIL test_dm1_v1_chest_c540_pickup_c040_live_pc34_compat failures=%d\n",
               g_failures);
        return 1;
    }
    printf("PASS test_dm1_v1_chest_c540_pickup_c040_live_pc34_compat\n");
    return 0;
}

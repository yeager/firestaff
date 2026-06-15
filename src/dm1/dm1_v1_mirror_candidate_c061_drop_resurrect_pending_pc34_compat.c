#include "firestaff/dm1/v1/mirror_candidate/c061_drop_resurrect_pending_pc34_compat.h"

#include <string.h>

typedef struct {
    int itemType;
    int weight;
    int charges;
} C061ResItemPc34;

typedef struct {
    C061ResItemPc34 slots[DM1_V1_MC_C061_RES_SLOT_COUNT_PC34];
    C061ResItemPc34 leaderHand;
    int candidateChain[DM1_V1_MC_C061_RES_CHAIN_COUNT_PC34];
    int championHealth[DM1_V1_MC_C061_RES_PARTY_COUNT_PC34];
    int leaderIndex;
    int candidateOwnerIndex;
    int openChestOwnerIndex;
    int g0299CandidateOrdinal;
    int candidateGraphic;
    int candidateCommand;
    int panelContent;
    int c028Pending;
    int commandQueueDepth;
    int openChestThing;
    int f0280Count;
    int f0282ClearCount;
    int f0282CommitCount;
    int f0282CancelCount;
    int f0333OpenCount;
    int f0334CloseCount;
    int f0344FoodWaterCount;
    int f0345FoodWaterCount;
    int f0346C040Count;
    int f0355ToggleCount;
    int f0359QueuedDropCount;
    int f0368SetLeaderCount;
    int f0378PanelRouteCount;
    int f0380DrainCount;
    int f0302DropDispatchCount;
    int f0298RemoveHandCount;
    int f0300ClearCount;
    int f0301WriteCount;
    int f0297PutHandCount;
    int saveLoadCount;
    int teleporterCount;
    int partyRotateCount;
    int c160CloseCount;
    int c045AcceptCount;
    int leaderLoad;
} C061ResRuntimePc34;

static const char s_source_evidence[] =
    "CHEST.C F0333:30-67 opens/materializes G0426 into C537..C544/G0425\n"
    "CHEST.C F0334:117-132 rewires visible non-empty G0425 slots on close\n"
    "CHAMPION.C F0297:243-298 owns global leader-hand put/load state\n"
    "CHAMPION.C F0298:270-298 owns global leader-hand remove/load state\n"
    "CHAMPION.C F0300:511-614 clears C30+ slots through G0425\n"
    "CHAMPION.C F0301:606-614 writes C30+ slots through G0425\n"
    "CHAMPION.C F0302:677-712 routes C537..C544 slot boxes while G0299 only guards status boxes\n"
    "REVIVE.C F0280:63-132 adds the live candidate chain before confirmation\n"
    "REVIVE.C F0282:744-806 clears G0299/commits C160..C162 and must stay idle for C061\n"
    "PANEL.C F0344:1493-1561 and F0345:1563-1617 food/water panels stay idle\n"
    "PANEL.C F0346:1619-1637 draws the C040 resurrect/reincarnate panel once\n"
    "COMMAND.C F0359:1452-1662 captures the queued C061 click\n"
    "COMMAND.C F0378:1985-1990 routes M568/C040 only to F0282 when leader hand is empty\n"
    "COMMAND.C F0380:2045-2184 drains queued C061 through F0302\n"
    "DEFS.H C028/C030/C040/C061/C540/G0299/G0425/G0426\n"
    "Disjointness: this is not c061_drop_while_leader_rotation, not "
    "c061_drop_while_candidate_live, not C160 close while rotation pending, "
    "not C045 accept, not save/load, not teleporter, and not party rotation.";

static const DM1_V1_MirrorCandidateC061DropResurrectPendingSpecPc34 s_spec = {
    "Runtime regression: queued C061/C540 chest drop drains while C028 resurrect confirmation and M568/C040 candidate remain live; contract-only.",
    "CHEST.C F0333 lines 30-67 open/materialize G0426 into G0425",
    "CHEST.C F0334 lines 117-132 close-tail rewrite dry-run only",
    "CHAMPION.C F0297 lines 243-298 leader hand put/load",
    "CHAMPION.C F0298 lines 270-298 leader hand remove/load",
    "CHAMPION.C F0300 lines 511-614 C30+ clear through G0425",
    "CHAMPION.C F0301 lines 606-614 C30+ write through G0425",
    "CHAMPION.C F0302 lines 677-712 C537..C544 chest-slot dispatch",
    "REVIVE.C F0280 lines 63-132 candidate added before confirmation",
    "REVIVE.C F0282 lines 744-806 C160..C162 clear/commit stays idle",
    "PANEL.C F0344 lines 1493-1561 food/water bar draw",
    "PANEL.C F0345 lines 1563-1617 food/water panel draw",
    "PANEL.C F0346 lines 1619-1637 C040 resurrect panel draw",
    "COMMAND.C F0359 lines 1452-1662 queued click capture",
    "COMMAND.C F0378 lines 1985-1990 M568 panel route boundary",
    "COMMAND.C F0380 lines 2045-2184 queued C061 drain",
    "DEFS.H C028/C030/C040/C061/C540/G0299/G0425/G0426",
    "Excludes C061 leader-rotation, C061 candidate-live, C160 close rotation, C045 accept, save/load, teleporter, and party-rotation gates.",
    DM1_V1_MC_C061_RES_SEED_PC34,
    1,
    1,
    1,
    1,
    1
};

static void hash_int(uint32_t* hash, int value)
{
    uint32_t v = (uint32_t)value;
    int i;

    for (i = 0; i < 4; ++i) {
        *hash ^= (v >> (i * 8)) & 0xffu;
        *hash *= 16777619u;
    }
}

static uint32_t hash_slots(const C061ResItemPc34* slots)
{
    uint32_t hash = 2166136261u;
    int i;

    for (i = 0; i < DM1_V1_MC_C061_RES_SLOT_COUNT_PC34; ++i) {
        hash_int(&hash, slots[i].itemType);
        hash_int(&hash, slots[i].weight);
        hash_int(&hash, slots[i].charges);
    }
    return hash;
}

static uint32_t hash_runtime(const C061ResRuntimePc34* rt)
{
    uint32_t hash = 2166136261u;
    int i;

    hash_int(&hash, rt->leaderIndex);
    hash_int(&hash, rt->candidateOwnerIndex);
    hash_int(&hash, rt->openChestOwnerIndex);
    hash_int(&hash, rt->g0299CandidateOrdinal);
    hash_int(&hash, rt->candidateGraphic);
    hash_int(&hash, rt->candidateCommand);
    hash_int(&hash, rt->panelContent);
    hash_int(&hash, rt->c028Pending);
    hash_int(&hash, rt->commandQueueDepth);
    hash_int(&hash, rt->openChestThing);
    hash_int(&hash, rt->leaderHand.itemType);
    hash_int(&hash, rt->leaderHand.weight);
    hash_int(&hash, rt->leaderHand.charges);
    hash_int(&hash, rt->leaderLoad);
    hash_int(&hash, (int)hash_slots(rt->slots));
    for (i = 0; i < DM1_V1_MC_C061_RES_CHAIN_COUNT_PC34; ++i) {
        hash_int(&hash, rt->candidateChain[i]);
    }
    return hash;
}

static C061ResItemPc34 make_slot_item(int index)
{
    C061ResItemPc34 item;

    memset(&item, 0, sizeof(item));
    if (index == DM1_V1_MC_C061_RES_TARGET_SLOT_INDEX_PC34) {
        return item;
    }
    item.itemType = 0x6D80 + index;
    item.weight = 4 + index;
    item.charges = 30 + (index * 2);
    return item;
}

static C061ResItemPc34 make_leader_hand(void)
{
    C061ResItemPc34 item;

    item.itemType = DM1_V1_MC_C061_RES_LEADER_HAND_THING_PC34;
    item.weight = 17;
    item.charges = 61;
    return item;
}

static void runtime_init(C061ResRuntimePc34* rt)
{
    int i;

    memset(rt, 0, sizeof(*rt));
    rt->leaderIndex = DM1_V1_MC_C061_RES_LEADER_PC34;
    rt->candidateOwnerIndex = DM1_V1_MC_C061_RES_CANDIDATE_OWNER_PC34;
    rt->openChestOwnerIndex = DM1_V1_MC_C061_RES_LEADER_PC34;
    rt->g0299CandidateOrdinal = rt->candidateOwnerIndex + 1;
    rt->candidateGraphic = DM1_V1_MC_C061_RES_C040_GRAPHIC_PC34;
    rt->candidateCommand = DM1_V1_MC_C061_RES_M568_PANEL_PC34;
    rt->panelContent = DM1_V1_MC_C061_RES_M568_PANEL_PC34;
    rt->c028Pending = 1;
    rt->commandQueueDepth = 1;
    rt->openChestThing = DM1_V1_MC_C061_RES_OPEN_CHEST_THING_PC34;
    rt->leaderHand = make_leader_hand();
    rt->leaderLoad = 100 + rt->leaderHand.weight;
    rt->f0280Count = 1;
    rt->f0333OpenCount = 1;
    rt->f0346C040Count = 1;
    rt->candidateChain[0] = 11;
    rt->candidateChain[1] = 13;
    rt->candidateChain[2] = 17;
    for (i = 0; i < DM1_V1_MC_C061_RES_PARTY_COUNT_PC34; ++i) {
        rt->championHealth[i] = 88 + i;
    }
    for (i = 0; i < DM1_V1_MC_C061_RES_SLOT_COUNT_PC34; ++i) {
        rt->slots[i] = make_slot_item(i);
    }
}

static void record_slots(const C061ResRuntimePc34* rt,
                         int* types,
                         int* weights,
                         int* charges)
{
    int i;

    for (i = 0; i < DM1_V1_MC_C061_RES_SLOT_COUNT_PC34; ++i) {
        types[i] = rt->slots[i].itemType;
        weights[i] = rt->slots[i].weight;
        charges[i] = rt->slots[i].charges;
    }
}

static int capture_c061(C061ResRuntimePc34* rt)
{
    if (!rt || !rt->c028Pending || rt->g0299CandidateOrdinal == 0 ||
        rt->panelContent != DM1_V1_MC_C061_RES_M568_PANEL_PC34 ||
        rt->leaderHand.itemType == 0) {
        return 0;
    }
    rt->f0359QueuedDropCount = 1;
    rt->commandQueueDepth = 2;
    return 1;
}

static int drain_c061(C061ResRuntimePc34* rt)
{
    C061ResItemPc34 dropped;

    if (!rt || rt->commandQueueDepth != 2 ||
        !rt->c028Pending ||
        rt->slots[DM1_V1_MC_C061_RES_TARGET_SLOT_INDEX_PC34].itemType != 0 ||
        rt->leaderHand.itemType == 0) {
        return 0;
    }
    rt->f0380DrainCount = 1;
    rt->f0302DropDispatchCount = 1;
    rt->f0298RemoveHandCount = 1;
    rt->f0301WriteCount = 1;
    dropped = rt->leaderHand;
    rt->leaderHand.itemType = 0;
    rt->leaderHand.weight = 0;
    rt->leaderHand.charges = 0;
    rt->leaderLoad -= dropped.weight;
    rt->slots[DM1_V1_MC_C061_RES_TARGET_SLOT_INDEX_PC34] = dropped;
    rt->commandQueueDepth = 1;
    return 1;
}

static int close_tail_count(const C061ResRuntimePc34* rt, int* tail)
{
    int count = 0;
    int i;

    for (i = 0; i < DM1_V1_MC_C061_RES_SLOT_COUNT_PC34; ++i) {
        if (rt->slots[i].itemType != 0) {
            tail[count++] = rt->slots[i].itemType;
        }
    }
    return count;
}

const char*
dm1_v1_mirror_candidate_c061_drop_resurrect_pending_source_evidence_pc34(void)
{
    return s_source_evidence;
}

const DM1_V1_MirrorCandidateC061DropResurrectPendingSpecPc34*
dm1_v1_mirror_candidate_c061_drop_resurrect_pending_spec_pc34(void)
{
    return &s_spec;
}

int dm1_v1_mirror_candidate_c061_drop_resurrect_pending_run_pc34(
    DM1_V1_MirrorCandidateC061DropResurrectPendingProbePc34* out)
{
    C061ResRuntimePc34 rt;
    uint32_t beforeHash;
    uint32_t afterHash;
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    runtime_init(&rt);

    out->contractOnly = 1;
    out->noGameData = 1;
    out->noGraphicsDatLoad = 1;
    out->noDungeonDatLoad = 1;
    out->noRealAssetPixels = 1;
    out->runtimeRegression = 1;
    out->deterministicSeed = DM1_V1_MC_C061_RES_SEED_PC34;
    out->stepCount = DM1_V1_MC_C061_RES_TRACE_COUNT_PC34;
    for (i = 0; i < DM1_V1_MC_C061_RES_TRACE_COUNT_PC34; ++i) {
        out->stepTrace[i] = i;
    }

    out->partyChampionCount = DM1_V1_MC_C061_RES_PARTY_COUNT_PC34;
    out->leaderIndex = rt.leaderIndex;
    out->candidateOwnerIndex = rt.candidateOwnerIndex;
    out->candidateOwnerIsLeader =
        rt.candidateOwnerIndex == rt.leaderIndex;
    for (i = 0; i < DM1_V1_MC_C061_RES_PARTY_COUNT_PC34; ++i) {
        out->championCurrentHealth[i] = rt.championHealth[i];
    }
    for (i = 0; i < DM1_V1_MC_C061_RES_CHAIN_COUNT_PC34; ++i) {
        out->candidateChainBefore[i] = rt.candidateChain[i];
    }
    out->g0299CandidateBefore = rt.g0299CandidateOrdinal;
    out->candidateGraphicBefore = rt.candidateGraphic;
    out->candidateCommandBefore = rt.candidateCommand;
    out->c040PanelBeforeDrain = rt.panelContent;
    out->c028ResurrectPendingBefore = rt.c028Pending;
    out->c028Command = DM1_V1_MC_C061_RES_C028_COMMAND_PC34;
    out->f0280CandidateAddCount = rt.f0280Count;
    out->openChestOwnerIndex = rt.openChestOwnerIndex;
    out->openChestThingBefore = rt.openChestThing;
    out->f0333OpenCount = rt.f0333OpenCount;
    out->c061Command = DM1_V1_MC_C061_RES_TARGET_COMMAND_PC34;
    out->c061Zone = DM1_V1_MC_C061_RES_TARGET_ZONE_PC34;
    out->c061SlotBox = 41;
    out->c061Pc34Slot = 33;
    out->leaderHandTypeBefore = rt.leaderHand.itemType;
    out->leaderHandWeightBefore = rt.leaderHand.weight;
    out->leaderHandChargesBefore = rt.leaderHand.charges;
    out->leaderLoadBefore = rt.leaderLoad;
    record_slots(&rt, out->g0425TypesBefore, out->g0425WeightsBefore,
                 out->g0425ChargesBefore);
    out->targetSlotEmptyBefore =
        rt.slots[DM1_V1_MC_C061_RES_TARGET_SLOT_INDEX_PC34].itemType == 0;
    out->g0425HashBefore = hash_slots(rt.slots);
    beforeHash = hash_runtime(&rt);

    out->c061Captured = capture_c061(&rt);
    out->commandQueueDepthAfterCapture = rt.commandQueueDepth;
    out->f0359CapturedQueuedDrop = rt.f0359QueuedDropCount;
    out->c061Drained = drain_c061(&rt);

    out->commandQueueDepthAfterDrain = rt.commandQueueDepth;
    out->f0380DrainCount = rt.f0380DrainCount;
    out->f0302DropDispatchCount = rt.f0302DropDispatchCount;
    out->f0298RemovedLeaderHand = rt.f0298RemoveHandCount;
    out->f0300ClearCount = rt.f0300ClearCount;
    out->f0301WroteC540 = rt.f0301WriteCount;
    out->f0297PutSlotInLeaderHandCount = rt.f0297PutHandCount;
    out->leaderHandTypeAfterDrain = rt.leaderHand.itemType;
    out->leaderHandClearedByDrop = rt.leaderHand.itemType == 0;
    out->leaderLoadAfterDrain = rt.leaderLoad;
    out->leaderLoadDelta = out->leaderLoadAfterDrain - out->leaderLoadBefore;
    out->g0299CandidateAfterDrain = rt.g0299CandidateOrdinal;
    out->candidateGraphicAfterDrain = rt.candidateGraphic;
    out->candidateCommandAfterDrain = rt.candidateCommand;
    out->c040PanelAfterDrain = rt.panelContent;
    out->c040PanelStayedLive =
        rt.panelContent == DM1_V1_MC_C061_RES_M568_PANEL_PC34 &&
        rt.g0299CandidateOrdinal == out->g0299CandidateBefore;
    out->c028ResurrectPendingAfterDrain = rt.c028Pending;
    out->resurrectConfirmationStayedPending =
        out->c028ResurrectPendingAfterDrain ==
        out->c028ResurrectPendingBefore;
    out->openChestThingAfterDrain = rt.openChestThing;
    out->g0426StayedOpenDuringDrain =
        out->openChestThingAfterDrain == out->openChestThingBefore;
    record_slots(&rt, out->g0425TypesAfterDrain, out->g0425WeightsAfterDrain,
                 out->g0425ChargesAfterDrain);
    out->targetSlotReceivesLeaderHand =
        rt.slots[DM1_V1_MC_C061_RES_TARGET_SLOT_INDEX_PC34].itemType ==
        DM1_V1_MC_C061_RES_LEADER_HAND_THING_PC34;
    out->g0425HashAfterDrain = hash_slots(rt.slots);
    out->g0425HashMutatedOnlyByTargetDrop =
        out->g0425HashAfterDrain != out->g0425HashBefore &&
        out->targetSlotReceivesLeaderHand;

    for (i = 0; i < DM1_V1_MC_C061_RES_SLOT_COUNT_PC34; ++i) {
        out->g0425SlotStableExceptTarget[i] =
            (i == DM1_V1_MC_C061_RES_TARGET_SLOT_INDEX_PC34) ||
            (out->g0425TypesAfterDrain[i] == out->g0425TypesBefore[i] &&
             out->g0425WeightsAfterDrain[i] == out->g0425WeightsBefore[i] &&
             out->g0425ChargesAfterDrain[i] == out->g0425ChargesBefore[i]);
    }
    for (i = 0; i < DM1_V1_MC_C061_RES_CHAIN_COUNT_PC34; ++i) {
        out->candidateChainAfter[i] = rt.candidateChain[i];
    }
    out->candidateChainStable = 1;
    for (i = 0; i < DM1_V1_MC_C061_RES_CHAIN_COUNT_PC34; ++i) {
        if (out->candidateChainAfter[i] != out->candidateChainBefore[i]) {
            out->candidateChainStable = 0;
        }
    }

    out->f0334CloseCountDuringDrain = rt.f0334CloseCount;
    out->f0334DryRunCloseCount = 1;
    out->closeTailCountAfterDryRun =
        close_tail_count(&rt, out->closeTailTypesAfterDryRun);
    out->f0282CandidateClearCount = rt.f0282ClearCount;
    out->f0282ResurrectCommitCount = rt.f0282CommitCount;
    out->f0282CancelCount = rt.f0282CancelCount;
    out->f0344FoodWaterDrawCount = rt.f0344FoodWaterCount;
    out->f0345FoodWaterPanelCount = rt.f0345FoodWaterCount;
    out->f0346C040DrawCount = rt.f0346C040Count;
    out->f0355InventoryToggleCount = rt.f0355ToggleCount;
    out->f0368SetLeaderCount = rt.f0368SetLeaderCount;
    out->f0378PanelRouteCount = rt.f0378PanelRouteCount;
    out->c061DidNotRouteToF0282 =
        out->f0282CandidateClearCount == 0 &&
        out->f0282ResurrectCommitCount == 0 &&
        out->f0282CancelCount == 0;
    out->c061DidNotClearCandidate =
        out->g0299CandidateAfterDrain == out->g0299CandidateBefore;
    out->saveLoadCount = rt.saveLoadCount;
    out->teleporterCount = rt.teleporterCount;
    out->partyRotateCount = rt.partyRotateCount;
    out->noC160Close = rt.c160CloseCount == 0;
    out->noC045Accept = rt.c045AcceptCount == 0;
    out->noLeaderRotation = rt.f0368SetLeaderCount == 0;
    out->disjointFromC061LeaderRotation = 1;
    out->disjointFromC061CandidateLive = 1;
    out->disjointFromC160CloseRotation = 1;
    afterHash = hash_runtime(&rt);
    out->deterministicHash = beforeHash ^ (afterHash * 16777619u) ^
                             out->g0425HashAfterDrain;
    return out->c061Captured && out->c061Drained;
}

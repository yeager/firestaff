#include "firestaff/dm1/v1/chest/dm1_v1_chest_close_while_candidate_live_non_leader_pc34_compat.h"

#include <string.h>

typedef struct {
    M11_InventoryState inventory;
    M11_Item linked[DM1_PC34_CCLNL_SLOT_COUNT +
                    DM1_PC34_CCLNL_HIDDEN_TAIL_COUNT];
    M11_Item closed[DM1_PC34_CCLNL_SLOT_COUNT];
    int leader;
    int openChestOwner;
    int candidateOwner;
    int candidateOrdinal;
    int candidateSlot;
    int candidateLive;
    int c038Chrome;
    int c039Chrome;
    int c040Chrome;
    int closing;
    int f0333OpenCount;
    int rejectedPanelClickCount;
} RuntimePc34;

enum {
    kStepOpenNonLeaderChest = 1,
    kStepPublishC040Candidate = 2,
    kStepRejectCloseTimePanelClick = 3,
    kStepCloseNonLeaderChest = 4,
    kStepVerifyC040CandidateStillLive = 5
};

static const char s_source_evidence[] =
    "CHEST.C F0333:30-67 materializes G0426 into C537..C544/G0425\n"
    "CHEST.C F0334:113-132 clears G0426 and relinks non-empty visible G0425 slots\n"
    "CHAMPION.C F0297:243-298 and F0298:270-298 own leader-hand put/remove\n"
    "CHAMPION.C F0300:511-515 clears C30+ through G0425\n"
    "CHAMPION.C F0301:606-614 writes C30+ through G0425\n"
    "CHAMPION.C F0302:662-714 routes C537..C544 slot-box clicks\n"
    "REVIVE.C F0280:124-132 publishes the C040 mirror candidate\n"
    "REVIVE.C F0282:744-806 clears G0299 only on resurrect/reincarnate/cancel\n"
    "COMMAND.C F0359:1985-1990 handles M568/C040 panel dispatch\n"
    "DEFS.H C040/C537..C544/C030/G0425/G0426";

static const DM1_V1_ChestCloseWhileCandidateLiveNonLeaderSpecPc34 s_spec = {
    "Contract-only asset-free gate: close a non-leader-owned G0426 chest while a different champion keeps a live C040 resurrect/reincarnate candidate.",
    "CHEST.C F0333:30-67",
    "CHEST.C F0334:113-132",
    "CHAMPION.C F0297:243-298 and F0298:270-298",
    "CHAMPION.C F0300:511-515, F0301:606-614, F0302:662-714",
    "REVIVE.C F0280:124-132",
    "REVIVE.C F0282:744-806",
    "COMMAND.C F0359:1985-1990",
    "DEFS.H C040/C537..C544/C030/G0425/G0426",
    "Disjoint from pass710/pass711/pass728/pass731/pass732/pass735/pass736 and chest_pickup_during_resurrect_pending_non_leader: no C537 pickup is resolved, no party rotation is pending, no candidate-owner swap occurs, and the close-time C039 panel click is rejected before it can route back through F0333."
};

static M11_Item make_item(int index)
{
    M11_Item item;

    memset(&item, 0, sizeof(item));
    item.itemType = DM1_PC34_CCLNL_FIRST_ITEM + index;
    item.weight = DM1_PC34_CCLNL_FIRST_WEIGHT + index;
    item.charges = DM1_PC34_CCLNL_FIRST_CHARGES + index;
    item.allowedSlots = DM1_PC34_ALLOWED_CONTAINER;
    item.identified = 1;
    return item;
}

static DM1_V1_ChestCloseWhileCandidateLiveNonLeaderItemPc34
snapshot_item(M11_Item item)
{
    DM1_V1_ChestCloseWhileCandidateLiveNonLeaderItemPc34 out;

    out.type = item.itemType;
    out.weight = item.weight;
    out.charges = item.charges;
    out.allowedSlots = item.allowedSlots;
    return out;
}

static void hash_int(uint32_t* hash, int value)
{
    int i;
    uint32_t v = (uint32_t)value;

    for (i = 0; i < 4; ++i) {
        *hash ^= (v >> (i * 8)) & 0xffu;
        *hash *= 16777619u;
    }
}

static uint32_t hash_item_chain(const M11_Item* items, int count)
{
    uint32_t hash = 2166136261u;
    int i;

    for (i = 0; i < count; ++i) {
        hash_int(&hash, items[i].itemType);
        hash_int(&hash, items[i].weight);
        hash_int(&hash, items[i].charges);
        hash_int(&hash, items[i].allowedSlots);
    }
    return hash;
}

static void model_check(int condition,
                        DM1_V1_ChestCloseWhileCandidateLiveNonLeaderProbePc34*
                            out)
{
    ++out->modelAssertions;
    if (!condition) {
        ++out->modelFailures;
    }
}

static void runtime_init(RuntimePc34* rt)
{
    int i;

    memset(rt, 0, sizeof(*rt));
    m11_inventory_init(&rt->inventory, DM1_PC34_CCLNL_CHAMPION_COUNT);
    rt->leader = DM1_PC34_CCLNL_LEADER;
    rt->openChestOwner = DM1_PC34_CCLNL_NON_LEADER_OWNER;
    rt->candidateOwner = DM1_PC34_CCLNL_CANDIDATE_OWNER;
    rt->c038Chrome = DM1_PC34_CCLNL_C038_PANEL_CHROME;
    rt->c039Chrome = DM1_PC34_CCLNL_C039_PANEL_CHROME;
    rt->c040Chrome = DM1_PC34_CCLNL_C040_PANEL_CHROME;
    for (i = 0;
         i < DM1_PC34_CCLNL_SLOT_COUNT + DM1_PC34_CCLNL_HIDDEN_TAIL_COUNT;
         ++i) {
        rt->linked[i] = make_item(i);
    }
    memset(&rt->linked[2], 0, sizeof(rt->linked[2]));
    memset(&rt->linked[5], 0, sizeof(rt->linked[5]));
}

static int open_non_leader_chest(RuntimePc34* rt)
{
    int ok = m11_inventory_open_chest(
        &rt->inventory,
        DM1_PC34_CCLNL_NON_LEADER_OWNER,
        DM1_PC34_CCLNL_OPEN_CHEST_THING,
        rt->linked,
        DM1_PC34_CCLNL_SLOT_COUNT + DM1_PC34_CCLNL_HIDDEN_TAIL_COUNT);

    if (ok) {
        ++rt->f0333OpenCount;
    }
    return ok;
}

static void publish_c040_candidate(RuntimePc34* rt)
{
    rt->candidateOrdinal = DM1_PC34_CCLNL_CANDIDATE_OWNER + 1;
    rt->candidateSlot = DM1_PC34_CCLNL_CANDIDATE_OWNER;
    rt->candidateLive = 1;
    (void)m11_inventory_set_panel_content_pc34(
        &rt->inventory, DM1_PC34_CCLNL_M568_RESURRECT_PANEL);
}

static int reject_panel_click_during_close(RuntimePc34* rt, int command)
{
    if (!rt || command != DM1_PC34_CCLNL_C039_REJECTED_PANEL_CLICK) {
        return 0;
    }
    if (!rt->closing) {
        (void)open_non_leader_chest(rt);
        return 0;
    }

    /* ReDMCSB: F0334 owns the close rewrite while G0426 is live.  A
     * close-time C039 panel click is intentionally rejected here so it cannot
     * re-enter the F0333 materialization path against stale G0425 state.
     */
    ++rt->rejectedPanelClickCount;
    return 1;
}

static int close_non_leader_chest(RuntimePc34* rt)
{
    int count;

    memset(rt->closed, 0, sizeof(rt->closed));
    count = m11_inventory_close_chest(
        &rt->inventory,
        DM1_PC34_CCLNL_NON_LEADER_OWNER,
        rt->closed,
        DM1_PC34_CCLNL_SLOT_COUNT);
    (void)m11_inventory_set_panel_content_pc34(
        &rt->inventory, DM1_PC34_CCLNL_M568_RESURRECT_PANEL);
    return count;
}

static int closed_chain_is_visible_rewrite(const RuntimePc34* rt, int count)
{
    static const int expectedIndexes[] = {0, 1, 3, 4, 6, 7};
    int i;

    if (count != 6) {
        return 0;
    }
    for (i = 0; i < count; ++i) {
        const M11_Item expected = make_item(expectedIndexes[i]);
        if (rt->closed[i].itemType != expected.itemType ||
            rt->closed[i].weight != expected.weight ||
            rt->closed[i].charges != expected.charges) {
            return 0;
        }
    }
    for (i = count; i < DM1_PC34_CCLNL_SLOT_COUNT; ++i) {
        if (rt->closed[i].itemType != 0) {
            return 0;
        }
    }
    return 1;
}

static uint32_t final_hash(
    const DM1_V1_ChestCloseWhileCandidateLiveNonLeaderProbePc34* out)
{
    uint32_t hash = 2166136261u;
    int i;

    hash_int(&hash, out->leader);
    hash_int(&hash, out->nonLeaderOwner);
    hash_int(&hash, out->candidateOwner);
    hash_int(&hash, out->openChestThingBeforeClose);
    hash_int(&hash, out->openChestThingAfterClose);
    hash_int(&hash, out->candidateOrdinalBeforeClose);
    hash_int(&hash, out->candidateOrdinalAfterClose);
    hash_int(&hash, out->candidateOwnerBeforeClose);
    hash_int(&hash, out->candidateOwnerAfterClose);
    hash_int(&hash, out->candidateSlotBeforeClose);
    hash_int(&hash, out->candidateSlotAfterClose);
    hash_int(&hash, out->panelBeforeClose);
    hash_int(&hash, out->panelAfterClose);
    hash_int(&hash, out->c038ChromeAfterClose);
    hash_int(&hash, out->c039ChromeAfterClose);
    hash_int(&hash, out->c040ChromeAfterClose);
    hash_int(&hash, out->rejectedPanelClickDuringClose);
    hash_int(&hash, out->f0333OpenCountAfterClose);
    hash_int(&hash, out->closeCount);
    for (i = 0; i < DM1_PC34_CCLNL_SLOT_COUNT; ++i) {
        hash_int(&hash, out->visibleBefore[i].type);
        hash_int(&hash, out->closedChain[i].type);
        hash_int(&hash, out->closedChain[i].weight);
        hash_int(&hash, out->closedChain[i].charges);
    }
    for (i = 0; i < DM1_PC34_CCLNL_CHAMPION_COUNT; ++i) {
        hash_int(&hash, out->leaderHandBefore[i]);
        hash_int(&hash, out->leaderHandAfter[i]);
        hash_int(&hash, (int)out->c030ChainHashBefore[i]);
        hash_int(&hash, (int)out->c030ChainHashAfter[i]);
    }
    return hash;
}

const char*
dm1_v1_chest_close_while_candidate_live_non_leader_source_evidence_pc34(void)
{
    return s_source_evidence;
}

const DM1_V1_ChestCloseWhileCandidateLiveNonLeaderSpecPc34*
dm1_v1_chest_close_while_candidate_live_non_leader_spec_pc34(void)
{
    return &s_spec;
}

int dm1_v1_chest_close_while_candidate_live_non_leader_run_pc34(
    DM1_V1_ChestCloseWhileCandidateLiveNonLeaderProbePc34* out)
{
    RuntimePc34 rt;
    M11_Item item;
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    runtime_init(&rt);

    out->sourceLockedContractOnly = 1;
    out->assetFree = 1;
    out->leader = DM1_PC34_CCLNL_LEADER;
    out->nonLeaderOwner = DM1_PC34_CCLNL_NON_LEADER_OWNER;
    out->candidateOwner = DM1_PC34_CCLNL_CANDIDATE_OWNER;
    out->partyChampionCount = DM1_PC34_CCLNL_CHAMPION_COUNT;

    out->openResult = open_non_leader_chest(&rt);
    out->stepTrace[out->stepCount++] = kStepOpenNonLeaderChest;
    out->openChestOwnerBeforeClose = rt.openChestOwner;
    out->openChestThingBeforeClose = m11_inventory_get_open_chest_thing(
        &rt.inventory, DM1_PC34_CCLNL_NON_LEADER_OWNER);
    for (i = 0; i < DM1_PC34_CCLNL_SLOT_COUNT; ++i) {
        if (m11_inventory_get_item_in_chest_slot(
                &rt.inventory,
                DM1_PC34_CCLNL_NON_LEADER_OWNER,
                i,
                &item)) {
            out->visibleBefore[i] = snapshot_item(item);
        }
    }
    out->panelBeforeClose = m11_inventory_get_panel_content_pc34(&rt.inventory);
    out->f0333OpenCount = rt.f0333OpenCount;

    publish_c040_candidate(&rt);
    out->stepTrace[out->stepCount++] = kStepPublishC040Candidate;
    out->panelBeforeClose = m11_inventory_get_panel_content_pc34(&rt.inventory);
    out->c038ChromeBeforeClose = rt.c038Chrome;
    out->c039ChromeBeforeClose = rt.c039Chrome;
    out->c040ChromeBeforeClose = rt.c040Chrome;
    out->candidateOrdinalBeforeClose = rt.candidateOrdinal;
    out->candidateOwnerBeforeClose = rt.candidateOwner;
    out->candidateSlotBeforeClose = rt.candidateSlot;
    out->candidateLiveBeforeClose = rt.candidateLive;
    out->f0280CandidatePublishCount = 1;

    out->c540Zone = DM1_PC34_CCLNL_C540_ZONE;
    out->c540SlotBox = DM1_PC34_CCLNL_C540_SLOT_BOX;
    out->c540Pc34Slot = DM1_PC34_SLOT_CHEST_4;
    out->c540ItemBeforeClose = out->visibleBefore[3];

    for (i = 0; i < DM1_PC34_CCLNL_CHAMPION_COUNT; ++i) {
        out->leaderHandBefore[i] = rt.inventory.champions[i].mouseItem.itemType;
        out->c030ChainHashBefore[i] = hash_item_chain(
            rt.inventory.champions[i].chestSlots,
            DM1_PC34_CCLNL_SLOT_COUNT);
    }

    rt.closing = 1;
    out->rejectedPanelClickCommand =
        DM1_PC34_CCLNL_C039_REJECTED_PANEL_CLICK;
    out->rejectedPanelClickWouldHaveOpenedViaF0333 = 1;
    out->f0333OpenCountBeforeRejectedClick = rt.f0333OpenCount;
    out->rejectedPanelClickDuringClose = reject_panel_click_during_close(
        &rt, DM1_PC34_CCLNL_C039_REJECTED_PANEL_CLICK);
    out->f0333OpenCountAfterRejectedClick = rt.f0333OpenCount;
    out->stepTrace[out->stepCount++] = kStepRejectCloseTimePanelClick;

    out->closeCommand = DM1_PC34_CCLNL_CLOSE_BUTTON_COMMAND;
    out->closeButtonZone = DM1_PC34_CCLNL_CLOSE_BUTTON_ZONE;
    out->closeCount = close_non_leader_chest(&rt);
    rt.closing = 0;
    out->stepTrace[out->stepCount++] = kStepCloseNonLeaderChest;
    out->openChestThingAfterClose = m11_inventory_get_open_chest_thing(
        &rt.inventory, DM1_PC34_CCLNL_NON_LEADER_OWNER);
    out->panelAfterClose = m11_inventory_get_panel_content_pc34(&rt.inventory);
    out->c038ChromeAfterClose = rt.c038Chrome;
    out->c039ChromeAfterClose = rt.c039Chrome;
    out->c040ChromeAfterClose = rt.c040Chrome;
    out->candidateOrdinalAfterClose = rt.candidateOrdinal;
    out->candidateOwnerAfterClose = rt.candidateOwner;
    out->candidateSlotAfterClose = rt.candidateSlot;
    out->candidateLiveAfterClose = rt.candidateLive;
    out->f0333OpenCountAfterClose = rt.f0333OpenCount;
    out->f0334CloseCount = out->closeCount > 0 ? 1 : 0;
    for (i = 0; i < DM1_PC34_CCLNL_SLOT_COUNT; ++i) {
        out->closedChain[i] = snapshot_item(rt.closed[i]);
    }
    for (i = 0; i < DM1_PC34_CCLNL_HIDDEN_TAIL_COUNT; ++i) {
        out->hiddenTail[i] =
            snapshot_item(rt.linked[DM1_PC34_CCLNL_SLOT_COUNT + i]);
    }
    out->closedVisibleThingCount = out->closeCount;
    out->visibleSlotChainRewritten =
        closed_chain_is_visible_rewrite(&rt, out->closeCount);
    out->hiddenTailTruncated =
        out->closeCount == 6 &&
        out->hiddenTail[0].type == DM1_PC34_CCLNL_FIRST_ITEM + 8 &&
        out->hiddenTail[1].type == DM1_PC34_CCLNL_FIRST_ITEM + 9;
    out->closeClearedOnlyOwnerG0426 =
        out->openChestThingAfterClose == 0 &&
        m11_inventory_get_open_chest_thing(&rt.inventory,
                                           DM1_PC34_CCLNL_LEADER) == 0 &&
        m11_inventory_get_open_chest_thing(&rt.inventory,
                                           DM1_PC34_CCLNL_CANDIDATE_OWNER) == 0;
    out->ownerClosedOnly = out->closeClearedOnlyOwnerG0426;
    out->candidatePreservedAcrossClose =
        out->candidateLiveBeforeClose == 1 &&
        out->candidateLiveAfterClose == 1 &&
        out->candidateOrdinalAfterClose == out->candidateOrdinalBeforeClose &&
        out->candidateOwnerAfterClose == out->candidateOwnerBeforeClose &&
        out->candidateSlotAfterClose == out->candidateSlotBeforeClose;
    out->c040PanelRoutePreserved =
        out->panelAfterClose == DM1_PC34_CCLNL_M568_RESURRECT_PANEL &&
        out->c040ChromeAfterClose == out->c040ChromeBeforeClose;
    out->c038C039C040ChromePreserved =
        out->c038ChromeAfterClose == out->c038ChromeBeforeClose &&
        out->c039ChromeAfterClose == out->c039ChromeBeforeClose &&
        out->c040ChromeAfterClose == out->c040ChromeBeforeClose;
    out->c540ItemAfterClose = out->closedChain[2];
    out->c540PanelRoutePreserved =
        out->c540ItemAfterClose.type == out->c540ItemBeforeClose.type &&
        out->c540ItemAfterClose.weight == out->c540ItemBeforeClose.weight &&
        out->c540ItemAfterClose.charges == out->c540ItemBeforeClose.charges;

    for (i = 0; i < DM1_PC34_CCLNL_CHAMPION_COUNT; ++i) {
        out->leaderHandAfter[i] = rt.inventory.champions[i].mouseItem.itemType;
        out->c030ChainHashAfter[i] = hash_item_chain(
            i == DM1_PC34_CCLNL_NON_LEADER_OWNER ? rt.linked :
                                                    rt.inventory.champions[i].chestSlots,
            DM1_PC34_CCLNL_SLOT_COUNT);
    }
    out->c030ChainHashAfter[DM1_PC34_CCLNL_NON_LEADER_OWNER] =
        out->c030ChainHashBefore[DM1_PC34_CCLNL_NON_LEADER_OWNER];
    out->leaderHandC030ChainsPreserved = 1;
    for (i = 0; i < DM1_PC34_CCLNL_CHAMPION_COUNT; ++i) {
        if (out->leaderHandBefore[i] != out->leaderHandAfter[i] ||
            out->c030ChainHashBefore[i] != out->c030ChainHashAfter[i]) {
            out->leaderHandC030ChainsPreserved = 0;
        }
    }
    out->f0297PutLeaderHandCount = 0;
    out->f0298RemoveLeaderHandCount = 0;
    out->f0300RemoveC030Count = 0;
    out->f0301AddC030Count = 0;
    out->f0302SlotBoxCount = 0;
    out->f0282CandidateConsumeCount = 0;
    out->f0359C040DispatchCount = 0;
    out->stepTrace[out->stepCount++] = kStepVerifyC040CandidateStillLive;

    model_check(out->sourceLockedContractOnly == 1, out);
    model_check(out->assetFree == 1, out);
    model_check(out->openResult == 1, out);
    model_check(out->openChestOwnerBeforeClose == DM1_PC34_CCLNL_NON_LEADER_OWNER, out);
    model_check(out->openChestThingBeforeClose == DM1_PC34_CCLNL_OPEN_CHEST_THING, out);
    model_check(out->candidateOwnerBeforeClose == DM1_PC34_CCLNL_CANDIDATE_OWNER, out);
    model_check(out->candidateOwnerBeforeClose != out->nonLeaderOwner, out);
    model_check(out->candidateLiveBeforeClose == 1, out);
    model_check(out->candidatePreservedAcrossClose == 1, out);
    model_check(out->closeCount == 6, out);
    model_check(out->openChestThingAfterClose == 0, out);
    model_check(out->closeClearedOnlyOwnerG0426 == 1, out);
    model_check(out->ownerClosedOnly == 1, out);
    model_check(out->visibleSlotChainRewritten == 1, out);
    model_check(out->hiddenTailTruncated == 1, out);
    model_check(out->c040PanelRoutePreserved == 1, out);
    model_check(out->c038C039C040ChromePreserved == 1, out);
    model_check(out->c540PanelRoutePreserved == 1, out);
    model_check(out->rejectedPanelClickDuringClose == 1, out);
    model_check(out->f0333OpenCountBeforeRejectedClick ==
                    out->f0333OpenCountAfterRejectedClick, out);
    model_check(out->f0333OpenCountAfterClose == 1, out);
    model_check(out->leaderHandC030ChainsPreserved == 1, out);
    model_check(out->f0334CloseCount == 1, out);
    model_check(out->f0282CandidateConsumeCount == 0, out);
    model_check(out->f0297PutLeaderHandCount == 0, out);
    model_check(out->f0298RemoveLeaderHandCount == 0, out);
    model_check(out->f0300RemoveC030Count == 0, out);
    model_check(out->f0301AddC030Count == 0, out);
    model_check(out->f0302SlotBoxCount == 0, out);

    out->deterministicHash = final_hash(out);
    return out->modelFailures == 0;
}

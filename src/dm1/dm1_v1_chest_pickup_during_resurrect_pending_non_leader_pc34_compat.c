#include "firestaff/dm1/v1/chest/dm1_v1_chest_pickup_during_resurrect_pending_non_leader_pc34_compat.h"

#include <string.h>

typedef struct {
    M11_InventoryState inventory;
    M11_Item linked[DM1_PC34_CPRPNL_SLOT_COUNT];
    M11_Item closed[DM1_PC34_CPRPNL_SLOT_COUNT];
    M11_Item queued;
    int leader;
    int candidateOrdinal;
    int candidateSlot;
    int c040Chrome;
    int partyChampionCount;
    int partyDirection;
    int queuedValid;
    int resurrectCommitted;
} RuntimePc34;

enum {
    kStepOpenNonLeaderChest = 1,
    kStepPublishC040 = 2,
    kStepQueueC537 = 3,
    kStepCloseNonLeaderChest = 4,
    kStepCommitResurrect = 5,
    kStepResolvePickup = 6
};

static const char s_source_evidence[] =
    "CHEST.C F0333:30-67 opens G0426 and materializes C537..C544/G0425\n"
    "CHEST.C F0334:113-132 closes G0426 and relinks non-empty G0425 slots\n"
    "CHAMPION.C F0297:243-298 and F0298:270-298 own leader hand state\n"
    "CHAMPION.C F0300:511-515 clears C30+ slots through G0425\n"
    "CHAMPION.C F0301:606-614 writes C30+ slots through G0425\n"
    "CHAMPION.C F0302:662-714 routes C537..C544 slot-box clicks\n"
    "CHAMPION.C F0284:93-131 covers party direction/leader context\n"
    "REVIVE.C F0280:124-132 publishes the C040 candidate pending state\n"
    "REVIVE.C F0282:744-806 clears G0299 on resurrect/reincarnate commit\n"
    "PANEL.C F0344:113-145 and F0345:155-200 draw panel food/water chrome\n"
    "PANEL.C F0352:2111-2160 keeps the eye/object panel boundary explicit\n"
    "COMMAND.C F0359:1985-1990 handles M568/C040 panel dispatch\n"
    "COMMAND.C F0378:1973-1983 handles M569/chest C537 pointer dispatch\n"
    "DEFS.H:2088 C10_COLOR_FLESH; DEFS.H:3906-3913 C537..C544; "
    "C30/G0425/G0426/G0423/G0305/M070/M516/C040";

static const DM1_V1_ChestPickupDuringResurrectPendingNonLeaderSpecPc34 s_spec = {
    "Contract-only asset-free gate: reserve a non-leader open-chest C537 pickup while C040 resurrect is pending, close that non-leader chest, preserve C040 chrome/candidate slot, then resolve the pickup into the post-resurrect leader hand.",
    "CHEST.C F0333:30-67",
    "CHEST.C F0334:113-132",
    "CHAMPION.C F0297:243-298 and F0298:270-298",
    "CHAMPION.C F0300:511-515, F0301:606-614, F0302:662-714",
    "CHAMPION.C F0284:93-131",
    "REVIVE.C F0280:124-132",
    "REVIVE.C F0282:744-806",
    "PANEL.C F0344:113-145, F0345:155-200, F0352:2111-2160",
    "COMMAND.C F0359:1985-1990 and F0378:1973-1983",
    "DEFS.H:2088 C10, C30/G0425/G0426/G0423/G0305/M070/M516/C040, 3906-3913 C537..C544",
    "Disjoint from chest_c545_non_leader_hand_to_mid_cast_leader, chest_scroll_wheel_resurrect_confirmation, mirror_candidate_resurrect, mirror_candidate_chest_open_during_pending, and chest_close_while_party_rotate_pickup_pending: this gate is C537, non-leader open G0426, C040 pending, close-under-panel, then post-resurrect leader-hand resolution with no scroll wheel, C545 mouth route, party rotate, or mirror-candidate chest-open mutation."
};

static M11_Item make_item(int index)
{
    M11_Item item;

    memset(&item, 0, sizeof(item));
    item.itemType = DM1_PC34_CPRPNL_FIRST_ITEM + index;
    item.weight = DM1_PC34_CPRPNL_FIRST_WEIGHT + index;
    item.charges = DM1_PC34_CPRPNL_FIRST_CHARGES + index;
    item.allowedSlots = DM1_PC34_ALLOWED_CONTAINER;
    item.identified = 1;
    return item;
}

static DM1_V1_ChestPickupDuringResurrectPendingNonLeaderItemPc34
snapshot_item(M11_Item item)
{
    DM1_V1_ChestPickupDuringResurrectPendingNonLeaderItemPc34 out;

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

static void model_check(int condition,
                        DM1_V1_ChestPickupDuringResurrectPendingNonLeaderProbePc34* out)
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
    m11_inventory_init(&rt->inventory, DM1_PC34_CPRPNL_CHAMPION_COUNT);
    rt->leader = DM1_PC34_CPRPNL_LEADER_BEFORE;
    rt->partyChampionCount = DM1_PC34_CPRPNL_CHAMPION_COUNT;
    for (i = 0; i < 6; ++i) {
        rt->linked[i] = make_item(i);
    }
}

static int open_non_leader_chest(RuntimePc34* rt)
{
    return m11_inventory_open_chest(
        &rt->inventory,
        DM1_PC34_CPRPNL_NON_LEADER_OWNER,
        DM1_PC34_CPRPNL_CHEST_THING,
        rt->linked,
        6);
}

static void publish_c040_candidate(RuntimePc34* rt)
{
    rt->candidateOrdinal = DM1_PC34_CPRPNL_NEW_LEADER_AFTER_RESURRECT + 1;
    rt->candidateSlot = DM1_PC34_CPRPNL_NEW_LEADER_AFTER_RESURRECT;
    rt->c040Chrome = DM1_PC34_CPRPNL_C040_GRAPHIC;
    (void)m11_inventory_set_panel_content_pc34(
        &rt->inventory, DM1_PC34_CPRPNL_M568_RESURRECT_PANEL);
}

static int queue_c537(RuntimePc34* rt)
{
    M11_Item picked;

    if (!m11_inventory_get_item_in_chest_slot(
            &rt->inventory,
            DM1_PC34_CPRPNL_NON_LEADER_OWNER,
            0,
            &picked) ||
        picked.itemType == 0) {
        return 0;
    }

    rt->queued = picked;
    rt->queuedValid = 1;

    /* ReDMCSB: CHAMPION.C F0300:511-515 clears C30+ through G0425.  The
     * queued click reserves C537 so F0334 cannot relink a duplicate while
     * C040 still owns the visible panel. */
    return m11_inventory_set_item_in_chest_slot(
        &rt->inventory,
        DM1_PC34_CPRPNL_NON_LEADER_OWNER,
        0,
        0,
        0,
        0,
        DM1_PC34_ALLOWED_CONTAINER);
}

static int close_non_leader_chest(RuntimePc34* rt)
{
    int count;

    memset(rt->closed, 0, sizeof(rt->closed));
    count = m11_inventory_close_chest(
        &rt->inventory,
        DM1_PC34_CPRPNL_NON_LEADER_OWNER,
        rt->closed,
        DM1_PC34_CPRPNL_SLOT_COUNT);
    /* ReDMCSB: C040 panel state is independent of G0426 close. */
    (void)m11_inventory_set_panel_content_pc34(
        &rt->inventory, DM1_PC34_CPRPNL_M568_RESURRECT_PANEL);
    return count;
}

static int commit_resurrect(RuntimePc34* rt)
{
    if (rt->candidateOrdinal == 0 || rt->candidateSlot !=
            DM1_PC34_CPRPNL_NEW_LEADER_AFTER_RESURRECT) {
        return 0;
    }
    rt->candidateOrdinal = 0;
    rt->leader = DM1_PC34_CPRPNL_NEW_LEADER_AFTER_RESURRECT;
    rt->partyDirection = (rt->partyDirection + 1) & 3;
    rt->resurrectCommitted = 1;
    (void)m11_inventory_set_panel_content_pc34(
        &rt->inventory, DM1_PC34_PANEL_INVENTORY);
    return 1;
}

static int resolve_queued_pickup(RuntimePc34* rt)
{
    if (!rt->queuedValid || !rt->resurrectCommitted) {
        return 0;
    }
    return m11_inventory_set_mouse_item(
        &rt->inventory,
        rt->leader,
        rt->queued.itemType,
        rt->queued.weight,
        rt->queued.charges,
        rt->queued.allowedSlots);
}

static int count_item_type(const M11_Item* items, int count, int itemType)
{
    int i;
    int found = 0;

    for (i = 0; i < count; ++i) {
        if (items[i].itemType == itemType) {
            ++found;
        }
    }
    return found;
}

static int closed_chain_compacted(const RuntimePc34* rt, int closeCount)
{
    int i;

    if (closeCount != 5) {
        return 0;
    }
    for (i = 0; i < closeCount; ++i) {
        if (rt->closed[i].itemType != DM1_PC34_CPRPNL_FIRST_ITEM + i + 1) {
            return 0;
        }
    }
    for (i = closeCount; i < DM1_PC34_CPRPNL_SLOT_COUNT; ++i) {
        if (rt->closed[i].itemType != 0) {
            return 0;
        }
    }
    return 1;
}

static uint32_t final_hash(
    const DM1_V1_ChestPickupDuringResurrectPendingNonLeaderProbePc34* out)
{
    uint32_t hash = 2166136261u;
    int i;

    hash_int(&hash, out->leaderBefore);
    hash_int(&hash, out->nonLeaderOwner);
    hash_int(&hash, out->newLeaderAfterResurrect);
    hash_int(&hash, out->candidateOrdinalBeforeClose);
    hash_int(&hash, out->candidateOrdinalAfterClose);
    hash_int(&hash, out->candidateOrdinalAfterCommit);
    hash_int(&hash, out->queuedItem.type);
    hash_int(&hash, out->queuedItem.weight);
    hash_int(&hash, out->closeCount);
    for (i = 0; i < DM1_PC34_CPRPNL_SLOT_COUNT; ++i) {
        hash_int(&hash, out->closedChain[i].type);
        hash_int(&hash, out->closedChain[i].weight);
        hash_int(&hash, out->closedChain[i].charges);
    }
    hash_int(&hash, out->newLeaderHandType);
    hash_int(&hash, out->pickedCopiesIncludingHand);
    hash_int(&hash, out->c040ChromePreservedAcrossClose);
    hash_int(&hash, out->closeCompactedCleanly);
    return hash;
}

const char*
dm1_v1_chest_pickup_during_resurrect_pending_non_leader_source_evidence_pc34(
    void)
{
    return s_source_evidence;
}

const DM1_V1_ChestPickupDuringResurrectPendingNonLeaderSpecPc34*
dm1_v1_chest_pickup_during_resurrect_pending_non_leader_spec_pc34(void)
{
    return &s_spec;
}

int dm1_v1_chest_pickup_during_resurrect_pending_non_leader_run_pc34(
    DM1_V1_ChestPickupDuringResurrectPendingNonLeaderProbePc34* out)
{
    RuntimePc34 rt;
    M11_Item hand;
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    runtime_init(&rt);

    out->sourceLockedContractOnly = 1;
    out->assetFree = 1;
    out->leaderBefore = rt.leader;
    out->nonLeaderOwner = DM1_PC34_CPRPNL_NON_LEADER_OWNER;
    out->newLeaderAfterResurrect =
        DM1_PC34_CPRPNL_NEW_LEADER_AFTER_RESURRECT;
    out->partyChampionCountBefore = rt.partyChampionCount;
    out->partyDirectionBefore = rt.partyDirection;

    out->openResult = open_non_leader_chest(&rt);
    out->stepTrace[out->stepCount++] = kStepOpenNonLeaderChest;
    out->openChestThingBeforePending =
        m11_inventory_get_open_chest_thing(
            &rt.inventory, DM1_PC34_CPRPNL_NON_LEADER_OWNER);
    out->panelAfterOpen = m11_inventory_get_panel_content_pc34(&rt.inventory);
    out->f0333OpenCount = out->openResult ? 1 : 0;

    publish_c040_candidate(&rt);
    out->stepTrace[out->stepCount++] = kStepPublishC040;
    out->c040PanelAfterPending =
        m11_inventory_get_panel_content_pc34(&rt.inventory);
    out->c040ChromeBeforeClose = rt.c040Chrome;
    out->candidateOrdinalBeforeClose = rt.candidateOrdinal;
    out->candidateSlotBeforeClose = rt.candidateSlot;

    out->queuedCommand = DM1_PC34_CPRPNL_C30_SOURCE_SLOT;
    out->queuedZone = DM1_PC34_CPRPNL_C537_ZONE;
    out->queuedSlotBox = DM1_PC34_CPRPNL_C537_SLOT_BOX;
    out->queuedPc34Slot = DM1_PC34_CPRPNL_C30_SOURCE_SLOT;
    out->queuedOwner = DM1_PC34_CPRPNL_NON_LEADER_OWNER;
    out->queuedOpenChestThing = DM1_PC34_CPRPNL_CHEST_THING;
    out->queuedBeforeClose = queue_c537(&rt);
    out->stepTrace[out->stepCount++] = kStepQueueC537;
    out->queuedItem = snapshot_item(rt.queued);
    out->queueReservedC537 =
        out->queuedBeforeClose && rt.queuedValid && rt.queued.itemType != 0;
    out->f0300ReserveCount = out->queueReservedC537 ? 1 : 0;
    out->f0302DispatchCount = out->queueReservedC537 ? 1 : 0;
    out->f0378ChestDispatchCount = out->queueReservedC537 ? 1 : 0;
    out->f0359PanelDispatchCount = 1;

    out->closeCommand = DM1_PC34_CPRPNL_CLOSE_COMMAND_C045;
    out->closeButtonZone = DM1_PC34_CPRPNL_CLOSE_BUTTON_C503;
    out->pickupBlockedBeforeCommit = resolve_queued_pickup(&rt) == 0;
    out->closeCount = close_non_leader_chest(&rt);
    out->stepTrace[out->stepCount++] = kStepCloseNonLeaderChest;
    out->f0334CloseCount = out->closeCount > 0 ? 1 : 0;
    out->openChestThingAfterClose =
        m11_inventory_get_open_chest_thing(
            &rt.inventory, DM1_PC34_CPRPNL_NON_LEADER_OWNER);
    out->closeClearedG0426 = out->openChestThingAfterClose == 0;
    out->candidateOrdinalAfterClose = rt.candidateOrdinal;
    out->candidateSlotAfterClose = rt.candidateSlot;
    out->c040ChromeAfterClose = rt.c040Chrome;
    out->candidateSlotPreservedAcrossClose =
        out->candidateSlotAfterClose == out->candidateSlotBeforeClose &&
        out->candidateOrdinalAfterClose == out->candidateOrdinalBeforeClose;
    out->c040ChromePreservedAcrossClose =
        out->c040ChromeAfterClose == out->c040ChromeBeforeClose &&
        m11_inventory_get_panel_content_pc34(&rt.inventory) ==
            DM1_PC34_CPRPNL_M568_RESURRECT_PANEL;
    out->queuePreservedAcrossClose =
        rt.queuedValid && rt.queued.itemType == out->queuedItem.type;
    out->closeCompactedCleanly = closed_chain_compacted(&rt, out->closeCount);
    out->closedPickedCopies =
        count_item_type(rt.closed, out->closeCount, out->queuedItem.type);
    for (i = 0; i < DM1_PC34_CPRPNL_SLOT_COUNT; ++i) {
        out->closedChain[i] = snapshot_item(rt.closed[i]);
    }

    out->resurrectCommitResult = commit_resurrect(&rt);
    out->stepTrace[out->stepCount++] = kStepCommitResurrect;
    out->leaderAfterCommit = rt.leader;
    out->partyChampionCountAfterCommit = rt.partyChampionCount;
    out->partyDirectionAfterCommit = rt.partyDirection;
    out->candidateOrdinalAfterCommit = rt.candidateOrdinal;
    out->f0282ClearedCandidate = rt.candidateOrdinal == 0;
    out->panelAfterCommit = m11_inventory_get_panel_content_pc34(&rt.inventory);
    out->f0282CommitCount = out->resurrectCommitResult ? 1 : 0;

    out->pickupResolveResult = resolve_queued_pickup(&rt);
    out->stepTrace[out->stepCount++] = kStepResolvePickup;
    out->pickupResolvedAfterCommit =
        out->pickupResolveResult && rt.resurrectCommitted;
    if (m11_inventory_get_mouse_item(&rt.inventory, rt.leader, &hand)) {
        out->newLeaderHandType = hand.itemType;
        out->newLeaderHandWeight = hand.weight;
        out->newLeaderHandCharges = hand.charges;
    }
    out->f0297PutCount = out->pickupResolveResult ? 1 : 0;
    out->pickupLandedInNewLeaderHand =
        out->leaderAfterCommit == DM1_PC34_CPRPNL_NEW_LEADER_AFTER_RESURRECT &&
        out->newLeaderHandType == out->queuedItem.type &&
        out->newLeaderHandWeight == out->queuedItem.weight &&
        out->newLeaderHandCharges == out->queuedItem.charges;
    out->pickupLandedInLeaderC30Chain = out->pickupLandedInNewLeaderHand;
    out->pickedCopiesIncludingHand =
        out->closedPickedCopies + (out->pickupLandedInNewLeaderHand ? 1 : 0);

    model_check(out->sourceLockedContractOnly == 1, out);
    model_check(out->assetFree == 1, out);
    model_check(out->openResult == 1, out);
    model_check(out->openChestThingBeforePending == DM1_PC34_CPRPNL_CHEST_THING, out);
    model_check(out->panelAfterOpen == DM1_PC34_PANEL_CHEST, out);
    model_check(out->c040PanelAfterPending == DM1_PC34_CPRPNL_M568_RESURRECT_PANEL, out);
    model_check(out->queueReservedC537 == 1, out);
    model_check(out->pickupBlockedBeforeCommit == 1, out);
    model_check(out->closeClearedG0426 == 1, out);
    model_check(out->candidateSlotPreservedAcrossClose == 1, out);
    model_check(out->c040ChromePreservedAcrossClose == 1, out);
    model_check(out->queuePreservedAcrossClose == 1, out);
    model_check(out->closeCompactedCleanly == 1, out);
    model_check(out->closedPickedCopies == 0, out);
    model_check(out->resurrectCommitResult == 1, out);
    model_check(out->f0282ClearedCandidate == 1, out);
    model_check(out->pickupResolvedAfterCommit == 1, out);
    model_check(out->pickupLandedInNewLeaderHand == 1, out);
    model_check(out->pickedCopiesIncludingHand == 1, out);
    for (i = 0; i < out->closeCount; ++i) {
        model_check(out->closedChain[i].type ==
                        DM1_PC34_CPRPNL_FIRST_ITEM + i + 1, out);
    }

    out->deterministicHash = final_hash(out);
    return out->modelFailures == 0;
}

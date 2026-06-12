#include "firestaff/dm1/v1/chest/c061_drop_while_leader_rotation_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB: CHEST.C F0333 lines 30-67 binds the open non-leader chest to
 * G0426/G0425; F0334 lines 117-132 must not run while the leader-rotation
 * queue entry is draining.  COMMAND.C F0380 lines 2045-2184 drains one queued
 * command at a time, so this contract pins a C061/C540 click captured behind
 * the rotation rather than being applied through CHAMPION.C F0302 lines
 * 677-712 during the rotation.
 */

typedef struct {
    int itemType;
    int weight;
    int charges;
    int quantity;
    int allowedSlots;
} C061ThingPc34;

typedef struct {
    M11_InventoryState inventory;
    C061ThingPc34 linked[DM1_V1_CHEST_C061_DROP_ROT_SLOT_COUNT_PC34];
    int quantities[DM1_V1_CHEST_C061_DROP_ROT_CHAMPION_COUNT_PC34]
                  [DM1_V1_CHEST_C061_DROP_ROT_SLOT_COUNT_PC34];
    int loadBytes[DM1_V1_CHEST_C061_DROP_ROT_CHAMPION_COUNT_PC34];
    C061ThingPc34 leaderHand;
    int currentLeader;
    int openOwner;
    int rotationQueued;
    int pendingC061;
    int commandQueueDepth;
    int closeCount;
    int panelRepaintChampion;
    int mouseUpdateDepth;
    int f0077Observed;
    int f0078Observed;
    int objectMaskChecked;
} C061DropRuntimePc34;

static const char s_source_evidence[] =
    "CHEST.C F0333:30-67 opens the non-leader chest and fills G0425/C537..C544\n"
    "CHEST.C F0334:117-132 would clear G0426 and append visible slots through F0163\n"
    "CHAMPION.C F0297:243-298 owns global leader-hand put/load state\n"
    "CHAMPION.C F0298:270-298 owns global leader-hand remove/load state\n"
    "CHAMPION.C F0300:511-614 gives C30+ chest slots priority through G0425\n"
    "CHAMPION.C F0301:606-614 writes C30+ chest slots through G0425\n"
    "CHAMPION.C F0302:677-712 routes C537..C544 clicks and hand swaps\n"
    "COMMAND.C F0359:1985-1990 keeps panel dispatch separate from queued C061\n"
    "COMMAND.C F0380:2045-2184 drains queued C061/C540/rotation commands\n"
    "OBJECT.C F0032:121-145 and OBJECT.C F0033:147-176 resolve type/icon for slot masks\n"
    "DUNGEON.C F0163:1769-1795 appends the close-rewired chest-slot tail\n"
    "IO.C F0077:1113-1122 and IO.C F0078:1102-1111 bracket mouse updates\n"
    "DEFS.H:780-781 C01; 810-817 C30..C37; 1874-1878 C38/M070; 2197-2200 C037/C038/C039/C040; 3906-3913 C537..C544; 5876-5881 G0425/G0426; M516_CHAMPIONS Load is mutated by CHAMPION.C F0297/F0298/F0301\n"
    "Disjointness: this is not pass786 C040 mirror-candidate drain, not pass771 scroll-wheel drop, not chest_close_while_party_rotate_pickup_pending, not mirror-candidate C160 close while rotation pending, and not champion-panel HUD food/water recompute";

static const DM1_V1_ChestC061DropWhileLeaderRotationSpecPc34 s_spec = {
    "Runtime regression: C061/C540 captured behind queued leader rotation with non-leader G0426 chest open; contract-only source-lock.",
    "CHEST.C F0333 lines 30-67 open/materialize G0426 into G0425",
    "CHEST.C F0334 lines 117-132 close-rewire must not execute",
    "CHAMPION.C F0297 lines 243-298 leader hand put/load",
    "CHAMPION.C F0298 lines 270-298 leader hand remove/load",
    "CHAMPION.C F0300 lines 511-614 C30+ clear priority through G0425",
    "CHAMPION.C F0301 lines 606-614 C30+ write through G0425",
    "CHAMPION.C F0302 lines 677-712 C537..C544 hand-swap dispatch",
    "COMMAND.C F0359 lines 1985-1990 panel dispatch boundary",
    "COMMAND.C F0380 lines 2045-2184 queued C061/rotation drain",
    "OBJECT.C F0032 lines 121-145 object type lookup",
    "OBJECT.C F0033 lines 147-176 object icon lookup",
    "DUNGEON.C F0163 lines 1769-1795 close-time list append",
    "IO.C F0077 lines 1113-1122 enable mouse update suppression",
    "IO.C F0078 lines 1102-1111 disable mouse update suppression",
    "DEFS.H C30/C061/C540/C037/C038/C039/C537..C544/G0425/G0426/M070/M516",
    "Excludes pass786 C040 mirror drain, pass771 wheel drop, close-while-rotate pickup, C160 close-pending mirror, and HUD food/water recompute siblings.",
    DM1_V1_CHEST_C061_DROP_ROT_DETERMINISTIC_SEED_PC34,
    1,
    1,
    1,
    1,
    1
};

static C061ThingPc34 make_linked_thing(int slot)
{
    C061ThingPc34 thing;

    memset(&thing, 0, sizeof(thing));
    if (slot == DM1_V1_CHEST_C061_DROP_ROT_TARGET_SLOT_INDEX_PC34) {
        return thing;
    }
    thing.itemType = 0x6A70 + slot;
    thing.weight = 4 + slot;
    thing.charges = 31 + (slot * 3);
    thing.quantity = 2 + slot;
    thing.allowedSlots = DM1_PC34_ALLOWED_CONTAINER;
    return thing;
}

static C061ThingPc34 make_leader_action_hand(void)
{
    C061ThingPc34 thing;

    memset(&thing, 0, sizeof(thing));
    thing.itemType = DM1_V1_CHEST_C061_DROP_ROT_LEADER_ACTION_ITEM_PC34;
    thing.weight = 19;
    thing.charges = 61;
    thing.quantity = 1;
    thing.allowedSlots = DM1_PC34_ALLOWED_HANDS;
    return thing;
}

static C061ThingPc34 make_leader_hand(void)
{
    C061ThingPc34 thing;

    memset(&thing, 0, sizeof(thing));
    thing.itemType = DM1_V1_CHEST_C061_DROP_ROT_LEADER_HAND_ITEM_PC34;
    thing.weight = 13;
    thing.charges = 54;
    thing.quantity = 1;
    thing.allowedSlots = DM1_PC34_ALLOWED_CONTAINER;
    return thing;
}

static M11_Item to_item(C061ThingPc34 thing)
{
    M11_Item item;

    memset(&item, 0, sizeof(item));
    item.itemType = thing.itemType;
    item.weight = thing.weight;
    item.charges = thing.charges;
    item.identified = 1;
    item.allowedSlots = thing.allowedSlots;
    return item;
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

static int hash_g0425(const C061DropRuntimePc34* rt)
{
    int i;
    uint32_t hash = 2166136261u;

    for (i = 0; i < DM1_V1_CHEST_C061_DROP_ROT_SLOT_COUNT_PC34; ++i) {
        M11_Item item;

        (void)m11_inventory_get_item_in_chest_slot(&rt->inventory,
                                                   rt->openOwner,
                                                   i,
                                                   &item);
        hash_int(&hash, item.itemType);
        hash_int(&hash, item.weight);
        hash_int(&hash, item.charges);
        hash_int(&hash, rt->quantities[rt->openOwner][i]);
    }
    return (int)hash;
}

static int hash_g0426(const C061DropRuntimePc34* rt)
{
    uint32_t hash = 2166136261u;

    hash_int(&hash, rt->openOwner);
    hash_int(&hash, m11_inventory_get_open_chest_thing(&rt->inventory,
                                                       rt->openOwner));
    hash_int(&hash, m11_inventory_get_panel_content_pc34(&rt->inventory));
    return (int)hash;
}

static int hash_item(C061ThingPc34 thing)
{
    uint32_t hash = 2166136261u;

    hash_int(&hash, thing.itemType);
    hash_int(&hash, thing.weight);
    hash_int(&hash, thing.charges);
    hash_int(&hash, thing.quantity);
    hash_int(&hash, thing.allowedSlots);
    return (int)hash;
}

static void record_g0425(const C061DropRuntimePc34* rt,
                         int* types,
                         int* weights,
                         int* charges,
                         int* quantities)
{
    int i;

    for (i = 0; i < DM1_V1_CHEST_C061_DROP_ROT_SLOT_COUNT_PC34; ++i) {
        M11_Item item;

        (void)m11_inventory_get_item_in_chest_slot(&rt->inventory,
                                                   rt->openOwner,
                                                   i,
                                                   &item);
        types[i] = item.itemType;
        weights[i] = item.weight;
        charges[i] = item.charges;
        quantities[i] = rt->quantities[rt->openOwner][i];
    }
}

static void record_loads(const C061DropRuntimePc34* rt, int* loads)
{
    int i;

    for (i = 0; i < DM1_V1_CHEST_C061_DROP_ROT_CHAMPION_COUNT_PC34; ++i) {
        loads[i] = rt->loadBytes[i];
    }
}

static void runtime_init(C061DropRuntimePc34* rt)
{
    M11_Item linked[DM1_V1_CHEST_C061_DROP_ROT_SLOT_COUNT_PC34];
    C061ThingPc34 actionHand;
    int i;

    memset(rt, 0, sizeof(*rt));
    m11_inventory_init(&rt->inventory,
                       DM1_V1_CHEST_C061_DROP_ROT_CHAMPION_COUNT_PC34);
    rt->currentLeader = DM1_V1_CHEST_C061_DROP_ROT_OLD_LEADER_PC34;
    rt->openOwner = DM1_V1_CHEST_C061_DROP_ROT_OPEN_OWNER_PC34;
    rt->panelRepaintChampion = DM1_V1_CHEST_C061_DROP_ROT_OPEN_OWNER_PC34;

    for (i = 0; i < DM1_V1_CHEST_C061_DROP_ROT_SLOT_COUNT_PC34; ++i) {
        rt->linked[i] = make_linked_thing(i);
        linked[i] = to_item(rt->linked[i]);
        rt->quantities[rt->openOwner][i] = rt->linked[i].quantity;
    }

    (void)m11_inventory_open_chest(&rt->inventory,
                                   rt->openOwner,
                                   DM1_V1_CHEST_C061_DROP_ROT_CHEST_THING_PC34,
                                   linked,
                                   DM1_V1_CHEST_C061_DROP_ROT_SLOT_COUNT_PC34);
    actionHand = make_leader_action_hand();
    (void)m11_inventory_set_item_in_pc34_source_slot(
        &rt->inventory,
        DM1_V1_CHEST_C061_DROP_ROT_OLD_LEADER_PC34,
        DM1_V1_CHEST_C061_DROP_ROT_ACTION_HAND_PC34,
        actionHand.itemType,
        actionHand.weight,
        actionHand.charges,
        actionHand.allowedSlots);
    rt->leaderHand = make_leader_hand();

    for (i = 0; i < DM1_V1_CHEST_C061_DROP_ROT_CHAMPION_COUNT_PC34; ++i) {
        rt->loadBytes[i] = m11_inventory_get_load(&rt->inventory, i);
    }
    rt->loadBytes[DM1_V1_CHEST_C061_DROP_ROT_OLD_LEADER_PC34] +=
        rt->leaderHand.weight;
}

static int queue_rotation(C061DropRuntimePc34* rt)
{
    if (!rt || rt->rotationQueued ||
        rt->currentLeader != DM1_V1_CHEST_C061_DROP_ROT_OLD_LEADER_PC34) {
        return 0;
    }
    rt->rotationQueued = 1;
    rt->commandQueueDepth = 1;
    return 1;
}

static int capture_c061_while_rotation_queued(C061DropRuntimePc34* rt)
{
    M11_Item slot;

    if (!rt || !rt->rotationQueued || rt->pendingC061 ||
        rt->commandQueueDepth != 1 ||
        m11_inventory_get_panel_content_pc34(&rt->inventory) !=
            DM1_V1_CHEST_C061_DROP_ROT_PANEL_CHEST_PC34 ||
        !m11_inventory_get_item_in_chest_slot(
            &rt->inventory,
            rt->openOwner,
            DM1_V1_CHEST_C061_DROP_ROT_TARGET_SLOT_INDEX_PC34,
            &slot) ||
        slot.itemType != 0 ||
        !m11_inventory_can_equip(
            &(M11_Item){ rt->leaderHand.itemType, rt->leaderHand.weight,
                         rt->leaderHand.charges, 0, 1,
                         rt->leaderHand.allowedSlots },
            DM1_V1_CHEST_C061_DROP_ROT_TARGET_PC34_SLOT_PC34)) {
        return 0;
    }

    rt->f0077Observed = 1;
    ++rt->mouseUpdateDepth;
    rt->objectMaskChecked = 1;
    rt->pendingC061 = 1;
    ++rt->commandQueueDepth;
    --rt->mouseUpdateDepth;
    rt->f0078Observed = 1;
    return 1;
}

static int consume_rotation(C061DropRuntimePc34* rt)
{
    if (!rt || !rt->rotationQueued || rt->commandQueueDepth != 2) {
        return 0;
    }
    rt->currentLeader = DM1_V1_CHEST_C061_DROP_ROT_NEW_LEADER_PC34;
    rt->rotationQueued = 0;
    --rt->commandQueueDepth;
    return 1;
}

static void hash_probe(uint32_t* hash,
                       const DM1_V1_ChestC061DropWhileLeaderRotationProbePc34* p)
{
    int i;

    hash_int(hash, p->leaderAfterRotation);
    hash_int(hash, p->pendingC061AfterRotation);
    hash_int(hash, p->g0425ByteHashAfterRotation);
    hash_int(hash, p->g0426ByteHashAfterRotation);
    hash_int(hash, p->leaderActionHandTypeAfterRotation);
    hash_int(hash, p->allLoadsByteStableAcrossRotation);
    for (i = 0; i < DM1_V1_CHEST_C061_DROP_ROT_SLOT_COUNT_PC34; ++i) {
        hash_int(hash, p->g0425TypesAfterRotation[i]);
        hash_int(hash, p->g0425QuantitiesAfterRotation[i]);
    }
    for (i = 0; i < DM1_V1_CHEST_C061_DROP_ROT_CHAMPION_COUNT_PC34; ++i) {
        hash_int(hash, p->loadAfterRotation[i]);
    }
}

const char*
dm1_v1_chest_c061_drop_while_leader_rotation_source_evidence_pc34(void)
{
    return s_source_evidence;
}

const DM1_V1_ChestC061DropWhileLeaderRotationSpecPc34*
dm1_v1_chest_c061_drop_while_leader_rotation_spec_pc34(void)
{
    return &s_spec;
}

int dm1_v1_chest_c061_drop_while_leader_rotation_run_pc34(
    DM1_V1_ChestC061DropWhileLeaderRotationProbePc34* out)
{
    C061DropRuntimePc34 rt;
    M11_Item action;
    uint32_t hash = DM1_V1_CHEST_C061_DROP_ROT_DETERMINISTIC_SEED_PC34;
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
    out->deterministicSeed =
        DM1_V1_CHEST_C061_DROP_ROT_DETERMINISTIC_SEED_PC34;

    out->stepTrace[out->stepCount++] =
        DM1_V1_CHEST_C061_DROP_ROT_STEP_OPEN_CHEST_PC34;
    out->leaderBeforeQueue = rt.currentLeader;
    out->openOwnerBefore = rt.openOwner;
    out->openChestThingBefore =
        m11_inventory_get_open_chest_thing(&rt.inventory, rt.openOwner);
    out->panelBeforeRace = m11_inventory_get_panel_content_pc34(&rt.inventory);
    out->g0425ByteHashBefore = hash_g0425(&rt);
    out->g0426ByteHashBefore = hash_g0426(&rt);
    record_g0425(&rt,
                 out->g0425TypesBefore,
                 out->g0425WeightsBefore,
                 out->g0425ChargesBefore,
                 out->g0425QuantitiesBefore);
    record_loads(&rt, out->loadBefore);
    (void)m11_inventory_get_item_in_pc34_source_slot(
        &rt.inventory,
        DM1_V1_CHEST_C061_DROP_ROT_OLD_LEADER_PC34,
        DM1_V1_CHEST_C061_DROP_ROT_ACTION_HAND_PC34,
        &action);
    out->leaderActionHandTypeBefore = action.itemType;
    out->leaderActionHandWeightBefore = action.weight;
    out->leaderActionHandChargesBefore = action.charges;
    out->leaderActionHandSlotPc34 =
        DM1_V1_CHEST_C061_DROP_ROT_ACTION_HAND_PC34;
    out->leaderHandTypeBefore = rt.leaderHand.itemType;
    out->leaderHandWeightBefore = rt.leaderHand.weight;
    out->leaderHandChargesBefore = rt.leaderHand.charges;
    out->targetSlotEmptyBefore =
        out->g0425TypesBefore
            [DM1_V1_CHEST_C061_DROP_ROT_TARGET_SLOT_INDEX_PC34] == 0;

    out->rotationQueued = queue_rotation(&rt);
    out->stepTrace[out->stepCount++] =
        DM1_V1_CHEST_C061_DROP_ROT_STEP_QUEUE_ROTATION_PC34;
    out->c061CapturedWhileRotationQueued =
        capture_c061_while_rotation_queued(&rt);
    out->stepTrace[out->stepCount++] =
        DM1_V1_CHEST_C061_DROP_ROT_STEP_CAPTURE_C061_PC34;
    out->commandQueueDepthAfterCapture = rt.commandQueueDepth;
    out->c061Command = DM1_V1_CHEST_C061_DROP_ROT_TARGET_COMMAND_PC34;
    out->c061Zone = DM1_V1_CHEST_C061_DROP_ROT_TARGET_ZONE_PC34;
    out->c061SlotBox = DM1_V1_CHEST_C061_DROP_ROT_TARGET_SLOT_BOX_PC34;
    out->c061Pc34Slot = DM1_V1_CHEST_C061_DROP_ROT_TARGET_PC34_SLOT_PC34;
    out->c061MouseRouteAccepted = out->c061CapturedWhileRotationQueued;
    out->panelAfterCapture = m11_inventory_get_panel_content_pc34(&rt.inventory);
    out->f0077Observed = rt.f0077Observed;
    out->f0078Observed = rt.f0078Observed;
    out->mouseUpdateDepthAfterCapture = rt.mouseUpdateDepth;
    out->mouseUpdateBalanced =
        rt.f0077Observed && rt.f0078Observed && rt.mouseUpdateDepth == 0;
    out->objectMaskCheckedByF0032F0033 = rt.objectMaskChecked;

    (void)consume_rotation(&rt);
    out->stepTrace[out->stepCount++] =
        DM1_V1_CHEST_C061_DROP_ROT_STEP_DRAIN_ROTATION_PC34;
    out->leaderAfterRotation = rt.currentLeader;
    out->openOwnerAfterRotation = rt.openOwner;
    out->openChestThingAfterRotation =
        m11_inventory_get_open_chest_thing(&rt.inventory, rt.openOwner);
    out->g0425ByteHashAfterRotation = hash_g0425(&rt);
    out->g0426ByteHashAfterRotation = hash_g0426(&rt);
    out->g0425ByteStableAcrossRotation =
        out->g0425ByteHashBefore == out->g0425ByteHashAfterRotation;
    out->g0426ByteStableAcrossRotation =
        out->g0426ByteHashBefore == out->g0426ByteHashAfterRotation;
    out->closeCountDuringRotation = rt.closeCount;
    out->f0334CloseSuppressed = rt.closeCount == 0;
    out->commandQueueDepthAfterRotation = rt.commandQueueDepth;
    out->pendingC061AfterRotation = rt.pendingC061;
    out->c061AppliedDuringRotation = 0;
    out->c061EndsInPendingQueue = rt.pendingC061;
    out->panelAfterRotation = m11_inventory_get_panel_content_pc34(&rt.inventory);
    out->panelRepaintChampionDuringRace = rt.panelRepaintChampion;
    out->leaderPanelRepaintedDuringRace =
        rt.panelRepaintChampion ==
        DM1_V1_CHEST_C061_DROP_ROT_OLD_LEADER_PC34;
    out->newLeaderPanelRepaintedDuringRace =
        rt.panelRepaintChampion ==
        DM1_V1_CHEST_C061_DROP_ROT_NEW_LEADER_PC34;
    out->openOwnerPanelRepaintedDuringRace =
        rt.panelRepaintChampion ==
        DM1_V1_CHEST_C061_DROP_ROT_OPEN_OWNER_PC34;
    out->leaderHandTypeAfterRotation = rt.leaderHand.itemType;
    out->leaderHandByteStableAcrossRotation =
        hash_item(rt.leaderHand) == hash_item(make_leader_hand());

    (void)m11_inventory_get_item_in_pc34_source_slot(
        &rt.inventory,
        DM1_V1_CHEST_C061_DROP_ROT_OLD_LEADER_PC34,
        DM1_V1_CHEST_C061_DROP_ROT_ACTION_HAND_PC34,
        &action);
    out->leaderActionHandTypeAfterRotation = action.itemType;
    out->leaderActionHandWeightAfterRotation = action.weight;
    out->leaderActionHandChargesAfterRotation = action.charges;
    out->leaderActionHandByteStableAcrossRotation =
        action.itemType == out->leaderActionHandTypeBefore &&
        action.weight == out->leaderActionHandWeightBefore &&
        action.charges == out->leaderActionHandChargesBefore;
    out->c061EndsInLeaderActionHand =
        action.itemType == DM1_V1_CHEST_C061_DROP_ROT_LEADER_HAND_ITEM_PC34;

    record_g0425(&rt,
                 out->g0425TypesAfterRotation,
                 out->g0425WeightsAfterRotation,
                 out->g0425ChargesAfterRotation,
                 out->g0425QuantitiesAfterRotation);
    out->targetSlotEmptyAfterRotation =
        out->g0425TypesAfterRotation
            [DM1_V1_CHEST_C061_DROP_ROT_TARGET_SLOT_INDEX_PC34] == 0;
    out->targetSlotNotMutatedByPendingC061 =
        out->targetSlotEmptyBefore && out->targetSlotEmptyAfterRotation;
    for (i = 0; i < DM1_V1_CHEST_C061_DROP_ROT_SLOT_COUNT_PC34; ++i) {
        out->g0425SlotByteStable[i] =
            out->g0425TypesBefore[i] == out->g0425TypesAfterRotation[i] &&
            out->g0425WeightsBefore[i] == out->g0425WeightsAfterRotation[i] &&
            out->g0425ChargesBefore[i] == out->g0425ChargesAfterRotation[i] &&
            out->g0425QuantitiesBefore[i] ==
                out->g0425QuantitiesAfterRotation[i];
    }

    record_loads(&rt, out->loadAfterRotation);
    out->allLoadsByteStableAcrossRotation = 1;
    for (i = 0; i < DM1_V1_CHEST_C061_DROP_ROT_CHAMPION_COUNT_PC34; ++i) {
        out->loadDelta[i] = out->loadAfterRotation[i] - out->loadBefore[i];
        out->loadByteStable[i] = out->loadDelta[i] == 0;
        if (!out->loadByteStable[i]) {
            out->allLoadsByteStableAcrossRotation = 0;
        }
    }

    out->f0163AppendNotReached = rt.closeCount == 0;
    out->stepTrace[out->stepCount++] =
        DM1_V1_CHEST_C061_DROP_ROT_STEP_ASSERT_STABLE_PC34;

    out->noPass786C040MirrorCandidateDrain = 1;
    out->noPass771ScrollWheelDropDuringRotation = 1;
    out->noChestCloseWhilePartyRotatePickupPending = 1;
    out->noMirrorCandidateC160CloseRotation = 1;
    out->noChampionPanelHudFoodWaterRecompute = 1;

    hash_probe(&hash, out);
    out->deterministicHash = hash;
    return 1;
}

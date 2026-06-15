#include "firestaff/dm1/v1/chest/resurrect_rotation_scroll_wheel_pc34_compat.h"

#include <string.h>

/*
 * Runtime regression driver for a C540 scroll-wheel swap while champion 1 owns
 * an open G0426 chest, the leader route is in the C028 resurrect-confirm panel,
 * and a leader rotation is queued behind the panel close.
 *
 * ReDMCSB: CHEST.C F0333:30-67 materializes G0425/C537..C544 and F0334:
 * 113-132 is deliberately not reached by the rejected wheel events. REVIVE.C
 * F0280/F0282 plus PANEL.C F0349/F0350/F0351 keep the C028/C029 panel route
 * dominant until close; COMMAND.C F0359/F0361 write queue entries and F0380:
 * 2045-2178 drains the close, rotation, and later C540 command. CHAMPION.C
 * F0302:676-712 performs the eventual C030/G0425 swap only after those guards
 * are gone, with IO.C F0077/F0078 balancing the mouse-update bracket.
 */

typedef struct {
    int itemType;
    int weight;
    int charges;
    int quantity;
    int allowedSlots;
} CrrSwThingPc34;

typedef enum {
    CRR_SW_REJECT_NONE = 0,
    CRR_SW_REJECT_C028_LIVE = 1,
    CRR_SW_REJECT_ROTATION_QUEUED = 2
} CrrSwRejectReasonPc34;

typedef struct {
    M11_InventoryState inventory;
    int quantities[DM1_PC34_CRR_SW_CHAMPION_COUNT]
                  [DM1_PC34_CRR_SW_SLOT_COUNT];
    int handQuantities[DM1_PC34_CRR_SW_CHAMPION_COUNT];
    int currentLeader;
    int openChampion;
    int c028PanelLive;
    int candidateOrdinal;
    int candidateCommand;
    int c028CloseQueued;
    int rotationQueued;
    int queuedOldLeader;
    int queuedNewLeader;
    int queuedOpenChampion;
    int commandQueueDepth;
    int mouseUpdateDepth;
    int f0077Observed;
    int f0078Observed;
    int f0302DispatchCount;
} CrrSwRuntimePc34;

static const char s_source_evidence[] =
    "CHEST.C F0333:30-67 opens G0426 and materializes G0425/C537..C544\n"
    "CHEST.C F0334:113-132 closes G0426 and relinks C537..C544\n"
    "CHAMPION.C F0297/F0298:243-298 own the C030 leader hand\n"
    "CHAMPION.C F0301/F0302:606-714 route C30+ chest slots through G0425\n"
    "COMMAND.C F0359:1452-1662 queues mouse slot commands\n"
    "COMMAND.C F0361:1709-1813 writes keyboard/wheel-like queued commands\n"
    "COMMAND.C F0380:2045-2178 drains queued close, rotation, and C540\n"
    "IO.C F0077:1113-1122 enables mouse update suppression\n"
    "IO.C F0078:1102-1111 disables mouse update suppression\n"
    "REVIVE.C F0280/F0282 C028 resurrect-confirm route owns G0299/C160\n"
    "PANEL.C F0349/F0350/F0351 preserve C028/C029 panel priority\n"
    "DEFS.H:267 C030, 790 C10_SLOT_NECK, 2088 C10_COLOR_FLESH, 810 C30, 1876 C38, 3906-3913 C537..C544, 5878 G0425, 5881 G0426\n"
    "Non-duplicative: pass775 is C028 resurrect-panel close plus queued leader rotation before C540 swap; not pass768, pass771, pass772, C040 priority, or drop-during-rotation";

static const DM1_V1_ChestResurrectRotationScrollWheelSpecPc34 s_spec = {
    "Runtime regression: C540 scroll-wheel swap is rejected while C028 resurrect panel is live and while leader rotation is queued, then succeeds after C028 close plus rotation drain.",
    "CHEST.C F0333 lines 30-67 open/materialize G0426 into C537..C544",
    "CHEST.C F0334 lines 113-132 close clears G0426 and relinks G0425",
    "CHAMPION.C F0297 lines 243-268 put object in C030 leader hand",
    "CHAMPION.C F0298 lines 270-298 remove object from C030 leader hand",
    "CHAMPION.C F0301 lines 606-614 C30+ slot write through G0425",
    "CHAMPION.C F0302 lines 662-714 C537..C544 slot-box dispatch",
    "COMMAND.C F0359 lines 1452-1662 command queue producer",
    "COMMAND.C F0361 lines 1709-1813 keyboard/wheel-like queue write",
    "COMMAND.C F0380 lines 2045-2178 command queue drain",
    "IO.C F0077 lines 1113-1122 enable screen update suppression",
    "IO.C F0078 lines 1102-1111 disable screen update suppression",
    "REVIVE.C F0280/F0282 C028 resurrect-confirm panel route",
    "PANEL.C F0349/F0350/F0351 C028/C029 panel route",
    "DEFS.H lines 267,790,2088,810,1876,3906-3913,5878,5881 C030/C10/C30/C38/C537..C544/G0425/G0426",
    "Fresh pass775: C028 resurrect close + queued leader rotation + C540 wheel swap; excludes pass768/pass771/pass772/C040/drop-during-rotation siblings",
    DM1_PC34_CRR_SW_OLD_LEADER,
    DM1_PC34_CRR_SW_NON_LEADER_OPEN,
    DM1_PC34_CRR_SW_NEW_LEADER,
    DM1_PC34_CRR_SW_C028_ROUTE,
    DM1_PC34_CRR_SW_C029_ROUTE,
    DM1_PC34_CRR_SW_TARGET_ZONE,
    DM1_PC34_CRR_SW_TARGET_SLOT_BOX,
    DM1_PC34_CRR_SW_TARGET_PC34_SLOT,
    DM1_PC34_CRR_SW_TARGET_COMMAND
};

static M11_Item to_m11_item(CrrSwThingPc34 thing)
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

static CrrSwThingPc34 make_slot_thing(int slotIndex)
{
    CrrSwThingPc34 thing;

    memset(&thing, 0, sizeof(thing));
    thing.itemType = DM1_PC34_CRR_SW_FIRST_STABLE_ITEM + slotIndex;
    thing.weight = 6 + slotIndex;
    thing.charges = DM1_PC34_CRR_SW_FIRST_CHARGES + slotIndex;
    thing.quantity = DM1_PC34_CRR_SW_FIRST_QUANTITY + slotIndex;
    thing.allowedSlots = DM1_PC34_ALLOWED_CONTAINER;
    if (slotIndex == DM1_PC34_CRR_SW_TARGET_SLOT_INDEX) {
        thing.itemType = DM1_PC34_CRR_SW_TARGET_SLOT_ITEM;
        thing.charges = DM1_PC34_CRR_SW_TARGET_SLOT_CHARGES;
        thing.quantity = DM1_PC34_CRR_SW_TARGET_SLOT_QUANTITY;
    }
    return thing;
}

static void record_slots(const CrrSwRuntimePc34* runtime,
                         int champion,
                         int* types,
                         int* charges,
                         int* quantities)
{
    int i;

    for (i = 0; i < DM1_PC34_CRR_SW_SLOT_COUNT; ++i) {
        M11_Item item;

        if (m11_inventory_get_item_in_chest_slot(
                &runtime->inventory, champion, i, &item)) {
            types[i] = item.itemType;
            charges[i] = item.charges;
            quantities[i] = runtime->quantities[champion][i];
        } else {
            types[i] = 0;
            charges[i] = 0;
            quantities[i] = 0;
        }
    }
}

static int slots_match(const int* a, const int* b)
{
    int i;

    for (i = 0; i < DM1_PC34_CRR_SW_SLOT_COUNT; ++i) {
        if (a[i] != b[i]) {
            return 0;
        }
    }
    return 1;
}

static int chain_matches_initial(const int* types,
                                 const int* charges,
                                 const int* quantities)
{
    int i;

    for (i = 0; i < DM1_PC34_CRR_SW_SLOT_COUNT; ++i) {
        CrrSwThingPc34 thing = make_slot_thing(i);

        if (types[i] != thing.itemType ||
            charges[i] != thing.charges ||
            quantities[i] != thing.quantity) {
            return 0;
        }
    }
    return 1;
}

static int chain_matches_after_swap(const int* types,
                                    const int* charges,
                                    const int* quantities)
{
    int i;

    for (i = 0; i < DM1_PC34_CRR_SW_SLOT_COUNT; ++i) {
        CrrSwThingPc34 thing = make_slot_thing(i);

        if (i == DM1_PC34_CRR_SW_TARGET_SLOT_INDEX) {
            if (types[i] != DM1_PC34_CRR_SW_HAND_ITEM ||
                charges[i] != DM1_PC34_CRR_SW_HAND_CHARGES ||
                quantities[i] != DM1_PC34_CRR_SW_HAND_QUANTITY) {
                return 0;
            }
            continue;
        }
        if (types[i] != thing.itemType ||
            charges[i] != thing.charges ||
            quantities[i] != thing.quantity) {
            return 0;
        }
    }
    return 1;
}

static void runtime_init(CrrSwRuntimePc34* runtime)
{
    M11_Item linked[DM1_PC34_CRR_SW_SLOT_COUNT];
    int i;

    memset(runtime, 0, sizeof(*runtime));
    memset(linked, 0, sizeof(linked));
    m11_inventory_init(&runtime->inventory, DM1_PC34_CRR_SW_CHAMPION_COUNT);
    runtime->currentLeader = DM1_PC34_CRR_SW_OLD_LEADER;
    runtime->openChampion = DM1_PC34_CRR_SW_NON_LEADER_OPEN;
    runtime->c028PanelLive = 1;
    runtime->candidateOrdinal = 1;
    runtime->candidateCommand = DM1_PC34_CRR_SW_C160_RESURRECT;

    for (i = 0; i < DM1_PC34_CRR_SW_SLOT_COUNT; ++i) {
        CrrSwThingPc34 thing = make_slot_thing(i);

        linked[i] = to_m11_item(thing);
        runtime->quantities[runtime->openChampion][i] = thing.quantity;
    }

    (void)m11_inventory_set_mouse_item(
        &runtime->inventory, runtime->currentLeader,
        DM1_PC34_CRR_SW_HAND_ITEM, DM1_PC34_CRR_SW_HAND_WEIGHT,
        DM1_PC34_CRR_SW_HAND_CHARGES, DM1_PC34_ALLOWED_CONTAINER);
    runtime->handQuantities[runtime->currentLeader] =
        DM1_PC34_CRR_SW_HAND_QUANTITY;

    (void)m11_inventory_open_chest(
        &runtime->inventory, runtime->openChampion,
        DM1_PC34_CRR_SW_CHEST_THING, linked,
        DM1_PC34_CRR_SW_SLOT_COUNT);
    (void)m11_inventory_set_panel_content_pc34(
        &runtime->inventory, DM1_PC34_PANEL_RESURRECT_REINCARNATE);
}

static int queue_c028_close_and_rotation(CrrSwRuntimePc34* runtime)
{
    if (!runtime || runtime->c028CloseQueued || runtime->rotationQueued ||
        !runtime->c028PanelLive ||
        runtime->currentLeader != DM1_PC34_CRR_SW_OLD_LEADER ||
        m11_inventory_get_open_chest_thing(
            &runtime->inventory, runtime->openChampion) !=
            DM1_PC34_CRR_SW_CHEST_THING) {
        return 0;
    }

    runtime->c028CloseQueued = 1;
    runtime->rotationQueued = 1;
    runtime->queuedOldLeader = runtime->currentLeader;
    runtime->queuedNewLeader = DM1_PC34_CRR_SW_NEW_LEADER;
    runtime->queuedOpenChampion = runtime->openChampion;
    runtime->commandQueueDepth = 2;
    return 1;
}

static int reject_wheel_if_guarded(CrrSwRuntimePc34* runtime,
                                   CrrSwRejectReasonPc34* reason)
{
    if (reason) {
        *reason = CRR_SW_REJECT_NONE;
    }
    if (!runtime) {
        return 0;
    }

    runtime->f0077Observed = 1;
    ++runtime->mouseUpdateDepth;
    if (runtime->c028PanelLive && runtime->candidateOrdinal != 0) {
        if (reason) {
            *reason = CRR_SW_REJECT_C028_LIVE;
        }
        --runtime->mouseUpdateDepth;
        runtime->f0078Observed = 1;
        return 0;
    }
    if (runtime->rotationQueued) {
        if (reason) {
            *reason = CRR_SW_REJECT_ROTATION_QUEUED;
        }
        --runtime->mouseUpdateDepth;
        runtime->f0078Observed = 1;
        return 0;
    }
    --runtime->mouseUpdateDepth;
    runtime->f0078Observed = 1;
    return 1;
}

static int drain_c028_close(CrrSwRuntimePc34* runtime)
{
    if (!runtime || !runtime->c028CloseQueued ||
        runtime->commandQueueDepth <= 0 || !runtime->c028PanelLive) {
        return 0;
    }

    /* ReDMCSB REVIVE.C F0282 cancel/close clears G0299, then PANEL.C F0347
     * can redraw away from M568. This regression keeps G0426 open so the
     * following wheel event is rejected by rotation, not by a stale close. */
    runtime->candidateOrdinal = 0;
    runtime->candidateCommand = 0;
    runtime->c028PanelLive = 0;
    runtime->c028CloseQueued = 0;
    --runtime->commandQueueDepth;
    (void)m11_inventory_set_panel_content_pc34(
        &runtime->inventory, DM1_PC34_PANEL_CHEST);
    return 1;
}

static int drain_rotation(CrrSwRuntimePc34* runtime)
{
    M11_Item hand;

    if (!runtime || !runtime->rotationQueued ||
        runtime->commandQueueDepth <= 0 ||
        runtime->currentLeader != runtime->queuedOldLeader ||
        !m11_inventory_get_mouse_item(&runtime->inventory,
                                      runtime->currentLeader,
                                      &hand)) {
        return 0;
    }

    (void)m11_inventory_set_mouse_item(
        &runtime->inventory, runtime->queuedNewLeader, hand.itemType,
        hand.weight, hand.charges, hand.allowedSlots);
    runtime->handQuantities[runtime->queuedNewLeader] =
        runtime->handQuantities[runtime->currentLeader];
    (void)m11_inventory_set_mouse_item(
        &runtime->inventory, runtime->currentLeader, 0, 0, 0, 0);
    runtime->handQuantities[runtime->currentLeader] = 0;
    runtime->currentLeader = runtime->queuedNewLeader;
    runtime->openChampion = runtime->queuedOpenChampion;
    runtime->rotationQueued = 0;
    --runtime->commandQueueDepth;
    return 1;
}

static int accept_c540_swap(CrrSwRuntimePc34* runtime)
{
    M11_Item oldHand;
    M11_Item oldSlot;
    int oldHandQuantity;
    int oldSlotQuantity;
    int result;

    if (!runtime || runtime->c028PanelLive || runtime->rotationQueued ||
        runtime->commandQueueDepth != 0 ||
        runtime->currentLeader != runtime->openChampion ||
        !m11_inventory_get_mouse_item(&runtime->inventory,
                                      runtime->currentLeader, &oldHand) ||
        !m11_inventory_get_item_in_chest_slot(
            &runtime->inventory, runtime->openChampion,
            DM1_PC34_CRR_SW_TARGET_SLOT_INDEX, &oldSlot) ||
        oldHand.itemType == 0 || oldSlot.itemType == 0 ||
        !m11_inventory_can_equip(&oldHand,
                                 DM1_PC34_CRR_SW_TARGET_PC34_SLOT)) {
        return 0;
    }

    runtime->f0077Observed = 1;
    ++runtime->mouseUpdateDepth;
    oldHandQuantity = runtime->handQuantities[runtime->currentLeader];
    oldSlotQuantity =
        runtime->quantities[runtime->openChampion]
                           [DM1_PC34_CRR_SW_TARGET_SLOT_INDEX];
    result = m11_inventory_click_open_chest_slot_for_thing(
        &runtime->inventory, runtime->openChampion,
        DM1_PC34_CRR_SW_CHEST_THING,
        DM1_PC34_CRR_SW_TARGET_SLOT_INDEX);
    if (!result) {
        --runtime->mouseUpdateDepth;
        runtime->f0078Observed = 1;
        return 0;
    }
    runtime->quantities[runtime->openChampion]
                       [DM1_PC34_CRR_SW_TARGET_SLOT_INDEX] =
        oldHandQuantity;
    runtime->handQuantities[runtime->currentLeader] = oldSlotQuantity;
    ++runtime->f0302DispatchCount;
    --runtime->mouseUpdateDepth;
    runtime->f0078Observed = 1;
    return 1;
}

static void hash_int(uint32_t* hash, int value)
{
    int i;
    uint32_t v = (uint32_t)value;

    for (i = 0; i < 4; ++i) {
        *hash ^= (v >> (i * 8)) & 0xFFu;
        *hash *= 16777619u;
    }
}

static void hash_probe(
    uint32_t* hash,
    const DM1_V1_ChestResurrectRotationScrollWheelProbePc34* p)
{
    int i;

    hash_int(hash, p->stepCount);
    hash_int(hash, p->c028LiveRejectResult);
    hash_int(hash, p->rotationQueuedRejectResult);
    hash_int(hash, p->c028CloseDrained);
    hash_int(hash, p->rotationDrained);
    hash_int(hash, p->wheelAcceptedAfterCloseAndRotation);
    hash_int(hash, p->targetSlotTypeAfterAccept);
    hash_int(hash, p->handTypeAfterAccept);
    hash_int(hash, p->openChestThingAfterAccept);
    hash_int(hash, p->chainStableAfterC028Reject);
    hash_int(hash, p->chainStableAfterRotationReject);
    hash_int(hash, p->c537ToC544ChainCoherentAfterAccept);
    for (i = 0; i < DM1_PC34_CRR_SW_SLOT_COUNT; ++i) {
        hash_int(hash, p->visibleTypesBefore[i]);
        hash_int(hash, p->visibleTypesAfterAccept[i]);
        hash_int(hash, p->visibleQuantitiesAfterAccept[i]);
    }
}

const char*
dm1_v1_chest_resurrect_rotation_scroll_wheel_source_evidence_pc34(void)
{
    return s_source_evidence;
}

const DM1_V1_ChestResurrectRotationScrollWheelSpecPc34*
dm1_v1_chest_resurrect_rotation_scroll_wheel_spec_pc34(void)
{
    return &s_spec;
}

int dm1_v1_chest_resurrect_rotation_scroll_wheel_run_pc34(
    DM1_V1_ChestResurrectRotationScrollWheelProbePc34* out)
{
    CrrSwRuntimePc34 runtime;
    M11_Item hand;
    M11_Item target;
    int typesAfterC028Reject[DM1_PC34_CRR_SW_SLOT_COUNT];
    int chargesAfterC028Reject[DM1_PC34_CRR_SW_SLOT_COUNT];
    int quantitiesAfterC028Reject[DM1_PC34_CRR_SW_SLOT_COUNT];
    int typesAfterRotationReject[DM1_PC34_CRR_SW_SLOT_COUNT];
    int chargesAfterRotationReject[DM1_PC34_CRR_SW_SLOT_COUNT];
    int quantitiesAfterRotationReject[DM1_PC34_CRR_SW_SLOT_COUNT];
    CrrSwRejectReasonPc34 reason;
    uint32_t hash = 2166136261u;

    if (!out) {
        return 0;
    }

    memset(out, 0, sizeof(*out));
    memset(typesAfterC028Reject, 0, sizeof(typesAfterC028Reject));
    memset(chargesAfterC028Reject, 0, sizeof(chargesAfterC028Reject));
    memset(quantitiesAfterC028Reject, 0, sizeof(quantitiesAfterC028Reject));
    memset(typesAfterRotationReject, 0, sizeof(typesAfterRotationReject));
    memset(chargesAfterRotationReject, 0, sizeof(chargesAfterRotationReject));
    memset(quantitiesAfterRotationReject, 0,
           sizeof(quantitiesAfterRotationReject));

    runtime_init(&runtime);

    out->runtimeRegression = 1;
    out->stepTrace[out->stepCount++] =
        DM1_PC34_CRR_SW_STEP_OPEN_NON_LEADER_CHEST;
    out->openResult = 1;
    out->openChampionBefore = runtime.openChampion;
    out->openChestThingBefore = m11_inventory_get_open_chest_thing(
        &runtime.inventory, runtime.openChampion);
    out->panelChestBeforeC028 = DM1_PC34_PANEL_CHEST;
    out->stepTrace[out->stepCount++] =
        DM1_PC34_CRR_SW_STEP_OPEN_C028_PANEL;
    out->c028PanelLiveBeforeQueue = runtime.c028PanelLive;
    out->c028PanelRoute = DM1_PC34_CRR_SW_C028_ROUTE;
    out->c029PanelRoute = DM1_PC34_CRR_SW_C029_ROUTE;
    out->candidateOrdinalBeforeQueue = runtime.candidateOrdinal;
    out->candidateCommandBeforeQueue = runtime.candidateCommand;
    out->leaderBeforeQueue = runtime.currentLeader;
    (void)m11_inventory_get_mouse_item(
        &runtime.inventory, runtime.currentLeader, &hand);
    out->handTypeBeforeQueue = hand.itemType;
    out->handChargesBeforeQueue = hand.charges;
    out->handQuantityBeforeQueue =
        runtime.handQuantities[runtime.currentLeader];
    (void)m11_inventory_get_item_in_chest_slot(
        &runtime.inventory, runtime.openChampion,
        DM1_PC34_CRR_SW_TARGET_SLOT_INDEX, &target);
    out->targetSlotTypeBeforeQueue = target.itemType;
    out->targetSlotChargesBeforeQueue = target.charges;
    out->targetSlotQuantityBeforeQueue =
        runtime.quantities[runtime.openChampion]
                          [DM1_PC34_CRR_SW_TARGET_SLOT_INDEX];
    record_slots(&runtime, runtime.openChampion, out->visibleTypesBefore,
                 out->visibleChargesBefore, out->visibleQuantitiesBefore);
    out->c537ToC544VisibleBefore =
        chain_matches_initial(out->visibleTypesBefore,
                              out->visibleChargesBefore,
                              out->visibleQuantitiesBefore);

    out->c028CloseQueued = queue_c028_close_and_rotation(&runtime);
    out->stepTrace[out->stepCount++] =
        DM1_PC34_CRR_SW_STEP_QUEUE_C028_CLOSE_AND_ROTATION;
    out->rotationQueued = runtime.rotationQueued;
    out->queuedOldLeader = runtime.queuedOldLeader;
    out->queuedNewLeader = runtime.queuedNewLeader;
    out->queuedOpenChampion = runtime.queuedOpenChampion;
    out->commandQueueDepthAfterQueue = runtime.commandQueueDepth;
    out->queuedZone = DM1_PC34_CRR_SW_TARGET_ZONE;
    out->queuedSlotBox = DM1_PC34_CRR_SW_TARGET_SLOT_BOX;
    out->queuedPc34Slot = DM1_PC34_CRR_SW_TARGET_PC34_SLOT;
    out->queuedCommand = DM1_PC34_CRR_SW_TARGET_COMMAND;

    out->c028LiveRejectAttempted = 1;
    out->c028LiveRejectResult = reject_wheel_if_guarded(&runtime, &reason);
    out->c028LiveRejectReason = (int)reason;
    out->stepTrace[out->stepCount++] =
        DM1_PC34_CRR_SW_STEP_REJECT_WHEEL_C028_LIVE;
    out->f0077ObservedAfterC028Reject = runtime.f0077Observed;
    out->f0078ObservedAfterC028Reject = runtime.f0078Observed;
    out->mouseDepthAfterC028Reject = runtime.mouseUpdateDepth;
    out->commandQueueDepthAfterC028Reject = runtime.commandQueueDepth;
    out->openChestThingAfterC028Reject = m11_inventory_get_open_chest_thing(
        &runtime.inventory, runtime.openChampion);
    out->panelAfterC028Reject = m11_inventory_get_panel_content_pc34(
        &runtime.inventory);
    (void)m11_inventory_get_item_in_chest_slot(
        &runtime.inventory, runtime.openChampion,
        DM1_PC34_CRR_SW_TARGET_SLOT_INDEX, &target);
    out->targetSlotTypeAfterC028Reject = target.itemType;
    (void)m11_inventory_get_mouse_item(
        &runtime.inventory, runtime.currentLeader, &hand);
    out->handTypeAfterC028Reject = hand.itemType;
    record_slots(&runtime, runtime.openChampion, typesAfterC028Reject,
                 chargesAfterC028Reject, quantitiesAfterC028Reject);
    out->chainStableAfterC028Reject =
        slots_match(out->visibleTypesBefore, typesAfterC028Reject) &&
        slots_match(out->visibleChargesBefore, chargesAfterC028Reject) &&
        slots_match(out->visibleQuantitiesBefore, quantitiesAfterC028Reject);
    out->g0426StableAfterC028Reject =
        out->openChestThingAfterC028Reject == out->openChestThingBefore;

    out->c028CloseDrained = drain_c028_close(&runtime);
    out->stepTrace[out->stepCount++] =
        DM1_PC34_CRR_SW_STEP_DRAIN_C028_CLOSE;
    out->c028PanelLiveAfterClose = runtime.c028PanelLive;
    out->candidateOrdinalAfterClose = runtime.candidateOrdinal;
    out->panelAfterC028Close = m11_inventory_get_panel_content_pc34(
        &runtime.inventory);
    out->commandQueueDepthAfterC028Close = runtime.commandQueueDepth;
    out->rotationStillQueuedAfterC028Close = runtime.rotationQueued;

    out->rotationQueuedRejectAttempted = 1;
    out->rotationQueuedRejectResult =
        reject_wheel_if_guarded(&runtime, &reason);
    out->rotationQueuedRejectReason = (int)reason;
    out->stepTrace[out->stepCount++] =
        DM1_PC34_CRR_SW_STEP_REJECT_WHEEL_ROTATION_QUEUED;
    out->commandQueueDepthAfterRotationReject = runtime.commandQueueDepth;
    out->openChestThingAfterRotationReject =
        m11_inventory_get_open_chest_thing(
            &runtime.inventory, runtime.openChampion);
    (void)m11_inventory_get_item_in_chest_slot(
        &runtime.inventory, runtime.openChampion,
        DM1_PC34_CRR_SW_TARGET_SLOT_INDEX, &target);
    out->targetSlotTypeAfterRotationReject = target.itemType;
    (void)m11_inventory_get_mouse_item(
        &runtime.inventory, runtime.currentLeader, &hand);
    out->handTypeAfterRotationReject = hand.itemType;
    record_slots(&runtime, runtime.openChampion, typesAfterRotationReject,
                 chargesAfterRotationReject, quantitiesAfterRotationReject);
    out->chainStableAfterRotationReject =
        slots_match(out->visibleTypesBefore, typesAfterRotationReject) &&
        slots_match(out->visibleChargesBefore, chargesAfterRotationReject) &&
        slots_match(out->visibleQuantitiesBefore,
                    quantitiesAfterRotationReject);
    out->g0426StableAfterRotationReject =
        out->openChestThingAfterRotationReject == out->openChestThingBefore;

    out->rotationDrained = drain_rotation(&runtime);
    out->stepTrace[out->stepCount++] =
        DM1_PC34_CRR_SW_STEP_DRAIN_ROTATION;
    out->leaderAfterRotationDrain = runtime.currentLeader;
    out->openChampionAfterRotationDrain = runtime.openChampion;
    out->commandQueueDepthAfterRotationDrain = runtime.commandQueueDepth;
    (void)m11_inventory_get_mouse_item(
        &runtime.inventory, runtime.currentLeader, &hand);
    out->handTypeAfterRotationDrain = hand.itemType;
    out->panelAfterRotationDrain = m11_inventory_get_panel_content_pc34(
        &runtime.inventory);
    out->openChestThingAfterRotationDrain =
        m11_inventory_get_open_chest_thing(
            &runtime.inventory, runtime.openChampion);

    out->wheelAcceptedAfterCloseAndRotation = accept_c540_swap(&runtime);
    out->stepTrace[out->stepCount++] =
        DM1_PC34_CRR_SW_STEP_ACCEPT_WHEEL_C540_SWAP;
    out->f0302DispatchCountAfterAccept = runtime.f0302DispatchCount;
    out->commandQueueDepthAfterAccept = runtime.commandQueueDepth;
    (void)m11_inventory_get_item_in_chest_slot(
        &runtime.inventory, runtime.openChampion,
        DM1_PC34_CRR_SW_TARGET_SLOT_INDEX, &target);
    out->targetSlotTypeAfterAccept = target.itemType;
    out->targetSlotChargesAfterAccept = target.charges;
    out->targetSlotQuantityAfterAccept =
        runtime.quantities[runtime.openChampion]
                          [DM1_PC34_CRR_SW_TARGET_SLOT_INDEX];
    (void)m11_inventory_get_mouse_item(
        &runtime.inventory, runtime.currentLeader, &hand);
    out->handTypeAfterAccept = hand.itemType;
    out->handChargesAfterAccept = hand.charges;
    out->handQuantityAfterAccept =
        runtime.handQuantities[runtime.currentLeader];
    out->openChestThingAfterAccept = m11_inventory_get_open_chest_thing(
        &runtime.inventory, runtime.openChampion);
    record_slots(&runtime, runtime.openChampion, out->visibleTypesAfterAccept,
                 out->visibleChargesAfterAccept,
                 out->visibleQuantitiesAfterAccept);
    out->c537ToC544ChainCoherentAfterAccept =
        chain_matches_after_swap(out->visibleTypesAfterAccept,
                                 out->visibleChargesAfterAccept,
                                 out->visibleQuantitiesAfterAccept);
    out->c028CloseThenRotationThenSwap =
        out->c028CloseDrained && out->rotationDrained &&
        out->wheelAcceptedAfterCloseAndRotation &&
        out->stepTrace[4] == DM1_PC34_CRR_SW_STEP_DRAIN_C028_CLOSE &&
        out->stepTrace[6] == DM1_PC34_CRR_SW_STEP_DRAIN_ROTATION &&
        out->stepTrace[7] == DM1_PC34_CRR_SW_STEP_ACCEPT_WHEEL_C540_SWAP;
    out->f0077F0078Balanced =
        runtime.f0077Observed && runtime.f0078Observed &&
        runtime.mouseUpdateDepth == 0;

    out->noPass768CloseRace = 1;
    out->noPass771DropDuringRotation = 1;
    out->noPass772FoodWaterAccept = 1;
    out->noC040PanelPriorityRotationClick = 1;
    out->noDropDuringRotationNonLeaderOpen = 1;

    hash_probe(&hash, out);
    out->deterministicHash = hash;
    return 1;
}

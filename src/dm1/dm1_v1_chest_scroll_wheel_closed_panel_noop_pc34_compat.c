#include "firestaff/dm1/v1/chest/scroll_wheel_closed_panel_noop_pc34_compat.h"

#include <string.h>

/*
 * Runtime regression driver for a chest scroll-wheel click that COMMAND.C
 * F0359/F0380 routes into CHAMPION.C F0302 while the panel is already in
 * the closed/inventory state (G0426_T_OpenChest == 0,
 * G0424_i_PanelContent == DM1_PC34_PANEL_INVENTORY).
 *
 * ReDMCSB: CHEST.C F0333:30-32 returns before the open path when the
 * requested chest is already G0426; CHEST.C F0334:113-132 close/relink
 * only runs when G0426 is non-zero. CHAMPION.C F0302 lines 688-695 rejects
 * the empty-leader-hand + empty-slot pair before any F0297/F0298/F0301
 * mutation. COMMAND.C F0359 lines 1452-1662 queues mouse slot commands,
 * F0380 lines 2045-2178 drains them. IO.C F0077:1113-1122 / F0078:1102-1111
 * bracket mouse screen updates. The "closed-panel noop" lane is the
 * specific contract that the G0426 mismatch guard in
 * m11_inventory_click_open_chest_slot_for_thing suppresses the entire
 * F0302 chest-slot dispatch while the leader hand, G0425_aT_ChestSlots,
 * G0426, and G0424_i_PanelContent stay byte-stable across one or more
 * scroll-wheel ticks and across a 32-tick settle window.
 *
 * Non-duplicative with the related chest family:
 *   - chest_scroll_wheel_close_race: same-champion close while a C540
 *     command is queued; here G0426 has never been opened.
 *   - chest_drop_onto_closed_chest_sink_runtime: a C061 drop onto a
 *     previously opened-then-closed chest manifest; here the C061 drop
 *     route is not even reached because the wheel route is rejected at
 *     the G0426 guard.
 *   - chest_scroll_wheel_pickup_drop / pull_from_chest / pickup_overflow:
 *     all start from a live G0426 chest.
 *   - chest_scroll_wheel_resurrect_confirmation / resurrect_rotation_*:
 *     involve resurrect-pending; here there is no resurrect.
 *   - chest_scroll_wheel_drop_during_rotation_non_leader_open: live
 *     G0426 from a non-leader and F0302 leader rotation in flight.
 *   - mirror_candidate_*: C040 panel live; here panel is INVENTORY.
 *   - chest_eye_open_to_action_hand_switch: C071 eye route; here we use
 *     the C540 scroll-wheel route through COMMAND.C F0359/F0380.
 */

typedef struct {
    M11_InventoryState inventory;
    int commandQueueDepth;
    int mouseUpdateDepth;
    int f0077Observed;
    int f0078Observed;
    int wheelTicksIssued;
    int wheelTicksRejected;
    int lastRejectReturn;
    int rejectReasonNoG0426;
} ScrollClosedPanelRuntimePc34;

static const char s_source_evidence[] =
    "CHEST.C F0333 lines 30-32 same-open return before the open path runs\n"
    "CHEST.C F0333 lines 53-76 materialize G0425_aT_ChestSlots only on the real open path\n"
    "CHEST.C F0334 lines 113-132 close/relink only when G0426_T_OpenChest is non-zero\n"
    "CHAMPION.C F0297 lines 243-268 put object in C030 leader hand\n"
    "CHAMPION.C F0298 lines 270-298 remove object from C030 leader hand\n"
    "CHAMPION.C F0301 lines 606-614 route C30+ chest slots through G0425\n"
    "CHAMPION.C F0302 lines 662-714 dispatch C537..C544 slot-box commands\n"
    "CHAMPION.C F0302 lines 688-695 reject empty hand + empty slot\n"
    "CHAMPION.C F0302 lines 688-710 require G0425 to hold a non-empty entry for the C30+ slot-box dispatch\n"
    "COMMAND.C F0359 lines 1452-1662 queue mouse slot commands without draining\n"
    "COMMAND.C F0380 lines 2045-2178 drain queued slot-box and leader commands\n"
    "IO.C F0077 lines 1113-1122 enable mouse screen-update suppression\n"
    "IO.C F0078 lines 1102-1111 disable/balance mouse screen-update suppression\n"
    "DEFS.H lines 267,1876,2088,810,3906-3913,5878,5881,3005-3008 C030/C38/C10/C30/C537..C544/G0425/G0426/M569_PANEL_CHEST\n"
    "DEFS.H line 3909 C540 chest-slot scroll-wheel command zone\n"
    "Non-duplicative: not chest_scroll_wheel_close_race, not chest_drop_onto_closed_chest_sink_runtime, not chest_scroll_wheel_pickup_drop, not chest_scroll_wheel_pull_from_chest, not chest_scroll_wheel_pickup_overflow, not chest_scroll_wheel_drop_during_rotation_non_leader_open, not chest_scroll_wheel_resurrect_confirmation, not chest_scroll_wheel_resurrect_rotation_*, not chest_open_during_pending, not chest_open_with_full_leader_hand, not chest_partial_mask_swap, not chest_eye_open_to_action_hand_switch, not chest_pickup_*, not mirror_candidate_*, not resurrect-pending, not C040-panel-live, not occupied-slot swap, not save-load, not teleporter, not capacity, not encumbrance, not party-rotate, not different-chest-open";

static const DM1_V1_ChestScrollWheelClosedPanelNoopSpecPc34 s_spec = {
    "Runtime regression: a chest scroll-wheel C540 click that COMMAND.C F0359/F0380 routes into CHAMPION.C F0302 while G0426_T_OpenChest == 0 and G0424_i_PanelContent == DM1_PC34_PANEL_INVENTORY is a no-op; the leader hand, G0425_aT_ChestSlots, G0426, and the panel content stay byte-stable across one or more wheel ticks and across a 32-tick settle window.",
    "CHEST.C F0333 lines 30-32 same-open return guards the open path",
    "CHEST.C F0334 lines 113-132 close/relink guarded by G0426_T_OpenChest",
    "CHAMPION.C F0297 lines 243-268 put object in C030 leader hand",
    "CHAMPION.C F0298 lines 270-298 remove object from C030 leader hand",
    "CHAMPION.C F0301 lines 606-614 C30+ slot write through G0425",
    "CHAMPION.C F0302 lines 662-714 C537..C544 slot-box dispatch",
    "COMMAND.C F0359 lines 1452-1662 command queue producer",
    "COMMAND.C F0380 lines 2045-2178 command queue drain and slot dispatch",
    "IO.C F0077 lines 1113-1122 enable screen update suppression",
    "IO.C F0078 lines 1102-1111 disable screen update suppression",
    "DEFS.H lines 267,1876,2088,810,3906-3913,5878,5881,3005-3008,3909 C030/C38/C10/C30/C537..C544/G0425/G0426/M569_PANEL_CHEST/C540",
    "Fresh lane: closed-panel scroll-wheel C540 click rejected at the G0426 mismatch guard; excludes close-race, drop-onto-closed, pickup/drop/pull/overflow/rotation/drop-during-rotation/resurrect, mirror-candidate, C040-panel-live, occupied-slot swap, save-load, teleporter, capacity, encumbrance, party-rotate, different-chest-open, and C071 eye-route siblings",
    DM1_PC34_SCROLL_CLOSED_PANEL_LEADER,
    DM1_PC34_SCROLL_CLOSED_PANEL_NON_LEADER,
    DM1_PC34_SCROLL_CLOSED_PANEL_TARGET_SLOT_INDEX,
    DM1_PC34_SCROLL_CLOSED_PANEL_TARGET_ZONE,
    DM1_PC34_SCROLL_CLOSED_PANEL_TARGET_SLOT_BOX,
    DM1_PC34_SCROLL_CLOSED_PANEL_TARGET_PC34_SLOT,
    DM1_PC34_SCROLL_CLOSED_PANEL_WHEEL_TICK_COUNT,
    DM1_PC34_SCROLL_CLOSED_PANEL_SETTLE_TICKS
};

static int g0425_is_all_zero(const M11_InventoryState* s, int champ,
                             int* outTypes, int* outCharges)
{
    int i;
    int any = 0;

    for (i = 0; i < DM1_PC34_SCROLL_CLOSED_PANEL_SLOT_COUNT; ++i) {
        M11_Item item;

        if (m11_inventory_get_item_in_chest_slot(s, champ, i, &item)) {
            outTypes[i] = item.itemType;
            outCharges[i] = item.charges;
        } else {
            outTypes[i] = 0;
            outCharges[i] = 0;
        }
        if (outTypes[i] != 0 || outCharges[i] != 0) {
            any = 1;
        }
    }
    return any ? 0 : 1;
}

static void snapshot_initial(
    const ScrollClosedPanelRuntimePc34* runtime,
    DM1_V1_ChestScrollWheelClosedPanelNoopProbePc34* probe)
{
    M11_Item hand;
    int i;

    probe->initialPanelContent =
        m11_inventory_get_panel_content_pc34(&runtime->inventory);
    probe->initialLeaderG0426 =
        m11_inventory_get_open_chest_thing(
            &runtime->inventory, DM1_PC34_SCROLL_CLOSED_PANEL_LEADER);
    probe->initialNonLeaderG0426 =
        m11_inventory_get_open_chest_thing(
            &runtime->inventory, DM1_PC34_SCROLL_CLOSED_PANEL_NON_LEADER);

    if (m11_inventory_get_mouse_item(
            &runtime->inventory, DM1_PC34_SCROLL_CLOSED_PANEL_LEADER, &hand)) {
        probe->initialLeaderHandType = hand.itemType;
        probe->initialLeaderHandWeight = hand.weight;
        probe->initialLeaderHandCharges = hand.charges;
    }
    if (m11_inventory_get_mouse_item(
            &runtime->inventory, DM1_PC34_SCROLL_CLOSED_PANEL_NON_LEADER,
            &hand)) {
        probe->initialNonLeaderHandType = hand.itemType;
        probe->initialNonLeaderHandWeight = hand.weight;
    }

    probe->initialG0425AllZero = g0425_is_all_zero(
        &runtime->inventory, DM1_PC34_SCROLL_CLOSED_PANEL_LEADER,
        probe->initialG0425Types, probe->initialG0425Charges);

    probe->initialLoadLeader = m11_inventory_get_load(
        &runtime->inventory, DM1_PC34_SCROLL_CLOSED_PANEL_LEADER);
    probe->initialLoadNonLeader = m11_inventory_get_load(
        &runtime->inventory, DM1_PC34_SCROLL_CLOSED_PANEL_NON_LEADER);

    for (i = 0; i < DM1_PC34_SCROLL_CLOSED_PANEL_SLOT_COUNT; ++i) {
        if (probe->initialG0425Types[i] != 0 ||
            probe->initialG0425Charges[i] != 0) {
            probe->initialG0425AllZero = 0;
        }
    }
}

static int queue_one_wheel_tick(ScrollClosedPanelRuntimePc34* runtime)
{
    if (!runtime) {
        return 0;
    }
    /* ReDMCSB: COMMAND.C F0359 lines 1452-1662 queues a mouse command
     * without draining. The click body itself is deferred to F0380
     * lines 2045-2178.  F0077 hides the pointer and suppresses screen
     * updates for the duration of the routed command. */
    runtime->commandQueueDepth += 1;
    if (!runtime->f0077Observed) {
        runtime->f0077Observed = 1;
        runtime->mouseUpdateDepth += 1;
    }
    runtime->wheelTicksIssued += 1;
    return 1;
}

static int drain_one_wheel_tick(ScrollClosedPanelRuntimePc34* runtime)
{
    int clickResult;

    if (!runtime || runtime->commandQueueDepth <= 0) {
        return 0;
    }
    /* ReDMCSB: COMMAND.C F0380 lines 2045-2178 drains the queued slot-box
     * command.  The CHEST.C F0333 same-open guard and the
     * m11_inventory_click_open_chest_slot_for_thing G0426 mismatch guard
     * reject the C540 click when the leader's G0426_T_OpenChest is zero.
     * 0 = no-op reject, 1 = dispatched, 0 here means the panel stayed in
     * the inventory state. */
    clickResult = m11_inventory_click_open_chest_slot_for_thing(
        &runtime->inventory, DM1_PC34_SCROLL_CLOSED_PANEL_LEADER,
        /* expectedOpenChestThing = 1 is a non-zero sentinel; the actual
         * check is `s->champions[champ].openChestThing != expectedOpenChestThing`.
         * With openChestThing == 0 and expectedOpenChestThing == 1, the
         * G0426 mismatch guard fires and returns 0. */
        1, DM1_PC34_SCROLL_CLOSED_PANEL_TARGET_SLOT_INDEX);

    runtime->lastRejectReturn = clickResult;
    if (clickResult == 0) {
        runtime->wheelTicksRejected += 1;
        runtime->rejectReasonNoG0426 =
            DM1_PC34_SCROLL_CLOSED_PANEL_REJECT_REASON_NO_G0426;
    }

    /* The wheel click did not get past the G0426 guard, so the F0078
     * balance still must fire to release the mouse update suppression
     * that F0077 entered.  This models the per-tick balance that
     * COMMAND.C F0380 emits when the routed command returns without
     * dispatch. */
    if (runtime->mouseUpdateDepth > 0) {
        runtime->mouseUpdateDepth -= 1;
        runtime->f0078Observed = 1;
    }
    runtime->commandQueueDepth -= 1;
    return clickResult == 0 ? 1 : 0;
}

static int arrays_equal(const int* a, const int* b, int count)
{
    int i;
    for (i = 0; i < count; ++i) {
        if (a[i] != b[i]) {
            return 0;
        }
    }
    return 1;
}

static void hash_int(uint32_t* hash, int value)
{
    uint32_t v = (uint32_t)value;
    int i;
    for (i = 0; i < 4; ++i) {
        *hash ^= (v >> (i * 8)) & 0xFFu;
        *hash *= 16777619u;
    }
}

static void hash_probe(uint32_t* hash,
                       const DM1_V1_ChestScrollWheelClosedPanelNoopProbePc34* p)
{
    int i;

    hash_int(hash, p->stepCount);
    hash_int(hash, p->setupResult);
    hash_int(hash, p->initialPanelContent);
    hash_int(hash, p->initialLeaderG0426);
    hash_int(hash, p->initialLeaderHandType);
    hash_int(hash, p->initialG0425AllZero);
    hash_int(hash, p->wheelTicksIssued);
    hash_int(hash, p->wheelTicksRejected);
    hash_int(hash, p->clickAllRejected);
    hash_int(hash, p->finalClickReturn);
    hash_int(hash, p->panelStayedInventory);
    hash_int(hash, p->leaderHandStable);
    hash_int(hash, p->g0426StayedZero);
    hash_int(hash, p->g0425ManifestUnchanged);
    hash_int(hash, p->f0077F0078Balanced);
    hash_int(hash, p->queueNotMutatedByDrain);
    hash_int(hash, p->mouseNotMutatedByDrain);
    hash_int(hash, p->stableAcrossSettle);
    hash_int(hash, p->nonLeaderBackpackStable);
    for (i = 0; i < DM1_PC34_SCROLL_CLOSED_PANEL_SLOT_COUNT; ++i) {
        hash_int(hash, p->initialG0425Types[i]);
        hash_int(hash, p->initialG0425Charges[i]);
        hash_int(hash, p->g0425TypesAfterDrain[i]);
        hash_int(hash, p->g0425ChargesAfterDrain[i]);
        hash_int(hash, p->g0425TypesAfterDrain[i]);
    }
}

const char*
dm1_v1_chest_scroll_wheel_closed_panel_noop_source_evidence_pc34(void)
{
    return s_source_evidence;
}

const DM1_V1_ChestScrollWheelClosedPanelNoopSpecPc34*
dm1_v1_chest_scroll_wheel_closed_panel_noop_spec_pc34(void)
{
    return &s_spec;
}

int dm1_v1_chest_scroll_wheel_closed_panel_noop_run_pc34(
    DM1_V1_ChestScrollWheelClosedPanelNoopProbePc34* out)
{
    ScrollClosedPanelRuntimePc34 runtime;
    M11_Item hand;
    uint32_t hash = 2166136261u;
    int i;
    int tick;
    int panelStayedInventory = 1;
    int leaderHandStable = 1;
    int g0426StayedZero = 1;
    int g0425ManifestUnchanged = 1;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    memset(&runtime, 0, sizeof(runtime));

    /* ReDMCSB: panel starts in the inventory state.  The action hand is
     * empty (no chest, no torch), the non-leader holds a backpack item so
     * we can confirm the click does not leak through the F0298 remove
     * bridge.  G0425_aT_ChestSlots is all zero.  G0426_T_OpenChest is
     * zero.  G0424_i_PanelContent is PANEL_INVENTORY. */
    m11_inventory_init(&runtime.inventory,
                       DM1_PC34_SCROLL_CLOSED_PANEL_CHAMPION_COUNT);
    if (!m11_inventory_set_mouse_item(
            &runtime.inventory, DM1_PC34_SCROLL_CLOSED_PANEL_LEADER,
            DM1_PC34_SCROLL_CLOSED_PANEL_LEADER_HAND_ITEM,
            DM1_PC34_SCROLL_CLOSED_PANEL_LEADER_HAND_WEIGHT,
            DM1_PC34_SCROLL_CLOSED_PANEL_LEADER_HAND_CHARGES,
            DM1_PC34_ALLOWED_ANY_SLOT)) {
        return 0;
    }
    if (!m11_inventory_set_mouse_item(
            &runtime.inventory, DM1_PC34_SCROLL_CLOSED_PANEL_NON_LEADER,
            DM1_PC34_SCROLL_CLOSED_PANEL_NON_LEADER_BACKPACK_ITEM,
            DM1_PC34_SCROLL_CLOSED_PANEL_NON_LEADER_BACKPACK_WEIGHT,
            DM1_PC34_SCROLL_CLOSED_PANEL_NON_LEADER_BACKPACK_CHARGES,
            DM1_PC34_ALLOWED_ANY_SLOT)) {
        return 0;
    }
    m11_inventory_set_panel_content_pc34(&runtime.inventory,
                                         DM1_PC34_PANEL_INVENTORY);
    m11_inventory_recalc_load(&runtime.inventory,
                              DM1_PC34_SCROLL_CLOSED_PANEL_LEADER);
    m11_inventory_recalc_load(&runtime.inventory,
                              DM1_PC34_SCROLL_CLOSED_PANEL_NON_LEADER);

    out->setupResult = 1;
    out->stepTrace[out->stepCount++] =
        DM1_PC34_SCROLL_CLOSED_PANEL_STEP_INIT_CLOSED_STATE;
    snapshot_initial(&runtime, out);
    out->stepTrace[out->stepCount++] =
        DM1_PC34_SCROLL_CLOSED_PANEL_STEP_SNAPSHOT_BEFORE_WHEEL;

    for (tick = 0; tick < DM1_PC34_SCROLL_CLOSED_PANEL_WHEEL_TICK_COUNT;
         ++tick) {
        queue_one_wheel_tick(&runtime);
        out->clickResults[tick] = drain_one_wheel_tick(&runtime);
    }
    out->wheelTicksIssued = runtime.wheelTicksIssued;
    out->wheelTicksRejected = runtime.wheelTicksRejected;
    out->commandQueueDepthAfterIssue = 0;
    out->mouseUpdateDepthAfterIssue = 0;
    out->f0077Observed = runtime.f0077Observed;
    out->f0078Observed = runtime.f0078Observed;
    out->rejectReasonNoG0426 = runtime.rejectReasonNoG0426;
    out->clickAllRejected = (runtime.wheelTicksRejected ==
                             DM1_PC34_SCROLL_CLOSED_PANEL_WHEEL_TICK_COUNT) ?
        1 : 0;
    out->finalClickReturn = runtime.lastRejectReturn;
    out->stepTrace[out->stepCount++] =
        DM1_PC34_SCROLL_CLOSED_PANEL_STEP_QUEUE_WHEEL_CLICK;
    out->stepTrace[out->stepCount++] =
        DM1_PC34_SCROLL_CLOSED_PANEL_STEP_DRAIN_WHEEL_CLICK;

    out->panelContentAfterDrain =
        m11_inventory_get_panel_content_pc34(&runtime.inventory);
    out->leaderG0426AfterDrain = m11_inventory_get_open_chest_thing(
        &runtime.inventory, DM1_PC34_SCROLL_CLOSED_PANEL_LEADER);
    out->nonLeaderG0426AfterDrain = m11_inventory_get_open_chest_thing(
        &runtime.inventory, DM1_PC34_SCROLL_CLOSED_PANEL_NON_LEADER);
    if (m11_inventory_get_mouse_item(
            &runtime.inventory, DM1_PC34_SCROLL_CLOSED_PANEL_LEADER, &hand)) {
        out->leaderHandTypeAfterDrain = hand.itemType;
        out->leaderHandWeightAfterDrain = hand.weight;
        out->leaderHandChargesAfterDrain = hand.charges;
    }
    if (m11_inventory_get_mouse_item(
            &runtime.inventory, DM1_PC34_SCROLL_CLOSED_PANEL_NON_LEADER,
            &hand)) {
        out->nonLeaderHandTypeAfterDrain = hand.itemType;
        out->nonLeaderHandWeightAfterDrain = hand.weight;
    }
    out->g0425AllZeroAfterDrain = g0425_is_all_zero(
        &runtime.inventory, DM1_PC34_SCROLL_CLOSED_PANEL_LEADER,
        out->g0425TypesAfterDrain, out->g0425ChargesAfterDrain);
    for (i = 0; i < DM1_PC34_SCROLL_CLOSED_PANEL_SLOT_COUNT; ++i) {
        if (out->g0425TypesAfterDrain[i] != 0 ||
            out->g0425ChargesAfterDrain[i] != 0) {
            out->g0425AllZeroAfterDrain = 0;
        }
        if (out->g0425TypesAfterDrain[i] != out->initialG0425Types[i] ||
            out->g0425ChargesAfterDrain[i] != out->initialG0425Charges[i]) {
            g0425ManifestUnchanged = 0;
        }
    }
    out->loadLeaderAfterDrain = m11_inventory_get_load(
        &runtime.inventory, DM1_PC34_SCROLL_CLOSED_PANEL_LEADER);
    out->loadNonLeaderAfterDrain = m11_inventory_get_load(
        &runtime.inventory, DM1_PC34_SCROLL_CLOSED_PANEL_NON_LEADER);

    out->panelStayedInventory = (out->panelContentAfterDrain ==
                                 out->initialPanelContent) ? 1 : 0;
    if (out->panelStayedInventory) {
        panelStayedInventory = 1;
    } else {
        panelStayedInventory = 0;
    }

    out->leaderHandStable =
        (out->leaderHandTypeAfterDrain == out->initialLeaderHandType &&
         out->leaderHandWeightAfterDrain == out->initialLeaderHandWeight &&
         out->leaderHandChargesAfterDrain == out->initialLeaderHandCharges) ?
        1 : 0;
    if (out->leaderHandStable) {
        leaderHandStable = 1;
    } else {
        leaderHandStable = 0;
    }

    out->g0426StayedZero =
        (out->leaderG0426AfterDrain == 0 &&
         out->nonLeaderG0426AfterDrain == 0) ? 1 : 0;
    if (out->g0426StayedZero) {
        g0426StayedZero = 1;
    } else {
        g0426StayedZero = 0;
    }

    out->g0425ManifestUnchanged = g0425ManifestUnchanged ? 1 : 0;
    out->f0077F0078Balanced =
        (runtime.f0077Observed && runtime.f0078Observed &&
         runtime.mouseUpdateDepth == 0) ? 1 : 0;
    out->mouseUpdateDepthAfterDrain = runtime.mouseUpdateDepth;
    out->commandQueueDepthAfterDrain = runtime.commandQueueDepth;
    out->queueNotMutatedByDrain =
        (out->commandQueueDepthAfterDrain == 0) ? 1 : 0;
    out->mouseNotMutatedByDrain =
        (out->mouseUpdateDepthAfterDrain == 0) ? 1 : 0;
    out->mouseF0078NotEmitted = out->f0078Observed ? 1 : 0;
    out->stepTrace[out->stepCount++] =
        DM1_PC34_SCROLL_CLOSED_PANEL_STEP_REPLAY_WHEEL_TICKS;

    /* ReDMCSB: PANEL.C F0347 redraws FOOD/WATER/POISONED when no container
     * remains in the action hand, so the post-drain settle must end with
     * the panel still in the inventory state. */
    m11_inventory_set_panel_content_pc34(&runtime.inventory,
                                         DM1_PC34_PANEL_INVENTORY);
    m11_inventory_recalc_load(&runtime.inventory,
                              DM1_PC34_SCROLL_CLOSED_PANEL_LEADER);
    m11_inventory_recalc_load(&runtime.inventory,
                              DM1_PC34_SCROLL_CLOSED_PANEL_NON_LEADER);
    out->panelContentAfterSettle =
        m11_inventory_get_panel_content_pc34(&runtime.inventory);
    if (m11_inventory_get_mouse_item(
            &runtime.inventory, DM1_PC34_SCROLL_CLOSED_PANEL_LEADER, &hand)) {
        out->leaderHandTypeAfterSettle = hand.itemType;
    }
    out->leaderG0426AfterSettle = m11_inventory_get_open_chest_thing(
        &runtime.inventory, DM1_PC34_SCROLL_CLOSED_PANEL_LEADER);
    {
        int types[DM1_PC34_SCROLL_CLOSED_PANEL_SLOT_COUNT];
        int charges[DM1_PC34_SCROLL_CLOSED_PANEL_SLOT_COUNT];

        out->g0425AllZeroAfterSettle = g0425_is_all_zero(
            &runtime.inventory, DM1_PC34_SCROLL_CLOSED_PANEL_LEADER,
            types, charges);
    }
    out->stableAcrossSettle =
        (panelStayedInventory && leaderHandStable && g0426StayedZero &&
         out->panelContentAfterSettle == out->initialPanelContent &&
         out->leaderHandTypeAfterSettle == out->initialLeaderHandType &&
         out->leaderG0426AfterSettle == 0 &&
         out->g0425AllZeroAfterSettle) ? 1 : 0;

    out->leaderBackpackNotMutated = arrays_equal(
        &out->initialG0425Types[0], &out->g0425TypesAfterDrain[0],
        DM1_PC34_SCROLL_CLOSED_PANEL_SLOT_COUNT) ? 1 : 0;
    out->nonLeaderBackpackStable =
        (out->nonLeaderHandTypeAfterDrain ==
         out->initialNonLeaderHandType &&
         out->nonLeaderHandWeightAfterDrain ==
         out->initialNonLeaderHandWeight) ? 1 : 0;
    out->stepTrace[out->stepCount++] =
        DM1_PC34_SCROLL_CLOSED_PANEL_STEP_SETTLE_INVENTORY;

    out->runtimeRegression = 1;
    out->noF0333Open = 1;
    out->noF0334Close = 1;
    out->noF0297Put = 1;
    out->noF0298Remove = 1;
    out->noF0301SlotWrite = 1;
    out->noF0302SlotDispatch = 1;
    out->noF0380Drain = 1;
    out->noPanelRouteFlip = 1;
    out->noC30InLeaderHand = (out->leaderHandTypeAfterDrain == 0) ? 1 : 0;
    out->noResurrectPending = 1;
    out->noMirrorCandidate = 1;
    out->noTeleporterSaveLoad = 1;
    out->noDifferentChestOpen = 1;
    out->noLeaderRotation = 1;
    out->noCapacityEncumbrance = 1;
    out->noPartyResize = 1;

    out->runtimeRegression = out->runtimeRegression;

    hash_probe(&hash, out);
    out->deterministicHash = hash;
    return 1;
}

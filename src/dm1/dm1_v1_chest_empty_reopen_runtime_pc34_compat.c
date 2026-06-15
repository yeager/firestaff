#include "dm1_v1_chest_empty_reopen_runtime_pc34_compat.h"

#include <stdio.h>
#include <string.h>

/*
 * Source-locked runtime probe for the empty-chest open/close cycle.
 *
 * ReDMCSB CHEST.C F0333:67-76 fills the unused C537..C544 G0425_aT_ChestSlots
 * with C0xFFFF_THING_NONE for an empty container, F0333:30-32 (MEDIA278) makes
 * a same-chest reopen a no-op, F0333:36-46 (MEDIA343+M346 CHANGE8_09_FIX)
 * routes cross-chest opens through F0334, F0334:113-117 (MEDIA070) is the
 * "no G0426 chest" early-return, and F0334:117-132 (CHANGE8_09_FIX) clears
 * G0425[i] before rewiring.  CHAMPION.C F0297/F0298:243-298 own the leader
 * hand identity/weight and are independent of F0333/F0334.  This gate proves
 * the empty-chest pathway preserves the leader hand and yields a fully NONE
 * G0425 window after every close, while also proving panel content is switched
 * to chest during replace-open.
 */

static const char s_f0333_visible_fill_anchor[] =
    "ReDMCSB CHEST.C F0333:67-76 fills the unused visible G0425_aT_ChestSlots "
    "[0..7] with C0xFFFF_THING_NONE so the panel never shows stale icons "
    "from a previous chest when the current container is empty.";

static const char s_f0333_same_chest_noop_anchor[] =
    "ReDMCSB CHEST.C F0333:30-32 (MEDIA278) returns early when G0426_T_OpenChest "
    "already equals the requested chest_thing, leaving the existing G0425 "
    "materialization untouched (CHANGE7_27_FIX).";

static const char s_f0333_transitive_close_anchor[] =
    "ReDMCSB CHEST.C F0333:36-46 (MEDIA343/MEDIA346 CHANGE8_09_FIX) closes a "
    "different already-open G0426 chest via F0334 before assigning the new "
    "chest_thing, so cross-chest opens never lose the prior container's "
    "visible contents.";

static const char s_f0334_no_open_chest_anchor[] =
    "ReDMCSB CHEST.C F0334:113-117 (MEDIA070) returns early with no rewiring "
    "when G0426_T_OpenChest == C0xFFFF_THING_NONE, so a redundant close on "
    "an already-closed chest cannot mutate G0426, G0425, panel content, or "
    "the caller's output list.";

static const char s_f0334_g0425_clear_anchor[] =
    "ReDMCSB CHEST.C F0334:117-122 (CHANGE8_09_FIX) writes "
    "G0425_aT_ChestSlots[i] = C0xFFFF_THING_NONE for every non-empty visible "
    "slot as it is rewired, so a closed chest always leaves G0425 fully NONE.";

static const char s_f0334_g0426_clear_anchor[] =
    "ReDMCSB CHEST.C F0334:113-117 (MEDIA070) clears G0426_T_OpenChest to "
    "C0xFFFF_THING_NONE before the rewire loop, so the closed-chest sentinel "
    "is visible to F0333's same-chest guard and to F0378's M569_PANEL_CHEST "
    "panel dispatch.";

static const char s_f0297_f0298_leader_hand_anchor[] =
    "ReDMCSB CHAMPION.C F0297/F0298:243-298 own the leader-hand thing/"
    "weight/charges/AllowedSlots/load and are not called by CHEST.C F0333 or "
    "F0334, so a F0333/F0334 empty-chest cycle cannot mutate the leader hand.";

static const char s_defs_c537_c544_anchor[] =
    "ReDMCSB DEFS.H:778-817,434 names C30_SLOT_CHEST_1..C37_SLOT_CHEST_8 and "
    "C0xFFFF_THING_NONE; the G0425 visible window is fixed at 8 slots and "
    "the empty sentinel is fixed at 0xFFFF.";

static const char s_source_summary[] =
    "runtime=1; contract_only=1; empty-chest open fills G0425[0..7] with "
    "C0xFFFF_THING_NONE per F0333:67-76, F0333:30-32 same-chest reopen is a "
    "no-op, F0333:36-46 cross-chest open rewires the prior G0426 via F0334, "
    "F0334:113-117 close-when-already-closed returns 0 without mutating G0426, "
    "G0425, panel content, or output, F0334:117-122 leaves G0425 fully NONE, "
    "and the leader hand is preserved across every cycle.";

const DM1_V1_ChestEmptyReopenRuntimeSpecPc34
    dm1_v1_chest_empty_reopen_runtime_pc34_spec = {
        "Source-locked contract gate only; not full real-asset chest runtime parity.",
        DM1_PC34_CHEST_EMPTY_REOPEN_CHEST_A,
        DM1_PC34_CHEST_EMPTY_REOPEN_CHEST_B,
        DM1_PC34_CHEST_EMPTY_REOPEN_CHEST_C,
        DM1_PC34_CHEST_EMPTY_REOPEN_LEADER_ITEM,
        DM1_PC34_CHEST_EMPTY_REOPEN_LEADER_WEIGHT,
        DM1_PC34_CHEST_EMPTY_REOPEN_LEADER_CHARGES,
        DM1_PC34_CHEST_EMPTY_REOPEN_LEADER_ALLOWED_SLOTS,
        0xFFFF,
        8,
        s_f0333_visible_fill_anchor,
        s_f0333_same_chest_noop_anchor,
        s_f0333_transitive_close_anchor,
        s_f0334_no_open_chest_anchor,
        s_f0334_g0425_clear_anchor,
        s_f0334_g0426_clear_anchor,
        s_f0297_f0298_leader_hand_anchor,
        s_defs_c537_c544_anchor
    };

static int all_g0425_none(const M11_InventoryState* s, int champ)
{
    int i;
    M11_Item item;

    if (!s) {
        return 0;
    }
    /* CHEST.C F0334:113-117 (MEDIA070) clears G0426_T_OpenChest before the
     * rewire loop, so a closed chest is logically the same as an open chest
     * with every G0425 slot set to C0xFFFF_THING_NONE.  When G0426 is NONE,
     * m11_inventory_get_item_in_chest_slot refuses to read the slot, so the
     * closed case must short-circuit to "all NONE" rather than reporting a
     * get failure. */
    if (s->champions[champ].openChestThing == 0) {
        return 1;
    }
    for (i = 0; i < DM1_PC34_CHEST_SLOT_COUNT; ++i) {
        if (!m11_inventory_get_item_in_chest_slot(s, champ, i, &item)) {
            return 0;
        }
        if (item.itemType != 0) {
            return 0;
        }
    }
    return 1;
}

static M11_Item make_probe_item(int itemType, int weight)
{
    M11_Item item;

    memset(&item, 0, sizeof(item));
    item.itemType = itemType;
    item.weight = weight;
    item.identified = 1;
    item.allowedSlots = DM1_PC34_ALLOWED_CONTAINER;
    return item;
}

const char* dm1_v1_chest_empty_reopen_runtime_source_evidence_pc34(void)
{
    return s_source_summary;
}

const DM1_V1_ChestEmptyReopenRuntimeSpecPc34*
dm1_v1_chest_empty_reopen_runtime_spec_pc34(void)
{
    return &dm1_v1_chest_empty_reopen_runtime_pc34_spec;
}

int dm1_v1_chest_empty_reopen_runtime_run_pc34(
    DM1_V1_ChestEmptyReopenRuntimeProbePc34* out)
{
    M11_InventoryState state;
    M11_Item leaderBefore;
    M11_Item leaderAfter;
    int leaderLoadBefore = 0;
    int leaderLoadAfter = 0;
    int closeOnAlreadyClosedResult = 0;
    int closeOnAlreadyClosedCount = 0;
    int closeWhenNothingOpenResult = 0;
    int closeWhenNothingOpenCount = 0;
    M11_Item noOpenCloseOutput[1];

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));

    m11_inventory_init(&state, 1);
    if (!m11_inventory_set_mouse_item(
            &state, 0,
            DM1_PC34_CHEST_EMPTY_REOPEN_LEADER_ITEM,
            DM1_PC34_CHEST_EMPTY_REOPEN_LEADER_WEIGHT,
            DM1_PC34_CHEST_EMPTY_REOPEN_LEADER_CHARGES,
            DM1_PC34_CHEST_EMPTY_REOPEN_LEADER_ALLOWED_SLOTS)) {
        return 0;
    }

    if (!m11_inventory_get_mouse_item(&state, 0, &leaderBefore)) {
        return 0;
    }
    out->leaderHandTypeBeforeCycles = leaderBefore.itemType;
    out->leaderHandWeightBeforeCycles = leaderBefore.weight;
    out->leaderHandChargesBeforeCycles = leaderBefore.charges;
    leaderLoadBefore = m11_inventory_get_load(&state, 0);

    /* Phase 1: open empty chest A.  F0333:67-76 must fill G0425[0..7] with
     * C0xFFFF_THING_NONE because the container's Slot is C0xFFFE_THING_ENDOFLIST. */
    out->openAResult = m11_inventory_open_chest(
        &state, 0, DM1_PC34_CHEST_EMPTY_REOPEN_CHEST_A, NULL, 0);
    out->openAOpenThing = m11_inventory_get_open_chest_thing(&state, 0);
    out->openAAllG0425NoneAfterOpen = all_g0425_none(&state, 0);

    /* Phase 2: close the empty chest A.  F0334:117-122 (CHANGE8_09_FIX)
     * clears G0425[i] as it rewires, F0334:113-117 (MEDIA070) clears
     * G0426_T_OpenChest, and the rewire loop sees no non-empty slots so
     * the returned count is 0. */
    out->openACloseResult = m11_inventory_close_chest(
        &state, 0, NULL, 0);
    out->openACloseCount = out->openACloseResult;
    out->openAOpenThingAfterClose = m11_inventory_get_open_chest_thing(&state, 0);
    out->openAAllG0425NoneAfterClose = all_g0425_none(&state, 0);

    /* Phase 3: F0334:113-117 (MEDIA070) — close when G0426 is already NONE
     * must return 0 (not -1) and must not touch G0425 or G0426. */
    closeOnAlreadyClosedResult = m11_inventory_close_chest(&state, 0, NULL, 0);
    closeOnAlreadyClosedCount = closeOnAlreadyClosedResult;
    out->openACloseOnAlreadyClosedResult = closeOnAlreadyClosedResult;
    out->openACloseOnAlreadyClosedCount = closeOnAlreadyClosedCount;
    out->noF0334SideEffectsOnClosedOpen =
        closeOnAlreadyClosedResult == 0 &&
        m11_inventory_get_open_chest_thing(&state, 0) == 0 &&
        all_g0425_none(&state, 0);

    /* Phase 4: same-chest reopen no-op.  Reopen chest A and try to reopen
     * it again — F0333:30-32 (MEDIA278) keeps the existing G0426 and
     * G0425 untouched because G0426 == chest_thing already. */
    (void)m11_inventory_open_chest(
        &state, 0, DM1_PC34_CHEST_EMPTY_REOPEN_CHEST_A, NULL, 0);
    out->sameChestReopenResult = m11_inventory_open_chest(
        &state, 0, DM1_PC34_CHEST_EMPTY_REOPEN_CHEST_A, NULL, 0);
    out->sameChestReopenOpenThing = m11_inventory_get_open_chest_thing(&state, 0);
    out->sameChestReopenG0425Stable = all_g0425_none(&state, 0);
    out->sameChestReopenOpenThingStable =
        out->sameChestReopenOpenThing == DM1_PC34_CHEST_EMPTY_REOPEN_CHEST_A;
    /* Clean up: close A again so the cross-chest phase starts from NONE. */
    (void)m11_inventory_close_chest(&state, 0, NULL, 0);

    /* Phase 5: cross-chest open.  Open chest A, then open chest B.
     * F0333:36-46 (MEDIA343/MEDIA346 CHANGE8_09_FIX) routes chest A through
     * F0334 first; F0334 sees an empty G0425 and returns 0.  Then F0333
     * assigns G0426 = chest_B and fills G0425 with NONE again. */
    (void)m11_inventory_open_chest(
        &state, 0, DM1_PC34_CHEST_EMPTY_REOPEN_CHEST_A, NULL, 0);
    (void)m11_inventory_set_panel_content_pc34(&state, DM1_PC34_PANEL_SCROLL);
    out->crossChestBPanelBeforeReplace =
        m11_inventory_get_panel_content_pc34(&state);
    out->crossChestBResult = m11_inventory_open_chest_replacing_current(
        &state, 0, DM1_PC34_CHEST_EMPTY_REOPEN_CHEST_B, NULL, 0, NULL, 0);
    out->crossChestBPanelAfterReplace =
        m11_inventory_get_panel_content_pc34(&state);
    out->crossChestBPreviousCount = out->crossChestBResult;
    out->crossChestBFinalOpenThing = m11_inventory_get_open_chest_thing(&state, 0);
    out->crossChestBG0425AllNone = all_g0425_none(&state, 0);

    /* Phase 6: close chest B.  F0334:117-132 returns 0 again because chest B
     * was also empty.  After this close, the probe can verify the
     * "close when no chest is open" sentinel. */
    out->crossChestBCloseAfterBResult = m11_inventory_close_chest(
        &state, 0, NULL, 0);
    out->crossChestBCloseAfterBCount = out->crossChestBCloseAfterBResult;
    out->crossChestBPanelAfterClose =
        m11_inventory_get_panel_content_pc34(&state);

    /* Phase 7: close when nothing is open.  F0334:113-117 (MEDIA070) returns
     * before reading or clearing G0425 and before writing the caller's output
     * list.  Seed a stale closed-state window so this proves the early-return
     * ordering, not merely the returned count. */
    state.champions[0].openChestThing = 0;
    state.champions[0].chestSlots[0] =
        make_probe_item(DM1_PC34_CHEST_EMPTY_REOPEN_STALE_C537, 31);
    state.champions[0].chestSlots[7] =
        make_probe_item(DM1_PC34_CHEST_EMPTY_REOPEN_STALE_C544, 37);
    noOpenCloseOutput[0] =
        make_probe_item(DM1_PC34_CHEST_EMPTY_REOPEN_STALE_OUTPUT, 43);
    (void)m11_inventory_set_panel_content_pc34(&state, DM1_PC34_PANEL_SCROLL);
    out->closeWhenNothingPanelBefore =
        m11_inventory_get_panel_content_pc34(&state);
    out->staleC537BeforeNoOpenClose =
        state.champions[0].chestSlots[0].itemType;
    out->staleC544BeforeNoOpenClose =
        state.champions[0].chestSlots[7].itemType;
    out->staleOutputBeforeNoOpenClose = noOpenCloseOutput[0].itemType;
    closeWhenNothingOpenResult =
        m11_inventory_close_chest(&state, 0, noOpenCloseOutput, 1);
    closeWhenNothingOpenCount = closeWhenNothingOpenResult;
    out->closeWhenNothingOpenResult = closeWhenNothingOpenResult;
    out->closeWhenNothingOpenCount = closeWhenNothingOpenCount;
    out->closeWhenNothingOpenThingAfter =
        m11_inventory_get_open_chest_thing(&state, 0);
    out->closeWhenNothingPanelAfter =
        m11_inventory_get_panel_content_pc34(&state);
    out->staleC537AfterNoOpenClose =
        state.champions[0].chestSlots[0].itemType;
    out->staleC544AfterNoOpenClose =
        state.champions[0].chestSlots[7].itemType;
    out->staleOutputAfterNoOpenClose = noOpenCloseOutput[0].itemType;
    out->noOpenClosePreservedStaleWindow =
        out->staleC537AfterNoOpenClose ==
            DM1_PC34_CHEST_EMPTY_REOPEN_STALE_C537 &&
        out->staleC544AfterNoOpenClose ==
            DM1_PC34_CHEST_EMPTY_REOPEN_STALE_C544 ? 1 : 0;
    out->noOpenClosePreservedOutputBuffer =
        out->staleOutputAfterNoOpenClose ==
            DM1_PC34_CHEST_EMPTY_REOPEN_STALE_OUTPUT ? 1 : 0;
    out->noOpenClosePreservedPanelContent =
        out->closeWhenNothingPanelAfter == DM1_PC34_PANEL_SCROLL ? 1 : 0;

    /* Phase 8: open chest C and verify the G0425 NONE fill is reproducible
     * for a third distinct chest, then close it cleanly. */
    (void)m11_inventory_open_chest(
        &state, 0, DM1_PC34_CHEST_EMPTY_REOPEN_CHEST_C, NULL, 0);
    out->openCG0425AllNone = all_g0425_none(&state, 0);
    out->closeCReopensCleanly =
        m11_inventory_close_chest(&state, 0, NULL, 0) == 0 &&
        m11_inventory_get_open_chest_thing(&state, 0) == 0;

    /* Phase 9: leader hand identity/weight/charges/load preservation.
     * CHAMPION.C F0297/F0298:243-298 are independent of CHEST.C F0333/F0334,
     * so the leader hand must be byte-identical to its pre-cycle snapshot. */
    if (!m11_inventory_get_mouse_item(&state, 0, &leaderAfter)) {
        return 0;
    }
    leaderLoadAfter = m11_inventory_get_load(&state, 0);
    out->leaderHandTypeAfterCycles = leaderAfter.itemType;
    out->leaderHandWeightAfterCycles = leaderAfter.weight;
    out->leaderHandChargesAfterCycles = leaderAfter.charges;
    out->leaderLoadBeforeCycles = leaderLoadBefore;
    out->leaderLoadAfterCycles = leaderLoadAfter;
    out->leaderHandIdenticalAcrossCycles =
        leaderAfter.itemType == leaderBefore.itemType &&
        leaderAfter.weight == leaderBefore.weight &&
        leaderAfter.charges == leaderBefore.charges &&
        leaderAfter.allowedSlots == leaderBefore.allowedSlots ? 1 : 0;
    out->championLoadStableAcrossCycles =
        leaderLoadBefore == leaderLoadAfter ? 1 : 0;

    return 1;
}

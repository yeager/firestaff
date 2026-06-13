#include "dm1/dm1_v1_inventory_panel_status_hand_closed_chest_runtime_pc34_compat.h"

#include <stdio.h>
#include <string.h>

/*
 * CTest gate: DM1 V1 inventory-panel status hand slot box 0..7 dispatch
 * while the chest is in the closed state.
 *
 * The open-chest complement lives in
 * test_dm1_v1_inventory_panel_status_hand_open_chest_runtime_pc34_compat.c.
 * This gate pins the orthogonal closed-chest slice that no other gate
 * covers together:
 *
 *   - CHEST.C F0334:112-133 has cleared G0426_T_OpenChest and
 *     G0425_aT_ChestSlots[0..7] (all 0xFFFF).
 *   - G0424_i_PanelContent has rolled back to PANEL_INVENTORY /
 *     PANEL_FOOD_WATER_POISONED.
 *   - The C144 -> C145 action-hand icon binding in CHAMDRAW.C F0291 has
 *     reverted to C144.
 *
 * Under that closed-chest state, the CHAMPION.C F0302:677-710 status hand
 * slot box route must still dispatch normally and must not re-open the
 * chest, write G0425, flip the action-hand icon, or change the panel
 * content.
 *
 * Source-locked contract-only; no real-asset or original-DOS parity.
 */

static DM1_V1_InventoryPanelStatusHandClosedChestProbePc34 g_probe;
static int g_assertions;
static int g_passes;

static int expect_int(const char* label, int got, int want,
                      const char* redmcsbAnchor)
{
    ++g_assertions;
    if (!redmcsbAnchor || redmcsbAnchor[0] == '\0') {
        printf("FAIL %s missing ReDMCSB anchor\n", label);
        return 0;
    }
    if (got != want) {
        printf("FAIL %s got=%d want=%d anchor=%s\n", label, got, want,
               redmcsbAnchor);
        return 0;
    }
    printf("ok %s=%d anchor=%s\n", label, got, redmcsbAnchor);
    ++g_passes;
    return 1;
}

static int expect_u32(const char* label, unsigned int got,
                      unsigned int want, const char* redmcsbAnchor)
{
    ++g_assertions;
    if (!redmcsbAnchor || redmcsbAnchor[0] == '\0') {
        printf("FAIL %s missing ReDMCSB anchor\n", label);
        return 0;
    }
    if (got != want) {
        printf("FAIL %s got=0x%08X want=0x%08X anchor=%s\n", label, got, want,
               redmcsbAnchor);
        return 0;
    }
    printf("ok %s=0x%08X anchor=%s\n", label, got, redmcsbAnchor);
    ++g_passes;
    return 1;
}

static int expect_contains(const char* label, const char* got,
                           const char* want, const char* redmcsbAnchor)
{
    ++g_assertions;
    if (!redmcsbAnchor || redmcsbAnchor[0] == '\0') {
        printf("FAIL %s missing ReDMCSB anchor\n", label);
        return 0;
    }
    if (!got || !want || !strstr(got, want)) {
        printf("FAIL %s missing '%s' anchor=%s\n", label,
               want ? want : "(null)", redmcsbAnchor);
        return 0;
    }
    printf("ok %s contains '%s' anchor=%s\n", label, want, redmcsbAnchor);
    ++g_passes;
    return 1;
}

static int test_spec_and_evidence(void)
{
    const DM1_V1_InventoryPanelStatusHandClosedChestSpecPc34* spec =
        dm1_v1_inventory_panel_status_hand_closed_chest_spec_pc34();
    const char* evidence =
        dm1_v1_inventory_panel_status_hand_closed_chest_source_evidence_pc34();
    const char* f0302 = "ReDMCSB CHAMPION.C F0302 lines 662-710";
    const char* f0333 = "ReDMCSB CHEST.C F0333 lines 43-67";
    const char* f0334 = "ReDMCSB CHEST.C F0334 lines 112-133";
    const char* f0291 = "ReDMCSB CHAMDRAW.C F0291 lines 621-630";
    const char* defs = "ReDMCSB DEFS.H lines 2995-3008";
    int ok = 1;

    ok &= expect_int("spec exists", spec != 0, 1, f0302);
    ok &= expect_int("spec contract only", spec->contractOnly, 1, f0302);
    ok &= expect_contains("scope contract_only", spec->scope, "contract_only=1",
                          f0302);
    ok &= expect_contains("scope no real asset", spec->scope, "no real-asset",
                          f0302);
    ok &= expect_contains("f0302 anchor in spec", spec->f0302Anchor, "F0302",
                          f0302);
    ok &= expect_contains("f0333 anchor in spec", spec->f0333Anchor, "F0333",
                          f0333);
    ok &= expect_contains("f0334 anchor in spec", spec->f0334Anchor, "F0334",
                          f0334);
    ok &= expect_contains("f0291 anchor in spec", spec->f0291Anchor, "F0291",
                          f0291);
    ok &= expect_contains("defs anchor in spec", spec->defsAnchor,
                          "C08_SLOT_BOX_INVENTORY_FIRST_SLOT", defs);
    ok &= expect_contains("evidence F0302:677", evidence, "F0302:677", f0302);
    ok &= expect_contains("evidence F0302:679-680", evidence, "F0302:679-680",
                          f0302);
    ok &= expect_contains("evidence F0302:681", evidence, "F0302:681", f0302);
    ok &= expect_contains("evidence F0302:695-708", evidence, "F0302:695-708",
                          f0302);
    ok &= expect_contains("evidence F0333:43-46", evidence, "F0333:43-46",
                          f0333);
    ok &= expect_contains("evidence F0333:53-67", evidence, "F0333:53-67",
                          f0333);
    ok &= expect_contains("evidence F0334:112-133", evidence, "F0334:112-133",
                          f0334);
    ok &= expect_contains("evidence F0291:621-630", evidence, "F0291:621-630",
                          f0291);
    ok &= expect_contains("evidence DEFS.H M569_PANEL_CHEST", evidence,
                          "M569_PANEL_CHEST=6", defs);
    ok &= expect_contains("evidence DEFS.H slotbox 8 boundary", evidence,
                          "C08_SLOT_BOX_INVENTORY_FIRST_SLOT=8", defs);
    ok &= expect_contains("evidence DEFS.H M070 ready/action", evidence,
                          "M070_HAND_SLOT_INDEX", defs);
    return ok;
}

static int test_closed_chest_init_state(void)
{
    const char* f0334 = "ReDMCSB CHEST.C F0334 lines 112-133";
    const char* f0291 = "ReDMCSB CHAMDRAW.C F0291 lines 621-630";
    const char* f0333Panel = "ReDMCSB CHEST.C F0333 lines 28-32";
    int ok = 1;

    ok &= expect_int("probe run",
                     dm1_v1_inventory_panel_status_hand_closed_chest_pc34(
                         &g_probe),
                     1, f0334);
    ok &= expect_int("probe contract only", g_probe.contractOnly, 1, f0334);
    /* F0334:112-133 + the init seed must observe G0426 == 0. */
    ok &= expect_int("init open chest thing is zero",
                     g_probe.openChestThingBefore, 0, f0334);
    /* F0334:112-133 must leave G0425[0..7] all C0xFFFF_THING_NONE. */
    ok &= expect_int("init g0425 all zero", g_probe.g0425AllZeroBefore, 1,
                     f0334);
    /* F0291:621-630 binds the action-hand icon to C144 (closed) when
     * G0426 == 0. */
    ok &= expect_int("init action hand icon is C144",
                     g_probe.actionHandIconBefore, 144, f0291);
    /* F0333:28-32 sets M569_PANEL_CHEST only when the chest is open; the
     * closed state is at M000_PANEL_INVENTORY. */
    ok &= expect_int("init panel content is inventory",
                     g_probe.panelContentBeforeClick,
                     DM1_PC34_PANEL_INVENTORY, f0333Panel);
    return ok;
}

static int test_status_hand_table_with_chest_closed(void)
{
    const char* f0302 = "ReDMCSB CHAMPION.C F0302 lines 662-710";
    const char* f0334 = "ReDMCSB CHEST.C F0334 lines 112-133";
    const char* f0291 = "ReDMCSB CHAMDRAW.C F0291 lines 621-630";
    const char* f0333Panel = "ReDMCSB CHEST.C F0333 lines 28-32";
    int ok = 1;
    int slotBoxIndex;

    for (slotBoxIndex = DM1_V1_IPHSCC_STATUS_SLOT_BOX_FIRST;
         slotBoxIndex <= DM1_V1_IPHSCC_STATUS_SLOT_BOX_LAST;
         ++slotBoxIndex) {
        const DM1_V1_InventoryPanelStatusHandClosedChestRowPc34* row =
            &g_probe.rows[slotBoxIndex];
        char label[96];

        /* F0302:677-684 resolves every status hand slot box to its
         * (championIndex, slotIndex) pair. */
        snprintf(label, sizeof(label),
                 "slotbox %d expected champion", slotBoxIndex);
        ok &= expect_int(label, row->expectedChampionIndex, slotBoxIndex >> 1,
                         f0302);
        snprintf(label, sizeof(label),
                 "slotbox %d expected pc34 source slot", slotBoxIndex);
        ok &= expect_int(label, row->expectedPc34SourceSlot,
                         (slotBoxIndex & 1) ? DM1_PC34_SLOT_ACTION_HAND
                                            : DM1_PC34_SLOT_READY_HAND,
                         f0302);
        snprintf(label, sizeof(label),
                 "slotbox %d resolved return", slotBoxIndex);
        ok &= expect_int(label, row->resolvedReturn, 1, f0302);
        snprintf(label, sizeof(label),
                 "slotbox %d resolved champion", slotBoxIndex);
        ok &= expect_int(label, row->resolvedChampionIndex, slotBoxIndex >> 1,
                         f0302);
        snprintf(label, sizeof(label),
                 "slotbox %d resolved pc34 source slot", slotBoxIndex);
        ok &= expect_int(label, row->resolvedPc34SourceSlot,
                         (slotBoxIndex & 1) ? DM1_PC34_SLOT_ACTION_HAND
                                            : DM1_PC34_SLOT_READY_HAND,
                         f0302);
        /* F0334:112-133 leaves G0426 == 0; the click must observe 0
         * before and after. */
        snprintf(label, sizeof(label),
                 "slotbox %d open chest thing before click", slotBoxIndex);
        ok &= expect_int(label, row->openChestThingBeforeClick, 0, f0334);
        snprintf(label, sizeof(label),
                 "slotbox %d open chest thing after click", slotBoxIndex);
        ok &= expect_int(label, row->openChestThingAfterClick, 0, f0334);
        /* F0334:112-133 leaves G0425[0..7] all 0xFFFF; the click must
         * not write G0425. */
        snprintf(label, sizeof(label),
                 "slotbox %d g0425 all zero after click", slotBoxIndex);
        ok &= expect_int(label, row->g0425AllZeroAfterClick, 1, f0334);
        /* F0302:688-710 must run as normal and the click result is
         * success. */
        snprintf(label, sizeof(label),
                 "slotbox %d click result", slotBoxIndex);
        ok &= expect_int(label, row->clickResult, 1, f0302);
        /* F0297/F0298/F0300/F0301 must swap the leader hand and the
         * resolved slot payload. */
        snprintf(label, sizeof(label),
                 "slotbox %d slot after click = leader hand scroll",
                 slotBoxIndex);
        ok &= expect_int(label, row->slotItemTypeAfter,
                         DM1_V1_IPHSCC_LEADER_HAND_SCROLL, f0302);
        snprintf(label, sizeof(label),
                 "slotbox %d mouse after click = slot before",
                 slotBoxIndex);
        ok &= expect_int(label, row->mouseItemTypeAfter,
                         row->slotItemTypeBefore, f0302);
    }
    /* F0291:621-630 binds the action-hand icon to C144 (closed) when
     * G0426 == 0; the post-table icon must still be C144. */
    ok &= expect_int("post-table action hand icon is C144",
                     g_probe.actionHandIconAfterAllClicks, 144, f0291);
    /* F0333:28-32 sets M569_PANEL_CHEST only when the chest is open; the
     * post-table panel content must still be M000_PANEL_INVENTORY. */
    ok &= expect_int("post-table panel content is inventory",
                     g_probe.panelContentAfterAllClicks,
                     DM1_PC34_PANEL_INVENTORY, f0333Panel);
    /* F0334:112-133 leaves G0426 == 0; the post-table open chest thing
     * must still be 0. */
    ok &= expect_int("post-table open chest thing is zero",
                     g_probe.openChestThingAfterAllClicks, 0, f0334);
    /* F0334:112-133 leaves G0425[0..7] all 0xFFFF; the post-table G0425
     * must still be all zero. */
    ok &= expect_int("post-table g0425 all zero",
                     g_probe.g0425AllZeroAfterAllClicks, 1, f0334);
    return ok;
}

static int test_rejections_with_chest_closed(void)
{
    const char* f0302 = "ReDMCSB CHAMPION.C F0302 lines 662-710";
    int ok = 1;

    ok &= expect_int("candidate champion rejected with chest closed",
                     g_probe.candidateChampionRejectedWithChestClosed, 0, f0302);
    ok &= expect_int("dead champion rejected with chest closed",
                     g_probe.deadChampionRejectedWithChestClosed, 0, f0302);
    ok &= expect_int("slotbox 6 above party rejected with chest closed",
                     g_probe.slotbox6AbovePartyRejectedWithChestClosed, 0, f0302);
    ok &= expect_int("slotbox 0 open inventory champion rejected with chest "
                     "closed",
                     g_probe.slotbox0OpenInventoryChampionRejectedWithChestClosed,
                     0, f0302);
    return ok;
}

static int test_slotbox_selectors_with_chest_closed(void)
{
    const char* f0302 = "ReDMCSB CHAMPION.C F0302 lines 662-710";
    int ok = 1;

    ok &= expect_int("slotbox 0 closed chest selects champion 0 ready hand",
                     g_probe.slotbox0ClosedChestSelectsChampion0ReadyHand, 1,
                     f0302);
    ok &= expect_int("slotbox 3 closed chest selects champion 1 action hand",
                     g_probe.slotbox3ClosedChestSelectsChampion1ActionHand, 1,
                     f0302);
    ok &= expect_int("slotbox 7 closed chest selects champion 3 action hand",
                     g_probe.slotbox7ClosedChestSelectsChampion3ActionHand, 1,
                     f0302);
    return ok;
}

static int test_click_swaps_leader_hand_with_chest_closed(void)
{
    const char* f0302 = "ReDMCSB CHAMPION.C F0302 lines 662-710";
    int ok = 1;

    ok &= expect_int("click on champion 0 ready hand swaps leader hand",
                     g_probe.clickOnChampion0ReadyHandSwapsLeaderHand, 1, f0302);
    ok &= expect_int("click on champion 1 action hand swaps leader hand",
                     g_probe.clickOnChampion1ActionHandSwapsLeaderHand, 1, f0302);
    ok &= expect_int("click on champion 3 action hand swaps leader hand",
                     g_probe.clickOnChampion3ActionHandSwapsLeaderHand, 1, f0302);
    return ok;
}

static int test_f0334_close_resets_closed_chest_state(void)
{
    const char* f0334 = "ReDMCSB CHEST.C F0334 lines 112-133";
    const char* f0291 = "ReDMCSB CHAMDRAW.C F0291 lines 621-630";
    const char* f0333Panel = "ReDMCSB CHEST.C F0333 lines 28-32";
    int ok = 1;

    /* After F0334 runs, G0426 is cleared, G0425 is all 0xFFFF, the panel
     * rolls back to PANEL_INVENTORY or PANEL_FOOD_WATER_POISONED, and the
     * C144 -> C145 action-hand icon swap reverts. */
    ok &= expect_int("F0334 leaves G0426 == 0 after open->close",
                     g_probe.f0334ClearsG0426OnInit, 1, f0334);
    ok &= expect_int("F0334 leaves G0425 all 0xFFFF after open->close",
                     g_probe.f0334ClearsG0425OnInit, 1, f0334);
    ok &= expect_int("F0334 leaves action hand icon C144 after open->close",
                     g_probe.f0334LeavesActionHandIconClosedOnInit, 1, f0291);
    ok &= expect_int("F0334 leaves panel content inventory/food-water after "
                     "open->close",
                     g_probe.f0334LeavesPanelInventoryOnInit, 1, f0333Panel);
    return ok;
}

static int test_status_click_keeps_closed_chest_state(void)
{
    const char* f0334 = "ReDMCSB CHEST.C F0334 lines 112-133";
    const char* f0291 = "ReDMCSB CHAMDRAW.C F0291 lines 621-630";
    const char* f0333Panel = "ReDMCSB CHEST.C F0333 lines 28-32";
    int ok = 1;

    /* A status hand click on the action hand slot box in the closed
     * state must not re-open the chest. */
    ok &= expect_int("status click keeps G0426 == 0",
                     g_probe.openChestThingStaysZeroAcrossStatusClick, 1, f0334);
    /* A status hand click must not write G0425 in the closed state. */
    ok &= expect_int("status click keeps G0425 all 0xFFFF",
                     g_probe.g0425StaysZeroAcrossStatusClick, 1, f0334);
    /* A status hand click must not flip the C144 -> C145 icon swap in
     * the closed state. */
    ok &= expect_int("status click keeps action hand icon C144",
                     g_probe.closedActionHandStaysClosedIconAfterStatusClick,
                     1, f0291);
    /* A status hand click must not change the panel content. */
    ok &= expect_int("status click keeps panel content inventory",
                     g_probe.panelContentStaysInventoryAfterStatusClick, 1,
                     f0333Panel);
    /* The F0077 / F0078 mouse screen-update bracketing balances across
     * the click + F0292 redraw. */
    ok &= expect_int("F0077 / F0078 mouse screen-update bracket balances",
                     g_probe.f0077F0078BalancedAcrossClick, 1, f0333Panel);
    return ok;
}

static int test_deterministic_hash(void)
{
    const char* f0302 = "ReDMCSB CHAMPION.C F0302 lines 662-710";
    int ok = 1;
    /* Run the probe a second time and confirm the hash is stable. */
    DM1_V1_InventoryPanelStatusHandClosedChestProbePc34 second;
    memset(&second, 0, sizeof(second));
    if (!dm1_v1_inventory_panel_status_hand_closed_chest_pc34(&second)) {
        ++g_assertions;
        printf("FAIL second probe run anchor=%s\n", f0302);
        return 0;
    }
    ok &= expect_u32("deterministic hash stable across runs",
                     second.deterministicHash, g_probe.deterministicHash, f0302);
    /* Sanity-pin the recorded hash so a future accidental hash mutation
     * is caught by the test. */
    ok &= expect_u32("deterministic hash matches expected record",
                     g_probe.deterministicHash,
                     (unsigned int)DM1_V1_IPHSCC_EXPECTED_HASH, f0302);
    return ok;
}

int main(void)
{
    int ok = 1;
    printf("== DM1 V1 inventory panel status hand closed chest runtime ==\n");
    ok &= test_spec_and_evidence();
    ok &= test_closed_chest_init_state();
    ok &= test_status_hand_table_with_chest_closed();
    ok &= test_rejections_with_chest_closed();
    ok &= test_slotbox_selectors_with_chest_closed();
    ok &= test_click_swaps_leader_hand_with_chest_closed();
    ok &= test_f0334_close_resets_closed_chest_state();
    ok &= test_status_click_keeps_closed_chest_state();
    ok &= test_deterministic_hash();
    if (ok) {
        printf("dm1V1InventoryPanelStatusHandClosedChestRuntimeOk=1\n");
        printf("Assertions: %d, Passes: %d\n", g_assertions, g_passes);
        return 0;
    }
    printf("FAIL: %d assertion(s) failed of %d total\n",
           g_assertions - g_passes, g_assertions);
    return 1;
}

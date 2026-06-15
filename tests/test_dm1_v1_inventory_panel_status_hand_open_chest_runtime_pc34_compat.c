#include "dm1/dm1_v1_inventory_panel_status_hand_open_chest_runtime_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions;
static int g_passes;

static DM1_V1_InventoryPanelStatusHandOpenChestProbePc34 g_probe;

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
    const DM1_V1_InventoryPanelStatusHandOpenChestSpecPc34* spec =
        dm1_v1_inventory_panel_status_hand_open_chest_spec_pc34();
    const char* evidence =
        dm1_v1_inventory_panel_status_hand_open_chest_source_evidence_pc34();
    const char* f0302 = "ReDMCSB CHAMPION.C F0302 lines 662-710";
    const char* f0333 = "ReDMCSB CHEST.C F0333 lines 30-76";
    const char* f0333Panel = "ReDMCSB CHEST.C F0333 lines 28-32";
    const char* f0334 = "ReDMCSB CHEST.C F0334 lines 112-133";
    const char* f0347 = "ReDMCSB PANEL.C F0347 lines 1639-1691";
    const char* f0291 = "ReDMCSB CHAMDRAW.C F0291 lines 621-630";
    const char* defs = "ReDMCSB DEFS.H lines 778-817";
    int ok = 1;

    ok &= expect_int("spec exists", spec != 0, 1, f0302);
    ok &= expect_int("spec contract only", spec->contractOnly, 1, f0302);
    ok &= expect_contains("scope contract_only", spec->scope,
                          "contract_only=1", f0302);
    ok &= expect_contains("scope no real asset", spec->scope,
                          "no real-asset", f0302);
    ok &= expect_contains("f0302 anchor in spec", spec->f0302Anchor,
                          "F0302", f0302);
    ok &= expect_contains("f0333 anchor in spec", spec->f0333Anchor,
                          "F0333", f0333);
    ok &= expect_contains("f0334 anchor in spec", spec->f0334Anchor,
                          "F0334", f0334);
    ok &= expect_contains("f0347 anchor in spec", spec->f0347Anchor,
                          "F0347", f0347);
    ok &= expect_contains("f0291 anchor in spec", spec->f0291Anchor,
                          "F0291", f0291);
    ok &= expect_contains("defs anchor in spec", spec->defsAnchor,
                          "C08_SLOT_BOX_INVENTORY_FIRST_SLOT", defs);
    ok &= expect_contains("evidence F0302:677", evidence, "F0302:677",
                          f0302);
    ok &= expect_contains("evidence F0302:681", evidence, "F0302:681",
                          f0302);
    ok &= expect_contains("evidence F0333:43-46", evidence, "F0333:43-46",
                          f0333);
    ok &= expect_contains("evidence F0333:28-32", evidence, "F0333:28-32",
                          f0333Panel);
    ok &= expect_contains("evidence F0334:112-133", evidence,
                          "F0334:112-133", f0334);
    ok &= expect_contains("evidence F0347:1639-1691", evidence, "F0347:1639-1691",
                          f0347);
    ok &= expect_contains("evidence F0291:621-630", evidence,
                          "F0291:621-630", f0291);
    ok &= expect_contains("evidence DEFS.H boundary", evidence,
                          "C08_SLOT_BOX_INVENTORY_FIRST_SLOT=8", defs);
    ok &= expect_contains("evidence PC34 chest panel value", evidence,
                          "M569_PANEL_CHEST=6", defs);
    return ok;
}

static int test_open_chest_state_with_status_hand_table(void)
{
    const char* f0302 = "ReDMCSB CHAMPION.C F0302 lines 662-710";
    const char* f0333 = "ReDMCSB CHEST.C F0333 lines 30-76";
    const char* f0333Panel = "ReDMCSB CHEST.C F0333 lines 28-32";
    const char* f0334 = "ReDMCSB CHEST.C F0334 lines 112-133";
    const char* f0347 = "ReDMCSB PANEL.C F0347 lines 1639-1691";
    const char* f0291 = "ReDMCSB CHAMDRAW.C F0291 lines 621-630";
    int ok = 1;

    ok &= expect_int("probe run",
                     dm1_v1_inventory_panel_status_hand_open_chest_pc34(
                         &g_probe),
                     1, f0302);
    ok &= expect_int("probe contract only", g_probe.contractOnly, 1,
                     f0302);

    /* Open-chest state invariants. */
    ok &= expect_int("chest closed at start (no open chest thing yet)",
                     g_probe.chestOpenBefore, 0, f0333);
    ok &= expect_int("chest open succeeds while status hand table is "
                     "still resolved",
                     g_probe.chestOpenAfter, 1, f0333);
    ok &= expect_int("open chest thing reflects the opened chest",
                     g_probe.openChestThingAfterOpen,
                     g_probe.openChestThingAfterAllClicks, f0333);
    ok &= expect_int("open chest thing is non-zero after open",
                     g_probe.openChestThingAfterOpen != 0, 1, f0333);
    ok &= expect_int("panel content starts as inventory",
                     g_probe.panelContentBeforeOpen,
                     DM1_PC34_PANEL_INVENTORY, f0333Panel);
    ok &= expect_int("open chest sets panel content to M569 chest",
                     g_probe.panelContentAfterOpen, DM1_PC34_PANEL_CHEST,
                     f0333Panel);
    ok &= expect_int("same-open chest path returns success",
                     g_probe.sameOpenChestResult, 1, f0333Panel);
    ok &= expect_int("same-open chest refreshes panel content before return",
                     g_probe.panelContentAfterSameOpen,
                     DM1_PC34_PANEL_CHEST, f0333Panel);
    ok &= expect_int("close leaves panel content on chest until redraw",
                     g_probe.panelContentAfterClose, DM1_PC34_PANEL_CHEST,
                     f0334);
    ok &= expect_int("redraw route sends panel back to food/water for "
                     "non-container action hand",
                     g_probe.panelContentAfterCloseRedraw,
                     DM1_PC34_PANEL_FOOD_WATER_POISONED, f0347);
    ok &= expect_int("closed action-hand icon baseline is C144",
                     g_probe.actionHandIconBefore, 144, f0291);
    ok &= expect_int("open action-hand icon swaps to C145",
                     g_probe.actionHandIconAfterOpen, 145, f0291);
    ok &= expect_int("close action-hand icon reverts to C144",
                     g_probe.actionHandIconAfterClose, 144, f0291);
    ok &= expect_int("close clears open chest thing",
                     g_probe.openChestThingAfterClose, 0, f0334);
    ok &= expect_int("chest still open after champion 0 ready-hand click",
                     g_probe.chestStillOpenAfterChampion0ReadyHandClick, 1,
                     f0302);
    ok &= expect_int("chest still open after champion 3 action-hand click",
                     g_probe.chestStillOpenAfterChampion3ActionHandClick,
                     1, f0302);
    ok &= expect_int("chest still open after candidate champion reject",
                     g_probe.chestStillOpenAfterCandidateChampionReject, 1,
                     f0302);
    ok &= expect_int("chest still open after dead champion reject",
                     g_probe.chestStillOpenAfterDeadChampionReject, 1,
                     f0302);
    ok &= expect_int("open chest thing unchanged after all status hand "
                     "clicks and rejections",
                     g_probe.openChestThingAfterAllClicks,
                     g_probe.openChestThingAfterOpen, f0302);
    ok &= expect_int("ready-hand status click leaves action-hand chest in "
                     "place",
                     g_probe.readyHandClickLeavesActionHandChestInPlace, 1,
                     f0302);
    ok &= expect_int("ready-hand status click keeps open action-hand icon",
                     g_probe.readyHandClickKeepsOpenActionHandIcon, 1,
                     f0291);
    ok &= expect_int("action-hand status click moves open chest to leader "
                     "hand",
                     g_probe.actionHandClickMovesOpenChestToLeaderHand, 1,
                     f0302);
    ok &= expect_int("action-hand status click replaces action hand with "
                     "leader object",
                     g_probe.actionHandClickReplacesActionHandWithLeaderHandObject,
                     1, f0302);
    ok &= expect_int("action-hand status click drops action-hand chest icon to closed",
                     g_probe.actionHandClickDropsActionHandIconToClosed, 1,
                     f0291);
    ok &= expect_int("action-hand status click preserves open chest thing",
                     g_probe.actionHandClickPreservesOpenChestThing, 1,
                     f0333);

    /* Status hand slot box 0..7 reduction table with chest open. */
    ok &= expect_int("slotbox 0 resolves champion 0 ready hand with "
                     "chest open",
                     g_probe.slotbox0ResolvesChampion0ReadyHandWithChestOpen,
                     1, f0302);
    ok &= expect_int("slotbox 3 resolves champion 1 action hand with "
                     "chest open",
                     g_probe.slotbox3ResolvesChampion1ActionHandWithChestOpen,
                     1, f0302);
    ok &= expect_int("slotbox 7 resolves champion 3 action hand with "
                     "chest open",
                     g_probe.slotbox7ResolvesChampion3ActionHandWithChestOpen,
                     1, f0302);
    ok &= expect_int("click on champion 0 ready hand swaps leader hand",
                     g_probe.clickOnChampion0ReadyHandSwapsLeaderHand, 1,
                     f0302);
    ok &= expect_int("click on champion 1 action hand swaps leader hand",
                     g_probe.clickOnChampion1ActionHandSwapsLeaderHand, 1,
                     f0302);
    ok &= expect_int("click on champion 3 action hand swaps leader hand",
                     g_probe.clickOnChampion3ActionHandSwapsLeaderHand, 1,
                     f0302);

    /* F0302:677-684 rejection rules with chest open. */
    ok &= expect_int("candidate champion flow rejected with chest open",
                     g_probe.candidateChampionRejectedWithChestOpen, 0,
                     f0302);
    ok &= expect_int("dead champion rejected with chest open",
                     g_probe.deadChampionRejectedWithChestOpen, 0, f0302);
    ok &= expect_int("slotbox 4 above party rejected with chest open",
                     g_probe.slotbox4AbovePartyRejectedWithChestOpen, 0,
                     f0302);
    ok &= expect_int("inventory champion 1 slotbox 0 rejected with chest "
                     "open",
                     g_probe.slotbox0OpenInventoryChampionRejectedWithChestOpen,
                     0, f0302);
    return ok;
}

static int test_status_row_slotbox_rows(void)
{
    const char* f0302 = "ReDMCSB CHAMPION.C F0302 lines 662-710";
    int ok = 1;
    int slotBoxIndex;

    /* ReDMCSB DEFS.H C08_SLOT_BOX_INVENTORY_FIRST_SLOT=8 + M070_HAND_SLOT_INDEX
     * maps slotbox 0..7 to (championIndex = slotbox>>1, slot = slotbox&1).
     * The chest-open state must not perturb that reduction. */
    for (slotBoxIndex = 0; slotBoxIndex < 8; ++slotBoxIndex) {
        char label[96];
        int expectedChampion = slotBoxIndex >> 1;
        int expectedSlot = (slotBoxIndex & 1) ? 1 : 0; /* action : ready */

        snprintf(label, sizeof(label),
                 "slotbox %d expected champion", slotBoxIndex);
        ok &= expect_int(label,
                         g_probe.rows[slotBoxIndex].expectedChampionIndex,
                         expectedChampion, f0302);
        snprintf(label, sizeof(label),
                 "slotbox %d expected pc34 source slot", slotBoxIndex);
        ok &= expect_int(label,
                         g_probe.rows[slotBoxIndex].expectedPc34SourceSlot,
                         expectedSlot, f0302);
        snprintf(label, sizeof(label),
                 "slotbox %d resolved champion", slotBoxIndex);
        ok &= expect_int(label,
                         g_probe.rows[slotBoxIndex].resolvedChampionIndex,
                         expectedChampion, f0302);
        snprintf(label, sizeof(label),
                 "slotbox %d resolved pc34 source slot", slotBoxIndex);
        ok &= expect_int(label,
                         g_probe.rows[slotBoxIndex].resolvedPc34SourceSlot,
                         expectedSlot, f0302);
        snprintf(label, sizeof(label),
                 "slotbox %d open chest before click", slotBoxIndex);
        ok &= expect_int(label,
                         g_probe.rows[slotBoxIndex].openChestThingBeforeClick,
                         g_probe.openChestThingAfterOpen, f0302);
        snprintf(label, sizeof(label),
                 "slotbox %d open chest after click", slotBoxIndex);
        ok &= expect_int(label,
                         g_probe.rows[slotBoxIndex].openChestThingAfterClick,
                         g_probe.openChestThingAfterOpen, f0302);
        snprintf(label, sizeof(label),
                 "slotbox %d preserves open chest through click",
                 slotBoxIndex);
        ok &= expect_int(label,
                         g_probe.rows[slotBoxIndex]
                             .openChestThingPreservedByClick,
                         1, f0302);
    }
    return ok;
}

int main(void)
{
    int ok = 1;

    printf("probe=dm1_v1_inventory_panel_status_hand_open_chest_runtime_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           dm1_v1_inventory_panel_status_hand_open_chest_source_evidence_pc34());

    ok &= test_spec_and_evidence();
    ok &= test_open_chest_state_with_status_hand_table();
    ok &= test_status_row_slotbox_rows();

    g_probe.totalAssertions = g_assertions;
    printf("assertions=%d passes=%d\n", g_assertions, g_passes);
    printf("dm1V1InventoryPanelStatusHandOpenChestRuntimeOk=%d\n",
           ok ? 1 : 0);
    return ok ? 0 : 1;
}

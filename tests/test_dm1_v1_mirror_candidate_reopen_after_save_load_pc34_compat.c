/*
 * DM1 V1 mirror-candidate reopen-after-save/load gate test harness.
 *
 * Asserts the contract pinned by
 * include/firestaff/dm1/v1/mirror_candidate/reopen_after_save_load_pc34_compat.h:
 *
 *   - The four runtime UI globals G0299_ui_CandidateChampionOrdinal,
 *     G0424_i_PanelContent, G0425_aT_ChestSlots[8] and
 *     G0426_T_OpenChest are NOT in the F0433 save blob and are NOT
 *     touched by the F0435 load path.
 *   - The leader hand (GLOBAL_DATA.LeaderHandObject at
 *     L1348_s_GlobalData line 1536) IS in the save blob and
 *     survives the round-trip.
 *   - The party pose and M516_CHAMPIONS chain (C2_SAVE_PART_PARTY at
 *     LOADSAVE.C F0433:1579-1584) survives the round-trip.
 *   - F0333 and F0334 are not invoked across the save+load.
 *   - After load, a fresh F0280 publication on the loaded party
 *     routes F0347 -> F0346 and re-renders the C040 panel at M568
 *     with the C040 graphic.
 *
 * Source-lock anchors asserted below:
 *   - LOADSAVE.C F0433:1502-1707 (F0433_STARTEND_ProcessCommand140_SaveGame_CPSCDF);
 *     :1571-1584 C2_SAVE_PART_PARTY serialization.
 *   - LOADSAVE.C F0435:2192-2660 (F0435_STARTEND_LoadGame) read path.
 *   - DEFS.H:534-571 GLOBAL_DATA struct (no G0299/G0424/G0425/G0426).
 *   - DEFS.H:5694/5877/5878/5881 UI globals.
 *   - REVIVE.C F0280:124-132 / F0282:744-806.
 *   - PANEL.C F0346:1619-1637 / F0347:1639-1693 / F0355:2244-2330.
 *   - CHEST.C F0333:30-67 / F0334:79-130 (no-invoke anchor).
 */

#include "firestaff/dm1/v1/mirror_candidate/reopen_after_save_load_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions;
static int g_failures;

static int expect_int(const char* label, int got, int want, const char* anchor)
{
    ++g_assertions;
    if (!anchor || anchor[0] == '\0') {
        ++g_failures;
        printf("FAIL %s missing-anchor\n", label);
        return 0;
    }
    if (got != want) {
        ++g_failures;
        printf("FAIL %s got=%d want=%d anchor=%s\n",
               label, got, want, anchor);
        return 0;
    }
    return 1;
}

static int expect_u32(const char* label,
                      uint32_t got,
                      uint32_t want,
                      const char* anchor)
{
    ++g_assertions;
    if (!anchor || anchor[0] == '\0') {
        ++g_failures;
        printf("FAIL %s missing-anchor\n", label);
        return 0;
    }
    if (got != want) {
        ++g_failures;
        printf("FAIL %s got=0x%08X want=0x%08X anchor=%s\n",
               label, (unsigned)got, (unsigned)want, anchor);
        return 0;
    }
    return 1;
}

static int expect_true(const char* label, int condition, const char* anchor)
{
    ++g_assertions;
    if (!anchor || anchor[0] == '\0' || !condition) {
        ++g_failures;
        printf("FAIL %s condition=%d anchor=%s\n",
               label, condition, anchor ? anchor : "(null)");
        return 0;
    }
    return 1;
}

static int expect_contains(const char* label,
                           const char* haystack,
                           const char* needle,
                           const char* anchor)
{
    ++g_assertions;
    if (!anchor || anchor[0] == '\0' || !haystack || !needle ||
        !strstr(haystack, needle)) {
        ++g_failures;
        printf("FAIL %s missing=%s anchor=%s\n",
               label, needle ? needle : "(null)",
               anchor ? anchor : "(null)");
        return 0;
    }
    return 1;
}

static int expected_step(int index)
{
    static const int steps[] = {
        DM1_V1_MC_RASL_STEP_INIT_PC34,
        DM1_V1_MC_RASL_STEP_F0355_OPEN_PC34,
        DM1_V1_MC_RASL_STEP_F0280_PUBLISH_PC34,
        DM1_V1_MC_RASL_STEP_F0433_SAVE_PC34,
        DM1_V1_MC_RASL_STEP_F0435_LOAD_PC34,
        DM1_V1_MC_RASL_STEP_ASSERT_NO_UI_MUTATE_PC34,
        DM1_V1_MC_RASL_STEP_F0280_REOPEN_PC34,
        DM1_V1_MC_RASL_STEP_F0347_REOPEN_PC34
    };

    return steps[index];
}

static int test_source_metadata(
    const DM1_V1_MirrorCandidateReopenAfterSaveLoadSpecPc34* spec)
{
    const char* evidence =
        dm1_v1_mirror_candidate_reopen_after_save_load_source_evidence_pc34();
    int ok = 1;

    ok &= expect_contains("source F0433 save", evidence,
                          "F0433:1502-1707", spec->f0433SaveAnchor);
    ok &= expect_contains("source F0435 load", evidence,
                          "F0435:2192-2660", spec->f0435LoadAnchor);
    ok &= expect_contains("source F0280 publish", evidence,
                          "F0280:124-132", spec->f0280PublishAnchor);
    ok &= expect_contains("source F0282 clear", evidence,
                          "F0282:744-806", spec->f0282ClearAnchor);
    ok &= expect_contains("source F0346 resurrect", evidence,
                          "F0346:1619-1637", spec->f0346ResurrectAnchor);
    ok &= expect_contains("source F0347 panel", evidence,
                          "F0347:1639-1693", spec->f0347PanelAnchor);
    ok &= expect_contains("source F0355 toggle", evidence,
                          "F0355:2244-2330", spec->f0355ToggleAnchor);
    ok &= expect_contains("source F0333 open", evidence,
                          "F0333:30-67", spec->f0333OpenAnchor);
    ok &= expect_contains("source F0334 close", evidence,
                          "F0334:79-130", spec->f0334CloseAnchor);
    ok &= expect_contains("defs GLOBAL_DATA", evidence,
                          "GLOBAL_DATA",
                          spec->defsGlobalDataAnchor);
    ok &= expect_contains("defs G0299", evidence, "G0299",
                          spec->defsUiGlobalsAnchor);
    ok &= expect_contains("defs G0424", evidence, "G0424",
                          spec->defsUiGlobalsAnchor);
    ok &= expect_contains("defs G0425", evidence, "G0425",
                          spec->defsUiGlobalsAnchor);
    ok &= expect_contains("defs G0426", evidence, "G0426",
                          spec->defsUiGlobalsAnchor);
    ok &= expect_contains("disjoint c160", spec->disjointness,
                          "C160 close-while-rotation-pending",
                          spec->disjointness);
    ok &= expect_contains("disjoint c061", spec->disjointness,
                          "C061 drop-during-resurrect-pending",
                          spec->disjointness);
    ok &= expect_contains("disjoint c045", spec->disjointness,
                          "C045 food/water accept cross-rotation",
                          spec->disjointness);
    ok &= expect_contains("disjoint c040 browse", spec->disjointness,
                          "C040 panel browse pickup-rotate race",
                          spec->disjointness);
    ok &= expect_contains("disjoint panel redraw", spec->disjointness,
                          "C040 panel redraw after inventory exit",
                          spec->disjointness);
    ok &= expect_contains("disjoint resurrect chest close", spec->disjointness,
                          "resurrect-chest-close-order",
                          spec->disjointness);
    ok &= expect_contains("disjoint resurrect confirm", spec->disjointness,
                          "resurrect-confirm-inventory-interrupt",
                          spec->disjointness);
    ok &= expect_contains("disjoint close after shuffle", spec->disjointness,
                          "close-after-party-shuffle",
                          spec->disjointness);
    ok &= expect_contains("disjoint close while resurrect pending pickup",
                          spec->disjointness,
                          "close-while-resurrect-pending-with-inventory-pickup",
                          spec->disjointness);
    ok &= expect_contains("disjoint c040 eye", spec->disjointness,
                          "C040 eye live candidate",
                          spec->disjointness);
    ok &= expect_contains("disjoint c040 owner", spec->disjointness,
                          "C040 chrome inventory owner swap",
                          spec->disjointness);
    ok &= expect_contains("disjoint c040 redraw chest", spec->disjointness,
                          "C040 redraw after chest close",
                          spec->disjointness);
    ok &= expect_int("contract-only", spec->contractOnly, 1,
                     spec->contractMarker);
    ok &= expect_int("no game data", spec->noGameData, 1,
                     spec->contractMarker);
    ok &= expect_int("no graphics", spec->noGraphicsDatLoad, 1,
                     spec->contractMarker);
    ok &= expect_int("no dungeon", spec->noDungeonDatLoad, 1,
                     spec->contractMarker);
    ok &= expect_int("no pixels", spec->noRealAssetPixels, 1,
                     spec->contractMarker);
    ok &= expect_u32("seed", spec->deterministicSeed,
                     DM1_V1_MC_RASL_DETERMINISTIC_SEED_PC34,
                     spec->contractMarker);
    return ok;
}

static int test_probe_flags(
    const DM1_V1_MirrorCandidateReopenAfterSaveLoadProbePc34* p,
    const DM1_V1_MirrorCandidateReopenAfterSaveLoadSpecPc34* spec)
{
    int ok = 1;

    ok &= expect_int("probe contract-only", p->contractOnly, 1,
                     spec->contractMarker);
    ok &= expect_int("probe no game data", p->noGameData, 1,
                     spec->contractMarker);
    ok &= expect_int("probe no graphics", p->noGraphicsDatLoad, 1,
                     spec->contractMarker);
    ok &= expect_int("probe no dungeon", p->noDungeonDatLoad, 1,
                     spec->contractMarker);
    ok &= expect_int("probe no pixels", p->noRealAssetPixels, 1,
                     spec->contractMarker);
    ok &= expect_u32("probe seed", p->deterministicSeed,
                     spec->deterministicSeed,
                     spec->contractMarker);
    ok &= expect_true("probe hash", p->deterministicHash != 0u,
                      spec->contractMarker);
    return ok;
}

static int test_sequence(
    const DM1_V1_MirrorCandidateReopenAfterSaveLoadProbePc34* p,
    const DM1_V1_MirrorCandidateReopenAfterSaveLoadSpecPc34* spec)
{
    int i;
    int ok = 1;

    ok &= expect_int("step count", p->stepCount,
                     DM1_V1_MC_RASL_TRACE_COUNT_PC34,
                     spec->f0280PublishAnchor);
    for (i = 0; i < DM1_V1_MC_RASL_TRACE_COUNT_PC34; ++i) {
        char label[48];
        (void)snprintf(label, sizeof(label), "step %d", i);
        ok &= expect_int(label, p->stepTrace[i], expected_step(i),
                         spec->f0280PublishAnchor);
    }
    return ok;
}

static int test_save_load_no_ui_mutate(
    const DM1_V1_MirrorCandidateReopenAfterSaveLoadProbePc34* p,
    const DM1_V1_MirrorCandidateReopenAfterSaveLoadSpecPc34* spec)
{
    int ok = 1;

    /* F0433 / F0435 only — F0280 / F0282 / F0346 / F0347 / F0355
     * are dispatch points, not save/load functions. */
    ok &= expect_int("f0433 save count", p->f0433SaveCount, 1,
                     spec->f0433SaveAnchor);
    ok &= expect_int("f0435 load count", p->f0435LoadCount, 1,
                     spec->f0435LoadAnchor);

    /* The four UI globals are NOT in the save snapshot. */
    ok &= expect_int("g0299 cleared by save (not in snapshot)",
                     p->g0299ClearedBySave, 1,
                     spec->f0433SaveAnchor);
    ok &= expect_int("g0424 not mutated by save (not in snapshot)",
                     p->g0424MutatedBySave, 0,
                     spec->f0433SaveAnchor);
    ok &= expect_int("g0425 not mutated by save (not in snapshot)",
                     p->g0425MutatedBySave, 0,
                     spec->f0433SaveAnchor);
    ok &= expect_int("g0426 not mutated by save (not in snapshot)",
                     p->g0426MutatedBySave, 0,
                     spec->f0433SaveAnchor);

    /* The four UI globals are NOT in the load snapshot. */
    ok &= expect_int("g0299 cleared by load (not in snapshot)",
                     p->g0299ClearedByLoad, 1,
                     spec->f0435LoadAnchor);
    ok &= expect_int("g0424 not mutated by load (not in snapshot)",
                     p->g0424MutatedByLoad, 0,
                     spec->f0435LoadAnchor);
    ok &= expect_int("g0425 not mutated by load (not in snapshot)",
                     p->g0425MutatedByLoad, 0,
                     spec->f0435LoadAnchor);
    ok &= expect_int("g0426 not mutated by load (not in snapshot)",
                     p->g0426MutatedByLoad, 0,
                     spec->f0435LoadAnchor);

    /* F0333 / F0334 are not invoked across save+load. */
    ok &= expect_int("f0333 not invoked across save+load",
                     p->f0333NotInvokedAcrossSaveLoad, 1,
                     spec->f0333OpenAnchor);
    ok &= expect_int("f0334 not invoked across save+load",
                     p->f0334NotInvokedAcrossSaveLoad, 1,
                     spec->f0334CloseAnchor);
    return ok;
}

static int test_save_load_state(
    const DM1_V1_MirrorCandidateReopenAfterSaveLoadProbePc34* p,
    const DM1_V1_MirrorCandidateReopenAfterSaveLoadSpecPc34* spec)
{
    int ok = 1;

    /* Live C040 candidate state before save. */
    ok &= expect_int("g0299 before save non-zero", p->g0299BeforeSave != 0, 1,
                     spec->f0280PublishAnchor);
    ok &= expect_int("g0424 before save at M568",
                     p->g0424BeforeSave,
                     DM1_V1_MC_RASL_M568_PANEL_RESURRECT_REINCARNATE_PC34,
                     spec->f0346ResurrectAnchor);
    ok &= expect_int("panel content before save at M568",
                     p->panelContentBeforeSave,
                     DM1_V1_MC_RASL_M568_PANEL_RESURRECT_REINCARNATE_PC34,
                     spec->f0346ResurrectAnchor);
    ok &= expect_int("g0425 visible count before save > 0",
                     p->g0425VisibleCountBeforeSave > 0 ? 1 : 0, 1,
                     spec->f0333OpenAnchor);
    ok &= expect_int("g0425 non-empty before save",
                     p->g0425NonEmptyBeforeSave, 1,
                     spec->f0333OpenAnchor);
    ok &= expect_int("g0426 before save non-zero", p->g0426BeforeSave != 0, 1,
                     spec->f0333OpenAnchor);

    /* G0299 / G0424 / G0426 are byte-stable across the F0433 save
     * path (the runtime UI state is not in the save blob). */
    ok &= expect_int("g0299 after save = before save",
                     p->g0299AfterSave, p->g0299BeforeSave,
                     spec->f0433SaveAnchor);
    ok &= expect_int("g0424 after save = before save",
                     p->g0424AfterSave, p->g0424BeforeSave,
                     spec->f0433SaveAnchor);
    ok &= expect_int("g0426 after save = before save",
                     p->g0426AfterSave, p->g0426BeforeSave,
                     spec->f0433SaveAnchor);

    /* The runtime UI state resets on load. */
    ok &= expect_int("g0299 reset to zero by load",
                     p->g0299ResetToZeroByLoad, 1,
                     spec->f0435LoadAnchor);
    ok &= expect_int("g0424 reset to inventory by load",
                     p->g0424ResetToInventoryByLoad, 1,
                     spec->f0435LoadAnchor);
    ok &= expect_int("g0426 reset to no-thing by load",
                     p->g0426ResetToNoThingByLoad, 1,
                     spec->f0435LoadAnchor);
    ok &= expect_int("g0425 reset to all-none by load",
                     p->g0425ResetToAllNoneByLoad, 1,
                     spec->f0435LoadAnchor);
    ok &= expect_int("g0425 cleared by load",
                     p->g0425ClearedByLoad, 1,
                     spec->f0435LoadAnchor);

    ok &= expect_int("g0299 after load = 0", p->g0299AfterLoad, 0,
                     spec->f0435LoadAnchor);
    ok &= expect_int("g0424 after load = C00_PANEL_INVENTORY",
                     p->g0424AfterLoad,
                     DM1_V1_MC_RASL_C00_PANEL_INVENTORY_PC34,
                     spec->f0435LoadAnchor);
    ok &= expect_int("g0426 after load = C0xFFFF_THING_NONE",
                     p->g0426AfterLoad,
                     DM1_V1_MC_RASL_NO_THING_PC34,
                     spec->f0435LoadAnchor);
    ok &= expect_int("panel content after load = C00_PANEL_INVENTORY",
                     p->panelContentAfterLoad,
                     DM1_V1_MC_RASL_C00_PANEL_INVENTORY_PC34,
                     spec->f0435LoadAnchor);

    /* The leader hand IS in the save blob (GLOBAL_DATA.LeaderHandObject
     * at F0433 line 1536), so it survives the round-trip. */
    ok &= expect_int("leader hand item survives save+load",
                     p->leaderHandItemAfterLoad, p->leaderHandItemBeforeSave,
                     spec->defsGlobalDataAnchor);
    return ok;
}

static int test_reopen(
    const DM1_V1_MirrorCandidateReopenAfterSaveLoadProbePc34* p,
    const DM1_V1_MirrorCandidateReopenAfterSaveLoadSpecPc34* spec)
{
    int ok = 1;

    /* Post-load F0280 publication re-routes F0347 -> F0346. */
    ok &= expect_int("g0299 reopened by f0280", p->g0299ReopenedByF0280, 1,
                     spec->f0280PublishAnchor);
    ok &= expect_int("g0299 after reopen non-zero",
                     p->g0299AfterReopen != 0, 1,
                     spec->f0280PublishAnchor);
    ok &= expect_int("g0424 after reopen = M568",
                     p->g0424AfterReopen,
                     DM1_V1_MC_RASL_M568_PANEL_RESURRECT_REINCARNATE_PC34,
                     spec->f0346ResurrectAnchor);
    ok &= expect_int("panel content after reopen = M568",
                     p->panelContentAfterReopen,
                     DM1_V1_MC_RASL_M568_PANEL_RESURRECT_REINCARNATE_PC34,
                     spec->f0346ResurrectAnchor);
    ok &= expect_int("reopen routed to f0346", p->reopenRoutedToF0346, 1,
                     spec->f0346ResurrectAnchor);
    ok &= expect_int("reopen c040 graphic drawn",
                     p->reopenC040GraphicDrawn, 1,
                     spec->f0346ResurrectAnchor);
    ok &= expect_int("reopen m568 panel set", p->reopenM568PanelSet, 1,
                     spec->f0346ResurrectAnchor);
    /* The reopen path does not call F0282: confirm/cancel is a
     * later user action that comes after the reopen. */
    ok &= expect_int("reopen no f0282 clear", p->reopenNoF0282Clear, 1,
                     spec->f0282ClearAnchor);

    /* The reopen preserves the party and the leader hand. */
    ok &= expect_int("reopen party preserved", p->reopenPartyPreserved, 1,
                     spec->defsGlobalDataAnchor);
    ok &= expect_int("reopen leader hand preserved",
                     p->reopenLeaderHandPreserved, 1,
                     spec->defsGlobalDataAnchor);

    /* F0280 publish count: 1 for the original, 1 for the reopen = 2. */
    ok &= expect_int("f0280 publish count (initial + reopen)",
                     p->f0280PublishCount, 2,
                     spec->f0280PublishAnchor);
    ok &= expect_int("f0346 resurrect draw count (initial + reopen)",
                     p->f0346ResurrectDrawCount, 2,
                     spec->f0346ResurrectAnchor);
    ok &= expect_int("f0347 panel draw count (initial + reopen)",
                     p->f0347PanelDrawCount, 2,
                     spec->f0347PanelAnchor);
    ok &= expect_int("f0282 clear count = 0", p->f0282ClearCount, 0,
                     spec->f0282ClearAnchor);
    return ok;
}

static int test_f0355_open(
    const DM1_V1_MirrorCandidateReopenAfterSaveLoadProbePc34* p,
    const DM1_V1_MirrorCandidateReopenAfterSaveLoadSpecPc34* spec)
{
    int ok = 1;

    /* F0355 fires once for the leader open; the !G0299 candidate
     * gate at lines 2318-2322 is asserted as a no-op here because
     * the inventory is opened before the F0280 candidate publishes. */
    ok &= expect_int("f0355 open for leader count",
                     p->f0355OpenForLeaderCount, 1,
                     spec->f0355ToggleAnchor);
    return ok;
}

static int test_disjoint(
    const DM1_V1_MirrorCandidateReopenAfterSaveLoadProbePc34* p,
    const DM1_V1_MirrorCandidateReopenAfterSaveLoadSpecPc34* spec)
{
    int ok = 1;

    ok &= expect_int("not pass c160 close rotation pending",
                     p->noPassC160CloseRotationPending, 1,
                     spec->disjointness);
    ok &= expect_int("not pass c061 drop resurrect pending",
                     p->noPassC061DropResurrectPending, 1,
                     spec->disjointness);
    ok &= expect_int("not pass c045 food water accept cross rotation",
                     p->noPassC045FoodWaterAcceptCrossRotation, 1,
                     spec->disjointness);
    ok &= expect_int("not pass c040 panel browse pickup rotate race",
                     p->noPassC040PanelBrowsePickupRotateRace, 1,
                     spec->disjointness);
    ok &= expect_int("not pass panel redraw after inventory exit",
                     p->noPassPanelRedrawAfterInventoryExit, 1,
                     spec->disjointness);
    ok &= expect_int("not pass resurrect chest close order",
                     p->noPassResurrectChestCloseOrder, 1,
                     spec->disjointness);
    ok &= expect_int("not pass resurrect confirm inventory interrupt",
                     p->noPassResurrectConfirmInventoryInterrupt, 1,
                     spec->disjointness);
    ok &= expect_int("not pass close after party shuffle",
                     p->noPassCloseAfterPartyShuffle, 1,
                     spec->disjointness);
    ok &= expect_int("not pass close while resurrect pending pickup",
                     p->noPassCloseWhileResurrectPendingWithInventoryPickup, 1,
                     spec->disjointness);
    ok &= expect_int("not pass c040 eye live candidate",
                     p->noPassC040EyeLiveCandidate, 1,
                     spec->disjointness);
    ok &= expect_int("not pass c040 owner swap",
                     p->noPassC040OwnerSwap, 1,
                     spec->disjointness);
    ok &= expect_int("not pass c040 redraw after chest close",
                     p->noPassC040RedrawAfterChestClose, 1,
                     spec->disjointness);
    return ok;
}

int main(void)
{
    const DM1_V1_MirrorCandidateReopenAfterSaveLoadSpecPc34* spec =
        dm1_v1_mirror_candidate_reopen_after_save_load_spec_pc34();
    DM1_V1_MirrorCandidateReopenAfterSaveLoadProbePc34 probe;
    int ok = 1;

    memset(&probe, 0, sizeof(probe));
    ok &= expect_true("spec accessor", spec != NULL,
                      "LOADSAVE.C F0433:1502-1707");
    ok &= test_source_metadata(spec);
    ok &= expect_int("run accepted",
                     dm1_v1_mirror_candidate_reopen_after_save_load_run_pc34(
                         &probe),
                     1, spec->f0433SaveAnchor);
    ok &= test_probe_flags(&probe, spec);
    ok &= test_sequence(&probe, spec);
    ok &= test_save_load_no_ui_mutate(&probe, spec);
    ok &= test_save_load_state(&probe, spec);
    ok &= test_f0355_open(&probe, spec);
    ok &= test_reopen(&probe, spec);
    ok &= test_disjoint(&probe, spec);
    ok &= expect_int("null run rejected",
                     dm1_v1_mirror_candidate_reopen_after_save_load_run_pc34(
                         NULL),
                     0, spec->contractMarker);
    ok &= expect_true("assertion floor", g_assertions >= 80,
                      spec->contractMarker);

    if (!ok || g_failures) {
        printf("FAIL test_dm1_v1_mirror_candidate_reopen_after_save_load_pc34_compat assertions=%d failures=%d hash=0x%08X\n",
               g_assertions, g_failures,
               (unsigned)probe.deterministicHash);
        return 1;
    }

    printf("PASS test_dm1_v1_mirror_candidate_reopen_after_save_load_pc34_compat assertions=%d failures=0 hash=0x%08X\n",
           g_assertions,
           (unsigned)probe.deterministicHash);
    return 0;
}

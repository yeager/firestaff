#include "dm1_v1_chest_empty_reopen_runtime_pc34_compat.h"

#include <stdio.h>
#include <string.h>

/*
 * Test harness for the DM1 V1 empty-chest open/close runtime gate.
 *
 * ReDMCSB anchors asserted below:
 * CHEST.C F0333:30-32, F0333:36-46, F0333:67-76, F0334:113-117, F0334:117-132.
 * CHAMPION.C F0297/F0298:243-298.
 * DEFS.H:434,778-817.
 */

static int g_assertions;
static int g_failures;

static int expect_int(const char* label,
                      int got,
                      int want,
                      const char* redmcsbAnchor)
{
    ++g_assertions;
    if (!redmcsbAnchor || redmcsbAnchor[0] == '\0') {
        ++g_failures;
        printf("FAIL %s missing ReDMCSB anchor\n", label);
        return 0;
    }
    if (got != want) {
        ++g_failures;
        printf("FAIL %s got=%d want=%d anchor=%s\n",
               label, got, want, redmcsbAnchor);
        return 0;
    }
    printf("PASS %s=%d anchor=%s\n", label, got, redmcsbAnchor);
    return 1;
}

static int expect_str(const char* label,
                      const char* got,
                      const char* want,
                      const char* redmcsbAnchor)
{
    ++g_assertions;
    if (!redmcsbAnchor || redmcsbAnchor[0] == '\0') {
        ++g_failures;
        printf("FAIL %s missing ReDMCSB anchor\n", label);
        return 0;
    }
    if (!got || !want || strcmp(got, want) != 0) {
        ++g_failures;
        printf("FAIL %s got=%s want=%s anchor=%s\n",
               label, got ? got : "(null)", want ? want : "(null)",
               redmcsbAnchor);
        return 0;
    }
    printf("PASS %s=%s anchor=%s\n", label, got, redmcsbAnchor);
    return 1;
}

static int test_spec(const DM1_V1_ChestEmptyReopenRuntimeSpecPc34* spec)
{
    const char* f0333Visible =
        "ReDMCSB CHEST.C F0333:67-76 fills the unused G0425 slots";
    const char* f0333Noop =
        "ReDMCSB CHEST.C F0333:30-32 (MEDIA278) same-chest no-op";
    const char* f0333Transitive =
        "ReDMCSB CHEST.C F0333:36-46 (CHANGE8_09_FIX) cross-chest close";
    const char* f0334NoOpen =
        "ReDMCSB CHEST.C F0334:113-117 (MEDIA070) close-when-already-closed";
    const char* f0334G0425Clear =
        "ReDMCSB CHEST.C F0334:117-122 (CHANGE8_09_FIX) G0425 clear";
    const char* f0334G0426Clear =
        "ReDMCSB CHEST.C F0334:113-117 (MEDIA070) G0426 clear";
    const char* f0297 =
        "ReDMCSB CHAMPION.C F0297/F0298:243-298 leader hand independence";
    const char* defs =
        "ReDMCSB DEFS.H:434,778-817 C0xFFFF_THING_NONE and C30..C37";
    int ok = 1;

    ok &= expect_str("contract marker", spec->contractMarker,
                     "Source-locked contract gate only; not full real-asset chest runtime parity.",
                     f0333Visible);
    ok &= expect_int("chest A sentinel", spec->chestA,
                     DM1_PC34_CHEST_EMPTY_REOPEN_CHEST_A, f0333Noop);
    ok &= expect_int("chest B sentinel", spec->chestB,
                     DM1_PC34_CHEST_EMPTY_REOPEN_CHEST_B, f0333Transitive);
    ok &= expect_int("chest C sentinel", spec->chestC,
                     DM1_PC34_CHEST_EMPTY_REOPEN_CHEST_C, f0333Visible);
    ok &= expect_int("leader item", spec->leaderItem,
                     DM1_PC34_CHEST_EMPTY_REOPEN_LEADER_ITEM, f0297);
    ok &= expect_int("leader weight", spec->leaderWeight,
                     DM1_PC34_CHEST_EMPTY_REOPEN_LEADER_WEIGHT, f0297);
    ok &= expect_int("leader charges", spec->leaderCharges,
                     DM1_PC34_CHEST_EMPTY_REOPEN_LEADER_CHARGES, f0297);
    ok &= expect_int("leader allowed slots", spec->leaderAllowedSlots,
                     DM1_PC34_ALLOWED_HEAD | DM1_PC34_ALLOWED_CONTAINER,
                     f0297);
    ok &= expect_int("thing none sentinel", spec->thingNoneSentinel,
                     0xFFFF, defs);
    ok &= expect_int("expected chest slot count", spec->expectedChestSlotCount,
                     8, defs);

    /* ReDMCSB CHEST.C F0333:67-76 is the visible-fill anchor. */
    ok &= expect_str("F0333 visible fill anchor prefix",
                     strstr(spec->f0333VisibleFillAnchor,
                            "CHEST.C F0333:67-76") != NULL ?
                         "CHEST.C F0333:67-76" : "",
                     "CHEST.C F0333:67-76", f0333Visible);
    /* ReDMCSB CHEST.C F0333:30-32 is the same-chest no-op anchor. */
    ok &= expect_str("F0333 same chest anchor prefix",
                     strstr(spec->f0333SameChestNoopAnchor,
                            "CHEST.C F0333:30-32") != NULL ?
                         "CHEST.C F0333:30-32" : "",
                     "CHEST.C F0333:30-32", f0333Noop);
    /* ReDMCSB CHEST.C F0333:36-46 is the cross-chest transitive close anchor. */
    ok &= expect_str("F0333 transitive anchor prefix",
                     strstr(spec->f0333TransitiveCloseAnchor,
                            "CHEST.C F0333:36-46") != NULL ?
                         "CHEST.C F0333:36-46" : "",
                     "CHEST.C F0333:36-46", f0333Transitive);
    /* ReDMCSB CHEST.C F0334:113-117 is the close-when-already-closed anchor. */
    ok &= expect_str("F0334 no open chest anchor prefix",
                     strstr(spec->f0334NoOpenChestAnchor,
                            "CHEST.C F0334:113-117") != NULL ?
                         "CHEST.C F0334:113-117" : "",
                     "CHEST.C F0334:113-117", f0334NoOpen);
    /* ReDMCSB CHEST.C F0334:117-122 is the G0425 clear anchor. */
    ok &= expect_str("F0334 G0425 clear anchor prefix",
                     strstr(spec->f0334G0425ClearAnchor,
                            "CHEST.C F0334:117-122") != NULL ?
                         "CHEST.C F0334:117-122" : "",
                     "CHEST.C F0334:117-122", f0334G0425Clear);
    /* ReDMCSB CHEST.C F0334:113-117 also clears G0426. */
    ok &= expect_str("F0334 G0426 clear anchor prefix",
                     strstr(spec->f0334G0426ClearAnchor,
                            "CHEST.C F0334:113-117") != NULL ?
                         "CHEST.C F0334:113-117" : "",
                     "CHEST.C F0334:113-117", f0334G0426Clear);
    /* ReDMCSB CHAMPION.C F0297/F0298:243-298 is the leader hand anchor. */
    ok &= expect_str("F0297/F0298 leader anchor prefix",
                     strstr(spec->f0297F0298LeaderHandAnchor,
                            "CHAMPION.C F0297/F0298:243-298") != NULL ?
                         "CHAMPION.C F0297/F0298:243-298" : "",
                     "CHAMPION.C F0297/F0298:243-298", f0297);
    /* ReDMCSB DEFS.H is the C30..C37 + 0xFFFF anchor. */
    ok &= expect_str("DEFS C537..C544 anchor prefix",
                     strstr(spec->defsC537C544Anchor,
                            "DEFS.H:778-817,434") != NULL ?
                         "DEFS.H:778-817,434" : "",
                     "DEFS.H:778-817,434", defs);
    return ok;
}

static int test_phase_1_open_empty(
    const DM1_V1_ChestEmptyReopenRuntimeProbePc34* probe,
    const DM1_V1_ChestEmptyReopenRuntimeSpecPc34* spec)
{
    const char* f0333Visible = spec->f0333VisibleFillAnchor;
    int ok = 1;

    ok &= expect_int("open A result", probe->openAResult, 1, f0333Visible);
    ok &= expect_int("open A open thing", probe->openAOpenThing,
                     spec->chestA, f0333Visible);
    ok &= expect_int("open A G0425 all NONE", probe->openAAllG0425NoneAfterOpen,
                     1, f0333Visible);
    return ok;
}

static int test_phase_2_close_empty(
    const DM1_V1_ChestEmptyReopenRuntimeProbePc34* probe,
    const DM1_V1_ChestEmptyReopenRuntimeSpecPc34* spec)
{
    const char* f0334G0425 = spec->f0334G0425ClearAnchor;
    int ok = 1;

    ok &= expect_int("close A result zero", probe->openACloseResult, 0,
                     f0334G0425);
    ok &= expect_int("close A count zero", probe->openACloseCount, 0,
                     f0334G0425);
    ok &= expect_int("close A G0426 cleared", probe->openAOpenThingAfterClose,
                     0, f0334G0425);
    ok &= expect_int("close A G0425 still all NONE",
                     probe->openAAllG0425NoneAfterClose, 1, f0334G0425);
    return ok;
}

static int test_phase_3_close_when_closed(
    const DM1_V1_ChestEmptyReopenRuntimeProbePc34* probe,
    const DM1_V1_ChestEmptyReopenRuntimeSpecPc34* spec)
{
    const char* f0334NoOpen = spec->f0334NoOpenChestAnchor;
    const char* f0334G0425 = spec->f0334G0425ClearAnchor;
    int ok = 1;

    ok &= expect_int("close-on-closed result", probe->openACloseOnAlreadyClosedResult,
                     0, f0334NoOpen);
    ok &= expect_int("close-on-closed count", probe->openACloseOnAlreadyClosedCount,
                     0, f0334NoOpen);
    ok &= expect_int("close-on-closed side-effect free",
                     probe->noF0334SideEffectsOnClosedOpen, 1, f0334G0425);
    return ok;
}

static int test_phase_4_same_chest_reopen(
    const DM1_V1_ChestEmptyReopenRuntimeProbePc34* probe,
    const DM1_V1_ChestEmptyReopenRuntimeSpecPc34* spec)
{
    const char* f0333Noop = spec->f0333SameChestNoopAnchor;
    const char* f0333Visible = spec->f0333VisibleFillAnchor;
    int ok = 1;

    ok &= expect_int("same-chest reopen result", probe->sameChestReopenResult,
                     1, f0333Noop);
    ok &= expect_int("same-chest reopen G0426 stable",
                     probe->sameChestReopenOpenThing, spec->chestA, f0333Noop);
    ok &= expect_int("same-chest reopen G0425 stable",
                     probe->sameChestReopenG0425Stable, 1, f0333Visible);
    ok &= expect_int("same-chest reopen G0426 stable flag",
                     probe->sameChestReopenOpenThingStable, 1, f0333Noop);
    return ok;
}

static int test_phase_5_cross_chest(
    const DM1_V1_ChestEmptyReopenRuntimeProbePc34* probe,
    const DM1_V1_ChestEmptyReopenRuntimeSpecPc34* spec)
{
    const char* f0333Transitive = spec->f0333TransitiveCloseAnchor;
    const char* f0333Visible = spec->f0333VisibleFillAnchor;
    const char* panelRouteAnchor = "ReDMCSB CHEST.C F0333 line 28";
    int ok = 1;

    ok &= expect_int("cross-chest A->B previous count",
                     probe->crossChestBPreviousCount, 0, f0333Transitive);
    ok &= expect_int("cross-chest A->B panel before replace",
                     probe->crossChestBPanelBeforeReplace,
                     DM1_PC34_PANEL_SCROLL, panelRouteAnchor);
    ok &= expect_int("cross-chest A->B panel after replace",
                     probe->crossChestBPanelAfterReplace,
                     DM1_PC34_PANEL_CHEST, panelRouteAnchor);
    ok &= expect_int("cross-chest A->B final open thing",
                     probe->crossChestBFinalOpenThing, spec->chestB,
                     f0333Transitive);
    ok &= expect_int("cross-chest A->B G0425 all NONE",
                     probe->crossChestBG0425AllNone, 1, f0333Visible);
    ok &= expect_int("cross-chest A->B panel after B close",
                     probe->crossChestBPanelAfterClose,
                     DM1_PC34_PANEL_CHEST, panelRouteAnchor);
    return ok;
}

static int test_phase_6_close_after_b(
    const DM1_V1_ChestEmptyReopenRuntimeProbePc34* probe,
    const DM1_V1_ChestEmptyReopenRuntimeSpecPc34* spec)
{
    const char* f0334G0425 = spec->f0334G0425ClearAnchor;
    int ok = 1;

    ok &= expect_int("close after B result", probe->crossChestBCloseAfterBResult,
                     0, f0334G0425);
    ok &= expect_int("close after B count", probe->crossChestBCloseAfterBCount,
                     0, f0334G0425);
    return ok;
}

static int test_phase_7_close_when_nothing(
    const DM1_V1_ChestEmptyReopenRuntimeProbePc34* probe,
    const DM1_V1_ChestEmptyReopenRuntimeSpecPc34* spec)
{
    const char* f0334NoOpen = spec->f0334NoOpenChestAnchor;
    const char* f0334NoWrite =
        "ReDMCSB CHEST.C F0334:113-117 returns before G0425/output writes";
    int ok = 1;

    ok &= expect_int("close when nothing result",
                     probe->closeWhenNothingOpenResult, 0, f0334NoOpen);
    ok &= expect_int("close when nothing count",
                     probe->closeWhenNothingOpenCount, 0, f0334NoOpen);
    ok &= expect_int("close when nothing G0426 stays NONE",
                     probe->closeWhenNothingOpenThingAfter, 0, f0334NoOpen);
    ok &= expect_int("close when nothing panel before",
                     probe->closeWhenNothingPanelBefore,
                     DM1_PC34_PANEL_SCROLL, f0334NoWrite);
    ok &= expect_int("close when nothing panel after",
                     probe->closeWhenNothingPanelAfter,
                     DM1_PC34_PANEL_SCROLL, f0334NoWrite);
    ok &= expect_int("no-open close preserves C537 stale item",
                     probe->staleC537AfterNoOpenClose,
                     DM1_PC34_CHEST_EMPTY_REOPEN_STALE_C537, f0334NoWrite);
    ok &= expect_int("no-open close preserves C544 stale item",
                     probe->staleC544AfterNoOpenClose,
                     DM1_PC34_CHEST_EMPTY_REOPEN_STALE_C544, f0334NoWrite);
    ok &= expect_int("no-open close preserves caller output",
                     probe->staleOutputAfterNoOpenClose,
                     DM1_PC34_CHEST_EMPTY_REOPEN_STALE_OUTPUT, f0334NoWrite);
    ok &= expect_int("no-open close stale window stable",
                     probe->noOpenClosePreservedStaleWindow, 1,
                     f0334NoWrite);
    ok &= expect_int("no-open close output buffer stable",
                     probe->noOpenClosePreservedOutputBuffer, 1,
                     f0334NoWrite);
    ok &= expect_int("no-open close panel content stable",
                     probe->noOpenClosePreservedPanelContent, 1,
                     f0334NoWrite);
    return ok;
}

static int test_phase_8_open_close_c(
    const DM1_V1_ChestEmptyReopenRuntimeProbePc34* probe,
    const DM1_V1_ChestEmptyReopenRuntimeSpecPc34* spec)
{
    const char* f0333Visible = spec->f0333VisibleFillAnchor;
    const char* f0334G0425 = spec->f0334G0425ClearAnchor;
    int ok = 1;

    ok &= expect_int("open C G0425 all NONE", probe->openCG0425AllNone, 1,
                     f0333Visible);
    ok &= expect_int("close C reopens cleanly", probe->closeCReopensCleanly,
                     1, f0334G0425);
    return ok;
}

static int test_phase_9_leader_hand(
    const DM1_V1_ChestEmptyReopenRuntimeProbePc34* probe,
    const DM1_V1_ChestEmptyReopenRuntimeSpecPc34* spec)
{
    const char* f0297 = spec->f0297F0298LeaderHandAnchor;
    int ok = 1;

    ok &= expect_int("leader hand type before",
                     probe->leaderHandTypeBeforeCycles, spec->leaderItem, f0297);
    ok &= expect_int("leader hand weight before",
                     probe->leaderHandWeightBeforeCycles, spec->leaderWeight,
                     f0297);
    ok &= expect_int("leader hand charges before",
                     probe->leaderHandChargesBeforeCycles, spec->leaderCharges,
                     f0297);
    ok &= expect_int("leader hand type after",
                     probe->leaderHandTypeAfterCycles, spec->leaderItem, f0297);
    ok &= expect_int("leader hand weight after",
                     probe->leaderHandWeightAfterCycles, spec->leaderWeight,
                     f0297);
    ok &= expect_int("leader hand charges after",
                     probe->leaderHandChargesAfterCycles, spec->leaderCharges,
                     f0297);
    ok &= expect_int("leader hand identical across cycles",
                     probe->leaderHandIdenticalAcrossCycles, 1, f0297);
    ok &= expect_int("champion load stable across cycles",
                     probe->championLoadStableAcrossCycles, 1, f0297);
    return ok;
}

int main(void)
{
    const DM1_V1_ChestEmptyReopenRuntimeSpecPc34* spec;
    const DM1_V1_ChestEmptyReopenRuntimeProbePc34* probe;
    const char* f0333Visible;
    int ok = 1;

    printf("probe=dm1_v1_chest_empty_reopen_runtime_pc34_compat\n");
    spec = dm1_v1_chest_empty_reopen_runtime_spec_pc34();
    f0333Visible = spec->f0333VisibleFillAnchor;
    ok &= expect_int("run result",
                     dm1_v1_chest_empty_reopen_runtime_run_pc34(NULL), 0,
                     f0333Visible);
    probe = NULL;
    {
        DM1_V1_ChestEmptyReopenRuntimeProbePc34 local;
        memset(&local, 0, sizeof(local));
        ok &= expect_int("run result populated",
                         dm1_v1_chest_empty_reopen_runtime_run_pc34(&local), 1,
                         f0333Visible);
        probe = &local;
    }
    /* Reach the persisted probe via the spec helper. */
    spec = dm1_v1_chest_empty_reopen_runtime_spec_pc34();

    ok &= test_spec(spec);
    ok &= test_phase_1_open_empty(probe, spec);
    ok &= test_phase_2_close_empty(probe, spec);
    ok &= test_phase_3_close_when_closed(probe, spec);
    ok &= test_phase_4_same_chest_reopen(probe, spec);
    ok &= test_phase_5_cross_chest(probe, spec);
    ok &= test_phase_6_close_after_b(probe, spec);
    ok &= test_phase_7_close_when_nothing(probe, spec);
    ok &= test_phase_8_open_close_c(probe, spec);
    ok &= test_phase_9_leader_hand(probe, spec);

    ok &= expect_int("minimum assertion count",
                     g_assertions >= 59 ? 1 : 0, 1, f0333Visible);

    printf("assertions=%d failures=%d\n", g_assertions, g_failures);
    printf("chestEmptyReopenRuntimeOk=%d\n",
           ok && g_failures == 0 ? 1 : 0);
    if (ok && g_failures == 0) {
        printf("PASS dm1_v1_chest_empty_reopen_runtime_pc34_compat assertions=%d\n",
               g_assertions);
        return 0;
    }
    return 1;
}

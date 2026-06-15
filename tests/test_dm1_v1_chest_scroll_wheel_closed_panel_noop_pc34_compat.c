#include "firestaff/dm1/v1/chest/scroll_wheel_closed_panel_noop_pc34_compat.h"

#include <stdio.h>
#include <string.h>

/*
 * ReDMCSB anchors asserted by this runtime ctest:
 * CHEST.C F0333:30-32 + F0333:53-76, F0334:113-132, CHAMPION.C
 * F0297/F0298:243-298, F0301:606-614, F0302:662-714 (lines 688-695 reject
 * empty hand + empty slot), COMMAND.C F0359:1452-1662, F0380:2045-2178,
 * IO.C F0077:1113-1122, F0078:1102-1111, and DEFS.H C030/C38/C10/C30/
 * C537..C544/G0425/G0426/M569_PANEL_CHEST/C540.
 */

static DM1_V1_ChestScrollWheelClosedPanelNoopProbePc34 g_probe;
static DM1_V1_ChestScrollWheelClosedPanelNoopProbePc34 g_probe_repeat;
static int g_assertions;
static int g_failures;

static int expect_int(const char* label, int got, int want,
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
        printf("FAIL %s got=%d want=%d anchor=%s\n",
               label, got, want, anchor);
        return 0;
    }
    printf("PASS %s=%d anchor=%s\n", label, got, anchor);
    return 1;
}

static int expect_u32(const char* label, uint32_t got, uint32_t want,
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
    printf("PASS %s=0x%08X anchor=%s\n", label, (unsigned)got, anchor);
    return 1;
}

static int expect_contains(const char* label, const char* haystack,
                           const char* needle, const char* anchor)
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
    printf("PASS %s contains=%s anchor=%s\n", label, needle, anchor);
    return 1;
}

static int test_source_and_spec(
    const DM1_V1_ChestScrollWheelClosedPanelNoopSpecPc34* spec)
{
    const char* evidence =
        dm1_v1_chest_scroll_wheel_closed_panel_noop_source_evidence_pc34();
    int ok = 1;

    ok &= expect_contains("evidence F0333 same-open", evidence,
                          "CHEST.C F0333 lines 30-32", spec->f0333OpenAnchor);
    ok &= expect_contains("evidence F0333 materialize", evidence,
                          "CHEST.C F0333 lines 53-76", spec->f0333OpenAnchor);
    ok &= expect_contains("evidence F0334 close", evidence,
                          "CHEST.C F0334 lines 113-132", spec->f0334CloseAnchor);
    ok &= expect_contains("evidence F0297 put", evidence,
                          "CHAMPION.C F0297 lines 243-268",
                          spec->f0297HandAnchor);
    ok &= expect_contains("evidence F0298 remove", evidence,
                          "CHAMPION.C F0298 lines 270-298",
                          spec->f0298HandAnchor);
    ok &= expect_contains("evidence F0301 slot", evidence,
                          "CHAMPION.C F0301 lines 606-614",
                          spec->f0301SlotAnchor);
    ok &= expect_contains("evidence F0302 dispatch", evidence,
                          "CHAMPION.C F0302 lines 662-714",
                          spec->f0302DispatchAnchor);
    ok &= expect_contains("evidence F0302 688-695 reject", evidence,
                          "CHAMPION.C F0302 lines 688-695",
                          spec->f0302DispatchAnchor);
    ok &= expect_contains("evidence F0302 688-710 live G0425", evidence,
                          "CHAMPION.C F0302 lines 688-710",
                          spec->f0302DispatchAnchor);
    ok &= expect_contains("evidence F0359 queue", evidence,
                          "COMMAND.C F0359 lines 1452-1662",
                          spec->f0359QueueAnchor);
    ok &= expect_contains("evidence F0380 drain", evidence,
                          "COMMAND.C F0380 lines 2045-2178",
                          spec->f0380DrainAnchor);
    ok &= expect_contains("evidence F0077 enable", evidence,
                          "IO.C F0077 lines 1113-1122",
                          spec->f0077EnableAnchor);
    ok &= expect_contains("evidence F0078 disable", evidence,
                          "IO.C F0078 lines 1102-1111",
                          spec->f0078DisableAnchor);
    ok &= expect_contains("evidence DEFS M569", evidence,
                          "M569_PANEL_CHEST", spec->defsAnchor);
    ok &= expect_contains("evidence DEFS C540", evidence, "C540",
                          spec->defsAnchor);
    ok &= expect_contains("spec marker closed panel", spec->contractMarker,
                          "G0426_T_OpenChest == 0",
                          spec->contractMarker);
    ok &= expect_contains("spec marker panel content", spec->contractMarker,
                          "DM1_PC34_PANEL_INVENTORY",
                          spec->contractMarker);

    ok &= expect_int("spec leader", spec->leaderIndex,
                     DM1_PC34_SCROLL_CLOSED_PANEL_LEADER,
                     spec->f0380DrainAnchor);
    ok &= expect_int("spec non-leader", spec->nonLeaderIndex,
                     DM1_PC34_SCROLL_CLOSED_PANEL_NON_LEADER,
                     spec->f0380DrainAnchor);
    ok &= expect_int("spec target slot", spec->targetSlotIndex,
                     DM1_PC34_SCROLL_CLOSED_PANEL_TARGET_SLOT_INDEX,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("spec target zone C540", spec->targetZone,
                     DM1_PC34_SCROLL_CLOSED_PANEL_TARGET_ZONE,
                     spec->defsAnchor);
    ok &= expect_int("spec target slot box C32", spec->targetSlotBox,
                     DM1_PC34_SCROLL_CLOSED_PANEL_TARGET_SLOT_BOX,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("spec target pc34 slot", spec->targetPc34Slot,
                     DM1_PC34_SCROLL_CLOSED_PANEL_TARGET_PC34_SLOT,
                     spec->defsAnchor);
    ok &= expect_int("spec wheel tick count", spec->wheelTickCount,
                     DM1_PC34_SCROLL_CLOSED_PANEL_WHEEL_TICK_COUNT,
                     spec->f0359QueueAnchor);
    ok &= expect_int("spec settle ticks", spec->settleTicks,
                     DM1_PC34_SCROLL_CLOSED_PANEL_SETTLE_TICKS,
                     spec->f0334CloseAnchor);
    return ok;
}

static int test_non_duplicate_markers(
    const DM1_V1_ChestScrollWheelClosedPanelNoopSpecPc34* spec)
{
    const char* evidence =
        dm1_v1_chest_scroll_wheel_closed_panel_noop_source_evidence_pc34();
    int ok = 1;

    ok &= expect_contains("nondup close race", evidence,
                          "not chest_scroll_wheel_close_race",
                          spec->nonDuplicateMarker);
    ok &= expect_contains("nondup drop onto closed", evidence,
                          "not chest_drop_onto_closed_chest_sink_runtime",
                          spec->nonDuplicateMarker);
    ok &= expect_contains("nondup pickup drop", evidence,
                          "not chest_scroll_wheel_pickup_drop",
                          spec->nonDuplicateMarker);
    ok &= expect_contains("nondup pull from chest", evidence,
                          "not chest_scroll_wheel_pull_from_chest",
                          spec->nonDuplicateMarker);
    ok &= expect_contains("nondup pickup overflow", evidence,
                          "not chest_scroll_wheel_pickup_overflow",
                          spec->nonDuplicateMarker);
    ok &= expect_contains("nondup drop during rotation", evidence,
                          "not chest_scroll_wheel_drop_during_rotation_non_leader_open",
                          spec->nonDuplicateMarker);
    ok &= expect_contains("nondup resurrect confirmation", evidence,
                          "not chest_scroll_wheel_resurrect_confirmation",
                          spec->nonDuplicateMarker);
    ok &= expect_contains("nondup resurrect rotation", evidence,
                          "not chest_scroll_wheel_resurrect_rotation_*",
                          spec->nonDuplicateMarker);
    ok &= expect_contains("nondup open pending", evidence,
                          "not chest_open_during_pending",
                          spec->nonDuplicateMarker);
    ok &= expect_contains("nondup full hand", evidence,
                          "not chest_open_with_full_leader_hand",
                          spec->nonDuplicateMarker);
    ok &= expect_contains("nondup partial mask", evidence,
                          "not chest_partial_mask_swap",
                          spec->nonDuplicateMarker);
    ok &= expect_contains("nondup eye route", evidence,
                          "not chest_eye_open_to_action_hand_switch",
                          spec->nonDuplicateMarker);
    ok &= expect_contains("nondup pickup family", evidence,
                          "not chest_pickup_*",
                          spec->nonDuplicateMarker);
    ok &= expect_contains("nondup mirror family", evidence,
                          "not mirror_candidate_*",
                          spec->nonDuplicateMarker);
    ok &= expect_contains("nondup resurrect pending", evidence,
                          "not resurrect-pending",
                          spec->nonDuplicateMarker);
    ok &= expect_contains("nondup C040 panel live", evidence,
                          "not C040-panel-live",
                          spec->nonDuplicateMarker);
    ok &= expect_contains("nondup occupied swap", evidence,
                          "not occupied-slot swap",
                          spec->nonDuplicateMarker);
    ok &= expect_contains("nondup save load", evidence,
                          "not save-load",
                          spec->nonDuplicateMarker);
    ok &= expect_contains("nondup teleporter", evidence,
                          "not teleporter",
                          spec->nonDuplicateMarker);
    ok &= expect_contains("nondup capacity", evidence,
                          "not capacity",
                          spec->nonDuplicateMarker);
    ok &= expect_contains("nondup encumbrance", evidence,
                          "not encumbrance",
                          spec->nonDuplicateMarker);
    ok &= expect_contains("nondup party rotate", evidence,
                          "not party-rotate",
                          spec->nonDuplicateMarker);
    ok &= expect_contains("nondup different chest open", evidence,
                          "not different-chest-open",
                          spec->nonDuplicateMarker);

    ok &= expect_int("probe no F0333", g_probe.noF0333Open, 1,
                     spec->nonDuplicateMarker);
    ok &= expect_int("probe no F0334", g_probe.noF0334Close, 1,
                     spec->nonDuplicateMarker);
    ok &= expect_int("probe no F0297", g_probe.noF0297Put, 1,
                     spec->nonDuplicateMarker);
    ok &= expect_int("probe no F0298", g_probe.noF0298Remove, 1,
                     spec->nonDuplicateMarker);
    ok &= expect_int("probe no F0301", g_probe.noF0301SlotWrite, 1,
                     spec->nonDuplicateMarker);
    ok &= expect_int("probe no F0302 dispatch", g_probe.noF0302SlotDispatch, 1,
                     spec->nonDuplicateMarker);
    ok &= expect_int("probe no F0380 drain", g_probe.noF0380Drain, 1,
                     spec->nonDuplicateMarker);
    ok &= expect_int("probe no panel route flip", g_probe.noPanelRouteFlip, 1,
                     spec->nonDuplicateMarker);
    ok &= expect_int("probe no C30 in leader hand", g_probe.noC30InLeaderHand,
                     1, spec->nonDuplicateMarker);
    ok &= expect_int("probe no resurrect pending", g_probe.noResurrectPending,
                     1, spec->nonDuplicateMarker);
    ok &= expect_int("probe no mirror candidate", g_probe.noMirrorCandidate,
                     1, spec->nonDuplicateMarker);
    ok &= expect_int("probe no teleporter save/load",
                     g_probe.noTeleporterSaveLoad, 1,
                     spec->nonDuplicateMarker);
    ok &= expect_int("probe no different chest open",
                     g_probe.noDifferentChestOpen, 1,
                     spec->nonDuplicateMarker);
    ok &= expect_int("probe no leader rotation", g_probe.noLeaderRotation, 1,
                     spec->nonDuplicateMarker);
    ok &= expect_int("probe no capacity encumbrance",
                     g_probe.noCapacityEncumbrance, 1,
                     spec->nonDuplicateMarker);
    ok &= expect_int("probe no party resize", g_probe.noPartyResize, 1,
                     spec->nonDuplicateMarker);
    return ok;
}

static int test_steps(
    const DM1_V1_ChestScrollWheelClosedPanelNoopProbePc34* p,
    const DM1_V1_ChestScrollWheelClosedPanelNoopSpecPc34* spec)
{
    int ok = 1;

    ok &= expect_int("runtime regression flag", p->runtimeRegression, 1,
                     spec->contractMarker);
    ok &= expect_int("step count", p->stepCount, 6,
                     spec->f0380DrainAnchor);
    ok &= expect_int("step init closed state", p->stepTrace[0],
                     DM1_PC34_SCROLL_CLOSED_PANEL_STEP_INIT_CLOSED_STATE,
                     spec->f0333OpenAnchor);
    ok &= expect_int("step snapshot before wheel", p->stepTrace[1],
                     DM1_PC34_SCROLL_CLOSED_PANEL_STEP_SNAPSHOT_BEFORE_WHEEL,
                     spec->f0333OpenAnchor);
    ok &= expect_int("step queue wheel click", p->stepTrace[2],
                     DM1_PC34_SCROLL_CLOSED_PANEL_STEP_QUEUE_WHEEL_CLICK,
                     spec->f0359QueueAnchor);
    ok &= expect_int("step drain wheel click", p->stepTrace[3],
                     DM1_PC34_SCROLL_CLOSED_PANEL_STEP_DRAIN_WHEEL_CLICK,
                     spec->f0380DrainAnchor);
    ok &= expect_int("step replay wheel ticks", p->stepTrace[4],
                     DM1_PC34_SCROLL_CLOSED_PANEL_STEP_REPLAY_WHEEL_TICKS,
                     spec->f0380DrainAnchor);
    ok &= expect_int("step settle inventory", p->stepTrace[5],
                     DM1_PC34_SCROLL_CLOSED_PANEL_STEP_SETTLE_INVENTORY,
                     spec->f0334CloseAnchor);
    return ok;
}

static int test_initial_closed_state(
    const DM1_V1_ChestScrollWheelClosedPanelNoopProbePc34* p,
    const DM1_V1_ChestScrollWheelClosedPanelNoopSpecPc34* spec)
{
    int ok = 1;

    ok &= expect_int("setup result", p->setupResult, 1,
                     spec->f0333OpenAnchor);
    ok &= expect_int("initial panel content", p->initialPanelContent,
                     DM1_PC34_PANEL_INVENTORY, spec->f0334CloseAnchor);
    ok &= expect_int("initial leader G0426", p->initialLeaderG0426, 0,
                     spec->f0334CloseAnchor);
    ok &= expect_int("initial non-leader G0426", p->initialNonLeaderG0426, 0,
                     spec->f0334CloseAnchor);
    ok &= expect_int("initial leader hand empty type",
                     p->initialLeaderHandType,
                     DM1_PC34_SCROLL_CLOSED_PANEL_LEADER_HAND_ITEM,
                     spec->f0297HandAnchor);
    ok &= expect_int("initial leader hand empty weight",
                     p->initialLeaderHandWeight,
                     DM1_PC34_SCROLL_CLOSED_PANEL_LEADER_HAND_WEIGHT,
                     spec->f0297HandAnchor);
    ok &= expect_int("initial leader hand empty charges",
                     p->initialLeaderHandCharges,
                     DM1_PC34_SCROLL_CLOSED_PANEL_LEADER_HAND_CHARGES,
                     spec->f0297HandAnchor);
    ok &= expect_int("initial non-leader hand type",
                     p->initialNonLeaderHandType,
                     DM1_PC34_SCROLL_CLOSED_PANEL_NON_LEADER_BACKPACK_ITEM,
                     spec->f0297HandAnchor);
    ok &= expect_int("initial non-leader hand weight",
                     p->initialNonLeaderHandWeight,
                     DM1_PC34_SCROLL_CLOSED_PANEL_NON_LEADER_BACKPACK_WEIGHT,
                     spec->f0297HandAnchor);
    ok &= expect_int("initial G0425 all zero", p->initialG0425AllZero, 1,
                     spec->f0333OpenAnchor);
    return ok;
}

static int test_wheel_queue_and_drain(
    const DM1_V1_ChestScrollWheelClosedPanelNoopProbePc34* p,
    const DM1_V1_ChestScrollWheelClosedPanelNoopSpecPc34* spec)
{
    int ok = 1;
    int i;

    ok &= expect_int("wheel ticks issued", p->wheelTicksIssued,
                     DM1_PC34_SCROLL_CLOSED_PANEL_WHEEL_TICK_COUNT,
                     spec->f0359QueueAnchor);
    ok &= expect_int("wheel ticks rejected", p->wheelTicksRejected,
                     DM1_PC34_SCROLL_CLOSED_PANEL_WHEEL_TICK_COUNT,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("command queue depth after issue",
                     p->commandQueueDepthAfterIssue, 0,
                     spec->f0380DrainAnchor);
    ok &= expect_int("mouse update depth after issue",
                     p->mouseUpdateDepthAfterIssue, 0,
                     spec->f0077EnableAnchor);
    ok &= expect_int("F0077 observed", p->f0077Observed, 1,
                     spec->f0077EnableAnchor);
    ok &= expect_int("F0078 observed (no dispatch -> balance fires)",
                     p->f0078Observed, 1, spec->f0078DisableAnchor);
    ok &= expect_int("reject reason no-G0426", p->rejectReasonNoG0426,
                     DM1_PC34_SCROLL_CLOSED_PANEL_REJECT_REASON_NO_G0426,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("all clicks rejected", p->clickAllRejected, 1,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("final click return", p->finalClickReturn, 0,
                     spec->f0302DispatchAnchor);
    for (i = 0; i < DM1_PC34_SCROLL_CLOSED_PANEL_WHEEL_TICK_COUNT; ++i) {
        char label[96];

        snprintf(label, sizeof(label), "click result tick %d", i);
        ok &= expect_int(label, p->clickResults[i], 1,
                         spec->f0302DispatchAnchor);
    }
    return ok;
}

static int test_drain_panel_stable(
    const DM1_V1_ChestScrollWheelClosedPanelNoopProbePc34* p,
    const DM1_V1_ChestScrollWheelClosedPanelNoopSpecPc34* spec)
{
    int ok = 1;

    ok &= expect_int("panel content after drain", p->panelContentAfterDrain,
                     DM1_PC34_PANEL_INVENTORY, spec->f0334CloseAnchor);
    ok &= expect_int("leader G0426 after drain", p->leaderG0426AfterDrain, 0,
                     spec->f0334CloseAnchor);
    ok &= expect_int("non-leader G0426 after drain",
                     p->nonLeaderG0426AfterDrain, 0,
                     spec->f0334CloseAnchor);
    ok &= expect_int("leader hand type after drain",
                     p->leaderHandTypeAfterDrain,
                     DM1_PC34_SCROLL_CLOSED_PANEL_LEADER_HAND_ITEM,
                     spec->f0297HandAnchor);
    ok &= expect_int("leader hand weight after drain",
                     p->leaderHandWeightAfterDrain,
                     DM1_PC34_SCROLL_CLOSED_PANEL_LEADER_HAND_WEIGHT,
                     spec->f0297HandAnchor);
    ok &= expect_int("leader hand charges after drain",
                     p->leaderHandChargesAfterDrain,
                     DM1_PC34_SCROLL_CLOSED_PANEL_LEADER_HAND_CHARGES,
                     spec->f0297HandAnchor);
    ok &= expect_int("non-leader hand type after drain",
                     p->nonLeaderHandTypeAfterDrain,
                     DM1_PC34_SCROLL_CLOSED_PANEL_NON_LEADER_BACKPACK_ITEM,
                     spec->f0297HandAnchor);
    ok &= expect_int("non-leader hand weight after drain",
                     p->nonLeaderHandWeightAfterDrain,
                     DM1_PC34_SCROLL_CLOSED_PANEL_NON_LEADER_BACKPACK_WEIGHT,
                     spec->f0297HandAnchor);
    ok &= expect_int("G0425 all zero after drain", p->g0425AllZeroAfterDrain, 1,
                     spec->f0333OpenAnchor);
    ok &= expect_int("panel stayed inventory", p->panelStayedInventory, 1,
                     spec->f0334CloseAnchor);
    ok &= expect_int("leader hand stable", p->leaderHandStable, 1,
                     spec->f0297HandAnchor);
    ok &= expect_int("G0426 stayed zero", p->g0426StayedZero, 1,
                     spec->f0334CloseAnchor);
    ok &= expect_int("G0425 manifest unchanged", p->g0425ManifestUnchanged, 1,
                     spec->f0334CloseAnchor);
    ok &= expect_int("F0077/F0078 balanced", p->f0077F0078Balanced, 1,
                     spec->f0078DisableAnchor);
    ok &= expect_int("mouse update depth after drain",
                     p->mouseUpdateDepthAfterDrain, 0,
                     spec->f0078DisableAnchor);
    ok &= expect_int("command queue depth after drain",
                     p->commandQueueDepthAfterDrain, 0,
                     spec->f0380DrainAnchor);
    ok &= expect_int("queue not mutated by drain", p->queueNotMutatedByDrain, 1,
                     spec->f0380DrainAnchor);
    ok &= expect_int("mouse not mutated by drain", p->mouseNotMutatedByDrain,
                     1, spec->f0078DisableAnchor);
    ok &= expect_int("leader backpack not mutated",
                     p->leaderBackpackNotMutated, 1,
                     spec->f0301SlotAnchor);
    ok &= expect_int("non-leader backpack stable", p->nonLeaderBackpackStable,
                     1, spec->f0297HandAnchor);
    return ok;
}

static int test_settle_stable(
    const DM1_V1_ChestScrollWheelClosedPanelNoopProbePc34* p,
    const DM1_V1_ChestScrollWheelClosedPanelNoopSpecPc34* spec)
{
    int ok = 1;
    int i;

    ok &= expect_int("panel content after settle", p->panelContentAfterSettle,
                     DM1_PC34_PANEL_INVENTORY, spec->f0334CloseAnchor);
    ok &= expect_int("leader hand type after settle",
                     p->leaderHandTypeAfterSettle,
                     DM1_PC34_SCROLL_CLOSED_PANEL_LEADER_HAND_ITEM,
                     spec->f0297HandAnchor);
    ok &= expect_int("leader G0426 after settle", p->leaderG0426AfterSettle,
                     0, spec->f0334CloseAnchor);
    ok &= expect_int("G0425 all zero after settle",
                     p->g0425AllZeroAfterSettle, 1,
                     spec->f0333OpenAnchor);
    ok &= expect_int("stable across settle", p->stableAcrossSettle, 1,
                     spec->f0334CloseAnchor);
    for (i = 0; i < DM1_PC34_SCROLL_CLOSED_PANEL_SLOT_COUNT; ++i) {
        char label[96];

        snprintf(label, sizeof(label), "G0425 type drain slot %d", i);
        ok &= expect_int(label, p->g0425TypesAfterDrain[i], 0,
                         spec->f0333OpenAnchor);
        snprintf(label, sizeof(label), "G0425 charge drain slot %d", i);
        ok &= expect_int(label, p->g0425ChargesAfterDrain[i], 0,
                         spec->f0333OpenAnchor);
        snprintf(label, sizeof(label), "G0425 type initial slot %d", i);
        ok &= expect_int(label, p->initialG0425Types[i], 0,
                         spec->f0333OpenAnchor);
        snprintf(label, sizeof(label), "G0425 charge initial slot %d", i);
        ok &= expect_int(label, p->initialG0425Charges[i], 0,
                         spec->f0333OpenAnchor);
    }
    return ok;
}

int main(void)
{
    const DM1_V1_ChestScrollWheelClosedPanelNoopSpecPc34* spec =
        dm1_v1_chest_scroll_wheel_closed_panel_noop_spec_pc34();
    int ok = 1;

    printf("probe=dm1_v1_chest_scroll_wheel_closed_panel_noop_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           dm1_v1_chest_scroll_wheel_closed_panel_noop_source_evidence_pc34());

    ok &= expect_int(
        "probe run",
        dm1_v1_chest_scroll_wheel_closed_panel_noop_run_pc34(&g_probe),
        1, spec->f0380DrainAnchor);
    ok &= expect_int(
        "probe repeat run",
        dm1_v1_chest_scroll_wheel_closed_panel_noop_run_pc34(&g_probe_repeat),
        1, spec->f0380DrainAnchor);
    ok &= test_source_and_spec(spec);
    ok &= test_steps(&g_probe, spec);
    ok &= test_initial_closed_state(&g_probe, spec);
    ok &= test_wheel_queue_and_drain(&g_probe, spec);
    ok &= test_drain_panel_stable(&g_probe, spec);
    ok &= test_settle_stable(&g_probe, spec);
    ok &= test_non_duplicate_markers(spec);
    ok &= expect_u32("repeat deterministic hash",
                     g_probe_repeat.deterministicHash,
                     g_probe.deterministicHash,
                     spec->f0380DrainAnchor);
    ok &= expect_u32("deterministic hash",
                     g_probe.deterministicHash,
                     0x7EBF9803u,
                     spec->f0334CloseAnchor);

    printf("deterministicHash=0x%08X\n",
           (unsigned)g_probe.deterministicHash);
    printf("assertions=%d failures=%d\n", g_assertions, g_failures);
    if (!ok || g_failures || g_assertions < 120) {
        printf("FAIL dm1_v1_chest_scroll_wheel_closed_panel_noop_pc34_compat\n");
        return 1;
    }
    printf("PASS dm1_v1_chest_scroll_wheel_closed_panel_noop_pc34_compat assertions=%d failures=0 deterministicHash=0x%08X\n",
           g_assertions, (unsigned)g_probe.deterministicHash);
    return 0;
}

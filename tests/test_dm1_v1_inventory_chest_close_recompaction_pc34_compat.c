#include "dm1_v1_inventory_chest_close_recompaction_pc34_compat.h"

#include <stdio.h>

static int expect_int(const char* label, int got, int want,
                      const char* redmcsbAnchor, int* assertionCount)
{
    if (assertionCount) {
        ++(*assertionCount);
    }
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
    return 1;
}

static int expect_less(const char* label, int left, int right,
                       const char* redmcsbAnchor, int* assertionCount)
{
    if (assertionCount) {
        ++(*assertionCount);
    }
    if (!redmcsbAnchor || redmcsbAnchor[0] == '\0') {
        printf("FAIL %s missing ReDMCSB anchor\n", label);
        return 0;
    }
    if (!(left < right)) {
        printf("FAIL %s left=%d right=%d anchor=%s\n", label, left, right,
               redmcsbAnchor);
        return 0;
    }
    printf("ok %s left=%d right=%d anchor=%s\n", label, left, right,
           redmcsbAnchor);
    return 1;
}

static int test_full_order(
    const DM1_V1_InventoryChestCloseRecompactionTracePc34* trace,
    int* assertionCount)
{
    const char* f0334F0163 =
        "ReDMCSB CHEST.C F0334 lines 112-132; DUNGEON.C F0163 lines 1796-1837";
    const int want[DM1_PC34_CHEST_CLOSE_RECOMPACTION_SLOT_COUNT] = {
        305, 101, 404, 202, 808, 707, 606, 505
    };
    int ok = 1;
    int i;

    ok &= expect_int("full order head is first G0425 non-empty slot",
                     trace->chainHead, want[0], f0334F0163, assertionCount);
    ok &= expect_int("full order rewrites eight visible slots",
                     trace->chainCount,
                     DM1_PC34_CHEST_CLOSE_RECOMPACTION_SLOT_COUNT,
                     f0334F0163, assertionCount);
    ok &= expect_int("full order terminates with END",
                     trace->chainTerminator,
                     DM1_PC34_CHEST_CLOSE_RECOMPACTION_THING_ENDOFLIST,
                     f0334F0163, assertionCount);
    ok &= expect_int("full order leaks no NONE sentinel",
                     trace->chainContainsNone, 0, f0334F0163,
                     assertionCount);
    for (i = 0; i < DM1_PC34_CHEST_CLOSE_RECOMPACTION_SLOT_COUNT; ++i) {
        ok &= expect_int("full order chain follows G0425 slot order",
                         trace->chain[i], want[i], f0334F0163,
                         assertionCount);
    }
    ok &= expect_int("full order first slot guard flips at slot zero",
                     trace->processFirstFalseSlot, 0, f0334F0163,
                     assertionCount);
    ok &= expect_int("full order first thing sentinel assignment observed",
                     trace->firstThingNextAfterSentinel,
                     DM1_PC34_CHEST_CLOSE_RECOMPACTION_THING_ENDOFLIST,
                     f0334F0163, assertionCount);
    ok &= expect_less("full order first sentinel precedes first F0163 append",
                      trace->firstNextSentinelEvent,
                      trace->firstLinkCallEvent, f0334F0163,
                      assertionCount);
    ok &= expect_int("full order first thing later points to second link",
                     trace->firstThingNextAfterClose, want[1], f0334F0163,
                     assertionCount);
    ok &= expect_less("full order container END marker precedes head overwrite",
                      trace->endMarkerEvent, trace->headAssignEvent,
                      f0334F0163, assertionCount);
    ok &= expect_int("full order container END marker value",
                     trace->containerSlotAfterEndMarker,
                     DM1_PC34_CHEST_CLOSE_RECOMPACTION_THING_ENDOFLIST,
                     f0334F0163, assertionCount);
    ok &= expect_int("full order head overwrites container END marker",
                     trace->containerSlotAfterFirstHead, want[0],
                     f0334F0163, assertionCount);
    ok &= expect_int("full order close resets G0426 open chest pointer",
                     trace->openChestAfterReset,
                     DM1_PC34_CHEST_CLOSE_RECOMPACTION_THING_NONE,
                     f0334F0163, assertionCount);
    ok &= expect_int("full order second close is no-op",
                     trace->secondCloseNoOp, 1, f0334F0163,
                     assertionCount);
    ok &= expect_int("full order second close does not append links",
                     trace->secondCloseLinkCallDelta, 0, f0334F0163,
                     assertionCount);
    ok &= expect_int("full order G0426 remains NONE after second close",
                     trace->openChestAfterSecondClose,
                     DM1_PC34_CHEST_CLOSE_RECOMPACTION_THING_NONE,
                     f0334F0163, assertionCount);

    return ok;
}

static int test_sparse_order(
    const DM1_V1_InventoryChestCloseRecompactionTracePc34* trace,
    int* assertionCount)
{
    const char* f0334F0163 =
        "ReDMCSB CHEST.C F0334 lines 112-132; DUNGEON.C F0163 lines 1796-1837";
    const int want[4] = { 110, 220, 330, 440 };
    int ok = 1;
    int i;

    ok &= expect_int("sparse order head skips no-empty prefix",
                     trace->chainHead, want[0], f0334F0163, assertionCount);
    ok &= expect_int("sparse order closes four non-empty slots",
                     trace->chainCount, 4, f0334F0163, assertionCount);
    ok &= expect_int("sparse order terminates with END",
                     trace->chainTerminator,
                     DM1_PC34_CHEST_CLOSE_RECOMPACTION_THING_ENDOFLIST,
                     f0334F0163, assertionCount);
    ok &= expect_int("sparse order leaks no NONE entries",
                     trace->chainContainsNone, 0, f0334F0163,
                     assertionCount);
    for (i = 0; i < 4; ++i) {
        ok &= expect_int("sparse order concatenates non-empty slots only",
                         trace->chain[i], want[i], f0334F0163,
                         assertionCount);
    }
    ok &= expect_int("sparse order uses F0163 for second non-empty slot",
                     trace->linkThingArgs[0], want[1], f0334F0163,
                     assertionCount);
    ok &= expect_int("sparse order carries previous A into first append",
                     trace->linkPreviousArgs[0], want[0], f0334F0163,
                     assertionCount);
    ok &= expect_int("sparse order uses F0163 for third non-empty slot",
                     trace->linkThingArgs[1], want[2], f0334F0163,
                     assertionCount);
    ok &= expect_int("sparse order carries previous B into second append",
                     trace->linkPreviousArgs[1], want[1], f0334F0163,
                     assertionCount);
    ok &= expect_int("sparse order uses F0163 for fourth non-empty slot",
                     trace->linkThingArgs[2], want[3], f0334F0163,
                     assertionCount);
    ok &= expect_int("sparse order carries previous C into third append",
                     trace->linkPreviousArgs[2], want[2], f0334F0163,
                     assertionCount);
    ok &= expect_int("sparse order calls F0163 once per subsequent slot",
                     trace->linkCallCount, 3, f0334F0163, assertionCount);

    return ok;
}

static int test_guard_order(
    const DM1_V1_InventoryChestCloseRecompactionTracePc34* trace,
    int* assertionCount)
{
    const char* f0334F0163 =
        "ReDMCSB CHEST.C F0334 lines 112-132; DUNGEON.C F0163 lines 1796-1837";
    int ok = 1;

    ok &= expect_int("guard slot zero starts process-first TRUE",
                     trace->processFirstBefore[0], 1, f0334F0163,
                     assertionCount);
    ok &= expect_int("guard empty slot zero preserves process-first TRUE",
                     trace->processFirstAfter[0], 1, f0334F0163,
                     assertionCount);
    ok &= expect_int("guard empty slot one still process-first TRUE",
                     trace->processFirstAfter[1], 1, f0334F0163,
                     assertionCount);
    ok &= expect_int("guard first non-empty sees TRUE before transition",
                     trace->processFirstBefore[2], 1, f0334F0163,
                     assertionCount);
    ok &= expect_int("guard first non-empty transitions to FALSE",
                     trace->processFirstAfter[2], 0, f0334F0163,
                     assertionCount);
    ok &= expect_int("guard subsequent slot observes FALSE",
                     trace->processFirstBefore[3], 0, f0334F0163,
                     assertionCount);
    ok &= expect_int("guard transition recorded on first non-empty slot",
                     trace->processFirstFalseSlot, 2, f0334F0163,
                     assertionCount);
    ok &= expect_less("guard FALSE transition precedes first F0163 append",
                      trace->processFirstFalseEvent,
                      trace->firstLinkCallEvent, f0334F0163,
                      assertionCount);

    return ok;
}

static int test_overfull_order(
    const DM1_V1_InventoryChestCloseRecompactionTracePc34* trace,
    int* assertionCount)
{
    const char* f0334F0163 =
        "ReDMCSB CHEST.C F0334 lines 112-132; DUNGEON.C F0163 lines 1796-1837";
    const int want[DM1_PC34_CHEST_CLOSE_RECOMPACTION_SLOT_COUNT] = {
        610, 620, 630, 640, 650, 660, 670, 680
    };
    int ok = 1;
    int i;

    ok &= expect_int("overfull first slot begins with nested tail",
                     trace->firstThingNextBefore, 690, f0334F0163,
                     assertionCount);
    ok &= expect_int("overfull first slot nested tail is cut to END first",
                     trace->firstThingNextAfterSentinel,
                     DM1_PC34_CHEST_CLOSE_RECOMPACTION_THING_ENDOFLIST,
                     f0334F0163, assertionCount);
    ok &= expect_int("overfull close rewrites eight flat links",
                     trace->chainCount,
                     DM1_PC34_CHEST_CLOSE_RECOMPACTION_SLOT_COUNT,
                     f0334F0163, assertionCount);
    for (i = 0; i < DM1_PC34_CHEST_CLOSE_RECOMPACTION_SLOT_COUNT; ++i) {
        ok &= expect_int("overfull chain excludes nested ninth thing",
                         trace->chain[i], want[i], f0334F0163,
                         assertionCount);
    }
    ok &= expect_int("overfull nested ninth thing absent from chain",
                     trace->nestedThingInChain, 0, f0334F0163,
                     assertionCount);
    ok &= expect_int("overfull chain terminates with END",
                     trace->chainTerminator,
                     DM1_PC34_CHEST_CLOSE_RECOMPACTION_THING_ENDOFLIST,
                     f0334F0163, assertionCount);
    ok &= expect_int("overfull first thing links flat to slot one",
                     trace->firstThingNextAfterClose, want[1],
                     f0334F0163, assertionCount);

    return ok;
}

static int test_empty_order(
    const DM1_V1_InventoryChestCloseRecompactionTracePc34* trace,
    int* assertionCount)
{
    const char* f0334F0163 =
        "ReDMCSB CHEST.C F0334 lines 112-132; DUNGEON.C F0163 lines 1796-1837";
    int ok = 1;

    ok &= expect_int("empty close leaves container slot END",
                     trace->chainHead,
                     DM1_PC34_CHEST_CLOSE_RECOMPACTION_THING_ENDOFLIST,
                     f0334F0163, assertionCount);
    ok &= expect_int("empty close writes no chain entries",
                     trace->chainCount, 0, f0334F0163, assertionCount);
    ok &= expect_int("empty close terminates with END",
                     trace->chainTerminator,
                     DM1_PC34_CHEST_CLOSE_RECOMPACTION_THING_ENDOFLIST,
                     f0334F0163, assertionCount);
    ok &= expect_int("empty close calls no F0163 append",
                     trace->linkCallCount, 0, f0334F0163,
                     assertionCount);
    ok &= expect_int("empty close still resets G0426",
                     trace->openChestAfterReset,
                     DM1_PC34_CHEST_CLOSE_RECOMPACTION_THING_NONE,
                     f0334F0163, assertionCount);
    ok &= expect_int("empty close second call remains no-op",
                     trace->secondCloseNoOp, 1, f0334F0163,
                     assertionCount);

    return ok;
}

static int test_base_weight(
    const DM1_V1_InventoryChestCloseRecompactionProbePc34* probe,
    int* assertionCount)
{
    const char* f0334F0301F0140 =
        "ReDMCSB CHEST.C F0334 lines 112-132; CHAMPION.C F0301 lines 609-614; DUNGEON.C F0140 lines 1114-1120";
    int ok = 1;

    ok &= expect_int("base weight close keeps two child containers",
                     probe->baseWeightCloseCount, 2, f0334F0301F0140,
                     assertionCount);
    ok &= expect_int("base weight first child includes container base",
                     probe->baseWeightFirstChildWeight, 57,
                     f0334F0301F0140, assertionCount);
    ok &= expect_int("base weight second child includes container base",
                     probe->baseWeightSecondChildWeight, 61,
                     f0334F0301F0140, assertionCount);
    ok &= expect_int("base weight raw contents alone stay below load",
                     probe->baseWeightContentsOnly, 18,
                     f0334F0301F0140, assertionCount);
    ok &= expect_int("base weight parent includes both container bases",
                     probe->baseWeightParentContainerWeight, 168,
                     f0334F0301F0140, assertionCount);
    ok &= expect_int("base weight expected champion load",
                     probe->baseWeightExpectedChampionLoad, 168,
                     f0334F0301F0140, assertionCount);
    ok &= expect_int("base weight F0301 champion load uses F0140",
                     probe->baseWeightChampionLoadAfterF0301, 168,
                     f0334F0301F0140, assertionCount);
    ok &= expect_int("base weight F0301 called once",
                     probe->baseWeightF0301Calls, 1, f0334F0301F0140,
                     assertionCount);

    return ok;
}

int main(void)
{
    const char* f0334F0163 =
        "ReDMCSB CHEST.C F0334 lines 112-132; DUNGEON.C F0163 lines 1796-1837";
    DM1_V1_InventoryChestCloseRecompactionProbePc34 probe;
    int assertionCount = 0;
    int ok = 1;

    printf("probe=dm1_v1_inventory_chest_close_recompaction_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           dm1_inventory_chest_close_recompaction_source_evidence_pc34());

    ok &= expect_int("probe setup",
                     m11_inventory_pc34_probe_chest_close_recompaction(&probe),
                     1, f0334F0163, &assertionCount);
    if (!ok) {
        printf("assertionCount=%d\n", assertionCount);
        printf("inventoryChestCloseRecompactionInvariantOk=0\n");
        return 1;
    }

    ok &= test_full_order(&probe.fullOrder, &assertionCount);
    ok &= test_sparse_order(&probe.sparseOrder, &assertionCount);
    ok &= test_guard_order(&probe.guardOrder, &assertionCount);
    ok &= test_base_weight(&probe, &assertionCount);
    ok &= test_overfull_order(&probe.overfullOrder, &assertionCount);
    ok &= test_empty_order(&probe.emptyOrder, &assertionCount);

    ok &= expect_int("minimum assertion budget",
                     assertionCount >= 40 ? 1 : 0, 1, f0334F0163,
                     &assertionCount);

    printf("assertionCount=%d\n", assertionCount);
    printf("inventoryChestCloseRecompactionInvariantOk=%d\n", ok ? 1 : 0);
    return ok ? 0 : 1;
}

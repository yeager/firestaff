#include "dm1_v1_chest_cross_champion_hand_swap_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static M11_GameView_ChestCrossChampionHandSwapProbePc34 g_probe;
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
    printf("ok %s=%d anchor=%s\n", label, got, redmcsbAnchor);
    return 1;
}

static int expect_contains(const char* label,
                           const char* haystack,
                           const char* needle,
                           const char* redmcsbAnchor)
{
    ++g_assertions;
    if (!redmcsbAnchor || redmcsbAnchor[0] == '\0') {
        ++g_failures;
        printf("FAIL %s missing ReDMCSB anchor\n", label);
        return 0;
    }
    if (!haystack || !needle || !strstr(haystack, needle)) {
        ++g_failures;
        printf("FAIL %s missing=%s anchor=%s\n",
               label, needle ? needle : "(null)", redmcsbAnchor);
        return 0;
    }
    printf("ok %s contains=%s anchor=%s\n", label, needle, redmcsbAnchor);
    return 1;
}

static int expect_case_name(int caseIndex)
{
    const char* name =
        M11_GameView_ChestCrossChampionHandSwapCaseNamePc34(caseIndex);

    ++g_assertions;
    if (!name || name[0] == '\0') {
        ++g_failures;
        printf("FAIL case name %d empty anchor=ReDMCSB CHEST.C F0333:43\n",
               caseIndex);
        return 0;
    }
    printf("ok case name %d=%s anchor=ReDMCSB CHEST.C F0333:43\n",
           caseIndex, name);
    return 1;
}

static int expect_slot_arrays(const char* label,
                              const int* got,
                              const int* want,
                              int linkedCount,
                              const char* redmcsbAnchor)
{
    int i;
    int ok = 1;

    for (i = 0; i < DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_SLOT_COUNT; ++i) {
        char slotLabel[128];
        int expected = i < linkedCount ? want[i] :
            DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_THING_NONE;

        snprintf(slotLabel, sizeof(slotLabel), "%s slot C%d", label,
                 DM1_PC34_SLOT_CHEST_1 + i);
        ok &= expect_int(slotLabel, got[i], expected, redmcsbAnchor);
    }
    return ok;
}

static int expect_weight_arrays(const char* label,
                                const int* got,
                                const int* want,
                                int linkedCount,
                                const char* redmcsbAnchor)
{
    int i;
    int ok = 1;

    for (i = 0; i < DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_SLOT_COUNT; ++i) {
        char slotLabel[128];
        int expected = i < linkedCount ? want[i] : 0;

        snprintf(slotLabel, sizeof(slotLabel), "%s weight C%d", label,
                 DM1_PC34_SLOT_CHEST_1 + i);
        ok &= expect_int(slotLabel, got[i], expected, redmcsbAnchor);
    }
    return ok;
}

static int test_source_evidence(void)
{
    const char* evidence =
        M11_GameView_ChestCrossChampionHandSwapSourceEvidencePc34();
    int ok = 1;

    ok &= expect_contains("evidence same open", evidence,
                          "CHEST.C F0333:30-32",
                          "ReDMCSB CHEST.C F0333 lines 30-32");
    ok &= expect_contains("evidence open path", evidence,
                          "CHEST.C F0333:43",
                          "ReDMCSB CHEST.C F0333 line 43");
    ok &= expect_contains("evidence open materialize", evidence,
                          "F0333:53-67",
                          "ReDMCSB CHEST.C F0333 lines 53-67");
    ok &= expect_contains("evidence close rewrite", evidence,
                          "CHEST.C F0334:113-132",
                          "ReDMCSB CHEST.C F0334 lines 113-132");
    ok &= expect_contains("evidence leader hand", evidence,
                          "CHAMPION.C F0297:243-298",
                          "ReDMCSB CHAMPION.C F0297 lines 243-298");
    ok &= expect_contains("evidence F0300", evidence,
                          "F0300:511-515",
                          "ReDMCSB CHAMPION.C F0300 lines 511-515");
    ok &= expect_contains("evidence F0301", evidence,
                          "F0301:606-614",
                          "ReDMCSB CHAMPION.C F0301 lines 606-614");
    ok &= expect_contains("evidence F0302", evidence,
                          "F0302:688-710",
                          "ReDMCSB CHAMPION.C F0302 lines 688-710");
    ok &= expect_contains("evidence dungeon", evidence,
                          "DUNGEON.C F0163:1796-1837",
                          "ReDMCSB DUNGEON.C F0163 lines 1796-1837");
    ok &= expect_contains("evidence command", evidence,
                          "COMMAND.C F0359:1985-1990",
                          "ReDMCSB COMMAND.C F0359 lines 1985-1990");
    ok &= expect_contains("evidence panel", evidence,
                          "PANEL.C F0354:2208-2240",
                          "ReDMCSB PANEL.C F0354 lines 2208-2240");
    ok &= expect_contains("evidence defs", evidence,
                          "DEFS.H:2088",
                          "ReDMCSB DEFS.H line 2088");
    ok &= expect_contains("evidence C30", evidence, "C30",
                          "ReDMCSB DEFS.H C30/G0425/G0426/G0423/G0305");
    ok &= expect_contains("evidence G0425", evidence, "G0425",
                          "ReDMCSB DEFS.H C30/G0425/G0426/G0423/G0305");
    ok &= expect_contains("evidence G0426", evidence, "G0426",
                          "ReDMCSB DEFS.H C30/G0425/G0426/G0423/G0305");
    ok &= expect_contains("evidence G0423", evidence, "G0423",
                          "ReDMCSB DEFS.H C30/G0425/G0426/G0423/G0305");
    ok &= expect_contains("evidence G0305", evidence, "G0305",
                          "ReDMCSB DEFS.H C30/G0425/G0426/G0423/G0305");
    ok &= expect_contains("evidence blit", evidence,
                          "BLITMASK.C F0133:30-33",
                          "ReDMCSB BLITMASK.C F0133 lines 30-33");
    return ok;
}

static int test_probe_spec(void)
{
    const char* defs =
        "ReDMCSB DEFS.H:2088 C10_COLOR_FLESH/C30/G0425/G0426/G0423/G0305";
    int ok = 1;
    int i;

    ok &= expect_int("runtime gate marker",
                     g_probe.sourceLockedRuntimeGate, 1,
                     "ReDMCSB CHEST.C F0333 lines 43,53-67");
    ok &= expect_int("C10 color flesh", g_probe.c10ColorFlesh, 10, defs);
    ok &= expect_int("C30 chest slot base",
                     g_probe.c30ChestSlotBase, DM1_PC34_SLOT_CHEST_1, defs);
    ok &= expect_int("G0425 slot count",
                     g_probe.g0425SlotCount,
                     DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_SLOT_COUNT,
                     defs);
    ok &= expect_int("G0426 none sentinel",
                     g_probe.g0426NoneSentinel,
                     DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_THING_NONE,
                     defs);
    ok &= expect_int("G0423 inventory champion starts A",
                     g_probe.g0423InventoryChampionOrdinalStart, 1, defs);
    ok &= expect_int("G0305 party champion count",
                     g_probe.g0305PartyChampionCount, 4, defs);
    ok &= expect_int("case count", g_probe.caseCount,
                     DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_CASE_COUNT,
                     defs);
    for (i = 0; i < g_probe.caseCount; ++i) {
        ok &= expect_case_name(i);
    }
    return ok;
}

static int test_common_case(
    const M11_GameView_ChestCrossChampionHandSwapCasePc34* c)
{
    const char* f0333 =
        "ReDMCSB CHEST.C F0333 lines 30-32,43,53-67";
    const char* f0334 = "ReDMCSB CHEST.C F0334 lines 113-132";
    const char* f0297 = "ReDMCSB CHAMPION.C F0297 lines 243-298";
    const char* f0300 =
        "ReDMCSB CHAMPION.C F0300/F0301 lines 511-515,606-614";
    const char* f0302 = "ReDMCSB CHAMPION.C F0302 lines 688-710";
    const char* dungeon = "ReDMCSB DUNGEON.C F0163 lines 1796-1837";
    const char* command = "ReDMCSB COMMAND.C F0359 lines 1985-1990";
    const char* panel = "ReDMCSB PANEL.C F0354 lines 2208-2240";
    const char* defs =
        "ReDMCSB DEFS.H:2088 C10_COLOR_FLESH/C30/G0425/G0426/G0423/G0305";
    const char* blit = "ReDMCSB BLITMASK.C F0133 lines 30-33";
    int ok = 1;

    ok &= expect_int("context original leader A",
                     c->context.originalLeaderOrdinal, 1, defs);
    ok &= expect_int("context rotated leader B",
                     c->context.rotatedLeaderOrdinal, 2, command);
    ok &= expect_int("context inventory champion A",
                     c->context.inventoryChampionOrdinal, 1, defs);
    ok &= expect_int("context champion A ordinal",
                     c->context.championAOrdinal, 1, defs);
    ok &= expect_int("context champion B ordinal",
                     c->context.championBOrdinal, 2, defs);

    ok &= expect_int("action log starts with open",
                     c->actionLog.entries[0].action,
                     M11_DM1_PC34_CHEST_CROSS_CHAMPION_ACTION_OPEN_CHEST,
                     f0333);
    ok &= expect_int("action log has close",
                     c->actionLog.entries[c->actionLog.count - 2].action,
                     M11_DM1_PC34_CHEST_CROSS_CHAMPION_ACTION_CLOSE_CHEST,
                     f0334);
    ok &= expect_int("action log ends with reopen",
                     c->actionLog.entries[c->actionLog.count - 1].action,
                     M11_DM1_PC34_CHEST_CROSS_CHAMPION_ACTION_REOPEN_CHEST,
                     f0333);
    ok &= expect_int("open result", c->expected.openResult, 1, f0333);
    ok &= expect_int("open chest thing after open",
                     c->expected.openChestThingAfterOpen,
                     c->context.chestThing, f0333);
    ok &= expect_int("visible count after open",
                     c->expected.visibleCountAfterOpen,
                     c->context.linkedCount, f0333);
    ok &= expect_slot_arrays("opened G0425",
                             c->expected.openedSlotThings,
                             c->context.chestLinkedThings,
                             c->context.linkedCount, f0333);
    ok &= expect_weight_arrays("opened G0425",
                               c->expected.openedSlotWeights,
                               c->context.chestLinkedWeights,
                               c->context.linkedCount, f0297);

    ok &= expect_int("pickup A result",
                     c->expected.pickupAResult, 1, f0297);
    ok &= expect_int("leader hand after pickup is A object",
                     c->expected.leaderHandAfterPickup,
                     c->context.championAInitialHandThing, f0297);
    ok &= expect_int("champion A hand after pickup empty",
                     c->expected.championAHandAfterPickup,
                     DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_THING_NONE,
                     f0297);
    ok &= expect_int("leader ordinal after rotation",
                     c->expected.leaderOrdinalAfterRotation,
                     c->context.rotatedLeaderOrdinal, command);
    ok &= expect_int("inventory ordinal preserved during rotation",
                     c->expected.inventoryChampionOrdinalDuringRotation,
                     c->context.inventoryChampionOrdinal, defs);
    ok &= expect_int("open chest remains during rotation",
                     c->expected.openChestThingDuringRotation,
                     c->context.chestThing, command);
    ok &= expect_int("map X invariant",
                     c->expected.mapXAfterRotation,
                     c->context.mapX, dungeon);
    ok &= expect_int("map Y invariant",
                     c->expected.mapYAfterRotation,
                     c->context.mapY, dungeon);
    ok &= expect_int("facing invariant",
                     c->expected.facingAfterRotation,
                     c->context.facing, dungeon);
    ok &= expect_int("portrait switch count positive",
                     c->expected.portraitSwitchCount > 0 ? 1 : 0,
                     1, panel);
    ok &= expect_int("redraw cadence count positive",
                     c->expected.redrawCadenceCount > 0 ? 1 : 0,
                     1, panel);

    ok &= expect_int("swap with B result",
                     c->expected.swapWithBResult, 1, f0302);
    ok &= expect_int("remove leader hand call",
                     c->expected.removedLeaderHandCall, 1, f0302);
    ok &= expect_int("remove champion B slot call",
                     c->expected.removedChampionBSlotCall, 1, f0302);
    ok &= expect_int("put B object in leader hand call",
                     c->expected.putBObjectInLeaderHandCall, 1, f0297);
    ok &= expect_int("put A object in champion B slot call",
                     c->expected.putAObjectInChampionBSlotCall, 1, f0302);
    ok &= expect_int("leader hand after swap is B object",
                     c->expected.leaderHandAfterSwap,
                     c->context.championBInitialHandThing, f0297);
    ok &= expect_int("champion A hand after swap empty",
                     c->expected.championAHandAfterSwap,
                     DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_THING_NONE,
                     f0297);
    ok &= expect_int("champion B hand after swap is A object",
                     c->expected.championBHandAfterSwap,
                     c->context.championAInitialHandThing, f0302);

    ok &= expect_int("close result", c->expected.closeResult, 1, f0334);
    ok &= expect_int("close count",
                     c->expected.closeCount,
                     c->context.linkedCount, f0334);
    ok &= expect_int("open chest thing after close",
                     c->expected.openChestThingAfterClose,
                     DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_THING_NONE,
                     f0334);
    ok &= expect_int("closed link head",
                     c->expected.closedLinkHead,
                     c->context.linkedCount > 0 ?
                         c->context.chestLinkedThings[0] :
                         DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_END_OF_LIST,
                     f0334);
    ok &= expect_int("closed link tail",
                     c->expected.closedLinkTail,
                     c->context.chestLinkedThings[c->context.linkedCount - 1],
                     f0334);
    ok &= expect_slot_arrays("closed link order",
                             c->expected.closedLinkThings,
                             c->context.chestLinkedThings,
                             c->context.linkedCount, f0334);
    ok &= expect_weight_arrays("closed link order",
                               c->expected.closedLinkWeights,
                               c->context.chestLinkedWeights,
                               c->context.linkedCount, f0334);
    ok &= expect_slot_arrays("G0425 cleared after close",
                             c->expected.slotsClearedAfterClose,
                             c->expected.slotsClearedAfterClose,
                             0, f0300);

    ok &= expect_int("reopen result", c->expected.reopenResult, 1, f0333);
    ok &= expect_int("open chest thing after reopen",
                     c->expected.openChestThingAfterReopen,
                     c->context.chestThing, f0333);
    ok &= expect_int("reopened visible count",
                     c->expected.reopenedVisibleCount,
                     c->context.linkedCount, f0333);
    ok &= expect_slot_arrays("reopened G0425",
                             c->expected.reopenedSlotThings,
                             c->context.chestLinkedThings,
                             c->context.linkedCount, f0333);
    ok &= expect_weight_arrays("reopened G0425",
                               c->expected.reopenedSlotWeights,
                               c->context.chestLinkedWeights,
                               c->context.linkedCount, f0333);

    ok &= expect_int("link order preserved",
                     c->expected.linkOrderPreserved, 1, f0334);
    ok &= expect_int("G0425 view matches closed links",
                     c->expected.g0425ViewMatchesClosedLinks, 1, f0333);
    ok &= expect_int("leader hand identity preserved",
                     c->expected.leaderHandIdentityPreserved, 1, f0297);
    ok &= expect_int("inventory champion identity preserved",
                     c->expected.inventoryChampionIdentityPreserved, 1, defs);
    ok &= expect_int("inventory champion hand coherent",
                     c->expected.inventoryChampionHandStateCoherent, 1,
                     f0297);
    ok &= expect_int("cross champion swap coherent",
                     c->expected.crossChampionSwapCoherent, 1, f0302);
    ok &= expect_int("dungeon position invariant",
                     c->expected.dungeonPositionInvariant, 1, dungeon);
    ok &= expect_int("no detached C30+ occupant",
                     c->expected.noDetachedC30PlusOccupant, 1, f0300);
    ok &= expect_int("sparse clear coherent",
                     c->expected.sparseClearCoherent, 1, f0334);
    ok &= expect_int("same-open noop flag coherent",
                     c->expected.sameOpenNoopPreserved, 1, f0333);
    ok &= expect_int("blit routing presentation only",
                     c->expected.blitRoutingPresentationOnly, 1, blit);
    ok &= expect_int("C10 color flesh known",
                     c->expected.c10ColorFleshKnown, 10, defs);
    return ok;
}

static int test_case_variants(void)
{
    const char* f0333 =
        "ReDMCSB CHEST.C F0333 lines 30-32,43,53-67";
    const char* f0359 = "ReDMCSB COMMAND.C F0359 lines 1985-1990";
    int ok = 1;
    int i;

    for (i = 0; i < g_probe.caseCount; ++i) {
        ok &= test_common_case(&g_probe.cases[i]);
    }

    ok &= expect_int("sparse case linked count",
                     g_probe.cases[
                         DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_CASE_SPARSE_FIVE]
                         .context.linkedCount,
                     5, "ReDMCSB CHEST.C F0334 lines 113-132");
    ok &= expect_int("same-open case requested noop",
                     g_probe.cases[
                         DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_CASE_SAME_OPEN_NOOP]
                         .context.sameOpenBeforeSwap,
                     1, f0333);
    ok &= expect_int("same-open action recorded",
                     g_probe.cases[
                         DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_CASE_SAME_OPEN_NOOP]
                         .actionLog.entries[1].action,
                     M11_DM1_PC34_CHEST_CROSS_CHAMPION_ACTION_SAME_OPEN_NOOP,
                     f0333);
    ok &= expect_int("same-open result",
                     g_probe.cases[
                         DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_CASE_SAME_OPEN_NOOP]
                         .expected.sameOpenNoopResult,
                     1, f0333);
    ok &= expect_int("double rotation requested",
                     g_probe.cases[
                         DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_CASE_DOUBLE_ROTATION]
                         .context.rotateTwiceWhileOpen,
                     1, f0359);
    ok &= expect_int("double rotation count",
                     g_probe.cases[
                         DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_CASE_DOUBLE_ROTATION]
                         .expected.rotationCountWhileOpen,
                     3, f0359);
    ok &= expect_int("double rotation final leader B",
                     g_probe.cases[
                         DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_CASE_DOUBLE_ROTATION]
                         .expected.leaderOrdinalAfterRotation,
                     2, f0359);
    ok &= expect_int("inventory A case still inventory A",
                     g_probe.cases[
                         DM1_PC34_CHEST_CROSS_CHAMPION_HAND_SWAP_CASE_INVENTORY_A]
                         .expected.inventoryChampionOrdinalDuringRotation,
                     1,
                     "ReDMCSB DEFS.H G0423/G0305");
    return ok;
}

int main(void)
{
    int ok = 1;

    printf("probe=dm1_v1_chest_cross_champion_hand_swap_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           M11_GameView_ChestCrossChampionHandSwapSourceEvidencePc34());

    ok &= expect_int(
        "probe run",
        M11_GameView_ChestCrossChampionHandSwapRunPc34(&g_probe), 1,
        "ReDMCSB CHEST.C F0333 lines 43,53-67");
    if (!ok) {
        printf("assertionCount=%d failures=%d\n", g_assertions, g_failures);
        return 1;
    }

    ok &= test_source_evidence();
    ok &= test_probe_spec();
    ok &= test_case_variants();
    ok &= expect_int("minimum assertion budget",
                     g_assertions >= 200 ? 1 : 0, 1,
                     "ReDMCSB CHEST.C F0333/F0334 and CHAMPION.C F0302");

    printf("assertionCount=%d failures=%d\n", g_assertions, g_failures);
    printf("chestCrossChampionHandSwapResultOk=%d cases=%d finalLeader=%d "
           "leaderHand=%d championBHand=%d\n",
           ok && g_failures == 0 ? 1 : 0,
           g_probe.caseCount,
           g_probe.cases[0].expected.leaderOrdinalAfterRotation,
           g_probe.cases[0].expected.leaderHandAfterSwap,
           g_probe.cases[0].expected.championBHandAfterSwap);

    if (!ok || g_failures != 0) {
        printf("FAIL dm1_v1_chest_cross_champion_hand_swap_pc34_compat "
               "assertions=%d failures=%d\n",
               g_assertions, g_failures);
        return 1;
    }
    printf("PASS dm1_v1_chest_cross_champion_hand_swap_pc34_compat "
           "assertions=%d\n",
           g_assertions);
    return 0;
}

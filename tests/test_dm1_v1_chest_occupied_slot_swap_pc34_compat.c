#include "dm1_v1_chest_occupied_slot_swap_pc34_compat.h"

#include <stdio.h>
#include <string.h>

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

static int expect_contains(const char* label,
                           const char* got,
                           const char* needle,
                           const char* redmcsbAnchor)
{
    ++g_assertions;
    if (!redmcsbAnchor || redmcsbAnchor[0] == '\0') {
        ++g_failures;
        printf("FAIL %s missing ReDMCSB anchor\n", label);
        return 0;
    }
    if (!got || !needle || !strstr(got, needle)) {
        ++g_failures;
        printf("FAIL %s missing=%s anchor=%s\n",
               label, needle ? needle : "(null)", redmcsbAnchor);
        return 0;
    }
    printf("PASS %s contains=%s anchor=%s\n",
           label, needle, redmcsbAnchor);
    return 1;
}

static int expect_empty_tail(const char* phase,
                             const int* types,
                             const char* redmcsbAnchor)
{
    int ok = 1;
    int i;

    for (i = 3; i < DM1_PC34_CHEST_OCCUPIED_SWAP_SLOT_COUNT; ++i) {
        char label[96];

        snprintf(label, sizeof(label), "%s C%d empty",
                 phase, DM1_PC34_CHEST_OCCUPIED_SWAP_C537_ORDINAL + i);
        ok &= expect_int(label, types[i], 0, redmcsbAnchor);
    }
    return ok;
}

static int test_spec(void)
{
    const char* defs =
        "ReDMCSB DEFS.H lines 810-817,1878,3906-3913";
    const char* f0302 =
        "ReDMCSB CHAMPION.C F0302 lines 662-713";
    const DM1_V1_ChestOccupiedSlotSwapSpecPc34* spec =
        dm1_v1_chest_occupied_slot_swap_spec_pc34();
    int ok = 1;

    ok &= expect_contains("contract marker", spec->contractMarker,
                          "pass706", f0302);
    ok &= expect_int("spec G0305 party count",
                     spec->g0305PartyChampionCount, 1, defs);
    ok &= expect_int("spec G0423 ordinal",
                     spec->g0423InventoryChampionOrdinal, 1, defs);
    ok &= expect_int("spec C537 ordinal", spec->c537Ordinal,
                     DM1_PC34_CHEST_OCCUPIED_SWAP_C537_ORDINAL, defs);
    ok &= expect_int("spec C538 ordinal", spec->c538Ordinal,
                     DM1_PC34_CHEST_OCCUPIED_SWAP_C538_ORDINAL, defs);
    ok &= expect_int("spec C544 ordinal", spec->c544Ordinal,
                     DM1_PC34_CHEST_OCCUPIED_SWAP_C544_ORDINAL, defs);
    ok &= expect_int("spec C538 pc34 slot", spec->c538Pc34Slot,
                     DM1_PC34_SLOT_CHEST_2, defs);
    ok &= expect_int("spec C538 G0425 index", spec->c538G0425Index,
                     DM1_PC34_CHEST_OCCUPIED_SWAP_C538_INDEX, defs);
    ok &= expect_int("spec M070 ready hand",
                     spec->m070ReadyHandSlotIndex, 0, defs);
    ok &= expect_int("spec M070 action hand",
                     spec->m070ActionHandSlotIndex, 1, defs);
    ok &= expect_int("spec visible slots", spec->visibleSlotCount,
                     DM1_PC34_CHEST_OCCUPIED_SWAP_SLOT_COUNT, defs);
    ok &= expect_int("spec old stack type", spec->oldStackType,
                     DM1_PC34_CHEST_OCCUPIED_SWAP_C538_STACK, f0302);
    ok &= expect_int("spec old stack count", spec->oldStackCount,
                     DM1_PC34_CHEST_OCCUPIED_SWAP_C538_COUNT, f0302);
    ok &= expect_int("spec leader stack type", spec->leaderStackType,
                     DM1_PC34_CHEST_OCCUPIED_SWAP_LEADER_STACK, f0302);
    ok &= expect_int("spec leader stack count", spec->leaderStackCount,
                     DM1_PC34_CHEST_OCCUPIED_SWAP_LEADER_COUNT, f0302);
    ok &= expect_contains("evidence includes F0333",
                          dm1_v1_chest_occupied_slot_swap_source_evidence_pc34(),
                          "CHEST.C F0333:30-67",
                          "ReDMCSB CHEST.C F0333 lines 30-67");
    ok &= expect_contains("evidence includes F0300",
                          dm1_v1_chest_occupied_slot_swap_source_evidence_pc34(),
                          "CHAMPION.C F0300:489-584",
                          "ReDMCSB CHAMPION.C F0300 lines 489-584");
    ok &= expect_contains("evidence includes PANEL",
                          dm1_v1_chest_occupied_slot_swap_source_evidence_pc34(),
                          "PANEL.C F0354:2299-2322",
                          "ReDMCSB PANEL.C F0354 lines 2299-2322");
    ok &= expect_contains("evidence includes UTAMSCR",
                          dm1_v1_chest_occupied_slot_swap_source_evidence_pc34(),
                          "UTAMSCR.C F0077/F0078:141-150",
                          "ReDMCSB UTAMSCR.C F0077/F0078 lines 141-150");
    ok &= expect_contains("evidence includes BLITMASK",
                          dm1_v1_chest_occupied_slot_swap_source_evidence_pc34(),
                          "BLITMASK.C F0133:30-33",
                          "ReDMCSB BLITMASK.C F0133 lines 30-33");
    ok &= expect_contains("evidence includes OBJECT",
                          dm1_v1_chest_occupied_slot_swap_source_evidence_pc34(),
                          "OBJECT.C F0033:147-212",
                          "ReDMCSB OBJECT.C F0033 lines 147-212");
    return ok;
}

static int test_setup(
    const DM1_V1_ChestOccupiedSlotSwapRuntimePc34* state,
    const DM1_V1_ChestOccupiedSlotSwapProbePc34* probe)
{
    const char* f0333 = "ReDMCSB CHEST.C F0333 lines 30-67";
    const char* f0302 = "ReDMCSB CHAMPION.C F0302 lines 662-713";
    const char* defs =
        "ReDMCSB DEFS.H lines 810-817,1878,5700,5876-5881,3906-3913";
    int ok = 1;

    ok &= expect_int("runtime G0305 party count",
                     state->g0305PartyChampionCount, 1, defs);
    ok &= expect_int("runtime G0423 ordinal",
                     state->g0423InventoryChampionOrdinal, 1, defs);
    ok &= expect_int("runtime G0426 open chest",
                     probe->openChestThingBefore,
                     DM1_PC34_CHEST_OCCUPIED_SWAP_OPEN_CHEST_THING, f0333);
    ok &= expect_int("runtime M070 ready",
                     state->m070ReadyHandSlotIndex, 0, defs);
    ok &= expect_int("runtime M070 action",
                     state->m070ActionHandSlotIndex, 1, defs);
    ok &= expect_int("setup visible count", probe->visibleCountBefore, 3,
                     f0333);
    ok &= expect_int("setup C537 stack", probe->beforeTypes[0],
                     DM1_PC34_CHEST_OCCUPIED_SWAP_C537_STACK, f0333);
    ok &= expect_int("setup C538 occupied stack", probe->beforeTypes[1],
                     DM1_PC34_CHEST_OCCUPIED_SWAP_C538_STACK, f0333);
    ok &= expect_int("setup C538 stack count", probe->beforeCounts[1],
                     DM1_PC34_CHEST_OCCUPIED_SWAP_C538_COUNT, f0333);
    ok &= expect_int("setup C538 stack weight", probe->beforeWeights[1],
                     DM1_PC34_CHEST_OCCUPIED_SWAP_C538_WEIGHT, f0333);
    ok &= expect_int("setup C539 stack", probe->beforeTypes[2],
                     DM1_PC34_CHEST_OCCUPIED_SWAP_C539_STACK, f0333);
    ok &= expect_empty_tail("setup", probe->beforeTypes, f0333);
    ok &= expect_int("setup leader stack", probe->leaderHandBeforeType,
                     DM1_PC34_CHEST_OCCUPIED_SWAP_LEADER_STACK, f0302);
    ok &= expect_int("setup leader stack count",
                     probe->leaderHandBeforeCount,
                     DM1_PC34_CHEST_OCCUPIED_SWAP_LEADER_COUNT, f0302);
    ok &= expect_int("setup leader stack allowed in C538",
                     probe->replacementAllowedInC538, 1, f0302);
    ok &= expect_int("setup source-equivalent load",
                     probe->sourceEquivalentLoadBefore,
                     DM1_PC34_CHEST_OCCUPIED_SWAP_C537_WEIGHT +
                     DM1_PC34_CHEST_OCCUPIED_SWAP_C538_WEIGHT +
                     DM1_PC34_CHEST_OCCUPIED_SWAP_C539_WEIGHT +
                     DM1_PC34_CHEST_OCCUPIED_SWAP_LEADER_WEIGHT,
                     f0302);
    return ok;
}

static int test_exercise(
    const DM1_V1_ChestOccupiedSlotSwapProbePc34* probe)
{
    const char* f0297 = "ReDMCSB CHAMPION.C F0297 lines 243-268";
    const char* f0300 = "ReDMCSB CHAMPION.C F0300 lines 489-584";
    const char* f0301 = "ReDMCSB CHAMPION.C F0301 lines 587-660";
    const char* f0302 = "ReDMCSB CHAMPION.C F0302 lines 662-713";
    const char* f0333 = "ReDMCSB CHEST.C F0333 lines 30-67";
    int ok = 1;

    ok &= expect_int("exercise result", probe->exerciseResult, 1, f0302);
    ok &= expect_int("F0302 click accepted", probe->f0302Accepted, 1,
                     f0302);
    ok &= expect_int("F0300 removed occupied C538",
                     probe->f0300RemovedOccupiedC538, 1, f0300);
    ok &= expect_int("F0297 old stack in leader hand",
                     probe->f0297PlacedOldC538InLeaderHand, 1, f0297);
    ok &= expect_int("F0301 replacement in C538",
                     probe->f0301StoredLeaderObjectInC538, 1, f0301);
    ok &= expect_int("after leader has old stack",
                     probe->leaderHandAfterType,
                     DM1_PC34_CHEST_OCCUPIED_SWAP_C538_STACK, f0297);
    ok &= expect_int("after leader old stack count",
                     probe->leaderHandAfterCount,
                     DM1_PC34_CHEST_OCCUPIED_SWAP_C538_COUNT, f0297);
    ok &= expect_int("after C538 has replacement",
                     probe->afterTypes[1],
                     DM1_PC34_CHEST_OCCUPIED_SWAP_LEADER_STACK, f0301);
    ok &= expect_int("after C538 replacement count",
                     probe->afterCounts[1],
                     DM1_PC34_CHEST_OCCUPIED_SWAP_LEADER_COUNT, f0301);
    ok &= expect_int("after C537 stable", probe->c537StableAfterClick, 1,
                     f0333);
    ok &= expect_int("after C539 stable", probe->c539StableAfterClick, 1,
                     f0333);
    ok &= expect_int("after visible count", probe->visibleCountAfterClick,
                     3, f0333);
    ok &= expect_empty_tail("after click", probe->afterTypes, f0333);
    ok &= expect_int("after old stack absent from chest",
                     probe->oldStackNoLongerInChestAfterClick, 1, f0300);
    ok &= expect_int("after replacement absent from leader",
                     probe->replacementNoLongerInLeaderHandAfterClick, 1,
                     f0301);
    ok &= expect_int("source-equivalent load unchanged",
                     probe->sourceEquivalentLoadUnchanged, 1, f0302);
    ok &= expect_int("source-equivalent load after",
                     probe->sourceEquivalentLoadAfterClick,
                     probe->sourceEquivalentLoadBefore, f0302);
    ok &= expect_int("G0426 still open after click",
                     probe->openChestThingAfterClick,
                     DM1_PC34_CHEST_OCCUPIED_SWAP_OPEN_CHEST_THING, f0333);
    return ok;
}

static int test_close_reopen(
    const DM1_V1_ChestOccupiedSlotSwapProbePc34* probe)
{
    const char* f0333 = "ReDMCSB CHEST.C F0333 lines 30-67";
    const char* f0334 = "ReDMCSB CHEST.C F0334 lines 113-132";
    int ok = 1;

    ok &= expect_int("close count", probe->closedCount, 3, f0334);
    ok &= expect_int("close cleared G0426", probe->closeClearedG0426, 1,
                     f0334);
    ok &= expect_int("close visible rewrite", probe->closeRewroteVisibleOnly,
                     1, f0334);
    ok &= expect_int("closed C537", probe->closedTypes[0],
                     DM1_PC34_CHEST_OCCUPIED_SWAP_C537_STACK, f0334);
    ok &= expect_int("closed C538 replacement", probe->closedTypes[1],
                     DM1_PC34_CHEST_OCCUPIED_SWAP_LEADER_STACK, f0334);
    ok &= expect_int("closed C539", probe->closedTypes[2],
                     DM1_PC34_CHEST_OCCUPIED_SWAP_C539_STACK, f0334);
    ok &= expect_empty_tail("closed", probe->closedTypes, f0334);
    ok &= expect_int("reopen result", probe->reopenResult, 1, f0333);
    ok &= expect_int("reopen count", probe->reopenedCount, 3, f0333);
    ok &= expect_int("reopen keeps C538 replacement",
                     probe->reopenPreservedC538Replacement, 1, f0333);
    ok &= expect_int("reopened C537", probe->reopenedTypes[0],
                     DM1_PC34_CHEST_OCCUPIED_SWAP_C537_STACK, f0333);
    ok &= expect_int("reopened C538 replacement", probe->reopenedTypes[1],
                     DM1_PC34_CHEST_OCCUPIED_SWAP_LEADER_STACK, f0333);
    ok &= expect_int("reopened C539", probe->reopenedTypes[2],
                     DM1_PC34_CHEST_OCCUPIED_SWAP_C539_STACK, f0333);
    ok &= expect_empty_tail("reopened", probe->reopenedTypes, f0333);
    return ok;
}

static int test_module_assertions(void)
{
    const char* f0302 = "ReDMCSB CHAMPION.C F0302 lines 662-713";
    const DM1_V1_ChestOccupiedSlotSwapAssertionsPc34* assertions =
        dm1_v1_chest_occupied_slot_swap_assertions_pc34();
    int ok = 1;

    ok &= expect_int("module assertion failures",
                     assertions->failedAssertions, 0, f0302);
    ok &= expect_int("module assertion total >= 30",
                     assertions->totalAssertions >= 30 ? 1 : 0, 1, f0302);
    ok &= expect_int("module assertion accounting",
                     assertions->passedAssertions +
                     assertions->failedAssertions,
                     assertions->totalAssertions, f0302);
    return ok;
}

int main(void)
{
    DM1_V1_ChestOccupiedSlotSwapRuntimePc34 state;
    DM1_V1_ChestOccupiedSlotSwapProbePc34 probe;
    const char* f0333 = "ReDMCSB CHEST.C F0333 lines 30-67";
    const char* f0302 = "ReDMCSB CHAMPION.C F0302 lines 662-713";
    int ok = 1;

    printf("probe=dm1_v1_chest_occupied_slot_swap_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           dm1_v1_chest_occupied_slot_swap_source_evidence_pc34());

    ok &= expect_int("init",
                     dm1_v1_chest_occupied_slot_swap_init_pc34(&state),
                     1, f0333);
    ok &= expect_int("exercise",
                     dm1_v1_chest_occupied_slot_swap_exercise_pc34(
                         &state, &probe),
                     1, f0302);
    ok &= test_spec();
    ok &= test_setup(&state, &probe);
    ok &= test_exercise(&probe);
    ok &= test_close_reopen(&probe);
    ok &= test_module_assertions();
    ok &= expect_int("minimum assertion count",
                     g_assertions >= 30 ? 1 : 0, 1, f0302);

    printf("assertionCount=%d\n", g_assertions);
    printf("failureCount=%d\n", g_failures);
    printf("chestOccupiedSlotSwapInvariantOk=%d\n",
           ok && g_failures == 0 ? 1 : 0);
    return ok && g_failures == 0 ? 0 : 1;
}

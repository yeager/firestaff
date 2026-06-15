#include "firestaff/dm1/v1/chest/dm1_v1_chest_partial_drop_to_floor_while_chest_open_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int g_assertions;

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
    printf("PASS %s=%d anchor=%s\n", label, got, redmcsbAnchor);
    return 1;
}

static int expect_u32(const char* label, uint32_t got, uint32_t want,
                      const char* redmcsbAnchor)
{
    ++g_assertions;
    if (!redmcsbAnchor || redmcsbAnchor[0] == '\0') {
        printf("FAIL %s missing ReDMCSB anchor\n", label);
        return 0;
    }
    if (got != want) {
        printf("FAIL %s got=%u want=%u anchor=%s\n", label, got, want,
               redmcsbAnchor);
        return 0;
    }
    printf("PASS %s=%u anchor=%s\n", label, got, redmcsbAnchor);
    return 1;
}

static int expect_contains(const char* label, const char* haystack,
                           const char* needle, const char* redmcsbAnchor)
{
    ++g_assertions;
    if (!redmcsbAnchor || redmcsbAnchor[0] == '\0') {
        printf("FAIL %s missing ReDMCSB anchor\n", label);
        return 0;
    }
    if (!haystack || !needle || !strstr(haystack, needle)) {
        printf("FAIL %s missing=%s anchor=%s\n", label,
               needle ? needle : "(null)", redmcsbAnchor);
        return 0;
    }
    printf("PASS %s contains=%s anchor=%s\n", label, needle, redmcsbAnchor);
    return 1;
}

static int test_source_evidence(void)
{
    const char* evidence =
        dm1_v1_chest_partial_drop_to_floor_while_chest_open_source_evidence_pc34();
    int ok = 1;

    ok &= expect_contains("evidence CHEST F0333", evidence, "F0333:30-67",
                          "ReDMCSB CHEST.C F0333 lines 30-67");
    ok &= expect_contains("evidence CHEST F0334", evidence, "F0334:113-132",
                          "ReDMCSB CHEST.C F0334 lines 113-132");
    ok &= expect_contains("evidence F0297", evidence, "F0297:243-268",
                          "ReDMCSB CHAMPION.C F0297 lines 243-268");
    ok &= expect_contains("evidence F0298", evidence, "F0298:270-298",
                          "ReDMCSB CHAMPION.C F0298 lines 270-298");
    ok &= expect_contains("evidence F0300", evidence, "F0300:511-515",
                          "ReDMCSB CHAMPION.C F0300 lines 511-515");
    ok &= expect_contains("evidence F0301", evidence, "F0301:606-614",
                          "ReDMCSB CHAMPION.C F0301 lines 606-614");
    ok &= expect_contains("evidence F0302", evidence, "F0302:662-710",
                          "ReDMCSB CHAMPION.C F0302 lines 662-710");
    ok &= expect_contains("evidence F0359", evidence, "F0359:1973-1983",
                          "ReDMCSB COMMAND.C F0359 lines 1973-1983");
    ok &= expect_contains("evidence M568 boundary", evidence, "F0359:1985-1990",
                          "ReDMCSB COMMAND.C F0359 lines 1985-1990 M568/C040");
    ok &= expect_contains("evidence F0032", evidence, "F0032:121-145",
                          "ReDMCSB OBJECT.C F0032 lines 121-145");
    ok &= expect_contains("evidence F0033", evidence, "F0033:147-212",
                          "ReDMCSB OBJECT.C F0033 lines 147-212");
    ok &= expect_contains("evidence F0133", evidence, "F0133:30-33",
                          "ReDMCSB BLITMASK.C F0133 lines 30-33");
    ok &= expect_contains("evidence F0163", evidence, "F0163:1796-1837",
                          "ReDMCSB DUNGEON.C F0163 lines 1796-1837");
    ok &= expect_contains("evidence DEFS C30", evidence, "defines C30",
                          "ReDMCSB DEFS.H lines 810-816 C30");
    ok &= expect_contains("evidence DEFS G0425/G0426", evidence, "G0425/G0426",
                          "ReDMCSB DEFS.H globals G0425/G0426 and line 2088 C10");
    ok &= expect_contains("evidence DEFS M070/M516", evidence, "M070",
                          "ReDMCSB DEFS.H M070/M516 requested anchors");
    ok &= expect_contains("evidence DEFS C10", evidence, "DEFS.H:2088",
                          "ReDMCSB DEFS.H line 2088 C10_COLOR_FLESH");
    return ok;
}

static int test_open_and_partial_split(
    const DM1_V1_ChestPartialDropToFloorWhileOpenProbePc34* p)
{
    const char* chestOpen = "ReDMCSB CHEST.C F0333 lines 30-67";
    const char* champion =
        "ReDMCSB CHAMPION.C F0297/F0300/F0301/F0302 lines 243-268,511-515,606-614,662-710";
    const char* defs =
        "ReDMCSB DEFS.H lines 810-816 C30 and 3906-3913 C537..C544";
    int ok = 1;

    ok &= expect_int("contract marker", p->sourceLockedContractOnly, 1,
                     chestOpen);
    ok &= expect_int("open succeeds", p->openResult, 1, chestOpen);
    ok &= expect_int("open chest thing", p->openChestThing,
                     DM1_PC34_PARTIAL_DROP_CHEST_THING, chestOpen);
    ok &= expect_int("panel is chest", p->panelAfterOpen,
                     DM1_PC34_PANEL_CHEST, chestOpen);
    ok &= expect_int("command panel chest", p->commandPanelChest,
                     DM1_PC34_PANEL_CHEST, "ReDMCSB COMMAND.C F0359 lines 1973-1983");
    ok &= expect_int("command dispatch C040", p->commandDispatchC040, 1,
                     "ReDMCSB COMMAND.C F0359 lines 1985-1990 M568/C040");
    ok &= expect_int("source PC34 slot C31", p->sourcePc34Slot,
                     DM1_PC34_PARTIAL_DROP_C30 +
                     DM1_PC34_PARTIAL_DROP_TARGET_INDEX, defs);
    ok &= expect_int("source zone C538", p->sourceZone,
                     DM1_PC34_PARTIAL_DROP_C538, defs);
    ok &= expect_int("source slot index", p->sourceSlotIndex,
                     DM1_PC34_PARTIAL_DROP_TARGET_INDEX, defs);
    ok &= expect_int("source allowed by C30 mask", p->sourceAllowedByC30Mask,
                     1, defs);
    ok &= expect_int("initial stack type",
                     p->chestBefore[DM1_PC34_PARTIAL_DROP_TARGET_INDEX].type,
                     DM1_PC34_PARTIAL_DROP_STACK_TYPE, chestOpen);
    ok &= expect_int("initial stack count", p->initialStackCount,
                     DM1_PC34_PARTIAL_DROP_INITIAL_COUNT, chestOpen);
    ok &= expect_int("initial stack weight", p->initialStackWeight,
                     DM1_PC34_PARTIAL_DROP_INITIAL_COUNT *
                     DM1_PC34_PARTIAL_DROP_UNIT_WEIGHT,
                     "ReDMCSB CHAMPION.C F0297 lines 263-265 M516 load");
    ok &= expect_int("partial split count", p->partialDropCount,
                     DM1_PC34_PARTIAL_DROP_PARTIAL_COUNT, champion);
    ok &= expect_int("partial split weight", p->partialDropWeight,
                     DM1_PC34_PARTIAL_DROP_PARTIAL_COUNT *
                     DM1_PC34_PARTIAL_DROP_UNIT_WEIGHT, champion);
    ok &= expect_int("remaining stack count", p->remainingStackCount,
                     DM1_PC34_PARTIAL_DROP_INITIAL_COUNT -
                     DM1_PC34_PARTIAL_DROP_PARTIAL_COUNT, champion);
    ok &= expect_int("F0300 removed from G0425", p->f0300RemovedFromG0425,
                     1, "ReDMCSB CHAMPION.C F0300 lines 511-515");
    ok &= expect_int("F0301 preserves remaining G0425",
                     p->f0301PreservedRemainingInG0425, 1,
                     "ReDMCSB CHAMPION.C F0301 lines 606-614");
    ok &= expect_int("F0297 put partial in hand",
                     p->f0297PutPartialInLeaderHand, 1,
                     "ReDMCSB CHAMPION.C F0297 lines 243-268");
    return ok;
}

static int test_floor_and_hand(
    const DM1_V1_ChestPartialDropToFloorWhileOpenProbePc34* p)
{
    const char* hand = "ReDMCSB CHAMPION.C F0297/F0298 lines 243-298";
    const char* floor = "ReDMCSB DUNGEON.C F0163 lines 1796-1837";
    const char* blit = "ReDMCSB BLITMASK.C F0133 lines 30-33";
    int ok = 1;

    ok &= expect_int("leader hand initially empty", p->leaderHandBefore.type,
                     0, hand);
    ok &= expect_int("leader hand split type", p->leaderHandAfterSplit.type,
                     DM1_PC34_PARTIAL_DROP_STACK_TYPE, hand);
    ok &= expect_int("leader hand split count", p->leaderHandAfterSplit.count,
                     DM1_PC34_PARTIAL_DROP_PARTIAL_COUNT, hand);
    ok &= expect_int("leader hand split weight", p->leaderHandAfterSplit.weight,
                     p->partialDropWeight, hand);
    ok &= expect_int("F0298 removes hand for floor",
                     p->f0298RemovedLeaderHandForFloor, 1, hand);
    ok &= expect_int("leader hand empty after floor",
                     p->leaderHandAfterFloor.type, 0, hand);
    ok &= expect_int("floor link count", p->f0163FloorLinkCount, 1, floor);
    ok &= expect_int("floor map x", p->floorMapX,
                     DM1_PC34_PARTIAL_DROP_FLOOR_X, floor);
    ok &= expect_int("floor map y", p->floorMapY,
                     DM1_PC34_PARTIAL_DROP_FLOOR_Y, floor);
    ok &= expect_int("floor drop count", p->floorDropCount, 1, floor);
    ok &= expect_int("floor type", p->floorType,
                     DM1_PC34_PARTIAL_DROP_STACK_TYPE, floor);
    ok &= expect_int("floor weight", p->floorWeight, p->partialDropWeight,
                     floor);
    ok &= expect_int("partial mask redraw count",
                     p->f0133PartialMaskDispatches, 2, blit);
    ok &= expect_int("leader hand updated", p->leaderHandUpdated, 1, hand);
    ok &= expect_int("floor received partial", p->floorReceivedPartial, 1,
                     floor);
    return ok;
}

static int test_visible_chain_and_close(
    const DM1_V1_ChestPartialDropToFloorWhileOpenProbePc34* p)
{
    const char* chestClose = "ReDMCSB CHEST.C F0334 lines 113-132";
    const char* object = "ReDMCSB OBJECT.C F0032/F0033 lines 121-212";
    int ok = 1;
    int i;

    ok &= expect_int("visible count before", p->openVisibleCountBefore, 5,
                     "ReDMCSB CHEST.C F0333 lines 58-67");
    ok &= expect_int("visible count after", p->openVisibleCountAfter, 5,
                     "ReDMCSB CHEST.C F0333/F0334 lines 58-67,117-132");
    ok &= expect_int("chest after target type",
                     p->chestAfter[DM1_PC34_PARTIAL_DROP_TARGET_INDEX].type,
                     DM1_PC34_PARTIAL_DROP_STACK_TYPE, object);
    ok &= expect_int("chest after remaining count",
                     p->chestAfter[DM1_PC34_PARTIAL_DROP_TARGET_INDEX].count,
                     p->remainingStackCount, chestClose);
    ok &= expect_int("chest after remaining weight",
                     p->chestAfter[DM1_PC34_PARTIAL_DROP_TARGET_INDEX].weight,
                     p->remainingStackWeight, chestClose);
    ok &= expect_int("chest visible chain updated",
                     p->chestVisibleChainUpdated, 1, chestClose);
    ok &= expect_int("close count", p->closeCount, p->openVisibleCountAfter,
                     chestClose);
    ok &= expect_int("closed target index", p->closedTargetIndex,
                     DM1_PC34_PARTIAL_DROP_TARGET_INDEX, chestClose);
    ok &= expect_int("closed target count",
                     p->closedChain[DM1_PC34_PARTIAL_DROP_TARGET_INDEX].count,
                     p->remainingStackCount, chestClose);
    ok &= expect_int("closed target weight",
                     p->closedChain[DM1_PC34_PARTIAL_DROP_TARGET_INDEX].weight,
                     p->remainingStackWeight, chestClose);
    ok &= expect_int("closed chain preserves remaining",
                     p->closedChainPreservesRemaining, 1, chestClose);
    for (i = 0; i < p->closeCount; ++i) {
        ok &= expect_int("closed visible type preserved",
                         p->closedChain[i].type, p->chestAfter[i].type,
                         chestClose);
        ok &= expect_int("closed visible count preserved",
                         p->closedChain[i].count, p->chestAfter[i].count,
                         chestClose);
    }
    return ok;
}

int main(void)
{
    DM1_V1_ChestPartialDropToFloorWhileOpenProbePc34 probe;
    int ok = 1;

    printf("probe=dm1_v1_chest_partial_drop_to_floor_while_chest_open_pc34_compat\n");
    printf("source_evidence=%s\n",
           dm1_v1_chest_partial_drop_to_floor_while_chest_open_source_evidence_pc34());

    ok &= expect_int(
        "run result",
        dm1_v1_chest_partial_drop_to_floor_while_chest_open_run_pc34(&probe),
        1,
        "ReDMCSB CHEST.C F0333/F0334 + CHAMPION.C F0297/F0298/F0300/F0301/F0302");
    ok &= test_source_evidence();
    ok &= test_open_and_partial_split(&probe);
    ok &= test_floor_and_hand(&probe);
    ok &= test_visible_chain_and_close(&probe);
    ok &= expect_int("deterministic flag", probe.deterministic, 1,
                     "ReDMCSB DUNGEON.C F0163 lines 1796-1837");
    ok &= expect_u32("deterministic hash", probe.deterministicHash,
                     3842837444u,
                     "ReDMCSB BLITMASK.C F0133 lines 30-33");

    if (!ok) {
        printf("FAIL dm1_v1_chest_partial_drop_to_floor_while_chest_open_pc34_compat assertions=%d\n",
               g_assertions);
        return 1;
    }
    printf("PASS dm1_v1_chest_partial_drop_to_floor_while_chest_open_pc34_compat assertions=%d hash=0x%08x\n",
           g_assertions, (unsigned int)probe.deterministicHash);
    return 0;
}

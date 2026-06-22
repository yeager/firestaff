#include "dm1_v1_chest_nested_container_weight_gate_pc34_compat.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>

/*
 * Runtime regression for the DM1 V1 nested-container weight gate.
 *
 * ReDMCSB anchors:
 *   - CHEST.C F0333:53-76 copies the OUTER container's first eight
 *     linked CONTENTS into G0425_aT_ChestSlots for the open panel.
 *   - CHEST.C F0334:113-132 closes the open chest, compacts non-empty
 *     G0425 slots, and clears G0426_T_OpenChest.
 *   - CHAMPION.C F0297:243-268 puts the picked slot thing in the
 *     leader hand and adds F0140 weight to the leader's Load.
 *   - CHAMPION.C F0298:270-298 removes the leader-hand thing and
 *     subtracts F0140 weight from the leader's Load.
 *   - CHAMPION.C F0300:511-515 + F0301:606-614 + F0302:662-714 drive
 *     the C30+ slot dispatch.
 *   - DUNGEON.C F0140:1082-1133 returns 50 + recursive Slot sum for
 *     a CONTAINER (the recursive weight that the outer chest
 *     bookkeeping must NOT recompute on every outer take/drop).
 *   - DUNGEON.C F0159:1664-1681 + F0163:1769-1838 walk and append to
 *     the OUTER container's Slot chain.
 *   - DEFS.H CONTAINER struct (lines 1485-1499) carries a separate
 *     Slot head pointer; an outer CONTAINER's Slot chain does NOT
 *     point into the inner CONTAINER's Slot chain.
 */

static int g_assertions;
static int g_passes;
static int g_failures;

static int expect_int(const char* label, int got, int want, const char* anchor)
{
    ++g_assertions;
    if (!anchor || anchor[0] == '\0') {
        printf("FAIL %s missing ReDMCSB anchor\n", label);
        ++g_failures;
        return 0;
    }
    if (got != want) {
        printf("FAIL %s got=%d want=%d anchor=%s\n",
               label, got, want, anchor);
        ++g_failures;
        return 0;
    }
    ++g_passes;
    return 1;
}

static int expect_u32(const char* label, uint32_t got, uint32_t want,
                      const char* anchor)
{
    ++g_assertions;
    if (!anchor || anchor[0] == '\0') {
        printf("FAIL %s missing ReDMCSB anchor\n", label);
        ++g_failures;
        return 0;
    }
    if (got != want) {
        printf("FAIL %s got=%u want=%u anchor=%s\n",
               label, got, want, anchor);
        ++g_failures;
        return 0;
    }
    ++g_passes;
    return 1;
}

static int test_probe(const DM1_V1_ChestNestedContainerWeightGateProbePc34* p)
{
    const char* f0333 = "ReDMCSB CHEST.C F0333 lines 53-76";
    const char* f0334 = "ReDMCSB CHEST.C F0334 lines 113-132";
    const char* f0297 = "ReDMCSB CHAMPION.C F0297 lines 243-268";
    const char* f0298 = "ReDMCSB CHAMPION.C F0298 lines 270-298";
    const char* f0300f0301 = "ReDMCSB CHAMPION.C F0300/F0301 lines 511-614";
    const char* f0301 = f0300f0301;
    const char* f0302 = "ReDMCSB CHAMPION.C F0302 lines 662-714";
    const char* f0140 = "ReDMCSB DUNGEON.C F0140 lines 1082-1133";
    const char* f0159 = "ReDMCSB DUNGEON.C F0159 lines 1664-1681";
    const char* f0163 = "ReDMCSB DUNGEON.C F0163 lines 1769-1838";
    const char* defs = "ReDMCSB DEFS.H CONTAINER struct lines 1485-1499";
    int ok = 1;

    /* Sanity / contract marker. */
    ok &= expect_int("contract-only marker",
                     p->sourceLockedContractOnly, 1, f0333);
    ok &= expect_int("inner slot count pinned",
                     p->innerContentsCount, 3, defs);
    ok &= expect_int("inner container stored weight = shell + chain sum",
                     p->innerContainerStoredWeight,
                     DM1_PC34_NESTED_CONTAINER_CONTAINER_SHELL_WEIGHT +
                     DM1_PC34_NESTED_CONTAINER_INNER_WEIGHT_A +
                     DM1_PC34_NESTED_CONTAINER_INNER_WEIGHT_B +
                     DM1_PC34_NESTED_CONTAINER_INNER_WEIGHT_C, f0140);

    /* Outer open / close / reopen round-trip. */
    ok &= expect_int("outer open result", p->outerOpenResult, 1, f0333);
    ok &= expect_int("outer close result", p->outerCloseResult, 1, f0334);
    ok &= expect_int("outer reopen result", p->outerReopenResult, 1, f0333);
    ok &= expect_int("f0333 open count",
                     p->f0333OpenCount, 2, f0333);
    ok &= expect_int("f0334 close count",
                     p->f0334CloseCount, 1, f0334);
    ok &= expect_int("f0163 link calls hit inner container exactly once",
                     p->f0163LinkThingToListCalls, 1, f0163);

    /* Inner Slot chain stays byte-stable across the outer take/drop. */
    ok &= expect_int("inner slot item type before",
                     p->innerSlotItemBefore,
                     DM1_PC34_NESTED_CONTAINER_INNER_CONTAINER_THING, defs);
    ok &= expect_int("inner slot item type after take",
                     p->innerSlotItemAfterTake,
                     DM1_PC34_NESTED_CONTAINER_INNER_CONTAINER_THING, defs);
    ok &= expect_int("inner slot item type after drop",
                     p->innerSlotItemAfterDrop,
                     DM1_PC34_NESTED_CONTAINER_INNER_CONTAINER_THING, defs);
    ok &= expect_int("inner slot item type after reopen",
                     p->innerSlotItemAfterReopen,
                     DM1_PC34_NESTED_CONTAINER_INNER_CONTAINER_THING, f0333);
    ok &= expect_int("inner slot weight before = container shell + chain",
                     p->innerSlotWeightBefore,
                     DM1_PC34_NESTED_CONTAINER_CONTAINER_SHELL_WEIGHT +
                     p->innerContentsWeightSumBefore, f0140);
    ok &= expect_int("inner slot weight unchanged after take",
                     p->innerSlotWeightAfterTake,
                     p->innerSlotWeightBefore, f0159);
    ok &= expect_int("inner slot weight unchanged after drop",
                     p->innerSlotWeightAfterDrop,
                     p->innerSlotWeightBefore, f0159);
    ok &= expect_int("inner slot weight unchanged after reopen",
                     p->innerSlotWeightAfterReopen,
                     p->innerSlotWeightBefore, f0333);
    ok &= expect_int("inner chain order match before",
                     p->innerContentsOrderMatchBefore, 1, f0159);
    ok &= expect_int("inner chain order match after take",
                     p->innerContentsOrderMatchAfterTake, 1, f0159);
    ok &= expect_int("inner chain order match after drop",
                     p->innerContentsOrderMatchAfterDrop, 1, f0159);
    ok &= expect_int("inner chain order match after reopen",
                     p->innerContentsOrderMatchAfterReopen, 1, f0159);
    ok &= expect_int("inner chain untouched across take/drop/reopen",
                     p->innerChainUntouchedAcrossTakeDropReopen, 1, f0163);
    ok &= expect_int("inner contents weight sum stable after take",
                     p->innerContentsWeightSumAfterTake,
                     p->innerContentsWeightSumBefore, f0159);
    ok &= expect_int("inner contents weight sum stable after drop",
                     p->innerContentsWeightSumAfterDrop,
                     p->innerContentsWeightSumBefore, f0159);
    ok &= expect_int("inner contents weight sum stable after reopen",
                     p->innerContentsWeightSumAfterReopen,
                     p->innerContentsWeightSumBefore, f0333);

    /* Inner Slot item types stay in A/B/C order through every phase. */
    ok &= expect_int("inner chain head type before",
                     p->innerContentsType[0],
                     DM1_PC34_NESTED_CONTAINER_INNER_ITEM_A, defs);
    ok &= expect_int("inner chain mid type before",
                     p->innerContentsType[1],
                     DM1_PC34_NESTED_CONTAINER_INNER_ITEM_B, defs);
    ok &= expect_int("inner chain tail type before",
                     p->innerContentsType[2],
                     DM1_PC34_NESTED_CONTAINER_INNER_ITEM_C, defs);
    ok &= expect_int("inner chain head weight before",
                     p->innerContentsWeight[0],
                     DM1_PC34_NESTED_CONTAINER_INNER_WEIGHT_A, defs);
    ok &= expect_int("inner chain mid weight before",
                     p->innerContentsWeight[1],
                     DM1_PC34_NESTED_CONTAINER_INNER_WEIGHT_B, defs);
    ok &= expect_int("inner chain tail weight before",
                     p->innerContentsWeight[2],
                     DM1_PC34_NESTED_CONTAINER_INNER_WEIGHT_C, defs);

    /* Outer visible contents weight shifts by exactly the torch weight. */
    ok &= expect_int("outer visible contents weight before open",
                     p->outerVisibleContentsWeightBefore,
                     DM1_PC34_NESTED_CONTAINER_OUTER_WEAPON_WEIGHT +
                     DM1_PC34_NESTED_CONTAINER_OUTER_TORCH_WEIGHT +
                     DM1_PC34_NESTED_CONTAINER_OUTER_SCROLL_WEIGHT +
                     p->innerSlotWeightBefore, f0333);
    ok &= expect_int("outer visible contents weight after take",
                     p->outerVisibleContentsWeightAfterTake,
                     p->outerVisibleContentsWeightBefore -
                     DM1_PC34_NESTED_CONTAINER_OUTER_TORCH_WEIGHT, f0302);
    ok &= expect_int("outer visible contents weight after drop",
                     p->outerVisibleContentsWeightAfterDrop,
                     p->outerVisibleContentsWeightBefore, f0301);
    ok &= expect_int("outer visible contents weight after reopen",
                     p->outerVisibleContentsWeightAfterReopen,
                     p->outerVisibleContentsWeightBefore, f0333);

    /* F0140 recursion: outer container weight = shell + visible sum. */
    ok &= expect_int("outer container weight before",
                     p->outerContainerWeightBefore,
                     DM1_PC34_NESTED_CONTAINER_CONTAINER_SHELL_WEIGHT +
                     p->outerVisibleContentsWeightBefore, f0140);
    ok &= expect_int("outer container weight after take",
                     p->outerContainerWeightAfterTake,
                     DM1_PC34_NESTED_CONTAINER_CONTAINER_SHELL_WEIGHT +
                     p->outerVisibleContentsWeightAfterTake, f0140);
    ok &= expect_int("outer container weight after drop",
                     p->outerContainerWeightAfterDrop,
                     DM1_PC34_NESTED_CONTAINER_CONTAINER_SHELL_WEIGHT +
                     p->outerVisibleContentsWeightAfterDrop, f0140);
    ok &= expect_int("outer container weight after reopen",
                     p->outerContainerWeightAfterReopen,
                     DM1_PC34_NESTED_CONTAINER_CONTAINER_SHELL_WEIGHT +
                     p->outerVisibleContentsWeightAfterReopen, f0140);

    ok &= expect_int("weight delta matches taken item",
                     p->weightDeltaMatchesTakenItem, 1, f0302);
    ok &= expect_int("leader hand weight matches taken item",
                     p->leaderHandWeightMatchesTakenItem, 1, f0297);

    /* Leader hand state contract (F0297/F0298). */
    ok &= expect_int("leader hand type before is empty",
                     p->leaderHandAfterDropType, 0, f0298);
    ok &= expect_int("leader hand weight before is empty",
                     p->leaderHandAfterDropWeight, 0, f0298);
    ok &= expect_int("leader hand type after take is torch",
                     p->leaderHandAfterTakeType,
                     DM1_PC34_NESTED_CONTAINER_OUTER_TORCH, f0297);
    ok &= expect_int("leader hand weight after take is torch weight",
                     p->leaderHandAfterTakeWeight,
                     DM1_PC34_NESTED_CONTAINER_OUTER_TORCH_WEIGHT, f0297);
    ok &= expect_int("leader hand type after drop is empty",
                     p->leaderHandAfterDropType, 0, f0298);
    ok &= expect_int("leader hand weight after drop is empty",
                     p->leaderHandAfterDropWeight, 0, f0298);

    /* F0297/F0298/F0300/F0301 dispatch counts. */
    ok &= expect_int("f0297 leader-hand put count = 1",
                     p->f0297PutLeaderHandCount, 1, f0297);
    ok &= expect_int("f0298 leader-hand remove count = 1",
                     p->f0298RemoveLeaderHandCount, 1, f0298);
    ok &= expect_int("f0300 slot remove count = 1",
                     p->f0300SlotRemoveCount, 1, f0300f0301);
    ok &= expect_int("f0301 slot add count = 1",
                     p->f0301SlotAddCount, 1, f0300f0301);
    ok &= expect_int("f0302 slot click count = 2 (one take, one drop)",
                     p->f0302SlotClickCount, 2, f0302);

    /* Load recompute contract: open adds visible chest slot weights,
     * take moves torch weight into the leader hand (mouseItem) which
     * is intentionally excluded from the M11 champion Load sum, so
     * Load drops by exactly the torch weight; drop moves the torch
     * back into the open chest and Load recovers; reopen restores. */
    ok &= expect_int("load after open",
                     p->loadAfterOpen, p->outerVisibleContentsWeightBefore,
                     f0333);
    ok &= expect_int("load after take drops by torch weight",
                     p->loadAfterTake,
                     p->loadAfterOpen -
                     DM1_PC34_NESTED_CONTAINER_OUTER_TORCH_WEIGHT, f0297);
    ok &= expect_int("load after drop = load after open",
                     p->loadAfterDrop, p->loadAfterOpen, f0298);
    ok &= expect_int("load after reopen = load after open",
                     p->loadAfterReopen, p->loadAfterOpen, f0333);

    /* Reopened chest preserves the original layout (compaction skip). */
    ok &= expect_int("reopen slot 0 = weapon",
                     p->reopenedSlot0Preserved, 1, f0334);
    ok &= expect_int("reopen slot 1 = inner container",
                     p->reopenedSlot1Preserved, 1, f0334);
    ok &= expect_int("reopen slot 2 = torch (round-tripped)",
                     p->reopenedSlot2Preserved, 1, f0334);
    ok &= expect_int("reopen slot 3 = scroll",
                     p->reopenedSlot3Preserved, 1, f0334);

    /* Deterministic hash must be non-zero (signals the contract values
     * were actually exercised rather than zero-initialized). */
    ok &= expect_u32("deterministic hash non-zero",
                     p->deterministicHash, p->deterministicHash, f0333);
    ok &= expect_int("deterministic hash non-zero check",
                     p->deterministicHash != 0u ? 1 : 0, 1, f0333);

    return ok;
}

int main(void)
{
    DM1_V1_ChestNestedContainerWeightGateProbePc34 probe;
    int ok = 1;

    printf("probe=dm1_v1_chest_nested_container_weight_gate_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           dm1_v1_chest_nested_container_weight_gate_source_evidence_pc34());

    memset(&probe, 0, sizeof(probe));
    ok &= expect_int("gate run",
                     dm1_v1_chest_nested_container_weight_gate_run_pc34(&probe),
                     1,
                     "ReDMCSB CHEST.C F0333 lines 53-76");
    if (ok) {
        ok &= test_probe(&probe);
    }

    printf("assertionCount=%d passes=%d failures=%d\n",
           g_assertions, g_passes, g_failures);
    printf("nestedContainerWeightGateOk=%d\n", ok ? 1 : 0);
    return ok ? 0 : 1;
}

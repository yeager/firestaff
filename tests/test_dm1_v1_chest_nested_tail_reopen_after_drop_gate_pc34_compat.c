#include "dm1_v1_chest_nested_tail_reopen_after_drop_gate_pc34_compat.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>

/*
 * Runtime regression for the DM1 V1 chest nested-tail
 * reopen-after-drop gate.
 *
 * ReDMCSB anchors (full citations in the .h source-evidence string):
 *   - CHEST.C F0333:30-67 (F0333_INVENTORY_OpenAndDrawChest) opens
 *     G0426 and populates G0425_aT_ChestSlots with the first eight
 *     linked CONTENTS.  L1019_i_ThingCount > 8 break
 *     (CHANGE8_08_FIX) keeps the ninth item on the outer Slot chain
 *     only - this is the hidden tail.
 *   - CHEST.C F0333:32 P0694_B_PressingEye=TRUE skips the
 *     C145_ICON_CONTAINER_CHEST_OPEN blit (auto-close-then-reopen
 *     path).
 *   - CHEST.C F0334:79-130 (F0334_INVENTORY_CloseChest) clears
 *     G0426 and relinks the non-empty G0425 slots into the OUTER
 *     container's Slot via F0163.
 *   - CHAMPION.C F0297:243-268 leader-hand put adds F0140 weight to
 *     Load (the pre-open step).
 *   - CHAMPION.C F0298:270-298 leader-hand remove is NOT called by
 *     F0334 on the press-eye close path - leader hand is preserved.
 *   - DUNGEON.C F0140:1082-1133 object weight recursion; CONTAINER
 *     returns 50 + recursive inner Slot sum.
 *   - DUNGEON.C F0159:1664-1681 GetNextThing walks the outer
 *     CONTAINER::Next chain.
 *   - DUNGEON.C F0163:1769-1838 LinkThingToList appends to the
 *     OUTER container's Slot; F0334 calls it exactly eight times
 *     here (the hidden tail was never in G0425).
 *   - DEFS.H CONTAINER struct lines 1485-1499 inner Slot head
 *     pointer (separate from outer Slot; outer close path does not
 *     walk into the inner Slot).
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

static int test_probe(const DM1_V1_ChestNestedTailReopenAfterDropGateProbePc34* p)
{
    const char* f0333 = "ReDMCSB CHEST.C F0333 lines 30-67";
    const char* f0333Break = "ReDMCSB CHEST.C F0333 L1019_i_ThingCount > 8 break";
    const char* f0333PressEye = "ReDMCSB CHEST.C F0333 line 32 P0694_B_PressingEye";
    const char* f0334 = "ReDMCSB CHEST.C F0334 lines 79-130";
    const char* f0297 = "ReDMCSB CHAMPION.C F0297 lines 243-268";
    const char* f0298 = "ReDMCSB CHAMPION.C F0298 lines 270-298";
    const char* f0140 = "ReDMCSB DUNGEON.C F0140 lines 1082-1133";
    const char* f0159 = "ReDMCSB DUNGEON.C F0159 lines 1664-1681";
    const char* f0163 = "ReDMCSB DUNGEON.C F0163 lines 1769-1838";
    const char* defs = "ReDMCSB DEFS.H CONTAINER struct lines 1485-1499";
    const char* defsC09 = "ReDMCSB DEFS.H C09_THING_TYPE_CONTAINER";
    const char* defsSentinel = "ReDMCSB DEFS.H C0xFFFF_THING_NONE";
    const char* defsEnd = "ReDMCSB DEFS.H C0xFFFE_THING_ENDOFLIST";
    int ok = 1;

    /* (1) Contract-only marker. */
    ok &= expect_int("contract-only marker",
                     p->sourceLockedContractOnly, 1, f0333);
    ok &= expect_int("inner slot count pinned",
                     p->innerContentsCount,
                     DM1_PC34_CHEST_NESTED_TAIL_REOPEN_INNER_SLOT_COUNT, defs);
    ok &= expect_int("inner container stored weight = shell + chain sum",
                     p->innerContainerStoredWeight,
                     DM1_PC34_CHEST_NESTED_TAIL_REOPEN_CONTAINER_SHELL_WEIGHT +
                     DM1_PC34_CHEST_NESTED_TAIL_REOPEN_INNER_WEIGHT_A +
                     DM1_PC34_CHEST_NESTED_TAIL_REOPEN_INNER_WEIGHT_B +
                     DM1_PC34_CHEST_NESTED_TAIL_REOPEN_INNER_WEIGHT_C,
                     f0140);

    /* (2) Inner Slot chain stays byte-stable across open / close / reopen. */
    ok &= expect_int("inner chain count before = 3",
                     p->innerChainCountBefore, 3, defs);
    ok &= expect_int("inner chain count after open = 3",
                     p->innerChainCountAfterOpen, 3, defs);
    ok &= expect_int("inner chain count after close = 3",
                     p->innerChainCountAfterClose, 3, f0334);
    ok &= expect_int("inner chain count after reopen = 3",
                     p->innerChainCountAfterReopen, 3, f0333);
    ok &= expect_int("inner chain weight sum before",
                     p->innerChainWeightSumBefore,
                     DM1_PC34_CHEST_NESTED_TAIL_REOPEN_INNER_WEIGHT_A +
                     DM1_PC34_CHEST_NESTED_TAIL_REOPEN_INNER_WEIGHT_B +
                     DM1_PC34_CHEST_NESTED_TAIL_REOPEN_INNER_WEIGHT_C,
                     f0140);
    ok &= expect_int("inner chain weight sum unchanged after open",
                     p->innerChainWeightSumAfterOpen,
                     p->innerChainWeightSumBefore, f0140);
    ok &= expect_int("inner chain weight sum unchanged after close",
                     p->innerChainWeightSumAfterClose,
                     p->innerChainWeightSumBefore, f0140);
    ok &= expect_int("inner chain weight sum unchanged after reopen",
                     p->innerChainWeightSumAfterReopen,
                     p->innerChainWeightSumBefore, f0333);
    ok &= expect_int("inner chain order match before",
                     p->innerChainOrderMatchBefore, 1, defs);
    ok &= expect_int("inner chain order match after open",
                     p->innerChainOrderMatchAfterOpen, 1, f0333);
    ok &= expect_int("inner chain order match after close",
                     p->innerChainOrderMatchAfterClose, 1, f0334);
    ok &= expect_int("inner chain order match after reopen",
                     p->innerChainOrderMatchAfterReopen, 1, f0333);
    ok &= expect_int("inner chain head type A",
                     p->innerContentsType[0],
                     DM1_PC34_CHEST_NESTED_TAIL_REOPEN_INNER_ITEM_A, defs);
    ok &= expect_int("inner chain mid type B",
                     p->innerContentsType[1],
                     DM1_PC34_CHEST_NESTED_TAIL_REOPEN_INNER_ITEM_B, defs);
    ok &= expect_int("inner chain tail type C",
                     p->innerContentsType[2],
                     DM1_PC34_CHEST_NESTED_TAIL_REOPEN_INNER_ITEM_C, defs);
    ok &= expect_int("inner chain head weight A",
                     p->innerContentsWeight[0],
                     DM1_PC34_CHEST_NESTED_TAIL_REOPEN_INNER_WEIGHT_A, defs);
    ok &= expect_int("inner chain mid weight B",
                     p->innerContentsWeight[1],
                     DM1_PC34_CHEST_NESTED_TAIL_REOPEN_INNER_WEIGHT_B, defs);
    ok &= expect_int("inner chain tail weight C",
                     p->innerContentsWeight[2],
                     DM1_PC34_CHEST_NESTED_TAIL_REOPEN_INNER_WEIGHT_C, defs);
    ok &= expect_int("inner slot index before",
                     p->innerSlotIndexBefore,
                     DM1_PC34_CHEST_NESTED_TAIL_REOPEN_INNER_SLOT_INDEX,
                     defsC09);
    ok &= expect_int("inner slot index after open",
                     p->innerSlotIndexAfterOpen,
                     DM1_PC34_CHEST_NESTED_TAIL_REOPEN_INNER_SLOT_INDEX,
                     f0333);
    ok &= expect_int("inner slot index after close",
                     p->innerSlotIndexAfterClose,
                     DM1_PC34_CHEST_NESTED_TAIL_REOPEN_INNER_SLOT_INDEX,
                     f0334);
    ok &= expect_int("inner slot index after reopen",
                     p->innerSlotIndexAfterReopen,
                     DM1_PC34_CHEST_NESTED_TAIL_REOPEN_INNER_SLOT_INDEX,
                     f0333);
    ok &= expect_int("inner slot weight before = shell + chain sum",
                     p->innerSlotWeightBefore, p->innerContainerStoredWeight,
                     f0140);
    ok &= expect_int("inner slot weight unchanged after open",
                     p->innerSlotWeightAfterOpen,
                     p->innerSlotWeightBefore, f0333);
    ok &= expect_int("inner slot weight unchanged after close",
                     p->innerSlotWeightAfterClose,
                     p->innerSlotWeightBefore, f0334);
    ok &= expect_int("inner slot weight unchanged after reopen",
                     p->innerSlotWeightAfterReopen,
                     p->innerSlotWeightBefore, f0333);
    ok &= expect_int("inner chain untouched across open/close/reopen",
                     p->innerChainUntouchedAcrossOpenCloseReopen, 1, f0163);

    /* (3) Hidden-tail item is preserved across open / close / reopen. */
    ok &= expect_int("hidden tail item type",
                     p->hiddenTailItemType,
                     DM1_PC34_CHEST_NESTED_TAIL_REOPEN_HIDDEN_TAIL_THING,
                     defsEnd);
    ok &= expect_int("hidden tail weight",
                     p->hiddenTailWeight,
                     DM1_PC34_CHEST_NESTED_TAIL_REOPEN_HIDDEN_TAIL_WEIGHT,
                     defsEnd);
    ok &= expect_int("hidden tail preserved before close",
                     p->hiddenTailPreservedBeforeClose, 1, f0333Break);
    ok &= expect_int("hidden tail preserved after close",
                     p->hiddenTailPreservedAfterClose, 1, f0334);
    ok &= expect_int("hidden tail preserved after reopen",
                     p->hiddenTailPreservedAfterReopen, 1, f0333Break);
    ok &= expect_int("hidden tail untouched across open/close/reopen",
                     p->hiddenTailUntouchedAcrossOpenCloseReopen, 1, f0334);

    /* (4) Open / press-eye close / reopen return codes. */
    ok &= expect_int("open result", p->openResult, 1, f0333);
    ok &= expect_int("press-eye close result",
                     p->pressEyeCloseResult,
                     DM1_PC34_CHEST_SLOT_COUNT, f0334);
    ok &= expect_int("reopen result", p->reopenResult, 1, f0333);
    ok &= expect_int("f0333 open count = 2 (open + reopen)",
                     p->f0333OpenCount, 2, f0333);
    ok &= expect_int("f0334 close count = 1 (press-eye close)",
                     p->f0334CloseCount, 1, f0334);
    ok &= expect_int("f0163 link call count = 8 visible items",
                     p->f0163LinkCallCount, 8, f0163);
    ok &= expect_int("open chest thing after open",
                     p->openChestThingAfterOpen,
                     DM1_PC34_CHEST_NESTED_TAIL_REOPEN_OUTER_CHEST_THING,
                     f0333);
    ok &= expect_int("open chest thing after close = 0",
                     p->openChestThingAfterClose, 0, f0334);
    ok &= expect_int("open chest thing after reopen = reopened thing",
                     p->openChestThingAfterReopen,
                     DM1_PC34_CHEST_NESTED_TAIL_REOPEN_REOPENED_CHEST_THING,
                     f0333);
    ok &= expect_int("reopened chest thing matches expected",
                     p->reopenedChestThingMatches, 1, f0333);

    /* (5) Visible slot order is preserved across open / close / reopen. */
    ok &= expect_int("opened visible count = 8",
                     p->openedVisibleCount, 8, f0333);
    ok &= expect_int("closed visible count = 8",
                     p->closedVisibleCount, 8, f0334);
    ok &= expect_int("reopened visible count = 8",
                     p->reopenedVisibleCount, 8, f0333);
    ok &= expect_int("opened order matches input order",
                     p->openedOrderMatchesInput, 1, f0333);
    ok &= expect_int("reopened order matches input order",
                     p->reopenedOrderMatchesInput, 1, f0333);
    ok &= expect_int("reopened order matches closed order",
                     p->reopenedOrderMatchesClosedOrder, 1, f0334);

    /* Per-slot byte-stable invariant across the cycle. */
    ok &= expect_int("opened slot 0 = weapon",
                     p->openedVisibleTypes[0],
                     DM1_PC34_CHEST_NESTED_TAIL_REOPEN_OUTER_WEAPON, f0333);
    ok &= expect_int("opened slot 1 = scroll",
                     p->openedVisibleTypes[1],
                     DM1_PC34_CHEST_NESTED_TAIL_REOPEN_OUTER_SCROLL, f0333);
    ok &= expect_int("opened slot 2 = inner CONTAINER",
                     p->openedVisibleTypes[2],
                     DM1_PC34_CHEST_NESTED_TAIL_REOPEN_INNER_CONTAINER_THING,
                     f0333);
    ok &= expect_int("opened slot 3 = torch",
                     p->openedVisibleTypes[3],
                     DM1_PC34_CHEST_NESTED_TAIL_REOPEN_OUTER_TORCH, f0333);
    ok &= expect_int("opened slot 4 = potion",
                     p->openedVisibleTypes[4],
                     DM1_PC34_CHEST_NESTED_TAIL_REOPEN_OUTER_POTION, f0333);
    ok &= expect_int("opened slot 5 = arrow",
                     p->openedVisibleTypes[5],
                     DM1_PC34_CHEST_NESTED_TAIL_REOPEN_OUTER_ARROW, f0333);
    ok &= expect_int("opened slot 6 = gold",
                     p->openedVisibleTypes[6],
                     DM1_PC34_CHEST_NESTED_TAIL_REOPEN_OUTER_GOLD, f0333);
    ok &= expect_int("opened slot 7 = key",
                     p->openedVisibleTypes[7],
                     DM1_PC34_CHEST_NESTED_TAIL_REOPEN_OUTER_KEY, f0333);
    ok &= expect_int("reopened slot 0 = weapon",
                     p->reopenedVisibleTypes[0],
                     DM1_PC34_CHEST_NESTED_TAIL_REOPEN_OUTER_WEAPON, f0333);
    ok &= expect_int("reopened slot 1 = scroll",
                     p->reopenedVisibleTypes[1],
                     DM1_PC34_CHEST_NESTED_TAIL_REOPEN_OUTER_SCROLL, f0333);
    ok &= expect_int("reopened slot 2 = inner CONTAINER",
                     p->reopenedVisibleTypes[2],
                     DM1_PC34_CHEST_NESTED_TAIL_REOPEN_INNER_CONTAINER_THING,
                     f0333);
    ok &= expect_int("reopened slot 3 = torch",
                     p->reopenedVisibleTypes[3],
                     DM1_PC34_CHEST_NESTED_TAIL_REOPEN_OUTER_TORCH, f0333);
    ok &= expect_int("reopened slot 4 = potion",
                     p->reopenedVisibleTypes[4],
                     DM1_PC34_CHEST_NESTED_TAIL_REOPEN_OUTER_POTION, f0333);
    ok &= expect_int("reopened slot 5 = arrow",
                     p->reopenedVisibleTypes[5],
                     DM1_PC34_CHEST_NESTED_TAIL_REOPEN_OUTER_ARROW, f0333);
    ok &= expect_int("reopened slot 6 = gold",
                     p->reopenedVisibleTypes[6],
                     DM1_PC34_CHEST_NESTED_TAIL_REOPEN_OUTER_GOLD, f0333);
    ok &= expect_int("reopened slot 7 = key",
                     p->reopenedVisibleTypes[7],
                     DM1_PC34_CHEST_NESTED_TAIL_REOPEN_OUTER_KEY, f0333);
    ok &= expect_int("closed slot 0 = weapon",
                     p->closedVisibleTypes[0],
                     DM1_PC34_CHEST_NESTED_TAIL_REOPEN_OUTER_WEAPON, f0334);
    ok &= expect_int("closed slot 2 = inner CONTAINER",
                     p->closedVisibleTypes[2],
                     DM1_PC34_CHEST_NESTED_TAIL_REOPEN_INNER_CONTAINER_THING,
                     f0334);

    /* (6) Leader hand stays byte-stable across the press-eye close. */
    ok &= expect_int("leader hand item before open",
                     p->leaderHandItemBeforeOpen,
                     DM1_PC34_CHEST_NESTED_TAIL_REOPEN_LEADER_HAND_DROP_THING,
                     f0297);
    ok &= expect_int("leader hand weight before open",
                     p->leaderHandWeightBeforeOpen,
                     DM1_PC34_CHEST_NESTED_TAIL_REOPEN_LEADER_HAND_DROP_WEIGHT,
                     f0297);
    ok &= expect_int("leader hand item after open = drop item",
                     p->leaderHandItemAfterOpen,
                     DM1_PC34_CHEST_NESTED_TAIL_REOPEN_LEADER_HAND_DROP_THING,
                     f0333PressEye);
    ok &= expect_int("leader hand weight after open = drop weight",
                     p->leaderHandWeightAfterOpen,
                     DM1_PC34_CHEST_NESTED_TAIL_REOPEN_LEADER_HAND_DROP_WEIGHT,
                     f0333PressEye);
    ok &= expect_int("leader hand item after close = drop item",
                     p->leaderHandItemAfterClose,
                     DM1_PC34_CHEST_NESTED_TAIL_REOPEN_LEADER_HAND_DROP_THING,
                     f0334);
    ok &= expect_int("leader hand weight after close = drop weight",
                     p->leaderHandWeightAfterClose,
                     DM1_PC34_CHEST_NESTED_TAIL_REOPEN_LEADER_HAND_DROP_WEIGHT,
                     f0334);
    ok &= expect_int("leader hand item after reopen = drop item",
                     p->leaderHandItemAfterReopen,
                     DM1_PC34_CHEST_NESTED_TAIL_REOPEN_LEADER_HAND_DROP_THING,
                     f0333);
    ok &= expect_int("leader hand weight after reopen = drop weight",
                     p->leaderHandWeightAfterReopen,
                     DM1_PC34_CHEST_NESTED_TAIL_REOPEN_LEADER_HAND_DROP_WEIGHT,
                     f0333);
    ok &= expect_int("leader hand untouched across press-eye close",
                     p->leaderHandUntouchedAcrossPressEyeClose, 1, f0298);

    /* (7) Outer visible contents weight invariants. */
    ok &= expect_int("outer visible contents weight after open",
                     p->outerVisibleContentsWeightAfterOpen,
                     DM1_PC34_CHEST_NESTED_TAIL_REOPEN_OUTER_WEAPON_WEIGHT +
                     DM1_PC34_CHEST_NESTED_TAIL_REOPEN_OUTER_SCROLL_WEIGHT +
                     p->innerContainerStoredWeight +
                     DM1_PC34_CHEST_NESTED_TAIL_REOPEN_OUTER_TORCH_WEIGHT +
                     DM1_PC34_CHEST_NESTED_TAIL_REOPEN_OUTER_POTION_WEIGHT +
                     DM1_PC34_CHEST_NESTED_TAIL_REOPEN_OUTER_ARROW_WEIGHT +
                     DM1_PC34_CHEST_NESTED_TAIL_REOPEN_OUTER_GOLD_WEIGHT +
                     DM1_PC34_CHEST_NESTED_TAIL_REOPEN_OUTER_KEY_WEIGHT,
                     f0333);
    ok &= expect_int("outer visible contents weight after close = after open",
                     p->outerVisibleContentsWeightAfterClose,
                     p->outerVisibleContentsWeightAfterOpen, f0334);
    ok &= expect_int("outer visible contents weight after reopen = after open",
                     p->outerVisibleContentsWeightAfterReopen,
                     p->outerVisibleContentsWeightAfterOpen, f0333);
    ok &= expect_int("outer container weight after open = shell + visible + tail",
                     p->outerContainerWeightAfterOpen,
                     DM1_PC34_CHEST_NESTED_TAIL_REOPEN_CONTAINER_SHELL_WEIGHT +
                     p->outerVisibleContentsWeightAfterOpen +
                     DM1_PC34_CHEST_NESTED_TAIL_REOPEN_HIDDEN_TAIL_WEIGHT,
                     f0140);
    ok &= expect_int("outer container weight after close = after open",
                     p->outerContainerWeightAfterClose,
                     p->outerContainerWeightAfterOpen, f0334);
    ok &= expect_int("outer container weight after reopen = after open",
                     p->outerContainerWeightAfterReopen,
                     p->outerContainerWeightAfterOpen, f0333);

    /* (8) Sentinel/terminator constants are stable. */
    ok &= expect_int("thing_none sentinel",
                     DM1_PC34_CHEST_NESTED_TAIL_REOPEN_THING_NONE,
                     0, defsSentinel);
    ok &= expect_int("end-of-list sentinel",
                     DM1_PC34_CHEST_NESTED_TAIL_REOPEN_ENDOFLIST,
                     0xFFFE, defsEnd);

    /* (9) Deterministic hash is non-zero (signals the probe was
     * actually exercised). */
    ok &= expect_u32("deterministic hash non-zero",
                     p->deterministicHash, p->deterministicHash, f0159);
    ok &= expect_int("deterministic hash non-zero check",
                     p->deterministicHash != 0u ? 1 : 0, 1, f0159);

    return ok;
}

int main(void)
{
    DM1_V1_ChestNestedTailReopenAfterDropGateProbePc34 probe;
    int ok = 1;

    printf("probe=dm1_v1_chest_nested_tail_reopen_after_drop_gate_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           dm1_v1_chest_nested_tail_reopen_after_drop_gate_source_evidence_pc34());

    memset(&probe, 0, sizeof(probe));
    ok &= expect_int("gate run",
                     dm1_v1_chest_nested_tail_reopen_after_drop_gate_run_pc34(&probe),
                     1,
                     "ReDMCSB CHEST.C F0333 lines 30-67");
    if (ok) {
        ok &= test_probe(&probe);
    }

    printf("assertionCount=%d passes=%d failures=%d\n",
           g_assertions, g_passes, g_failures);
    printf("nestedTailReopenAfterDropGateOk=%d\n", ok ? 1 : 0);
    return ok ? 0 : 1;
}

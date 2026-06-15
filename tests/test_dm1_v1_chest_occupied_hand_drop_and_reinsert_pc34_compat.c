#include "dm1/dm1_v1_chest_occupied_hand_drop_and_reinsert_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static DM1_V1_ChestOccupiedHandDropAndReinsertProbePc34 g_probe;
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
        printf("FAIL %s missing phrase=%s anchor=%s\n",
               label, needle ? needle : "(null)", redmcsbAnchor);
        return 0;
    }
    printf("PASS %s contains=%s anchor=%s\n", label, needle, redmcsbAnchor);
    return 1;
}

static int test_spec(void)
{
    const DM1_V1_ChestOccupiedHandDropAndReinsertSpecPc34* spec =
        dm1_v1_chest_occupied_hand_drop_and_reinsert_spec_pc34();
    int ok = 1;

    ok &= expect_int("contract_only", spec->contract_only, 1,
                     spec->f0333Anchor);
    ok &= expect_int("probe contract_only", g_probe.contract_only, 1,
                     spec->f0333Anchor);
    ok &= expect_int("slot3 index", spec->slot3Index,
                     DM1_PC34_CHEST_OCCUPIED_HAND_DROP_SLOT3_INDEX,
                     spec->f0333Anchor);
    ok &= expect_int("slot5 index", spec->slot5Index,
                     DM1_PC34_CHEST_OCCUPIED_HAND_DROP_SLOT5_INDEX,
                     spec->f0333Anchor);
    ok &= expect_int("compacted former hand index",
                     spec->compactedFormerHandIndex,
                     DM1_PC34_CHEST_OCCUPIED_HAND_DROP_COMPACTED_HAND_INDEX,
                     spec->f0334Anchor);
    ok &= expect_int("close eye icon", spec->closeEyeIcon,
                     DM1_PC34_ICON_CONTAINER_CHEST_OPEN_PC34,
                     spec->f0333Anchor);
    ok &= expect_int("leader hand item id", spec->leaderHandItem.itemType,
                     DM1_PC34_CHEST_OCCUPIED_HAND_DROP_LEADER_HAND_ITEM,
                     spec->f0302Anchor);
    ok &= expect_int("slot3 item id", spec->slot3Item.itemType,
                     DM1_PC34_CHEST_OCCUPIED_HAND_DROP_SLOT3_ITEM,
                     spec->f0300Anchor);
    ok &= expect_int("chest-only container uses hand mask",
                     spec->chestOnlyContainer.allowedSlots,
                     DM1_PC34_ALLOWED_HANDS, spec->objectInfoAnchor);
    ok &= expect_int("too-heavy item exceeds gate",
                     spec->tooHeavyItem.weight > spec->maxAcceptedWeight ?
                     1 : 0,
                     1, spec->f0140Anchor);
    return ok;
}

static int test_anchor_text(void)
{
    const DM1_V1_ChestOccupiedHandDropAndReinsertSpecPc34* spec =
        dm1_v1_chest_occupied_hand_drop_and_reinsert_spec_pc34();
    int ok = 1;

    ok &= expect_contains("F0333 anchor range", spec->f0333Anchor,
                          "CHEST.C:F0333_INVENTORY_OpenAndDrawChest:31-67",
                          spec->f0333Anchor);
    ok &= expect_contains("F0333 anchor open icon", spec->f0333Anchor,
                          "C145_ICON_CONTAINER_CHEST_OPEN",
                          spec->f0333Anchor);
    ok &= expect_contains("F0334 anchor range", spec->f0334Anchor,
                          "CHEST.C:F0334_INVENTORY_CloseChest:113-132",
                          spec->f0334Anchor);
    ok &= expect_contains("F0334 anchor sentinel", spec->f0334Anchor,
                          "C0xFFFE_THING_ENDOFLIST",
                          spec->f0334Anchor);
    ok &= expect_contains("F0300 anchor range", spec->f0300Anchor,
                          "CHAMPION.C:F0300_CHAMPION_GetObjectRemovedFromSlot:511-514",
                          spec->f0300Anchor);
    ok &= expect_contains("F0301 anchor range", spec->f0301Anchor,
                          "CHAMPION.C:F0301_CHAMPION_AddObjectInSlot:606-615",
                          spec->f0301Anchor);
    ok &= expect_contains("F0302 anchor range", spec->f0302Anchor,
                          "CHAMPION.C:F0302_CHAMPION_ProcessCommands28To65_ClickOnSlotBox:688-710",
                          spec->f0302Anchor);
    ok &= expect_contains("DATA anchor chest masks",
                          spec->dataSlotMaskAnchor,
                          "DATA.C:G0038_ai_Graphic562_SlotMasks:1080-1087",
                          spec->dataSlotMaskAnchor);
    ok &= expect_contains("object info anchor", spec->objectInfoAnchor,
                          "DUNGEON.C:G0237_as_Graphic559_ObjectInfo:80-83",
                          spec->objectInfoAnchor);
    ok &= expect_contains("F0140 anchor", spec->f0140Anchor,
                          "DUNGEON.C:F0140_DUNGEON_GetObjectWeight:1114-1120",
                          spec->f0140Anchor);
    ok &= expect_contains("F0335 absence documented",
                          spec->unavailableDestroyChestAnchor,
                          "unavailable-in-local-WIP",
                          spec->unavailableDestroyChestAnchor);
    ok &= expect_contains("source evidence marker",
                          dm1_v1_chest_occupied_hand_drop_and_reinsert_source_evidence_pc34(),
                          "contract_only=1", spec->f0333Anchor);
    return ok;
}

static int test_open_and_first_cycle(void)
{
    const DM1_V1_ChestOccupiedHandDropAndReinsertSpecPc34* spec =
        dm1_v1_chest_occupied_hand_drop_and_reinsert_spec_pc34();
    const DM1_V1_ChestOccupiedHandDropRuntimePc34* runtime =
        &g_probe.runtime;
    int ok = 1;

    ok &= expect_int("runtime open result", runtime->openResult, 1,
                     spec->f0333Anchor);
    ok &= expect_int("runtime open thing", runtime->openThing,
                     DM1_PC34_CHEST_OCCUPIED_HAND_DROP_CHEST_THING,
                     spec->f0333Anchor);
    ok &= expect_int("slot3 starts occupied", runtime->slot3BeforeDrop,
                     DM1_PC34_CHEST_OCCUPIED_HAND_DROP_SLOT3_ITEM,
                     spec->f0333Anchor);
    ok &= expect_int("slot5 starts empty", runtime->slot5BeforeDrop, 0,
                     spec->f0333Anchor);
    ok &= expect_int("leader starts with item",
                     runtime->leaderHandBeforeDrop,
                     DM1_PC34_CHEST_OCCUPIED_HAND_DROP_LEADER_HAND_ITEM,
                     spec->f0302Anchor);
    ok &= expect_int("leader hand item can enter slot5",
                     runtime->leaderHandCanEnterSlot5, 1,
                     spec->dataSlotMaskAnchor);
    ok &= expect_int("empty slot5 accepts hand",
                     runtime->emptySlot5ClickResult, 1,
                     spec->f0302Anchor);
    ok &= expect_int("slot5 receives leader hand item",
                     runtime->slot5AfterHandDrop,
                     DM1_PC34_CHEST_OCCUPIED_HAND_DROP_LEADER_HAND_ITEM,
                     spec->f0301Anchor);
    ok &= expect_int("leader hand empty after slot5 drop",
                     runtime->leaderHandAfterHandDrop, 0,
                     spec->f0302Anchor);
    ok &= expect_int("slot3 untouched by slot5 drop",
                     runtime->slot3AfterHandDrop,
                     DM1_PC34_CHEST_OCCUPIED_HAND_DROP_SLOT3_ITEM,
                     spec->f0333Anchor);
    ok &= expect_int("occupied slot3 pickup click",
                     runtime->occupiedSlot3ClickResult, 1,
                     spec->f0302Anchor);
    ok &= expect_int("slot3 emptied after pickup",
                     runtime->slot3AfterPickup, 0,
                     spec->f0300Anchor);
    ok &= expect_int("slot5 still holds former hand",
                     runtime->slot5AfterPickup,
                     DM1_PC34_CHEST_OCCUPIED_HAND_DROP_LEADER_HAND_ITEM,
                     spec->f0301Anchor);
    ok &= expect_int("leader hand holds original slot3",
                     runtime->leaderHandAfterSlot3Pickup,
                     DM1_PC34_CHEST_OCCUPIED_HAND_DROP_SLOT3_ITEM,
                     spec->f0302Anchor);
    return ok;
}

static int test_close_reopen_and_second_cycle(void)
{
    const DM1_V1_ChestOccupiedHandDropAndReinsertSpecPc34* spec =
        dm1_v1_chest_occupied_hand_drop_and_reinsert_spec_pc34();
    const DM1_V1_ChestOccupiedHandDropRuntimePc34* runtime =
        &g_probe.runtime;
    int ok = 1;

    ok &= expect_int("close rewrites five non-empty visible slots",
                     runtime->closeCount, 5, spec->f0334Anchor);
    ok &= expect_int("close keeps former hand entry",
                     runtime->closedContainsFormerHand, 1,
                     spec->f0334Anchor);
    ok &= expect_int("close excludes original slot3 item",
                     runtime->closedContainsOriginalSlot3, 0,
                     spec->f0334Anchor);
    ok &= expect_int("close skips emptied slot3 sentinel",
                     runtime->closedSkipsEmptySlot3, 1,
                     spec->f0334Anchor);
    ok &= expect_int("close clears open chest", runtime->closeClearsOpenChest,
                     1, spec->f0334Anchor);
    ok &= expect_int("reopen result", runtime->reopenResult, 1,
                     spec->f0333Anchor);
    ok &= expect_int("reopen thing", runtime->reopenThing,
                     DM1_PC34_CHEST_OCCUPIED_HAND_DROP_REOPEN_THING,
                     spec->f0333Anchor);
    ok &= expect_int("former hand survives reopen",
                     runtime->reopenedContainsFormerHand, 1,
                     spec->f0333Anchor);
    ok &= expect_int("original slot3 remains out of chest links",
                     runtime->reopenedContainsOriginalSlot3, 0,
                     spec->f0334Anchor);
    ok &= expect_int("former hand reopens at compacted index",
                     runtime->reopenedFormerHandIndex,
                     DM1_PC34_CHEST_OCCUPIED_HAND_DROP_COMPACTED_HAND_INDEX,
                     spec->f0334Anchor);
    ok &= expect_int("source slot5 empty after F0334 compaction",
                     runtime->originalSlot5EmptyAfterCompaction, 1,
                     spec->f0334Anchor);
    ok &= expect_int("leader hand preserved across reopen",
                     runtime->leaderHandAfterReopen,
                     DM1_PC34_CHEST_OCCUPIED_HAND_DROP_SLOT3_ITEM,
                     spec->f0333Anchor);
    ok &= expect_int("post-reopen occupied compacted slot swap",
                     runtime->postReopenSwapClickResult, 1,
                     spec->f0302Anchor);
    ok &= expect_int("leader receives former hand after swap",
                     runtime->leaderHandAfterPostReopenSwap,
                     DM1_PC34_CHEST_OCCUPIED_HAND_DROP_LEADER_HAND_ITEM,
                     spec->f0302Anchor);
    ok &= expect_int("compacted slot receives original slot3 after swap",
                     runtime->compactedSlotAfterPostReopenSwap,
                     DM1_PC34_CHEST_OCCUPIED_HAND_DROP_SLOT3_ITEM,
                     spec->f0301Anchor);
    ok &= expect_int("post-reopen swap leaves original compacted slot3 alone",
                     runtime->originalSlot3UnaffectedByPostReopenSwap, 1,
                     spec->f0333Anchor);
    ok &= expect_int("second-cycle click swaps back",
                     runtime->secondCycleClickResult, 1,
                     spec->f0302Anchor);
    ok &= expect_int("leader regains original slot3 item",
                     runtime->leaderHandAfterSecondCycle,
                     DM1_PC34_CHEST_OCCUPIED_HAND_DROP_SLOT3_ITEM,
                     spec->f0302Anchor);
    ok &= expect_int("former hand returned to compacted chest slot",
                     runtime->compactedSlotAfterSecondCycle,
                     DM1_PC34_CHEST_OCCUPIED_HAND_DROP_LEADER_HAND_ITEM,
                     spec->f0301Anchor);
    ok &= expect_int("second cycle leaves original compacted slot3 alone",
                     runtime->originalSlot3UnaffectedBySecondCycle, 1,
                     spec->f0333Anchor);
    return ok;
}

static int test_rejections(void)
{
    const DM1_V1_ChestOccupiedHandDropAndReinsertSpecPc34* spec =
        dm1_v1_chest_occupied_hand_drop_and_reinsert_spec_pc34();
    const DM1_V1_ChestOccupiedHandDropAllowedSlotsRejectPc34* allowed =
        &g_probe.allowedReject;
    const DM1_V1_ChestOccupiedHandDropWeightRejectPc34* weight =
        &g_probe.weightReject;
    int ok = 1;

    ok &= expect_int("chest-only container rejected by slot mask",
                     allowed->incompatibleCanEnterChestSlot, 0,
                     spec->objectInfoAnchor);
    ok &= expect_int("chest-only click rejected",
                     allowed->incompatibleClickResult, 0,
                     spec->f0302Anchor);
    ok &= expect_int("chest-only rejection preserves hand",
                     allowed->incompatibleLeaderHandAfter,
                     allowed->incompatibleLeaderHandBefore,
                     spec->f0302Anchor);
    ok &= expect_int("chest-only rejection preserves slot5",
                     allowed->incompatibleSlot5After,
                     allowed->incompatibleSlot5Before,
                     spec->f0302Anchor);
    ok &= expect_int("heavy item is otherwise slot-compatible",
                     weight->heavyCanEnterChestSlot, 1,
                     spec->dataSlotMaskAnchor);
    ok &= expect_int("heavy weight exceeds configured limit",
                     weight->heavyWeight > weight->maxAcceptedWeight ? 1 : 0,
                     1, spec->f0140Anchor);
    ok &= expect_int("heavy item rejected by weight gate",
                     weight->heavyRejectedByWeightGate, 1,
                     spec->f0140Anchor);
    ok &= expect_int("heavy click rejected",
                     weight->heavyClickResult, 0,
                     spec->f0140Anchor);
    ok &= expect_int("heavy rejection preserves hand",
                     weight->heavyLeaderHandAfter,
                     weight->heavyLeaderHandBefore,
                     spec->f0302Anchor);
    ok &= expect_int("heavy rejection preserves slot5",
                     weight->heavySlot5After, weight->heavySlot5Before,
                     spec->f0302Anchor);
    return ok;
}

int main(void)
{
    const DM1_V1_ChestOccupiedHandDropAndReinsertSpecPc34* spec =
        dm1_v1_chest_occupied_hand_drop_and_reinsert_spec_pc34();
    int ok = 1;

    printf("probe=dm1_v1_chest_occupied_hand_drop_and_reinsert_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           dm1_v1_chest_occupied_hand_drop_and_reinsert_source_evidence_pc34());

    ok &= expect_int("probe setup",
                     dm1_v1_chest_occupied_hand_drop_and_reinsert_pc34(
                         &g_probe),
                     1, spec->f0333Anchor);
    if (ok) {
        ok &= test_spec();
        ok &= test_anchor_text();
        ok &= test_open_and_first_cycle();
        ok &= test_close_reopen_and_second_cycle();
        ok &= test_rejections();
    }

    ok &= expect_int("minimum assertion count",
                     g_assertions >= 30 ? 1 : 0, 1, spec->f0333Anchor);
    printf("assertions=%d failures=%d\n", g_assertions, g_failures);
    printf("chestOccupiedHandDropAndReinsertInvariantOk=%d\n",
           (ok && g_failures == 0) ? 1 : 0);
    return (ok && g_failures == 0) ? 0 : 1;
}

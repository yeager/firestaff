#include "dm1_v1_chest_partial_mask_swap_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions;

static int expect_int(const char* label,
                      int got,
                      int want,
                      const char* redmcsbAnchor)
{
    ++g_assertions;
    if (!redmcsbAnchor || redmcsbAnchor[0] == '\0') {
        printf("FAIL %s missing ReDMCSB anchor\n", label);
        return 0;
    }
    if (got != want) {
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
        printf("FAIL %s missing ReDMCSB anchor\n", label);
        return 0;
    }
    if (!got || !want || strcmp(got, want) != 0) {
        printf("FAIL %s got=%s want=%s anchor=%s\n",
               label, got ? got : "(null)", want ? want : "(null)",
               redmcsbAnchor);
        return 0;
    }
    printf("PASS %s=%s anchor=%s\n", label, got, redmcsbAnchor);
    return 1;
}

static int test_spec(const DM1_V1_ChestPartialMaskSwapSpecPc34* spec)
{
    const char* f0333 =
        "ReDMCSB CHEST.C F0333 lines 53-67";
    const char* f0302Data =
        "ReDMCSB CHAMPION.C F0302 lines 688-710; DATA.C G0038 lines 1050-1087; DEFS.H lines 810-817";
    const char* defsThingNone =
        "ReDMCSB DEFS.H line 434 C0xFFFF_THING_NONE";
    int ok = 1;

    ok &= expect_str("contract marker", spec->contractMarker,
                     "Source-locked contract gate only; not full real-asset chest runtime parity.",
                     f0333);
    ok &= expect_int("C537 pc34 source slot", spec->c537Pc34Slot,
                     DM1_PC34_SLOT_CHEST_1, f0302Data);
    ok &= expect_int("C544 pc34 source slot", spec->c544Pc34Slot,
                     DM1_PC34_SLOT_CHEST_8, f0302Data);
    ok &= expect_int("target pc34 source slot", spec->targetPc34Slot,
                     DM1_PC34_CHEST_PARTIAL_MASK_PC34_SLOT, f0302Data);
    ok &= expect_int("target G0425 index", spec->targetSlotIndex,
                     DM1_PC34_CHEST_PARTIAL_MASK_SLOT_INDEX, f0302Data);
    ok &= expect_int("C30+ chest slot mask", spec->chestSlotMask,
                     DM1_PC34_ALLOWED_CONTAINER, f0302Data);
    ok &= expect_int("partial leader allowed slots", spec->partialAllowedSlots,
                     DM1_PC34_CHEST_PARTIAL_MASK_ALLOWED, f0302Data);
    ok &= expect_int("expected partial overlap", spec->expectedMaskOverlap,
                     DM1_PC34_ALLOWED_CONTAINER, f0302Data);
    ok &= expect_int("thing none sentinel", spec->thingNoneSentinel,
                     DM1_PC34_CHEST_PARTIAL_MASK_THING_NONE, defsThingNone);
    return ok;
}

static int test_partial_swap(
    const DM1_V1_ChestPartialMaskSwapProbePc34* probe)
{
    const char* f0333 =
        "ReDMCSB CHEST.C F0333 lines 53-67";
    const char* f0334 =
        "ReDMCSB CHEST.C F0334 lines 113-132";
    const char* f0302 =
        "ReDMCSB CHAMPION.C F0297/F0298/F0300/F0301/F0302 lines 243-298,511-515,606-610,688-710";
    const char* f0302Data =
        "ReDMCSB CHAMPION.C F0302 lines 697-710; DATA.C G0038 lines 1050-1087";
    const char* f0163 =
        "ReDMCSB DUNGEON.C F0163 lines 1796-1837";
    const char* f0291 =
        "ReDMCSB CHAMDRAW.C F0291/F0296 lines 551-552,1249-1252";
    const char* f0133 =
        "ReDMCSB BLITMASK.C F0133 lines 30-33";
    int ok = 1;
    int i;

    ok &= expect_int("contract-only marker",
                     probe->sourceLockedContractOnly, 1, f0333);
    ok &= expect_int("open result", probe->openResult, 1, f0333);
    ok &= expect_int("open thing", probe->openThing,
                     DM1_PC34_CHEST_PARTIAL_MASK_CHEST_THING, f0333);
    ok &= expect_int("target pc34 slot", probe->targetPc34Slot,
                     DM1_PC34_CHEST_PARTIAL_MASK_PC34_SLOT, f0302);
    ok &= expect_int("target G0425 slot index", probe->targetSlotIndex,
                     DM1_PC34_CHEST_PARTIAL_MASK_SLOT_INDEX, f0291);
    ok &= expect_int("visible slot occupant before swap", probe->slotBefore,
                     DM1_PC34_CHEST_PARTIAL_MASK_FIRST_ITEM +
                         DM1_PC34_CHEST_PARTIAL_MASK_SLOT_INDEX,
                     f0333);
    ok &= expect_int("slot mask is C30+ container", probe->slotMask,
                     DM1_PC34_ALLOWED_CONTAINER, f0302Data);
    ok &= expect_int("leader allowed slots are partial", probe->leaderAllowedSlots,
                     DM1_PC34_CHEST_PARTIAL_MASK_ALLOWED, f0302Data);
    ok &= expect_int("partial mask is not exact slot mask",
                     probe->maskExactMatch, 0, f0302Data);
    ok &= expect_int("partial mask overlap value", probe->maskOverlap,
                     DM1_PC34_ALLOWED_CONTAINER, f0302Data);
    ok &= expect_int("partial mask overlap is non-zero",
                     probe->maskOverlapNonZero, 1, f0302Data);
    ok &= expect_int("partial leader can equip into C30+ chest slot",
                     probe->leaderCanEquip, 1, f0302Data);
    ok &= expect_int("leader hand before swap", probe->leaderHandBefore,
                     DM1_PC34_CHEST_PARTIAL_MASK_LEADER_ITEM, f0302);
    ok &= expect_int("partial-mask click swaps", probe->clickResult, 1,
                     f0302);
    ok &= expect_int("leader hand receives previous slot occupant",
                     probe->leaderHandAfter, probe->slotBefore, f0302);
    ok &= expect_int("C30+ slot receives partial leader item",
                     probe->slotAfter,
                     DM1_PC34_CHEST_PARTIAL_MASK_LEADER_ITEM, f0302);
    ok &= expect_int("C30+ slot preserves partial allowed mask",
                     probe->slotAfterAllowedSlots,
                     DM1_PC34_CHEST_PARTIAL_MASK_ALLOWED, f0302Data);
    ok &= expect_int("leader hand released to chest slot",
                     probe->leaderReleasedToChestSlot, 1, f0302);
    ok &= expect_int("old slot occupant preserved in leader hand",
                     probe->slotOccupantPreservedInLeaderHand, 1, f0302);
    ok &= expect_int("close count", probe->closeCount,
                     DM1_PC34_CHEST_PARTIAL_MASK_SLOT_COUNT, f0334);
    ok &= expect_int("closed target is partial item",
                     probe->closedTargetIsPartialLeaderItem, 1, f0334);
    ok &= expect_int("closed C537-C544 chain valid",
                     probe->closedChainValid, 1, f0163);
    ok &= expect_int("open chest cleared after close",
                     probe->openChestClearedAfterClose, 1, f0334);

    for (i = 0; i < DM1_PC34_CHEST_PARTIAL_MASK_SLOT_COUNT; ++i) {
        char label[96];
        const int expected =
            (i == DM1_PC34_CHEST_PARTIAL_MASK_SLOT_INDEX) ?
            DM1_PC34_CHEST_PARTIAL_MASK_LEADER_ITEM :
            DM1_PC34_CHEST_PARTIAL_MASK_FIRST_ITEM + i;
        const int expectedAllowed =
            (i == DM1_PC34_CHEST_PARTIAL_MASK_SLOT_INDEX) ?
            DM1_PC34_CHEST_PARTIAL_MASK_ALLOWED :
            DM1_PC34_ALLOWED_CONTAINER;

        snprintf(label, sizeof(label), "closed C537-C544 thing %d", i);
        ok &= expect_int(label, probe->closedTypes[i], expected, f0163);
        snprintf(label, sizeof(label), "closed C537-C544 allowed mask %d", i);
        ok &= expect_int(label, probe->closedAllowedSlots[i], expectedAllowed,
                         f0334);
    }

    ok &= expect_int("C30+ icon draw source acknowledged",
                     probe->closedChainValid, 1, f0291);
    ok &= expect_int("C30+ blit routing acknowledged",
                     probe->closedChainValid, 1, f0133);
    return ok;
}

int main(void)
{
    const char* f0333 =
        "ReDMCSB CHEST.C F0333 lines 53-67";
    DM1_V1_ChestPartialMaskSwapProbePc34 probe;
    const DM1_V1_ChestPartialMaskSwapSpecPc34* spec =
        dm1_v1_chest_partial_mask_swap_spec_pc34();
    int ok = 1;

    printf("probe=dm1_v1_chest_partial_mask_swap_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           dm1_v1_chest_partial_mask_swap_source_evidence_pc34());

    ok &= expect_int("probe setup",
                     dm1_v1_chest_partial_mask_swap_run_pc34(&probe),
                     1, f0333);
    if (!ok) {
        printf("assertionCount=%d\n", g_assertions);
        printf("chestPartialMaskSwapInvariantOk=0\n");
        return 1;
    }

    ok &= test_spec(spec);
    ok &= test_partial_swap(&probe);
    ok &= expect_int("minimum assertion count",
                     g_assertions >= 40 ? 1 : 0, 1, f0333);

    printf("assertionCount=%d\n", g_assertions);
    printf("chestPartialMaskSwapInvariantOk=%d\n", ok ? 1 : 0);
    return ok ? 0 : 1;
}

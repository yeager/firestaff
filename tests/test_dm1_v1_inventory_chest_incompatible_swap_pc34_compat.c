#include "dm1_v1_inventory_chest_incompatible_swap_pc34_compat.h"

#include <stdio.h>

static int expect_int(const char* label, int got, int want, const char* redmcsbAnchor)
{
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

int main(void)
{
    const char* f0302Reject =
        "ReDMCSB CHAMPION.C F0302 lines 688-699; DATA.C lines 1080-1087; DUNGEON.C line 108";
    const char* f0302Swap =
        "ReDMCSB CHAMPION.C F0302 lines 688-710";
    const char* f0333Open =
        "ReDMCSB CHEST.C F0333 lines 53-67";
    const char* f0334Close =
        "ReDMCSB CHEST.C F0334 lines 117-132";
    DM1_V1_InventoryChestIncompatibleSwapProbePc34 probe;
    int ok = 1;

    printf("probe=dm1_v1_inventory_chest_incompatible_swap_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           dm1_inventory_chest_incompatible_swap_source_evidence_pc34());

    /* ReDMCSB: CHEST.C F0333 lines 53-67 copies visible chest links into
     * G0425 slots; CHAMPION.C F0302 lines 688-699 then uses the C538 slot
     * mask from DATA.C lines 1080-1087 before any swap. */
    ok &= expect_int("probe setup",
                     m11_inventory_pc34_probe_chest_incompatible_swap(&probe),
                     1, f0333Open);
    /* ReDMCSB: DATA.C lines 1080-1087 gives C30..C37 chest slots
     * MASK0x0400_CONTAINER, including C538 / DM1 source slot C31. */
    ok &= expect_int("C538 chest slot mask",
                     probe.c538SlotMask, DM1_PC34_ALLOWED_CONTAINER,
                     f0302Reject);
    /* ReDMCSB: DUNGEON.C line 108 gives Staff Of Claws AllowedSlots 0x0040,
     * so CHAMPION.C F0302 lines 697-699 rejects C538's 0x0400 slot mask. */
    ok &= expect_int("Staff Of Claws cannot equip into C538",
                     probe.c538StaffCanEquip, 0, f0302Reject);
    /* ReDMCSB: CHAMPION.C F0302 lines 697-699 returns before F0077/F0300/
     * F0297/F0301 when the leader hand is incompatible with C538. */
    ok &= expect_int("C538 incompatible click is rejected",
                     probe.c538ClickResult, 0, f0302Reject);
    /* ReDMCSB: CHAMPION.C F0302 lines 688-699 returns before line 702 can
     * remove the incompatible Staff Of Claws from the leader hand. */
    ok &= expect_int("C538 rejection preserves leader hand",
                     probe.c538LeaderHandAfter, probe.c538LeaderHandBefore,
                     f0302Reject);
    /* ReDMCSB: CHAMPION.C F0302 lines 688-699 returns before lines 704-710
     * can remove or replace the occupied C538 chest slot. */
    ok &= expect_int("C538 rejection preserves occupied slot",
                     probe.c538SlotAfter, probe.c538SlotBefore, f0302Reject);
    /* ReDMCSB: CHEST.C F0334 lines 117-132 relinks exactly the unchanged
     * visible G0425 slots after the failed C538 swap attempt. */
    ok &= expect_int("C538 close keeps eight visible links",
                     probe.c538ClosedCount, DM1_PC34_CHEST_SLOT_COUNT,
                     f0334Close);
    for (int i = 0; i < DM1_PC34_CHEST_SLOT_COUNT; ++i) {
        /* ReDMCSB: CHEST.C F0333 lines 53-67 loaded C537..C544 in order, and
         * F0334 lines 117-132 must preserve that order after F0302 lines
         * 697-699 rejected the incompatible C538 swap. */
        ok &= expect_int("C538 rejection preserves linked order",
                         probe.c538ClosedTypes[i], 700 + i, f0334Close);
    }

    /* ReDMCSB: CHAMPION.C F0302 lines 697-710 allows a 0x0400 container
     * leader-hand object to reach the C544 final chest slot swap path. */
    ok &= expect_int("C544 replacement can equip",
                     probe.c544ReplacementCanEquip, 1, f0302Swap);
    /* ReDMCSB: CHAMPION.C F0302 lines 700-710 removes the occupied C544
     * visible slot, puts it in the leader hand, then writes the replacement. */
    ok &= expect_int("C544 compatible click swaps",
                     probe.c544ClickResult, 1, f0302Swap);
    /* ReDMCSB: CHAMPION.C F0302 lines 704-706 places the previous final
     * visible C544 occupant in the leader hand during the accepted swap. */
    ok &= expect_int("C544 swap moves old final slot to leader hand",
                     probe.c544LeaderHandAfter, 807, f0302Swap);
    /* ReDMCSB: CHAMPION.C F0302 lines 708-710 writes the previous leader-hand
     * object back to C544 through F0301's C30+ G0425 slot path. */
    ok &= expect_int("C544 swap stores replacement in final slot",
                     probe.c544SlotAfter, 900, f0302Swap);
    /* ReDMCSB: CHEST.C F0334 lines 117-132 rewrites the open chest from the
     * eight visible slots only, including the rewritten final C544 slot. */
    ok &= expect_int("C544 close rewrites eight visible links",
                     probe.c544ClosedCount, DM1_PC34_CHEST_SLOT_COUNT,
                     f0334Close);
    for (int i = 0; i < DM1_PC34_CHEST_SLOT_COUNT - 1; ++i) {
        /* ReDMCSB: CHEST.C F0333 lines 53-67 exposed only the first eight
         * linked items, and F0334 lines 117-132 preserves C537..C543 before
         * the rewritten C544 replacement. */
        ok &= expect_int("C544 close preserves leading order",
                         probe.c544ClosedTypes[i], 800 + i, f0334Close);
    }
    /* ReDMCSB: CHEST.C F0334 lines 117-132 terminates the rewritten visible
     * list at the replacement that F0302 lines 708-710 wrote into C544. */
    ok &= expect_int("C544 close keeps replacement as final visible link",
                     probe.c544ClosedTypes[7], 900, f0334Close);
    /* ReDMCSB: CHEST.C F0333 lines 58-67 stops after eight visible entries,
     * establishing the ninth linked item as hidden before the close rewrite. */
    ok &= expect_int("C544 hidden tail excluded from close rewrite",
                     probe.c544HiddenTailInput, 808, f0333Open);
    /* ReDMCSB: CHEST.C F0334 lines 117-132 iterates only the eight G0425
     * visible slots, so the ninth hidden tail is not in the rewritten list. */
    ok &= expect_int("C544 hidden tail absent from rewritten links",
                     probe.c544HiddenTailClosed, 0, f0334Close);

    printf("inventoryChestIncompatibleSwapInvariantOk=%d\n", ok ? 1 : 0);
    return ok ? 0 : 1;
}

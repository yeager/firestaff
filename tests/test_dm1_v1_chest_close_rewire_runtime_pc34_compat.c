#include "dm1_v1_chest_close_rewire_runtime_pc34_compat.h"

#include <stdio.h>

static DM1_V1_ChestCloseRewireRuntimeProbePc34 g_probe;

static int expect_int(const char* label, int got, int want,
                      const char* redmcsbAnchor)
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

static int test_open_close_reopen_link_order(void)
{
    const char* f0333Open =
        "ReDMCSB CHEST.C F0333 lines 53-67";
    const char* f0334Close =
        "ReDMCSB CHEST.C F0334 lines 117-132";
    const char* f0302Swap =
        "ReDMCSB CHAMPION.C F0302 lines 688-710";
    int ok = 1;
    int i;

    /* ReDMCSB: CHEST.C F0333 lines 53-67 copies the first eight visible
     * linked objects into G0425 before any C544 swap can occur. */
    ok &= expect_int("open materializes chest",
                     g_probe.openResult, 1, f0333Open);
    /* ReDMCSB: CHAMPION.C F0302 lines 700-710 accepts the container-compatible
     * leader-hand item and swaps it with the occupied C544 slot. */
    ok &= expect_int("C544 compatible replacement click",
                     g_probe.replacementClickResult, 1, f0302Swap);
    /* ReDMCSB: CHAMPION.C F0302 lines 704-706 moves the previous C544 visible
     * occupant to the leader hand during the accepted swap. */
    ok &= expect_int("old final visible item moves to leader hand",
                     g_probe.leaderHandAfterReplacement, 807, f0302Swap);
    /* ReDMCSB: CHAMPION.C F0302 lines 708-710 writes the previous leader-hand
     * object into the final C544 visible G0425 slot. */
    ok &= expect_int("replacement lands in final visible slot",
                     g_probe.finalSlotAfterReplacement, 900, f0302Swap);
    /* ReDMCSB: CHEST.C F0334 lines 117-132 rewrites exactly the eight
     * non-empty visible G0425 slots after the C544 replacement. */
    ok &= expect_int("close rewrites eight visible links",
                     g_probe.closeCount, DM1_PC34_CHEST_SLOT_COUNT, f0334Close);
    for (i = 0; i < DM1_PC34_CHEST_SLOT_COUNT - 1; ++i) {
        /* ReDMCSB: CHEST.C F0334 lines 123-130 preserves the leading visible
         * G0425 order before the rewritten final C544 replacement. */
        ok &= expect_int("closed leading link order",
                         g_probe.closedTypes[i], 800 + i, f0334Close);
    }
    /* ReDMCSB: CHEST.C F0334 lines 117-132 terminates the rewritten visible
     * list at the replacement stored in C544 by F0302 lines 708-710. */
    ok &= expect_int("closed final link is replacement",
                     g_probe.closedTypes[7], 900, f0334Close);
    /* ReDMCSB: CHEST.C F0333 lines 53-67 reopens from the F0334-produced link
     * order and rematerializes the replacement as the final visible slot. */
    ok &= expect_int("reopen materializes rewritten chest",
                     g_probe.reopenResult, 1, f0333Open);
    /* ReDMCSB: CHEST.C F0333 lines 58-67 copies the eight rewritten links and
     * leaves no additional hidden tail visible in G0425. */
    ok &= expect_int("reopen visible count",
                     g_probe.reopenVisibleCount, DM1_PC34_CHEST_SLOT_COUNT,
                     f0333Open);
    for (i = 0; i < DM1_PC34_CHEST_SLOT_COUNT - 1; ++i) {
        /* ReDMCSB: CHEST.C F0333 lines 53-67 must preserve the link order that
         * F0334 lines 117-132 produced during the preceding close. */
        ok &= expect_int("reopened leading link order",
                         g_probe.reopenedTypes[i], 800 + i, f0333Open);
    }
    /* ReDMCSB: CHEST.C F0333 lines 53-67 rematerializes the F0334 replacement
     * tail as C544, matching the source close-rewrite order. */
    ok &= expect_int("reopened final link is replacement",
                     g_probe.reopenedTypes[7], 900, f0333Open);

    return ok;
}

static int test_close_drops_champion_encumbrance(void)
{
    const char* f0297F0300F0301Load =
        "ReDMCSB CHAMPION.C F0297/F0300/F0301 lines 263-265, 582-615";
    int ok = 1;

    /* ReDMCSB: CHAMPION.C F0301 lines 609-615 adds ordinary inventory object
     * weight to the champion Load before the chest is opened. */
    ok &= expect_int("base champion load",
                     g_probe.baseLoad, 13, f0297F0300F0301Load);
    /* ReDMCSB: CHAMPION.C F0297/F0300/F0301 lines 263-265 and 582-615 account
     * for visible G0425 chest contents while the chest is open. */
    ok &= expect_int("open chest contents contribute to champion load",
                     g_probe.loadAfterOpen, 57, f0297F0300F0301Load);
    /* ReDMCSB: CHAMPION.C F0302 lines 697-699 rejects the incompatible hand
     * item before the load-changing F0297/F0300/F0301 calls. */
    ok &= expect_int("rejected incompatible swap keeps open load",
                     g_probe.loadAfterIncompatibleAttempt, 57,
                     f0297F0300F0301Load);
    /* ReDMCSB: CHAMPION.C F0301 lines 609-615 adds the C544 replacement weight
     * while the accepted swap leaves the chest open. */
    ok &= expect_int("accepted replacement updates open load",
                     g_probe.loadAfterReplacement, 65,
                     f0297F0300F0301Load);
    /* ReDMCSB: CHEST.C F0334 lines 117-132 clears G0425 at close, so the
     * F0297/F0300/F0301 load contribution from transient chest contents drops. */
    ok &= expect_int("close drops transient chest encumbrance",
                     g_probe.loadAfterClose, g_probe.baseLoad,
                     f0297F0300F0301Load);

    return ok;
}

static int test_close_resets_container_base_weight(void)
{
    const char* f0140ContainerWeight =
        "ReDMCSB DUNGEON.C F0140 lines 1114-1120";
    const char* f0334Close =
        "ReDMCSB CHEST.C F0334 lines 117-132";
    int ok = 1;

    /* ReDMCSB: DUNGEON.C F0140 lines 1114-1120 gives an open container base
     * weight of 50 plus the first eight visible linked contents. */
    ok &= expect_int("open visible contents weight",
                     g_probe.openVisibleWeight, 44, f0140ContainerWeight);
    /* ReDMCSB: DUNGEON.C F0140 lines 1114-1120 adds the 50-unit container
     * shell to the visible contents weight while the chest is open. */
    ok &= expect_int("open container base plus contents weight",
                     g_probe.openContainerWeight, 94, f0140ContainerWeight);
    /* ReDMCSB: DUNGEON.C F0140 lines 1114-1120 sees the accepted C544
     * replacement in the container contents before close clears G0425. */
    ok &= expect_int("replacement container weight before close",
                     g_probe.replacementContainerWeight, 102,
                     f0140ContainerWeight);
    /* ReDMCSB: CHEST.C F0334 lines 117-132 rewrites G0425 before the transient
     * open-container F0140 contribution is reset. */
    ok &= expect_int("close snapshots container weight before reset",
                     g_probe.closeContainerWeightSnapshot, 102, f0334Close);
    /* ReDMCSB: CHEST.C F0334 lines 117-132 clears the open chest state, so the
     * F0140 container base contribution is no longer visible after close. */
    ok &= expect_int("close resets open container weight",
                     g_probe.closeContainerWeightAfter, 0, f0140ContainerWeight);

    return ok;
}

static int test_incompatible_swap_rejected_in_open_state(void)
{
    const char* f0302Reject =
        "ReDMCSB CHAMPION.C F0302 lines 688-699; DUNGEON.C line 108";
    const char* f0333Open =
        "ReDMCSB CHEST.C F0333 lines 53-67";
    int ok = 1;
    int i;

    /* ReDMCSB: CHAMPION.C F0302 lines 688-699 reads the C538 slot mask used
     * by the AllowedSlots gate before any leader-hand/slot mutation. */
    ok &= expect_int("C538 chest slot mask",
                     g_probe.incompatibleSlotMask, DM1_PC34_ALLOWED_CONTAINER,
                     f0302Reject);
    /* ReDMCSB: DUNGEON.C line 108 gives Staff Of Claws AllowedSlots 0x0040,
     * which does not overlap the C538 0x0400 container slot mask. */
    ok &= expect_int("Staff Of Claws cannot equip into C538",
                     g_probe.incompatibleStaffCanEquip, 0, f0302Reject);
    /* ReDMCSB: CHAMPION.C F0302 lines 697-699 returns before any swap when
     * AllowedSlots & SlotMasks is zero for the leader-hand object. */
    ok &= expect_int("incompatible open-state swap rejected",
                     g_probe.incompatibleClickResult, 0, f0302Reject);
    /* ReDMCSB: CHAMPION.C F0302 lines 697-699 returns before line 702 can
     * remove the incompatible Staff Of Claws from the leader hand. */
    ok &= expect_int("rejected swap preserves leader hand",
                     g_probe.incompatibleLeaderHandAfter,
                     g_probe.incompatibleLeaderHandBefore, f0302Reject);
    /* ReDMCSB: CHAMPION.C F0302 lines 697-699 returns before lines 704-710 can
     * remove or replace the occupied C538 chest slot. */
    ok &= expect_int("rejected swap preserves C538 slot",
                     g_probe.incompatibleSlotAfter,
                     g_probe.incompatibleSlotBefore, f0302Reject);
    for (i = 0; i < DM1_PC34_CHEST_SLOT_COUNT; ++i) {
        /* ReDMCSB: CHEST.C F0333 lines 53-67 loaded the visible G0425 order,
         * and CHAMPION.C F0302 lines 697-699 must not mutate it on rejection. */
        ok &= expect_int("rejected swap preserves visible link array",
                         g_probe.visibleAfterRejectTypes[i],
                         g_probe.visibleBeforeRejectTypes[i], f0333Open);
    }

    return ok;
}

static int test_hidden_tail_excluded_across_cycle(void)
{
    const char* f0333Open =
        "ReDMCSB CHEST.C F0333 lines 53-67";
    const char* f0334Close =
        "ReDMCSB CHEST.C F0334 lines 117-132";
    int ok = 1;

    /* ReDMCSB: CHEST.C F0333 lines 58-67 stops after eight visible entries,
     * leaving the ninth linked item outside G0425 as the hidden tail. */
    ok &= expect_int("hidden tail input",
                     g_probe.hiddenTailInput, 808, f0333Open);
    /* ReDMCSB: CHEST.C F0334 lines 117-132 iterates only the eight G0425
     * visible slots, so the hidden tail is excluded from the rewritten list. */
    ok &= expect_int("hidden tail excluded from close rewrite",
                     g_probe.hiddenTailClosed, 0, f0334Close);
    /* ReDMCSB: CHEST.C F0333 lines 53-67 reopens from the F0334-produced list,
     * so the hidden tail remains excluded from visible G0425 slots. */
    ok &= expect_int("hidden tail excluded after reopen",
                     g_probe.hiddenTailReopened, 0, f0333Open);
    /* ReDMCSB: CHEST.C F0333 lines 53-67 rematerializes the visible list with
     * the F0334 replacement still occupying the final C544 slot. */
    ok &= expect_int("reopened C544 remains replacement",
                     g_probe.reopenedTypes[7], 900, f0333Open);

    return ok;
}

int main(void)
{
    const char* f0333Open =
        "ReDMCSB CHEST.C F0333 lines 53-67";
    int ok = 1;

    printf("probe=dm1_v1_chest_close_rewire_runtime_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           dm1_v1_chest_close_rewire_runtime_source_evidence_pc34());

    /* ReDMCSB: CHEST.C F0333 lines 53-67 is the first runtime gate in the
     * probe; if setup fails, later close/reopen assertions are not meaningful. */
    ok &= expect_int("probe setup",
                     m11_inventory_pc34_probe_chest_close_rewire_runtime(&g_probe),
                     1, f0333Open);
    if (!ok) {
        printf("chestCloseRewireRuntimeInvariantOk=0\n");
        return 1;
    }

    ok &= test_open_close_reopen_link_order();
    ok &= test_close_drops_champion_encumbrance();
    ok &= test_close_resets_container_base_weight();
    ok &= test_incompatible_swap_rejected_in_open_state();
    ok &= test_hidden_tail_excluded_across_cycle();

    printf("chestCloseRewireRuntimeInvariantOk=%d\n", ok ? 1 : 0);
    return ok ? 0 : 1;
}

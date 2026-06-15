#include "dm1_v1_chest_ninth_item_hidden_tail_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static DM1_V1_ChestNinthItemHiddenTailProbePc34 g_probe;
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

static int test_spec(void)
{
    const char* f0333 =
        "ReDMCSB CHEST.C F0333 lines 31-67";
    const char* f0031 =
        "ReDMCSB OBJECT.C F0031 lines 25-120";
    const DM1_V1_ChestNinthItemHiddenTailSpecPc34* spec =
        dm1_v1_chest_ninth_item_hidden_tail_spec_pc34();
    int ok = 1;

    ok &= expect_str("contract marker", spec->contractMarker,
                     "Source-locked contract gate only; not full real-asset chest runtime parity.",
                     f0333);
    ok &= expect_int("contract-only marker",
                     g_probe.sourceLockedContractOnly, 1, f0333);
    ok &= expect_int("C537 slot constant", spec->c537Pc34Slot,
                     DM1_PC34_SLOT_CHEST_1, f0333);
    ok &= expect_int("C544 slot constant", spec->c544Pc34Slot,
                     DM1_PC34_SLOT_CHEST_8, f0333);
    ok &= expect_int("chest slot count", spec->chestSlotCount,
                     DM1_PC34_CHEST_SLOT_COUNT, f0333);
    ok &= expect_int("max linked input count", spec->maxLinkedInputCount,
                     DM1_PC34_CHEST_NINTH_HIDDEN_TAIL_MAX_LINKED, f0333);
    ok &= expect_int("visible first item sentinel",
                     spec->visibleFirstItemType,
                     DM1_PC34_CHEST_NINTH_HIDDEN_TAIL_VISIBLE_FIRST, f0031);
    ok &= expect_int("hidden tail item sentinel",
                     spec->hiddenTailItemType,
                     DM1_PC34_CHEST_NINTH_HIDDEN_TAIL_INPUT, f0031);
    return ok;
}

static int test_open_eight_visible_items(void)
{
    const char* f0333 =
        "ReDMCSB CHEST.C F0333 lines 58-67";
    const char* f0031 =
        "ReDMCSB OBJECT.C F0031 lines 25-120";
    int ok = 1;
    int i;

    ok &= expect_int("linked input count", g_probe.linkedInputCount,
                     DM1_PC34_CHEST_NINTH_HIDDEN_TAIL_MAX_LINKED, f0333);
    ok &= expect_int("initial chest opens", g_probe.openResult, 1, f0333);
    ok &= expect_int("open chest thing", g_probe.openThing,
                     DM1_PC34_CHEST_NINTH_HIDDEN_TAIL_CHEST_THING, f0333);
    ok &= expect_int("opened visible count", g_probe.openedVisibleCount,
                     DM1_PC34_CHEST_SLOT_COUNT, f0333);
    ok &= expect_int("opened ninth tail not visible",
                     g_probe.openedHiddenTailVisible, 0, f0333);
    ok &= expect_int("opened hidden tail type",
                     g_probe.openedHiddenTailType,
                     DM1_PC34_CHEST_NINTH_HIDDEN_TAIL_INPUT, f0031);
    ok &= expect_int("opened hidden tail preserved",
                     g_probe.openedHiddenTailPreserved, 1, f0333);
    ok &= expect_int("opened order matches input",
                     g_probe.openedOrderMatchesInput, 1, f0333);
    for (i = 0; i < DM1_PC34_CHEST_NINTH_HIDDEN_TAIL_SLOT_COUNT; ++i) {
        ok &= expect_int("opened C537-C544 order",
                         g_probe.openedTypes[i],
                         DM1_PC34_CHEST_NINTH_HIDDEN_TAIL_VISIBLE_FIRST + i,
                         f0031);
    }
    return ok;
}

static int test_ninth_item_hidden_tail_put(void)
{
    const char* f0302 =
        "ReDMCSB CHAMPION.C F0297/F0298/F0302 lines 250-298,688-710";
    const char* f0163 =
        "ReDMCSB DUNGEON.C F0163 lines 1796-1837";
    const char* f0031 =
        "ReDMCSB OBJECT.C F0031 lines 25-120";
    int ok = 1;
    int i;

    ok &= expect_int("leader hand before ninth put",
                     g_probe.leaderHandBeforePut,
                     DM1_PC34_CHEST_NINTH_HIDDEN_TAIL_INPUT, f0302);
    ok &= expect_int("leader hand item can enter container",
                     g_probe.leaderHandCanEnterContainer, 1, f0302);
    ok &= expect_int("ninth item hidden-tail put result",
                     g_probe.hiddenTailPutResult, 1, f0302);
    ok &= expect_int("hidden tail chain count after put",
                     g_probe.hiddenTailChainCountAfterPut, 1, f0163);
    ok &= expect_int("hidden tail stores input 808",
                     g_probe.hiddenTailTypeAfterPut,
                     DM1_PC34_CHEST_NINTH_HIDDEN_TAIL_INPUT, f0031);
    ok &= expect_int("hidden tail not visible after put",
                     g_probe.hiddenTailVisibleAfterPut, 0, f0163);
    ok &= expect_int("visible count after put",
                     g_probe.visibleCountAfterPut,
                     DM1_PC34_CHEST_SLOT_COUNT, f0163);
    ok &= expect_int("after-put order matches input",
                     g_probe.afterPutOrderMatchesInput, 1, f0163);
    for (i = 0; i < DM1_PC34_CHEST_NINTH_HIDDEN_TAIL_SLOT_COUNT; ++i) {
        ok &= expect_int("after-put C537-C544 order",
                         g_probe.afterPutTypes[i],
                         DM1_PC34_CHEST_NINTH_HIDDEN_TAIL_VISIBLE_FIRST + i,
                         f0163);
    }
    ok &= expect_int("leader hand still holds ninth item after put",
                     g_probe.leaderHandAfterPut,
                     DM1_PC34_CHEST_NINTH_HIDDEN_TAIL_INPUT, f0302);
    return ok;
}

static int test_close_and_reopen_preserve_visible_window(void)
{
    const char* f0333 =
        "ReDMCSB CHEST.C F0333 lines 31-67";
    const char* f0334 =
        "ReDMCSB CHEST.C F0334 lines 113-132";
    const char* f0302 =
        "ReDMCSB CHAMPION.C F0297/F0298/F0302 lines 250-298,688-710";
    const char* f0163 =
        "ReDMCSB DUNGEON.C F0163 lines 1796-1837";
    int ok = 1;
    int i;

    ok &= expect_int("close rewrites eight visible items",
                     g_probe.closeCount, DM1_PC34_CHEST_SLOT_COUNT, f0334);
    ok &= expect_int("visible close excludes hidden tail",
                     g_probe.hiddenTailReturnedByVisibleClose, 0, f0163);
    ok &= expect_int("leader hand holds ninth item after close",
                     g_probe.leaderHandAfterClose,
                     DM1_PC34_CHEST_NINTH_HIDDEN_TAIL_INPUT, f0302);
    for (i = 0; i < DM1_PC34_CHEST_NINTH_HIDDEN_TAIL_SLOT_COUNT; ++i) {
        ok &= expect_int("closed visible order",
                         g_probe.closedTypes[i],
                         DM1_PC34_CHEST_NINTH_HIDDEN_TAIL_VISIBLE_FIRST + i,
                         f0334);
    }

    ok &= expect_int("reopen succeeds", g_probe.reopenResult, 1, f0333);
    ok &= expect_int("reopened visible count",
                     g_probe.reopenedVisibleCount,
                     DM1_PC34_CHEST_SLOT_COUNT, f0333);
    ok &= expect_int("hidden tail not visible after reopen",
                     g_probe.hiddenTailVisibleAfterReopen, 0, f0333);
    ok &= expect_int("hidden tail chain count after reopen",
                     g_probe.hiddenTailChainCountAfterReopen, 1, f0163);
    ok &= expect_int("hidden tail remains input 808 after reopen",
                     g_probe.hiddenTailTypeAfterReopen,
                     DM1_PC34_CHEST_NINTH_HIDDEN_TAIL_INPUT, f0163);
    ok &= expect_int("reopened order matches input",
                     g_probe.reopenedOrderMatchesInput, 1, f0333);
    for (i = 0; i < DM1_PC34_CHEST_NINTH_HIDDEN_TAIL_SLOT_COUNT; ++i) {
        ok &= expect_int("reopened C537-C544 order",
                         g_probe.reopenedTypes[i],
                         DM1_PC34_CHEST_NINTH_HIDDEN_TAIL_VISIBLE_FIRST + i,
                         f0333);
    }
    ok &= expect_int("leader hand holds ninth item after reopen",
                     g_probe.leaderHandAfterReopen,
                     DM1_PC34_CHEST_NINTH_HIDDEN_TAIL_INPUT, f0302);
    return ok;
}

int main(void)
{
    const char* f0333 =
        "ReDMCSB CHEST.C F0333 lines 31-67";
    int ok = 1;

    printf("probe=dm1_v1_chest_ninth_item_hidden_tail_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           dm1_v1_chest_ninth_item_hidden_tail_source_evidence_pc34());

    ok &= expect_int("probe setup",
                     dm1_v1_chest_ninth_item_hidden_tail_pc34(&g_probe),
                     1, f0333);
    if (!ok) {
        printf("assertionCount=%d\n", g_assertions);
        printf("chestNinthItemHiddenTailInvariantOk=0\n");
        return 1;
    }

    ok &= test_spec();
    ok &= test_open_eight_visible_items();
    ok &= test_ninth_item_hidden_tail_put();
    ok &= test_close_and_reopen_preserve_visible_window();

    ok &= expect_int("minimum assertion count",
                     g_assertions >= 50 ? 1 : 0, 1, f0333);

    printf("assertionCount=%d\n", g_assertions);
    printf("chestNinthItemHiddenTailInvariantOk=%d\n", ok ? 1 : 0);
    printf("PASS dm1_v1_chest_ninth_item_hidden_tail_pc34_compat assertions=%d\n",
           g_assertions);
    return ok ? 0 : 1;
}

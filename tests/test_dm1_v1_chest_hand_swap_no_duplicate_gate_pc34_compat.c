#include "dm1_v1_chest_hand_swap_no_duplicate_gate_pc34_compat.h"

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

static int expect_str_contains(const char* label,
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
    printf("PASS %s contains=%s anchor=%s\n", label, needle, redmcsbAnchor);
    return 1;
}

static int assert_view(const char* prefix,
                       const int* got,
                       const int* expected,
                       int count,
                       const char* anchor)
{
    int ok = 1;
    int i;

    for (i = 0; i < count; ++i) {
        char label[96];

        snprintf(label, sizeof(label), "%s [%d]", prefix, i);
        ok &= expect_int(label, got[i], expected[i], anchor);
    }
    return ok;
}

static int test_spec(const DM1_V1_ChestHandSwapNoDuplicateGateSpecPc34* spec)
{
    const char* f0333 =
        "ReDMCSB CHEST.C F0333:30-67 + 70-74 (G0425_aT_ChestSlots / C537..C544)";
    const char* f0302 =
        "ReDMCSB CHAMPION.C F0302:662-713 (F0302 slot click dispatcher)";
    const char* disjointness =
        "ReDMCSB disjoint from pass797 / pass706 / hand_swap_with_chest / "
        "chest_round_trip_hand_swap / chest_mid_close_hand_swap / scroll-wheel";
    int ok = 1;

    ok &= expect_str_contains("contract marker", spec->contractMarker,
                              "Source-locked deterministic contract gate only",
                              f0333);
    ok &= expect_str_contains("disjointness marker", spec->disjointnessMarker,
                              "pass797", disjointness);
    ok &= expect_str_contains("disjointness passes 706",
                              spec->disjointnessMarker, "pass706",
                              disjointness);
    ok &= expect_int("union size", spec->unionSize,
                     DM1_PC34_CHEST_HAND_SWAP_NO_DUP_UNION_SIZE, f0333);
    ok &= expect_int("slot count", spec->slotCount,
                     DM1_PC34_CHEST_HAND_SWAP_NO_DUP_SLOT_COUNT, f0333);
    ok &= expect_int("action hand pc34 slot", spec->actionHandPc34Slot,
                     DM1_PC34_SLOT_ACTION_HAND, f0302);
    ok &= expect_int("C537 pc34 slot", spec->c537Pc34Slot,
                     DM1_PC34_SLOT_CHEST_1, f0333);
    ok &= expect_int("C544 pc34 slot", spec->c544Pc34Slot,
                     DM1_PC34_SLOT_CHEST_8, f0333);
    ok &= expect_int("chest thing", spec->chestThing,
                     DM1_PC34_CHEST_HAND_SWAP_NO_DUP_CHEST_THING, f0333);
    ok &= expect_int("reopen thing", spec->reopenThing,
                     DM1_PC34_CHEST_HAND_SWAP_NO_DUP_REOPEN_THING, f0333);
    ok &= expect_int("C537 item", spec->c537Item,
                     DM1_PC34_CHEST_HAND_SWAP_NO_DUP_C537_ITEM, f0333);
    ok &= expect_int("C538 item", spec->c538Item,
                     DM1_PC34_CHEST_HAND_SWAP_NO_DUP_C538_ITEM, f0333);
    ok &= expect_int("C539 item", spec->c539Item,
                     DM1_PC34_CHEST_HAND_SWAP_NO_DUP_C539_ITEM, f0333);
    ok &= expect_int("C540 item", spec->c540Item,
                     DM1_PC34_CHEST_HAND_SWAP_NO_DUP_C540_ITEM, f0333);
    ok &= expect_int("C541 item", spec->c541Item,
                     DM1_PC34_CHEST_HAND_SWAP_NO_DUP_C541_ITEM, f0333);
    ok &= expect_int("hand item", spec->handItem,
                     DM1_PC34_CHEST_HAND_SWAP_NO_DUP_HAND_ITEM, f0302);
    ok &= expect_int("same-type hand", spec->sameTypeHand,
                     DM1_PC34_CHEST_HAND_SWAP_NO_DUP_SAME_TYPE_HAND, f0302);
    ok &= expect_int("same-type chest", spec->sameTypeChest,
                     DM1_PC34_CHEST_HAND_SWAP_NO_DUP_SAME_TYPE_CHEST, f0302);
    ok &= expect_int("none sentinel", spec->noneSentinel, 0xFFFF,
                     "ReDMCSB DEFS.H C0xFFFF_THING_NONE");
    ok &= expect_int("end sentinel", spec->endSentinel, 0xFFFE,
                     "ReDMCSB DEFS.H C0xFFFE_THING_ENDOFLIST");
    ok &= expect_int("action hand mask container", spec->actionHandMaskContainer,
                     DM1_PC34_ALLOWED_CONTAINER,
                     "ReDMCSB DATA.C:320-357 MASK0x0400_CONTAINER");
    ok &= expect_int("assertion budget", spec->assertionBudget,
                     DM1_PC34_CHEST_HAND_SWAP_NO_DUP_ASSERTION_BUDGET, f0333);
    return ok;
}

static int test_probe(const DM1_V1_ChestHandSwapNoDuplicateGateProbePc34* probe)
{
    const char* f0333 =
        "ReDMCSB CHEST.C F0333:30-67 + 70-74 (G0425_aT_ChestSlots / C537..C544)";
    const char* f0334 =
        "ReDMCSB CHEST.C F0334:113-132 (F0334 close-loop / L1026 / F0163)";
    const char* f0297 =
        "ReDMCSB CHAMPION.C F0297:243-268 (F0297 put leader hand)";
    const char* f0298 =
        "ReDMCSB CHAMPION.C F0298:270-298 (F0298 remove leader hand)";
    const char* f0300 =
        "ReDMCSB CHAMPION.C F0300:511-584 (F0300 remove G0425 slot)";
    const char* f0301 =
        "ReDMCSB CHAMPION.C F0301:606-660 (F0301 add G0425 slot)";
    const char* f0302 =
        "ReDMCSB CHAMPION.C F0302:662-713 (F0302 slot click dispatcher)";
    const char* f0302Reject =
        "ReDMCSB CHAMPION.C F0302:694-700 (empty/empty and incompatible mask rejection)";
    const char* f0163 =
        "ReDMCSB DUNGEON.C F0163:1769-1838 (F0163 link thing to list)";
    int ok = 1;

    /* Spec / disjointness */
    ok &= expect_int("source-locked contract only",
                     probe->sourceLockedContractOnly, 1, f0333);
    ok &= expect_int("disjoint from pass797", probe->disjointFromPass797, 1,
                     "ReDMCSB pass797 auto-close press-eye disjointness");
    ok &= expect_int("disjoint from pass706", probe->disjointFromPass706, 1,
                     "ReDMCSB pass706 chest slot-to-slot disjointness");
    ok &= expect_int("disjoint from hand_swap_with_chest",
                     probe->disjointFromHandSwapWithChest, 1,
                     "ReDMCSB hand_swap_with_chest per-slot click result disjointness");
    ok &= expect_int("disjoint from chest_round_trip",
                     probe->disjointFromChestRoundTrip, 1,
                     "ReDMCSB chest_round_trip_hand_swap disjointness");
    ok &= expect_int("disjoint from scroll-wheel",
                     probe->disjointFromScrollWheel, 1,
                     "ReDMCSB scroll-wheel pickup/drop disjointness");
    ok &= expect_int("disjoint from C040 mirror",
                     probe->disjointFromC040Mirror, 1,
                     "ReDMCSB c040_cancel_reopen_pickup mirror disjointness");

    /* Open + close/reopen */
    ok &= expect_int("chest open result", probe->chestOpenResult, 1, f0333);
    ok &= expect_int("chest open thing", probe->chestOpenThing,
                     DM1_PC34_CHEST_HAND_SWAP_NO_DUP_CHEST_THING, f0333);
    ok &= expect_int("reopen result", probe->reopenResult, 1, f0333);
    ok &= expect_int("reopen thing", probe->reopenThing,
                     DM1_PC34_CHEST_HAND_SWAP_NO_DUP_REOPEN_THING, f0333);
    ok &= expect_int("closed count", probe->closedCount, 5, f0334);
    ok &= expect_int("reopened count", probe->reopenedCount, 5, f0333);

    /* Scenario A: empty hand + occupied C540 -> pickup. */
    ok &= expect_int("A hand before type", probe->handTypeBeforeEmptyPickup,
                     0, f0298);
    ok &= expect_int("A slot index", probe->slotIndexEmptyPickup,
                     DM1_PC34_CHEST_HAND_SWAP_NO_DUP_C540_INDEX, f0333);
    ok &= expect_int("A click accepted", probe->clickAcceptedEmptyPickup, 1,
                     f0302);
    ok &= expect_int("A hand after type", probe->handTypeAfterEmptyPickup,
                     DM1_PC34_CHEST_HAND_SWAP_NO_DUP_C540_ITEM, f0297);
    ok &= expect_int("A hand after weight", probe->handWeightAfterEmptyPickup,
                     6, f0301);
    ok &= expect_int("A slot after type", probe->slotTypeAfterEmptyPickup, 0,
                     f0300);
    ok &= expect_int("A pre-click non-zero count",
                     probe->preClickNonZeroCountEmptyPickup, 5, f0333);
    ok &= expect_int("A post-click non-zero count",
                     probe->postEmptyPickupNonZeroCount, 5, f0333);
    ok &= expect_int("A union stable", probe->unionStableAfterEmptyPickup, 1,
                     f0302);
    ok &= expect_int("A no duplicate", probe->noDuplicateAfterEmptyPickup, 1,
                     f0302);
    ok &= expect_int("A non-empty count after",
                     probe->nonEmptyCountAfterEmptyPickup, 5, f0300);

    /* Scenario B: full hand + empty C542 -> drop. */
    ok &= expect_int("B hand before type", probe->handTypeBeforeFullDrop,
                     DM1_PC34_CHEST_HAND_SWAP_NO_DUP_HAND_ITEM, f0298);
    ok &= expect_int("B slot index", probe->slotIndexFullDrop,
                     DM1_PC34_CHEST_HAND_SWAP_NO_DUP_C542_INDEX, f0333);
    ok &= expect_int("B click accepted", probe->clickAcceptedFullDrop, 1,
                     f0302);
    ok &= expect_int("B hand after type", probe->handTypeAfterFullDrop, 0,
                     f0297);
    ok &= expect_int("B slot after type", probe->slotTypeAfterFullDrop,
                     DM1_PC34_CHEST_HAND_SWAP_NO_DUP_HAND_ITEM, f0301);
    ok &= expect_int("B pre-click non-zero count",
                     probe->preClickNonZeroCountFullDrop, 6, f0333);
    ok &= expect_int("B post-click non-zero count",
                     probe->postFullDropNonZeroCount, 6, f0333);
    ok &= expect_int("B union stable", probe->unionStableAfterFullDrop, 1,
                     f0302);
    ok &= expect_int("B no duplicate", probe->noDuplicateAfterFullDrop, 1,
                     f0302);
    ok &= expect_int("B non-empty count after",
                     probe->nonEmptyCountAfterFullDrop, 6, f0301);

    /* Scenario C: full hand + occupied C538 -> swap. */
    ok &= expect_int("C hand before type", probe->handTypeBeforeFullSwap,
                     DM1_PC34_CHEST_HAND_SWAP_NO_DUP_HAND_ITEM, f0298);
    ok &= expect_int("C slot index", probe->slotIndexFullSwap,
                     DM1_PC34_CHEST_HAND_SWAP_NO_DUP_C538_INDEX, f0333);
    ok &= expect_int("C click accepted", probe->clickAcceptedFullSwap, 1,
                     f0302);
    ok &= expect_int("C hand after type", probe->handTypeAfterFullSwap,
                     DM1_PC34_CHEST_HAND_SWAP_NO_DUP_C538_ITEM, f0297);
    ok &= expect_int("C slot after type", probe->slotTypeAfterFullSwap,
                     DM1_PC34_CHEST_HAND_SWAP_NO_DUP_HAND_ITEM, f0301);
    ok &= expect_int("C hand weight after", probe->handWeightAfterFullSwap, 7,
                     f0301);
    ok &= expect_int("C pre-click non-zero count",
                     probe->preClickNonZeroCountFullSwap, 6, f0333);
    ok &= expect_int("C post-click non-zero count",
                     probe->postFullSwapNonZeroCount, 6, f0333);
    ok &= expect_int("C union stable", probe->unionStableAfterFullSwap, 1,
                     f0302);
    ok &= expect_int("C no duplicate", probe->noDuplicateAfterFullSwap, 1,
                     f0302);
    ok &= expect_int("C non-empty count after",
                     probe->nonEmptyCountAfterFullSwap, 6, f0300);

    /* Scenario D: leader hand = C539, click C539 slot -> same-type swap.
     * The chain carries 2 copies of C539_ITEM before and after; the
     * multiset stability is the source-faithful contract. The
     * no-duplicate guard is intentionally NOT asserted for this
     * scenario. */
    ok &= expect_int("D hand before type", probe->handTypeBeforeSameTypeClick,
                     DM1_PC34_CHEST_HAND_SWAP_NO_DUP_SAME_TYPE_HAND, f0298);
    ok &= expect_int("D click accepted", probe->clickAcceptedSameTypeClick, 1,
                     f0302);
    ok &= expect_int("D hand after type", probe->handTypeAfterSameTypeClick,
                     DM1_PC34_CHEST_HAND_SWAP_NO_DUP_SAME_TYPE_HAND, f0297);
    ok &= expect_int("D slot after type", probe->slotTypeAfterSameTypeClick,
                     DM1_PC34_CHEST_HAND_SWAP_NO_DUP_SAME_TYPE_CHEST, f0301);
    ok &= expect_int("D pre-click non-zero count",
                     probe->preClickNonZeroCountSameType, 6, f0333);
    ok &= expect_int("D post-click non-zero count",
                     probe->postSameTypeNonZeroCount, 6, f0333);
    ok &= expect_int("D union stable", probe->unionStableAfterSameTypeClick, 1,
                     f0302);
    ok &= expect_int("D same-type carries duplicate",
                     probe->noDuplicateAfterSameTypeClick, 0,
                     "ReDMCSB F0302:700-710 identical-type swap carries 2 copies of the same id");
    ok &= expect_int("D slot count before", probe->slotCountBeforeSameTypeClick,
                     2, f0333);
    ok &= expect_int("D slot count after", probe->slotCountAfterSameTypeClick,
                     2, f0302);

    /* Scenario E: empty hand + empty C543 -> no-op. */
    ok &= expect_int("E hand before type", probe->handTypeBeforeEmptyEmptyNoop,
                     0, f0298);
    ok &= expect_int("E click accepted", probe->clickAcceptedEmptyEmptyNoop, 0,
                     f0302Reject);
    ok &= expect_int("E hand after type", probe->handTypeAfterEmptyEmptyNoop,
                     0, f0297);
    ok &= expect_int("E pre-click non-zero count",
                     probe->preClickNonZeroCountEmptyEmptyNoop, 5, f0333);
    ok &= expect_int("E post-click non-zero count",
                     probe->postEmptyEmptyNoopNonZeroCount, 5, f0333);
    ok &= expect_int("E union stable", probe->unionStableAfterEmptyEmptyNoop, 1,
                     f0302Reject);
    ok &= expect_int("E no duplicate", probe->noDuplicateAfterEmptyEmptyNoop, 1,
                     f0333);

    /* Round-trip close/reopen invariants. */
    ok &= expect_int("round-trip pre-click non-zero count",
                     probe->preClickNonZeroCountRoundTrip, 6, f0333);
    ok &= expect_int("round-trip union stable",
                     probe->unionStableAcrossCloseReopen, 1, f0334);
    ok &= expect_int("round-trip no duplicate",
                     probe->noDuplicateAcrossCloseReopen, 1, f0333);
    ok &= expect_int("round-trip full hand loaded after reopen",
                     probe->fullHandLoadedAfterReopen, 1, f0297);
    ok &= expect_int("round-trip no orphan item",
                     probe->noOrphanItemAcrossRoundTrip, 1, f0163);
    ok &= expect_int("closed hand type", probe->closedHandType,
                     DM1_PC34_CHEST_HAND_SWAP_NO_DUP_HAND_ITEM, f0298);
    ok &= expect_int("reopened hand type", probe->reopenedHandType,
                     DM1_PC34_CHEST_HAND_SWAP_NO_DUP_HAND_ITEM, f0297);
    {
        const int expectedClosed[DM1_PC34_CHEST_HAND_SWAP_NO_DUP_SLOT_COUNT] = {
            DM1_PC34_CHEST_HAND_SWAP_NO_DUP_C537_ITEM,
            DM1_PC34_CHEST_HAND_SWAP_NO_DUP_C538_ITEM,
            DM1_PC34_CHEST_HAND_SWAP_NO_DUP_C539_ITEM,
            DM1_PC34_CHEST_HAND_SWAP_NO_DUP_C540_ITEM,
            DM1_PC34_CHEST_HAND_SWAP_NO_DUP_C541_ITEM,
            0, 0, 0
        };
        const int expectedReopened[DM1_PC34_CHEST_HAND_SWAP_NO_DUP_SLOT_COUNT] = {
            DM1_PC34_CHEST_HAND_SWAP_NO_DUP_C537_ITEM,
            DM1_PC34_CHEST_HAND_SWAP_NO_DUP_C538_ITEM,
            DM1_PC34_CHEST_HAND_SWAP_NO_DUP_C539_ITEM,
            DM1_PC34_CHEST_HAND_SWAP_NO_DUP_C540_ITEM,
            DM1_PC34_CHEST_HAND_SWAP_NO_DUP_C541_ITEM,
            0, 0, 0
        };

        ok &= assert_view("closed types", probe->closedTypes, expectedClosed,
                          DM1_PC34_CHEST_HAND_SWAP_NO_DUP_SLOT_COUNT, f0334);
        ok &= assert_view("reopened types", probe->reopenedTypes,
                          expectedReopened,
                          DM1_PC34_CHEST_HAND_SWAP_NO_DUP_SLOT_COUNT, f0333);
    }

    return ok;
}

static int test_source_evidence(void)
{
    const char* evidence =
        dm1_v1_chest_hand_swap_no_duplicate_gate_source_evidence_pc34();
    int ok = 1;

    ok &= expect_str_contains("source F0333", evidence,
                              "CHEST.C F0333:30-67",
                              "ReDMCSB CHEST.C F0333:30-67");
    ok &= expect_str_contains("source F0334", evidence,
                              "CHEST.C F0334:113-132",
                              "ReDMCSB CHEST.C F0334:113-132");
    ok &= expect_str_contains("source F0297", evidence,
                              "F0297_CHAMPION_PutObjectInLeaderHand",
                              "ReDMCSB CHAMPION.C F0297:243-268");
    ok &= expect_str_contains("source F0298", evidence,
                              "F0298_CHAMPION_GetObjectRemovedFromLeaderHand",
                              "ReDMCSB CHAMPION.C F0298:270-298");
    ok &= expect_str_contains("source F0300", evidence,
                              "F0300_CHAMPION_GetObjectRemovedFromSlot",
                              "ReDMCSB CHAMPION.C F0300:511-584");
    ok &= expect_str_contains("source F0301", evidence,
                              "F0301_CHAMPION_AddObjectInSlot",
                              "ReDMCSB CHAMPION.C F0301:606-660");
    ok &= expect_str_contains("source F0302", evidence,
                              "F0302_CHAMPION_ProcessCommands28To65_ClickOnSlotBox",
                              "ReDMCSB CHAMPION.C F0302:662-713");
    ok &= expect_str_contains("source F0302 reject", evidence,
                              "F0302:694-700",
                              "ReDMCSB CHAMPION.C F0302:694-700");
    ok &= expect_str_contains("source F0347", evidence,
                              "PANEL.C F0347:1639-1691",
                              "ReDMCSB PANEL.C F0347:1639-1691");
    ok &= expect_str_contains("source DEFS", evidence,
                              "C0xFFFF_THING_NONE",
                              "ReDMCSB DEFS.H C0xFFFF_THING_NONE");
    ok &= expect_str_contains("source PANEL.F0347", evidence,
                              "PANEL.C F0347:1639-1691",
                              "ReDMCSB PANEL.C F0347:1639-1691");
    return ok;
}

int main(void)
{
    const char* f0333 =
        "ReDMCSB CHEST.C F0333:30-67 + 70-74 (G0425_aT_ChestSlots / C537..C544)";
    DM1_V1_ChestHandSwapNoDuplicateGateProbePc34 probe;
    const DM1_V1_ChestHandSwapNoDuplicateGateSpecPc34* spec =
        dm1_v1_chest_hand_swap_no_duplicate_gate_spec_pc34();
    int ok = 1;

    printf("probe=dm1_v1_chest_hand_swap_no_duplicate_gate_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           dm1_v1_chest_hand_swap_no_duplicate_gate_source_evidence_pc34());

    ok &= expect_int("probe setup",
                     dm1_v1_chest_hand_swap_no_duplicate_gate_run_pc34(&probe),
                     1, f0333);
    if (!ok) {
        printf("assertionCount=%d failures=%d\n",
               g_assertions, g_failures);
        printf("chestHandSwapNoDuplicateInvariantOk=0\n");
        return 1;
    }

    ok &= test_spec(spec);
    ok &= test_source_evidence();
    ok &= test_probe(&probe);
    ok &= expect_int("minimum assertion count",
                     g_assertions >=
                     DM1_PC34_CHEST_HAND_SWAP_NO_DUP_ASSERTION_BUDGET ?
                     1 : 0, 1, f0333);
    ok &= expect_int("no test failures",
                     g_failures == 0 ? 1 : 0, 1,
                     "ReDMCSB F0302:700-710 chain-preserving swap contract");

    printf("assertionCount=%d failures=%d\n",
           g_assertions, g_failures);
    printf("chestHandSwapNoDuplicateInvariantOk=%d\n", ok ? 1 : 0);
    return ok ? 0 : 1;
}

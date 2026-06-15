#include "dm1_v1_chest_pickup_after_party_rotate_with_non_leader_open_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static DM1_V1_ChestPickupAfterPartyRotateProbePc34 g_probe;
static int g_assertions;
static int g_failures;

static int expect_int(const char* label,
                      int got,
                      int want,
                      const char* anchor)
{
    ++g_assertions;
    if (!anchor || anchor[0] == '\0') {
        ++g_failures;
        printf("FAIL %s missing-anchor\n", label);
        return 0;
    }
    if (got != want) {
        ++g_failures;
        printf("FAIL %s got=%d want=%d anchor=%s\n",
               label, got, want, anchor);
        return 0;
    }
    printf("PASS %s=%d anchor=%s\n", label, got, anchor);
    return 1;
}

static int expect_uint32(const char* label,
                         uint32_t got,
                         uint32_t want,
                         const char* anchor)
{
    ++g_assertions;
    if (!anchor || anchor[0] == '\0') {
        ++g_failures;
        printf("FAIL %s missing-anchor\n", label);
        return 0;
    }
    if (got != want) {
        ++g_failures;
        printf("FAIL %s got=0x%08X want=0x%08X anchor=%s\n",
               label, (unsigned)got, (unsigned)want, anchor);
        return 0;
    }
    printf("PASS %s=0x%08X anchor=%s\n",
           label, (unsigned)got, anchor);
    return 1;
}

static int expect_contains(const char* label,
                           const char* haystack,
                           const char* needle,
                           const char* anchor)
{
    ++g_assertions;
    if (!anchor || anchor[0] == '\0' || !haystack || !needle ||
        !strstr(haystack, needle)) {
        ++g_failures;
        printf("FAIL %s missing=%s anchor=%s\n",
               label, needle ? needle : "(null)",
               anchor ? anchor : "(null)");
        return 0;
    }
    printf("PASS %s contains=%s anchor=%s\n", label, needle, anchor);
    return 1;
}

static int expected_type_at_initial(int index)
{
    return DM1_PC34_CHEST_PICKUP_AFTER_ROTATE_FIRST_ITEM + index;
}

static int expected_weight_at_initial(int index)
{
    return 5 + index;
}

static int expected_direction_after_mode(int mode, int championIndex)
{
    int delta =
        mode == DM1_PC34_CHEST_PICKUP_AFTER_ROTATE_MODE_DIRECTION ? 1 : 2;

    return (championIndex + delta) & 3;
}

static int test_source_evidence(
    const DM1_V1_ChestPickupAfterPartyRotateSpecPc34* spec)
{
    const char* evidence =
        dm1_v1_chest_pickup_after_party_rotate_with_non_leader_open_source_evidence_pc34();
    int ok = 1;

    ok &= expect_contains("sourceEvidence F0333 same-open", evidence,
                          "CHEST.C F0333:30-32",
                          spec->chestSameOpenAnchor);
    ok &= expect_contains("sourceEvidence F0333 materialize", evidence,
                          "CHEST.C F0333:53-76",
                          spec->chestMaterializeAnchor);
    ok &= expect_contains("sourceEvidence F0284 direction", evidence,
                          "CHAMPION.C F0284:93-130",
                          spec->partyDirectionAnchor);
    ok &= expect_contains("sourceEvidence F0284 alternate rotate", evidence,
                          "CHAMPION.C F0284:131-178",
                          spec->leaderSwapAnchor);
    ok &= expect_contains("sourceEvidence F0368 leader switch", evidence,
                          "CLIKCHAM.C F0368:51-68",
                          spec->leaderSwapAnchor);
    ok &= expect_contains("sourceEvidence F0302", evidence,
                          "CHAMPION.C F0302:677-699",
                          spec->slotDispatchAnchor);
    ok &= expect_contains("sourceEvidence defs", evidence,
                          "C537..C544", spec->defsAnchor);
    return ok;
}

static int test_spec(
    const DM1_V1_ChestPickupAfterPartyRotateSpecPc34* spec)
{
    int ok = 1;

    ok &= expect_int("spec party count", spec->partyCount, 2,
                     spec->partyDirectionAnchor);
    ok &= expect_int("spec old leader", spec->oldLeaderIndex,
                     DM1_PC34_CHEST_PICKUP_AFTER_ROTATE_OLD_LEADER,
                     spec->leaderSwapAnchor);
    ok &= expect_int("spec new leader", spec->newLeaderIndex,
                     DM1_PC34_CHEST_PICKUP_AFTER_ROTATE_NEW_LEADER,
                     spec->leaderSwapAnchor);
    ok &= expect_int("spec non-leader inventory champion",
                     spec->inventoryChampionIndex,
                     DM1_PC34_CHEST_PICKUP_AFTER_ROTATE_INVENTORY_CHAMPION,
                     spec->slotDispatchAnchor);
    ok &= expect_int("spec picked index", spec->pickedChestIndex,
                     DM1_PC34_CHEST_PICKUP_AFTER_ROTATE_PICKED_INDEX,
                     spec->chestMaterializeAnchor);
    ok &= expect_int("spec picked pc34 slot", spec->pickedPc34Slot,
                     DM1_PC34_CHEST_PICKUP_AFTER_ROTATE_PICKED_PC34_SLOT,
                     spec->defsAnchor);
    ok &= expect_int("spec C537", spec->c537Pc34Slot,
                     DM1_PC34_SLOT_CHEST_1, spec->defsAnchor);
    ok &= expect_int("spec C544", spec->c544Pc34Slot,
                     DM1_PC34_SLOT_CHEST_8, spec->defsAnchor);
    ok &= expect_int("spec expected visible count",
                     spec->expectedVisibleCount,
                     DM1_PC34_CHEST_PICKUP_AFTER_ROTATE_SLOT_COUNT,
                     spec->chestSameOpenAnchor);
    return ok;
}

static int test_case_header(
    const DM1_V1_ChestPickupAfterPartyRotateCasePc34* c,
    const DM1_V1_ChestPickupAfterPartyRotateSpecPc34* spec)
{
    int ok = 1;

    ok &= expect_int("case runtime marker", c->sourceLockedContractOnly, 0,
                     spec->slotDispatchAnchor);
    ok &= expect_int("case party count", c->partyCount, spec->partyCount,
                     spec->partyDirectionAnchor);
    ok &= expect_int("case old leader", c->oldLeaderIndex,
                     spec->oldLeaderIndex, spec->leaderSwapAnchor);
    ok &= expect_int("case new leader", c->newLeaderIndex,
                     spec->newLeaderIndex, spec->leaderSwapAnchor);
    ok &= expect_int("case inventory champion",
                     c->inventoryChampionIndex,
                     spec->inventoryChampionIndex,
                     spec->slotDispatchAnchor);
    ok &= expect_int("leader before rotation",
                     c->leaderBeforeRotation,
                     spec->oldLeaderIndex,
                     spec->leaderSwapAnchor);
    ok &= expect_int("leader after rotation",
                     c->leaderAfterRotation,
                     spec->newLeaderIndex,
                     spec->leaderSwapAnchor);
    ok &= expect_int("open result", c->openResult, 1,
                     spec->chestMaterializeAnchor);
    ok &= expect_int("open chest thing", c->openChestThing,
                     DM1_PC34_CHEST_PICKUP_AFTER_ROTATE_CHEST_THING,
                     spec->chestMaterializeAnchor);
    ok &= expect_int("same-open result", c->sameOpenResult, 1,
                     spec->chestSameOpenAnchor);
    ok &= expect_int("same-open chest thing", c->sameOpenChestThing,
                     DM1_PC34_CHEST_PICKUP_AFTER_ROTATE_CHEST_THING,
                     spec->chestSameOpenAnchor);
    return ok;
}

static int test_rotation(
    const DM1_V1_ChestPickupAfterPartyRotateCasePc34* c,
    const DM1_V1_ChestPickupAfterPartyRotateSpecPc34* spec)
{
    int ok = 1;
    int wantDirection =
        c->mode == DM1_PC34_CHEST_PICKUP_AFTER_ROTATE_MODE_DIRECTION ? 1 : 2;

    ok &= expect_int("party direction before",
                     c->partyDirectionBefore, 0,
                     spec->partyDirectionAnchor);
    ok &= expect_int("party direction after",
                     c->partyDirectionAfter, wantDirection,
                     spec->partyDirectionAnchor);
    ok &= expect_int("old leader direction after",
                     c->oldLeaderDirectionAfter,
                     expected_direction_after_mode(c->mode, 0),
                     spec->partyDirectionAnchor);
    ok &= expect_int("new leader direction after",
                     c->newLeaderDirectionAfter,
                     expected_direction_after_mode(c->mode, 1),
                     spec->partyDirectionAnchor);
    ok &= expect_int("old leader cell after",
                     c->oldLeaderCellAfter,
                     expected_direction_after_mode(c->mode, 0),
                     spec->partyDirectionAnchor);
    ok &= expect_int("new leader cell after",
                     c->newLeaderCellAfter,
                     expected_direction_after_mode(c->mode, 1),
                     spec->partyDirectionAnchor);
    return ok;
}

static int test_hands_and_reject(
    const DM1_V1_ChestPickupAfterPartyRotateCasePc34* c,
    const DM1_V1_ChestPickupAfterPartyRotateSpecPc34* spec)
{
    int ok = 1;

    ok &= expect_int("old leader hand before",
                     c->oldLeaderHandBeforeType,
                     DM1_PC34_CHEST_PICKUP_AFTER_ROTATE_OLD_HAND_ITEM,
                     spec->leaderSwapAnchor);
    ok &= expect_int("old leader hand after",
                     c->oldLeaderHandAfterType, 0,
                     spec->leaderSwapAnchor);
    ok &= expect_int("old leader hand dropped",
                     c->oldLeaderHandDropped, 1,
                     spec->leaderSwapAnchor);
    ok &= expect_int("new leader hand before",
                     c->newLeaderHandBeforeType,
                     DM1_PC34_CHEST_PICKUP_AFTER_ROTATE_NEW_HAND_ITEM,
                     spec->slotDispatchAnchor);
    ok &= expect_int("new leader hand after",
                     c->newLeaderHandAfterType,
                     DM1_PC34_CHEST_PICKUP_AFTER_ROTATE_NEW_HAND_ITEM,
                     spec->slotDispatchAnchor);
    ok &= expect_int("new leader hand preserved",
                     c->newLeaderHandPreserved, 1,
                     spec->slotDispatchAnchor);
    ok &= expect_int("in-flight pickup refused",
                     c->inFlightPickupResult, 0,
                     spec->slotDispatchAnchor);
    ok &= expect_int("refused because leader changed",
                     c->refusedBecauseLeaderChanged, 1,
                     spec->leaderSwapAnchor);
    ok &= expect_int("refused because new leader hand full",
                     c->refusedBecauseNewLeaderHandFull, 1,
                     spec->slotDispatchAnchor);
    ok &= expect_int("rejected before slot swap",
                     c->rejectedBeforeSlotSwap, 1,
                     spec->slotDispatchAnchor);
    ok &= expect_int("picked item count after reject",
                     c->pickedItemCountAfterReject, 1,
                     spec->slotDispatchAnchor);
    ok &= expect_int("new leader hand item count after reject",
                     c->newLeaderHandItemCountAfterReject, 1,
                     spec->slotDispatchAnchor);
    return ok;
}

static int test_chest_order(
    const DM1_V1_ChestPickupAfterPartyRotateCasePc34* c,
    const DM1_V1_ChestPickupAfterPartyRotateSpecPc34* spec)
{
    int i;
    int ok = 1;

    ok &= expect_int("chest link state intact",
                     c->chestLinkStateIntact, 1,
                     spec->chestSameOpenAnchor);
    ok &= expect_int("reopen order preserved",
                     c->reopenOrderPreserved, 1,
                     spec->chestSameOpenAnchor);
    ok &= expect_int("visible count after reject",
                     c->visibleCountAfterReject,
                     spec->expectedVisibleCount,
                     spec->chestMaterializeAnchor);
    ok &= expect_int("picked item type",
                     c->pickedItemType,
                     expected_type_at_initial(
                         DM1_PC34_CHEST_PICKUP_AFTER_ROTATE_PICKED_INDEX),
                     spec->chestMaterializeAnchor);

    for (i = 0; i < DM1_PC34_CHEST_PICKUP_AFTER_ROTATE_SLOT_COUNT; ++i) {
        char label[96];

        snprintf(label, sizeof(label), "initial C%d type", 537 + i);
        ok &= expect_int(label, c->initialTypes[i],
                         expected_type_at_initial(i),
                         spec->chestMaterializeAnchor);
        snprintf(label, sizeof(label), "after reject C%d type", 537 + i);
        ok &= expect_int(label, c->afterRejectTypes[i],
                         expected_type_at_initial(i),
                         spec->slotDispatchAnchor);
        snprintf(label, sizeof(label), "reopened C%d type", 537 + i);
        ok &= expect_int(label, c->reopenedTypes[i],
                         expected_type_at_initial(i),
                         spec->chestSameOpenAnchor);
        snprintf(label, sizeof(label), "initial C%d weight", 537 + i);
        ok &= expect_int(label, c->initialWeights[i],
                         expected_weight_at_initial(i),
                         spec->chestMaterializeAnchor);
        snprintf(label, sizeof(label), "after reject C%d weight", 537 + i);
        ok &= expect_int(label, c->afterRejectWeights[i],
                         expected_weight_at_initial(i),
                         spec->slotDispatchAnchor);
        snprintf(label, sizeof(label), "reopened C%d weight", 537 + i);
        ok &= expect_int(label, c->reopenedWeights[i],
                         expected_weight_at_initial(i),
                         spec->chestSameOpenAnchor);
    }
    return ok;
}

static int test_case(
    const DM1_V1_ChestPickupAfterPartyRotateCasePc34* c,
    const DM1_V1_ChestPickupAfterPartyRotateSpecPc34* spec)
{
    int ok = 1;

    ok &= test_case_header(c, spec);
    ok &= test_rotation(c, spec);
    ok &= test_hands_and_reject(c, spec);
    ok &= test_chest_order(c, spec);
    return ok;
}

int main(void)
{
    const DM1_V1_ChestPickupAfterPartyRotateSpecPc34* spec =
        dm1_v1_chest_pickup_after_party_rotate_with_non_leader_open_spec_pc34();
    int ok = 1;

    printf("probe=dm1_v1_chest_pickup_after_party_rotate_with_non_leader_open_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           dm1_v1_chest_pickup_after_party_rotate_with_non_leader_open_source_evidence_pc34());

    ok &= test_source_evidence(spec);
    ok &= test_spec(spec);
    ok &= expect_int("probe run",
                     dm1_v1_chest_pickup_after_party_rotate_with_non_leader_open_run_pc34(
                         &g_probe),
                     1, spec->slotDispatchAnchor);
    ok &= expect_int("probe case count", g_probe.caseCount, 2,
                     spec->partyDirectionAnchor);
    ok &= test_case(&g_probe.directionCase, spec);
    ok &= test_case(&g_probe.leaderSwapCase, spec);
    ok &= expect_uint32("deterministic hash",
                        g_probe.deterministicHash,
                        0xB93FAC47u,
                        spec->chestSameOpenAnchor);

    printf("deterministicHash=0x%08X\n",
           (unsigned)g_probe.deterministicHash);
    printf("assertions=%d failures=%d\n", g_assertions, g_failures);
    if (!ok || g_failures || g_assertions < 120 || g_assertions > 220) {
        printf("FAIL dm1_v1_chest_pickup_after_party_rotate_with_non_leader_open_pc34_compat\n");
        return 1;
    }
    printf("PASS dm1_v1_chest_pickup_after_party_rotate_with_non_leader_open_pc34_compat assertions=%d failures=%d deterministicHash=0x%08X\n",
           g_assertions, g_failures, (unsigned)g_probe.deterministicHash);
    return 0;
}

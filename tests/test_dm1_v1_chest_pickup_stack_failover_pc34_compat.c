#include "dm1_v1_chest_pickup_stack_failover_pc34_compat.h"

#include <stdio.h>
#include <string.h>

/*
 * Test harness for the DM1 V1 stackable chest pickup failover gate.
 *
 * ReDMCSB anchors asserted below:
 * CHEST.C F0333:30-32 and 53-67, CHEST.C F0334:117-132,
 * OBJECT.C F0032/F0033:121-212, AMMO.C F0294:54-79, and
 * BLITMASK.C F0133:30-33.
 */

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
        printf("FAIL %s missing=%s anchor=%s\n",
               label, needle ? needle : "(null)", redmcsbAnchor);
        return 0;
    }
    printf("PASS %s contains=%s anchor=%s\n",
           label, needle, redmcsbAnchor);
    return 1;
}

static int test_spec_and_source(void)
{
    const DM1_V1_ChestPickupStackFailoverSpecPc34* spec =
        dm1_v1_chest_pickup_stack_failover_spec_pc34();
    const char* f0333Guard = spec->f0333AlreadyOpenAnchor;
    const char* f0333Dispatch = spec->f0333DispatchAnchor;
    const char* f0334 = spec->f0334RewireAnchor;
    const char* object = spec->objectClassifyAnchor;
    const char* ammo = spec->ammoQuiverAnchor;
    const char* blit = spec->blitmaskHandRedrawAnchor;
    int ok = 1;

    ok &= expect_int("spec present", spec != NULL ? 1 : 0, 1, f0333Guard);
    ok &= expect_int("spec case count", spec->expectedCaseCount, 4,
                     f0333Dispatch);
    ok &= expect_int("spec free slots", spec->expectedFreeInventorySlots, 2,
                     f0333Dispatch);
    ok &= expect_int("spec target slot", spec->targetChestPc34Slot,
                     DM1_PC34_SLOT_CHEST_1, f0333Dispatch);
    ok &= expect_int("spec sentinel slot", spec->sentinelChestPc34Slot,
                     DM1_PC34_SLOT_CHEST_2, f0334);
    ok &= expect_contains("F0333 guard range", f0333Guard,
                          "CHEST.C F0333:30-32", f0333Guard);
    ok &= expect_contains("F0333 guard G0426", f0333Guard,
                          "G0426_T_OpenChest", f0333Guard);
    ok &= expect_contains("F0333 dispatch range", f0333Dispatch,
                          "CHEST.C F0333:53-67", f0333Dispatch);
    ok &= expect_contains("F0333 dispatch G0425", f0333Dispatch,
                          "G0425_aT_ChestSlots", f0333Dispatch);
    ok &= expect_contains("F0334 range", f0334,
                          "CHEST.C F0334:117-132", f0334);
    ok &= expect_contains("F0334 non-empty visible", f0334,
                          "non-empty visible", f0334);
    ok &= expect_contains("OBJECT classify range", object,
                          "OBJECT.C F0032/F0033:121-212", object);
    ok &= expect_contains("OBJECT classify stack sentinels", object,
                          "stack kinds", object);
    ok &= expect_contains("AMMO quiver range", ammo,
                          "AMMO.C F0294:54-79", ammo);
    ok &= expect_contains("AMMO quiver class", ammo,
                          "ammunition classes", ammo);
    ok &= expect_contains("BLITMASK range", blit,
                          "BLITMASK.C F0133:30-33", blit);
    ok &= expect_contains("source runtime",
                          dm1_v1_chest_pickup_stack_failover_source_evidence_pc34(),
                          "runtime=1", f0333Dispatch);
    ok &= expect_contains("source four cases",
                          dm1_v1_chest_pickup_stack_failover_source_evidence_pc34(),
                          "empty-hand pickup", f0333Dispatch);
    return ok;
}

static int expect_common_case(
    const char* prefix,
    const DM1_V1_ChestPickupStackFailoverCasePc34* c,
    const char* f0333,
    const char* f0334)
{
    int ok = 1;

    ok &= expect_int("case setup", c->setupResult, 1, f0333);
    ok &= expect_int("case open result", c->openResult, 1, f0333);
    ok &= expect_int("case open thing", c->openThing,
                     DM1_PC34_CHEST_STACK_FAILOVER_CHEST_THING, f0333);
    ok &= expect_int("case chest before bolt", c->chestBeforeType,
                     DM1_PC34_CHEST_STACK_FAILOVER_BOLT, f0333);
    ok &= expect_int("case chest stack kind", c->chestBeforeStackKind,
                     DM1_PC34_CHEST_STACK_KIND_BOLT, f0333);
    ok &= expect_int("case sentinel before", c->sentinelBeforeType,
                     DM1_PC34_CHEST_STACK_FAILOVER_SENTINEL, f0333);
    ok &= expect_int("case first free slot", c->firstFreeSlot,
                     DM1_PC34_CHEST_STACK_FAILOVER_FREE_A, f0333);
    ok &= expect_int("case second free slot", c->secondFreeSlot,
                     DM1_PC34_CHEST_STACK_FAILOVER_FREE_B, f0333);
    ok &= expect_int("case at least two free slots",
                     c->freeSlotCountBefore >= 2 ? 1 : 0, 1, f0333);
    ok &= expect_int("case picked removed from chest",
                     c->pickedRemovedFromChest, 1, f0334);
    ok &= expect_int("case chest after empty", c->chestAfterType, 0, f0334);
    ok &= expect_int("case sentinel after", c->sentinelAfterType,
                     DM1_PC34_CHEST_STACK_FAILOVER_SENTINEL, f0334);
    ok &= expect_int("case close count", c->closeCount, 1, f0334);
    ok &= expect_int("case closed first sentinel", c->closedFirstType,
                     DM1_PC34_CHEST_STACK_FAILOVER_SENTINEL, f0334);
    ok &= expect_int("case closed excludes picked",
                     c->closedContainsPickedType, 0, f0334);
    ok &= expect_int("case sentinel preserved", c->sentinelPreserved, 1,
                     f0334);
    ok &= expect_int(prefix, c->visibleWeightAfterPickup, 5, f0334);
    return ok;
}

static int test_non_stack_case(
    const DM1_V1_ChestPickupStackFailoverCasePc34* c,
    const DM1_V1_ChestPickupStackFailoverSpecPc34* spec)
{
    const char* f0333 = spec->f0333DispatchAnchor;
    const char* f0334 = spec->f0334RewireAnchor;
    const char* object = spec->objectClassifyAnchor;
    const char* blit = spec->blitmaskHandRedrawAnchor;
    int ok = expect_common_case("non-stack visible weight", c, f0333, f0334);

    ok &= expect_int("non-stack hand busy", c->handBusyBefore, 1, f0333);
    ok &= expect_int("non-stack hand type before", c->handBeforeType,
                     DM1_PC34_CHEST_STACK_FAILOVER_NON_STACK, object);
    ok &= expect_int("non-stack kind before", c->handBeforeStackKind,
                     DM1_PC34_CHEST_STACK_KIND_NONE, object);
    ok &= expect_int("non-stack outcome routed", c->outcome,
                     DM1_PC34_CHEST_STACK_OUTCOME_ROUTED_FREE_SLOT, f0333);
    ok &= expect_int("non-stack routed slot", c->routedSlot,
                     DM1_PC34_CHEST_STACK_FAILOVER_FREE_A, f0333);
    ok &= expect_int("non-stack routed type", c->routedSlotAfterType,
                     DM1_PC34_CHEST_STACK_FAILOVER_BOLT, f0333);
    ok &= expect_int("non-stack routed charges", c->routedSlotAfterCharges,
                     3, object);
    ok &= expect_int("non-stack hand preserved", c->handPreserved, 1, blit);
    ok &= expect_int("non-stack hand after", c->handAfterType,
                     DM1_PC34_CHEST_STACK_FAILOVER_NON_STACK, blit);
    ok &= expect_int("non-stack picked preserved",
                     c->pickedPreservedSomewhere, 1, f0333);
    ok &= expect_int("non-stack second free remains",
                     c->secondFreeSlotStayedFree, 1, f0333);
    return ok;
}

static int test_same_stack_case(
    const DM1_V1_ChestPickupStackFailoverCasePc34* c,
    const DM1_V1_ChestPickupStackFailoverSpecPc34* spec)
{
    const char* f0333 = spec->f0333DispatchAnchor;
    const char* f0334 = spec->f0334RewireAnchor;
    const char* object = spec->objectClassifyAnchor;
    const char* blit = spec->blitmaskHandRedrawAnchor;
    int ok = expect_common_case("same-stack visible weight", c, f0333, f0334);

    ok &= expect_int("same-stack hand busy", c->handBusyBefore, 1, f0333);
    ok &= expect_int("same-stack before kind", c->handBeforeStackKind,
                     DM1_PC34_CHEST_STACK_KIND_BOLT, object);
    ok &= expect_int("same-stack flag", c->sameStackBefore, 1, object);
    ok &= expect_int("same-stack outcome merge", c->outcome,
                     DM1_PC34_CHEST_STACK_OUTCOME_MERGED_HAND, object);
    ok &= expect_int("same-stack hand type after", c->handAfterType,
                     DM1_PC34_CHEST_STACK_FAILOVER_BOLT, blit);
    ok &= expect_int("same-stack merged weight", c->handAfterWeight, 7,
                     object);
    ok &= expect_int("same-stack merged charges", c->handAfterCharges, 7,
                     object);
    ok &= expect_int("same-stack routed slot untouched",
                     c->routedSlotAfterType, 0, f0333);
    ok &= expect_int("same-stack no route", c->routedSlot, -1, f0333);
    ok &= expect_int("same-stack picked preserved",
                     c->pickedPreservedSomewhere, 1, object);
    ok &= expect_int("same-stack second free remains",
                     c->secondFreeSlotStayedFree, 1, f0333);
    return ok;
}

static int test_different_stack_case(
    const DM1_V1_ChestPickupStackFailoverCasePc34* c,
    const DM1_V1_ChestPickupStackFailoverSpecPc34* spec)
{
    const char* f0333Guard = spec->f0333AlreadyOpenAnchor;
    const char* f0333 = spec->f0333DispatchAnchor;
    const char* object = spec->objectClassifyAnchor;
    const char* ammo = spec->ammoQuiverAnchor;
    const char* blit = spec->blitmaskHandRedrawAnchor;
    int ok = expect_common_case("different-stack visible weight", c, f0333,
                                spec->f0334RewireAnchor);

    ok &= expect_int("different-stack hand busy", c->handBusyBefore, 1,
                     f0333Guard);
    ok &= expect_int("different-stack hand scroll", c->handBeforeType,
                     DM1_PC34_CHEST_STACK_FAILOVER_SCROLL, object);
    ok &= expect_int("different-stack hand kind", c->handBeforeStackKind,
                     DM1_PC34_CHEST_STACK_KIND_SCROLL, object);
    ok &= expect_int("different-stack same flag", c->sameStackBefore, 0,
                     object);
    ok &= expect_int("different-stack outcome routed", c->outcome,
                     DM1_PC34_CHEST_STACK_OUTCOME_ROUTED_FREE_SLOT, ammo);
    ok &= expect_int("different-stack routed slot", c->routedSlot,
                     DM1_PC34_CHEST_STACK_FAILOVER_FREE_A, ammo);
    ok &= expect_int("different-stack routed type", c->routedSlotAfterType,
                     DM1_PC34_CHEST_STACK_FAILOVER_BOLT, ammo);
    ok &= expect_int("different-stack hand preserved", c->handPreserved, 1,
                     blit);
    ok &= expect_int("different-stack hand after", c->handAfterType,
                     DM1_PC34_CHEST_STACK_FAILOVER_SCROLL, blit);
    ok &= expect_int("different-stack picked preserved",
                     c->pickedPreservedSomewhere, 1, ammo);
    ok &= expect_int("different-stack second free remains",
                     c->secondFreeSlotStayedFree, 1, ammo);
    return ok;
}

static int test_empty_hand_case(
    const DM1_V1_ChestPickupStackFailoverCasePc34* c,
    const DM1_V1_ChestPickupStackFailoverSpecPc34* spec)
{
    const char* f0333 = spec->f0333DispatchAnchor;
    const char* f0334 = spec->f0334RewireAnchor;
    const char* blit = spec->blitmaskHandRedrawAnchor;
    int ok = expect_common_case("empty-hand visible weight", c, f0333,
                                f0334);

    ok &= expect_int("empty-hand starts empty", c->handBusyBefore, 0,
                     f0333);
    ok &= expect_int("empty-hand before type", c->handBeforeType, 0,
                     f0333);
    ok &= expect_int("empty-hand outcome to hand", c->outcome,
                     DM1_PC34_CHEST_STACK_OUTCOME_TO_HAND, f0333);
    ok &= expect_int("empty-hand after bolt", c->handAfterType,
                     DM1_PC34_CHEST_STACK_FAILOVER_BOLT, blit);
    ok &= expect_int("empty-hand after kind", c->handAfterStackKind,
                     DM1_PC34_CHEST_STACK_KIND_BOLT,
                     spec->objectClassifyAnchor);
    ok &= expect_int("empty-hand after weight", c->handAfterWeight, 3,
                     blit);
    ok &= expect_int("empty-hand after charges", c->handAfterCharges, 3,
                     blit);
    ok &= expect_int("empty-hand no route", c->routedSlot, -1, f0333);
    ok &= expect_int("empty-hand free slot untouched",
                     c->routedSlotAfterType, 0, f0333);
    ok &= expect_int("empty-hand picked preserved",
                     c->pickedPreservedSomewhere, 1, blit);
    ok &= expect_int("empty-hand second free remains",
                     c->secondFreeSlotStayedFree, 1, f0333);
    return ok;
}

int main(void)
{
    const DM1_V1_ChestPickupStackFailoverSpecPc34* spec;
    const DM1_V1_ChestPickupStackFailoverProbePc34* probe;
    int runPassed = 0;
    int runFailed = 0;
    int ok = 1;

    printf("probe=dm1_v1_chest_pickup_stack_failover_pc34_compat\n");
    spec = dm1_v1_chest_pickup_stack_failover_spec_pc34();
    ok &= expect_int("run result",
                     dm1_v1_chest_pickup_stack_failover_run(
                         &runPassed, &runFailed),
                     1, spec->f0333DispatchAnchor);
    probe = dm1_v1_chest_pickup_stack_failover_last_probe_pc34();
    ok &= expect_int("internal passed nonzero", runPassed > 0 ? 1 : 0,
                     1, spec->f0333DispatchAnchor);
    ok &= expect_int("internal failed zero", runFailed, 0,
                     spec->f0334RewireAnchor);

    ok &= test_spec_and_source();
    ok &= test_non_stack_case(&probe->nonStackHand, spec);
    ok &= test_same_stack_case(&probe->sameStackHand, spec);
    ok &= test_different_stack_case(&probe->differentStackHand, spec);
    ok &= test_empty_hand_case(&probe->emptyHand, spec);
    ok &= expect_int("minimum assertion count",
                     g_assertions >= 60 ? 1 : 0, 1,
                     spec->f0333DispatchAnchor);

    printf("assertions=%d failures=%d internalPassed=%d internalFailed=%d\n",
           g_assertions, g_failures, runPassed, runFailed);
    printf("chestPickupStackFailoverInvariantOk=%d\n",
           ok && g_failures == 0 ? 1 : 0);
    if (ok && g_failures == 0) {
        printf("PASS dm1_v1_chest_pickup_stack_failover_pc34_compat assertions=%d\n",
               g_assertions);
        return 0;
    }
    return 1;
}

#include "dm1_v1_chest_scroll_wheel_leader_stack_rotation_pc34_compat.h"

#include <stdio.h>
#include <string.h>

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

static int expect_array(const char* label,
                        const int* got,
                        const int* want,
                        int count,
                        const char* anchor)
{
    int i;
    int ok = 1;

    for (i = 0; i < count; ++i) {
        char itemLabel[96];

        snprintf(itemLabel, sizeof(itemLabel), "%s[%d]", label, i);
        ok &= expect_int(itemLabel, got[i], want[i], anchor);
    }
    return ok;
}

static int test_source_evidence(
    const DM1_V1_ChestLeaderStackRotationSpecPc34* spec)
{
    int ok = 1;

    ok &= expect_contains("evidence F0333", spec->sourceEvidence,
                          "CHEST.C F0333:30-67", spec->f0333OpenAnchor);
    ok &= expect_contains("evidence F0334", spec->sourceEvidence,
                          "CHEST.C F0334:113-132", spec->f0334CloseAnchor);
    ok &= expect_contains("evidence F0297", spec->sourceEvidence,
                          "F0297:243-268", spec->f0297PutAnchor);
    ok &= expect_contains("evidence F0298", spec->sourceEvidence,
                          "F0298:270-298", spec->f0298RemoveAnchor);
    ok &= expect_contains("evidence F0301", spec->sourceEvidence,
                          "F0301:606-614", spec->f0301WriteAnchor);
    ok &= expect_contains("evidence F0302", spec->sourceEvidence,
                          "F0302:662-710", spec->f0302DispatchAnchor);
    ok &= expect_contains("evidence command", spec->sourceEvidence,
                          "COMMAND.C F0378:1973-1983",
                          spec->commandDispatchAnchor);
    ok &= expect_contains("evidence panel close", spec->sourceEvidence,
                          "PANEL.C F0354:2307-2344",
                          spec->panelCloseAnchor);
    ok &= expect_contains("evidence mouse", spec->sourceEvidence,
                          "UTAMSCR.C F0077:147-151", spec->mouseAnchor);
    ok &= expect_contains("evidence object", spec->sourceEvidence,
                          "OBJECT.C F0033:147-212", spec->objectAnchor);
    ok &= expect_contains("evidence blit", spec->sourceEvidence,
                          "BLITMASK.C F0133:30-33",
                          spec->blitMaskAnchor);
    ok &= expect_contains("evidence defs", spec->sourceEvidence,
                          "DEFS.H:810-816", spec->defsAnchor);
    return ok;
}

static int test_initial_state(
    const DM1_V1_ChestLeaderStackRotationProbePc34* probe,
    const DM1_V1_ChestLeaderStackRotationSpecPc34* spec)
{
    const int chestWant[DM1_V1_CHEST_LEADER_STACK_ROTATION_SLOT_COUNT] = {
        DM1_V1_CHEST_LEADER_STACK_ROTATION_ITEM0,
        DM1_V1_CHEST_LEADER_STACK_ROTATION_ITEM1,
        DM1_V1_CHEST_LEADER_STACK_ROTATION_ITEM2,
        DM1_V1_CHEST_LEADER_STACK_ROTATION_NONE,
        DM1_V1_CHEST_LEADER_STACK_ROTATION_NONE,
        DM1_V1_CHEST_LEADER_STACK_ROTATION_NONE,
        DM1_V1_CHEST_LEADER_STACK_ROTATION_NONE,
        DM1_V1_CHEST_LEADER_STACK_ROTATION_NONE
    };
    const int leaderWant[DM1_V1_CHEST_LEADER_STACK_ROTATION_STACK_COUNT] = {
        DM1_V1_CHEST_LEADER_STACK_ROTATION_LEADER0,
        DM1_V1_CHEST_LEADER_STACK_ROTATION_LEADER1,
        DM1_V1_CHEST_LEADER_STACK_ROTATION_LEADER2
    };
    int ok = 1;

    ok &= expect_int("C30 constant", DM1_V1_CHEST_LEADER_STACK_ROTATION_C30,
                     30, spec->defsAnchor);
    ok &= expect_int("C537 constant", DM1_V1_CHEST_LEADER_STACK_ROTATION_C537,
                     537, spec->defsAnchor);
    ok &= expect_int("C540 constant", DM1_V1_CHEST_LEADER_STACK_ROTATION_C540,
                     540, spec->defsAnchor);
    ok &= expect_int("C541 constant", DM1_V1_CHEST_LEADER_STACK_ROTATION_C541,
                     541, spec->defsAnchor);
    ok &= expect_int("C542 constant", DM1_V1_CHEST_LEADER_STACK_ROTATION_C542,
                     542, spec->defsAnchor);
    ok &= expect_int("C544 constant", DM1_V1_CHEST_LEADER_STACK_ROTATION_C544,
                     544, spec->defsAnchor);
    ok &= expect_int("open chest thing", probe->openChestThing,
                     DM1_V1_CHEST_LEADER_STACK_ROTATION_OPEN_CHEST,
                     spec->f0333OpenAnchor);
    ok &= expect_array("initial chest", probe->initialChest, chestWant,
                       DM1_V1_CHEST_LEADER_STACK_ROTATION_SLOT_COUNT,
                       spec->f0333OpenAnchor);
    ok &= expect_array("initial leader stack", probe->initialLeaderStack,
                       leaderWant,
                       DM1_V1_CHEST_LEADER_STACK_ROTATION_STACK_COUNT,
                       spec->f0297PutAnchor);
    ok &= expect_int("initial focus C540", probe->focusZoneTrace[0],
                     DM1_V1_CHEST_LEADER_STACK_ROTATION_C540,
                     spec->panelCloseAnchor);
    return ok;
}

static int test_rotation_before_mutation(
    const DM1_V1_ChestLeaderStackRotationProbePc34* probe,
    const DM1_V1_ChestLeaderStackRotationSpecPc34* spec)
{
    const int focusWant[DM1_V1_CHEST_LEADER_STACK_ROTATION_WHEEL_STEPS + 1] = {
        0, 1, 2, 0, 1
    };
    const int zoneWant[DM1_V1_CHEST_LEADER_STACK_ROTATION_WHEEL_STEPS + 1] = {
        DM1_V1_CHEST_LEADER_STACK_ROTATION_C540,
        DM1_V1_CHEST_LEADER_STACK_ROTATION_C541,
        DM1_V1_CHEST_LEADER_STACK_ROTATION_C542,
        DM1_V1_CHEST_LEADER_STACK_ROTATION_C540,
        DM1_V1_CHEST_LEADER_STACK_ROTATION_C541
    };
    int step;
    int ok = 1;

    ok &= expect_array("focus trace", probe->focusTrace, focusWant,
                       DM1_V1_CHEST_LEADER_STACK_ROTATION_WHEEL_STEPS + 1,
                       spec->panelCloseAnchor);
    ok &= expect_array("focus zone trace", probe->focusZoneTrace, zoneWant,
                       DM1_V1_CHEST_LEADER_STACK_ROTATION_WHEEL_STEPS + 1,
                       spec->defsAnchor);
    for (step = 0; step < DM1_V1_CHEST_LEADER_STACK_ROTATION_WHEEL_STEPS;
         ++step) {
        char label[96];

        snprintf(label, sizeof(label), "chest stable before mutation %d",
                 step);
        ok &= expect_int(label, probe->chestStableBeforeMutation[step], 1,
                         spec->f0333OpenAnchor);
        snprintf(label, sizeof(label), "leader stable before mutation %d",
                 step);
        ok &= expect_int(label, probe->leaderStackStableBeforeMutation[step],
                         1, spec->f0297PutAnchor);
    }
    ok &= expect_int("partial masks before mutation",
                     probe->partialMaskDispatches,
                     DM1_V1_CHEST_LEADER_STACK_ROTATION_WHEEL_STEPS,
                     spec->blitMaskAnchor);
    ok &= expect_int("mouse enables before mutation plus commit",
                     probe->mouseEnableCount,
                     DM1_V1_CHEST_LEADER_STACK_ROTATION_WHEEL_STEPS + 1,
                     spec->mouseAnchor);
    ok &= expect_int("mouse disables before mutation plus commit",
                     probe->mouseDisableCount,
                     DM1_V1_CHEST_LEADER_STACK_ROTATION_WHEEL_STEPS + 1,
                     spec->mouseAnchor);
    return ok;
}

static int test_first_mutation_and_close(
    const DM1_V1_ChestLeaderStackRotationProbePc34* probe,
    const DM1_V1_ChestLeaderStackRotationSpecPc34* spec)
{
    const int chestAfterMutation[DM1_V1_CHEST_LEADER_STACK_ROTATION_SLOT_COUNT] = {
        DM1_V1_CHEST_LEADER_STACK_ROTATION_ITEM0,
        DM1_V1_CHEST_LEADER_STACK_ROTATION_ITEM1,
        DM1_V1_CHEST_LEADER_STACK_ROTATION_ITEM2,
        DM1_V1_CHEST_LEADER_STACK_ROTATION_NONE,
        DM1_V1_CHEST_LEADER_STACK_ROTATION_LEADER1,
        DM1_V1_CHEST_LEADER_STACK_ROTATION_NONE,
        DM1_V1_CHEST_LEADER_STACK_ROTATION_NONE,
        DM1_V1_CHEST_LEADER_STACK_ROTATION_NONE
    };
    const int leaderAfterMutation[DM1_V1_CHEST_LEADER_STACK_ROTATION_STACK_COUNT] = {
        DM1_V1_CHEST_LEADER_STACK_ROTATION_LEADER0,
        DM1_V1_CHEST_LEADER_STACK_ROTATION_NONE,
        DM1_V1_CHEST_LEADER_STACK_ROTATION_LEADER2
    };
    const int closedWant[DM1_V1_CHEST_LEADER_STACK_ROTATION_SLOT_COUNT] = {
        DM1_V1_CHEST_LEADER_STACK_ROTATION_ITEM0,
        DM1_V1_CHEST_LEADER_STACK_ROTATION_ITEM1,
        DM1_V1_CHEST_LEADER_STACK_ROTATION_ITEM2,
        DM1_V1_CHEST_LEADER_STACK_ROTATION_LEADER1,
        DM1_V1_CHEST_LEADER_STACK_ROTATION_NONE,
        DM1_V1_CHEST_LEADER_STACK_ROTATION_NONE,
        DM1_V1_CHEST_LEADER_STACK_ROTATION_NONE,
        DM1_V1_CHEST_LEADER_STACK_ROTATION_NONE
    };
    int ok = 1;

    ok &= expect_int("command dispatch count", probe->commandDispatchCount, 1,
                     spec->commandDispatchAnchor);
    ok &= expect_int("F0302 dispatch count", probe->f0302DispatchCount, 1,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("first mutation step", probe->firstMutationStep,
                     DM1_V1_CHEST_LEADER_STACK_ROTATION_WHEEL_STEPS,
                     spec->f0302DispatchAnchor);
    ok &= expect_int("first mutation zone", probe->firstMutationZone,
                     DM1_V1_CHEST_LEADER_STACK_ROTATION_C541,
                     spec->defsAnchor);
    ok &= expect_int("first mutation command slot",
                     probe->firstMutationCommandSlot,
                     DM1_V1_CHEST_LEADER_STACK_ROTATION_C30 + 4,
                     spec->f0301WriteAnchor);
    ok &= expect_int("first mutation leader thing",
                     probe->firstMutationLeaderThing,
                     DM1_V1_CHEST_LEADER_STACK_ROTATION_LEADER1,
                     spec->objectAnchor);
    ok &= expect_int("first mutation chest before empty",
                     probe->firstMutationChestBefore,
                     DM1_V1_CHEST_LEADER_STACK_ROTATION_NONE,
                     spec->f0302DispatchAnchor);
    ok &= expect_array("chest after mutation", probe->chestAfterMutation,
                       chestAfterMutation,
                       DM1_V1_CHEST_LEADER_STACK_ROTATION_SLOT_COUNT,
                       spec->f0301WriteAnchor);
    ok &= expect_array("leader after mutation",
                       probe->leaderStackAfterMutation, leaderAfterMutation,
                       DM1_V1_CHEST_LEADER_STACK_ROTATION_STACK_COUNT,
                       spec->f0298RemoveAnchor);
    ok &= expect_int("F0298 removes selected stack item",
                     probe->f0298RemoveCount, 1, spec->f0298RemoveAnchor);
    ok &= expect_int("F0301 writes selected chest slot",
                     probe->f0301WriteCount, 1, spec->f0301WriteAnchor);
    ok &= expect_int("F0297 no occupied chest pickup",
                     probe->f0297PutCount, 0, spec->f0297PutAnchor);
    ok &= expect_int("load mask after mutation", probe->loadMaskAfterMutation,
                     DM1_V1_CHEST_LEADER_STACK_ROTATION_LOAD_MASK,
                     spec->f0298RemoveAnchor);
    ok &= expect_int("close count", probe->closeCount, 4,
                     spec->f0334CloseAnchor);
    ok &= expect_int("closed head", probe->closedHead,
                     DM1_V1_CHEST_LEADER_STACK_ROTATION_ITEM0,
                     spec->f0334CloseAnchor);
    ok &= expect_array("closed slots", probe->closedSlots, closedWant,
                       DM1_V1_CHEST_LEADER_STACK_ROTATION_SLOT_COUNT,
                       spec->f0334CloseAnchor);
    ok &= expect_int("close end sentinel", probe->closeEndSentinel,
                     DM1_V1_CHEST_LEADER_STACK_ROTATION_END,
                     spec->f0334CloseAnchor);
    ok &= expect_int("open chest after close", probe->openChestAfterClose,
                     DM1_V1_CHEST_LEADER_STACK_ROTATION_NONE,
                     spec->f0334CloseAnchor);
    ok &= expect_int("G0425 cleared after close",
                     probe->g0425ClearedAfterClose, 1,
                     spec->f0334CloseAnchor);
    return ok;
}

int main(void)
{
    DM1_V1_ChestLeaderStackRotationProbePc34 probe;
    const DM1_V1_ChestLeaderStackRotationSpecPc34* spec =
        dm1_v1_chest_scroll_wheel_leader_stack_rotation_spec_pc34();
    int ok = 1;

    ok &= expect_int("probe run",
                     dm1_v1_chest_scroll_wheel_leader_stack_rotation_pc34(
                         &probe),
                     1, spec->f0333OpenAnchor);
    ok &= test_source_evidence(spec);
    ok &= test_initial_state(&probe, spec);
    ok &= test_rotation_before_mutation(&probe, spec);
    ok &= test_first_mutation_and_close(&probe, spec);

    printf("SUMMARY assertions=%d failures=%d\n", g_assertions, g_failures);
    return (ok && g_failures == 0) ? 0 : 1;
}

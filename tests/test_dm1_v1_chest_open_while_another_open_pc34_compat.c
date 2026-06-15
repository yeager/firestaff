#include "dm1_v1_chest_open_while_another_open_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions;

static int expect_int(const char* label, int got, int want,
                      const char* redmcsbAnchor)
{
    ++g_assertions;
    if (!redmcsbAnchor || redmcsbAnchor[0] == '\0') {
        printf("FAIL %s missing ReDMCSB anchor\n", label);
        return 0;
    }
    if (got != want) {
        printf("FAIL %s got=%d want=%d anchor=%s\n", label, got, want,
               redmcsbAnchor);
        return 0;
    }
    printf("PASS %s=%d anchor=%s\n", label, got, redmcsbAnchor);
    return 1;
}

static int expect_less(const char* label, int left, int right,
                       const char* redmcsbAnchor)
{
    ++g_assertions;
    if (!redmcsbAnchor || redmcsbAnchor[0] == '\0') {
        printf("FAIL %s missing ReDMCSB anchor\n", label);
        return 0;
    }
    if (!(left < right)) {
        printf("FAIL %s left=%d right=%d anchor=%s\n", label, left, right,
               redmcsbAnchor);
        return 0;
    }
    printf("PASS %s left=%d right=%d anchor=%s\n", label, left, right,
           redmcsbAnchor);
    return 1;
}

static int expect_contains(const char* label, const char* haystack,
                           const char* needle, const char* redmcsbAnchor)
{
    ++g_assertions;
    if (!redmcsbAnchor || redmcsbAnchor[0] == '\0') {
        printf("FAIL %s missing ReDMCSB anchor\n", label);
        return 0;
    }
    if (!haystack || !needle || !strstr(haystack, needle)) {
        printf("FAIL %s missing=%s anchor=%s\n", label,
               needle ? needle : "(null)", redmcsbAnchor);
        return 0;
    }
    printf("PASS %s contains=%s anchor=%s\n", label, needle, redmcsbAnchor);
    return 1;
}

static int test_source_evidence(void)
{
    const char* media = "ReDMCSB CHEST.C lines 1-31 MEDIA guards";
    const char* change8 =
        "ReDMCSB CHEST.C F0333 lines 36-44 CHANGE8_09_FIX";
    const char* close =
        "ReDMCSB CHEST.C F0334 lines 79-132 close-rewire";
    const char* open =
        "ReDMCSB CHEST.C F0333 lines 47-67 open materialization";
    const char* hand =
        "ReDMCSB CHAMPION.C F0297/F0298 lines 243-298";
    const char* c30 =
        "ReDMCSB CHAMPION.C F0300/F0301/F0302 lines 511-515,606-610,688-710";
    const char* evidence =
        dm1_v1_chest_open_while_another_open_source_evidence_pc34_compat();
    int ok = 1;

    ok &= expect_contains("evidence MEDIA042 guard", evidence, "MEDIA042",
                          media);
    ok &= expect_contains("evidence MEDIA278 guard", evidence, "MEDIA278",
                          media);
    ok &= expect_contains("evidence MEDIA343 guard", evidence, "MEDIA343",
                          media);
    ok &= expect_contains("evidence MEDIA346 guard", evidence, "MEDIA346",
                          media);
    ok &= expect_contains("evidence CHANGE8_09", evidence, "CHANGE8_09_FIX",
                          change8);
    ok &= expect_contains("evidence F0334 close", evidence, "F0334 lines 79-132",
                          close);
    ok &= expect_contains("evidence F0333 open", evidence, "F0333 lines 47-67",
                          open);
    ok &= expect_contains("evidence leader hand", evidence, "F0297/F0298",
                          hand);
    ok &= expect_contains("evidence C30 paths", evidence, "511-515,606-610,688-710",
                          c30);
    return ok;
}

static int test_spec_and_open_a(
    const DM1_V1_ChestOpenWhileAnotherOpenStatePc34* s)
{
    const char* media = "ReDMCSB CHEST.C lines 1-31 MEDIA guards";
    const char* open =
        "ReDMCSB CHEST.C F0333 lines 47-67 open materialization";
    int ok = 1;
    int i;

    ok &= expect_int("contract only", s->contractOnly, 1, media);
    ok &= expect_int("init result", s->initResult, 1, media);
    ok &= expect_int("slot count", s->slotCount,
                     DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_SLOT_COUNT, open);
    ok &= expect_int("thing none sentinel", s->thingNone,
                     DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_THING_NONE, media);
    ok &= expect_int("thing end sentinel", s->thingEndOfList,
                     DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_THING_ENDOFLIST, open);
    ok &= expect_int("chest A thing", s->chestAThing,
                     DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_CHEST_A, open);
    ok &= expect_int("chest B thing", s->chestBThing,
                     DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_CHEST_B, open);
    ok &= expect_int("media guard family present", s->mediaGuardFamilyPresent,
                     1, media);
    ok &= expect_int("G0426 before open A", s->g0426BeforeOpenA,
                     DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_THING_NONE, media);
    ok &= expect_int("open A result", s->openAResult, 1, open);
    ok &= expect_int("G0426 after open A", s->g0426AfterOpenA,
                     DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_CHEST_A, open);
    ok &= expect_int("open A materialized count", s->openAItemCount,
                     DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_A_COUNT, open);
    for (i = 0; i < DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_A_COUNT; ++i) {
        ok &= expect_int("open A G0425 visible item", s->openASlots[i],
                         DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_ITEM_A1 + i,
                         open);
    }
    for (; i < DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_SLOT_COUNT; ++i) {
        ok &= expect_int("open A G0425 empty tail", s->openASlots[i],
                         DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_THING_NONE,
                         open);
    }
    ok &= expect_less("open A assign before panel blit", s->openAAssignEvent,
                      s->openAPanelBlitEvent, open);
    ok &= expect_less("open A panel blit before materialize",
                      s->openAPanelBlitEvent, s->openAMaterializeEvent, open);
    return ok;
}

static int test_change8_close_before_open_b(
    const DM1_V1_ChestOpenWhileAnotherOpenStatePc34* s)
{
    const char* change8 =
        "ReDMCSB CHEST.C F0333 lines 36-44 CHANGE8_09_FIX";
    const char* close =
        "ReDMCSB CHEST.C F0334 lines 79-132 close-rewire";
    int ok = 1;
    int i;

    ok &= expect_int("G0426 before open B is A", s->g0426BeforeOpenB,
                     DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_CHEST_A, change8);
    ok &= expect_int("same chest guard bypassed", s->sameChestGuardBypassed,
                     1, change8);
    ok &= expect_int("another chest guard triggered",
                     s->anotherChestGuardTriggered, 1, change8);
    ok &= expect_int("close called before open B", s->closeCalledBeforeOpenB,
                     1, change8);
    ok &= expect_int("close call count", s->closeCallCount, 1, close);
    ok &= expect_int("close processed chest A", s->closeProcessedChest,
                     DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_CHEST_A, close);
    ok &= expect_int("G0426 reset during close", s->g0426AfterCloseReset,
                     DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_THING_NONE, close);
    ok &= expect_int("close non-empty count", s->closeNonEmptyCount,
                     DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_A_COUNT, close);
    ok &= expect_int("close first non-empty slot", s->closeFirstNonEmptySlot,
                     0, close);
    ok &= expect_int("process first false slot", s->processFirstFalseSlot,
                     0, close);
    ok &= expect_int("close clears all G0425 slots", s->closeClearedSlotsCount,
                     DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_SLOT_COUNT, close);
    for (i = 0; i < DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_SLOT_COUNT; ++i) {
        ok &= expect_int("G0425 after close before B materialize",
                         s->g0425AfterCloseBeforeOpenB[i],
                         DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_THING_NONE, close);
    }
    ok &= expect_less("CHANGE8 guard precedes close reset", s->closeGuardEvent,
                      s->closeOpenResetEvent, change8);
    ok &= expect_less("close reset precedes container END",
                      s->closeOpenResetEvent, s->closeContainerEndEvent, close);
    ok &= expect_less("container END precedes first head assign",
                      s->closeContainerEndEvent, s->firstHeadAssignEvent,
                      close);
    ok &= expect_less("close head assign precedes F0163 append",
                      s->firstHeadAssignEvent, s->firstLinkCallEvent, close);
    ok &= expect_less("F0163 close append precedes open B assignment",
                      s->firstLinkCallEvent, s->openBAssignEvent, change8);
    return ok;
}

static int test_chest_a_rewire(
    const DM1_V1_ChestOpenWhileAnotherOpenStatePc34* s)
{
    const char* close =
        "ReDMCSB CHEST.C F0334 lines 79-132 close-rewire";
    const char* c30 =
        "ReDMCSB CHAMPION.C F0300/F0301/F0302 lines 511-515,606-610,688-710";
    int ok = 1;
    int i;

    ok &= expect_int("chest A slot before close", s->chestASlotBeforeClose,
                     DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_ITEM_A1, close);
    ok &= expect_int("chest A slot restored after close", s->chestASlotAfterClose,
                     DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_ITEM_A1, close);
    ok &= expect_int("chest A closed chain count", s->chestAClosedCount,
                     DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_A_COUNT, close);
    ok &= expect_int("chest A all items linked",
                     s->chestAAllItemsStillLinked, 1, close);
    ok &= expect_int("chest A order preserved", s->chestAOrderPreserved,
                     1, close);
    for (i = 0; i < DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_A_COUNT; ++i) {
        ok &= expect_int("chest A closed chain item",
                         s->chestAClosedChain[i],
                         DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_ITEM_A1 + i,
                         close);
    }
    ok &= expect_int("chest A first next is item two",
                     s->chestANextAfterClose[0],
                     DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_ITEM_A1 + 1, close);
    ok &= expect_int("chest A second next is item three",
                     s->chestANextAfterClose[1],
                     DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_ITEM_A1 + 2, close);
    ok &= expect_int("chest A third next terminates",
                     s->chestANextAfterClose[2],
                     DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_THING_ENDOFLIST,
                     close);
    ok &= expect_int("close calls F0163 twice",
                     s->closeLinkThingToListCalls,
                     DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_A_COUNT - 1, close);
    ok &= expect_int("first F0163 thing", s->linkThingArgs[0],
                     DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_ITEM_A1 + 1, close);
    ok &= expect_int("first F0163 previous", s->linkPreviousArgs[0],
                     DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_ITEM_A1, close);
    ok &= expect_int("second F0163 thing", s->linkThingArgs[1],
                     DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_ITEM_A1 + 2, close);
    ok &= expect_int("second F0163 previous", s->linkPreviousArgs[1],
                     DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_ITEM_A1 + 1, close);
    ok &= expect_int("close does not call F0164",
                     s->closeUnlinkThingFromListCalls, 0, close);
    ok &= expect_int("candidate unlink path not involved",
                     s->candidateUnlinkThingFromListCalls, 0, c30);
    return ok;
}

static int test_chest_b_panel(
    const DM1_V1_ChestOpenWhileAnotherOpenStatePc34* s)
{
    const char* open =
        "ReDMCSB CHEST.C F0333 lines 47-67 open materialization";
    const char* change8 =
        "ReDMCSB CHEST.C F0333 lines 36-44 CHANGE8_09_FIX";
    int ok = 1;
    int i;

    ok &= expect_int("open B result", s->openBResult, 1, open);
    ok &= expect_int("G0426 after open B", s->g0426AfterOpenB,
                     DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_CHEST_B, open);
    ok &= expect_int("open B panel blit count", s->openBPanelBlitCount,
                     1, open);
    ok &= expect_int("open B panel blit thing", s->openBPanelBlitThing,
                     DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_CHEST_B, open);
    ok &= expect_int("open B item count", s->openBItemCount,
                     DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_B_COUNT, open);
    ok &= expect_int("open B panel fully populated",
                     s->openBPanelFullyPopulated, 1, open);
    ok &= expect_int("chest B order preserved", s->chestBOrderPreserved,
                     1, open);
    ok &= expect_int("chest B order does not leak A",
                     s->chestBOrderLeakedA, 0, change8);
    ok &= expect_int("chest B source slot before open",
                     s->chestBSlotBeforeOpen,
                     DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_ITEM_B1, open);
    ok &= expect_int("chest B source slot after open",
                     s->chestBSlotAfterOpen,
                     DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_ITEM_B1, open);
    for (i = 0; i < DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_SLOT_COUNT; ++i) {
        ok &= expect_int("open B G0425 panel item", s->openBSlots[i],
                         DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_ITEM_B1 + i,
                         open);
    }
    ok &= expect_less("open B assignment follows close reset",
                      s->closeOpenResetEvent, s->openBAssignEvent, change8);
    ok &= expect_less("open B assign precedes panel blit",
                      s->openBAssignEvent, s->openBPanelBlitEvent, open);
    ok &= expect_less("open B panel blit precedes materialize",
                      s->openBPanelBlitEvent, s->openBMaterializeEvent, open);
    return ok;
}

static int test_leader_hand_untouched(
    const DM1_V1_ChestOpenWhileAnotherOpenStatePc34* s)
{
    const char* hand =
        "ReDMCSB CHAMPION.C F0297/F0298 lines 243-298";
    int ok = 1;

    ok &= expect_int("leader hand before thing", s->leaderHandBeforeThing,
                     DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_LEADER_HAND, hand);
    ok &= expect_int("leader hand after thing", s->leaderHandAfterThing,
                     s->leaderHandBeforeThing, hand);
    ok &= expect_int("leader hand before weight", s->leaderHandBeforeWeight,
                     DM1_PC34_CHEST_OPEN_WHILE_ANOTHER_LEADER_HAND_WEIGHT,
                     hand);
    ok &= expect_int("leader hand after weight", s->leaderHandAfterWeight,
                     s->leaderHandBeforeWeight, hand);
    ok &= expect_int("leader empty before", s->leaderEmptyBefore, 0, hand);
    ok &= expect_int("leader empty after", s->leaderEmptyAfter,
                     s->leaderEmptyBefore, hand);
    return ok;
}

int main(void)
{
    const char* change8 =
        "ReDMCSB CHEST.C F0333 lines 36-44 CHANGE8_09_FIX";
    const DM1_V1_ChestOpenWhileAnotherOpenStatePc34* state;
    int ok = 1;

    printf("probe=dm1_v1_chest_open_while_another_open_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           dm1_v1_chest_open_while_another_open_source_evidence_pc34_compat());

    ok &= expect_int("driver returns success",
                     dm1_v1_chest_open_while_another_open_chest_a_then_b_pc34_compat(),
                     1, change8);
    state = dm1_v1_chest_open_while_another_open_state_after_pc34_compat();
    ok &= expect_int("state accessor returns state", state != NULL, 1,
                     change8);
    if (!ok || !state) {
        printf("assertionCount=%d\n", g_assertions);
        printf("chestOpenWhileAnotherOpenResultOk=0\n");
        return 1;
    }

    ok &= test_source_evidence();
    ok &= test_spec_and_open_a(state);
    ok &= test_change8_close_before_open_b(state);
    ok &= test_chest_a_rewire(state);
    ok &= test_chest_b_panel(state);
    ok &= test_leader_hand_untouched(state);
    ok &= expect_int("minimum assertion count",
                     g_assertions >= 50 ? 1 : 0, 1, change8);

    printf("assertionCount=%d\n", g_assertions);
    printf("chestOpenWhileAnotherOpenResultOk=%d openChest=%d closeCount=%d "
           "openBCount=%d linkCalls=%d\n",
           ok ? 1 : 0,
           state->g0426AfterOpenB,
           state->closeCallCount,
           state->openBItemCount,
           state->closeLinkThingToListCalls);
    return ok ? 0 : 1;
}

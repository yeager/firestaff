#include "dm1_v1_chest_reopen_after_leader_rotation_pc34_compat.h"

#include <stdio.h>

static M11_GameView_ChestReopenAfterLeaderRotationProbePc34 g_probe;
static int g_assertions;
static int g_failures;

static int expect_int(const char* label, int got, int want,
                      const char* redmcsbAnchor)
{
    ++g_assertions;
    if (!redmcsbAnchor || redmcsbAnchor[0] == '\0') {
        printf("FAIL %s missing ReDMCSB anchor\n", label);
        ++g_failures;
        return 0;
    }
    if (got != want) {
        printf("FAIL %s got=%d want=%d anchor=%s\n",
               label, got, want, redmcsbAnchor);
        ++g_failures;
        return 0;
    }
    printf("ok %s=%d anchor=%s\n", label, got, redmcsbAnchor);
    return 1;
}

static int expect_nonempty_string(const char* label, const char* got,
                                  const char* redmcsbAnchor)
{
    ++g_assertions;
    if (!redmcsbAnchor || redmcsbAnchor[0] == '\0') {
        printf("FAIL %s missing ReDMCSB anchor\n", label);
        ++g_failures;
        return 0;
    }
    if (!got || got[0] == '\0') {
        printf("FAIL %s empty string anchor=%s\n", label, redmcsbAnchor);
        ++g_failures;
        return 0;
    }
    printf("ok %s anchor=%s\n", label, redmcsbAnchor);
    return 1;
}

static int expect_slot_order(
    const char* label,
    const int* got,
    const int* want,
    const char* redmcsbAnchor)
{
    int ok = 1;
    int i;

    for (i = 0;
         i < DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_SLOT_COUNT;
         ++i) {
        char slotLabel[96];

        snprintf(slotLabel, sizeof(slotLabel), "%s slot %d", label, i);
        ok &= expect_int(slotLabel, got[i], want[i], redmcsbAnchor);
    }
    return ok;
}

static int test_probe_spec(void)
{
    const char* defs = "ReDMCSB DEFS.H:2088/C30/G0425/G0426/G0423/G0305";
    const char* chest = "ReDMCSB CHEST.C F0333:31-67";
    int ok = 1;

    ok &= expect_int("contract-only marker",
                     g_probe.sourceLockedContractOnly, 1, chest);
    ok &= expect_int("case count",
                     g_probe.caseCount,
                     DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_CASE_COUNT,
                     defs);
    ok &= expect_int("C0xFFFF none sentinel",
                     g_probe.c0xFFFFThingNone,
                     DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_THING_NONE,
                     defs);
    ok &= expect_int("C537 slot constant",
                     g_probe.c537Pc34Slot, DM1_PC34_SLOT_CHEST_1, defs);
    ok &= expect_int("C544 slot constant",
                     g_probe.c544Pc34Slot, DM1_PC34_SLOT_CHEST_8, defs);
    ok &= expect_int("chest slot count",
                     g_probe.chestSlotCount, DM1_PC34_CHEST_SLOT_COUNT, defs);
    ok &= expect_nonempty_string(
        "source evidence",
        M11_GameView_ChestReopenAfterLeaderRotationSourceEvidencePc34(),
        chest);
    return ok;
}

static int test_common_case_contract(
    const M11_GameView_ChestReopenAfterLeaderRotationCasePc34* c)
{
    const char* f0333 = "ReDMCSB CHEST.C F0333:31-67";
    const char* f0334 = "ReDMCSB CHEST.C F0334:113-132";
    const char* f0359 = "ReDMCSB COMMAND.C F0359 M568/C040";
    const char* defs = "ReDMCSB DEFS.H:2088/C30/G0425/G0426/G0423/G0305";
    int ok = 1;

    ok &= expect_nonempty_string("case name", c->caseName, f0333);
    ok &= expect_int("case index matches",
                     g_probe.cases[c->caseIndex].caseIndex, c->caseIndex,
                     defs);
    ok &= expect_int("party has four champions",
                     c->context.partyChampionCount, 4, defs);
    ok &= expect_int("roster ordinal 1",
                     c->context.partyRosterOrdinals[0], 1, defs);
    ok &= expect_int("roster ordinal 2",
                     c->context.partyRosterOrdinals[1], 2, defs);
    ok &= expect_int("original leader ordinal",
                     c->context.originalLeaderOrdinal, 1, defs);
    ok &= expect_int("original inventory champion ordinal",
                     c->context.originalInventoryChampionOrdinal, 1, defs);
    ok &= expect_int("current leader ordinal after rotation",
                     c->context.currentLeaderOrdinalAfterRotation, 2, f0359);
    ok &= expect_int("action log has at least six entries",
                     c->actionLog.count >= 6 ? 1 : 0, 1, f0359);
    ok &= expect_int("first action opens chest A",
                     c->actionLog.entries[0].action,
                     M11_DM1_PC34_CHEST_REOPEN_LEADER_ROTATION_ACTION_OPEN_CHEST_A,
                     f0333);
    ok &= expect_int("second action closes chest A",
                     c->actionLog.entries[1].action,
                     M11_DM1_PC34_CHEST_REOPEN_LEADER_ROTATION_ACTION_CLOSE_CHEST_A,
                     f0334);
    ok &= expect_int("third action rotates leader",
                     c->actionLog.entries[2].action,
                     M11_DM1_PC34_CHEST_REOPEN_LEADER_ROTATION_ACTION_ROTATE_LEADER,
                     f0359);
    ok &= expect_int("fourth action opens chest B",
                     c->actionLog.entries[3].action,
                     M11_DM1_PC34_CHEST_REOPEN_LEADER_ROTATION_ACTION_OPEN_CHEST_B,
                     f0333);
    ok &= expect_int("final action reopens chest B",
                     c->actionLog.entries[c->actionLog.count - 1].action,
                     M11_DM1_PC34_CHEST_REOPEN_LEADER_ROTATION_ACTION_REOPEN_CHEST_B,
                     f0333);
    ok &= expect_int("chest A close count",
                     c->expected.closeCountChestA,
                     DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_SLOT_COUNT,
                     f0334);
    ok &= expect_int("chest B close count",
                     c->expected.closeCountChestB,
                     DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_SLOT_COUNT,
                     f0334);
    ok &= expect_int("reopened visible count",
                     c->expected.reopenedVisibleCount,
                     DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_SLOT_COUNT,
                     f0333);
    ok &= expect_int("chest A link head preserved",
                     c->expected.chestALinkHead,
                     c->context.chestAVisibleSlots[0], f0334);
    ok &= expect_int("chest B link head preserved",
                     c->expected.chestBLinkHead,
                     c->context.chestBVisibleSlots[0], f0334);
    ok &= expect_slot_order("slots after rotation",
                            c->expected.slotsAfterRotation,
                            c->context.chestBVisibleSlots, f0359);
    ok &= expect_slot_order("visible order on reopen",
                            c->expected.visibleSlotOrderOnReopen,
                            c->context.chestBVisibleSlots, f0333);
    ok &= expect_int("final open chest is B",
                     c->expected.finalOpenChestThing,
                     c->context.chestBThing, f0333);
    ok &= expect_int("inventory champion ordinal preserved",
                     c->expected.finalInventoryChampionOrdinal,
                     c->context.originalInventoryChampionOrdinal, defs);
    ok &= expect_int("no detached C30+ occupant",
                     c->expected.noDetachedC30PlusOccupant, 1, defs);
    ok &= expect_int("leader hand identity preserved",
                     c->expected.leaderHandIdentityPreserved, 1,
                     "ReDMCSB CHAMPION.C F0297:243-298/F0298");
    ok &= expect_int("chest B visible order preserved",
                     c->expected.chestBVisibleOrderPreserved, 1,
                     "ReDMCSB DUNGEON.C F0163:1796-1837");
    return ok;
}

static int test_basic_rotation_reopen(void)
{
    const M11_GameView_ChestReopenAfterLeaderRotationCasePc34* c =
        &g_probe.cases[
            DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_CASE_BASIC];
    int ok = test_common_case_contract(c);

    ok &= expect_int("basic final leader is rotated champion",
                     c->expected.finalLeaderOrdinal, 2,
                     "ReDMCSB COMMAND.C F0359 M568/C040");
    ok &= expect_int("basic rotation count",
                     c->expected.rotationCount, 1,
                     "ReDMCSB COMMAND.C F0359 M568/C040");
    ok &= expect_int("basic leader hand empty",
                     c->expected.leaderHandAfter,
                     DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_THING_NONE,
                     "ReDMCSB CHAMPION.C F0298");
    ok &= expect_int("basic empty no-op preserved",
                     c->expected.emptyLeaderHandNoopPreserved, 1,
                     "ReDMCSB CHAMPION.C F0298");
    return ok;
}

static int test_non_empty_leader_hand_rotation(void)
{
    const M11_GameView_ChestReopenAfterLeaderRotationCasePc34* c =
        &g_probe.cases[
            DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_CASE_FULL_HAND];
    int ok = test_common_case_contract(c);

    ok &= expect_int("full hand starts with amulet",
                     c->context.leaderHandThing,
                     DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_LEADER_AMULET,
                     "ReDMCSB CHAMPION.C F0297:243-298");
    ok &= expect_int("full hand after reopen is amulet",
                     c->expected.leaderHandAfter,
                     DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_LEADER_AMULET,
                     "ReDMCSB CHAMPION.C F0297:243-298/F0298");
    ok &= expect_int("full hand still full",
                     c->expected.fullLeaderHandStillFull, 1,
                     "ReDMCSB CHAMPION.C F0297:243-298");
    ok &= expect_int("full hand not hidden tail",
                     c->expected.hiddenTailStaysWithLeader, 0,
                     "ReDMCSB CHAMPION.C F0300/F0301/F0302");
    return ok;
}

static int test_double_rotation_before_reopen(void)
{
    const M11_GameView_ChestReopenAfterLeaderRotationCasePc34* c =
        &g_probe.cases[
            DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_CASE_DOUBLE_ROTATE];
    int ok = test_common_case_contract(c);

    ok &= expect_int("double rotate action count",
                     c->actionLog.count, 7,
                     "ReDMCSB COMMAND.C F0359 M568/C040");
    ok &= expect_int("double rotate back action",
                     c->actionLog.entries[5].action,
                     M11_DM1_PC34_CHEST_REOPEN_LEADER_ROTATION_ACTION_ROTATE_LEADER_BACK,
                     "ReDMCSB COMMAND.C F0359 M568/C040");
    ok &= expect_int("double rotate count",
                     c->expected.rotationCount, 2,
                     "ReDMCSB COMMAND.C F0359 M568/C040");
    ok &= expect_int("double rotate returns final leader to A",
                     c->expected.finalLeaderOrdinal,
                     c->context.originalLeaderOrdinal,
                     "ReDMCSB COMMAND.C F0359 M568/C040");
    return ok;
}

static int test_hidden_tail_leader_hand_mid_close(void)
{
    const M11_GameView_ChestReopenAfterLeaderRotationCasePc34* c =
        &g_probe.cases[
            DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_CASE_HIDDEN_TAIL_HAND];
    int ok = test_common_case_contract(c);

    ok &= expect_int("hidden tail hand input",
                     c->context.leaderHandThing,
                     DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_HIDDEN_TAIL,
                     "ReDMCSB CHAMPION.C F0297:243-298");
    ok &= expect_int("hidden tail mid-close rotate action",
                     c->actionLog.entries[4].action,
                     M11_DM1_PC34_CHEST_REOPEN_LEADER_ROTATION_ACTION_ROTATE_LEADER,
                     "ReDMCSB COMMAND.C F0359 M568/C040");
    ok &= expect_int("hidden tail final leader changed mid-close",
                     c->expected.finalLeaderOrdinal, 3,
                     "ReDMCSB COMMAND.C F0359 M568/C040");
    ok &= expect_int("hidden tail rotation count",
                     c->expected.rotationCount, 2,
                     "ReDMCSB COMMAND.C F0359 M568/C040");
    ok &= expect_int("hidden tail remains in leader hand",
                     c->expected.leaderHandAfter,
                     DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_HIDDEN_TAIL,
                     "ReDMCSB CHAMPION.C F0297:243-298/F0298");
    ok &= expect_int("hidden tail stays with leader flag",
                     c->expected.hiddenTailStaysWithLeader, 1,
                     "ReDMCSB CHAMPION.C F0300/F0301/F0302");
    ok &= expect_int("hidden tail final identity",
                     c->expected.hiddenTailAfter,
                     DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_HIDDEN_TAIL,
                     "ReDMCSB CHAMPION.C F0297:243-298/F0298");
    ok &= expect_int("hidden tail not in chest B link",
                     c->expected.chestBLinkHead ==
                         DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_HIDDEN_TAIL ?
                         0 : 1,
                     1, "ReDMCSB DUNGEON.C F0163:1796-1837");
    return ok;
}

static int test_close_with_full_leader_hand(void)
{
    const M11_GameView_ChestReopenAfterLeaderRotationCasePc34* c =
        &g_probe.cases[
            DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_CASE_CLOSE_FULL_HAND];
    int ok = test_common_case_contract(c);

    ok &= expect_int("close full hand starts with shield",
                     c->context.leaderHandThing,
                     DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_LEADER_SHIELD,
                     "ReDMCSB CHAMPION.C F0297:243-298");
    ok &= expect_int("close full hand after reopen shield",
                     c->expected.leaderHandAfter,
                     DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_LEADER_SHIELD,
                     "ReDMCSB CHAMPION.C F0297:243-298/F0298");
    ok &= expect_int("close full leader hand still full",
                     c->expected.fullLeaderHandStillFull, 1,
                     "ReDMCSB CHEST.C F0334:113-132");
    ok &= expect_int("close full hand final leader rotated",
                     c->expected.finalLeaderOrdinal, 2,
                     "ReDMCSB COMMAND.C F0359 M568/C040");
    return ok;
}

static int test_empty_slot_noop_rotation(void)
{
    const M11_GameView_ChestReopenAfterLeaderRotationCasePc34* c =
        &g_probe.cases[
            DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_CASE_EMPTY_NOOP];
    int ok = test_common_case_contract(c);

    ok &= expect_int("empty no-op hand starts empty",
                     c->context.leaderHandThing,
                     DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_THING_NONE,
                     "ReDMCSB CHAMPION.C F0298");
    ok &= expect_int("empty no-op hand after reopen empty",
                     c->expected.leaderHandAfter,
                     DM1_PC34_CHEST_REOPEN_AFTER_LEADER_ROTATION_THING_NONE,
                     "ReDMCSB CHAMPION.C F0298");
    ok &= expect_int("empty no-op flag preserved",
                     c->expected.emptyLeaderHandNoopPreserved, 1,
                     "ReDMCSB CHAMPION.C F0300/F0301/F0302");
    ok &= expect_int("empty no-op final leader rotated",
                     c->expected.finalLeaderOrdinal, 2,
                     "ReDMCSB COMMAND.C F0359 M568/C040");
    return ok;
}

int main(void)
{
    int ok = 1;

    printf("probe=dm1_v1_chest_reopen_after_leader_rotation_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           M11_GameView_ChestReopenAfterLeaderRotationSourceEvidencePc34());

    ok &= expect_int(
        "probe setup",
        M11_GameView_ChestReopenAfterLeaderRotationRunPc34(&g_probe), 1,
        "ReDMCSB CHEST.C F0333:31-67");
    if (!ok) {
        printf("assertionCount=%d failures=%d\n", g_assertions, g_failures);
        return 1;
    }

    ok &= test_probe_spec();
    ok &= test_basic_rotation_reopen();
    ok &= test_non_empty_leader_hand_rotation();
    ok &= test_double_rotation_before_reopen();
    ok &= test_hidden_tail_leader_hand_mid_close();
    ok &= test_close_with_full_leader_hand();
    ok &= test_empty_slot_noop_rotation();
    ok &= expect_int("minimum assertion budget",
                     g_assertions >= 80 ? 1 : 0, 1,
                     "ReDMCSB CHEST.C F0333:31-67");

    printf("assertionCount=%d failures=%d\n", g_assertions, g_failures);
    if (ok && g_failures == 0) {
        printf("PASS dm1_v1_chest_reopen_after_leader_rotation_pc34_compat "
               "assertions=%d\n",
               g_assertions);
    }
    return ok && g_failures == 0 ? 0 : 1;
}

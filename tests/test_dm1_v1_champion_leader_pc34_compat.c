#include "dm1_v1_champion_leader_pc34_compat.h"

/*
 * pass795 - DM1 V1 champion leader state contract test
 * (CLIKCHAM.C F0367 status-box nested G0455 dispatch + F0368 set-leader
 * state transition with old-leader dirty marking, leader-hand weight
 * remove/add, G0411 set/clear, candidate leader redraw).
 * Source-locked against CLIKCHAM.C F0367:24-35 + F0368:51-72 +
 * DEFS.H:8090-8100 (C00..C03 champion indices, G0411, G0455) +
 * DEFS.H:1874-1878 (CM1_CHAMPION_NONE).
 */

#include <stdio.h>

static int g_assertions = 0;
static int g_failures = 0;

static void expect_int(const char *id, int got, int want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=%d want=%d at %s\n", id, got, want, anchor);
        ++g_failures;
    } else {
        printf("PASS %s == %d (%s)\n", id, want, anchor);
    }
}

static void expect_mask(const char *id, unsigned int got, unsigned int mask,
                        const char *anchor)
{
    ++g_assertions;
    if ((got & mask) != mask) {
        printf("FAIL %s got=0x%04x missing=0x%04x at %s\n",
               id, got, mask, anchor);
        ++g_failures;
    } else {
        printf("PASS %s has 0x%04x (%s)\n", id, mask, anchor);
    }
}

static void seed_live_party(Dm1V1ChampionLeaderStatePc34Compat *state,
                            int leaderIndex,
                            int handWeight,
                            unsigned int candidateChampionOrdinal)
{
    int i;

    DM1_V1_ChampionLeader_InitPc34Compat(state);
    state->leaderIndex = leaderIndex;
    state->partyDirection = 2;
    state->leaderHandWeight = handWeight;
    state->candidateChampionOrdinal = candidateChampionOrdinal;
    for (i = 0; i < DM1_V1_CHAMPION_LEADER_COUNT_PC34_COMPAT; ++i) {
        state->champions[i].currentHealth = 40 + i;
        state->champions[i].direction = i;
        state->champions[i].load = 100 + (i * 60);
        state->champions[i].attributes = 0u;
    }
    if (leaderIndex >= 0 && leaderIndex < DM1_V1_CHAMPION_LEADER_COUNT_PC34_COMPAT) {
        state->champions[leaderIndex].load += handWeight;
    }
}

static void test_same_leader_is_ignored(void)
{
    Dm1V1ChampionLeaderStatePc34Compat state;
    Dm1V1ChampionLeaderSetResultPc34Compat result;

    seed_live_party(&state, 1, 7, 0u);

    /* ReDMCSB CLIKCHAM.C F0368 lines 51-53 returns before detaching G0411
     * when the clicked champion is already the leader. */
    expect_int("same.return",
               DM1_V1_ChampionLeader_SetPc34Compat(&state, 1, &result),
               0, "CLIKCHAM.C F0368:51-53");
    expect_int("same.previous", result.previousLeaderIndex, 1,
               "CLIKCHAM.C F0368:51 G0411 compare");
    expect_int("same.new", result.newLeaderIndex, 1,
               "CLIKCHAM.C F0368:51-53 no G0411 write");
    expect_int("same.flag", result.ignoredSameLeader, 1,
               "CLIKCHAM.C F0368:51-53 same-leader return");
    expect_int("same.dead_flag", result.ignoredDeadTarget, 0,
               "CLIKCHAM.C F0368:51 dead branch not taken");
    expect_int("same.old_detached", result.oldLeaderDetached, 0,
               "CLIKCHAM.C F0368:54 not reached");
    expect_int("same.new_attached", result.newLeaderAttached, 0,
               "CLIKCHAM.C F0368:66 not reached");
    expect_int("same.old_draw", result.oldLeaderDrawStateCount, 0,
               "CLIKCHAM.C F0368:58 not reached");
    expect_int("same.new_draw", result.newLeaderDrawStateCount, 0,
               "CLIKCHAM.C F0368:71 not reached");
    expect_int("same.leader", state.leaderIndex, 1,
               "CLIKCHAM.C F0368:51-53 leaves G0411 unchanged");
    expect_int("same.load", state.champions[1].load, 167,
               "CLIKCHAM.C F0368:56 not reached");
    expect_int("same.attributes", (int)state.champions[1].attributes, 0,
               "CLIKCHAM.C F0368:55 not reached");
}

static void test_dead_target_is_ignored(void)
{
    Dm1V1ChampionLeaderStatePc34Compat state;
    Dm1V1ChampionLeaderSetResultPc34Compat result;

    seed_live_party(&state, 0, 11, 0u);
    state.champions[2].currentHealth = 0;

    /* ReDMCSB CLIKCHAM.C F0368 lines 51-53 rejects a dead non-CM1 target
     * before old leader load/name dirties, hand-weight removal, or redraw. */
    expect_int("dead.return",
               DM1_V1_ChampionLeader_SetPc34Compat(&state, 2, &result),
               0, "CLIKCHAM.C F0368:51-53");
    expect_int("dead.previous", result.previousLeaderIndex, 0,
               "CLIKCHAM.C F0368:51 G0411 compare");
    expect_int("dead.new", result.newLeaderIndex, 0,
               "CLIKCHAM.C F0368:51-53 no G0411 write");
    expect_int("dead.flag", result.ignoredDeadTarget, 1,
               "CLIKCHAM.C F0368:51-53 dead-target return");
    expect_int("dead.same_flag", result.ignoredSameLeader, 0,
               "CLIKCHAM.C F0368:51 same branch not taken");
    expect_int("dead.old_detached", result.oldLeaderDetached, 0,
               "CLIKCHAM.C F0368:54 not reached");
    expect_int("dead.new_attached", result.newLeaderAttached, 0,
               "CLIKCHAM.C F0368:66 not reached");
    expect_int("dead.old_draw", result.oldLeaderDrawStateCount, 0,
               "CLIKCHAM.C F0368:58 not reached");
    expect_int("dead.new_draw", result.newLeaderDrawStateCount, 0,
               "CLIKCHAM.C F0368:71 not reached");
    expect_int("dead.leader", state.leaderIndex, 0,
               "CLIKCHAM.C F0368:51-53 leaves G0411 unchanged");
    expect_int("dead.old_load", state.champions[0].load, 111,
               "CLIKCHAM.C F0368:56 not reached");
    expect_int("dead.target_load", state.champions[2].load, 220,
               "CLIKCHAM.C F0368:68 not reached");
}

static void test_live_target_swaps_leader(void)
{
    Dm1V1ChampionLeaderStatePc34Compat state;
    Dm1V1ChampionLeaderSetResultPc34Compat result;
    const unsigned int oldDirty =
        DM1_V1_CHAMPION_ATTR_LOAD_PC34_COMPAT |
        DM1_V1_CHAMPION_ATTR_NAME_TITLE_PC34_COMPAT;
    const unsigned int newDirty =
        DM1_V1_CHAMPION_ATTR_ICON_PC34_COMPAT |
        DM1_V1_CHAMPION_ATTR_LOAD_PC34_COMPAT |
        DM1_V1_CHAMPION_ATTR_NAME_TITLE_PC34_COMPAT;

    seed_live_party(&state, 0, 7, 0u);

    /* ReDMCSB CLIKCHAM.C F0368 lines 54-59 must dirty/redraw the old G0411
     * champion and clear G0411 before lines 66-72 attach the live target. */
    expect_int("swap.return",
               DM1_V1_ChampionLeader_SetPc34Compat(&state, 2, &result),
               1, "CLIKCHAM.C F0368:54-72");
    expect_int("swap.previous", result.previousLeaderIndex, 0,
               "CLIKCHAM.C F0368:54-55 old G0411 snapshot");
    expect_int("swap.new", result.newLeaderIndex, 2,
               "CLIKCHAM.C F0368:66 assigns G0411");
    expect_int("swap.old_detached", result.oldLeaderDetached, 1,
               "CLIKCHAM.C F0368:54-59 old leader detach");
    expect_int("swap.new_attached", result.newLeaderAttached, 1,
               "CLIKCHAM.C F0368:66-68 new leader attach");
    expect_int("swap.leader", state.leaderIndex, 2,
               "CLIKCHAM.C F0368:66 assigns G0411");
    expect_int("swap.old_load", state.champions[0].load, 100,
               "CLIKCHAM.C F0368:56 removes leader-hand weight");
    expect_int("swap.new_load", state.champions[2].load, 227,
               "CLIKCHAM.C F0368:68 adds leader-hand weight");
    expect_int("swap.new_direction", state.champions[2].direction,
               state.partyDirection, "CLIKCHAM.C F0368:67 copies party direction");
    expect_mask("swap.old_attrs", state.champions[0].attributes, oldDirty,
                "CLIKCHAM.C F0368:55 LOAD|NAME dirty");
    expect_int("swap.old_icon_clear",
               (int)(state.champions[0].attributes & DM1_V1_CHAMPION_ATTR_ICON_PC34_COMPAT),
               0, "CLIKCHAM.C F0368:55 does not set ICON on old leader");
    expect_mask("swap.new_attrs", state.champions[2].attributes, newDirty,
                "CLIKCHAM.C F0368:69-70 ICON|LOAD|NAME dirty");
    expect_int("swap.old_draw", result.oldLeaderDrawStateCount, 1,
               "CLIKCHAM.C F0368:58 redraws old leader first");
    expect_int("swap.new_draw", result.newLeaderDrawStateCount, 1,
               "CLIKCHAM.C F0368:69-71 redraws non-candidate new leader");
}

static void test_clear_to_none_removes_hand_weight_and_stops(void)
{
    Dm1V1ChampionLeaderStatePc34Compat state;
    Dm1V1ChampionLeaderSetResultPc34Compat result;
    const unsigned int oldDirty =
        DM1_V1_CHAMPION_ATTR_LOAD_PC34_COMPAT |
        DM1_V1_CHAMPION_ATTR_NAME_TITLE_PC34_COMPAT;

    seed_live_party(&state, 1, 5, 0u);

    /* ReDMCSB CLIKCHAM.C F0368 lines 54-64 clears G0411 to CM1_NONE after
     * removing the leader-hand weight, then returns before new-leader redraw. */
    expect_int("clear.return",
               DM1_V1_ChampionLeader_SetPc34Compat(
                   &state, DM1_V1_CHAMPION_LEADER_NONE_PC34_COMPAT, &result),
               1, "CLIKCHAM.C F0368:54-64");
    expect_int("clear.previous", result.previousLeaderIndex, 1,
               "CLIKCHAM.C F0368:54-55 old G0411 snapshot");
    expect_int("clear.new", result.newLeaderIndex,
               DM1_V1_CHAMPION_LEADER_NONE_PC34_COMPAT,
               "CLIKCHAM.C F0368:57/60-64 keeps G0411 clear");
    expect_int("clear.old_detached", result.oldLeaderDetached, 1,
               "CLIKCHAM.C F0368:54-59 old leader detach");
    expect_int("clear.new_attached", result.newLeaderAttached, 0,
               "CLIKCHAM.C F0368:66 not reached");
    expect_int("clear.leader", state.leaderIndex,
               DM1_V1_CHAMPION_LEADER_NONE_PC34_COMPAT,
               "CLIKCHAM.C F0368:57 clears G0411");
    expect_int("clear.old_load", state.champions[1].load, 160,
               "CLIKCHAM.C F0368:56 removes leader-hand weight");
    expect_mask("clear.old_attrs", state.champions[1].attributes, oldDirty,
                "CLIKCHAM.C F0368:55 LOAD|NAME dirty");
    expect_int("clear.old_draw", result.oldLeaderDrawStateCount, 1,
               "CLIKCHAM.C F0368:58 redraws old leader");
    expect_int("clear.new_draw", result.newLeaderDrawStateCount, 0,
               "CLIKCHAM.C F0368:60-64 returns before line 71");
}

static void test_candidate_target_suppresses_new_redraw(void)
{
    Dm1V1ChampionLeaderStatePc34Compat state;
    Dm1V1ChampionLeaderSetResultPc34Compat result;
    const unsigned int oldDirty =
        DM1_V1_CHAMPION_ATTR_LOAD_PC34_COMPAT |
        DM1_V1_CHAMPION_ATTR_NAME_TITLE_PC34_COMPAT;

    seed_live_party(&state, 0, 3, 3u);

    /* ReDMCSB CLIKCHAM.C F0368 lines 66-72 still assigns G0411 and adds the
     * hand weight, but lines 69-71 skip new redraw when target ordinal is G0299. */
    expect_int("candidate.return",
               DM1_V1_ChampionLeader_SetPc34Compat(&state, 2, &result),
               1, "CLIKCHAM.C F0368:66-72");
    expect_int("candidate.previous", result.previousLeaderIndex, 0,
               "CLIKCHAM.C F0368:54-55 old G0411 snapshot");
    expect_int("candidate.new", result.newLeaderIndex, 2,
               "CLIKCHAM.C F0368:66 assigns G0411");
    expect_int("candidate.leader", state.leaderIndex, 2,
               "CLIKCHAM.C F0368:66 assigns G0411");
    expect_int("candidate.new_attached", result.newLeaderAttached, 1,
               "CLIKCHAM.C F0368:66-68 new leader attach");
    expect_int("candidate.old_load", state.champions[0].load, 100,
               "CLIKCHAM.C F0368:56 removes leader-hand weight");
    expect_int("candidate.new_load", state.champions[2].load, 223,
               "CLIKCHAM.C F0368:68 adds leader-hand weight");
    expect_int("candidate.new_direction", state.champions[2].direction,
               state.partyDirection, "CLIKCHAM.C F0368:67 copies party direction");
    expect_mask("candidate.old_attrs", state.champions[0].attributes, oldDirty,
                "CLIKCHAM.C F0368:55 LOAD|NAME dirty");
    expect_int("candidate.new_attrs", (int)state.champions[2].attributes, 0,
               "CLIKCHAM.C F0368:69-71 candidate skips dirty/redraw");
    expect_int("candidate.old_draw", result.oldLeaderDrawStateCount, 1,
               "CLIKCHAM.C F0368:58 redraws old leader");
    expect_int("candidate.new_draw", result.newLeaderDrawStateCount, 0,
               "CLIKCHAM.C F0368:69-71 candidate skips redraw");
}

int main(void)
{
    test_same_leader_is_ignored();
    test_dead_target_is_ignored();
    test_live_target_swaps_leader();
    test_clear_to_none_removes_hand_weight_and_stops();
    test_candidate_target_suppresses_new_redraw();

    printf("dm1_v1_champion_leader_pc34_compat: assertions=%d failures=%d\n",
           g_assertions, g_failures);
    return g_failures ? 1 : 0;
}

#include "dm1_v1_champion_mirror_pc34_compat.h"

/*
 * pass796 - DM1 V1 champion mirror contract test
 * (COMMAND.C:484-488 PC-98/PC C159..C162 champion-name rows;
 * COMMAND.C:1437-1449 F0358 inclusive match; F0380:2158-2162
 * status-box click dispatch). Source-locked against COMMAND.C
 * F0358:1437-1449 + F0380:2158-2162 + DEFS.H C159..C162.
 */

#include <stdio.h>

static int gTests;
static int gPasses;

#define CHECK_ANCHOR(cond, msg, anchor) do { \
    ++gTests; \
    if (cond) { \
        ++gPasses; \
    } else { \
        printf("FAIL: %s [%s]\n", msg, anchor); \
    } \
} while (0)

static Dm1V1ChampionMirrorClickStatePc34Compat base_state(void)
{
    Dm1V1ChampionMirrorClickStatePc34Compat state;
    DM1_V1_ChampionMirror_InitClickStatePc34Compat(&state);
    state.partyChampionCount = 4;
    state.inventoryChampionOrdinal = 1u;
    return state;
}

static void test_c159_name_route_without_candidate_panel(void)
{
    Dm1V1ChampionMirrorClickStatePc34Compat state = base_state();
    Dm1V1ChampionMirrorClickResultPc34Compat result;
    int changed;

    state.partyChampionCount = 1;
    state.inventoryChampionOrdinal = 1u;
    state.leaderIndex = DM1_V1_CHAMPION_MIRROR_NONE_PC34_COMPAT;

    changed = DM1_V1_ChampionMirror_F0380ProcessStatusBoxClickPc34Compat(
        &state,
        DM1_V1_COMMAND_CLICK_STATUS_BOX_0_PC34_COMPAT,
        1,
        5,
        DM1_V1_CHAMPION_MIRROR_MOUSE_LEFT_PC34_COMPAT,
        &result);

    CHECK_ANCHOR(result.dispatchesStatusBoxClick == 1,
                 "C012 status-box click dispatches when G0299 is clear",
                 "COMMAND.C:2158-2162");
    CHECK_ANCHOR(result.attemptedSetLeader == 1,
                 "inventory champion C159 path reaches F0368",
                 "CLIKCHAM.C:24-25");
    CHECK_ANCHOR(changed == 1 && result.leaderChanged == 1,
                 "F0368 changes G0411 for a live non-leader champion",
                 "CLIKCHAM.C:51-72");
    CHECK_ANCHOR(state.leaderIndex == 0 && result.newLeaderIndex == 0,
                 "champion 0 becomes leader after C159 click",
                 "CLIKCHAM.C:66-72");
    CHECK_ANCHOR(result.nestedCommand == DM1_V1_COMMAND_NONE_PC34_COMPAT,
                 "inventory champion branch does not need nested G0455 scan",
                 "CLIKCHAM.C:24-27");
}

static void test_c159_name_route_blocked_while_c040_live(void)
{
    Dm1V1ChampionMirrorClickStatePc34Compat state = base_state();
    Dm1V1ChampionMirrorClickResultPc34Compat result;
    int changed;

    state.partyChampionCount = 1;
    state.inventoryChampionOrdinal = 1u;
    state.candidateChampionOrdinal = 1u;
    state.leaderIndex = DM1_V1_CHAMPION_MIRROR_NONE_PC34_COMPAT;

    changed = DM1_V1_ChampionMirror_F0380ProcessStatusBoxClickPc34Compat(
        &state,
        DM1_V1_COMMAND_CLICK_STATUS_BOX_0_PC34_COMPAT,
        1,
        5,
        DM1_V1_CHAMPION_MIRROR_MOUSE_LEFT_PC34_COMPAT,
        &result);

    CHECK_ANCHOR(changed == 0,
                 "C159 status-box click does not rotate leader while G0299 owns C040",
                 "COMMAND.C:2158-2162; PANEL.C:1654-1656");
    CHECK_ANCHOR(result.ignoredByCandidatePanel == 1,
                 "nonzero G0299 blocks the C012 status-box dispatch",
                 "COMMAND.C:2158-2162");
    CHECK_ANCHOR(result.dispatchesStatusBoxClick == 0,
                 "F0367 is not called while the candidate panel is live",
                 "COMMAND.C:2158-2162; CLIKCHAM.C:24-35");
    CHECK_ANCHOR(result.attemptedSetLeader == 0,
                 "blocked C159 click never reaches F0368",
                 "CLIKCHAM.C:24-35; CLIKCHAM.C:51-72");
    CHECK_ANCHOR(result.nestedCommand == DM1_V1_COMMAND_NONE_PC34_COMPAT,
                 "blocked C159 click does not scan G0455 name rows",
                 "COMMAND.C:484-488; COMMAND.C:1379-1449");
    CHECK_ANCHOR(state.leaderIndex == DM1_V1_CHAMPION_MIRROR_NONE_PC34_COMPAT,
                 "G0411 remains unchanged under C040 candidate-panel guard",
                 "CLIKCHAM.C:51-72");
    CHECK_ANCHOR(state.candidateChampionOrdinal == 1u,
                 "G0299 candidate ordinal is preserved by ignored C159 click",
                 "PANEL.C:1654-1656");
}

static void test_nested_c019_name_route_without_candidate_panel(void)
{
    Dm1V1ChampionMirrorClickStatePc34Compat state = base_state();
    Dm1V1ChampionMirrorClickResultPc34Compat result;
    int changed;

    state.inventoryChampionOrdinal = 2u;
    state.leaderIndex = 0;

    changed = DM1_V1_ChampionMirror_F0380ProcessStatusBoxClickPc34Compat(
        &state,
        DM1_V1_COMMAND_CLICK_STATUS_BOX_0_PC34_COMPAT,
        208,
        5,
        DM1_V1_CHAMPION_MIRROR_MOUSE_LEFT_PC34_COMPAT,
        &result);

    CHECK_ANCHOR(result.dispatchesStatusBoxClick == 1,
                 "outer status-box dispatch reaches F0367 when G0299 is clear",
                 "COMMAND.C:2158-2162");
    CHECK_ANCHOR(result.nestedCommand == DM1_V1_COMMAND_SET_LEADER_3_PC34_COMPAT,
                 "x=208,y=5 maps to C019 champion-3 name row",
                 "COMMAND.C:484-488; COMMAND.C:1437-1449");
    CHECK_ANCHOR(result.targetLeaderIndex == 3,
                 "C019 maps to leader target index 3",
                 "CLIKCHAM.C:27-30");
    CHECK_ANCHOR(changed == 1 && result.leaderChanged == 1,
                 "nested C019 route changes leader when target is alive",
                 "CLIKCHAM.C:51-72");
    CHECK_ANCHOR(state.leaderIndex == 3,
                 "G0411 records champion 3 after C019 route",
                 "CLIKCHAM.C:66-72");
}

static void test_nested_c019_route_blocked_while_c040_live(void)
{
    Dm1V1ChampionMirrorClickStatePc34Compat state = base_state();
    Dm1V1ChampionMirrorClickResultPc34Compat result;
    int changed;

    state.inventoryChampionOrdinal = 2u;
    state.candidateChampionOrdinal = 1u;
    state.leaderIndex = 0;

    changed = DM1_V1_ChampionMirror_F0380ProcessStatusBoxClickPc34Compat(
        &state,
        DM1_V1_COMMAND_CLICK_STATUS_BOX_0_PC34_COMPAT,
        208,
        5,
        DM1_V1_CHAMPION_MIRROR_MOUSE_LEFT_PC34_COMPAT,
        &result);

    CHECK_ANCHOR(changed == 0,
                 "C019 name-row rotation is blocked while C040 is live",
                 "COMMAND.C:2158-2162; PANEL.C:1654-1656");
    CHECK_ANCHOR(result.ignoredByCandidatePanel == 1,
                 "G0299 prevents the outer status-box owner from dispatching",
                 "COMMAND.C:2158-2162");
    CHECK_ANCHOR(result.nestedCommand == DM1_V1_COMMAND_NONE_PC34_COMPAT,
                 "blocked C019 route never enters F0358/G0455",
                 "COMMAND.C:484-488; COMMAND.C:1379-1449");
    CHECK_ANCHOR(result.targetLeaderIndex == DM1_V1_CHAMPION_MIRROR_NONE_PC34_COMPAT,
                 "blocked C019 route has no F0368 target",
                 "CLIKCHAM.C:27-30; CLIKCHAM.C:51-72");
    CHECK_ANCHOR(state.leaderIndex == 0,
                 "existing leader is preserved by the C040 panel guard",
                 "CLIKCHAM.C:51-72");
}

static void test_f0358_edges_and_f0368_dead_target(void)
{
    Dm1V1ChampionMirrorClickStatePc34Compat state = base_state();
    Dm1V1ChampionMirrorClickResultPc34Compat result;
    int changed;

    CHECK_ANCHOR(
        DM1_V1_ChampionMirror_F0358ChampionNamesHandsCommandPc34Compat(
            249, 6, DM1_V1_CHAMPION_MIRROR_MOUSE_LEFT_PC34_COMPAT) ==
            DM1_V1_COMMAND_SET_LEADER_3_PC34_COMPAT,
        "F0358 uses inclusive right/bottom edges for C019",
        "COMMAND.C:484-488; COMMAND.C:1437-1449");
    CHECK_ANCHOR(
        DM1_V1_ChampionMirror_F0358ChampionNamesHandsCommandPc34Compat(
            249, 7, DM1_V1_CHAMPION_MIRROR_MOUSE_LEFT_PC34_COMPAT) ==
            DM1_V1_COMMAND_NONE_PC34_COMPAT,
        "F0358 rejects points below the C019 name row",
        "COMMAND.C:484-488; COMMAND.C:1437-1449");
    CHECK_ANCHOR(
        DM1_V1_ChampionMirror_F0358ChampionNamesHandsCommandPc34Compat(
            249, 6, 0u) == DM1_V1_COMMAND_NONE_PC34_COMPAT,
        "F0358 requires the left mouse button bit",
        "COMMAND.C:484-488; COMMAND.C:1437-1449");

    state.champions[3].currentHealth = 0;
    state.inventoryChampionOrdinal = 2u;
    state.leaderIndex = 0;
    changed = DM1_V1_ChampionMirror_F0380ProcessStatusBoxClickPc34Compat(
        &state,
        DM1_V1_COMMAND_CLICK_STATUS_BOX_0_PC34_COMPAT,
        208,
        5,
        DM1_V1_CHAMPION_MIRROR_MOUSE_LEFT_PC34_COMPAT,
        &result);

    CHECK_ANCHOR(result.nestedCommand == DM1_V1_COMMAND_SET_LEADER_3_PC34_COMPAT,
                 "dead champion target is still selected by C019 before F0368 rejects it",
                 "CLIKCHAM.C:27-30");
    CHECK_ANCHOR(changed == 0 && result.ignoredDeadTarget == 1,
                 "F0368 ignores dead non-none leader targets",
                 "CLIKCHAM.C:51-53");
    CHECK_ANCHOR(state.leaderIndex == 0,
                 "dead target rejection leaves G0411 unchanged",
                 "CLIKCHAM.C:51-53");
}

int main(void)
{
    test_c159_name_route_without_candidate_panel();
    test_c159_name_route_blocked_while_c040_live();
    test_nested_c019_name_route_without_candidate_panel();
    test_nested_c019_route_blocked_while_c040_live();
    test_f0358_edges_and_f0368_dead_target();

    CHECK_ANCHOR(DM1_V1_ChampionMirror_SourceEvidencePc34Compat() != NULL,
                 "source evidence string is available",
                 "COMMAND.C:484-488; CLIKCHAM.C:24-72");

    printf("PASS dm1_v1_champion_mirror_pc34_compat %d/%d assertions\n",
           gPasses, gTests);
    return gPasses == gTests ? 0 : 1;
}

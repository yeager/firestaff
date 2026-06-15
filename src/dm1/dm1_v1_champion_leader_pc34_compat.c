#include "dm1_v1_champion_leader_pc34_compat.h"
#include <string.h>

static int valid_champion_index(int championIndex)
{
    return championIndex >= 0 && championIndex < DM1_V1_CHAMPION_LEADER_COUNT_PC34_COMPAT;
}

void DM1_V1_ChampionLeader_InitPc34Compat(
    Dm1V1ChampionLeaderStatePc34Compat *state)
{
    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    state->leaderIndex = DM1_V1_CHAMPION_LEADER_NONE_PC34_COMPAT;
}

int DM1_V1_ChampionLeader_SetPc34Compat(
    Dm1V1ChampionLeaderStatePc34Compat *state,
    int championIndex,
    Dm1V1ChampionLeaderSetResultPc34Compat *outResult)
{
    Dm1V1ChampionLeaderSetResultPc34Compat result;
    int oldLeader;

    memset(&result, 0, sizeof(result));
    result.previousLeaderIndex = state ? state->leaderIndex : DM1_V1_CHAMPION_LEADER_NONE_PC34_COMPAT;
    result.newLeaderIndex = result.previousLeaderIndex;
    if (!state || championIndex < DM1_V1_CHAMPION_LEADER_NONE_PC34_COMPAT ||
        championIndex >= DM1_V1_CHAMPION_LEADER_COUNT_PC34_COMPAT) {
        if (outResult) {
            *outResult = result;
        }
        return 0;
    }

    /* ReDMCSB CLIKCHAM.C F0368 lines 51-53 returns for the current leader or a dead non-none target. */
    if (championIndex == state->leaderIndex) {
        result.ignoredSameLeader = 1;
        if (outResult) {
            *outResult = result;
        }
        return 0;
    }
    if (valid_champion_index(championIndex) &&
        state->champions[championIndex].currentHealth == 0) {
        result.ignoredDeadTarget = 1;
        if (outResult) {
            *outResult = result;
        }
        return 0;
    }

    /* ReDMCSB CLIKCHAM.C F0368 lines 54-59 marks the old leader load/name dirty, removes the leader-hand weight, clears G0411, and redraws it. */
    oldLeader = state->leaderIndex;
    if (valid_champion_index(oldLeader)) {
        state->champions[oldLeader].attributes |=
            DM1_V1_CHAMPION_ATTR_LOAD_PC34_COMPAT |
            DM1_V1_CHAMPION_ATTR_NAME_TITLE_PC34_COMPAT;
        state->champions[oldLeader].load -= state->leaderHandWeight;
        result.oldLeaderDetached = 1;
        result.oldLeaderDrawStateCount = 1;
    }
    state->leaderIndex = DM1_V1_CHAMPION_LEADER_NONE_PC34_COMPAT;

    /* ReDMCSB CLIKCHAM.C F0368 lines 60-64 allows CM1_CHAMPION_NONE to only clear the old leader. */
    if (championIndex == DM1_V1_CHAMPION_LEADER_NONE_PC34_COMPAT) {
        result.newLeaderIndex = state->leaderIndex;
        if (outResult) {
            *outResult = result;
        }
        return 1;
    }

    /* ReDMCSB CLIKCHAM.C F0368 lines 66-72 assigns G0411, copies party direction, adds leader-hand weight, and redraws non-candidate leaders. */
    state->leaderIndex = championIndex;
    state->champions[championIndex].direction = state->partyDirection;
    state->champions[championIndex].load += state->leaderHandWeight;
    result.newLeaderIndex = championIndex;
    result.newLeaderAttached = 1;
    if ((unsigned int)(championIndex + 1) != state->candidateChampionOrdinal) {
        state->champions[championIndex].attributes |=
            DM1_V1_CHAMPION_ATTR_ICON_PC34_COMPAT |
            DM1_V1_CHAMPION_ATTR_LOAD_PC34_COMPAT |
            DM1_V1_CHAMPION_ATTR_NAME_TITLE_PC34_COMPAT;
        result.newLeaderDrawStateCount = 1;
    }

    if (outResult) {
        *outResult = result;
    }
    return 1;
}

const char *DM1_V1_ChampionLeader_SourceEvidencePc34Compat(void)
{
    return "ReDMCSB CLIKCHAM.C:24-35 F0367 status-box nested G0455 dispatch; "
           "CLIKCHAM.C:51-72 F0368 set-leader state transition; "
           "COMMAND.C:484-488 G0455 C016..C019 champion name rows.";
}

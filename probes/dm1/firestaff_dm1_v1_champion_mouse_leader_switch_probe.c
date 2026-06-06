#include <stdio.h>
#include <string.h>

#include "champion_name_hand_routes_pc34_compat.h"
#include "dm1_v1_champion_leader_pc34_compat.h"
#include "dm1_v1_input_command_queue_pc34_compat.h"

enum {
    DM1_COMMAND_SET_LEADER_CHAMPION_0 = 16
};

static int expect_int(const char *label, int got, int want)
{
    if (got != want) {
        fprintf(stderr, "FAIL %s got=%d want=%d\n", label, got, want);
        return 0;
    }
    return 1;
}

static void seed_leader_state(Dm1V1ChampionLeaderStatePc34Compat *state)
{
    int i;

    DM1_V1_ChampionLeader_InitPc34Compat(state);
    state->leaderIndex = 0;
    state->partyDirection = 2;
    state->leaderHandWeight = 7;
    state->candidateChampionOrdinal = 0u;
    for (i = 0; i < DM1_V1_CHAMPION_LEADER_COUNT_PC34_COMPAT; ++i) {
        state->champions[i].currentHealth = 50 + i;
        state->champions[i].direction = i;
        state->champions[i].load = 100 + (i * 20);
        state->champions[i].attributes = 0u;
    }
}

static int run_mouse_switch_case(Dm1V1ChampionLeaderStatePc34Compat *state,
                                 int championIndex,
                                 int screenX,
                                 int screenY)
{
    struct Dm1V1InputCommandQueuePc34Compat queue;
    struct Dm1V1QueuedCommandPc34Compat queued;
    ChampionStatusDispatchPc34Compat dispatch;
    Dm1V1ChampionLeaderSetResultPc34Compat result;
    int oldLeader = state->leaderIndex;
    int oldLoad = state->champions[oldLeader].load;
    int targetLoad = state->champions[championIndex].load;
    int ok = 1;

    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    ok &= expect_int("parent status mouse enqueues",
        DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(
            &queue,
            (struct Dm1V1InputEventPc34Compat){
                DM1_V1_INPUT_KIND_MOUSE, 0, screenX, screenY, DM1_V1_BUTTON_LEFT }),
        1);
    ok &= expect_int("parent status mouse peek",
        DM1_V1_InputCommandQueue_PeekPc34Compat(&queue, &queued), 1);
    ok &= expect_int("parent status command", queued.command,
        DM1_V1_COMMAND_CLICK_CHAMPION_STATUS_0 + championIndex);

    ok &= expect_int("nested name-hand dispatch",
        champion_name_hand_routes_ResolveStatusBoxClick(
            (unsigned int)championIndex, screenX, screenY, 0u, &dispatch),
        1);
    ok &= expect_int("nested dispatch kind", dispatch.kind,
        CHAMPION_STATUS_DISPATCH_SET_LEADER_PC34_COMPAT);
    ok &= expect_int("nested set-leader command", (int)dispatch.secondaryCommandId,
        DM1_COMMAND_SET_LEADER_CHAMPION_0 + championIndex);

    ok &= expect_int("set leader applies",
        DM1_V1_ChampionLeader_SetPc34Compat(
            state,
            (int)dispatch.secondaryCommandId - DM1_COMMAND_SET_LEADER_CHAMPION_0,
            &result),
        1);
    ok &= expect_int("previous leader recorded", result.previousLeaderIndex, oldLeader);
    ok &= expect_int("new leader recorded", result.newLeaderIndex, championIndex);
    ok &= expect_int("state leader updated", state->leaderIndex, championIndex);
    ok &= expect_int("old leader hand weight removed",
        state->champions[oldLeader].load, oldLoad - state->leaderHandWeight);
    ok &= expect_int("target leader hand weight added",
        state->champions[championIndex].load, targetLoad + state->leaderHandWeight);
    ok &= expect_int("target direction copies party",
        state->champions[championIndex].direction, state->partyDirection);
    ok &= expect_int("old leader draw requested", result.oldLeaderDrawStateCount, 1);
    ok &= expect_int("new leader draw requested", result.newLeaderDrawStateCount, 1);

    printf("mouseLeaderSwitch champ=%d parent=C%03d nested=C%03u zone=C%03u old=%d new=%d "
           "oldLoad=%d targetLoad=%d source=COMMAND.C:384-387,484-488 CLIKCHAM.C:27-30,51-72\n",
           championIndex,
           DM1_V1_COMMAND_CLICK_CHAMPION_STATUS_0 + championIndex,
           dispatch.secondaryCommandId,
           dispatch.secondaryZoneIndex,
           oldLeader,
           state->leaderIndex,
           state->champions[oldLeader].load,
           state->champions[championIndex].load);
    return ok;
}

int main(void)
{
    Dm1V1ChampionLeaderStatePc34Compat state;
    Dm1V1ChampionLeaderSetResultPc34Compat result;
    int ok = 1;

    printf("probe=firestaff_dm1_v1_champion_mouse_leader_switch_probe\n");
    printf("primarySource=ReDMCSB_WIP20210206/Toolchains/Common/Source\n");
    printf("sourceEvidence.inputQueue=%s\n", DM1_V1_InputCommandQueue_SourceEvidencePc34Compat());
    printf("sourceEvidence.nameHand=%s\n", champion_name_hand_routes_GetEvidence());
    printf("sourceEvidence.leader=%s\n", DM1_V1_ChampionLeader_SourceEvidencePc34Compat());

    seed_leader_state(&state);
    ok &= run_mouse_switch_case(&state, 1, 70, 1);
    ok &= run_mouse_switch_case(&state, 2, 139, 1);
    ok &= run_mouse_switch_case(&state, 3, 208, 1);

    ok &= expect_int("same leader is ignored",
        DM1_V1_ChampionLeader_SetPc34Compat(&state, 3, &result), 0);
    ok &= expect_int("same leader flag", result.ignoredSameLeader, 1);
    ok &= expect_int("same leader remains champion3", state.leaderIndex, 3);

    state.champions[2].currentHealth = 0;
    ok &= expect_int("dead target is ignored",
        DM1_V1_ChampionLeader_SetPc34Compat(&state, 2, &result), 0);
    ok &= expect_int("dead target flag", result.ignoredDeadTarget, 1);
    ok &= expect_int("dead target does not change leader", state.leaderIndex, 3);

    printf("result=%s\n", ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}

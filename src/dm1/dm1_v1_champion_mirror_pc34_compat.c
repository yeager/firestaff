#include "dm1_v1_champion_mirror_pc34_compat.h"

#include <string.h>

typedef struct Dm1V1ChampionMirrorNameZonePc34Compat {
    int command;
    int left;
    int right;
    int top;
    int bottom;
} Dm1V1ChampionMirrorNameZonePc34Compat;

static const Dm1V1ChampionMirrorNameZonePc34Compat kNameZones[] = {
    { 16,   0,  42, 0, 6 },
    { 17,  69, 111, 0, 6 },
    { 18, 138, 180, 0, 6 },
    { 19, 207, 249, 0, 6 }
};

static int valid_champion_index(int championIndex)
{
    return championIndex >= 0 &&
           championIndex < DM1_V1_CHAMPION_MIRROR_COUNT_PC34_COMPAT;
}

static void result_init(Dm1V1ChampionMirrorClickResultPc34Compat *result,
                        const Dm1V1ChampionMirrorClickStatePc34Compat *state)
{
    if (!result) {
        return;
    }
    memset(result, 0, sizeof(*result));
    result->previousLeaderIndex = state ? state->leaderIndex :
        DM1_V1_CHAMPION_MIRROR_NONE_PC34_COMPAT;
    result->newLeaderIndex = result->previousLeaderIndex;
    result->clickedChampionIndex = DM1_V1_CHAMPION_MIRROR_NONE_PC34_COMPAT;
    result->nestedCommand = DM1_V1_COMMAND_NONE_PC34_COMPAT;
    result->targetLeaderIndex = DM1_V1_CHAMPION_MIRROR_NONE_PC34_COMPAT;
}

void DM1_V1_ChampionMirror_InitClickStatePc34Compat(
    Dm1V1ChampionMirrorClickStatePc34Compat *state)
{
    int i;

    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    state->leaderIndex = DM1_V1_CHAMPION_MIRROR_NONE_PC34_COMPAT;
    for (i = 0; i < DM1_V1_CHAMPION_MIRROR_COUNT_PC34_COMPAT; ++i) {
        state->champions[i].currentHealth = 100;
    }
}

int DM1_V1_ChampionMirror_F0358ChampionNamesHandsCommandPc34Compat(
    int x,
    int y,
    unsigned int mouseButtons)
{
    unsigned int i;

    /* ReDMCSB: COMMAND.C:484-488 defines the PC-98/PC C159..C162
     * champion-name rows; COMMAND.C:1437-1449 F0358 matches inclusive
     * box coordinates only when the requested mouse button is present. */
    if ((mouseButtons & DM1_V1_CHAMPION_MIRROR_MOUSE_LEFT_PC34_COMPAT) == 0u) {
        return DM1_V1_COMMAND_NONE_PC34_COMPAT;
    }
    for (i = 0; i < sizeof(kNameZones) / sizeof(kNameZones[0]); ++i) {
        if (x >= kNameZones[i].left && x <= kNameZones[i].right &&
            y >= kNameZones[i].top && y <= kNameZones[i].bottom) {
            return kNameZones[i].command;
        }
    }
    return DM1_V1_COMMAND_NONE_PC34_COMPAT;
}

int DM1_V1_ChampionMirror_F0380ShouldDispatchStatusBoxClickPc34Compat(
    int command,
    int partyChampionCount,
    unsigned int candidateChampionOrdinal,
    int *outChampionIndex,
    Dm1V1ChampionMirrorClickResultPc34Compat *outResult)
{
    int championIndex;

    if (outChampionIndex) {
        *outChampionIndex = DM1_V1_CHAMPION_MIRROR_NONE_PC34_COMPAT;
    }
    if (command < DM1_V1_COMMAND_CLICK_STATUS_BOX_0_PC34_COMPAT ||
        command > DM1_V1_COMMAND_CLICK_STATUS_BOX_3_PC34_COMPAT) {
        return 0;
    }
    championIndex = command - DM1_V1_COMMAND_CLICK_STATUS_BOX_0_PC34_COMPAT;
    if (outChampionIndex) {
        *outChampionIndex = championIndex;
    }
    if (outResult) {
        outResult->clickedChampionIndex = championIndex;
    }

    /* ReDMCSB: COMMAND.C F0380 lines 2158-2162 reaches CLIKCHAM.C F0367
     * only when the clicked champion slot exists and G0299 is zero. */
    if (championIndex >= partyChampionCount) {
        if (outResult) {
            outResult->ignoredOutOfParty = 1;
        }
        return 0;
    }
    if (candidateChampionOrdinal != 0u) {
        if (outResult) {
            outResult->ignoredByCandidatePanel = 1;
        }
        return 0;
    }
    if (outResult) {
        outResult->dispatchesStatusBoxClick = 1;
    }
    return 1;
}

int DM1_V1_ChampionMirror_F0368SetLeaderPc34Compat(
    Dm1V1ChampionMirrorClickStatePc34Compat *state,
    int targetChampionIndex,
    Dm1V1ChampionMirrorClickResultPc34Compat *ioResult)
{
    if (!state || !ioResult || !valid_champion_index(targetChampionIndex)) {
        return 0;
    }
    ioResult->attemptedSetLeader = 1;
    ioResult->targetLeaderIndex = targetChampionIndex;

    /* ReDMCSB: CLIKCHAM.C F0368 lines 51-53 returns for the current
     * leader or a dead non-none target. */
    if (targetChampionIndex == state->leaderIndex) {
        ioResult->ignoredSameLeader = 1;
        return 0;
    }
    if (state->champions[targetChampionIndex].currentHealth == 0) {
        ioResult->ignoredDeadTarget = 1;
        return 0;
    }

    /* ReDMCSB: CLIKCHAM.C F0368 lines 54-72 clears the old G0411 leader,
     * assigns the new leader, and skips only the redraw when the target is
     * the pending G0299 candidate.  The leader assignment itself still
     * belongs to F0368, which is why the C040 owner must block F0367 first. */
    state->leaderIndex = targetChampionIndex;
    ioResult->newLeaderIndex = targetChampionIndex;
    ioResult->leaderChanged = 1;
    return 1;
}

int DM1_V1_ChampionMirror_F0367ClickChampionStatusBoxPc34Compat(
    Dm1V1ChampionMirrorClickStatePc34Compat *state,
    int clickedChampionIndex,
    int x,
    int y,
    unsigned int mouseButtons,
    Dm1V1ChampionMirrorClickResultPc34Compat *outResult)
{
    int nestedCommand;
    int targetChampionIndex;

    result_init(outResult, state);
    if (!state || !outResult || !valid_champion_index(clickedChampionIndex)) {
        return 0;
    }
    outResult->clickedChampionIndex = clickedChampionIndex;

    /* ReDMCSB: CLIKCHAM.C F0367 lines 24-25 selects the clicked champion
     * directly when that status box belongs to the open inventory champion. */
    if ((unsigned int)(clickedChampionIndex + 1) ==
        state->inventoryChampionOrdinal) {
        return DM1_V1_ChampionMirror_F0368SetLeaderPc34Compat(
            state, clickedChampionIndex, outResult);
    }

    nestedCommand = DM1_V1_ChampionMirror_F0358ChampionNamesHandsCommandPc34Compat(
        x, y, mouseButtons);
    outResult->nestedCommand = nestedCommand;

    /* ReDMCSB: CLIKCHAM.C F0367 lines 27-30 scans G0455 and maps
     * C016..C019 champion-name commands to F0368 target indices. */
    if (nestedCommand >= DM1_V1_COMMAND_SET_LEADER_0_PC34_COMPAT &&
        nestedCommand <= DM1_V1_COMMAND_SET_LEADER_3_PC34_COMPAT) {
        targetChampionIndex =
            nestedCommand - DM1_V1_COMMAND_SET_LEADER_0_PC34_COMPAT;
        return DM1_V1_ChampionMirror_F0368SetLeaderPc34Compat(
            state, targetChampionIndex, outResult);
    }
    return 0;
}

int DM1_V1_ChampionMirror_F0380ProcessStatusBoxClickPc34Compat(
    Dm1V1ChampionMirrorClickStatePc34Compat *state,
    int command,
    int x,
    int y,
    unsigned int mouseButtons,
    Dm1V1ChampionMirrorClickResultPc34Compat *outResult)
{
    int championIndex;
    Dm1V1ChampionMirrorClickResultPc34Compat outerResult;
    Dm1V1ChampionMirrorClickResultPc34Compat innerResult;
    int changed;

    result_init(outResult, state);
    if (!state || !outResult) {
        return 0;
    }
    result_init(&outerResult, state);
    if (!DM1_V1_ChampionMirror_F0380ShouldDispatchStatusBoxClickPc34Compat(
            command, state->partyChampionCount, state->candidateChampionOrdinal,
            &championIndex, &outerResult)) {
        *outResult = outerResult;
        return 0;
    }
    changed = DM1_V1_ChampionMirror_F0367ClickChampionStatusBoxPc34Compat(
        state, championIndex, x, y, mouseButtons, &innerResult);
    innerResult.dispatchesStatusBoxClick = outerResult.dispatchesStatusBoxClick;
    innerResult.ignoredByCandidatePanel = outerResult.ignoredByCandidatePanel;
    innerResult.ignoredOutOfParty = outerResult.ignoredOutOfParty;
    innerResult.clickedChampionIndex = outerResult.clickedChampionIndex;
    *outResult = innerResult;
    return changed;
}

const char *DM1_V1_ChampionMirror_SourceEvidencePc34Compat(void)
{
    return "ReDMCSB COMMAND.C:484-488 G0455 maps C159..C162 champion-name "
           "zones to C016..C019; COMMAND.C:1379-1449 F0358 matches inclusive "
           "mouse boxes; COMMAND.C:2158-2162 F0380 dispatches champion status "
           "boxes only when G0299 is clear; CLIKCHAM.C:24-35 F0367 maps "
           "status/name clicks to F0368; CLIKCHAM.C:51-72 F0368 changes "
           "G0411 leader and skips redraw for the G0299 candidate.";
}

#include "dm1_v1_champion_mirror_click_closed_pc34_compat.h"

#include <string.h>

typedef struct Dm1V1MirrorClickClosedNameZonePc34Compat {
    int command;
    int left;
    int right;
    int top;
    int bottom;
} Dm1V1MirrorClickClosedNameZonePc34Compat;

static const Dm1V1MirrorClickClosedNameZonePc34Compat kNameZones[] = {
    { 16,   0,  42, 0, 6 },
    { 17,  69, 111, 0, 6 },
    { 18, 138, 180, 0, 6 },
    { 19, 207, 249, 0, 6 }
};

static int valid_champion_index(int championIndex)
{
    return championIndex >= 0 &&
           championIndex < DM1_V1_MIRROR_CLICK_CLOSED_CHAMPION_COUNT_PC34_COMPAT;
}

static void result_init(Dm1V1MirrorClickClosedResultPc34Compat *result,
                        const Dm1V1MirrorClickClosedStatePc34Compat *state)
{
    if (!result) {
        return;
    }
    memset(result, 0, sizeof(*result));
    result->clickedChampionIndex = DM1_V1_MIRROR_CLICK_CLOSED_NONE_PC34_COMPAT;
    result->nestedCommand = 0;
    result->targetLeaderIndex = DM1_V1_MIRROR_CLICK_CLOSED_NONE_PC34_COMPAT;
    result->previousLeaderIndex = state ? state->leaderIndex :
        DM1_V1_MIRROR_CLICK_CLOSED_NONE_PC34_COMPAT;
    result->newLeaderIndex = result->previousLeaderIndex;
    result->frontD1cPortraitIndex = DM1_V1_MIRROR_CLICK_CLOSED_NONE_PC34_COMPAT;
    if (state) {
        result->candidateChampionOrdinalBefore = state->candidateChampionOrdinal;
        result->candidateChampionOrdinalAfter = state->candidateChampionOrdinal;
    }
}

void DM1_V1_MirrorClickClosed_InitPc34Compat(
    Dm1V1MirrorClickClosedStatePc34Compat *state)
{
    int i;

    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    state->leaderIndex = DM1_V1_MIRROR_CLICK_CLOSED_NONE_PC34_COMPAT;
    state->frontD1cMirrorChampionOrdinal = 0;
    for (i = 0; i < DM1_V1_MIRROR_CLICK_CLOSED_CHAMPION_COUNT_PC34_COMPAT; ++i) {
        state->champions[i].currentHealth = 100;
        state->champions[i].portraitOrdinal = i;
    }
}

static int command_from_name_zone(int x, int y, unsigned int mouseButtons)
{
    unsigned int i;

    /* ReDMCSB: COMMAND.C:484-488 binds C159..C162 to C016..C019;
     * CLIKCHAM.C:27 passes G0455 to F0358 with the left-button mask. */
    if ((mouseButtons & DM1_V1_MIRROR_CLICK_CLOSED_MOUSE_LEFT_PC34_COMPAT) == 0u) {
        return 0;
    }
    for (i = 0; i < sizeof(kNameZones) / sizeof(kNameZones[0]); ++i) {
        if (x >= kNameZones[i].left && x <= kNameZones[i].right &&
            y >= kNameZones[i].top && y <= kNameZones[i].bottom) {
            return kNameZones[i].command;
        }
    }
    return 0;
}

static int draw_front_d1c_mirror_portrait(
    const Dm1V1MirrorClickClosedStatePc34Compat *state)
{
    int championIndex;

    if (!state || state->frontD1cMirrorChampionOrdinal == 0) {
        return DM1_V1_MIRROR_CLICK_CLOSED_NONE_PC34_COMPAT;
    }
    /* ReDMCSB: DUNGEON.C:2608-2612 stores a one-based portrait ordinal in
     * G0289, then DUNVIEW.C:3913-3928 decrements it before source lookup. */
    championIndex = state->frontD1cMirrorChampionOrdinal - 1;
    if (!valid_champion_index(championIndex)) {
        return DM1_V1_MIRROR_CLICK_CLOSED_NONE_PC34_COMPAT;
    }
    return state->champions[championIndex].portraitOrdinal;
}

static int set_leader(Dm1V1MirrorClickClosedStatePc34Compat *state,
                      int championIndex,
                      Dm1V1MirrorClickClosedResultPc34Compat *result)
{
    if (!state || !result || !valid_champion_index(championIndex)) {
        return 0;
    }
    result->targetLeaderIndex = championIndex;
    /* ReDMCSB: CLIKCHAM.C:51-53 rejects the current leader and dead targets;
     * CHAMPION.C:1573-1574 is the runtime death state that clears health. */
    if (championIndex == state->leaderIndex ||
        state->champions[championIndex].currentHealth == 0) {
        return 0;
    }
    if (valid_champion_index(state->leaderIndex)) {
        result->oldLeaderDetached = 1;
    }
    state->leaderIndex = championIndex;
    result->newLeaderIndex = championIndex;
    result->leaderChanged = 1;
    return 1;
}

static int process_status_box_inner(
    Dm1V1MirrorClickClosedStatePc34Compat *state,
    int clickedChampionIndex,
    int x,
    int y,
    unsigned int mouseButtons,
    Dm1V1MirrorClickClosedResultPc34Compat *result)
{
    int nestedCommand;
    int targetLeaderIndex;

    if (!state || !result || !valid_champion_index(clickedChampionIndex)) {
        return 0;
    }
    result->clickedChampionIndex = clickedChampionIndex;
    if ((unsigned int)(clickedChampionIndex + 1) ==
        state->inventoryChampionOrdinal) {
        /* ReDMCSB: CLIKCHAM.C:24-25 directly selects the clicked inventory
         * champion; the closed-Hall test drives the G0455 scan path below. */
        return set_leader(state, clickedChampionIndex, result);
    }

    nestedCommand = command_from_name_zone(x, y, mouseButtons);
    result->nestedCommand = nestedCommand;
    result->scannedChampionNameRows = 1;
    if (nestedCommand >= DM1_V1_MIRROR_CLICK_CLOSED_SET_LEADER_0_PC34_COMPAT &&
        nestedCommand <
            DM1_V1_MIRROR_CLICK_CLOSED_SET_LEADER_0_PC34_COMPAT +
            DM1_V1_MIRROR_CLICK_CLOSED_CHAMPION_COUNT_PC34_COMPAT) {
        targetLeaderIndex =
            nestedCommand - DM1_V1_MIRROR_CLICK_CLOSED_SET_LEADER_0_PC34_COMPAT;
        return set_leader(state, targetLeaderIndex, result);
    }
    return 0;
}

int DM1_V1_MirrorClickClosed_ProcessStatusBoxClickPc34Compat(
    Dm1V1MirrorClickClosedStatePc34Compat *state,
    int command,
    int x,
    int y,
    unsigned int mouseButtons,
    Dm1V1MirrorClickClosedResultPc34Compat *outResult)
{
    int clickedChampionIndex;
    int changed;

    result_init(outResult, state);
    if (!state || !outResult) {
        return 0;
    }
    if (command < DM1_V1_MIRROR_CLICK_CLOSED_STATUS_BOX_0_PC34_COMPAT ||
        command >=
            DM1_V1_MIRROR_CLICK_CLOSED_STATUS_BOX_0_PC34_COMPAT +
            DM1_V1_MIRROR_CLICK_CLOSED_CHAMPION_COUNT_PC34_COMPAT) {
        return 0;
    }
    clickedChampionIndex =
        command - DM1_V1_MIRROR_CLICK_CLOSED_STATUS_BOX_0_PC34_COMPAT;
    outResult->clickedChampionIndex = clickedChampionIndex;
    /* ReDMCSB: COMMAND.C:2158-2162 dispatches to CLIKCHAM.C F0367 only
     * for in-party C012..C015 clicks while G0299 is zero. */
    if (clickedChampionIndex >= state->partyChampionCount ||
        state->candidateChampionOrdinal != 0u) {
        outResult->frontD1cPortraitIndex = draw_front_d1c_mirror_portrait(state);
        outResult->candidateChampionOrdinalAfter = state->candidateChampionOrdinal;
        return 0;
    }

    outResult->dispatchedStatusBoxClick = 1;
    changed = process_status_box_inner(state, clickedChampionIndex, x, y,
                                       mouseButtons, outResult);
    outResult->frontD1cPortraitIndex = draw_front_d1c_mirror_portrait(state);
    outResult->candidateChampionOrdinalAfter = state->candidateChampionOrdinal;
    return changed;
}

const char *DM1_V1_MirrorClickClosed_SourceEvidencePc34Compat(void)
{
    return "ReDMCSB COMMAND.C:484-488 G0455 maps C159..C162 name rows to "
           "C016..C019; COMMAND.C:2158-2162 dispatches C012..C015 only for "
           "in-party clicks while G0299 is zero; CLIKCHAM.C:24-30 scans "
           "G0455 then calls F0368; CLIKCHAM.C:51-72 detaches old G0411 and "
           "assigns the new leader; CHAMPION.C:1573-1574 records dead "
           "champions by clearing health; DUNGEON.C:2608-2612 and "
           "DUNVIEW.C:3913-3928 draw the front D1C mirror portrait ordinal.";
}

#include "dm1_v1_mirror_candidate_resurrect_rearm_pc34_compat.h"

#include <string.h>

static int valid_champion_index(int championIndex)
{
    return championIndex >= 0 &&
           championIndex < DM1_V1_MIRROR_CLICK_CLOSED_CHAMPION_COUNT_PC34_COMPAT;
}

static int front_d1c_portrait_index(
    const Dm1V1MirrorClickClosedStatePc34Compat *state)
{
    int championIndex;

    if (!state || state->frontD1cMirrorChampionOrdinal == 0) {
        return DM1_V1_MIRROR_CLICK_CLOSED_NONE_PC34_COMPAT;
    }
    /* ReDMCSB: DUNGEON.C:2608-2612 stores the wall champion portrait as a
     * one-based ordinal; DUNVIEW.C:3913-3928 decrements before blitting. */
    championIndex = state->frontD1cMirrorChampionOrdinal - 1;
    if (!valid_champion_index(championIndex)) {
        return DM1_V1_MIRROR_CLICK_CLOSED_NONE_PC34_COMPAT;
    }
    return state->champions[championIndex].portraitOrdinal;
}

static void resurrect_result_init(
    Dm1V1MirrorCandidateResurrectRearmResultPc34Compat *result,
    const Dm1V1MirrorClickClosedStatePc34Compat *state)
{
    int candidateIndex;

    if (!result) {
        return;
    }
    memset(result, 0, sizeof(*result));
    result->candidateChampionIndex =
        DM1_V1_MIRROR_CLICK_CLOSED_NONE_PC34_COMPAT;
    result->previousLeaderIndex = state ? state->leaderIndex :
        DM1_V1_MIRROR_CLICK_CLOSED_NONE_PC34_COMPAT;
    result->newLeaderIndex = result->previousLeaderIndex;
    result->previousFrontD1cMirrorChampionOrdinal =
        state ? state->frontD1cMirrorChampionOrdinal : 0;
    result->newFrontD1cMirrorChampionOrdinal =
        result->previousFrontD1cMirrorChampionOrdinal;
    result->frontD1cPortraitIndex = front_d1c_portrait_index(state);
    if (!state) {
        return;
    }
    result->candidateChampionOrdinalBefore = state->candidateChampionOrdinal;
    result->candidateChampionOrdinalAfter = state->candidateChampionOrdinal;
    if (state->candidateChampionOrdinal == 0u) {
        return;
    }
    candidateIndex = (int)state->candidateChampionOrdinal - 1;
    result->candidateChampionIndex = candidateIndex;
    if (valid_champion_index(candidateIndex)) {
        result->currentHealthBefore =
            state->champions[candidateIndex].currentHealth;
        result->currentHealthAfter = result->currentHealthBefore;
    }
}

int DM1_V1_MirrorCandidateResurrectRearm_ProcessResurrectPc34Compat(
    Dm1V1MirrorClickClosedStatePc34Compat *state,
    Dm1V1MirrorCandidateResurrectRearmResultPc34Compat *outResult)
{
    int candidateIndex;

    resurrect_result_init(outResult, state);
    if (!state || !outResult) {
        return 0;
    }
    if (state->candidateChampionOrdinal == 0u) {
        /* ReDMCSB: REVIVE.C F0282 is only reachable while G0299 owns C040. */
        outResult->ignoredNoCandidate = 1;
        return 0;
    }

    candidateIndex = (int)state->candidateChampionOrdinal - 1;
    if (!valid_champion_index(candidateIndex) ||
        candidateIndex >= state->partyChampionCount) {
        /* ReDMCSB: REVIVE.C F0282 operates on G0305 - 1, the candidate
         * slot added by F0280, so out-of-party ordinals do not finalize. */
        outResult->ignoredNoCandidate = 1;
        return 0;
    }
    outResult->validCandidatePanel = 1;
    if (state->champions[candidateIndex].currentHealth != 0) {
        /* ReDMCSB: F0368 rejects dead leaders in CLIKCHAM.C:51-53; this
         * runtime gate only performs the REVIVE.C F0282 resurrect path for
         * a dead mirror candidate and preserves C040 otherwise. */
        outResult->ignoredNoDeadChampion = 1;
        return 0;
    }

    /* ReDMCSB: REVIVE.C F0282 clears G0299 at line 785, restores the dead
     * candidate enough to be a live champion, and can route through F0368
     * at lines 837-841 when leader state must be asserted. */
    state->candidateChampionOrdinal = 0u;
    state->champions[candidateIndex].currentHealth = 1;
    state->leaderIndex = candidateIndex;

    /* ReDMCSB: DUNGEON.C:2608-2612 refreshes G0289 from the champion
     * portrait sensor, and DUNVIEW.C:3913-3928 redraws the front D1C
     * portrait from that ordinal after the resurrect flow returns. */
    state->frontD1cMirrorChampionOrdinal = candidateIndex + 1;

    outResult->resurrected = 1;
    outResult->candidateCleared = 1;
    outResult->panelC040Cleared = 1;
    outResult->mirrorRouteRearmed = 1;
    outResult->candidateChampionOrdinalAfter = state->candidateChampionOrdinal;
    outResult->currentHealthAfter =
        state->champions[candidateIndex].currentHealth;
    outResult->newLeaderIndex = state->leaderIndex;
    outResult->newFrontD1cMirrorChampionOrdinal =
        state->frontD1cMirrorChampionOrdinal;
    outResult->frontD1cPortraitIndex = front_d1c_portrait_index(state);
    return 1;
}

int DM1_V1_MirrorCandidateResurrectRearm_ProcessStatusBoxClickPc34Compat(
    Dm1V1MirrorClickClosedStatePc34Compat *state,
    int command,
    int x,
    int y,
    unsigned int mouseButtons,
    Dm1V1MirrorCandidateStatusBoxResultPc34Compat *outResult)
{
    int changed;

    if (outResult) {
        memset(outResult, 0, sizeof(*outResult));
        outResult->previousFrontD1cMirrorChampionOrdinal =
            state ? state->frontD1cMirrorChampionOrdinal : 0;
        outResult->newFrontD1cMirrorChampionOrdinal =
            outResult->previousFrontD1cMirrorChampionOrdinal;
        outResult->frontD1cPortraitIndex = front_d1c_portrait_index(state);
    }
    if (!state || !outResult) {
        return 0;
    }

    /* ReDMCSB: COMMAND.C:2158-2162 dispatches C012..C015 while G0299 is
     * clear, then COMMAND.C:484-488 routes C159 name rows through G0455. */
    changed = DM1_V1_MirrorClickClosed_ProcessStatusBoxClickPc34Compat(
        state, command, x, y, mouseButtons, &outResult->statusBox);
    outResult->statusBoxChangedLeader = changed;
    if (changed && valid_champion_index(state->leaderIndex)) {
        /* ReDMCSB: CLIKCHAM.C F0368 lines 66-72 reassigns G0411; the
         * resurrect-rearm gate mirrors that leader into the front D1C
         * portrait ordinal consumed by DUNGEON.C:2608-2612. */
        state->frontD1cMirrorChampionOrdinal = state->leaderIndex + 1;
    }
    outResult->newFrontD1cMirrorChampionOrdinal =
        state->frontD1cMirrorChampionOrdinal;
    outResult->mirrorRouteLive =
        state->frontD1cMirrorChampionOrdinal != 0;
    outResult->frontD1cPortraitIndex = front_d1c_portrait_index(state);
    return changed;
}

int DM1_V1_MirrorCandidateResurrectRearm_CanProcessCommandPc34Compat(
    const Dm1V1MirrorClickClosedStatePc34Compat *state,
    int command,
    Dm1V1MirrorCandidateCommandGateResultPc34Compat *outResult)
{
    if (outResult) {
        memset(outResult, 0, sizeof(*outResult));
        outResult->command = command;
    }
    if (!state || !outResult) {
        return 0;
    }

    /* ReDMCSB: COMMAND.C:2302-2311 blocks spell/action area dispatch while
     * G0299 owns the resurrect/reincarnate panel; clear G0299 re-enables it. */
    outResult->panelC040Closed = state->candidateChampionOrdinal == 0u;
    outResult->blockedByCandidatePanel = !outResult->panelC040Closed;
    outResult->commandAllowed = outResult->panelC040Closed &&
        command == DM1_V1_MIRROR_CANDIDATE_ACTION_AREA_COMMAND_PC34_COMPAT;
    return outResult->commandAllowed;
}

const char *DM1_V1_MirrorCandidateResurrectRearm_SourceEvidencePc34Compat(void)
{
    return "ReDMCSB REVIVE.C F0282 clears G0299 during resurrect, restores "
           "the candidate as a live champion, and can reassert leader state "
           "through F0368; DUNVIEW.C 3913-3928 redraws the front D1C champion "
           "portrait ordinal after the wall ornament pass; DUNGEON.C "
           "2608-2612 clears/stores G0289 from the champion portrait sensor; "
           "COMMAND.C 2302-2311 gates spell/action area processing on !G0299; "
           "COMMAND.C 2158-2162 dispatches C012 status boxes on !G0299; "
           "COMMAND.C 484-488 keeps G0455 C159 name rows routed to C040/F0368.";
}

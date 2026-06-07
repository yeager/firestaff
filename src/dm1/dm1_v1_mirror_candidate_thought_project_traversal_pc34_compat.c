#include "dm1/dm1_v1_mirror_candidate_thought_project_traversal_pc34_compat.h"

#include <string.h>

/* Source-lock anchors for this contract_only=1 C157/C158 traversal gate:
 * COMMAND.C:F0361_COMMAND_ProcessKeyPress:1709-1806 maps keyboard rows into
 * G0432 before the queue processor consumes them.
 * COMMAND.C:F0380_COMMAND_ProcessQueue_CPSC:2045-2127 dequeues one command and
 * preserves its X/Y payload before later command-specific routing.
 * COMMAND.C:F0378_COMMAND_ProcessType81_ClickInPanel:1956-1994 routes the
 * live C040 panel through C160/C161/C162; this fixture uses that path as the
 * close/clear analogue for the synthetic C157/C158 thought-project overlay.
 * REVIVE.C:F0280_CHAMPION_AddCandidateChampionToParty:124-276 publishes
 * G0299 and appends the candidate champion into the party roster.
 * REVIVE.C:F0282_CHAMPION_ProcessCommands160To162_ClickInResurrectReincarnatePanel:704-806
 * owns panel close/commit cleanup and clears G0299 on cancel/finish.
 */

enum {
    kCandidate0Ordinal = 3,
    kCandidate1Ordinal = 4,
    kInitialIdentityOrdinal = 3
};

static const Dm1V1MirrorCandidateThoughtProjectEvidencePc34Compat
    s_evidence = {
        1,
        "COMMAND.C:F0361_COMMAND_ProcessKeyPress:1709-1806",
        "COMMAND.C:F0380_COMMAND_ProcessQueue_CPSC:2045-2127",
        "COMMAND.C:F0378_COMMAND_ProcessType81_ClickInPanel:1956-1994",
        "REVIVE.C:F0280_CHAMPION_AddCandidateChampionToParty:124-276",
        "REVIVE.C:F0282_CHAMPION_ProcessCommands160To162_ClickInResurrectReincarnatePanel:704-806",
        "COMMAND.C:F0380_COMMAND_ProcessQueue_CPSC:2158-2182",
        "contract_only=1 synthetic C157/C158 thought-project traversal; "
        "ReDMCSB PC34 has no native C157/C158 definitions, so this gate "
        "asserts Firestaff's mirror-candidate overlay must not swap the live "
        "candidate or leave pending state across the C040 close/reopen path"
    };

static void copy_text(char *dst, const char *src, int dstBytes)
{
    if (!dst || dstBytes <= 0) {
        return;
    }
    if (!src) {
        src = "";
    }
    strncpy(dst, src, (size_t)dstBytes - 1u);
    dst[dstBytes - 1] = '\0';
}

static int valid_candidate_index(int candidateIndex)
{
    return candidateIndex >= 0 &&
           candidateIndex <
               DM1_V1_MIRROR_CANDIDATE_THOUGHT_PROJECT_CANDIDATE_COUNT_PC34_COMPAT;
}

static void sync_live_candidate(
    Dm1V1MirrorCandidateThoughtProjectStatePc34Compat *state)
{
    if (!state || !valid_candidate_index(state->activeCandidateIndex)) {
        return;
    }
    state->g0299CandidateChampionOrdinal =
        state->candidates[state->activeCandidateIndex].championOrdinal;
    state->g0420CandidateIdentityOrdinal =
        state->g0299CandidateChampionOrdinal;
}

static void clear_project_state(
    Dm1V1MirrorCandidateThoughtProjectStatePc34Compat *state)
{
    if (!state) {
        return;
    }
    state->pendingThoughtActive = 0;
    state->pendingThoughtCandidateIndex =
        DM1_V1_MIRROR_CANDIDATE_THOUGHT_PROJECT_NONE_PC34_COMPAT;
    state->projectedSlotIndex =
        DM1_V1_MIRROR_CANDIDATE_THOUGHT_PROJECT_NONE_PC34_COMPAT;
    state->projectedSlotCount = 0;
    state->pendingThoughtText[0] = '\0';
}

static void snapshot_begin(
    const Dm1V1MirrorCandidateThoughtProjectStatePc34Compat *state,
    Dm1V1MirrorCandidateThoughtProjectCommandPc34Compat command,
    Dm1V1MirrorCandidateThoughtProjectResultPc34Compat *result)
{
    int activeIndex = 0;

    memset(result, 0, sizeof(*result));
    result->evidence = &s_evidence;
    result->command = command;
    if (!state) {
        return;
    }
    activeIndex = valid_candidate_index(state->activeCandidateIndex)
        ? state->activeCandidateIndex
        : 0;
    result->panelOpenBefore = state->panelOpen;
    result->panelOpenAfter = state->panelOpen;
    result->activeCandidateIndexBefore = state->activeCandidateIndex;
    result->activeCandidateIndexAfter = state->activeCandidateIndex;
    result->g0299Before = state->g0299CandidateChampionOrdinal;
    result->g0299After = state->g0299CandidateChampionOrdinal;
    result->g0420Before = state->g0420CandidateIdentityOrdinal;
    result->g0420After = state->g0420CandidateIdentityOrdinal;
    result->pendingActiveBefore = state->pendingThoughtActive;
    result->pendingActiveAfter = state->pendingThoughtActive;
    result->pendingCandidateBefore = state->pendingThoughtCandidateIndex;
    result->pendingCandidateAfter = state->pendingThoughtCandidateIndex;
    result->projectedSlotIndexBefore = state->projectedSlotIndex;
    result->projectedSlotIndexAfter = state->projectedSlotIndex;
    result->projectedSlotCountBefore = state->projectedSlotCount;
    result->projectedSlotCountAfter = state->projectedSlotCount;
    result->projectDispatchCountBefore = state->projectDispatchCount;
    result->projectDispatchCountAfter = state->projectDispatchCount;
    result->commitDispatchCountBefore = state->commitDispatchCount;
    result->commitDispatchCountAfter = state->commitDispatchCount;
    result->closeDispatchCountBefore = state->closeDispatchCount;
    result->closeDispatchCountAfter = state->closeDispatchCount;
    result->reopenDispatchCountBefore = state->reopenDispatchCount;
    result->reopenDispatchCountAfter = state->reopenDispatchCount;
    copy_text(result->statusBoxBefore,
              state->statusBoxText,
              sizeof(result->statusBoxBefore));
    copy_text(result->statusBoxAfter,
              state->statusBoxText,
              sizeof(result->statusBoxAfter));
    copy_text(result->pendingTextBefore,
              state->pendingThoughtText,
              sizeof(result->pendingTextBefore));
    copy_text(result->pendingTextAfter,
              state->pendingThoughtText,
              sizeof(result->pendingTextAfter));
    copy_text(result->committedBefore,
              state->candidates[activeIndex].committedThought,
              sizeof(result->committedBefore));
    copy_text(result->committedAfter,
              state->candidates[activeIndex].committedThought,
              sizeof(result->committedAfter));
}

static void snapshot_finish(
    const Dm1V1MirrorCandidateThoughtProjectStatePc34Compat *state,
    Dm1V1MirrorCandidateThoughtProjectResultPc34Compat *result)
{
    int activeIndex = 0;

    if (!state || !result) {
        return;
    }
    activeIndex = valid_candidate_index(state->activeCandidateIndex)
        ? state->activeCandidateIndex
        : 0;
    result->panelOpenAfter = state->panelOpen;
    result->activeCandidateIndexAfter = state->activeCandidateIndex;
    result->g0299After = state->g0299CandidateChampionOrdinal;
    result->g0420After = state->g0420CandidateIdentityOrdinal;
    result->pendingActiveAfter = state->pendingThoughtActive;
    result->pendingCandidateAfter = state->pendingThoughtCandidateIndex;
    result->projectedSlotIndexAfter = state->projectedSlotIndex;
    result->projectedSlotCountAfter = state->projectedSlotCount;
    result->projectDispatchCountAfter = state->projectDispatchCount;
    result->commitDispatchCountAfter = state->commitDispatchCount;
    result->closeDispatchCountAfter = state->closeDispatchCount;
    result->reopenDispatchCountAfter = state->reopenDispatchCount;
    copy_text(result->statusBoxAfter,
              state->statusBoxText,
              sizeof(result->statusBoxAfter));
    copy_text(result->pendingTextAfter,
              state->pendingThoughtText,
              sizeof(result->pendingTextAfter));
    copy_text(result->committedAfter,
              state->candidates[activeIndex].committedThought,
              sizeof(result->committedAfter));

    result->liveCandidatePreserved =
        result->activeCandidateIndexBefore == result->activeCandidateIndexAfter &&
        result->g0299Before == result->g0299After &&
        result->g0420Before == result->g0420After;
    result->thoughtProjected =
        result->projectDispatchCountAfter == result->projectDispatchCountBefore + 1 &&
        result->pendingActiveAfter == 1 &&
        result->pendingCandidateAfter == result->activeCandidateIndexAfter &&
        strcmp(result->statusBoxAfter, result->pendingTextAfter) == 0 &&
        result->statusBoxAfter[0] != '\0';
    result->thoughtCommitted =
        result->commitDispatchCountAfter == result->commitDispatchCountBefore + 1 &&
        strcmp(result->committedAfter, result->statusBoxBefore) == 0 &&
        result->committedAfter[0] != '\0';
    result->emptyThoughtNoOp =
        result->command ==
            DM1_V1_MIRROR_CANDIDATE_THOUGHT_PROJECT_C157_PROJECT_PC34_COMPAT &&
        result->projectDispatchCountAfter == result->projectDispatchCountBefore &&
        result->pendingActiveBefore == result->pendingActiveAfter &&
        strcmp(result->statusBoxBefore, result->statusBoxAfter) == 0;
    result->firstSlotOnly =
        result->projectedSlotIndexAfter == 0 &&
        result->projectedSlotCountAfter == 1;
    result->projectStateCleared =
        result->pendingActiveAfter == 0 &&
        result->pendingCandidateAfter ==
            DM1_V1_MIRROR_CANDIDATE_THOUGHT_PROJECT_NONE_PC34_COMPAT &&
        result->projectedSlotIndexAfter ==
            DM1_V1_MIRROR_CANDIDATE_THOUGHT_PROJECT_NONE_PC34_COMPAT &&
        result->pendingTextAfter[0] == '\0';
    result->reopenReset =
        result->command ==
            DM1_V1_MIRROR_CANDIDATE_THOUGHT_PROJECT_C162_CLOSE_PC34_COMPAT &&
        result->panelOpenBefore == 0 &&
        result->panelOpenAfter == 1 &&
        result->projectStateCleared;
}

void DM1_V1_MirrorCandidateThoughtProjectTraversal_InitPc34Compat(
    Dm1V1MirrorCandidateThoughtProjectStatePc34Compat *state)
{
    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    state->contractOnly = 1;
    state->panelOpen = 1;
    state->activeCandidateIndex = 0;
    state->pendingThoughtCandidateIndex =
        DM1_V1_MIRROR_CANDIDATE_THOUGHT_PROJECT_NONE_PC34_COMPAT;
    state->projectedSlotIndex =
        DM1_V1_MIRROR_CANDIDATE_THOUGHT_PROJECT_NONE_PC34_COMPAT;
    state->candidates[0].championOrdinal = kCandidate0Ordinal;
    state->candidates[1].championOrdinal = kCandidate1Ordinal;
    copy_text(state->candidates[0].thoughtSlots[0],
              "FUL BRO NETA",
              sizeof(state->candidates[0].thoughtSlots[0]));
    copy_text(state->candidates[0].thoughtSlots[1],
              "second slot ignored",
              sizeof(state->candidates[0].thoughtSlots[1]));
    copy_text(state->candidates[1].thoughtSlots[0],
              "DES EW SAR",
              sizeof(state->candidates[1].thoughtSlots[0]));
    copy_text(state->candidates[1].thoughtSlots[1],
              "candidate one second slot ignored",
              sizeof(state->candidates[1].thoughtSlots[1]));
    sync_live_candidate(state);
    state->g0420CandidateIdentityOrdinal = kInitialIdentityOrdinal;
}

int DM1_V1_MirrorCandidateThoughtProjectTraversal_ProjectPc34Compat(
    Dm1V1MirrorCandidateThoughtProjectStatePc34Compat *state,
    Dm1V1MirrorCandidateThoughtProjectResultPc34Compat *outResult)
{
    const char *thought;

    if (!state || !outResult || !state->contractOnly ||
        !valid_candidate_index(state->activeCandidateIndex)) {
        return 0;
    }
    snapshot_begin(
        state,
        DM1_V1_MIRROR_CANDIDATE_THOUGHT_PROJECT_C157_PROJECT_PC34_COMPAT,
        outResult);

    thought = state->candidates[state->activeCandidateIndex].thoughtSlots[0];
    if (state->panelOpen && thought[0] != '\0') {
        copy_text(state->statusBoxText, thought, sizeof(state->statusBoxText));
        copy_text(state->pendingThoughtText,
                  thought,
                  sizeof(state->pendingThoughtText));
        state->pendingThoughtActive = 1;
        state->pendingThoughtCandidateIndex = state->activeCandidateIndex;
        state->projectedSlotIndex = 0;
        state->projectedSlotCount = 1;
        ++state->projectDispatchCount;
    }

    snapshot_finish(state, outResult);
    return outResult->thoughtProjected || outResult->emptyThoughtNoOp;
}

int DM1_V1_MirrorCandidateThoughtProjectTraversal_CommitPc34Compat(
    Dm1V1MirrorCandidateThoughtProjectStatePc34Compat *state,
    Dm1V1MirrorCandidateThoughtProjectResultPc34Compat *outResult)
{
    if (!state || !outResult || !state->contractOnly ||
        !valid_candidate_index(state->activeCandidateIndex)) {
        return 0;
    }
    snapshot_begin(
        state,
        DM1_V1_MIRROR_CANDIDATE_THOUGHT_PROJECT_C158_COMMIT_PC34_COMPAT,
        outResult);

    if (state->panelOpen &&
        state->pendingThoughtActive &&
        state->pendingThoughtCandidateIndex == state->activeCandidateIndex &&
        state->pendingThoughtText[0] != '\0') {
        copy_text(state->candidates[state->activeCandidateIndex].committedThought,
                  state->pendingThoughtText,
                  sizeof(state->candidates[state->activeCandidateIndex]
                             .committedThought));
        ++state->commitDispatchCount;
    }

    snapshot_finish(state, outResult);
    return outResult->thoughtCommitted;
}

int DM1_V1_MirrorCandidateThoughtProjectTraversal_SwapCandidatePc34Compat(
    Dm1V1MirrorCandidateThoughtProjectStatePc34Compat *state,
    int candidateIndex)
{
    if (!state || !state->contractOnly || !valid_candidate_index(candidateIndex)) {
        return 0;
    }
    clear_project_state(state);
    state->activeCandidateIndex = candidateIndex;
    sync_live_candidate(state);
    return 1;
}

int DM1_V1_MirrorCandidateThoughtProjectTraversal_ClosePc34Compat(
    Dm1V1MirrorCandidateThoughtProjectStatePc34Compat *state,
    Dm1V1MirrorCandidateThoughtProjectResultPc34Compat *outResult)
{
    if (!state || !outResult || !state->contractOnly) {
        return 0;
    }
    snapshot_begin(
        state,
        DM1_V1_MIRROR_CANDIDATE_THOUGHT_PROJECT_C162_CLOSE_PC34_COMPAT,
        outResult);

    state->panelOpen = 0;
    clear_project_state(state);
    state->statusBoxText[0] = '\0';
    ++state->closeDispatchCount;

    snapshot_finish(state, outResult);
    return outResult->projectStateCleared;
}

int DM1_V1_MirrorCandidateThoughtProjectTraversal_ReopenPc34Compat(
    Dm1V1MirrorCandidateThoughtProjectStatePc34Compat *state,
    Dm1V1MirrorCandidateThoughtProjectResultPc34Compat *outResult)
{
    if (!state || !outResult || !state->contractOnly) {
        return 0;
    }
    snapshot_begin(
        state,
        DM1_V1_MIRROR_CANDIDATE_THOUGHT_PROJECT_C162_CLOSE_PC34_COMPAT,
        outResult);

    state->panelOpen = 1;
    clear_project_state(state);
    state->statusBoxText[0] = '\0';
    ++state->reopenDispatchCount;

    snapshot_finish(state, outResult);
    return outResult->reopenReset;
}

const Dm1V1MirrorCandidateThoughtProjectEvidencePc34Compat *
DM1_V1_MirrorCandidateThoughtProjectTraversal_EvidencePc34Compat(void)
{
    return &s_evidence;
}

#include "dm1_v1_mirror_candidate_keyboard_rotation_combo_pc34_compat.h"

#include <string.h>

/* Source-lock anchors for this contract_only=1 combo-input regression:
 * COMMAND.C F0361_COMMAND_ProcessKeyPress:1709-1806 inserts matched
 * keyboard commands into the command queue before F0380 consumes them.
 * COMMAND.C F0380_COMMAND_ProcessQueue_CPSC:2164-2170 routes C125-C129
 * champion-icon rotation/release without a !G0299 guard while live.
 * COMMAND.C F0380_COMMAND_ProcessQueue_CPSC:2158-2182 gates C012-C015
 * status-box clicks and C007-C011 inventory toggles on !G0299.
 * COMMAND.C F0380_COMMAND_ProcessQueue_CPSC:2302-2311 gates C100 spell
 * and C111 action-area dispatch on !G0299.
 * COMMAND.C F0380_COMMAND_ProcessQueue_CPSC:2336-2359 gates C145 rest on
 * !G0299 before rest state, rest screen, and input discard.
 * COMMAND.C F0380_COMMAND_ProcessQueue_CPSC:2366-2370 gates C140 save on
 * party count and !G0299 before F0433 save dispatch.
 */

static const Dm1V1MirrorCandidateKeyboardRotationComboEvidencePc34Compat
    s_evidence = {
        1,
        "COMMAND.C F0361_COMMAND_ProcessKeyPress:1709-1806",
        "COMMAND.C F0380_COMMAND_ProcessQueue_CPSC:2164-2170 C125-C129",
        "COMMAND.C F0380_COMMAND_ProcessQueue_CPSC:2158-2182 !G0299",
        "COMMAND.C F0380_COMMAND_ProcessQueue_CPSC:2302-2311 !G0299",
        "COMMAND.C F0380_COMMAND_ProcessQueue_CPSC:2336-2359 !G0299",
        "COMMAND.C F0380_COMMAND_ProcessQueue_CPSC:2366-2370 !G0299",
        "Only the C125-C129 champion-icon rotation/release dispatch is "
        "modeled as allowed while G0299 is live in this combo contract.",
        "contract_only=1 deterministic combo path; no fixture claims real "
        "runtime keycode, asset, save-file, or spell-engine parity"
    };

static void capture_before(
    const Dm1V1MirrorCandidateKeyboardRotationComboStatePc34Compat *state,
    Dm1V1MirrorCandidateKeyboardRotationComboResultPc34Compat *result)
{
    result->evidence = &s_evidence;
    result->candidateBefore = state->g0420CandidateIdentityOrdinal;
    result->candidateAfterRotation = state->g0420CandidateIdentityOrdinal;
    result->candidateAfterCombo = state->g0420CandidateIdentityOrdinal;
    result->rosterIndexBefore = state->rosterIndex;
    result->rosterIndexAfter = state->rosterIndex;
    result->statusBoxDispatchCountBefore = state->statusBoxDispatchCount;
    result->statusBoxDispatchCountAfter = state->statusBoxDispatchCount;
    result->spellRuneDispatchCountBefore = state->spellRuneDispatchCount;
    result->spellRuneDispatchCountAfter = state->spellRuneDispatchCount;
    result->saveDispatchCountBefore = state->saveDispatchCount;
    result->saveDispatchCountAfter = state->saveDispatchCount;
    result->restDispatchCountBefore = state->restDispatchCount;
    result->restDispatchCountAfter = state->restDispatchCount;
    result->lastSaveTickBefore = state->lastSaveTick;
    result->lastSaveTickAfter = state->lastSaveTick;
    result->runeCountBefore = state->runeCount;
    result->runeCountAfter = state->runeCount;
    memcpy(result->runeBufferBefore,
           state->runeBuffer,
           sizeof(result->runeBufferBefore));
    memcpy(result->runeBufferAfter,
           state->runeBuffer,
           sizeof(result->runeBufferAfter));
}

static void capture_after(
    const Dm1V1MirrorCandidateKeyboardRotationComboStatePc34Compat *state,
    Dm1V1MirrorCandidateKeyboardRotationComboResultPc34Compat *result)
{
    result->candidateAfterCombo = state->g0420CandidateIdentityOrdinal;
    result->rosterIndexAfter = state->rosterIndex;
    result->statusBoxDispatchCountAfter = state->statusBoxDispatchCount;
    result->spellRuneDispatchCountAfter = state->spellRuneDispatchCount;
    result->saveDispatchCountAfter = state->saveDispatchCount;
    result->restDispatchCountAfter = state->restDispatchCount;
    result->lastSaveTickAfter = state->lastSaveTick;
    result->runeCountAfter = state->runeCount;
    memcpy(result->runeBufferAfter,
           state->runeBuffer,
           sizeof(result->runeBufferAfter));
    result->rotationProcessedFirst =
        result->candidateAfterRotation != result->candidateBefore &&
        result->candidateAfterRotation == result->candidateAfterCombo;
    result->rotationAllowedWhileCandidateLive =
        state->g0299CandidateChampionOrdinal != 0u &&
        result->rotationProcessedFirst;
    result->onlyRotationSucceeded =
        result->rotationAllowedWhileCandidateLive &&
        result->nonRotationRejectedByCandidate &&
        result->statusBoxDispatchCountAfter ==
            result->statusBoxDispatchCountBefore &&
        result->spellRuneDispatchCountAfter ==
            result->spellRuneDispatchCountBefore &&
        result->saveDispatchCountAfter == result->saveDispatchCountBefore &&
        result->restDispatchCountAfter == result->restDispatchCountBefore &&
        result->lastSaveTickAfter == result->lastSaveTickBefore &&
        result->runeCountAfter == result->runeCountBefore &&
        memcmp(result->runeBufferAfter,
               result->runeBufferBefore,
               sizeof(result->runeBufferAfter)) == 0;
}

static void rotate_candidate(
    Dm1V1MirrorCandidateKeyboardRotationComboStatePc34Compat *state,
    Dm1V1MirrorCandidateKeyboardRotationComboRotationPc34Compat rotation)
{
    int nextIndex = state->rosterIndex + (int)rotation;

    if (nextIndex < 0) {
        nextIndex =
            DM1_V1_MIRROR_CANDIDATE_KEYBOARD_ROTATION_COMBO_ROSTER_COUNT_PC34_COMPAT -
            1;
    } else if (nextIndex >=
               DM1_V1_MIRROR_CANDIDATE_KEYBOARD_ROTATION_COMBO_ROSTER_COUNT_PC34_COMPAT) {
        nextIndex = 0;
    }
    state->rosterIndex = nextIndex;
    state->g0420CandidateIdentityOrdinal = state->roster[nextIndex];
    state->g0299CandidateChampionOrdinal = state->g0420CandidateIdentityOrdinal;
    ++state->rotationDispatchCount;
}

static int candidate_blocks_non_rotation(
    const Dm1V1MirrorCandidateKeyboardRotationComboStatePc34Compat *state,
    Dm1V1MirrorCandidateKeyboardRotationComboInputPc34Compat input)
{
    if (state->g0299CandidateChampionOrdinal == 0u) {
        return 0;
    }
    return input ==
               DM1_V1_MIRROR_CANDIDATE_KEYBOARD_ROTATION_COMBO_STATUS_BOX_PC34_COMPAT ||
           input ==
               DM1_V1_MIRROR_CANDIDATE_KEYBOARD_ROTATION_COMBO_SPELL_RUNE_PC34_COMPAT ||
           input ==
               DM1_V1_MIRROR_CANDIDATE_KEYBOARD_ROTATION_COMBO_SAVE_PC34_COMPAT ||
           input ==
               DM1_V1_MIRROR_CANDIDATE_KEYBOARD_ROTATION_COMBO_REST_PC34_COMPAT;
}

static void dispatch_non_rotation(
    Dm1V1MirrorCandidateKeyboardRotationComboStatePc34Compat *state,
    Dm1V1MirrorCandidateKeyboardRotationComboInputPc34Compat input)
{
    switch (input) {
    case DM1_V1_MIRROR_CANDIDATE_KEYBOARD_ROTATION_COMBO_STATUS_BOX_PC34_COMPAT:
        ++state->statusBoxDispatchCount;
        break;
    case DM1_V1_MIRROR_CANDIDATE_KEYBOARD_ROTATION_COMBO_SPELL_RUNE_PC34_COMPAT:
        if (state->runeCount <
            DM1_V1_MIRROR_CANDIDATE_KEYBOARD_ROTATION_COMBO_RUNE_BUFFER_SIZE_PC34_COMPAT) {
            state->runeBuffer[state->runeCount++] = 'A';
        }
        ++state->spellRuneDispatchCount;
        break;
    case DM1_V1_MIRROR_CANDIDATE_KEYBOARD_ROTATION_COMBO_SAVE_PC34_COMPAT:
        ++state->lastSaveTick;
        ++state->saveDispatchCount;
        break;
    case DM1_V1_MIRROR_CANDIDATE_KEYBOARD_ROTATION_COMBO_REST_PC34_COMPAT:
        ++state->restDispatchCount;
        break;
    default:
        break;
    }
}

void DM1_V1_MirrorCandidateKeyboardRotationCombo_InitPc34Compat(
    Dm1V1MirrorCandidateKeyboardRotationComboStatePc34Compat *state)
{
    int i;

    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    state->contractOnly = 1;
    state->lastSaveTick = 42;
    for (i = 0;
         i < DM1_V1_MIRROR_CANDIDATE_KEYBOARD_ROTATION_COMBO_ROSTER_COUNT_PC34_COMPAT;
         ++i) {
        state->roster[i] = (unsigned int)(i + 1);
    }
    state->rosterIndex = 0;
    state->g0420CandidateIdentityOrdinal = state->roster[0];
    state->g0299CandidateChampionOrdinal = state->g0420CandidateIdentityOrdinal;
}

int DM1_V1_MirrorCandidateKeyboardRotationCombo_ApplyPc34Compat(
    Dm1V1MirrorCandidateKeyboardRotationComboStatePc34Compat *state,
    Dm1V1MirrorCandidateKeyboardRotationComboRotationPc34Compat rotation,
    Dm1V1MirrorCandidateKeyboardRotationComboInputPc34Compat nonRotation,
    Dm1V1MirrorCandidateKeyboardRotationComboResultPc34Compat *outResult)
{
    if (!state || !outResult || !state->contractOnly) {
        return 0;
    }
    memset(outResult, 0, sizeof(*outResult));
    capture_before(state, outResult);

    rotate_candidate(state, rotation);
    outResult->candidateAfterRotation = state->g0420CandidateIdentityOrdinal;
    if (candidate_blocks_non_rotation(state, nonRotation)) {
        outResult->nonRotationRejectedByCandidate = 1;
    } else {
        dispatch_non_rotation(state, nonRotation);
    }

    capture_after(state, outResult);
    return outResult->onlyRotationSucceeded;
}

const Dm1V1MirrorCandidateKeyboardRotationComboEvidencePc34Compat *
DM1_V1_MirrorCandidateKeyboardRotationCombo_EvidencePc34Compat(void)
{
    return &s_evidence;
}

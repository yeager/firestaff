#include "dm1_v1_mirror_candidate_runtime_spell_rune_pc34_compat.h"

#include <string.h>

enum {
    kNoCommand = 0,
    kSpellSymbol1Command = 101,
    kSpellSymbol6Command = 106,
    kSpellRecantCommand = 107,
    kSpellSymbol1Left = 235,
    kSpellSymbol1Top = 51,
    kSpellSymbolWidth = 13,
    kSpellSymbolHeight = 11,
    kSpellSymbolStride = 14,
    kSpellRecantLeft = 305,
    kSpellRecantTop = 63,
    kSpellRecantWidth = 14,
    kSpellRecantHeight = 11
};

static int in_box(int x, int y, int left, int top, int width, int height)
{
    return x >= left && x < left + width && y >= top && y < top + height;
}

static int nested_spell_symbol_command(int x, int y)
{
    int symbolIndex;

    /* ReDMCSB: COMMAND.C:474-482 maps the C100 spell-area child click
     * through G0454; CLIKMENU.C F0370:386-510 resolves C101..C106 to
     * F0369, which calls SYMBOL.C F0399 to append the selected rune. */
    for (symbolIndex = 0; symbolIndex < 6; ++symbolIndex) {
        if (in_box(x, y,
                   kSpellSymbol1Left + symbolIndex * kSpellSymbolStride,
                   kSpellSymbol1Top,
                   kSpellSymbolWidth,
                   kSpellSymbolHeight)) {
            return kSpellSymbol1Command + symbolIndex;
        }
    }
    if (in_box(x, y,
               kSpellRecantLeft,
               kSpellRecantTop,
               kSpellRecantWidth,
               kSpellRecantHeight)) {
        return kSpellRecantCommand;
    }
    return kNoCommand;
}

static void capture_result_before(
    const Dm1V1MirrorCandidateRuntimeSpellRuneStatePc34Compat *state,
    Dm1V1MirrorCandidateRuntimeSpellRuneResultPc34Compat *result)
{
    int casterIndex;

    result->candidateChampionOrdinalBefore = state->candidateChampionOrdinal;
    result->candidateChampionOrdinalAfter = state->candidateChampionOrdinal;
    casterIndex = state->magicCasterChampionIndex;
    if (casterIndex >= 0 && casterIndex < 4) {
        result->symbolStepBefore = state->spellState.input[casterIndex].symbolStep;
        memcpy(result->symbolsBefore,
               state->spellState.input[casterIndex].symbols,
               sizeof(result->symbolsBefore));
    }
    result->symbolStepAfter = result->symbolStepBefore;
    memcpy(result->symbolsAfter, result->symbolsBefore, sizeof(result->symbolsAfter));
    result->manaBefore = state->casterStats.currentMana;
    result->manaAfter = state->casterStats.currentMana;
}

static void capture_result_after(
    const Dm1V1MirrorCandidateRuntimeSpellRuneStatePc34Compat *state,
    Dm1V1MirrorCandidateRuntimeSpellRuneResultPc34Compat *result)
{
    int casterIndex = state->magicCasterChampionIndex;

    result->candidateChampionOrdinalAfter = state->candidateChampionOrdinal;
    if (casterIndex >= 0 && casterIndex < 4) {
        result->symbolStepAfter = state->spellState.input[casterIndex].symbolStep;
        memcpy(result->symbolsAfter,
               state->spellState.input[casterIndex].symbols,
               sizeof(result->symbolsAfter));
    }
    result->manaAfter = state->casterStats.currentMana;
}

void DM1_V1_MirrorCandidateRuntimeSpellRune_InitPc34Compat(
    Dm1V1MirrorCandidateRuntimeSpellRuneStatePc34Compat *state)
{
    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    state->candidateChampionOrdinal = 2u;
    state->partyChampionCount = 2;
    state->magicCasterChampionIndex = 0;
    DM1_V1_InputCommandQueue_InitPc34Compat(&state->queue);
    dm1_spell_init(&state->spellState);
    state->spellState.magicCasterIndex = state->magicCasterChampionIndex;
    state->casterStats.currentHealth = 100;
    state->casterStats.currentMana = 50;
    state->casterStats.maximumMana = 50;
    state->casterStats.wisdom = 50;
}

void DM1_V1_MirrorCandidateRuntimeSpellRune_ClearCandidatePc34Compat(
    Dm1V1MirrorCandidateRuntimeSpellRuneStatePc34Compat *state)
{
    if (!state) {
        return;
    }
    state->candidateChampionOrdinal = 0u;
}

int DM1_V1_MirrorCandidateRuntimeSpellRune_ClickSpellAreaPc34Compat(
    Dm1V1MirrorCandidateRuntimeSpellRuneStatePc34Compat *state,
    int x,
    int y,
    Dm1V1MirrorCandidateRuntimeSpellRuneResultPc34Compat *outResult)
{
    struct Dm1V1InputQueueProcessResultPc34Compat queueResult;
    int nestedCommand;
    int casterIndex;

    if (!state || !outResult) {
        return 0;
    }
    memset(outResult, 0, sizeof(*outResult));
    capture_result_before(state, outResult);

    outResult->commandQueued = DM1_V1_InputCommandQueue_EnqueueCommandPc34Compat(
        &state->queue,
        DM1_V1_MIRROR_CANDIDATE_RUNTIME_SPELL_RUNE_C100_PC34_COMPAT,
        x,
        y);
    queueResult = DM1_V1_InputCommandQueue_ProcessOnePc34Compat(
        &state->queue, 0, 0, 0, 0);
    outResult->commandDequeued = queueResult.dequeued;
    outResult->command = queueResult.command;

    if (!queueResult.dequeued ||
        queueResult.command != DM1_V1_MIRROR_CANDIDATE_RUNTIME_SPELL_RUNE_C100_PC34_COMPAT) {
        capture_result_after(state, outResult);
        return 0;
    }

    /* ReDMCSB: COMMAND.C:2302-2311 reaches F0370 only when G0299 is clear
     * and G0514 names a live magic caster; this runtime path uses Firestaff's
     * command queue and spell-symbol API rather than a contract-only struct. */
    outResult->reachedF0380SpellGate = 1;
    if (state->candidateChampionOrdinal != 0u) {
        outResult->blockedByCandidate = 1;
        capture_result_after(state, outResult);
        return 0;
    }
    casterIndex = state->magicCasterChampionIndex;
    if (casterIndex < 0 || casterIndex >= state->partyChampionCount) {
        outResult->blockedByMissingCaster = 1;
        capture_result_after(state, outResult);
        return 0;
    }

    outResult->dispatchedSpellArea = 1;
    nestedCommand = nested_spell_symbol_command(x, y);
    outResult->nestedSpellSymbolCommand = nestedCommand;
    if (nestedCommand >= kSpellSymbol1Command &&
        nestedCommand <= kSpellSymbol6Command) {
        outResult->symbolIndex = nestedCommand - kSpellSymbol1Command;
        outResult->runeAdded = dm1_spell_addSymbol(
            &state->spellState,
            casterIndex,
            &state->casterStats,
            outResult->symbolIndex);
    } else if (nestedCommand == kSpellRecantCommand) {
        /* ReDMCSB: COMMAND.C:482 maps C107 to the recant zone; CLIKMENU.C
         * F0369:373-381/F0370:499-512 routes index >= 6 to SYMBOL.C F0400,
         * which deletes the previous Champion.Symbol without refunding mana. */
        outResult->symbolIndex =
            DM1_V1_MIRROR_CANDIDATE_RUNTIME_SPELL_RUNE_NONE_PC34_COMPAT;
        dm1_spell_deleteSymbol(&state->spellState, casterIndex);
        outResult->runeDeleted = 1;
    } else {
        outResult->symbolIndex =
            DM1_V1_MIRROR_CANDIDATE_RUNTIME_SPELL_RUNE_NONE_PC34_COMPAT;
    }
    capture_result_after(state, outResult);
    return outResult->dispatchedSpellArea;
}

int DM1_V1_MirrorCandidateRuntimeSpellRune_CancelPc34Compat(
    Dm1V1MirrorCandidateRuntimeSpellRuneStatePc34Compat *state,
    Dm1V1MirrorCandidateRuntimeSpellRuneResultPc34Compat *outResult)
{
    int casterIndex;

    if (!state || !outResult) {
        return 0;
    }
    memset(outResult, 0, sizeof(*outResult));
    capture_result_before(state, outResult);

    casterIndex = state->magicCasterChampionIndex;
    if (casterIndex < 0 || casterIndex >= state->partyChampionCount) {
        outResult->blockedByMissingCaster = 1;
        capture_result_after(state, outResult);
        return 0;
    }

    /* Firestaff's cancel/clear binding preserves the source-locked clear
     * contract used after a completed cast: MENU.C:1656-1657 clears
     * Champion.Symbols[0] and resets Champion.SymbolStep to 0. */
    state->spellState.input[casterIndex].symbols[0] = '\0';
    state->spellState.input[casterIndex].symbolStep = 0;
    outResult->spellCancelled = 1;

    capture_result_after(state, outResult);
    return 1;
}

const char *DM1_V1_MirrorCandidateRuntimeSpellRune_SourceEvidencePc34Compat(void)
{
    return "ReDMCSB REVIVE.C F0282:744-806 clears/cancels G0299 for the "
           "candidate panel; COMMAND.C:2155-2182 gates status/inventory on "
           "!G0299; COMMAND.C:2302-2311 gates C100 spell-area dispatch on "
           "!G0299 and G0514; MOVESENS.C:1501-1503 sends C127 portrait "
           "sensors to F0280; COMMAND.C:474-482 and CLIKMENU.C F0370:386-510 "
           "resolve C100 child spell-symbol clicks; CLIKMENU.C F0369:373-381 "
           "and F0370:499-512 route C107 recant to SYMBOL.C F0400; SYMBOL.C "
           "F0399 appends the selected rune and advances Champion.SymbolStep; "
           "MENU.C:1656-1657 clears Champion.Symbols and resets SymbolStep.";
}

#include "dm1_v1_mirror_candidate_runtime_spell_rune_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int gTests;
static int gPasses;

#define CHECK_RUNTIME(cond, msg, anchor) do { \
    ++gTests; \
    if (cond) { \
        ++gPasses; \
    } else { \
        printf("FAIL: %s [%s]\n", msg, anchor); \
    } \
} while (0)

static void test_candidate_blocks_spell_rune_dispatch(void)
{
    Dm1V1MirrorCandidateRuntimeSpellRuneStatePc34Compat state;
    Dm1V1MirrorCandidateRuntimeSpellRuneResultPc34Compat blocked;

    DM1_V1_MirrorCandidateRuntimeSpellRune_InitPc34Compat(&state);
    (void)DM1_V1_MirrorCandidateRuntimeSpellRune_ClickSpellAreaPc34Compat(
        &state, 235, 51, &blocked);

    CHECK_RUNTIME(blocked.commandQueued == 1 &&
                      blocked.commandDequeued == 1 &&
                      blocked.command ==
                          DM1_V1_MIRROR_CANDIDATE_RUNTIME_SPELL_RUNE_C100_PC34_COMPAT,
                  "runtime queue dequeues the real C100 spell-area command",
                  "COMMAND.C:2045-2156; COMMAND.C:2302-2311");
    CHECK_RUNTIME(blocked.reachedF0380SpellGate == 1 &&
                      blocked.blockedByCandidate == 1,
                  "G0299 candidate panel blocks spell-area dispatch",
                  "COMMAND.C:2302-2311");
    CHECK_RUNTIME(blocked.dispatchedSpellArea == 0 &&
                      blocked.runeAdded == 0,
                  "blocked C100 does not enter the rune append path",
                  "COMMAND.C:2302-2311; CLIKMENU.C F0370");
    CHECK_RUNTIME(state.spellState.input[0].symbolStep == 0 &&
                      state.spellState.input[0].symbols[0] == '\0' &&
                      state.casterStats.currentMana == 50,
                  "candidate-blocked rune click leaves spell symbols and mana untouched",
                  "SYMBOL.C F0399; COMMAND.C:2302-2311");
}

static void test_cleared_candidate_allows_same_spell_rune_dispatch(void)
{
    Dm1V1MirrorCandidateRuntimeSpellRuneStatePc34Compat state;
    Dm1V1MirrorCandidateRuntimeSpellRuneResultPc34Compat blocked;
    Dm1V1MirrorCandidateRuntimeSpellRuneResultPc34Compat allowed;

    DM1_V1_MirrorCandidateRuntimeSpellRune_InitPc34Compat(&state);
    (void)DM1_V1_MirrorCandidateRuntimeSpellRune_ClickSpellAreaPc34Compat(
        &state, 235, 51, &blocked);
    DM1_V1_MirrorCandidateRuntimeSpellRune_ClearCandidatePc34Compat(&state);
    (void)DM1_V1_MirrorCandidateRuntimeSpellRune_ClickSpellAreaPc34Compat(
        &state, 235, 51, &allowed);

    CHECK_RUNTIME(allowed.candidateChampionOrdinalBefore == 0u &&
                      allowed.blockedByCandidate == 0,
                  "clearing G0299 reopens the C100 spell-area dispatch gate",
                  "REVIVE.C F0282:785-806; COMMAND.C:2302-2311");
    CHECK_RUNTIME(allowed.dispatchedSpellArea == 1 &&
                      allowed.nestedSpellSymbolCommand == 101 &&
                      allowed.symbolIndex == 0,
                  "same spell-area click reaches the C101 symbol child command",
                  "COMMAND.C:474-482; CLIKMENU.C F0370:386-510");
    CHECK_RUNTIME(allowed.runeAdded == 1 &&
                      allowed.symbolStepBefore == 0 &&
                      allowed.symbolStepAfter == 1,
                  "runtime spell API appends the Lo power rune",
                  "SYMBOL.C F0399");
    CHECK_RUNTIME(allowed.symbolsAfter[0] == dm1_encodeSymbol(0, 0) &&
                      allowed.symbolsAfter[1] == '\0',
                  "appended rune matches SYMBOL.C character encoding",
                  "SYMBOL.C F0399");
    CHECK_RUNTIME(allowed.manaBefore == 50 &&
                      allowed.manaAfter == 49 &&
                      state.casterStats.currentMana == 49,
                  "rune dispatch spends the real symbol mana cost",
                  "SYMBOL.C F0399; MENU.C:44-49");
}

static void test_missing_caster_still_does_not_dispatch(void)
{
    Dm1V1MirrorCandidateRuntimeSpellRuneStatePc34Compat state;
    Dm1V1MirrorCandidateRuntimeSpellRuneResultPc34Compat result;

    DM1_V1_MirrorCandidateRuntimeSpellRune_InitPc34Compat(&state);
    DM1_V1_MirrorCandidateRuntimeSpellRune_ClearCandidatePc34Compat(&state);
    state.magicCasterChampionIndex =
        DM1_V1_MIRROR_CANDIDATE_RUNTIME_SPELL_RUNE_NONE_PC34_COMPAT;
    (void)DM1_V1_MirrorCandidateRuntimeSpellRune_ClickSpellAreaPc34Compat(
        &state, 235, 51, &result);

    CHECK_RUNTIME(result.reachedF0380SpellGate == 1 &&
                      result.blockedByMissingCaster == 1,
                  "G0514 none blocks C100 even when G0299 is clear",
                  "COMMAND.C:2302-2311");
    CHECK_RUNTIME(result.dispatchedSpellArea == 0 &&
                      result.runeAdded == 0,
                  "missing magic caster does not append a rune",
                  "COMMAND.C:2302-2311; SYMBOL.C F0399");
}

static void test_source_evidence_mentions_runtime_anchors(void)
{
    const char *evidence =
        DM1_V1_MirrorCandidateRuntimeSpellRune_SourceEvidencePc34Compat();

    CHECK_RUNTIME(evidence != NULL,
                  "source evidence is present",
                  "REVIVE.C; COMMAND.C; MOVESENS.C");
    CHECK_RUNTIME(strstr(evidence, "REVIVE.C F0282:744-806") != NULL,
                  "evidence cites F0282 candidate clear/cancel path",
                  "REVIVE.C F0282:744-806");
    CHECK_RUNTIME(strstr(evidence, "COMMAND.C:2155-2182") != NULL,
                  "evidence cites status/inventory G0299 gates",
                  "COMMAND.C:2155-2182");
    CHECK_RUNTIME(strstr(evidence, "COMMAND.C:2302-2311") != NULL,
                  "evidence cites C100 spell/action G0299 gate",
                  "COMMAND.C:2302-2311");
    CHECK_RUNTIME(strstr(evidence, "MOVESENS.C:1501-1503") != NULL,
                  "evidence cites C127 champion portrait sensor",
                  "MOVESENS.C:1501-1503");
    CHECK_RUNTIME(strstr(evidence, "CLIKMENU.C F0370") != NULL &&
                      strstr(evidence, "SYMBOL.C F0399") != NULL,
                  "evidence cites runtime spell-area and rune append paths",
                  "CLIKMENU.C F0370; SYMBOL.C F0399");
}

int main(void)
{
    Dm1V1MirrorCandidateRuntimeSpellRuneStatePc34Compat state;
    Dm1V1MirrorCandidateRuntimeSpellRuneResultPc34Compat blocked;
    Dm1V1MirrorCandidateRuntimeSpellRuneResultPc34Compat allowed;

    test_candidate_blocks_spell_rune_dispatch();
    test_cleared_candidate_allows_same_spell_rune_dispatch();
    test_missing_caster_still_does_not_dispatch();
    test_source_evidence_mentions_runtime_anchors();

    DM1_V1_MirrorCandidateRuntimeSpellRune_InitPc34Compat(&state);
    (void)DM1_V1_MirrorCandidateRuntimeSpellRune_ClickSpellAreaPc34Compat(
        &state, 235, 51, &blocked);
    DM1_V1_MirrorCandidateRuntimeSpellRune_ClearCandidatePc34Compat(&state);
    (void)DM1_V1_MirrorCandidateRuntimeSpellRune_ClickSpellAreaPc34Compat(
        &state, 235, 51, &allowed);

    printf("PASS dm1_v1_mirror_candidate_runtime_spell_rune_pc34_compat "
           "%d/%d assertions; runtime C100 blocked=%d dispatched=%d "
           "symbolStep=%d->%d mana=%d->%d\n",
           gPasses, gTests, blocked.blockedByCandidate,
           allowed.dispatchedSpellArea, allowed.symbolStepBefore,
           allowed.symbolStepAfter, allowed.manaBefore, allowed.manaAfter);
    return gPasses == gTests ? 0 : 1;
}

#ifndef DM1_V1_MIRROR_CANDIDATE_RUNTIME_SPELL_RUNE_PC34_COMPAT_H
#define DM1_V1_MIRROR_CANDIDATE_RUNTIME_SPELL_RUNE_PC34_COMPAT_H

#include "dm1_v1_input_command_queue_pc34_compat.h"
#include "dm1_v1_spell_casting_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_MIRROR_CANDIDATE_RUNTIME_SPELL_RUNE_NONE_PC34_COMPAT (-1)
#define DM1_V1_MIRROR_CANDIDATE_RUNTIME_SPELL_RUNE_C100_PC34_COMPAT 100
#define DM1_V1_MIRROR_CANDIDATE_RUNTIME_SPELL_RUNE_C107_PC34_COMPAT 107

typedef struct DM1_V1_MirrorCandidateRuntimeSpellRuneStatePc34Compat {
    unsigned int candidateChampionOrdinal;
    int partyChampionCount;
    int magicCasterChampionIndex;
    struct Dm1V1InputCommandQueuePc34Compat queue;
    DM1_SpellCastingState spellState;
    DM1_ChampionSpellStats casterStats;
} DM1_V1_MirrorCandidateRuntimeSpellRuneStatePc34Compat;

typedef struct DM1_V1_MirrorCandidateRuntimeSpellRuneResultPc34Compat {
    unsigned int candidateChampionOrdinalBefore;
    unsigned int candidateChampionOrdinalAfter;
    int commandQueued;
    int commandDequeued;
    int command;
    int reachedF0380SpellGate;
    int blockedByCandidate;
    int blockedByMissingCaster;
    int dispatchedSpellArea;
    int nestedSpellSymbolCommand;
    int symbolIndex;
    int runeAdded;
    int runeDeleted;
    int spellCancelled;
    int symbolStepBefore;
    int symbolStepAfter;
    int manaBefore;
    int manaAfter;
    char symbolsBefore[5];
    char symbolsAfter[5];
} DM1_V1_MirrorCandidateRuntimeSpellRuneResultPc34Compat;

typedef DM1_V1_MirrorCandidateRuntimeSpellRuneStatePc34Compat
    Dm1V1MirrorCandidateRuntimeSpellRuneStatePc34Compat;
typedef DM1_V1_MirrorCandidateRuntimeSpellRuneResultPc34Compat
    Dm1V1MirrorCandidateRuntimeSpellRuneResultPc34Compat;

void DM1_V1_MirrorCandidateRuntimeSpellRune_InitPc34Compat(
    DM1_V1_MirrorCandidateRuntimeSpellRuneStatePc34Compat *state);

void DM1_V1_MirrorCandidateRuntimeSpellRune_ClearCandidatePc34Compat(
    DM1_V1_MirrorCandidateRuntimeSpellRuneStatePc34Compat *state);

int DM1_V1_MirrorCandidateRuntimeSpellRune_ClickSpellAreaPc34Compat(
    DM1_V1_MirrorCandidateRuntimeSpellRuneStatePc34Compat *state,
    int x,
    int y,
    DM1_V1_MirrorCandidateRuntimeSpellRuneResultPc34Compat *outResult);

int DM1_V1_MirrorCandidateRuntimeSpellRune_CancelPc34Compat(
    DM1_V1_MirrorCandidateRuntimeSpellRuneStatePc34Compat *state,
    DM1_V1_MirrorCandidateRuntimeSpellRuneResultPc34Compat *outResult);

const char *DM1_V1_MirrorCandidateRuntimeSpellRune_SourceEvidencePc34Compat(void);

#ifdef __cplusplus
}
#endif

#endif

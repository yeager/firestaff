#ifndef DM1_V1_MIRROR_CANDIDATE_RESURRECT_REARM_PC34_COMPAT_H
#define DM1_V1_MIRROR_CANDIDATE_RESURRECT_REARM_PC34_COMPAT_H

#include "dm1_v1_champion_mirror_click_closed_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_MIRROR_CANDIDATE_RESURRECT_COMMAND_PC34_COMPAT 160
#define DM1_V1_MIRROR_CANDIDATE_ACTION_AREA_COMMAND_PC34_COMPAT 111

typedef struct Dm1V1MirrorCandidateResurrectRearmResultPc34Compat {
    int candidateChampionIndex;
    unsigned int candidateChampionOrdinalBefore;
    unsigned int candidateChampionOrdinalAfter;
    int currentHealthBefore;
    int currentHealthAfter;
    int previousLeaderIndex;
    int newLeaderIndex;
    int previousFrontD1cMirrorChampionOrdinal;
    int newFrontD1cMirrorChampionOrdinal;
    int frontD1cPortraitIndex;
    int validCandidatePanel;
    int resurrected;
    int ignoredNoCandidate;
    int ignoredNoDeadChampion;
    int candidateCleared;
    int panelC040Cleared;
    int mirrorRouteRearmed;
} Dm1V1MirrorCandidateResurrectRearmResultPc34Compat;

typedef struct Dm1V1MirrorCandidateStatusBoxResultPc34Compat {
    Dm1V1MirrorClickClosedResultPc34Compat statusBox;
    int statusBoxChangedLeader;
    int previousFrontD1cMirrorChampionOrdinal;
    int newFrontD1cMirrorChampionOrdinal;
    int frontD1cPortraitIndex;
    int mirrorRouteLive;
} Dm1V1MirrorCandidateStatusBoxResultPc34Compat;

typedef struct Dm1V1MirrorCandidateCommandGateResultPc34Compat {
    int command;
    int panelC040Closed;
    int blockedByCandidatePanel;
    int commandAllowed;
} Dm1V1MirrorCandidateCommandGateResultPc34Compat;

int DM1_V1_MirrorCandidateResurrectRearm_ProcessResurrectPc34Compat(
    Dm1V1MirrorClickClosedStatePc34Compat *state,
    Dm1V1MirrorCandidateResurrectRearmResultPc34Compat *outResult);

int DM1_V1_MirrorCandidateResurrectRearm_ProcessStatusBoxClickPc34Compat(
    Dm1V1MirrorClickClosedStatePc34Compat *state,
    int command,
    int x,
    int y,
    unsigned int mouseButtons,
    Dm1V1MirrorCandidateStatusBoxResultPc34Compat *outResult);

int DM1_V1_MirrorCandidateResurrectRearm_CanProcessCommandPc34Compat(
    const Dm1V1MirrorClickClosedStatePc34Compat *state,
    int command,
    Dm1V1MirrorCandidateCommandGateResultPc34Compat *outResult);

const char *DM1_V1_MirrorCandidateResurrectRearm_SourceEvidencePc34Compat(void);

#ifdef __cplusplus
}
#endif

#endif

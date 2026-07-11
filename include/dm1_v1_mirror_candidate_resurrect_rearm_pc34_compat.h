#ifndef DM1_V1_MIRROR_CANDIDATE_RESURRECT_REARM_PC34_COMPAT_H
#define DM1_V1_MIRROR_CANDIDATE_RESURRECT_REARM_PC34_COMPAT_H

#include "dm1_v1_champion_mirror_click_closed_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_MIRROR_CANDIDATE_RESURRECT_COMMAND_PC34_COMPAT 160
#define DM1_V1_MIRROR_CANDIDATE_ACTION_AREA_COMMAND_PC34_COMPAT 111

typedef struct DM1_V1_MirrorCandidateResurrectRearmResultPc34Compat {
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
} DM1_V1_MirrorCandidateResurrectRearmResultPc34Compat;

typedef struct DM1_V1_MirrorCandidateStatusBoxResultPc34Compat {
    DM1_V1_MirrorClickClosedResultPc34 statusBox;
    int statusBoxChangedLeader;
    int previousFrontD1cMirrorChampionOrdinal;
    int newFrontD1cMirrorChampionOrdinal;
    int frontD1cPortraitIndex;
    int mirrorRouteLive;
} DM1_V1_MirrorCandidateStatusBoxResultPc34Compat;

typedef struct DM1_V1_MirrorCandidateCommandGateResultPc34Compat {
    int command;
    int panelC040Closed;
    int blockedByCandidatePanel;
    int commandAllowed;
} DM1_V1_MirrorCandidateCommandGateResultPc34Compat;

typedef DM1_V1_MirrorCandidateResurrectRearmResultPc34Compat
    Dm1V1MirrorCandidateResurrectRearmResultPc34Compat;
typedef DM1_V1_MirrorCandidateStatusBoxResultPc34Compat
    Dm1V1MirrorCandidateStatusBoxResultPc34Compat;
typedef DM1_V1_MirrorCandidateCommandGateResultPc34Compat
    Dm1V1MirrorCandidateCommandGateResultPc34Compat;

int DM1_V1_MirrorCandidateResurrectRearm_ProcessResurrectPc34Compat(
    DM1_V1_MirrorClickClosedStatePc34 *state,
    DM1_V1_MirrorCandidateResurrectRearmResultPc34Compat *outResult);

int DM1_V1_MirrorCandidateResurrectRearm_ProcessStatusBoxClickPc34Compat(
    DM1_V1_MirrorClickClosedStatePc34 *state,
    int command,
    int x,
    int y,
    unsigned int mouseButtons,
    DM1_V1_MirrorCandidateStatusBoxResultPc34Compat *outResult);

int DM1_V1_MirrorCandidateResurrectRearm_CanProcessCommandPc34Compat(
    const DM1_V1_MirrorClickClosedStatePc34 *state,
    int command,
    DM1_V1_MirrorCandidateCommandGateResultPc34Compat *outResult);

const char *DM1_V1_MirrorCandidateResurrectRearm_SourceEvidencePc34Compat(void);

#ifdef __cplusplus
}
#endif

#endif

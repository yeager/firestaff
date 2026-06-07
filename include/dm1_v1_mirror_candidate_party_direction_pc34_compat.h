#ifndef DM1_V1_MIRROR_CANDIDATE_PARTY_DIRECTION_PC34_COMPAT_H
#define DM1_V1_MIRROR_CANDIDATE_PARTY_DIRECTION_PC34_COMPAT_H

#include "dm1_v1_input_command_queue_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_MIRROR_CANDIDATE_PARTY_DIRECTION_PANEL_W_PC34_COMPAT 4
#define DM1_V1_MIRROR_CANDIDATE_PARTY_DIRECTION_PANEL_H_PC34_COMPAT 3
#define DM1_V1_MIRROR_CANDIDATE_PARTY_DIRECTION_C012_PC34_COMPAT 12
#define DM1_V1_MIRROR_CANDIDATE_PARTY_DIRECTION_C016_PC34_COMPAT 16
#define DM1_V1_MIRROR_CANDIDATE_PARTY_DIRECTION_C040_GRAPHIC_PC34_COMPAT 40
#define DM1_V1_MIRROR_CANDIDATE_PARTY_DIRECTION_C10_TRANSPARENT_PC34_COMPAT 10
#define DM1_V1_MIRROR_CANDIDATE_PARTY_DIRECTION_M568_PANEL_PC34_COMPAT 568

typedef struct Dm1V1MirrorCandidatePartyDirectionEvidencePc34Compat {
    const char *commandRouteAnchor;
    const char *championLeaderAnchor;
    const char *dunviewPortraitAnchor;
    const char *revivePanelAnchor;
    const char *panelPixelAnchor;
    const char *g0299Anchor;
    const char *g0420Anchor;
    const char *nonOverlapNote;
    const char *contractScope;
} Dm1V1MirrorCandidatePartyDirectionEvidencePc34Compat;

typedef struct Dm1V1MirrorCandidatePartyDirectionStatePc34Compat {
    unsigned int candidateChampionOrdinal;
    unsigned int candidateIdentityAnchor;
    unsigned int syntheticC127PortraitToken;
    unsigned int partyChampionCount;
    int partyDirection;
    int candidateDirection;
    int leaderIndex;
    int panelContent;
    int c040PanelOpen;
    int inventoryChampionOrdinal;
    int c159NestedReached;
    int f0367StatusDispatchCount;
    int f0368SetLeaderCount;
    int duplicateCandidateAppendCount;
    int resurrectDispatchCount;
    int reincarnateDispatchCount;
    int cancelDispatchCount;
    int restDispatchCount;
    int saveDispatchCount;
    int realAssetPortraitParityClaimed;
    unsigned char panelSource
        [DM1_V1_MIRROR_CANDIDATE_PARTY_DIRECTION_PANEL_W_PC34_COMPAT *
         DM1_V1_MIRROR_CANDIDATE_PARTY_DIRECTION_PANEL_H_PC34_COMPAT];
    unsigned char panelDestination
        [DM1_V1_MIRROR_CANDIDATE_PARTY_DIRECTION_PANEL_W_PC34_COMPAT *
         DM1_V1_MIRROR_CANDIDATE_PARTY_DIRECTION_PANEL_H_PC34_COMPAT];
    struct Dm1V1InputCommandQueuePc34Compat queue;
} Dm1V1MirrorCandidatePartyDirectionStatePc34Compat;

typedef struct Dm1V1MirrorCandidatePartyDirectionResultPc34Compat {
    const Dm1V1MirrorCandidatePartyDirectionEvidencePc34Compat *evidence;
    unsigned int candidateChampionOrdinalBefore;
    unsigned int candidateChampionOrdinalAfter;
    unsigned int candidateIdentityBefore;
    unsigned int candidateIdentityAfter;
    unsigned int partyChampionCountBefore;
    unsigned int partyChampionCountAfter;
    int partyDirectionBefore;
    int partyDirectionAfter;
    int candidateDirectionBefore;
    int candidateDirectionAfter;
    int leaderIndexBefore;
    int leaderIndexAfter;
    int panelContentBefore;
    int panelContentAfter;
    int c040PanelOpenBefore;
    int c040PanelOpenAfter;
    int inventoryChampionOrdinalBefore;
    int inventoryChampionOrdinalAfter;
    int commandQueuedCount;
    int commandDequeuedCount;
    int turnDispatchCount;
    int statusCommandDequeued;
    int statusGateBlockedByG0299;
    int c159NestedReachedBefore;
    int c159NestedReachedAfter;
    int f0367StatusDispatchCountBefore;
    int f0367StatusDispatchCountAfter;
    int f0368SetLeaderCountBefore;
    int f0368SetLeaderCountAfter;
    int duplicateCandidateAppendCountBefore;
    int duplicateCandidateAppendCountAfter;
    int resurrectDispatchCountBefore;
    int resurrectDispatchCountAfter;
    int reincarnateDispatchCountBefore;
    int reincarnateDispatchCountAfter;
    int cancelDispatchCountBefore;
    int cancelDispatchCountAfter;
    int restDispatchCountBefore;
    int restDispatchCountAfter;
    int saveDispatchCountBefore;
    int saveDispatchCountAfter;
    int c10TransparentPixelPreserved;
    int c040OpaquePanelPixelCopied;
    int panelPixelContractReal;
    int portraitContractOnly;
    int g0299AnchorPreserved;
    int g0420IdentityPreserved;
    int panelOwnerPreserved;
    int noDuplicateCandidate;
} Dm1V1MirrorCandidatePartyDirectionResultPc34Compat;

void DM1_V1_MirrorCandidatePartyDirection_InitPc34Compat(
    Dm1V1MirrorCandidatePartyDirectionStatePc34Compat *state);

int DM1_V1_MirrorCandidatePartyDirection_RunFiveTurnC159ScenarioPc34Compat(
    Dm1V1MirrorCandidatePartyDirectionStatePc34Compat *state,
    Dm1V1MirrorCandidatePartyDirectionResultPc34Compat *outResult);

const Dm1V1MirrorCandidatePartyDirectionEvidencePc34Compat *
DM1_V1_MirrorCandidatePartyDirection_EvidencePc34Compat(void);

#ifdef __cplusplus
}
#endif

#endif

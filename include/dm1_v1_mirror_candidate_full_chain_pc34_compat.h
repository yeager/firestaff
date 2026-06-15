#ifndef DM1_V1_MIRROR_CANDIDATE_FULL_CHAIN_PC34_COMPAT_H
#define DM1_V1_MIRROR_CANDIDATE_FULL_CHAIN_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_MIRROR_CANDIDATE_FULL_CHAIN_CHAMPION_COUNT_PC34_COMPAT 4
#define DM1_V1_MIRROR_CANDIDATE_FULL_CHAIN_THING_NONE_PC34_COMPAT 0xFFFFu
#define DM1_V1_MIRROR_CANDIDATE_FULL_CHAIN_C040_GRAPHIC_PC34_COMPAT 40
#define DM1_V1_MIRROR_CANDIDATE_FULL_CHAIN_C159_ZONE_PC34_COMPAT 159
#define DM1_V1_MIRROR_CANDIDATE_FULL_CHAIN_M568_PANEL_PC34_COMPAT 568

typedef struct Dm1V1MirrorCandidateFullChainEvidencePc34Compat {
    const char *chamdrawAllStatesAnchor;
    const char *championPartyDirectionAnchor;
    const char *championLeaderHandAnchor;
    const char *commandPanelAnchor;
    const char *revivePanelAnchor;
    const char *nonOverlapNote;
    const char *contractScope;
} Dm1V1MirrorCandidateFullChainEvidencePc34Compat;

typedef struct Dm1V1MirrorCandidateFullChainChampionPc34Compat {
    unsigned int ordinal;
    unsigned int handThing;
    int present;
    int cell;
    int direction;
    int wounded;
    int poisoned;
    int iconClickCount;
    int drawStateCount;
} Dm1V1MirrorCandidateFullChainChampionPc34Compat;

typedef struct Dm1V1MirrorCandidateFullChainStatePc34Compat {
    int contractOnly;
    int partyChampionCount;
    int leaderIndex;
    int nonLeaderChampionCount;
    int partyDirection;
    unsigned int leaderHandThing;
    int leaderHandEmpty;
    int leaderHandFullBeforePickup;
    int panelContent;
    int c040PanelOpen;
    int c040PanelPixelsDrawn;
    unsigned int candidateChampionOrdinal;
    unsigned int activeCandidateOrdinal;
    int activeCandidateRowIndex;
    int openedPanelCount;
    int allStateDrawCount;
    int woundedPoisonedDrawCount;
    int partyDirectionSetCount;
    int rotatedCandidateCount;
    int firstCandidateIconClickCount;
    int rotatedCandidateIconClickCount;
    int leaderHandPutCount;
    int occupiedHandRejectCount;
    int candidateClearCount;
    Dm1V1MirrorCandidateFullChainChampionPc34Compat
        champions[DM1_V1_MIRROR_CANDIDATE_FULL_CHAIN_CHAMPION_COUNT_PC34_COMPAT];
} Dm1V1MirrorCandidateFullChainStatePc34Compat;

typedef struct Dm1V1MirrorCandidateFullChainResultPc34Compat {
    const Dm1V1MirrorCandidateFullChainEvidencePc34Compat *evidence;
    int openedPanel;
    int firstIconClicked;
    int rotatedCandidate;
    int rotatedIconClicked;
    int pickupAttempted;
    int pickupSucceeded;
    int pickupRejectedOccupiedHand;
    int panelOpenAfterOpen;
    int panelOpenAfterFirstClick;
    int panelOpenAfterRotation;
    int panelOpenAfterRotatedClick;
    int panelOpenAfterPickup;
    int candidateOrdinalBeforeOpen;
    int candidateOrdinalAfterOpen;
    int candidateOrdinalAfterFirstClick;
    int candidateOrdinalAfterRotation;
    int candidateOrdinalAfterRotatedClick;
    int candidateOrdinalAfterPickup;
    int activeCandidateBeforeRotation;
    int activeCandidateAfterRotation;
    int leaderHandEmptyBeforePickup;
    int leaderHandEmptyAfterPickup;
    unsigned int leaderHandThingBeforePickup;
    unsigned int leaderHandThingAfterPickup;
    int partyDirectionBeforeRotation;
    int partyDirectionAfterRotation;
    int rotatedCandidateDirectionBefore;
    int rotatedCandidateDirectionAfter;
    int allStateDrawCountAfterFirstClick;
    int allStateDrawCountAfterRotatedClick;
    int woundedPoisonedDrawCountAfterRotatedClick;
    int leaderHandPutCountAfterPickup;
    int occupiedHandRejectCountAfterPickup;
    int candidateClearCountAfterPickup;
    int nonLeaderChampionPresent;
    int sourceLockedFullChain;
} Dm1V1MirrorCandidateFullChainResultPc34Compat;

void DM1_V1_MirrorCandidateFullChain_InitPc34Compat(
    Dm1V1MirrorCandidateFullChainStatePc34Compat *state);

void DM1_V1_MirrorCandidateFullChain_SetLeaderHandFullBeforePickupPc34Compat(
    Dm1V1MirrorCandidateFullChainStatePc34Compat *state,
    int leaderHandFullBeforePickup);

int DM1_V1_MirrorCandidateFullChain_RunPc34Compat(
    Dm1V1MirrorCandidateFullChainStatePc34Compat *state,
    Dm1V1MirrorCandidateFullChainResultPc34Compat *outResult);

const Dm1V1MirrorCandidateFullChainEvidencePc34Compat *
DM1_V1_MirrorCandidateFullChain_EvidencePc34Compat(void);

#ifdef __cplusplus
}
#endif

#endif

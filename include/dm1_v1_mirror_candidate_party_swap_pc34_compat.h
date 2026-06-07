#ifndef DM1_V1_MIRROR_CANDIDATE_PARTY_SWAP_PC34_COMPAT_H
#define DM1_V1_MIRROR_CANDIDATE_PARTY_SWAP_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_MIRROR_CANDIDATE_PARTY_SWAP_MAX_PARTY_PC34_COMPAT 4
#define DM1_V1_MIRROR_CANDIDATE_PARTY_SWAP_EMPTY_PC34_COMPAT 0
#define DM1_V1_MIRROR_CANDIDATE_PARTY_SWAP_LEADER_ID_PC34_COMPAT 101
#define DM1_V1_MIRROR_CANDIDATE_PARTY_SWAP_A_ID_PC34_COMPAT 202
#define DM1_V1_MIRROR_CANDIDATE_PARTY_SWAP_B_ID_PC34_COMPAT 404
#define DM1_V1_MIRROR_CANDIDATE_PARTY_SWAP_C_ID_PC34_COMPAT 303
#define DM1_V1_MIRROR_CANDIDATE_PARTY_SWAP_C00_SLOT_LEADER_HAND_PC34_COMPAT -1
#define DM1_V1_MIRROR_CANDIDATE_PARTY_SWAP_C01_SLOT_ACTION_HAND_PC34_COMPAT 1
#define DM1_V1_MIRROR_CANDIDATE_PARTY_SWAP_C28_PARTY_FIRST_PC34_COMPAT 28
#define DM1_V1_MIRROR_CANDIDATE_PARTY_SWAP_C29_PARTY_LAST_PC34_COMPAT 29

typedef struct Dm1V1MirrorCandidatePartySwapEvidencePc34Compat {
    const char *leaderHandAnchor;
    const char *slotClickAnchor;
    const char *drawAllAnchor;
    const char *defsAnchor;
    const char *chamdrawAnchor;
    const char *partyAnchor;
    const char *mirrorClosedAnchor;
    const char *nonOverlapNote;
} Dm1V1MirrorCandidatePartySwapEvidencePc34Compat;

typedef struct Dm1V1MirrorCandidatePartySwapStatePc34Compat {
    int g0227_aT_Party
        [DM1_V1_MIRROR_CANDIDATE_PARTY_SWAP_MAX_PARTY_PC34_COMPAT];
    int g0305_ui_PartyChampionCount;
    int g0411_i_LeaderIndex;
    int g0299_ui_CandidateChampionOrdinal;
    int g0423_i_InventoryChampionOrdinal;
    int mirrorPanelOpen;
    int panelDrawCount;
    int f0293DrawAllChampionStatesCount;
    int swapAttemptCount;
    int swapAcceptedCount;
    int rejectedLeaderSwapCount;
    int rejectedEmptySlotCount;
    int rejectedMirrorClosedCount;
} Dm1V1MirrorCandidatePartySwapStatePc34Compat;

typedef struct Dm1V1MirrorCandidatePartySwapResultPc34Compat {
    const Dm1V1MirrorCandidatePartySwapEvidencePc34Compat *evidence;
    int requestedIndexA;
    int requestedIndexB;
    int beforeParty
        [DM1_V1_MIRROR_CANDIDATE_PARTY_SWAP_MAX_PARTY_PC34_COMPAT];
    int afterParty
        [DM1_V1_MIRROR_CANDIDATE_PARTY_SWAP_MAX_PARTY_PC34_COMPAT];
    int partyCountBefore;
    int partyCountAfter;
    int leaderIndexBefore;
    int leaderIndexAfter;
    int leaderIdBefore;
    int leaderIdAfter;
    int mirrorPanelOpenBefore;
    int mirrorPanelOpenAfter;
    int candidateOrdinalBefore;
    int candidateOrdinalAfter;
    int inventoryOrdinalBefore;
    int inventoryOrdinalAfter;
    int panelDrawCountBefore;
    int panelDrawCountAfter;
    int f0293DrawAllCountBefore;
    int f0293DrawAllCountAfter;
    int accepted;
    int rejectedLeaderSwap;
    int rejectedEmptySlot;
    int rejectedMirrorClosed;
    int partyReordered;
    int partyOrderUnchanged;
    int leaderPreserved;
    int partyCountPreserved;
    int mirrorOwnerPreserved;
    int panelDrawRefreshedOnce;
    int f0293CalledOnce;
    int f0293NotCalledTwice;
    int onlyRequestedPairMoved;
    int g0227PartyContract;
} Dm1V1MirrorCandidatePartySwapResultPc34Compat;

void DM1_V1_MirrorCandidatePartySwap_InitPc34Compat(
    Dm1V1MirrorCandidatePartySwapStatePc34Compat *state,
    int mirrorPanelOpen);

int DM1_V1_MirrorCandidatePartySwap_RunPc34Compat(
    Dm1V1MirrorCandidatePartySwapStatePc34Compat *state,
    int indexA,
    int indexB,
    Dm1V1MirrorCandidatePartySwapResultPc34Compat *outResult);

const Dm1V1MirrorCandidatePartySwapEvidencePc34Compat *
DM1_V1_MirrorCandidatePartySwap_EvidencePc34Compat(void);

#ifdef __cplusplus
}
#endif

#endif

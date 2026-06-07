#ifndef DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_PC34_COMPAT_H
#define DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_SLOT_COUNT_PC34_COMPAT 8
#define DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_PARTY_CAP_PC34_COMPAT 4
#define DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_NONE_PC34_COMPAT (-1)
#define DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_C30_SLOT_CHEST_1_PC34_COMPAT 30

#define DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_CHEST_A_PC34_COMPAT 0x0700
#define DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_CHEST_B_PC34_COMPAT 0x0701
#define DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_LEADER_HAND_PC34_COMPAT 0x0444
#define DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_SLOT0_THING_PC34_COMPAT 0x1100
#define DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_SLOT1_THING_PC34_COMPAT 0x1101
#define DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_SLOT2_THING_PC34_COMPAT 0x1102
#define DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_SLOT4_THING_PC34_COMPAT 0x1104
#define DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_SLOT7_THING_PC34_COMPAT 0x1107

typedef enum Dm1V1MirrorCandidateChestCloseLeaderHandPickupCasePc34Compat {
    DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_CASE_PENDING_PICKUP_CLOSE_PC34_COMPAT = 1,
    DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_CASE_CONFIRM_BEFORE_CLOSE_PC34_COMPAT = 2,
    DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_CASE_CONFIRM_AFTER_CLOSE_PC34_COMPAT = 3,
    DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_CASE_LAST_SLOT_GUARD_PC34_COMPAT = 4,
    DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_CASE_OCCUPIED_HAND_SWAP_PC34_COMPAT = 5,
    DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_CASE_EMPTY_PICKUP_NOOP_PC34_COMPAT = 6,
    DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_CASE_ROTATED_INVENTORY_OWNER_PC34_COMPAT = 7
} Dm1V1MirrorCandidateChestCloseLeaderHandPickupCasePc34Compat;

typedef struct Dm1V1MirrorCandidateChestCloseLeaderHandPickupEvidencePc34Compat {
    const char *chestOpenAnchor;
    const char *chestCloseAnchor;
    const char *leaderHandPutAnchor;
    const char *leaderHandRemoveAnchor;
    const char *slotRemoveAddDispatchAnchor;
    const char *championStateDrawAnchor;
    const char *candidateDispatchGuardAnchor;
    const char *candidateClearAnchor;
    const char *candidateFullChainClearAnchor;
    const char *defsAnchor;
    const char *blitRoutingAnchor;
    const char *contractScope;
} Dm1V1MirrorCandidateChestCloseLeaderHandPickupEvidencePc34Compat;

typedef struct Dm1V1MirrorCandidateChestCloseLeaderHandPickupContextPc34Compat {
    int chestAOpenThing;
    int chestBThing;
    int leaderHandThing;
    unsigned int leaderHandOwnerOrdinal;
    unsigned int candidateChampionOrdinal;
    unsigned int inventoryChampionOrdinal;
    unsigned int partyChampionCount;
    unsigned int partyRoster[
        DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_PARTY_CAP_PC34_COMPAT];
    int candidateCurrentHealth;
    int g0425[
        DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_SLOT_COUNT_PC34_COMPAT];
    int chestBLink[
        DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_SLOT_COUNT_PC34_COMPAT];
    int closeVisitOrder[
        DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_SLOT_COUNT_PC34_COMPAT];
} Dm1V1MirrorCandidateChestCloseLeaderHandPickupContextPc34Compat;

typedef struct Dm1V1MirrorCandidateChestCloseLeaderHandPickupActionLogPc34Compat {
    int chestBOpen;
    int leaderHandPickupFromG0425;
    int pickupSlotIndex;
    int candidateConfirmBeforeClose;
    int candidateConfirmAfterClose;
    int chestCloseMidPickup;
    int candidateClear;
    int candidateFullChainClear;
    int chestReopen;
    int inventoryCandidateGuardHonored;
    int objectRemovedFromSlotCount;
    int objectRemovedFromLeaderHandCount;
    int objectPutInLeaderHandCount;
    int objectAddedToSlotCount;
    int f0292ChampionStateDrawCount;
    int f0133BlitRouteCount;
    int f0333OpenCount;
    int f0334CloseCount;
    int f0280CandidateClearCount;
    int f0282CandidateFullClearCount;
    int c040DispatchGuardCount;
    int noDoubleCandidateClear;
    int noF0300F0301SequenceReversal;
    int emptyPickupNoop;
    int pickupCompleted;
    int closeVisitCount;
    int closeLinkCount;
} Dm1V1MirrorCandidateChestCloseLeaderHandPickupActionLogPc34Compat;

typedef struct Dm1V1MirrorCandidateChestCloseLeaderHandPickupFinalStatePc34Compat {
    int chestBLink[
        DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_SLOT_COUNT_PC34_COMPAT];
    int chestBLinkCount;
    int leaderHandThingAfter;
    unsigned int leaderHandOwnerOrdinalAfter;
    unsigned int candidateChampionOrdinalAfter;
    unsigned int inventoryChampionOrdinalAfter;
    unsigned int partyChampionCountAfter;
    int candidateCurrentHealthAfter;
    int candidatePanelOpenAfter;
    int g0425[
        DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_LEADER_HAND_PICKUP_SLOT_COUNT_PC34_COMPAT];
    int g0426After;
    int visibleSlotOrderPreserved;
    int candidateStillPending;
    int candidateCleared;
    int pickedThingRemovedFromG0425;
    int swapIdentityPreserved;
    int guardHonoredForInventoryCandidate;
    int noDoubleClear;
} Dm1V1MirrorCandidateChestCloseLeaderHandPickupFinalStatePc34Compat;

typedef struct Dm1V1MirrorCandidateChestCloseLeaderHandPickupResultPc34Compat {
    const Dm1V1MirrorCandidateChestCloseLeaderHandPickupEvidencePc34Compat
        *evidence;
    int caseId;
    Dm1V1MirrorCandidateChestCloseLeaderHandPickupContextPc34Compat before;
    Dm1V1MirrorCandidateChestCloseLeaderHandPickupActionLogPc34Compat log;
    Dm1V1MirrorCandidateChestCloseLeaderHandPickupFinalStatePc34Compat final;
    int pickedThing;
    int swappedInThing;
    int accepted;
} Dm1V1MirrorCandidateChestCloseLeaderHandPickupResultPc34Compat;

void M11_GameView_MirrorCandidateChestCloseLeaderHandPickup_InitContextPc34Compat(
    Dm1V1MirrorCandidateChestCloseLeaderHandPickupContextPc34Compat *context);

int M11_GameView_MirrorCandidateChestCloseLeaderHandPickup_RunCasePc34Compat(
    int caseId,
    Dm1V1MirrorCandidateChestCloseLeaderHandPickupResultPc34Compat *outResult);

const Dm1V1MirrorCandidateChestCloseLeaderHandPickupEvidencePc34Compat *
M11_GameView_MirrorCandidateChestCloseLeaderHandPickup_EvidencePc34Compat(void);

int dm1_v1_mirror_candidate_chest_close_leader_hand_pickup_pc34_compat_run(
    int *passed,
    int *failed);

#ifdef __cplusplus
}
#endif

#endif

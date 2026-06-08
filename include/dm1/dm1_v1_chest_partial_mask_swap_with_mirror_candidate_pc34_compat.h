#ifndef FIRESTAFF_DM1_V1_CHEST_PARTIAL_MASK_SWAP_WITH_MIRROR_CANDIDATE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_PARTIAL_MASK_SWAP_WITH_MIRROR_CANDIDATE_PC34_COMPAT_H

#include "dm1_v1_inventory_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_V1_CPSWMC_SLOT_COUNT_PC34 = DM1_PC34_CHEST_SLOT_COUNT,
    DM1_V1_CPSWMC_NONE_PC34 = -1,
    DM1_V1_CPSWMC_PANEL_M568_C040_PC34 = 568,
    DM1_V1_CPSWMC_COMMAND_CONFIRM_PC34 = 160,
    DM1_V1_CPSWMC_COMMAND_CANCEL_PC34 = 162,
    DM1_V1_CPSWMC_C38_CHEST_SLOT_1_PC34 = 38,
    DM1_V1_CPSWMC_CHEST_THING_PC34 = 0x7E20,
    DM1_V1_CPSWMC_LEADER_ITEM_PC34 = 0x5400,
    DM1_V1_CPSWMC_SLOT0_ITEM_PC34 = 0x6100,
    DM1_V1_CPSWMC_CANDIDATE_ORDINAL_PC34 = 2,
    DM1_V1_CPSWMC_INVENTORY_ORDINAL_PC34 = 1,
    DM1_V1_CPSWMC_TARGET_SLOT_INDEX_PC34 = 3,
    DM1_V1_CPSWMC_TARGET_PC34_SLOT_PC34 = DM1_PC34_SLOT_CHEST_4,
    DM1_V1_CPSWMC_PARTIAL_ALLOWED_MASK_PC34 =
        DM1_PC34_ALLOWED_CONTAINER | DM1_PC34_ALLOWED_HANDS,
    DM1_V1_CPSWMC_PARTY_COUNT_WITH_CANDIDATE_PC34 = 2
};

typedef enum Dm1V1ChestPartialMaskSwapWithMirrorCandidateCasePc34 {
    DM1_V1_CPSWMC_CASE_CONFIRM_PC34 = 1,
    DM1_V1_CPSWMC_CASE_CANCEL_PC34 = 2,
    DM1_V1_CPSWMC_CASE_NO_CANDIDATE_PC34 = 3,
    DM1_V1_CPSWMC_CASE_CROSS_CHAMPION_PC34 = 4,
    DM1_V1_CPSWMC_CASE_EMPTY_HAND_CANDIDATE_PC34 = 5,
    DM1_V1_CPSWMC_CASE_CLOSED_CHEST_CANDIDATE_PC34 = 6
} Dm1V1ChestPartialMaskSwapWithMirrorCandidateCasePc34;

typedef struct Dm1V1ChestPartialMaskSwapWithMirrorCandidateEvidencePc34 {
    const char *chestOpenAnchor;
    const char *chestCloseAnchor;
    const char *leaderHandAnchor;
    const char *chestSlotRemoveAnchor;
    const char *chestSlotAddAnchor;
    const char *encumbranceAnchor;
    const char *commandPanelGateAnchor;
    const char *championStateRedrawAnchor;
    const char *candidateClearAnchor;
    const char *objectRefreshAnchor;
    const char *partialMaskAnchor;
    const char *contractScope;
} Dm1V1ChestPartialMaskSwapWithMirrorCandidateEvidencePc34;

typedef struct Dm1V1ChestPartialMaskSwapWithMirrorCandidateStatePc34 {
    int panelContent;
    int c040PanelOpen;
    unsigned int candidateChampionOrdinal;
    unsigned int inventoryChampionOrdinal;
    unsigned int partyChampionCount;
    int leaderIndex;
    int selectedChampionIndex;
    int leaderHandThing;
    int leaderHandAllowedMask;
    int leaderHandWeight;
    int openChestThing;
    int g0425[DM1_V1_CPSWMC_SLOT_COUNT_PC34];
    int g0425AllowedMasks[DM1_V1_CPSWMC_SLOT_COUNT_PC34];
    int g0425Weights[DM1_V1_CPSWMC_SLOT_COUNT_PC34];
    int championLoads[4];
    int f0359PanelGateCount;
    int f0133PartialMaskDispatchCount;
    int f0302DispatchCount;
    int f0302SwapCount;
    int f0297LeaderHandPutCount;
    int f0298LeaderHandRemoveCount;
    int f0300ChestSlotRemoveCount;
    int f0301ChestSlotAddCount;
    int f0302EncumbranceRefreshCount;
    int f0334CloseCount;
    int g0425RecompactCount;
    int candidateClearCount;
    int f0293RedrawAllCount;
    int f0292DrawStateCount;
    int objectPointerRefreshCount;
    int objectNameRefreshCount;
} Dm1V1ChestPartialMaskSwapWithMirrorCandidateStatePc34;

typedef struct Dm1V1ChestPartialMaskSwapWithMirrorCandidateResultPc34 {
    const Dm1V1ChestPartialMaskSwapWithMirrorCandidateEvidencePc34 *evidence;
    int caseId;
    int command;
    int requestedPc34Slot;
    int requestedChestSlotIndex;
    int accepted;
    int rejected;
    int candidateOwnedInput;
    int candidateWasActive;
    int chestWasOpen;
    int partialMaskDispatched;
    int maskOverlap;
    int maskExactMatch;
    int slotBefore;
    int slotAfter;
    int leaderHandBefore;
    int leaderHandAfter;
    int openChestBefore;
    int openChestAfter;
    unsigned int candidateBefore;
    unsigned int candidateAfter;
    unsigned int partyCountBefore;
    unsigned int partyCountAfter;
    int selectedChampionBefore;
    int selectedChampionAfter;
    int selectedLoadBefore;
    int selectedLoadAfter;
    int leaderLoadBefore;
    int leaderLoadAfter;
    int f0359PanelGateDelta;
    int f0133PartialMaskDispatchDelta;
    int f0302DispatchDelta;
    int f0302SwapDelta;
    int f0297LeaderHandPutDelta;
    int f0298LeaderHandRemoveDelta;
    int f0300ChestSlotRemoveDelta;
    int f0301ChestSlotAddDelta;
    int f0302EncumbranceRefreshDelta;
    int f0334CloseDelta;
    int g0425RecompactDelta;
    int candidateClearDelta;
    int f0293RedrawAllDelta;
    int f0292DrawStateDelta;
    int objectPointerRefreshDelta;
    int objectNameRefreshDelta;
    int g0425Unchanged;
    int leaderHandPreserved;
    int noSideEffects;
    int ordinaryRoute;
    int crossChampionRoute;
    int closedChainCount;
    int closedChain[DM1_V1_CPSWMC_SLOT_COUNT_PC34];
} Dm1V1ChestPartialMaskSwapWithMirrorCandidateResultPc34;

void dm1_v1_chest_partial_mask_swap_with_mirror_candidate_init_pc34(
    Dm1V1ChestPartialMaskSwapWithMirrorCandidateStatePc34 *state);

int dm1_v1_chest_partial_mask_swap_with_mirror_candidate_run_case_pc34(
    int caseId,
    Dm1V1ChestPartialMaskSwapWithMirrorCandidateResultPc34 *outResult);

const Dm1V1ChestPartialMaskSwapWithMirrorCandidateEvidencePc34 *
dm1_v1_chest_partial_mask_swap_with_mirror_candidate_evidence_pc34(void);

int dm1_v1_chest_partial_mask_swap_with_mirror_candidate_run_pc34(
    int *passed,
    int *failed);

#ifdef __cplusplus
}
#endif

#endif

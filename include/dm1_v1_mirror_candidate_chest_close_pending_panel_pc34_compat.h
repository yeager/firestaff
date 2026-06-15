#ifndef DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_PENDING_PANEL_PC34_COMPAT_H
#define DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_PENDING_PANEL_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_PENDING_PANEL_SLOT_COUNT_PC34_COMPAT 8
#define DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_PENDING_PANEL_NONE_PC34_COMPAT (-1)

#define DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_PENDING_PANEL_C011_CLOSE_INVENTORY_PC34_COMPAT 11
#define DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_PENDING_PANEL_C038_CHEST_SLOT_1_PC34_COMPAT 38
#define DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_PENDING_PANEL_C162_CANCEL_PC34_COMPAT 162
#define DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_PENDING_PANEL_M568_C040_PC34_COMPAT 568
#define DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_PENDING_PANEL_M569_CHEST_PC34_COMPAT 569
#define DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_PENDING_PANEL_OPEN_CHEST_PC34_COMPAT 0x0700
#define DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_PENDING_PANEL_LEADER_HAND_PC34_COMPAT 0x0444
#define DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_PENDING_PANEL_SLOT0_THING_PC34_COMPAT 0x0101
#define DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_PENDING_PANEL_SLOT2_THING_PC34_COMPAT 0x0102
#define DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_PENDING_PANEL_SLOT3_THING_PC34_COMPAT 0x0103

typedef struct Dm1V1MirrorCandidateChestClosePendingPanelEvidencePc34Compat {
    const char *commandInventoryGuardAnchor;
    const char *commandSlotDispatchAnchor;
    const char *championSlotBoxAnchor;
    const char *panelToggleCloseAnchor;
    const char *chestCloseAnchor;
    const char *reviveCancelAnchor;
    const char *contractScope;
    const char *nonOverlapNote;
} Dm1V1MirrorCandidateChestClosePendingPanelEvidencePc34Compat;

typedef struct Dm1V1MirrorCandidateChestClosePendingPanelStatePc34Compat {
    int panelContent;
    int c040PanelOpen;
    unsigned int candidateChampionOrdinal;
    unsigned int inventoryChampionOrdinal;
    unsigned int partyChampionCount;
    int leaderIndex;
    int leaderHandThing;
    int openChestThing;
    int containerHeadThing;
    int chestSlots[DM1_V1_MIRROR_CANDIDATE_CHEST_CLOSE_PENDING_PANEL_SLOT_COUNT_PC34_COMPAT];
    int f0355ToggleCount;
    int f0334CloseCount;
    int f0302SlotDispatchCount;
    int f0302SwapCount;
    int blockedInventoryCloseCount;
    int explicitC040CancelCount;
    int candidateClearCount;
    int partyDecrementCount;
    int chestSlotClearCount;
    int chestFirstSlotWriteCount;
    int chestRelinkCount;
} Dm1V1MirrorCandidateChestClosePendingPanelStatePc34Compat;

typedef struct Dm1V1MirrorCandidateChestClosePendingPanelResultPc34Compat {
    const Dm1V1MirrorCandidateChestClosePendingPanelEvidencePc34Compat *evidence;
    int command;
    int requestedChestSlotIndex;
    int accepted;
    int ignored;
    int blockedByCandidate;
    int dispatchedF0302;
    int dispatchedF0355;
    int dispatchedF0334;
    int explicitC040Cancel;
    int leaderHandSwapped;
    int openChestBefore;
    int openChestAfter;
    int containerHeadBefore;
    int containerHeadAfter;
    int slot0Before;
    int slot0After;
    int slot1Before;
    int slot1After;
    int slot2Before;
    int slot2After;
    int slot3Before;
    int slot3After;
    int leaderHandBefore;
    int leaderHandAfter;
    int panelContentBefore;
    int panelContentAfter;
    int c040OpenBefore;
    int c040OpenAfter;
    unsigned int candidateOrdinalBefore;
    unsigned int candidateOrdinalAfter;
    unsigned int inventoryOrdinalBefore;
    unsigned int inventoryOrdinalAfter;
    unsigned int partyCountBefore;
    unsigned int partyCountAfter;
    int f0355ToggleCountBefore;
    int f0355ToggleCountAfter;
    int f0334CloseCountBefore;
    int f0334CloseCountAfter;
    int f0302SlotDispatchCountBefore;
    int f0302SlotDispatchCountAfter;
    int f0302SwapCountBefore;
    int f0302SwapCountAfter;
    int blockedInventoryCloseCountBefore;
    int blockedInventoryCloseCountAfter;
    int explicitC040CancelCountBefore;
    int explicitC040CancelCountAfter;
    int candidateClearCountBefore;
    int candidateClearCountAfter;
    int partyDecrementCountBefore;
    int partyDecrementCountAfter;
    int chestSlotClearCountBefore;
    int chestSlotClearCountAfter;
    int chestFirstSlotWriteCountBefore;
    int chestFirstSlotWriteCountAfter;
    int chestRelinkCountBefore;
    int chestRelinkCountAfter;
    int chestOpenPreserved;
    int chestSlotsPreserved;
    int candidatePreserved;
    int panelPreserved;
    int inventoryPreserved;
    int noChestCloseSideEffects;
    int closeRepackedNonEmptySlots;
} Dm1V1MirrorCandidateChestClosePendingPanelResultPc34Compat;

void DM1_V1_MirrorCandidateChestClosePendingPanel_InitPc34Compat(
    Dm1V1MirrorCandidateChestClosePendingPanelStatePc34Compat *state);

int DM1_V1_MirrorCandidateChestClosePendingPanel_AttemptInventoryClosePc34Compat(
    Dm1V1MirrorCandidateChestClosePendingPanelStatePc34Compat *state,
    Dm1V1MirrorCandidateChestClosePendingPanelResultPc34Compat *outResult);

int DM1_V1_MirrorCandidateChestClosePendingPanel_SwapChestSlotPc34Compat(
    Dm1V1MirrorCandidateChestClosePendingPanelStatePc34Compat *state,
    int chestSlotIndex,
    Dm1V1MirrorCandidateChestClosePendingPanelResultPc34Compat *outResult);

int DM1_V1_MirrorCandidateChestClosePendingPanel_CancelC040Pc34Compat(
    Dm1V1MirrorCandidateChestClosePendingPanelStatePc34Compat *state,
    Dm1V1MirrorCandidateChestClosePendingPanelResultPc34Compat *outResult);

const Dm1V1MirrorCandidateChestClosePendingPanelEvidencePc34Compat *
DM1_V1_MirrorCandidateChestClosePendingPanel_EvidencePc34Compat(void);

int dm1_v1_mirror_candidate_chest_close_pending_panel_run(
    int *passed,
    int *failed);

#ifdef __cplusplus
}
#endif

#endif

#ifndef DM1_V1_MIRROR_CANDIDATE_RESURRECT_RESELECT_WITH_INVENTORY_PICKUP_PC34_COMPAT_H
#define DM1_V1_MIRROR_CANDIDATE_RESURRECT_RESELECT_WITH_INVENTORY_PICKUP_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_MIRROR_CANDIDATE_RRIP_SLOT_COUNT_PC34_COMPAT 8
#define DM1_V1_MIRROR_CANDIDATE_RRIP_NONE_PC34_COMPAT (-1)
#define DM1_V1_MIRROR_CANDIDATE_RRIP_C038_SCROLL_PICKUP_PC34_COMPAT 38
#define DM1_V1_MIRROR_CANDIDATE_RRIP_C040_GRAPHIC_PC34_COMPAT 40
#define DM1_V1_MIRROR_CANDIDATE_RRIP_C160_RESURRECT_PC34_COMPAT 160
#define DM1_V1_MIRROR_CANDIDATE_RRIP_C162_CANCEL_PC34_COMPAT 162
#define DM1_V1_MIRROR_CANDIDATE_RRIP_C30_CHEST_SLOT_PC34_COMPAT 30
#define DM1_V1_MIRROR_CANDIDATE_RRIP_M568_PANEL_PC34_COMPAT 568

typedef enum Dm1V1MirrorCandidateRripFinishPc34Compat {
    DM1_V1_MIRROR_CANDIDATE_RRIP_FINISH_CONFIRM_PC34_COMPAT = 1,
    DM1_V1_MIRROR_CANDIDATE_RRIP_FINISH_CANCEL_PC34_COMPAT = 2
} Dm1V1MirrorCandidateRripFinishPc34Compat;

typedef struct Dm1V1MirrorCandidateRripEvidencePc34Compat {
    int contractOnly;
    const char *championDirectionAnchor;
    const char *championLeaderHandAnchor;
    const char *championSlotBoxAnchor;
    const char *reviveOpenAnchor;
    const char *reviveFinishAnchor;
    const char *panelDrawAnchor;
    const char *commandPanelAnchor;
    const char *commandQueueAnchor;
    const char *chestOpenAnchor;
    const char *chestCloseAnchor;
    const char *defsAnchor;
    const char *nonDuplicationScope;
} Dm1V1MirrorCandidateRripEvidencePc34Compat;

typedef struct Dm1V1MirrorCandidateRripStatePc34Compat {
    int partyChampionCount;
    int selectedChampionOrdinal;
    int candidateChampionOrdinal;
    int inventoryChampionOrdinal;
    int panelContent;
    int panelGraphic;
    int c040PanelOpen;
    int leaderEmptyHanded;
    int leaderHandThing;
    int sourceC30Thing;
    int chestSlot0Thing;
    int queuedInventoryCommand;
    int queuedInventorySlot;
    int queuedInventoryThing;
    int queuedWhileCandidateAlive;
    int refusedDuringCandidateAlive;
    int dispatchAfterCandidateFinished;
    int reselectCount;
    int reselectReissueCount;
    int resurrectRouteCount;
    int cancelRouteCount;
    int f0280CandidateOpenCount;
    int f0282FinishCount;
    int f0297PutLeaderHandCount;
    int f0302SlotBoxCount;
    int f0333OpenChestCount;
    int f0334CloseChestCount;
    int f0344PanelBarCount;
    int f0345PanelFoodWaterCount;
    int f0346DrawC040Count;
    int f0347DrawPanelCount;
    int f0359PanelDispatchCount;
    int f0380QueueDrainCount;
    int blockedInventoryClicks;
    int handPreservedCount;
    int deterministicHash;
} Dm1V1MirrorCandidateRripStatePc34Compat;

typedef struct Dm1V1MirrorCandidateRripResultPc34Compat {
    int accepted;
    int blocked;
    int queued;
    int dispatched;
    int candidateBefore;
    int candidateAfter;
    int selectedBefore;
    int selectedAfter;
    int sourceC30Before;
    int sourceC30After;
    int leaderHandBefore;
    int leaderHandAfter;
    int chestSlot0Before;
    int chestSlot0After;
    int queuedCommandBefore;
    int queuedCommandAfter;
    int queuedSlotBefore;
    int queuedSlotAfter;
    int f0359PanelDispatchBefore;
    int f0359PanelDispatchAfter;
    int f0380QueueDrainBefore;
    int f0380QueueDrainAfter;
    int blockedInventoryBefore;
    int blockedInventoryAfter;
    int candidateBoundToSelectedChampion;
    int handPreservedSourceC30;
    int queuedCommandPreserved;
    int dispatchWaitedForCandidateFinish;
    int deterministicHashAfter;
    const char *anchor;
} Dm1V1MirrorCandidateRripResultPc34Compat;

void DM1_V1_MirrorCandidateRrip_InitPc34Compat(
    Dm1V1MirrorCandidateRripStatePc34Compat *state);

int DM1_V1_MirrorCandidateRrip_ReselectSameChampionPc34Compat(
    Dm1V1MirrorCandidateRripStatePc34Compat *state,
    Dm1V1MirrorCandidateRripResultPc34Compat *outResult);

int DM1_V1_MirrorCandidateRrip_InventoryClickDuringReselectPc34Compat(
    Dm1V1MirrorCandidateRripStatePc34Compat *state,
    Dm1V1MirrorCandidateRripResultPc34Compat *outResult);

int DM1_V1_MirrorCandidateRrip_FinishCandidatePc34Compat(
    Dm1V1MirrorCandidateRripStatePc34Compat *state,
    Dm1V1MirrorCandidateRripFinishPc34Compat finish,
    Dm1V1MirrorCandidateRripResultPc34Compat *outResult);

const Dm1V1MirrorCandidateRripEvidencePc34Compat *
DM1_V1_MirrorCandidateRrip_EvidencePc34Compat(void);

const char *
DM1_V1_MirrorCandidateRrip_SourceEvidencePc34Compat(void);

int DM1_V1_MirrorCandidateRrip_RunSelfTestPc34Compat(void);
int DM1_V1_MirrorCandidateRrip_AssertionsPc34Compat(void);
int DM1_V1_MirrorCandidateRrip_FailuresPc34Compat(void);
int DM1_V1_MirrorCandidateRrip_DeterministicHashPc34Compat(void);

#ifdef __cplusplus
}
#endif

#endif

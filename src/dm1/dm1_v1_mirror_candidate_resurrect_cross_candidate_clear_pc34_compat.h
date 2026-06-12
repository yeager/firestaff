#ifndef DM1_V1_MIRROR_CANDIDATE_RESURRECT_CROSS_CANDIDATE_CLEAR_PC34_COMPAT_H
#define DM1_V1_MIRROR_CANDIDATE_RESURRECT_CROSS_CANDIDATE_CLEAR_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_MIRROR_CANDIDATE_RCC_CHAMPION_COUNT_PC34_COMPAT 4
#define DM1_V1_MIRROR_CANDIDATE_RCC_SLOT_COUNT_PC34_COMPAT 30
#define DM1_V1_MIRROR_CANDIDATE_RCC_CHEST_SLOT_COUNT_PC34_COMPAT 8
#define DM1_V1_MIRROR_CANDIDATE_RCC_NONE_PC34_COMPAT (-1)
#define DM1_V1_MIRROR_CANDIDATE_RCC_C040_GRAPHIC_PC34_COMPAT 40
#define DM1_V1_MIRROR_CANDIDATE_RCC_M568_PANEL_PC34_COMPAT 568
#define DM1_V1_MIRROR_CANDIDATE_RCC_C160_RESURRECT_PC34_COMPAT 160
#define DM1_V1_MIRROR_CANDIDATE_RCC_C30_SLOT_CHEST_1_PC34_COMPAT 30

enum Dm1V1MirrorCandidateRccStepIdPc34Compat {
    DM1_V1_MIRROR_CANDIDATE_RCC_STEP_SEED_STALE_B_PANEL_PC34_COMPAT = 1,
    DM1_V1_MIRROR_CANDIDATE_RCC_STEP_CLOSE_STALE_B_PANEL_PC34_COMPAT = 2,
    DM1_V1_MIRROR_CANDIDATE_RCC_STEP_QUEUE_FRESH_A_PANEL_PC34_COMPAT = 3,
    DM1_V1_MIRROR_CANDIDATE_RCC_STEP_RESURRECT_A_PC34_COMPAT = 4,
    DM1_V1_MIRROR_CANDIDATE_RCC_STEP_SETTLE_PANEL_PC34_COMPAT = 5
};

typedef struct Dm1V1MirrorCandidateRccChampionPc34Compat {
    int present;
    int currentHealth;
    int maximumHealth;
    int deathCount;
    int resurrectCount;
    int candidateCloseCount;
    int cell;
    int direction;
    int portraitOrdinal;
    int redrawStateCount;
    int redrawPanelCount;
    int slots[DM1_V1_MIRROR_CANDIDATE_RCC_SLOT_COUNT_PC34_COMPAT];
} Dm1V1MirrorCandidateRccChampionPc34Compat;

typedef struct Dm1V1MirrorCandidateRccPanelPc34Compat {
    int panelOpen;
    int panelContent;
    int panelGraphic;
    int ownerChampionIndex;
    unsigned int candidateOrdinal;
    int drawGeneration;
    int closeGeneration;
    int closedWithoutResurrect;
    int chestFirstSlot;
    int chestSlotProbeCount;
} Dm1V1MirrorCandidateRccPanelPc34Compat;

typedef struct Dm1V1MirrorCandidateRccStatePc34Compat {
    int partyChampionCount;
    int leaderIndex;
    int leaderEmptyHanded;
    unsigned int candidateChampionOrdinal;
    unsigned int inventoryChampionOrdinal;
    int candidateOwnerChampionIndex;
    int activePanelChampionIndex;
    int stalePanelOwnerChampionIndex;
    unsigned int staleCandidateOrdinal;
    int staleCandidateFreed;
    int stalePanelClosedWithoutResurrect;
    int freshCandidateQueuedAfterStaleClose;
    int leaderOwnedFreshCandidateThroughout;
    int bPanelEverResurrected;
    int f0280CandidateSetCount;
    int f0282ResurrectRouteCount;
    int f0282CancelRouteCount;
    int f0297PutLeaderHandCount;
    int f0298RemoveLeaderHandCount;
    int f0300SlotClearCount;
    int f0301SlotWriteCount;
    int f0302SlotWritebackGuardCount;
    int f0333OpenChestRouteCount;
    int f0334CloseChestRewriteCount;
    int f0344FoodWaterBarRedrawCount;
    int f0345FoodWaterPanelRedrawCount;
    int f0346DrawC040PanelCount;
    int f0347DrawPanelDispatchCount;
    int f0359C040DispatchCount;
    int g0426OpenChestThing;
    int g0425ChestSlots[
        DM1_V1_MIRROR_CANDIDATE_RCC_CHEST_SLOT_COUNT_PC34_COMPAT];
    Dm1V1MirrorCandidateRccPanelPc34Compat panel;
    Dm1V1MirrorCandidateRccChampionPc34Compat
        champions[DM1_V1_MIRROR_CANDIDATE_RCC_CHAMPION_COUNT_PC34_COMPAT];
} Dm1V1MirrorCandidateRccStatePc34Compat;

typedef struct Dm1V1MirrorCandidateRccStepPc34Compat {
    int stepId;
    const char *name;
    const char *redmcsbAnchor;
    unsigned int candidateOrdinalBefore;
    unsigned int candidateOrdinalAfter;
    unsigned int inventoryOrdinalBefore;
    unsigned int inventoryOrdinalAfter;
    int panelOwnerBefore;
    int panelOwnerAfter;
    int panelOpenBefore;
    int panelOpenAfter;
    int leaderIndexBefore;
    int leaderIndexAfter;
    int healthBefore[
        DM1_V1_MIRROR_CANDIDATE_RCC_CHAMPION_COUNT_PC34_COMPAT];
    int healthAfter[
        DM1_V1_MIRROR_CANDIDATE_RCC_CHAMPION_COUNT_PC34_COMPAT];
    int routeF0280;
    int routeF0282;
    int routeF0297;
    int routeF0298;
    int routeF0300;
    int routeF0301;
    int routeF0302;
    int routeF0333;
    int routeF0334;
    int routeF0344;
    int routeF0345;
    int routeF0346;
    int routeF0347;
    int routeF0359;
    int staleBClosedWithoutResurrect;
    int freshAIsCurrentCandidate;
    int leaderOwnsFreshA;
    int bStillDead;
    int aResurrected;
} Dm1V1MirrorCandidateRccStepPc34Compat;

void DM1_V1_MirrorCandidateRcc_InitPc34Compat(
    Dm1V1MirrorCandidateRccStatePc34Compat *state);

int DM1_V1_MirrorCandidateRcc_SeedStaleBPanelPc34Compat(
    Dm1V1MirrorCandidateRccStatePc34Compat *state,
    Dm1V1MirrorCandidateRccStepPc34Compat *outStep);

int DM1_V1_MirrorCandidateRcc_CloseStaleBPanelPc34Compat(
    Dm1V1MirrorCandidateRccStatePc34Compat *state,
    Dm1V1MirrorCandidateRccStepPc34Compat *outStep);

int DM1_V1_MirrorCandidateRcc_QueueFreshAPanelPc34Compat(
    Dm1V1MirrorCandidateRccStatePc34Compat *state,
    Dm1V1MirrorCandidateRccStepPc34Compat *outStep);

int DM1_V1_MirrorCandidateRcc_ResurrectAPc34Compat(
    Dm1V1MirrorCandidateRccStatePc34Compat *state,
    Dm1V1MirrorCandidateRccStepPc34Compat *outStep);

int DM1_V1_MirrorCandidateRcc_SettlePanelPc34Compat(
    Dm1V1MirrorCandidateRccStatePc34Compat *state,
    Dm1V1MirrorCandidateRccStepPc34Compat *outStep);

int DM1_V1_MirrorCandidateRcc_DriveRegressionPc34Compat(
    Dm1V1MirrorCandidateRccStatePc34Compat *state,
    Dm1V1MirrorCandidateRccStepPc34Compat *steps,
    int stepCapacity);

const char *
DM1_V1_MirrorCandidateRcc_SourceEvidencePc34Compat(void);

#ifdef __cplusplus
}
#endif

#endif

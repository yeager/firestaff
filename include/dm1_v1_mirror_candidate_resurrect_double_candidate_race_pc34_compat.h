#ifndef DM1_V1_MIRROR_CANDIDATE_RESURRECT_DOUBLE_CANDIDATE_RACE_PC34_COMPAT_H
#define DM1_V1_MIRROR_CANDIDATE_RESURRECT_DOUBLE_CANDIDATE_RACE_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_MIRROR_CANDIDATE_RDCR_CHAMPION_COUNT_PC34_COMPAT 4
#define DM1_V1_MIRROR_CANDIDATE_RDCR_SLOT_COUNT_PC34_COMPAT 30
#define DM1_V1_MIRROR_CANDIDATE_RDCR_PENDING_CAPACITY_PC34_COMPAT 2
#define DM1_V1_MIRROR_CANDIDATE_RDCR_CHEST_SLOT_COUNT_PC34_COMPAT 8
#define DM1_V1_MIRROR_CANDIDATE_RDCR_NONE_PC34_COMPAT (-1)
#define DM1_V1_MIRROR_CANDIDATE_RDCR_C160_RESURRECT_PC34_COMPAT 160
#define DM1_V1_MIRROR_CANDIDATE_RDCR_C040_GRAPHIC_PC34_COMPAT 40
#define DM1_V1_MIRROR_CANDIDATE_RDCR_M568_PANEL_PC34_COMPAT 568
#define DM1_V1_MIRROR_CANDIDATE_RDCR_C30_SLOT_CHEST_1_PC34_COMPAT 30

enum Dm1V1MirrorCandidateRdcrStepIdPc34Compat {
    DM1_V1_MIRROR_CANDIDATE_RDCR_STEP_FIRST_DEATH_PC34_COMPAT = 1,
    DM1_V1_MIRROR_CANDIDATE_RDCR_STEP_FIRST_RESURRECT_PC34_COMPAT = 2,
    DM1_V1_MIRROR_CANDIDATE_RDCR_STEP_SECOND_DEATH_PC34_COMPAT = 3,
    DM1_V1_MIRROR_CANDIDATE_RDCR_STEP_SECOND_RESURRECT_PC34_COMPAT = 4,
    DM1_V1_MIRROR_CANDIDATE_RDCR_STEP_PANEL_DRAW_AFTER_RACE_PC34_COMPAT = 5
};

typedef struct Dm1V1MirrorCandidateRdcrChampionPc34Compat {
    int present;
    int currentHealth;
    int deathCount;
    int resurrectCount;
    int cell;
    int direction;
    int portraitOrdinal;
    int redrawStateCount;
    int redrawPanelCount;
    int slots[DM1_V1_MIRROR_CANDIDATE_RDCR_SLOT_COUNT_PC34_COMPAT];
} Dm1V1MirrorCandidateRdcrChampionPc34Compat;

typedef struct Dm1V1MirrorCandidateRdcrPanelPc34Compat {
    int panelOpen;
    int panelContent;
    int panelGraphic;
    int activeCandidateOrdinal;
    int drawnQueueCount;
    int drawnCandidateOrdinals[
        DM1_V1_MIRROR_CANDIDATE_RDCR_PENDING_CAPACITY_PC34_COMPAT];
    int lastRaceCandidateOrdinal;
    int drawGeneration;
    int chestFirstSlot;
    int chestSlotProbeCount;
} Dm1V1MirrorCandidateRdcrPanelPc34Compat;

typedef struct Dm1V1MirrorCandidateRdcrStatePc34Compat {
    int partyChampionCount;
    int leaderIndex;
    int leaderEmptyHanded;
    int leaderSwitchCount;
    int leaderDeathSwitchCount;
    int nonLeaderDeathPreservedLeaderCount;
    unsigned int candidateChampionOrdinal;
    unsigned int inventoryChampionOrdinal;
    int activePanelChampionIndex;
    int pendingCount;
    unsigned int pendingOrdinals[
        DM1_V1_MIRROR_CANDIDATE_RDCR_PENDING_CAPACITY_PC34_COMPAT];
    int consumedCount;
    unsigned int consumedOrdinals[
        DM1_V1_MIRROR_CANDIDATE_RDCR_PENDING_CAPACITY_PC34_COMPAT];
    int firstDeathChampionIndex;
    int secondDeathChampionIndex;
    int secondDeathRegisteredSeparatePending;
    int secondDeathSilentlyAbsorbed;
    int clobberedFirstResurrectedChampion;
    int clobberedSecondDeadChampion;
    int f0280CandidateSetCount;
    int f0282ResurrectRouteCount;
    int f0297PutLeaderHandCount;
    int f0298RemoveLeaderHandCount;
    int f0302SlotWritebackGuardCount;
    int f0334CloseChestRewriteCount;
    int f0344PanelRedrawCount;
    int f0345PanelRedrawCount;
    int f0346DrawC040PanelCount;
    int f0347DrawPanelDispatchCount;
    int f0359C040DispatchCount;
    int panelDrawsWithSecondCandidate;
    int panelDrawsAfterRace;
    int g0426OpenChestThing;
    int g0425ChestSlots[
        DM1_V1_MIRROR_CANDIDATE_RDCR_CHEST_SLOT_COUNT_PC34_COMPAT];
    Dm1V1MirrorCandidateRdcrPanelPc34Compat panel;
    Dm1V1MirrorCandidateRdcrChampionPc34Compat
        champions[DM1_V1_MIRROR_CANDIDATE_RDCR_CHAMPION_COUNT_PC34_COMPAT];
} Dm1V1MirrorCandidateRdcrStatePc34Compat;

typedef struct Dm1V1MirrorCandidateRdcrStepPc34Compat {
    int stepId;
    const char *name;
    const char *redmcsbAnchor;
    unsigned int candidateOrdinalBefore;
    unsigned int candidateOrdinalAfter;
    int pendingCountBefore;
    int pendingCountAfter;
    unsigned int pendingHeadOrdinalBefore;
    unsigned int pendingHeadOrdinalAfter;
    unsigned int pendingTailOrdinalAfter;
    int leaderIndexBefore;
    int leaderIndexAfter;
    int partyChampionCountBefore;
    int partyChampionCountAfter;
    int activePanelChampionBefore;
    int activePanelChampionAfter;
    int aliveCountBefore;
    int aliveCountAfter;
    int panelDrawGenerationBefore;
    int panelDrawGenerationAfter;
    int routeF0280;
    int routeF0282;
    int routeF0297;
    int routeF0298;
    int routeF0302;
    int routeF0334;
    int routeF0344;
    int routeF0345;
    int routeF0346;
    int routeF0347;
    int routeF0359;
    int firstResurrectedAlive;
    int secondDeadStillDistinct;
    int separatePendingCandidateRegistered;
    int leaderSwitchPreserved;
    int panelShowsPendingCandidate;
    int noChampionSlotDroppedOrMerged;
    int healthBefore[
        DM1_V1_MIRROR_CANDIDATE_RDCR_CHAMPION_COUNT_PC34_COMPAT];
    int healthAfter[
        DM1_V1_MIRROR_CANDIDATE_RDCR_CHAMPION_COUNT_PC34_COMPAT];
} Dm1V1MirrorCandidateRdcrStepPc34Compat;

void DM1_V1_MirrorCandidateRdcr_InitPc34Compat(
    Dm1V1MirrorCandidateRdcrStatePc34Compat *state);

int DM1_V1_MirrorCandidateRdcr_FirstDeathPc34Compat(
    Dm1V1MirrorCandidateRdcrStatePc34Compat *state,
    Dm1V1MirrorCandidateRdcrStepPc34Compat *outStep);

int DM1_V1_MirrorCandidateRdcr_FirstResurrectPc34Compat(
    Dm1V1MirrorCandidateRdcrStatePc34Compat *state,
    Dm1V1MirrorCandidateRdcrStepPc34Compat *outStep);

int DM1_V1_MirrorCandidateRdcr_SecondDeathPc34Compat(
    Dm1V1MirrorCandidateRdcrStatePc34Compat *state,
    Dm1V1MirrorCandidateRdcrStepPc34Compat *outStep);

int DM1_V1_MirrorCandidateRdcr_SecondResurrectPc34Compat(
    Dm1V1MirrorCandidateRdcrStatePc34Compat *state,
    Dm1V1MirrorCandidateRdcrStepPc34Compat *outStep);

int DM1_V1_MirrorCandidateRdcr_PanelDrawAfterRacePc34Compat(
    Dm1V1MirrorCandidateRdcrStatePc34Compat *state,
    Dm1V1MirrorCandidateRdcrStepPc34Compat *outStep);

int DM1_V1_MirrorCandidateRdcr_DriveRegressionPc34Compat(
    Dm1V1MirrorCandidateRdcrStatePc34Compat *state,
    Dm1V1MirrorCandidateRdcrStepPc34Compat *steps,
    int stepCapacity);

const char *
DM1_V1_MirrorCandidateRdcr_SourceEvidencePc34Compat(void);

#ifdef __cplusplus
}
#endif

#endif

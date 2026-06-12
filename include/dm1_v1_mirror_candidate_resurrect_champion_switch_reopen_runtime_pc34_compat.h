#ifndef DM1_V1_MIRROR_CANDIDATE_RESURRECT_CHAMPION_SWITCH_REOPEN_RUNTIME_PC34_COMPAT_H
#define DM1_V1_MIRROR_CANDIDATE_RESURRECT_CHAMPION_SWITCH_REOPEN_RUNTIME_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_MIRROR_CANDIDATE_RCSR_CHAMPION_COUNT_PC34_COMPAT 4
#define DM1_V1_MIRROR_CANDIDATE_RCSR_SLOT_COUNT_PC34_COMPAT 30
#define DM1_V1_MIRROR_CANDIDATE_RCSR_CHEST_SLOT_COUNT_PC34_COMPAT 8
#define DM1_V1_MIRROR_CANDIDATE_RCSR_NONE_PC34_COMPAT (-1)
#define DM1_V1_MIRROR_CANDIDATE_RCSR_C040_GRAPHIC_PC34_COMPAT 40
#define DM1_V1_MIRROR_CANDIDATE_RCSR_M568_PANEL_PC34_COMPAT 568
#define DM1_V1_MIRROR_CANDIDATE_RCSR_C160_RESURRECT_PC34_COMPAT 160
#define DM1_V1_MIRROR_CANDIDATE_RCSR_C30_SLOT_CHEST_1_PC34_COMPAT 30

typedef struct Dm1V1MirrorCandidateRcsrChampionPc34Compat {
    int present;
    int currentHealth;
    int cell;
    int direction;
    int portraitOrdinal;
    int attributes;
    int redrawStateCount;
    int redrawPanelChromeCount;
    int redrawSlotCount;
    int slots[DM1_V1_MIRROR_CANDIDATE_RCSR_SLOT_COUNT_PC34_COMPAT];
} Dm1V1MirrorCandidateRcsrChampionPc34Compat;

typedef struct Dm1V1MirrorCandidateRcsrPanelChromePc34Compat {
    int panelContent;
    int panelGraphic;
    int panelBoxLeft;
    int panelBoxTop;
    int panelBoxRight;
    int panelBoxBottom;
    int panelByteWidth;
    int transparentColor;
    int chestFirstSlot;
    int chestSlotProbeCount;
} Dm1V1MirrorCandidateRcsrPanelChromePc34Compat;

typedef struct Dm1V1MirrorCandidateRcsrStatePc34Compat {
    int partyChampionCount;
    int partyDirection;
    int leaderEmptyHanded;
    unsigned int candidateChampionOrdinal;
    unsigned int inventoryChampionOrdinal;
    int activePanelChampionIndex;
    int originalCandidateChampionIndex;
    unsigned int originalCandidateChampionOrdinal;
    int leaderIndex;
    int f0280CandidateSetCount;
    int f0282RouteCount;
    int f0282ClearSkippedForHandIconCount;
    int f0284SetPartyDirectionCount;
    int f0291DrawSlotCount;
    int f0293DrawAllChampionStatesCount;
    int f0296DrawChangedObjectIconsCount;
    int f0346DrawC040PanelCount;
    int f0354ChampionSwitchCount;
    int f0355InventoryToggleCount;
    int f0359C040DispatchCount;
    int resurrectHandIconClickCount;
    int preservedAcrossSwitch;
    int reopenedDifferentChampion;
    int g0426OpenChestThing;
    int g0425ChestSlots[DM1_V1_MIRROR_CANDIDATE_RCSR_CHEST_SLOT_COUNT_PC34_COMPAT];
    Dm1V1MirrorCandidateRcsrPanelChromePc34Compat firstPanelChrome;
    Dm1V1MirrorCandidateRcsrPanelChromePc34Compat reopenedPanelChrome;
    Dm1V1MirrorCandidateRcsrChampionPc34Compat
        champions[DM1_V1_MIRROR_CANDIDATE_RCSR_CHAMPION_COUNT_PC34_COMPAT];
} Dm1V1MirrorCandidateRcsrStatePc34Compat;

typedef struct Dm1V1MirrorCandidateRcsrStepPc34Compat {
    int stepId;
    const char *name;
    const char *redmcsbAnchor;
    unsigned int candidateOrdinalBefore;
    unsigned int candidateOrdinalAfter;
    unsigned int inventoryOrdinalBefore;
    unsigned int inventoryOrdinalAfter;
    int panelChampionBefore;
    int panelChampionAfter;
    int routeF0282;
    int routeF0284;
    int routeF0293;
    int routeF0354;
    int g0299Preserved;
    int panelChromeStable;
} Dm1V1MirrorCandidateRcsrStepPc34Compat;

void DM1_V1_MirrorCandidateRcsr_InitPc34Compat(
    Dm1V1MirrorCandidateRcsrStatePc34Compat *state);

int DM1_V1_MirrorCandidateRcsr_SelectCandidatePc34Compat(
    Dm1V1MirrorCandidateRcsrStatePc34Compat *state,
    int championIndex,
    Dm1V1MirrorCandidateRcsrStepPc34Compat *outStep);

int DM1_V1_MirrorCandidateRcsr_ClickResurrectHandIconPc34Compat(
    Dm1V1MirrorCandidateRcsrStatePc34Compat *state,
    int championIndex,
    Dm1V1MirrorCandidateRcsrStepPc34Compat *outStep);

int DM1_V1_MirrorCandidateRcsr_SwitchInventoryChampionPc34Compat(
    Dm1V1MirrorCandidateRcsrStatePc34Compat *state,
    int championIndex,
    int newPartyDirection,
    Dm1V1MirrorCandidateRcsrStepPc34Compat *outStep);

int DM1_V1_MirrorCandidateRcsr_DriveRegressionPc34Compat(
    Dm1V1MirrorCandidateRcsrStatePc34Compat *state,
    Dm1V1MirrorCandidateRcsrStepPc34Compat *steps,
    int stepCapacity);

const char *
DM1_V1_MirrorCandidateRcsr_SourceEvidencePc34Compat(void);

#ifdef __cplusplus
}
#endif

#endif

#ifndef DM1_V1_MIRROR_CANDIDATE_ICON_REFRESH_PC34_COMPAT_H
#define DM1_V1_MIRROR_CANDIDATE_ICON_REFRESH_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_MIRROR_CANDIDATE_ICON_REFRESH_CHAMPION_COUNT_PC34_COMPAT 4
#define DM1_V1_MIRROR_CANDIDATE_ICON_REFRESH_HAND_SLOTS_PC34_COMPAT 2
#define DM1_V1_MIRROR_CANDIDATE_ICON_REFRESH_INVENTORY_SLOTS_PC34_COMPAT 8
#define DM1_V1_MIRROR_CANDIDATE_ICON_REFRESH_CHEST_SLOTS_PC34_COMPAT 8

typedef struct DM1_V1_MirrorCandidateIconRefreshEvidencePc34Compat {
    const char *chamdrawIconProbeAnchor;
    const char *chamdrawRefreshAnchor;
    const char *championPartyLoopAnchor;
    const char *commandCandidateGateAnchor;
    const char *defsPrototypeAnchor;
    const char *contractScope;
    const char *disjointFunctions;
} DM1_V1_MirrorCandidateIconRefreshEvidencePc34Compat;

typedef struct DM1_V1_MirrorCandidateIconSlotPc34Compat {
    int currentIcon;
    int objectIcon;
    int changed;
} DM1_V1_MirrorCandidateIconSlotPc34Compat;

typedef struct DM1_V1_MirrorCandidateIconRefreshStatePc34Compat {
    unsigned int candidateChampionOrdinal;
    unsigned int inventoryChampionOrdinal;
    int partyChampionCount;
    int c040PanelOpen;
    int leaderIndex;
    int leaderHandThingOrdinal;
    int leaderHandCurrentIcon;
    int leaderHandObjectIcon;
    int leaderHandPointerIcon;
    int mousePointerHiddenForChangedIcon;
    int mouseScreenUpdatePairs;
    int leaderHandIconRefreshCount;
    int leaderHandNameDrawCount;
    int partyStatusSlotRefreshCount;
    int partyActionIconDrawCount;
    int inventorySlotRefreshCount;
    int chestSlotRefreshCount;
    int viewportDrawCount;
    int earlyReturnCount;
    int partyLoopVisits;
    int inventoryLoopVisits;
    int chestLoopVisits;
    int candidateOrdinalClearedCount;
    int panelClearedCount;
    int commandQueueMutationCount;
    DM1_V1_MirrorCandidateIconSlotPc34Compat partyHandSlots
        [DM1_V1_MIRROR_CANDIDATE_ICON_REFRESH_CHAMPION_COUNT_PC34_COMPAT]
        [DM1_V1_MIRROR_CANDIDATE_ICON_REFRESH_HAND_SLOTS_PC34_COMPAT];
    DM1_V1_MirrorCandidateIconSlotPc34Compat inventorySlots
        [DM1_V1_MIRROR_CANDIDATE_ICON_REFRESH_INVENTORY_SLOTS_PC34_COMPAT];
    DM1_V1_MirrorCandidateIconSlotPc34Compat chestSlots
        [DM1_V1_MIRROR_CANDIDATE_ICON_REFRESH_CHEST_SLOTS_PC34_COMPAT];
} DM1_V1_MirrorCandidateIconRefreshStatePc34Compat;

typedef struct DM1_V1_MirrorCandidateIconRefreshResultPc34Compat {
    const DM1_V1_MirrorCandidateIconRefreshEvidencePc34Compat *evidence;
    int contractOnly;
    int suppressedByCandidateWithoutInventory;
    int processedWithInventoryOpen;
    int candidateOrdinalBefore;
    int candidateOrdinalAfter;
    int inventoryOrdinalBefore;
    int inventoryOrdinalAfter;
    int c040PanelBefore;
    int c040PanelAfter;
    int leaderHandCurrentIconBefore;
    int leaderHandCurrentIconAfter;
    int leaderHandPointerIconBefore;
    int leaderHandPointerIconAfter;
    int leaderHandNameDrawCountBefore;
    int leaderHandNameDrawCountAfter;
    int mousePointerHiddenBefore;
    int mousePointerHiddenAfter;
    int mouseScreenUpdatePairsBefore;
    int mouseScreenUpdatePairsAfter;
    int partyStatusSlotRefreshCountBefore;
    int partyStatusSlotRefreshCountAfter;
    int partyActionIconDrawCountBefore;
    int partyActionIconDrawCountAfter;
    int inventorySlotRefreshCountBefore;
    int inventorySlotRefreshCountAfter;
    int chestSlotRefreshCountBefore;
    int chestSlotRefreshCountAfter;
    int viewportDrawCountBefore;
    int viewportDrawCountAfter;
    int earlyReturnCountBefore;
    int earlyReturnCountAfter;
    int partyLoopVisitsBefore;
    int partyLoopVisitsAfter;
    int inventoryLoopVisitsBefore;
    int inventoryLoopVisitsAfter;
    int chestLoopVisitsBefore;
    int chestLoopVisitsAfter;
    int candidateOrdinalClearedCountBefore;
    int candidateOrdinalClearedCountAfter;
    int panelClearedCountBefore;
    int panelClearedCountAfter;
    int commandQueueMutationCountBefore;
    int commandQueueMutationCountAfter;
    int mutableIconLowRange;
    int mutableIconWeaponBoundaryRejected;
    int mutableIconPotionRange;
    int mutableIconEmptyFlask;
    int immutableIconRejected;
} DM1_V1_MirrorCandidateIconRefreshResultPc34Compat;

typedef struct DM1_V1_MirrorCandidateIconRefreshProbePc34Compat {
    const DM1_V1_MirrorCandidateIconRefreshEvidencePc34Compat *evidence;
    DM1_V1_MirrorCandidateIconRefreshResultPc34Compat suppressed;
    DM1_V1_MirrorCandidateIconRefreshResultPc34Compat inventoryOpen;
} DM1_V1_MirrorCandidateIconRefreshProbePc34Compat;

typedef DM1_V1_MirrorCandidateIconRefreshEvidencePc34Compat
    Dm1V1MirrorCandidateIconRefreshEvidencePc34Compat;
typedef DM1_V1_MirrorCandidateIconSlotPc34Compat
    Dm1V1MirrorCandidateIconSlotPc34Compat;
typedef DM1_V1_MirrorCandidateIconRefreshStatePc34Compat
    Dm1V1MirrorCandidateIconRefreshStatePc34Compat;
typedef DM1_V1_MirrorCandidateIconRefreshResultPc34Compat
    Dm1V1MirrorCandidateIconRefreshResultPc34Compat;
typedef DM1_V1_MirrorCandidateIconRefreshProbePc34Compat
    Dm1V1MirrorCandidateIconRefreshProbePc34Compat;

void DM1_V1_MirrorCandidateIconRefresh_InitSuppressedPc34Compat(
    DM1_V1_MirrorCandidateIconRefreshStatePc34Compat *state);

void DM1_V1_MirrorCandidateIconRefresh_InitInventoryOpenPc34Compat(
    DM1_V1_MirrorCandidateIconRefreshStatePc34Compat *state);

int DM1_V1_MirrorCandidateIconRefresh_DrawChangedObjectIconsPc34Compat(
    DM1_V1_MirrorCandidateIconRefreshStatePc34Compat *state,
    DM1_V1_MirrorCandidateIconRefreshResultPc34Compat *outResult);

DM1_V1_MirrorCandidateIconRefreshProbePc34Compat
DM1_V1_MirrorCandidateIconRefresh_ProbePc34Compat(void);

const DM1_V1_MirrorCandidateIconRefreshEvidencePc34Compat *
DM1_V1_MirrorCandidateIconRefresh_EvidencePc34Compat(void);

#ifdef __cplusplus
}
#endif

#endif

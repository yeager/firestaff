#ifndef DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_GUARD_PC34_COMPAT_H
#define DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_GUARD_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_GUARD_SLOT_COUNT_PC34 4
#define DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_GUARD_NONE_PC34 (-1)
#define DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_GUARD_M568_C040_PC34 568
#define DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_GUARD_M569_CHEST_PC34 569
#define DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_GUARD_C040_SLOT_PC34 40
#define DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_GUARD_C162_CLOSE_PC34 162
#define DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_GUARD_C030_CHEST_SLOT_PC34 30

typedef enum Dm1V1MirrorCandidateDoubleOpenCloseGuardEventKindPc34Compat {
    DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_GUARD_EVENT_C040_OPEN_PC34 = 1,
    DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_GUARD_EVENT_C040_CLOSE_PC34 = 2,
    DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_GUARD_EVENT_CHEST_CLOSE_PENDING_PC34 = 3,
    DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_GUARD_EVENT_INVENTORY_PORTRAIT_CLICK_PC34 = 4
} Dm1V1MirrorCandidateDoubleOpenCloseGuardEventKindPc34Compat;

typedef struct Dm1V1MirrorCandidateDoubleOpenCloseGuardEventPc34Compat {
    int kind;
    int tick;
    unsigned int candidateChampionOrdinal;
    int requestedSlotIndex;
} Dm1V1MirrorCandidateDoubleOpenCloseGuardEventPc34Compat;

typedef struct Dm1V1MirrorCandidateDoubleOpenCloseGuardEvidencePc34Compat {
    int contractOnly;
    const char *nonDuplicateScope;
    const char *chamdrawPanelOpenAnchor;
    const char *chamdrawPanelCloseAnchor;
    const char *championLeaderHandPutAnchor;
    const char *championOccupiedSlotClickAnchor;
    const char *commandPanelDispatchAnchor;
    const char *reviveCandidateClearAnchor;
    const char *defsAnchor;
} Dm1V1MirrorCandidateDoubleOpenCloseGuardEvidencePc34Compat;

typedef struct Dm1V1MirrorCandidateDoubleOpenCloseGuardStatePc34Compat {
    int contractOnly;
    int panelContent;
    int c040PanelOpen;
    unsigned int candidateChampionOrdinal;
    unsigned int inventoryChampionOrdinal;
    unsigned int partyChampionCount;
    int leaderHandThing;
    int leaderHandQueueThing;
    int openChestThing;
    int chestSlots[DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_GUARD_SLOT_COUNT_PC34];
    int pendingOpenArmed;
    unsigned int pendingCandidateChampionOrdinal;
    int openDispatchCount;
    int closeDispatchCount;
    int duplicateOpenNoopCount;
    int duplicateCloseNoopCount;
    int f0282CandidateClearCount;
    int f0297LeaderHandPutCount;
    int f0302SlotDispatchCount;
    int f0334ChestCloseCount;
    int f0355InventoryCloseCount;
    int panelZeroCount;
    int leaderHandQueueClearCount;
    int pendingOpenFlushCount;
    int inventoryPortraitClickCount;
    int sameTickCloseSlotOrderCount;
    int clickSlotOrderCount;
    int closeSlotOrder[
        DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_GUARD_SLOT_COUNT_PC34];
    int clickSlotOrder[
        DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_GUARD_SLOT_COUNT_PC34];
    int usedSlotOrder[
        DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_GUARD_SLOT_COUNT_PC34];
} Dm1V1MirrorCandidateDoubleOpenCloseGuardStatePc34Compat;

typedef struct Dm1V1MirrorCandidateDoubleOpenCloseGuardResultPc34Compat {
    const Dm1V1MirrorCandidateDoubleOpenCloseGuardEvidencePc34Compat *evidence;
    int eventsProcessed;
    int panelContentBefore;
    int panelContentAfter;
    int c040PanelOpenBefore;
    int c040PanelOpenAfter;
    unsigned int candidateBefore;
    unsigned int candidateAfter;
    unsigned int inventoryBefore;
    unsigned int inventoryAfter;
    unsigned int partyCountBefore;
    unsigned int partyCountAfter;
    int leaderHandBefore;
    int leaderHandAfter;
    int leaderHandQueueBefore;
    int leaderHandQueueAfter;
    int openChestBefore;
    int openChestAfter;
    int openDispatchCount;
    int closeDispatchCount;
    int duplicateOpenNoopCount;
    int duplicateCloseNoopCount;
    int f0282CandidateClearCount;
    int f0297LeaderHandPutCount;
    int f0302SlotDispatchCount;
    int f0334ChestCloseCount;
    int f0355InventoryCloseCount;
    int panelZeroCount;
    int leaderHandQueueClearCount;
    int pendingOpenFlushCount;
    int inventoryPortraitClickCount;
    int sameTickCloseSlotOrderCount;
    int clickSlotOrderCount;
    int usedSlotOrder[
        DM1_V1_MIRROR_CANDIDATE_DOUBLE_OPEN_CLOSE_GUARD_SLOT_COUNT_PC34];
    int doubleOpenWasNoop;
    int doubleOpenPreservedLeaderHand;
    int doubleOpenDidNotClearCandidate;
    int doubleCloseWasNoop;
    int doubleCloseDidNotClearCandidateAgain;
    int doubleClosePreservedClosedPanelState;
    int closeDuringPendingPreservedLeaderHandQueue;
    int closeDuringPendingDidNotClearCandidate;
    int closeDuringPendingOpenedCandidate;
    int inventoryClickUsedCloseSlotOrder;
    int inventoryClickDidNotUseClickSlotOrder;
    int inventoryClickDidNotDispatchF0302;
} Dm1V1MirrorCandidateDoubleOpenCloseGuardResultPc34Compat;

void dm1_v1_mirror_candidate_double_open_close_guard_init_open_pc34_compat(
    Dm1V1MirrorCandidateDoubleOpenCloseGuardStatePc34Compat *state);

void dm1_v1_mirror_candidate_double_open_close_guard_init_closed_pc34_compat(
    Dm1V1MirrorCandidateDoubleOpenCloseGuardStatePc34Compat *state);

int dm1_v1_mirror_candidate_double_open_close_guard_run_events_pc34_compat(
    const Dm1V1MirrorCandidateDoubleOpenCloseGuardStatePc34Compat *initial,
    const Dm1V1MirrorCandidateDoubleOpenCloseGuardEventPc34Compat *events,
    unsigned int eventCount,
    Dm1V1MirrorCandidateDoubleOpenCloseGuardResultPc34Compat *outResult);

int dm1_v1_mirror_candidate_double_open_close_guard_run_double_open_pc34_compat(
    Dm1V1MirrorCandidateDoubleOpenCloseGuardResultPc34Compat *outResult);

int dm1_v1_mirror_candidate_double_open_close_guard_run_double_close_pc34_compat(
    Dm1V1MirrorCandidateDoubleOpenCloseGuardResultPc34Compat *outResult);

int dm1_v1_mirror_candidate_double_open_close_guard_run_close_during_pending_pc34_compat(
    Dm1V1MirrorCandidateDoubleOpenCloseGuardResultPc34Compat *outResult);

int dm1_v1_mirror_candidate_double_open_close_guard_run_inventory_click_during_close_pc34_compat(
    Dm1V1MirrorCandidateDoubleOpenCloseGuardResultPc34Compat *outResult);

const Dm1V1MirrorCandidateDoubleOpenCloseGuardEvidencePc34Compat *
dm1_v1_mirror_candidate_double_open_close_guard_evidence_pc34_compat(void);

#ifdef __cplusplus
}
#endif

#endif

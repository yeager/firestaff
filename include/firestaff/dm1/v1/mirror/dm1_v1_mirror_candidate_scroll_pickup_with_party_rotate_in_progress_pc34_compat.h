#ifndef FIRESTAFF_DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_WITH_PARTY_ROTATE_IN_PROGRESS_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_WITH_PARTY_ROTATE_IN_PROGRESS_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_V1_MCSPPR_CHAMPION_COUNT_PC34 = 4,
    DM1_V1_MCSPPR_CANDIDATE_CHAIN_CAPACITY_PC34 = 4,
    DM1_V1_MCSPPR_CHEST_SLOT_COUNT_PC34 = 8,
    DM1_V1_MCSPPR_THING_NONE_PC34 = 0xFFFF,
    DM1_V1_MCSPPR_SCROLL_THING_PC34 = 0x6C40,
    DM1_V1_MCSPPR_C040_GRAPHIC_PC34 = 40,
    DM1_V1_MCSPPR_C001_TURN_LEFT_PC34 = 1,
    DM1_V1_MCSPPR_C002_TURN_RIGHT_PC34 = 2,
    DM1_V1_MCSPPR_C30_CHEST_SLOT_1_PC34 = 30,
    DM1_V1_MCSPPR_C537_ZONE_CHEST_SLOT_1_PC34 = 537
};

typedef struct DM1_V1_MirrorCandidateScrollPickupWithPartyRotateEvidencePc34 {
    int contractOnly;
    const char *panelClickAnchor;
    const char *championPutAnchor;
    const char *championRemoveAnchor;
    const char *championSlotClickAnchor;
    const char *commandClickAnchor;
    const char *commandKeyQueueAnchor;
    const char *commandDispatchAnchor;
    const char *mouseQueueAnchor;
    const char *reviveOpenAnchor;
    const char *reviveDecisionAnchor;
    const char *chamdrawSlotAnchor;
    const char *chamdrawStateAnchor;
    const char *chamdrawChangedAnchor;
    const char *defsAnchor;
    const char *nonOverlapScope;
} DM1_V1_MirrorCandidateScrollPickupWithPartyRotateEvidencePc34;

typedef struct DM1_V1_MirrorCandidateScrollPickupWithPartyRotateStatePc34 {
    int contractOnly;
    int partyChampionCount;
    int leaderIndex;
    int partyDirection;
    int candidateChampionIndex;
    int candidateOrdinal;
    int candidateIndex;
    int candidateChain[DM1_V1_MCSPPR_CANDIDATE_CHAIN_CAPACITY_PC34];
    int candidateChainLength;
    unsigned int candidateChainHash;
    int c040PanelOpen;
    int c040Graphic;
    int c040RedrawSerial;
    unsigned int c040RedrawHash;
    unsigned int leaderHandThing;
    int leaderHandEmpty;
    unsigned int chestSlots[DM1_V1_MCSPPR_CHEST_SLOT_COUNT_PC34];
    int scrollChestSlotIndex;
    int scrollPc34Slot;
    int scrollZone;
    int commandQueueLocked;
    int f0380DispatchInProgress;
    int partyRotationInProgress;
    int queuedTurnCommand;
    int candidateInternalRotationCount;
    int f0297PutCount;
    int f0298RemoveCount;
    int f0302SlotClickCount;
    int f0359PanelClickCount;
    int f0361QueueWriteCount;
    int f0380DispatchCount;
    int f0380RotationCompleteCount;
    int f0344PanelCellRouteCount;
    int f0345PanelHighlightRouteCount;
    int f0280OpenCount;
    int f0282DecisionCount;
    int f0291SlotDrawCount;
    int f0292StateDrawCount;
    int f0296ChangedObjectIconCount;
    int ignoredPickupDuringPartyRotationCount;
    int honoredPickupAfterRotationCount;
    int pickupAttemptCount;
    int rotationCompletedBeforePickup;
} DM1_V1_MirrorCandidateScrollPickupWithPartyRotateStatePc34;

typedef struct DM1_V1_MirrorCandidateScrollPickupWithPartyRotateSnapshotPc34 {
    int candidateOrdinal;
    int candidateIndex;
    int candidateChain[DM1_V1_MCSPPR_CANDIDATE_CHAIN_CAPACITY_PC34];
    int candidateChainLength;
    unsigned int candidateChainHash;
    int c040PanelOpen;
    int c040Graphic;
    int c040RedrawSerial;
    unsigned int c040RedrawHash;
    int f0282DecisionCount;
    int candidateInternalRotationCount;
} DM1_V1_MirrorCandidateScrollPickupWithPartyRotateSnapshotPc34;

typedef struct DM1_V1_MirrorCandidateScrollPickupWithPartyRotateResultPc34 {
    const DM1_V1_MirrorCandidateScrollPickupWithPartyRotateEvidencePc34 *evidence;
    int initialized;
    int openedCandidate;
    int queuedPartyTurn;
    int beganF0380PartyRotation;
    int ignoredPickupDuringPartyRotation;
    int candidateIndexPreservedDuringIgnore;
    int candidateChainPreservedDuringIgnore;
    int c040RedrawPreservedDuringIgnore;
    int candidateNotInternallyRotated;
    int noLeaderHandPickupDuringRotation;
    int noSlotMutationDuringRotation;
    int rotationCompleted;
    int rotationCompletedBeforePickup;
    int pickupHonoredAfterRotation;
    int leaderHandReceivedScroll;
    int scrollSlotClearedAfterPickup;
    int candidateStillLiveAfterPickup;
    int f0302OnlyAfterRotation;
    int f0297OnlyAfterRotation;
    int f0282NeverFired;
    int contractOnly;
    int noAssetsOrPixelParity;
    unsigned int ignoredSnapshotHash;
    unsigned int finalHash;
    int ok;
} DM1_V1_MirrorCandidateScrollPickupWithPartyRotateResultPc34;

typedef DM1_V1_MirrorCandidateScrollPickupWithPartyRotateEvidencePc34
    DM1_V1_MirrorCandidateScrollPickupWithPartyRotateEvidencePc34Compat;
typedef DM1_V1_MirrorCandidateScrollPickupWithPartyRotateStatePc34
    DM1_V1_MirrorCandidateScrollPickupWithPartyRotateStatePc34Compat;
typedef DM1_V1_MirrorCandidateScrollPickupWithPartyRotateSnapshotPc34
    DM1_V1_MirrorCandidateScrollPickupWithPartyRotateSnapshotPc34Compat;
typedef DM1_V1_MirrorCandidateScrollPickupWithPartyRotateResultPc34
    DM1_V1_MirrorCandidateScrollPickupWithPartyRotateResultPc34Compat;

typedef DM1_V1_MirrorCandidateScrollPickupWithPartyRotateEvidencePc34
    Dm1V1MirrorCandidateScrollPickupWithPartyRotateEvidencePc34Compat;
typedef DM1_V1_MirrorCandidateScrollPickupWithPartyRotateStatePc34
    Dm1V1MirrorCandidateScrollPickupWithPartyRotateStatePc34Compat;
typedef DM1_V1_MirrorCandidateScrollPickupWithPartyRotateSnapshotPc34
    Dm1V1MirrorCandidateScrollPickupWithPartyRotateSnapshotPc34Compat;
typedef DM1_V1_MirrorCandidateScrollPickupWithPartyRotateResultPc34
    Dm1V1MirrorCandidateScrollPickupWithPartyRotateResultPc34Compat;

void DM1_V1_MirrorCandidateScrollPickupWithPartyRotate_InitPc34(
    DM1_V1_MirrorCandidateScrollPickupWithPartyRotateStatePc34 *state);

void DM1_V1_MirrorCandidateScrollPickupWithPartyRotate_SnapshotPc34(
    const DM1_V1_MirrorCandidateScrollPickupWithPartyRotateStatePc34 *state,
    DM1_V1_MirrorCandidateScrollPickupWithPartyRotateSnapshotPc34 *snapshot);

int DM1_V1_MirrorCandidateScrollPickupWithPartyRotate_OpenCandidatePc34(
    DM1_V1_MirrorCandidateScrollPickupWithPartyRotateStatePc34 *state);

int DM1_V1_MirrorCandidateScrollPickupWithPartyRotate_QueueTurnPc34(
    DM1_V1_MirrorCandidateScrollPickupWithPartyRotateStatePc34 *state,
    int turnCommand);

int DM1_V1_MirrorCandidateScrollPickupWithPartyRotate_BeginF0380PartyRotationPc34(
    DM1_V1_MirrorCandidateScrollPickupWithPartyRotateStatePc34 *state);

int DM1_V1_MirrorCandidateScrollPickupWithPartyRotate_ClickScrollPickupPc34(
    DM1_V1_MirrorCandidateScrollPickupWithPartyRotateStatePc34 *state);

int DM1_V1_MirrorCandidateScrollPickupWithPartyRotate_CompletePartyRotationPc34(
    DM1_V1_MirrorCandidateScrollPickupWithPartyRotateStatePc34 *state);

int DM1_V1_MirrorCandidateScrollPickupWithPartyRotate_RunPc34(
    DM1_V1_MirrorCandidateScrollPickupWithPartyRotateStatePc34 *state,
    DM1_V1_MirrorCandidateScrollPickupWithPartyRotateResultPc34 *result);

const DM1_V1_MirrorCandidateScrollPickupWithPartyRotateEvidencePc34 *
DM1_V1_MirrorCandidateScrollPickupWithPartyRotate_EvidencePc34(void);

unsigned int DM1_V1_MirrorCandidateScrollPickupWithPartyRotate_HashPc34(
    const DM1_V1_MirrorCandidateScrollPickupWithPartyRotateStatePc34 *state,
    const DM1_V1_MirrorCandidateScrollPickupWithPartyRotateResultPc34 *result);

#define DM1_V1_MirrorCandidateScrollPickupWithPartyRotate_InitPc34Compat \
    DM1_V1_MirrorCandidateScrollPickupWithPartyRotate_InitPc34
#define DM1_V1_MirrorCandidateScrollPickupWithPartyRotate_SnapshotPc34Compat \
    DM1_V1_MirrorCandidateScrollPickupWithPartyRotate_SnapshotPc34
#define DM1_V1_MirrorCandidateScrollPickupWithPartyRotate_OpenCandidatePc34Compat \
    DM1_V1_MirrorCandidateScrollPickupWithPartyRotate_OpenCandidatePc34
#define DM1_V1_MirrorCandidateScrollPickupWithPartyRotate_QueueTurnPc34Compat \
    DM1_V1_MirrorCandidateScrollPickupWithPartyRotate_QueueTurnPc34
#define DM1_V1_MirrorCandidateScrollPickupWithPartyRotate_BeginF0380PartyRotationPc34Compat \
    DM1_V1_MirrorCandidateScrollPickupWithPartyRotate_BeginF0380PartyRotationPc34
#define DM1_V1_MirrorCandidateScrollPickupWithPartyRotate_ClickScrollPickupPc34Compat \
    DM1_V1_MirrorCandidateScrollPickupWithPartyRotate_ClickScrollPickupPc34
#define DM1_V1_MirrorCandidateScrollPickupWithPartyRotate_CompletePartyRotationPc34Compat \
    DM1_V1_MirrorCandidateScrollPickupWithPartyRotate_CompletePartyRotationPc34
#define DM1_V1_MirrorCandidateScrollPickupWithPartyRotate_RunPc34Compat \
    DM1_V1_MirrorCandidateScrollPickupWithPartyRotate_RunPc34
#define DM1_V1_MirrorCandidateScrollPickupWithPartyRotate_EvidencePc34Compat \
    DM1_V1_MirrorCandidateScrollPickupWithPartyRotate_EvidencePc34
#define DM1_V1_MirrorCandidateScrollPickupWithPartyRotate_HashPc34Compat \
    DM1_V1_MirrorCandidateScrollPickupWithPartyRotate_HashPc34

#ifdef __cplusplus
}
#endif

#endif

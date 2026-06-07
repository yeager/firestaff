/* ReDMCSB source-lock anchors for this pending-hand-queue gate:
 * CHEST.C F0333:30-32 keeps an already-open chest from being reopened.
 * CHEST.C F0334:113-132 closes G0426/G0425 and relinks chest contents.
 * COMMAND.C F0359:1985-1990 dispatches the mirror-candidate C040 panel.
 * REVIVE.C F0280:124-132 requires a live candidate add path.
 */
#ifndef DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_QUEUE_PC34_COMPAT_H
#define DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_QUEUE_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_QUEUE_CAPACITY_PC34_COMPAT 8
#define DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_QUEUE_SLOT_COUNT_PC34_COMPAT 8
#define DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_QUEUE_NONE_PC34_COMPAT (-1)
#define DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_QUEUE_C040_PANEL_PC34_COMPAT 568
#define DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_QUEUE_OPEN_CHEST_PC34_COMPAT 0x0720
#define DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_QUEUE_LEADER_HAND_PC34_COMPAT 0x0440
#define DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_QUEUE_SLOT0_THING_PC34_COMPAT 0x0101
#define DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_QUEUE_SLOT1_THING_PC34_COMPAT 0x0102
#define DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_QUEUE_SLOT2_THING_PC34_COMPAT 0x0103
#define DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_QUEUE_SLOT3_THING_PC34_COMPAT 0x0104
#define DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_QUEUE_SLOT4_THING_PC34_COMPAT 0x0105

typedef struct Dm1V1MirrorCandidatePendingHandQueueEvidencePc34Compat {
    const char *chestAlreadyOpenGuardAnchor;
    const char *chestCloseHandSwapAnchor;
    const char *commandMirrorQueueDispatchAnchor;
    const char *reviveCandidateAliveGuardAnchor;
    const char *contractScope;
    const char *nonOverlapNote;
} Dm1V1MirrorCandidatePendingHandQueueEvidencePc34Compat;

typedef struct Dm1V1MirrorCandidatePendingHandQueueEntryPc34Compat {
    int submissionOrdinal;
    int slotIndex;
    int command;
    int handThingAtSubmission;
} Dm1V1MirrorCandidatePendingHandQueueEntryPc34Compat;

typedef struct Dm1V1MirrorCandidatePendingHandQueueStatePc34Compat {
    int panelContent;
    int c040PanelOpen;
    unsigned int candidateChampionOrdinal;
    unsigned int partyChampionCount;
    int leaderHandThing;
    int openChestThing;
    int containerHeadThing;
    int chestSlots[DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_QUEUE_SLOT_COUNT_PC34_COMPAT];
    Dm1V1MirrorCandidatePendingHandQueueEntryPc34Compat
        pending[DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_QUEUE_CAPACITY_PC34_COMPAT];
    int pendingHead;
    int pendingCount;
    int nextSubmissionOrdinal;
    int submittedSlots[DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_QUEUE_CAPACITY_PC34_COMPAT];
    int drainedSlots[DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_QUEUE_CAPACITY_PC34_COMPAT];
    int drainedOrdinals[DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_QUEUE_CAPACITY_PC34_COMPAT];
    int queueSubmitCount;
    int handQueueDispatchCount;
    int candidateAliveGuardPassCount;
    int candidateAliveGuardBlockCount;
    int alreadyOpenGuardCount;
    int f0334CloseCount;
    int closeDuringPendingQueueCount;
    int chestSlotClearCount;
    int chestFirstSlotWriteCount;
    int chestRelinkCount;
    int slotIndexLostCount;
    int orderViolationCount;
} Dm1V1MirrorCandidatePendingHandQueueStatePc34Compat;

typedef struct Dm1V1MirrorCandidatePendingHandQueueResultPc34Compat {
    const Dm1V1MirrorCandidatePendingHandQueueEvidencePc34Compat *evidence;
    int accepted;
    int ignored;
    int queueFull;
    int blockedByCandidateAliveGuard;
    int submissionOrdinal;
    int slotIndex;
    int command;
    int pendingCountBefore;
    int pendingCountAfter;
    int queueSubmitCountBefore;
    int queueSubmitCountAfter;
    int handQueueDispatchCountBefore;
    int handQueueDispatchCountAfter;
    int candidateAliveGuardPassCountBefore;
    int candidateAliveGuardPassCountAfter;
    int candidateAliveGuardBlockCountBefore;
    int candidateAliveGuardBlockCountAfter;
    int alreadyOpenGuardCountBefore;
    int alreadyOpenGuardCountAfter;
    int f0334CloseCountBefore;
    int f0334CloseCountAfter;
    int closeDuringPendingQueueCountBefore;
    int closeDuringPendingQueueCountAfter;
    int chestSlotClearCountBefore;
    int chestSlotClearCountAfter;
    int chestFirstSlotWriteCountBefore;
    int chestFirstSlotWriteCountAfter;
    int chestRelinkCountBefore;
    int chestRelinkCountAfter;
    int slotIndexLostCountBefore;
    int slotIndexLostCountAfter;
    int orderViolationCountBefore;
    int orderViolationCountAfter;
    int openChestBefore;
    int openChestAfter;
    int containerHeadBefore;
    int containerHeadAfter;
    int candidateOrdinalBefore;
    int candidateOrdinalAfter;
    int partyCountBefore;
    int partyCountAfter;
    int panelContentBefore;
    int panelContentAfter;
    int c040OpenBefore;
    int c040OpenAfter;
    int drainedSlotIndex;
    int drainedSubmissionOrdinal;
    int queueOrderPreserved;
    int pendingSlotIndicesPreserved;
    int candidateAlivePreserved;
    int chestCloseRepackedSlots;
    int alreadyOpenPreservedQueue;
} Dm1V1MirrorCandidatePendingHandQueueResultPc34Compat;

void DM1_V1_MirrorCandidatePendingHandQueue_InitPc34Compat(
    Dm1V1MirrorCandidatePendingHandQueueStatePc34Compat *state);

int DM1_V1_MirrorCandidatePendingHandQueue_SubmitHandSwapPc34Compat(
    Dm1V1MirrorCandidatePendingHandQueueStatePc34Compat *state,
    int slotIndex,
    Dm1V1MirrorCandidatePendingHandQueueResultPc34Compat *outResult);

int DM1_V1_MirrorCandidatePendingHandQueue_OpenAlreadyOpenChestPc34Compat(
    Dm1V1MirrorCandidatePendingHandQueueStatePc34Compat *state,
    Dm1V1MirrorCandidatePendingHandQueueResultPc34Compat *outResult);

int DM1_V1_MirrorCandidatePendingHandQueue_CloseChestPc34Compat(
    Dm1V1MirrorCandidatePendingHandQueueStatePc34Compat *state,
    Dm1V1MirrorCandidatePendingHandQueueResultPc34Compat *outResult);

int DM1_V1_MirrorCandidatePendingHandQueue_DrainNextPc34Compat(
    Dm1V1MirrorCandidatePendingHandQueueStatePc34Compat *state,
    Dm1V1MirrorCandidatePendingHandQueueResultPc34Compat *outResult);

const Dm1V1MirrorCandidatePendingHandQueueEvidencePc34Compat *
DM1_V1_MirrorCandidatePendingHandQueue_EvidencePc34Compat(void);

int dm1_v1_mirror_candidate_pending_hand_queue_run(
    int *passed,
    int *failed);

#ifdef __cplusplus
}
#endif

#endif

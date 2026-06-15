#ifndef DM1_V1_MIRROR_CANDIDATE_THOUGHT_PROJECT_CANCEL_AFTER_PICKUP_PC34_COMPAT_H
#define DM1_V1_MIRROR_CANDIDATE_THOUGHT_PROJECT_CANCEL_AFTER_PICKUP_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB anchors: COMMAND.C F0378:1956-1994 and F0380:2045-2159 route
 * panel/queue commands; CHEST.C F0333:30-67 and F0334:113-132 materialize
 * and close G0425/G0426; CHAMPION.C F0297:243-268, F0298:270-298,
 * F0300:511-584, F0301:606-660, and F0302:662-713 own leader-hand and C30+
 * slot exchange; REVIVE.C F0280:124-132 and F0282:744-806 publish and cancel
 * G0299; PANEL.C F0346/F0347:1619-1657 keeps C040 visible while
 * G0299 is set; UTAMSCR.C F0077/F0078:141-150 brackets pointer redraw;
 * DEFS.H:338-340, 810-817, 1874-1878, 2200, 3001-3008, 5694, 5876-5881 names
 * C162, C30..C37, C38, C040, M568/M569, G0299, G0425, and G0426.
 */

#define DM1_V1_MIRROR_CANCEL_AFTER_PICKUP_CHEST_SLOT_COUNT_PC34_COMPAT 8
#define DM1_V1_MIRROR_CANCEL_AFTER_PICKUP_THOUGHT_TEXT_SIZE_PC34_COMPAT 48

enum {
    DM1_V1_MIRROR_CANCEL_AFTER_PICKUP_NONE_PC34_COMPAT = 0,
    DM1_V1_MIRROR_CANCEL_AFTER_PICKUP_C162_PC34_COMPAT = 162,
    DM1_V1_MIRROR_CANCEL_AFTER_PICKUP_C30_SLOT_CHEST_1_PC34_COMPAT = 30,
    DM1_V1_MIRROR_CANCEL_AFTER_PICKUP_C38_SLOT_BOX_CHEST_1_PC34_COMPAT = 38,
    DM1_V1_MIRROR_CANCEL_AFTER_PICKUP_C040_PANEL_PC34_COMPAT = 40,
    DM1_V1_MIRROR_CANCEL_AFTER_PICKUP_M568_PANEL_PC34_COMPAT = 5,
    DM1_V1_MIRROR_CANCEL_AFTER_PICKUP_M569_PANEL_PC34_COMPAT = 4
};

typedef struct Dm1V1MirrorCancelAfterPickupEvidencePc34Compat {
    int contractOnly;
    const char *commandPanelAnchor;
    const char *commandQueueAnchor;
    const char *chestOpenAnchor;
    const char *chestCloseAnchor;
    const char *leaderHandPutAnchor;
    const char *leaderHandRemoveAnchor;
    const char *slotRemoveAnchor;
    const char *slotAddAnchor;
    const char *slotDispatchAnchor;
    const char *candidateOpenAnchor;
    const char *candidateCancelAnchor;
    const char *panelAnchor;
    const char *mouseAnchor;
    const char *defsAnchor;
    const char *contractScope;
} Dm1V1MirrorCancelAfterPickupEvidencePc34Compat;

typedef struct Dm1V1MirrorCancelAfterPickupSpecPc34Compat {
    int leaderIndex;
    unsigned int candidateOrdinal;
    int thoughtCellX;
    int thoughtCellY;
    int chestThing;
    int scrollThing;
    int pickedSlotIndex;
    int pickedSlotId;
    int pickedSlotBox;
    int panelGraphic;
    int resurrectPanelId;
    int chestPanelId;
} Dm1V1MirrorCancelAfterPickupSpecPc34Compat;

typedef struct Dm1V1MirrorCancelAfterPickupStatePc34Compat {
    int contractOnly;
    int partyCount;
    int leaderIndex;
    int leaderHandThing;
    int leaderHandWasEmptiedByCancel;
    unsigned int candidateOrdinal;
    unsigned int g0299CandidateOrdinal;
    int panelOpen;
    int panelContent;
    int openChestThing;
    int chestSlots[DM1_V1_MIRROR_CANCEL_AFTER_PICKUP_CHEST_SLOT_COUNT_PC34_COMPAT];
    int cellX;
    int cellY;
    int cellThing;
    int previousCellThing;
    int thoughtThing;
    int thoughtProjected;
    int thoughtPickedUp;
    int thoughtRestoredByCancel;
    int projectDispatchCount;
    int pickupDispatchCount;
    int cancelDispatchCount;
    int f0333OpenCount;
    int f0334CloseCount;
    int f0297PutCount;
    int f0298RemoveCount;
    int f0300SlotRemoveCount;
    int f0301SlotAddCount;
    int f0302SlotDispatchCount;
    int f0280OpenCount;
    int f0282CancelCount;
    int panelRedrawCount;
    int mouseEnableCount;
    int mouseDisableCount;
    char thoughtText
        [DM1_V1_MIRROR_CANCEL_AFTER_PICKUP_THOUGHT_TEXT_SIZE_PC34_COMPAT];
} Dm1V1MirrorCancelAfterPickupStatePc34Compat;

typedef struct Dm1V1MirrorCancelAfterPickupResultPc34Compat {
    const Dm1V1MirrorCancelAfterPickupEvidencePc34Compat *evidence;
    const Dm1V1MirrorCancelAfterPickupSpecPc34Compat *spec;
    int accepted;
    int candidateOrdinalBefore;
    int candidateOrdinalAfterPickup;
    int candidateOrdinalAfterCancel;
    int panelOpenBefore;
    int panelOpenAfterPickup;
    int panelOpenAfterCancel;
    int panelContentBefore;
    int panelContentAfterPickup;
    int panelContentAfterCancel;
    int openChestBefore;
    int openChestAfterPickup;
    int openChestAfterCancel;
    int leaderHandBefore;
    int leaderHandAfterPickup;
    int leaderHandAfterCancel;
    int pickedSlotBefore;
    int pickedSlotAfterPickup;
    int pickedSlotAfterCancel;
    int cellThingBeforeProject;
    int cellThingAfterProject;
    int cellThingAfterPickup;
    int cellThingAfterCancel;
    int previousCellThing;
    int thoughtThing;
    int thoughtProjected;
    int scrollPickupDispatched;
    int cancelDispatched;
    int handEmptied;
    int previousCellRestored;
    int thoughtNoLongerProjected;
    int candidateCleared;
    int chestClosed;
    int pickedSlotRemainsEmpty;
    int unrelatedChestSlotsPreserved;
    int textPreservedForAudit;
    int countersBefore[12];
    int countersAfterPickup[12];
    int countersAfterCancel[12];
    int chestSlotsBefore[DM1_V1_MIRROR_CANCEL_AFTER_PICKUP_CHEST_SLOT_COUNT_PC34_COMPAT];
    int chestSlotsAfterPickup[DM1_V1_MIRROR_CANCEL_AFTER_PICKUP_CHEST_SLOT_COUNT_PC34_COMPAT];
    int chestSlotsAfterCancel[DM1_V1_MIRROR_CANCEL_AFTER_PICKUP_CHEST_SLOT_COUNT_PC34_COMPAT];
    char thoughtTextAfterCancel
        [DM1_V1_MIRROR_CANCEL_AFTER_PICKUP_THOUGHT_TEXT_SIZE_PC34_COMPAT];
} Dm1V1MirrorCancelAfterPickupResultPc34Compat;

void DM1_V1_MirrorCandidateThoughtProjectCancelAfterPickup_InitPc34Compat(
    Dm1V1MirrorCancelAfterPickupStatePc34Compat *state);

int DM1_V1_MirrorCandidateThoughtProjectCancelAfterPickup_RunPc34Compat(
    Dm1V1MirrorCancelAfterPickupStatePc34Compat *state,
    Dm1V1MirrorCancelAfterPickupResultPc34Compat *outResult);

const Dm1V1MirrorCancelAfterPickupEvidencePc34Compat *
DM1_V1_MirrorCandidateThoughtProjectCancelAfterPickup_EvidencePc34Compat(void);

const Dm1V1MirrorCancelAfterPickupSpecPc34Compat *
DM1_V1_MirrorCandidateThoughtProjectCancelAfterPickup_SpecPc34Compat(void);

const char *
DM1_V1_MirrorCandidateThoughtProjectCancelAfterPickup_SourceEvidencePc34Compat(void);

#ifdef __cplusplus
}
#endif

#endif

#ifndef DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_OCCUPIED_SLOT_PC34_COMPAT_H
#define DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_OCCUPIED_SLOT_PC34_COMPAT_H

#include "dm1_v1_inventory_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_OCCUPIED_SLOT_SLOT_COUNT_PC34_COMPAT DM1_PC34_CHEST_SLOT_COUNT
#define DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_OCCUPIED_SLOT_PARTY_COUNT_PC34_COMPAT 2
#define DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_OCCUPIED_SLOT_BROWSE_STEP_COUNT_PC34_COMPAT 3

typedef enum DM1_V1_MirrorCandidateKeyboardBrowseOccupiedSlotKeyPc34 {
    DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_OCCUPIED_SLOT_KEY_RIGHT_PC34_COMPAT = 1,
    DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_OCCUPIED_SLOT_KEY_LEFT_PC34_COMPAT = 2
} DM1_V1_MirrorCandidateKeyboardBrowseOccupiedSlotKeyPc34;

typedef struct DM1_V1_MirrorCandidateKeyboardBrowseOccupiedSlotSpecPc34 {
    int partyCount;
    int leaderIndex;
    unsigned int leaderOrdinal;
    unsigned int initialCandidateOrdinal;
    unsigned int browsedCandidateOrdinal;
    int inventoryChampionIndex;
    unsigned int inventoryChampionOrdinal;
    int openChestThing;
    int c538ChestSlotIndex;
    int c538Pc34Slot;
    int c538DisplayZone;
    int c040ScrollThing;
    int c538OriginalOccupantThing;
    int c10ColorFlesh;
    const char *chestOpenAnchor;
    const char *chestCloseAnchor;
    const char *championIdentityAnchor;
    const char *leaderHandPutAnchor;
    const char *leaderHandRemoveAnchor;
    const char *slotClearAnchor;
    const char *slotWriteAnchor;
    const char *occupiedSlotSwapAnchor;
    const char *clickToC040Anchor;
    const char *queueKeyboardBrowseAnchor;
    const char *candidateActivationAnchor;
    const char *candidatePanelAnchor;
    const char *panelClickAnchor;
    const char *panelReleaseAnchor;
    const char *screenUpdateAnchor;
    const char *objectIconAnchor;
    const char *partialMaskAnchor;
    const char *defsAnchor;
    const char *sourceEvidence;
} DM1_V1_MirrorCandidateKeyboardBrowseOccupiedSlotSpecPc34;

typedef struct DM1_V1_MirrorCandidateKeyboardBrowseOccupiedSlotStepPc34 {
    int key;
    unsigned int candidateOrdinalBefore;
    unsigned int candidateOrdinalAfter;
    int activeRosterIndexBefore;
    int activeRosterIndexAfter;
    int queueDispatched;
    int panelStillActive;
} DM1_V1_MirrorCandidateKeyboardBrowseOccupiedSlotStepPc34;

typedef struct DM1_V1_MirrorCandidateKeyboardBrowseOccupiedSlotResultPc34 {
    const DM1_V1_MirrorCandidateKeyboardBrowseOccupiedSlotSpecPc34 *spec;
    int accepted;
    int chestOpenDispatched;
    int sameOpenDisplayGuardHeld;
    int panelClickDispatched;
    int panelReleaseDispatched;
    int keyboardBrowseDispatched;
    int keyboardLeftCount;
    int keyboardRightCount;
    int candidateBecameNonLeader;
    int candidatePanelActiveBeforeSwap;
    int candidatePanelActiveAfterSwap;
    int candidatePanelActiveAfterClose;
    int candidateStatePreservedThroughSwap;
    int candidateStatePreservedThroughClose;
    int candidateNoPanelClear;
    int c040RoutingPreserved;
    int occupiedSlotSwapDispatched;
    int occupiedSlotSwapAccepted;
    int occupiedSlotSwapRejected;
    int leaderHandCanEquipC538;
    int f0298RemovedLeaderHand;
    int f0300ClearedC538;
    int f0297PutC538OccupantInLeaderHand;
    int f0301WroteScrollToC538;
    int screenUpdateEnableCount;
    int screenUpdateDisableCount;
    int partialMaskPresented;
    int iconIdentityPreserved;
    int chestCloseDispatched;
    int closedChestCount;
    int openChestThingAfterClose;
    int closedChainMatchesOpenPostSwap;
    int sourceLockedContractOnly;

    unsigned int candidateOrdinalBeforeBrowse;
    unsigned int candidateOrdinalAfterBrowse;
    unsigned int candidateOrdinalAfterSwap;
    unsigned int candidateOrdinalAfterClose;
    int activeRosterIndexBeforeBrowse;
    int activeRosterIndexAfterBrowse;
    int activeRosterIndexAfterSwap;
    int championOrdinals[DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_OCCUPIED_SLOT_PARTY_COUNT_PC34_COMPAT];
    int championIsLeader[DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_OCCUPIED_SLOT_PARTY_COUNT_PC34_COMPAT];
    int championCurrentHealth[DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_OCCUPIED_SLOT_PARTY_COUNT_PC34_COMPAT];

    int leaderHandTypeBeforeSwap;
    int leaderHandWeightBeforeSwap;
    int leaderHandChargesBeforeSwap;
    int leaderHandAllowedSlotsBeforeSwap;
    int leaderHandTypeAfterSwap;
    int leaderHandWeightAfterSwap;
    int leaderHandChargesAfterSwap;
    int leaderHandAllowedSlotsAfterSwap;

    int c537ToC544TypesBefore[DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_OCCUPIED_SLOT_SLOT_COUNT_PC34_COMPAT];
    int c537ToC544WeightsBefore[DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_OCCUPIED_SLOT_SLOT_COUNT_PC34_COMPAT];
    int c537ToC544TypesAfter[DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_OCCUPIED_SLOT_SLOT_COUNT_PC34_COMPAT];
    int c537ToC544WeightsAfter[DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_OCCUPIED_SLOT_SLOT_COUNT_PC34_COMPAT];
    int closedChainTypes[DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_OCCUPIED_SLOT_SLOT_COUNT_PC34_COMPAT];
    int expectedTypesAfter[DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_OCCUPIED_SLOT_SLOT_COUNT_PC34_COMPAT];
    int expectedWeightsAfter[DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_OCCUPIED_SLOT_SLOT_COUNT_PC34_COMPAT];

    DM1_V1_MirrorCandidateKeyboardBrowseOccupiedSlotStepPc34
        browseSteps[DM1_V1_MIRROR_CANDIDATE_KEYBOARD_BROWSE_OCCUPIED_SLOT_BROWSE_STEP_COUNT_PC34_COMPAT];
} DM1_V1_MirrorCandidateKeyboardBrowseOccupiedSlotResultPc34;

const DM1_V1_MirrorCandidateKeyboardBrowseOccupiedSlotSpecPc34 *
dm1_v1_mirror_candidate_keyboard_browse_occupied_slot_spec_pc34(void);

int dm1_v1_mirror_candidate_keyboard_browse_occupied_slot_probe_pc34(
    DM1_V1_MirrorCandidateKeyboardBrowseOccupiedSlotResultPc34 *outResult);

#ifdef __cplusplus
}
#endif

#endif

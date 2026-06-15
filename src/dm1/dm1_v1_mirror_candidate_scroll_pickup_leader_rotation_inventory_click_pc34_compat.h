#ifndef DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_LEADER_ROTATION_INVENTORY_CLICK_PC34_COMPAT_H
#define DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_LEADER_ROTATION_INVENTORY_CLICK_PC34_COMPAT_H

#include "dm1_v1_inventory_pc34_compat.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_LEADER_ROTATION_INVENTORY_CLICK_SLOT_COUNT_PC34_COMPAT DM1_PC34_CHEST_SLOT_COUNT
#define DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_LEADER_ROTATION_INVENTORY_CLICK_PARTY_COUNT_PC34_COMPAT 4
#define DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_LEADER_ROTATION_INVENTORY_CLICK_C040_PC34_COMPAT 40
#define DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_LEADER_ROTATION_INVENTORY_CLICK_C175_PORTRAIT_PC34_COMPAT 175

typedef enum DM1_V1_MirrorCandidateScrollPickupLeaderRotationInventoryClickPathPc34 {
    DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_LEADER_ROTATION_INVENTORY_CLICK_PATH_PORTRAIT_CLICK_PC34_COMPAT = 1,
    DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_LEADER_ROTATION_INVENTORY_CLICK_PATH_CLOSE_CHEST_PC34_COMPAT = 2
} DM1_V1_MirrorCandidateScrollPickupLeaderRotationInventoryClickPathPc34;

typedef struct DM1_V1_MirrorCandidateScrollPickupLeaderRotationInventoryClickPc34Spec {
    int partyCount;
    int leaderIndex;
    unsigned int inventoryChampionOrdinal;
    unsigned int candidateChampionOrdinal;
    int openChestThing;
    int pickedChestSlotIndex;
    int pickedPc34Slot;
    int c040PanelGraphic;
    int c175PortraitZone;
    int directionBefore;
    int directionAfter;
    const char *chestOpenAnchor;
    const char *chestCloseAnchor;
    const char *leaderHandPutAnchor;
    const char *leaderHandRemoveAnchor;
    const char *slotDispatchAnchor;
    const char *leaderRotationAnchor;
    const char *slotRenderAnchor;
    const char *championStateRedrawAnchor;
    const char *candidateDispatchAnchor;
    const char *candidatePanelOpenAnchor;
    const char *candidatePanelClearAnchor;
    const char *defsAnchor;
    const char *contractScope;
} DM1_V1_MirrorCandidateScrollPickupLeaderRotationInventoryClickPc34Spec;

typedef struct DM1_V1_MirrorCandidateScrollPickupLeaderRotationInventoryClickPc34Result {
    const DM1_V1_MirrorCandidateScrollPickupLeaderRotationInventoryClickPc34Spec
        *spec;
    int path;
    bool accepted;
    bool chestOpenDispatched;
    bool scrollPickupDispatched;
    bool resurrectPanelOpened;
    bool leaderRotationDispatched;
    bool inventoryPortraitClickDispatched;
    bool chestCloseDispatched;
    bool portraitClickRejectedByCandidate;
    bool candidatePanelVisible;
    bool candidateStatePreserved;
    bool candidateNoReinit;
    bool leaderHandStillC040;
    bool leaderHandStatePreservedThroughRotation;
    bool chestLinkOrderPreserved;
    bool c30SlotTypesWeightsByteEqualPrePost;
    bool championRotationCascadeApplied;

    unsigned int candidateOrdinalBefore;
    unsigned int candidateOrdinalAfter;
    int candidatePanelOpenCountBeforeFinal;
    int candidatePanelOpenCountAfterFinal;
    int candidatePanelGraphicBefore;
    int candidatePanelGraphicAfter;

    int leaderHandTypeBeforeRotation;
    int leaderHandTypeAfterRotation;
    int leaderHandTypeAfterFinal;
    int leaderHandWeightBeforeRotation;
    int leaderHandWeightAfterRotation;
    int leaderHandWeightAfterFinal;
    int leaderHandChargesBeforeRotation;
    int leaderHandChargesAfterRotation;
    int leaderHandChargesAfterFinal;
    int leaderHandAllowedSlotsBeforeRotation;
    int leaderHandAllowedSlotsAfterRotation;
    int leaderHandAllowedSlotsAfterFinal;

    int c30SlotTypesPre[
        DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_LEADER_ROTATION_INVENTORY_CLICK_SLOT_COUNT_PC34_COMPAT];
    int c30SlotWeightsPre[
        DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_LEADER_ROTATION_INVENTORY_CLICK_SLOT_COUNT_PC34_COMPAT];
    int c30SlotTypesPost[
        DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_LEADER_ROTATION_INVENTORY_CLICK_SLOT_COUNT_PC34_COMPAT];
    int c30SlotWeightsPost[
        DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_LEADER_ROTATION_INVENTORY_CLICK_SLOT_COUNT_PC34_COMPAT];
    int chestLinkOrder[
        DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_LEADER_ROTATION_INVENTORY_CLICK_SLOT_COUNT_PC34_COMPAT];
    int expectedChestLinkOrder[
        DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_LEADER_ROTATION_INVENTORY_CLICK_SLOT_COUNT_PC34_COMPAT];
    int chestLinkCount;
    int expectedChestLinkCount;
    int closedChestCount;
    int openChestThingAfterFinal;

    int partyDirectionBefore;
    int partyDirectionAfter;
    int championCellsBefore[
        DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_LEADER_ROTATION_INVENTORY_CLICK_PARTY_COUNT_PC34_COMPAT];
    int championCellsAfter[
        DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_LEADER_ROTATION_INVENTORY_CLICK_PARTY_COUNT_PC34_COMPAT];
    int championDirectionsBefore[
        DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_LEADER_ROTATION_INVENTORY_CLICK_PARTY_COUNT_PC34_COMPAT];
    int championDirectionsAfter[
        DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_LEADER_ROTATION_INVENTORY_CLICK_PARTY_COUNT_PC34_COMPAT];

    int f0333OpenCount;
    int f0334CloseCount;
    int f0297PutLeaderHandCount;
    int f0298RemoveLeaderHandCount;
    int f0302SlotDispatchCount;
    int f0284RotationCount;
    int f0291SlotRenderCount;
    int f0293ChampionStateRedrawCount;
    int f0359C040DispatchGuardCount;
    int f0280PanelOpenCount;
    int f0282PanelClearCount;
} DM1_V1_MirrorCandidateScrollPickupLeaderRotationInventoryClickPc34Result;

const char *
dm1_v1_mirror_candidate_scroll_pickup_leader_rotation_inventory_click_source_evidence_pc34(void);

const DM1_V1_MirrorCandidateScrollPickupLeaderRotationInventoryClickPc34Spec *
dm1_v1_mirror_candidate_scroll_pickup_leader_rotation_inventory_click_spec_pc34(void);

int dm1_v1_mirror_candidate_scroll_pickup_leader_rotation_inventory_click_simulate_pc34(
    int path,
    DM1_V1_MirrorCandidateScrollPickupLeaderRotationInventoryClickPc34Result
        *outResult);

#ifdef __cplusplus
}
#endif

#endif

#ifndef FIRESTAFF_DM1_V1_MIRROR_CANDIDATE_C040_PANEL_BROWSE_PICKUP_ROTATE_RACE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_MIRROR_CANDIDATE_C040_PANEL_BROWSE_PICKUP_ROTATE_RACE_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_MC_C040_PICKUP_ROTATE_PARTY_COUNT_PC34 4
#define DM1_V1_MC_C040_PICKUP_ROTATE_CHAIN_COUNT_PC34 3
#define DM1_V1_MC_C040_PICKUP_ROTATE_CHEST_SLOT_COUNT_PC34 8
#define DM1_V1_MC_C040_PICKUP_ROTATE_TRACE_COUNT_PC34 6

typedef struct {
    const char *chestOpenAnchor;
    const char *chestCloseAnchor;
    const char *championChainAnchor;
    const char *championSlotAnchor;
    const char *panelAnchor;
    const char *reviveAnchor;
    const char *commandClickAnchor;
    const char *commandQueueAnchor;
    const char *commandLeaderAnchor;
    const char *mouseWheelAnchor;
    const char *defsAnchor;
    const char *nonOverlap;
} Dm1V1MirrorCandidateC040PanelBrowsePickupRotateRaceEvidencePc34;

typedef struct {
    int championOrdinal;
    int alive;
    int leader;
    int c040ChainLinked;
    int load;
} Dm1V1MirrorCandidateC040PanelBrowsePickupRotateRaceChampionPc34;

typedef struct {
    int partyChampionCount;
    int leaderIndex;
    int pendingLeaderIndex;
    int inventoryChampionOrdinal;
    int c040PanelOpen;
    int panelContent;
    int panelGraphic;
    int candidateOwnerIndex;
    int candidateChainIndex;
    int candidateChainCount;
    int candidateChainOrdinals[DM1_V1_MC_C040_PICKUP_ROTATE_CHAIN_COUNT_PC34];
    int g0299CandidateOrdinal;
    int selectedCandidateOrdinal;
    int leaderHandThing;
    int leaderHandEmpty;
    int openChestThing;
    int g0426OpenChest;
    int chestSlots[DM1_V1_MC_C040_PICKUP_ROTATE_CHEST_SLOT_COUNT_PC34];
    int trace[DM1_V1_MC_C040_PICKUP_ROTATE_TRACE_COUNT_PC34];
    int f0077WheelQueueWriteCount;
    int f0078WheelQueueReadCount;
    int f0280CandidatePublishCount;
    int f0282CandidateClearCount;
    int f0302ChestPickupCount;
    int f0333OpenCount;
    int f0334CloseCount;
    int f0346C040DrawCount;
    int f0347CandidatePriorityCount;
    int f0359PanelClickCount;
    int f0361QueueWriteCount;
    int f0368SetLeaderCount;
    int f0378PanelRouteCount;
    int f0380QueueDispatchCount;
    int wheelQueueDepth;
    int chestPickupClickQueued;
    int chestPickupRejectedByC040Route;
    int sameTickWindow;
    uint32_t chainHash;
    uint32_t chestHash;
    uint32_t stateHash;
    Dm1V1MirrorCandidateC040PanelBrowsePickupRotateRaceChampionPc34
        champions[DM1_V1_MC_C040_PICKUP_ROTATE_PARTY_COUNT_PC34];
} Dm1V1MirrorCandidateC040PanelBrowsePickupRotateRaceStatePc34;

typedef struct {
    int accepted;
    int sameTickWindow;
    int initialLeaderIndex;
    int finalLeaderIndex;
    int pendingLeaderIndexBefore;
    int pendingLeaderIndexAfter;
    int c040PanelOpenBefore;
    int c040PanelOpenAfter;
    int panelContentBefore;
    int panelContentAfter;
    int panelGraphicBefore;
    int panelGraphicAfter;
    int candidateOwnerBefore;
    int candidateOwnerAfter;
    int candidateIndexBefore;
    int candidateIndexAfter;
    int g0299Before;
    int g0299After;
    int selectedCandidateBefore;
    int selectedCandidateAfter;
    int g0426Before;
    int g0426After;
    int openChestThingBefore;
    int openChestThingAfter;
    int chestSlotsBefore[DM1_V1_MC_C040_PICKUP_ROTATE_CHEST_SLOT_COUNT_PC34];
    int chestSlotsAfter[DM1_V1_MC_C040_PICKUP_ROTATE_CHEST_SLOT_COUNT_PC34];
    int trace[DM1_V1_MC_C040_PICKUP_ROTATE_TRACE_COUNT_PC34];
    int wheelQueuedByF0077;
    int wheelReadByF0078;
    int wheelQueueDepthAfterRead;
    int f0361QueueWriteCount;
    int f0380DispatchCount;
    int f0368SetLeaderCount;
    int f0359PanelClickCount;
    int f0378PanelRouteCount;
    int f0302ChestPickupCount;
    int f0334CloseCount;
    int f0282CandidateClearCount;
    int c040RouteRejectedChestPickup;
    int chestStatePreserved;
    int candidateStatePreserved;
    int championChainPreserved;
    int candidateIndexPreserved;
    int selectedCandidatePreserved;
    int g0426Preserved;
    int panelStayedC040;
    int leaderRotationConsumed;
    int noChestClose;
    int noCandidateClear;
    int noSaveLoadTeleporterResurrectCommit;
    int sourceLockAnchorsPresent;
    uint32_t chainHashBefore;
    uint32_t chainHashAfter;
    uint32_t chestHashBefore;
    uint32_t chestHashAfter;
    uint32_t beforeHash;
    uint32_t afterQueueHash;
    uint32_t afterRotationHash;
    uint32_t afterClickHash;
    uint32_t deterministicHash;
} Dm1V1MirrorCandidateC040PanelBrowsePickupRotateRaceResultPc34;

void dm1_v1_mirror_candidate_c040_panel_browse_pickup_rotate_race_init_pc34(
    Dm1V1MirrorCandidateC040PanelBrowsePickupRotateRaceStatePc34 *state);

int dm1_v1_mirror_candidate_c040_panel_browse_pickup_rotate_race_run_pc34(
    Dm1V1MirrorCandidateC040PanelBrowsePickupRotateRaceStatePc34 *state,
    Dm1V1MirrorCandidateC040PanelBrowsePickupRotateRaceResultPc34 *result);

const Dm1V1MirrorCandidateC040PanelBrowsePickupRotateRaceEvidencePc34 *
dm1_v1_mirror_candidate_c040_panel_browse_pickup_rotate_race_evidence_pc34(
    void);

const char *
dm1_v1_mirror_candidate_c040_panel_browse_pickup_rotate_race_source_evidence_pc34(
    void);

#ifdef __cplusplus
}
#endif

#endif

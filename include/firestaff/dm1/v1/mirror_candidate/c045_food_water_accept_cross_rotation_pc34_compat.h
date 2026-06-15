#ifndef FIRESTAFF_DM1_V1_MIRROR_CANDIDATE_C045_FOOD_WATER_ACCEPT_CROSS_ROTATION_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_MIRROR_CANDIDATE_C045_FOOD_WATER_ACCEPT_CROSS_ROTATION_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_MC_C045_ACCEPT_ROTATE_PARTY_COUNT_PC34 4
#define DM1_V1_MC_C045_ACCEPT_ROTATE_CHAIN_COUNT_PC34 4
#define DM1_V1_MC_C045_ACCEPT_ROTATE_TRACE_COUNT_PC34 8
#define DM1_V1_MC_C045_ACCEPT_ROTATE_NONE_PC34 0xffffu

typedef struct {
    const char *revivePublishAnchor;
    const char *reviveAcceptClearAnchor;
    const char *championHandAnchor;
    const char *championSlotAnchor;
    const char *panelAnchor;
    const char *commandQueueAnchor;
    const char *commandPanelRouteAnchor;
    const char *commandDrainAnchor;
    const char *leaderSetAnchor;
    const char *defsAnchor;
    const char *nonOverlap;
} Dm1V1MirrorCandidateC045FoodWaterAcceptCrossRotationEvidencePc34;

typedef struct {
    int ordinal;
    int alive;
    int leader;
    int chainLinked;
    uint16_t handThing;
    int load;
} Dm1V1MirrorCandidateC045FoodWaterAcceptCrossRotationChampionPc34;

typedef struct {
    int contractOnly;
    int sameDrainWindow;
    int partyChampionCount;
    int leaderIndex;
    int queuedLeaderIndex;
    int inventoryChampionOrdinal;
    int candidateChampionOrdinal;
    int candidateChainOrdinals[DM1_V1_MC_C045_ACCEPT_ROTATE_CHAIN_COUNT_PC34];
    int partyChainOrdinals[DM1_V1_MC_C045_ACCEPT_ROTATE_PARTY_COUNT_PC34];
    int g0299CandidateOrdinal;
    int c040PanelOpen;
    int c045PanelOpen;
    int panelContent;
    int panelGraphic;
    int acceptCommand;
    int queuedStatusCommand;
    int queuedSetLeaderCommand;
    uint16_t acceptedFoodThing;
    uint16_t leaderHandThing;
    int foodSlotIndex;
    int foodRemovedFromOldLeader;
    int candidateSensorDisabled;
    int candidateRemovedFromChain;
    int acceptClearCompleted;
    int acceptCompletedBeforeRotation;
    int doubleClearAttempted;
    int skippedClearAttempted;
    int f0280PublishCount;
    int f0282AcceptClearCount;
    int f0297PutLeaderHandCount;
    int f0298RemoveLeaderHandCount;
    int f0301FoodSlotAddCount;
    int f0302FoodSlotDispatchCount;
    int f0344FoodWaterReadCount;
    int f0345FoodWaterDrawCount;
    int f0359QueueWriteCount;
    int f0361WheelLikeQueueWriteCount;
    int f0367LeaderClickRouteCount;
    int f0368SetLeaderCount;
    int f0378PanelRouteCount;
    int f0380DispatchCount;
    int commandQueueDepth;
    int trace[DM1_V1_MC_C045_ACCEPT_ROTATE_TRACE_COUNT_PC34];
    uint32_t beforeHash;
    uint32_t afterQueueHash;
    uint32_t afterAcceptHash;
    uint32_t afterRotateHash;
    Dm1V1MirrorCandidateC045FoodWaterAcceptCrossRotationChampionPc34
        champions[DM1_V1_MC_C045_ACCEPT_ROTATE_PARTY_COUNT_PC34];
} Dm1V1MirrorCandidateC045FoodWaterAcceptCrossRotationStatePc34;

typedef struct {
    int accepted;
    int sameDrainWindow;
    int c045AcceptPath;
    int c040NotLive;
    int acceptClearRanFirst;
    int candidateRemovedFromChain;
    int g0299Cleared;
    int c040C045Cleared;
    int foodRemovedByAccept;
    int oldLeaderHandEmpty;
    int rotationCompletedAfterAccept;
    int newLeaderHandPreserved;
    int leaderHandCoherentAfterRotation;
    int noDoubleClear;
    int noSkippedClear;
    int noDanglingCandidate;
    int partyChainCorrect;
    int queueDrained;
    int sourceAnchorsPresent;
    int guardRejectsC040Live;
    int guardRejectsNoCandidate;
    int guardRejectsWrongPanel;
    int guardRejectsNoRotation;
    int trace[DM1_V1_MC_C045_ACCEPT_ROTATE_TRACE_COUNT_PC34];
    int leaderBefore;
    int leaderAfter;
    uint16_t oldLeaderHandBefore;
    uint16_t oldLeaderHandAfter;
    uint16_t newLeaderHandBefore;
    uint16_t newLeaderHandAfter;
    int g0299Before;
    int g0299After;
    int candidateChainBefore[DM1_V1_MC_C045_ACCEPT_ROTATE_CHAIN_COUNT_PC34];
    int candidateChainAfter[DM1_V1_MC_C045_ACCEPT_ROTATE_CHAIN_COUNT_PC34];
    uint32_t beforeHash;
    uint32_t afterAcceptHash;
    uint32_t afterRotateHash;
    uint32_t hash;
} Dm1V1MirrorCandidateC045FoodWaterAcceptCrossRotationResultPc34;

void dm1_v1_mirror_candidate_c045_food_water_accept_cross_rotation_init_pc34(
    Dm1V1MirrorCandidateC045FoodWaterAcceptCrossRotationStatePc34 *state);

int dm1_v1_mirror_candidate_c045_food_water_accept_cross_rotation_run_pc34(
    Dm1V1MirrorCandidateC045FoodWaterAcceptCrossRotationStatePc34 *state,
    Dm1V1MirrorCandidateC045FoodWaterAcceptCrossRotationResultPc34 *result);

const Dm1V1MirrorCandidateC045FoodWaterAcceptCrossRotationEvidencePc34 *
dm1_v1_mirror_candidate_c045_food_water_accept_cross_rotation_evidence_pc34(
    void);

const char *
dm1_v1_mirror_candidate_c045_food_water_accept_cross_rotation_source_evidence_pc34(
    void);

#ifdef __cplusplus
}
#endif

#endif

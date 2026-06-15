#ifndef FIRESTAFF_DM1_V1_MIRROR_CANDIDATE_RESURRECT_CHEST_CLOSE_ORDER_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_MIRROR_CANDIDATE_RESURRECT_CHEST_CLOSE_ORDER_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_MC_RCCO_CHEST_SLOT_COUNT_PC34 8
#define DM1_V1_MC_RCCO_CANDIDATE_CHAIN_COUNT_PC34 4
#define DM1_V1_MC_RCCO_COMMAND_COUNT_PC34 4
#define DM1_V1_MC_RCCO_TRACE_COUNT_PC34 12
#define DM1_V1_MC_RCCO_NONE_PC34 0xffffu

typedef enum {
    DM1_V1_MC_RCCO_COMMAND_NONE_PC34 = 0,
    DM1_V1_MC_RCCO_COMMAND_C040_YES_PC34 = 160,
    DM1_V1_MC_RCCO_COMMAND_CHEST_CLOSE_PC34 = 334,
    DM1_V1_MC_RCCO_COMMAND_MOVE_FORWARD_PC34 = 3,
    DM1_V1_MC_RCCO_COMMAND_WHEEL_UP_PC34 = 77
} Dm1V1MirrorCandidateResurrectChestCloseOrderCommandPc34;

typedef enum {
    DM1_V1_MC_RCCO_WHEEL_TARGET_NONE_PC34 = 0,
    DM1_V1_MC_RCCO_WHEEL_TARGET_CLOSED_CHEST_PC34 = 1,
    DM1_V1_MC_RCCO_WHEEL_TARGET_LEADER_HAND_PC34 = 2
} Dm1V1MirrorCandidateResurrectChestCloseOrderWheelTargetPc34;

typedef struct {
    int contractOnly;
    int noAssetReads;
    int noOriginalDosPixelParityClaim;
    int partyChampionCount;
    int leaderIndex;
    int inventoryChampionOrdinal;
    int candidateChampionOrdinal;
    int candidateIndexByte;
    int g0299CandidateOrdinal;
    int c040PanelOpen;
    int c040PanelClosed;
    int c038PanelPriorityByte;
    int c037StatusHandBoxByte;
    int c159ChampionIconByte;
    uint16_t g0426OpenChestThing;
    uint16_t leaderHandThing;
    uint16_t chestVisibleSlots[DM1_V1_MC_RCCO_CHEST_SLOT_COUNT_PC34];
    uint16_t chestContainerChain[DM1_V1_MC_RCCO_CHEST_SLOT_COUNT_PC34];
    int candidateChain[DM1_V1_MC_RCCO_CANDIDATE_CHAIN_COUNT_PC34];
    Dm1V1MirrorCandidateResurrectChestCloseOrderCommandPc34
        queuedCommands[DM1_V1_MC_RCCO_COMMAND_COUNT_PC34];
    Dm1V1MirrorCandidateResurrectChestCloseOrderCommandPc34
        dispatchOrder[DM1_V1_MC_RCCO_COMMAND_COUNT_PC34];
    int commandQueueDepth;
    int queueWriteCountF0359;
    int queueWriteCountF0361;
    int queueWriteCountWheelF0077;
    int wheelDrainCountF0078;
    int dispatchDrainCountF0380;
    int f0280PublishCount;
    int f0282AcceptClearCount;
    int f0334ChestCloseCount;
    int f0163RelinkCount;
    int f0297PutAlreadyDoneCount;
    int f0298RemoveCount;
    int f0302ClickDispatchCount;
    int forwardQueuedAfterChestClose;
    int forwardDrainedOnClosedChest;
    int wheelQueuedAfterForward;
    int wheelSawClosedChest;
    Dm1V1MirrorCandidateResurrectChestCloseOrderWheelTargetPc34
        wheelTarget;
    int trace[DM1_V1_MC_RCCO_TRACE_COUNT_PC34];
} Dm1V1MirrorCandidateResurrectChestCloseOrderStatePc34;

typedef struct {
    int acceptedFirst;
    int g0299ClearedFirst;
    int candidateChainRemovedFirst;
    int candidateIndexByteStable;
    int c040PanelClosed;
    int c038PanelPriorityPreserved;
    int c037StatusHandBoxStable;
    int chestClosedSecond;
    int g0426ClearedSecond;
    int visibleSlotsClearedSecond;
    int chestContainerRelinked;
    int leaderHandPreservedAfterClose;
    int leaderHandNotStripped;
    int forwardQueuedAfterClose;
    int forwardDrainedOnClosedChest;
    int wheelAfterForwardLandedOnLeaderHand;
    int wheelDidNotLandOnChest;
    int queueWriteOrderPreserved;
    int dispatchOrderPreserved;
    int f0380DrainProcessedAll;
    int c159ChampionIconStable;
    int sourceAnchorsPresent;
    int guardRejectsNullState;
    int guardRejectsNullResult;
    int guardRejectsNonContract;
    int guardRejectsNoCandidate;
    int guardRejectsClosedChest;
    int assertionsRepresented;
    uint32_t beforeHash;
    uint32_t afterAcceptHash;
    uint32_t afterChestCloseHash;
    uint32_t afterForwardHash;
    uint32_t afterWheelHash;
    uint32_t hash;
} Dm1V1MirrorCandidateResurrectChestCloseOrderResultPc34;

Dm1V1MirrorCandidateResurrectChestCloseOrderStatePc34
dm1_v1_mirror_candidate_resurrect_chest_close_order_default_state_pc34(void);

int dm1_v1_mirror_candidate_resurrect_chest_close_order_run_pc34(
    Dm1V1MirrorCandidateResurrectChestCloseOrderStatePc34 *state,
    Dm1V1MirrorCandidateResurrectChestCloseOrderResultPc34 *result);

uint32_t dm1_v1_mirror_candidate_resurrect_chest_close_order_hash_pc34(
    const Dm1V1MirrorCandidateResurrectChestCloseOrderStatePc34 *state);

const char *
dm1_v1_mirror_candidate_resurrect_chest_close_order_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif

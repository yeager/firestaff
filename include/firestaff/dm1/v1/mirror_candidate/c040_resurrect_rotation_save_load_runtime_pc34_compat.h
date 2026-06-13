#ifndef FIRESTAFF_DM1_V1_MIRROR_CANDIDATE_C040_RESURRECT_ROTATION_SAVE_LOAD_RUNTIME_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_MIRROR_CANDIDATE_C040_RESURRECT_ROTATION_SAVE_LOAD_RUNTIME_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_MC_C040_RRSL_PARTY_COUNT_PC34 4
#define DM1_V1_MC_C040_RRSL_CHEST_SLOT_COUNT_PC34 8
#define DM1_V1_MC_C040_RRSL_TRACE_COUNT_PC34 10

typedef struct {
    const char *commandQueueAnchor;
    const char *saveAnchor;
    const char *loadAnchor;
    const char *revivePublishAnchor;
    const char *reviveClearAnchor;
    const char *partyRotationAnchor;
    const char *leaderHandAnchor;
    const char *slotMutationAnchor;
    const char *panelAnchor;
    const char *defsAnchor;
    const char *nonOverlap;
    const char *contractMarker;
} Dm1V1MirrorCandidateC040ResurrectRotationSaveLoadEvidencePc34;

typedef struct {
    int ordinal;
    int cell;
    int direction;
    int currentHealth;
    int actionHandThing;
    uint32_t byteHash;
} Dm1V1MirrorCandidateC040ResurrectRotationSaveLoadChampionPc34;

typedef struct {
    int contractOnly;
    int noGameDataRequired;
    uint32_t seed;
    int partyChampionCount;
    int partyDirection;
    int leaderIndex;
    int leaderHandThing;
    int g0299CandidateOrdinal;
    int g0423InventoryChampionOrdinal;
    int g0424PanelContent;
    int g0426OpenChest;
    int g0425ChestSlots[DM1_V1_MC_C040_RRSL_CHEST_SLOT_COUNT_PC34];
    int c040GraphicDrawn;
    int queuedCommand;
    int pendingRotationCommand;
    int c140BlockedByCandidate;
    int directSaveLoadBoundaryUsed;
    int f0280PublishCount;
    int f0282ClearCount;
    int f0284RotationCount;
    int f0297PutLeaderHandCount;
    int f0298RemoveLeaderHandCount;
    int f0300RemoveSlotCount;
    int f0301AddSlotCount;
    int f0302SlotClickCount;
    int f0346ResurrectDrawCount;
    int f0347PanelDrawCount;
    int f0359ClickQueueCount;
    int f0380QueueDrainCount;
    int f0433SaveCount;
    int f0435LoadCount;
    int trace[DM1_V1_MC_C040_RRSL_TRACE_COUNT_PC34];
    Dm1V1MirrorCandidateC040ResurrectRotationSaveLoadChampionPc34
        champions[DM1_V1_MC_C040_RRSL_PARTY_COUNT_PC34];
} Dm1V1MirrorCandidateC040ResurrectRotationSaveLoadStatePc34;

typedef struct {
    int accepted;
    int contractOnly;
    int noGameDataRequired;
    int sourceLockAnchorsPresent;
    int guardRejectsNullState;
    int guardRejectsNullResult;
    int guardRejectsNoCandidate;
    int guardRejectsWrongPanel;
    int guardRejectsNoRotation;
    int c140BlockedByCandidate;
    int directSaveLoadBoundaryUsed;
    int g0299BeforeSave;
    int g0299AfterSave;
    int g0299AfterLoad;
    int g0423BeforeSave;
    int g0423AfterLoad;
    int g0424BeforeSave;
    int g0424AfterLoad;
    int g0426BeforeSave;
    int g0426AfterLoad;
    int f0282ClearCount;
    int f0284RotationCount;
    int f0297PutLeaderHandCount;
    int f0298RemoveLeaderHandCount;
    int f0300RemoveSlotCount;
    int f0301AddSlotCount;
    int f0302SlotClickCount;
    int f0433SaveCount;
    int f0435LoadCount;
    int candidateChainStableAcrossSaveLoad;
    int candidateUiStableAcrossSaveLoad;
    int panelOrdinalStableAcrossSaveLoad;
    int chestUiStableAcrossSaveLoad;
    int leaderHandStableAcrossSaveLoad;
    int rotationReplayDeterministic;
    int noCandidateClearAcrossSaveLoad;
    int noSlotMutationAcrossSaveLoad;
    int commandQueueDrainedBeforeSave;
    int trace[DM1_V1_MC_C040_RRSL_TRACE_COUNT_PC34];
    uint32_t candidateHashBeforeSave;
    uint32_t candidateHashAfterSave;
    uint32_t candidateHashAfterLoad;
    uint32_t g0425HashBeforeSave;
    uint32_t g0425HashAfterLoad;
    uint32_t partyPoseHashBeforeSave;
    uint32_t partyPoseHashAfterLoad;
    uint32_t rotationReplayHash;
    uint32_t deterministicHash;
} Dm1V1MirrorCandidateC040ResurrectRotationSaveLoadResultPc34;

void dm1_v1_mirror_candidate_c040_resurrect_rotation_save_load_init_pc34(
    Dm1V1MirrorCandidateC040ResurrectRotationSaveLoadStatePc34 *state,
    uint32_t seed);

int dm1_v1_mirror_candidate_c040_resurrect_rotation_save_load_run_pc34(
    Dm1V1MirrorCandidateC040ResurrectRotationSaveLoadStatePc34 *state,
    Dm1V1MirrorCandidateC040ResurrectRotationSaveLoadResultPc34 *result);

const Dm1V1MirrorCandidateC040ResurrectRotationSaveLoadEvidencePc34 *
dm1_v1_mirror_candidate_c040_resurrect_rotation_save_load_evidence_pc34(void);

const char *
dm1_v1_mirror_candidate_c040_resurrect_rotation_save_load_source_evidence_pc34(
    void);

#ifdef __cplusplus
}
#endif

#endif

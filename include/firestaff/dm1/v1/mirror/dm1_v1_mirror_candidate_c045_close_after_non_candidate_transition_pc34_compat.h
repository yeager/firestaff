#ifndef FIRESTAFF_DM1_V1_MIRROR_CANDIDATE_C045_CLOSE_AFTER_NON_CANDIDATE_TRANSITION_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_MIRROR_CANDIDATE_C045_CLOSE_AFTER_NON_CANDIDATE_TRANSITION_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_MC_C045_AFTER_NC_SLOT_COUNT_PC34 8
#define DM1_V1_MC_C045_AFTER_NC_NONE_PC34 0xffffu
#define DM1_V1_MC_C045_AFTER_NC_END_PC34 0xfffeu

typedef enum {
    DM1_V1_MC_C045_AFTER_NC_TRANSITION_NONE_PC34 = 0,
    DM1_V1_MC_C045_AFTER_NC_TRANSITION_C038_CANCEL_PC34 = 1,
    DM1_V1_MC_C045_AFTER_NC_TRANSITION_C040_CHROME_PC34 = 2,
    DM1_V1_MC_C045_AFTER_NC_TRANSITION_C503_C018_CHROME_PC34 = 3,
    DM1_V1_MC_C045_AFTER_NC_TRANSITION_PANEL_REDRAW_PC34 = 4
} Dm1V1MirrorCandidateC045AfterNonCandidateTransitionKindPc34Compat;

typedef struct {
    const char *chestOpenAnchor;
    const char *chestCloseAnchor;
    const char *championAnchor;
    const char *reviveAnchor;
    const char *commandAnchor;
    const char *panelAnchor;
    const char *defsAnchor;
    const char *scope;
} Dm1V1MirrorCandidateC045AfterNonCandidateEvidencePc34Compat;

typedef struct {
    int contractOnly;
    int transitionKind;
    int transitionApplied;
    int nonCandidateTransition;
    int leaderIndex;
    int inventoryChampionOrdinal;
    int leaderEmptyHanded;
    uint16_t leaderHandC30Thing;
    uint16_t g0426OpenChest;
    uint16_t sourceChain[DM1_V1_MC_C045_AFTER_NC_SLOT_COUNT_PC34];
    uint16_t visibleSlots[DM1_V1_MC_C045_AFTER_NC_SLOT_COUNT_PC34];
    int c540SlotIndex;
    int c540Zone;
    int c540Command;
    int c045Graphic;
    int panelContentBefore;
    int panelContentAfterTransition;
    int panelContentAfterClose;
    int panelOpen;
    int candidateChampionOrdinal;
    int c503CloseDispatches;
    int c018LeaderTransitionCommand;
    int c019LeaderTransitionCommand;
    int c038CancelCommand;
    int f0280Entered;
    int f0282Entered;
    int f0333MaterializeCount;
    int f0334RelinkCount;
    int f0297PutLeaderHandCount;
    int f0298RemoveLeaderHandCount;
    int f0300RemoveSlotCount;
    int f0301AddSlotCount;
    int f0302SlotCommandCount;
    int f0344FoodWaterReadCount;
    int f0345FoodWaterDrawCount;
    int f0354CloseCount;
    int f0359C040DispatchCount;
    int panelRedrawCount;
    uint32_t beforeHash;
    uint32_t transitionHash;
    uint32_t closeHash;
} Dm1V1MirrorCandidateC045AfterNonCandidateStatePc34Compat;

typedef struct {
    int accepted;
    int transitionWasAfterNonCandidate;
    int closeFiredAfterTransition;
    int leaderHandPreserved;
    int c30ChainPreserved;
    int noLeaderHandMutation;
    int noF0280C040Entry;
    int noF0282CandidateEntry;
    int g0426Preserved;
    int visibleSlotsPreserved;
    int c540RoutePreserved;
    int noF0333MaterializeOnClose;
    int noF0334RelinkOnClose;
    int noC040Dispatch;
    int c045PanelClosed;
    int c503CloseObserved;
    int foodWaterReadStable;
    int deterministicAgainstNoTransition;
    int transitionHashChanged;
    int closeHashStable;
    int guardRejectsEmptyLeaderHand;
    int guardRejectsCandidate;
    int guardRejectsWrongRoute;
    int guardRejectsClosedChest;
    uint16_t leaderHandBefore;
    uint16_t leaderHandAfter;
    uint16_t g0426Before;
    uint16_t g0426After;
    uint16_t visibleSlotsAfter[DM1_V1_MC_C045_AFTER_NC_SLOT_COUNT_PC34];
    uint32_t baselineCloseHash;
    uint32_t hash;
} Dm1V1MirrorCandidateC045AfterNonCandidateResultPc34Compat;

void dm1_v1_mirror_candidate_c045_close_after_non_candidate_transition_init_pc34(
    Dm1V1MirrorCandidateC045AfterNonCandidateStatePc34Compat *state);

int dm1_v1_mirror_candidate_c045_close_after_non_candidate_transition_run_pc34(
    Dm1V1MirrorCandidateC045AfterNonCandidateStatePc34Compat *state,
    Dm1V1MirrorCandidateC045AfterNonCandidateResultPc34Compat *result);

const Dm1V1MirrorCandidateC045AfterNonCandidateEvidencePc34Compat *
dm1_v1_mirror_candidate_c045_close_after_non_candidate_transition_evidence_pc34(void);

const char *
dm1_v1_mirror_candidate_c045_close_after_non_candidate_transition_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif

/* ReDMCSB anchors: COMMAND.C F0359:1985-1990 routes M568/C040 only
 * while the leader hand is empty; REVIVE.C F0280:124-132 opens the
 * candidate and F0282:744-806 consumes it; CHAMPION.C F0297/F0298/F0300/
 * F0301/F0302 own leader-hand and C30+ slot movement; CHEST.C F0333/F0334
 * own G0425/G0426 chest materialization; PANEL.C F0344/F0345/F0352 and
 * F0346/F0347 redraw panel chrome; DEFS.H:2088 C30/G0425/G0426/M070/M516/
 * C040 plus C537/M568/G0299 identify the routed state.
 */
#ifndef FIRESTAFF_DM1_V1_MIRROR_CANDIDATE_CLOSE_WHILE_RESURRECT_PENDING_WITH_INVENTORY_PICKUP_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_MIRROR_CANDIDATE_CLOSE_WHILE_RESURRECT_PENDING_WITH_INVENTORY_PICKUP_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_MIRROR_CWRPIP_CHAIN_COUNT_PC34_COMPAT 8
#define DM1_V1_MIRROR_CWRPIP_NONE_PC34_COMPAT 0xFFFF
#define DM1_V1_MIRROR_CWRPIP_C30_CHEST_SLOT_PC34_COMPAT 30
#define DM1_V1_MIRROR_CWRPIP_C38_SLOT_BOX_PC34_COMPAT 38
#define DM1_V1_MIRROR_CWRPIP_C040_PANEL_PC34_COMPAT 40
#define DM1_V1_MIRROR_CWRPIP_C537_VISIBLE_SLOT_PC34_COMPAT 537
#define DM1_V1_MIRROR_CWRPIP_M568_PANEL_PC34_COMPAT 568

typedef enum Dm1V1MirrorCwrpipStepPc34Compat {
    DM1_V1_MIRROR_CWRPIP_STEP_BOOTSTRAP_PC34_COMPAT = 0,
    DM1_V1_MIRROR_CWRPIP_STEP_RESURRECT_CANDIDATE_PENDING_PC34_COMPAT = 1,
    DM1_V1_MIRROR_CWRPIP_STEP_C537_PICKUP_BEFORE_CLOSE_PC34_COMPAT = 2,
    DM1_V1_MIRROR_CWRPIP_STEP_C040_CLOSE_WHILE_PENDING_PC34_COMPAT = 3,
    DM1_V1_MIRROR_CWRPIP_STEP_NEXT_OPEN_REFIRE_PC34_COMPAT = 4
} Dm1V1MirrorCwrpipStepPc34Compat;

typedef struct Dm1V1MirrorCwrpipEvidencePc34Compat {
    int contractOnly;
    const char *commandC040Anchor;
    const char *reviveOpenAnchor;
    const char *reviveFinishAnchor;
    const char *championHandAnchor;
    const char *championSlotAnchor;
    const char *chestAnchor;
    const char *panelAnchor;
    const char *defsAnchor;
    const char *scope;
} Dm1V1MirrorCwrpipEvidencePc34Compat;

typedef struct Dm1V1MirrorCwrpipStatePc34Compat {
    int contractOnly;
    unsigned int deterministicSeed;
    Dm1V1MirrorCwrpipStepPc34Compat currentStep;
    Dm1V1MirrorCwrpipStepPc34Compat dominantStep;
    unsigned int originalCandidateOrdinal;
    unsigned int pendingCandidateOrdinal;
    unsigned int activePanelCandidateOrdinal;
    unsigned int inventoryChampionOrdinal;
    int partyChampionCount;
    int leaderIndex;
    int leaderEmptyHanded;
    int leaderHandThing;
    int c040PanelOpen;
    int panelContent;
    int panelGraphic;
    int openChestThing;
    int c30Chain[DM1_V1_MIRROR_CWRPIP_CHAIN_COUNT_PC34_COMPAT];
    int c537Chain[DM1_V1_MIRROR_CWRPIP_CHAIN_COUNT_PC34_COMPAT];
    int pickedThing;
    int pickedSourceC30Slot;
    int pickedSourceC537Zone;
    int panelRedrawSawLeaderHandThing;
    int panelRedrawSawC30Slot0Thing;
    int panelRedrawSawPendingOrdinal;
    int nextOpenRefiredOrdinal;
    int f0280CandidateOpenCount;
    int f0282CandidateConsumeCount;
    int f0297PutLeaderHandCount;
    int f0298RemoveLeaderHandCount;
    int f0300RemoveC30Count;
    int f0301AddC30Count;
    int f0302SlotBoxCount;
    int f0333OpenChestCount;
    int f0334CloseChestCount;
    int f0344PanelChromeCount;
    int f0345PanelChromeCount;
    int f0346DrawC040Count;
    int f0347DrawPanelCount;
    int f0352PanelRedrawCount;
    int f0359C040CloseDispatchCount;
    int f0359C040CloseBlockedByLeaderHandCount;
    int c537PickupCount;
    int candidateConsumedByPickupCount;
    int candidateConsumedByCloseCount;
    unsigned int deterministicHash;
} Dm1V1MirrorCwrpipStatePc34Compat;

typedef struct Dm1V1MirrorCwrpipResultPc34Compat {
    int accepted;
    int rejected;
    Dm1V1MirrorCwrpipStepPc34Compat stepBefore;
    Dm1V1MirrorCwrpipStepPc34Compat stepAfter;
    Dm1V1MirrorCwrpipStepPc34Compat dominantStepAfter;
    unsigned int candidateBefore;
    unsigned int candidateAfter;
    int leaderHandBefore;
    int leaderHandAfter;
    int c30Slot0Before;
    int c30Slot0After;
    int c537Slot0Before;
    int c537Slot0After;
    int f0282Before;
    int f0282After;
    int f0359Before;
    int f0359After;
    int dominantPending;
    int pickupLandedInLeaderHandFromC30;
    int pendingCandidateNotConsumed;
    int closeBlockedByLeaderHand;
    int panelRedrewAgainstPickupModifiedChain;
    int reopenRefiredOriginalChampion;
    unsigned int deterministicHashAfter;
    const char *anchor;
} Dm1V1MirrorCwrpipResultPc34Compat;

void dm1_v1_mirror_candidate_cwrpip_init_pc34_compat(
    Dm1V1MirrorCwrpipStatePc34Compat *state);

int dm1_v1_mirror_candidate_cwrpip_c537_pickup_before_close_pc34_compat(
    Dm1V1MirrorCwrpipStatePc34Compat *state,
    Dm1V1MirrorCwrpipResultPc34Compat *outResult);

int dm1_v1_mirror_candidate_cwrpip_c040_close_while_pending_pc34_compat(
    Dm1V1MirrorCwrpipStatePc34Compat *state,
    Dm1V1MirrorCwrpipResultPc34Compat *outResult);

int dm1_v1_mirror_candidate_cwrpip_next_open_refire_pc34_compat(
    Dm1V1MirrorCwrpipStatePc34Compat *state,
    Dm1V1MirrorCwrpipResultPc34Compat *outResult);

int dm1_v1_mirror_candidate_cwrpip_drive_pc34_compat(
    Dm1V1MirrorCwrpipStatePc34Compat *state,
    Dm1V1MirrorCwrpipResultPc34Compat *steps,
    int stepCapacity);

const Dm1V1MirrorCwrpipEvidencePc34Compat *
dm1_v1_mirror_candidate_cwrpip_evidence_pc34_compat(void);

const char *
dm1_v1_mirror_candidate_cwrpip_source_evidence_pc34_compat(void);

int dm1_v1_mirror_candidate_cwrpip_run_self_test_pc34_compat(void);
int dm1_v1_mirror_candidate_cwrpip_assertions_pc34_compat(void);
int dm1_v1_mirror_candidate_cwrpip_failures_pc34_compat(void);
unsigned int dm1_v1_mirror_candidate_cwrpip_hash_pc34_compat(void);

#ifdef __cplusplus
}
#endif

#endif

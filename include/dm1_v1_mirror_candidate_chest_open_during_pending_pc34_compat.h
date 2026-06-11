/* ReDMCSB source-lock anchors for this chest-open-during-pending gate:
 * CHEST.C F0333:30-32 same G0426_T_OpenChest early return keeps already-open
 *               chest contents unchanged.
 * CHEST.C F0333:53-76 first open materializes G0425_aT_ChestSlots[0..7] from
 *               the linked container chain and pads remaining slots with
 *               C0xFFFF_THING_NONE.
 * COMMAND.C F0359:1985-1990 M568/C040 panel dispatch consumes the action-hand
 *               click and routes it through F0282 candidate panel handling.
 * REVIVE.C F0280:124-132 F0280 candidate admission requires a live leader hand
 *               and an empty party slot.
 * REVIVE.C F0282:744-806 C162 cancel clears G0299 and decrements the party
 *               count, while C160/C161 resurrect/reincarnate also clear
 *               G0299 before consuming the candidate.
 * CHAMPION.C F0297:243-267 leader-hand put writes G4055_s_LeaderHandObject.
 * CHAMPION.C F0298:270-298 leader-hand remove clears G4055 and updates load.
 * DEFS.H C30_SLOT_CHEST_1 (C30+ slot-box range that drives F0302 swaps).
 * DEFS.H G0425_aT_ChestSlots[8] visible chest slot array.
 * DEFS.H G0426_T_OpenChest currently open chest thing handle.
 * DEFS.H M070_HAND_SLOT_INDEX(slotboxindex) leader-hand slot-box detector.
 * DEFS.H M516_CHAMPIONS party champion array accessed by F0282.
 * DEFS.H C040_COMMAND_CLICK_ON_SLOT_BOX_20_INVENTORY_QUIVER_LINE1_1 first
 *       C040 slot-box command (the C040 family starts at command 40 and is
 *       used here to remind the runtime that C040 also lives in the slot-box
 *       space; the candidate panel commands are C160..C162).
 */
#ifndef DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_PC34_COMPAT_H
#define DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_SLOT_COUNT_PC34_COMPAT 8
#define DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_NONE_PC34_COMPAT (-1)

#define DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_M568_C040_PC34_COMPAT 568
#define DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_M569_CHEST_PC34_COMPAT 569

/* ReDMCSB CHEST.C F0333:30-32 same-open early return path */
#define DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_SAME_OPEN_GUARD_PC34_COMPAT 0x0F01
/* ReDMCSB CHEST.C F0333:53-76 first-eight slot materialization path */
#define DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_FIRST_OPEN_PC34_COMPAT 0x0F02
/* ReDMCSB COMMAND.C F0359:1985-1990 action-hand click dispatch on M568 panel */
#define DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_ACTION_HAND_CLICK_PC34_COMPAT 0x0F03
/* ReDMCSB CHEST.C F0333:30-32 non-chest cell click (wall / open floor) */
#define DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_NON_CHEST_CELL_PC34_COMPAT 0x0F04

/* The C040 panel identifier (M568 in COMMAND.C F0359:1985) */
#define DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_C040_PANEL_PC34_COMPAT 568
/* Leader empty-hand sentinel — REVIVE.C F0280:124-125 requires
 * G0415_ui_LeaderEmptyHanded to be true before a candidate is added. */
#define DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_LEADER_HAND_PC34_COMPAT (-1)
/* Source-lock fixture: the prior C508 chest that originally triggered the
 * C040 panel by click on its cell in the action-hand world view. */
#define DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_PRIOR_CHEST_PC34_COMPAT 0x0750
/* Source-lock fixture: the *new* C508 chest clicked while the C040 panel is
 * still pending (Scenario A in this gate). */
#define DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_NEW_CHEST_PC34_COMPAT 0x0751
/* First-eight G0425 slot values for the new chest after materialization.
 * They mirror the THING types used by ReDMCSB CHEST.C F0333:53-76. */
#define DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_NEW_SLOT0_PC34_COMPAT 0x0201
#define DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_NEW_SLOT1_PC34_COMPAT 0x0202
#define DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_NEW_SLOT2_PC34_COMPAT 0x0203
#define DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_NEW_SLOT3_PC34_COMPAT 0x0204
#define DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_NEW_SLOT4_PC34_COMPAT 0x0205
#define DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_NEW_SLOT5_PC34_COMPAT 0x0206
#define DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_NEW_SLOT6_PC34_COMPAT 0x0207
#define DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_NEW_SLOT7_PC34_COMPAT 0x0208
/* First-eight G0425 slot values for the prior chest (already open when the
 * C040 panel was opened — Scenario B in this gate). */
#define DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_PRIOR_SLOT0_PC34_COMPAT 0x0301
#define DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_PRIOR_SLOT1_PC34_COMPAT 0x0302
#define DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_PRIOR_SLOT2_PC34_COMPAT 0x0303
#define DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_PRIOR_SLOT3_PC34_COMPAT 0x0304
#define DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_PRIOR_SLOT4_PC34_COMPAT (-1)
#define DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_PRIOR_SLOT5_PC34_COMPAT (-1)
#define DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_PRIOR_SLOT6_PC34_COMPAT (-1)
#define DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_PRIOR_SLOT7_PC34_COMPAT (-1)

typedef struct Dm1V1MirrorCandidateChestOpenDuringPendingEvidencePc34Compat {
    const char *chestSameOpenGuardAnchor;
    const char *chestFirstOpenMaterializeAnchor;
    const char *commandPanelDispatchAnchor;
    const char *reviveCandidateAdmitAnchor;
    const char *reviveCandidateClearAnchor;
    const char *championHandPutAnchor;
    const char *championHandRemoveAnchor;
    const char *defsBindingsAnchor;
    const char *contractScope;
    const char *nonOverlapNote;
} Dm1V1MirrorCandidateChestOpenDuringPendingEvidencePc34Compat;

typedef struct Dm1V1MirrorCandidateChestOpenDuringPendingStatePc34Compat {
    int panelContent;
    int c040PanelOpen;
    unsigned int candidateChampionOrdinal;
    unsigned int partyChampionCount;
    int leaderIndex;
    int leaderHandThing;
    int priorOpenChestThing;
    int newOpenChestThing;
    int currentOpenChestThing;
    int currentChestSlots
        [DM1_V1_MIRROR_CANDIDATE_CHEST_OPEN_DURING_PENDING_SLOT_COUNT_PC34_COMPAT];
    int candidateClearCount;
    int partyDecrementCount;
    int commandClickConsumeCount;
    int f0333SameOpenEarlyReturnCount;
    int f0333FirstOpenMaterializeCount;
    int f0333FirstOpenMaterializeSlots;
    int f0333FirstOpenFirstSlotWriteCount;
    int f0333FirstOpenRelinkCount;
    int f0333NoOpenCount;
    int f0297LeaderHandPutCount;
    int f0298LeaderHandRemoveCount;
    int clickConsumedByCandidateClear;
    int clickConsumedByChestOpen;
} Dm1V1MirrorCandidateChestOpenDuringPendingStatePc34Compat;

typedef struct Dm1V1MirrorCandidateChestOpenDuringPendingResultPc34Compat {
    const Dm1V1MirrorCandidateChestOpenDuringPendingEvidencePc34Compat *evidence;
    int clickKind;
    int clickedChestThing;
    int clickConsumed;
    int clickConsumedByCandidateClear;
    int clickConsumedByChestOpen;
    int accepted;
    int ignored;
    int dispatchedF0282;
    int dispatchedF0333;
    int f0333PathTaken;
    int panelContentBefore;
    int panelContentAfter;
    int c040OpenBefore;
    int c040OpenAfter;
    unsigned int candidateOrdinalBefore;
    unsigned int candidateOrdinalAfter;
    unsigned int partyCountBefore;
    unsigned int partyCountAfter;
    int currentOpenChestBefore;
    int currentOpenChestAfter;
    int priorOpenChestBefore;
    int priorOpenChestAfter;
    int newOpenChestBefore;
    int newOpenChestAfter;
    int leaderHandBefore;
    int leaderHandAfter;
    int slot0Before;
    int slot0After;
    int slot1Before;
    int slot1After;
    int slot2Before;
    int slot2After;
    int slot3Before;
    int slot3After;
    int slot4Before;
    int slot4After;
    int slot5Before;
    int slot5After;
    int slot6Before;
    int slot6After;
    int slot7Before;
    int slot7After;
    int candidateClearCountBefore;
    int candidateClearCountAfter;
    int partyDecrementCountBefore;
    int partyDecrementCountAfter;
    int commandClickConsumeCountBefore;
    int commandClickConsumeCountAfter;
    int f0333SameOpenEarlyReturnCountBefore;
    int f0333SameOpenEarlyReturnCountAfter;
    int f0333FirstOpenMaterializeCountBefore;
    int f0333FirstOpenMaterializeCountAfter;
    int f0333FirstOpenMaterializeSlotsBefore;
    int f0333FirstOpenMaterializeSlotsAfter;
    int f0333FirstOpenFirstSlotWriteCountBefore;
    int f0333FirstOpenFirstSlotWriteCountAfter;
    int f0333FirstOpenRelinkCountBefore;
    int f0333FirstOpenRelinkCountAfter;
    int f0333NoOpenCountBefore;
    int f0333NoOpenCountAfter;
    int f0297LeaderHandPutCountBefore;
    int f0297LeaderHandPutCountAfter;
    int f0298LeaderHandRemoveCountBefore;
    int f0298LeaderHandRemoveCountAfter;
    int leaderHandMutationObserved;
    int chestHandSwapObserved;
    int g0425MaterializedFromNewChest;
    int g0425PreservedFromPriorChest;
    int noChestOpened;
    int candidateCleared;
    int panelClosed;
} Dm1V1MirrorCandidateChestOpenDuringPendingResultPc34Compat;

void DM1_V1_MirrorCandidateChestOpenDuringPending_InitPc34Compat(
    Dm1V1MirrorCandidateChestOpenDuringPendingStatePc34Compat *state);

int DM1_V1_MirrorCandidateChestOpenDuringPending_ActionHandClickPc34Compat(
    Dm1V1MirrorCandidateChestOpenDuringPendingStatePc34Compat *state,
    int clickKind,
    int clickedChestThing,
    Dm1V1MirrorCandidateChestOpenDuringPendingResultPc34Compat *outResult);

const Dm1V1MirrorCandidateChestOpenDuringPendingEvidencePc34Compat *
DM1_V1_MirrorCandidateChestOpenDuringPending_EvidencePc34Compat(void);

int dm1_v1_mirror_candidate_chest_open_during_pending_run(
    int *passed,
    int *failed);

#ifdef __cplusplus
}
#endif

#endif

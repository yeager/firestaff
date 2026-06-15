/* ReDMCSB anchors: COMMAND.C F0359:1985-1990 routes M568/C040 candidate
 * panel clicks only while the leader hand is empty; REVIVE.C F0280:124-132
 * publishes a candidate only with an empty hand and party room, and F0282:
 * 744-806 is the only candidate-consuming route; CHEST.C F0333/F0334 own
 * G0425/G0426 C30+ materialization; CHAMPION.C F0284/F0297/F0298/F0300/
 * F0301/F0302 own party/hand/slot mutation; PANEL.C F0344/F0345/F0352 and
 * F0346/F0347 redraw the panel; DEFS.H names C30..C37, G0425/G0426,
 * M070, M516, and C040.
 */
#ifndef DM1_V1_MIRROR_CANDIDATE_RESURRECT_FULL_C30_CHAIN_PC34_COMPAT_H
#define DM1_V1_MIRROR_CANDIDATE_RESURRECT_FULL_C30_CHAIN_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_MCRFC30_CHAIN_COUNT_PC34_COMPAT 8
#define DM1_V1_MCRFC30_CHAMPION_SLOT_COUNT_PC34_COMPAT 30
#define DM1_V1_MCRFC30_THING_NONE_PC34_COMPAT 0xFFFFu
#define DM1_V1_MCRFC30_C30_SLOT_PC34_COMPAT 30
#define DM1_V1_MCRFC30_C37_SLOT_PC34_COMPAT 37
#define DM1_V1_MCRFC30_C040_PANEL_PC34_COMPAT 40
#define DM1_V1_MCRFC30_C160_RESURRECT_PC34_COMPAT 160
#define DM1_V1_MCRFC30_C162_CANCEL_PC34_COMPAT 162
#define DM1_V1_MCRFC30_M568_PANEL_PC34_COMPAT 568
#define DM1_V1_MCRFC30_G0426_OPEN_CHEST_PC34_COMPAT 0x6401u

typedef struct Dm1V1MirrorCandidateResurrectFullC30EvidencePc34Compat {
    int contractOnly;
    const char *commandAnchor;
    const char *reviveOpenAnchor;
    const char *reviveFinishAnchor;
    const char *chestAnchor;
    const char *championPartyAnchor;
    const char *championHandAnchor;
    const char *championSlotAnchor;
    const char *panelAnchor;
    const char *defsAnchor;
    const char *scope;
} Dm1V1MirrorCandidateResurrectFullC30EvidencePc34Compat;

typedef struct Dm1V1MirrorCandidateResurrectFullC30StatePc34Compat {
    int contractOnly;
    unsigned int candidateOrdinal;
    unsigned int activePanelCandidateOrdinal;
    int partyChampionCount;
    int leaderIndex;
    int leaderEmptyHanded;
    unsigned int leaderHandThing;
    int panelContent;
    int c040PanelOpen;
    unsigned int openChestThing;
    unsigned int c30Chain[DM1_V1_MCRFC30_CHAIN_COUNT_PC34_COMPAT];
    unsigned int championSlots[DM1_V1_MCRFC30_CHAMPION_SLOT_COUNT_PC34_COMPAT];
    int fullC30Chain;
    int noEmptyC30RejectCount;
    int noCrashGuard;
    int f0280OpenCount;
    int f0282FinishCount;
    int f0297PutLeaderHandCount;
    int f0298RemoveLeaderHandCount;
    int f0300RemoveSlotCount;
    int f0301AddSlotCount;
    int f0302SlotBoxCount;
    int f0333OpenChestCount;
    int f0334CloseChestCount;
    int f0344PanelClickCount;
    int f0345PanelReleaseCount;
    int f0346DrawC040Count;
    int f0347DrawPanelCount;
    int f0352EyeClickCount;
    int f0359PanelDispatchCount;
    unsigned int deterministicHash;
} Dm1V1MirrorCandidateResurrectFullC30StatePc34Compat;

typedef struct Dm1V1MirrorCandidateResurrectFullC30ResultPc34Compat {
    int accepted;
    int rejectedNoEmptyC30;
    unsigned int candidateBefore;
    unsigned int candidateAfter;
    int panelOpenBefore;
    int panelOpenAfter;
    unsigned int leaderHandBefore;
    unsigned int leaderHandAfter;
    unsigned int c30Slot0Before;
    unsigned int c30Slot0After;
    unsigned int c30Slot7Before;
    unsigned int c30Slot7After;
    int f0282Before;
    int f0282After;
    int f0334Before;
    int f0334After;
    int c30Unchanged;
    int championSlotsUnchanged;
    int cleanFailure;
    int cleanSuccess;
    unsigned int deterministicHashAfter;
    const char *anchor;
} Dm1V1MirrorCandidateResurrectFullC30ResultPc34Compat;

void dm1_v1_mirror_candidate_resurrect_full_c30_init_pc34_compat(
    Dm1V1MirrorCandidateResurrectFullC30StatePc34Compat *state);

int dm1_v1_mirror_candidate_resurrect_full_c30_attempt_pc34_compat(
    Dm1V1MirrorCandidateResurrectFullC30StatePc34Compat *state,
    Dm1V1MirrorCandidateResurrectFullC30ResultPc34Compat *outResult);

void dm1_v1_mirror_candidate_resurrect_full_c30_free_last_slot_pc34_compat(
    Dm1V1MirrorCandidateResurrectFullC30StatePc34Compat *state);

const Dm1V1MirrorCandidateResurrectFullC30EvidencePc34Compat *
dm1_v1_mirror_candidate_resurrect_full_c30_evidence_pc34_compat(void);

const char *
dm1_v1_mirror_candidate_resurrect_full_c30_source_evidence_pc34_compat(void);

int dm1_v1_mirror_candidate_resurrect_full_c30_run_self_test_pc34_compat(void);
int dm1_v1_mirror_candidate_resurrect_full_c30_assertions_pc34_compat(void);
int dm1_v1_mirror_candidate_resurrect_full_c30_failures_pc34_compat(void);
unsigned int dm1_v1_mirror_candidate_resurrect_full_c30_hash_pc34_compat(void);

#ifdef __cplusplus
}
#endif

#endif

/* ReDMCSB anchors: REVIVE.C F0280:124-132 opens G0299/C040 only while the
 * leader hand is empty; REVIVE.C F0282:744-806 owns C160/C161/C162 finish
 * and candidate clearing; CHEST.C F0333:30-67/F0334:113-132 own G0425/G0426;
 * CHAMPION.C F0297:243-268, F0300:511-584, F0301:606-614, and
 * F0302:662-713 own leader-hand/C30 slot metadata; COMMAND.C F0359:1985-1990
 * dispatches M568/C040 panel clicks and F0380:2045-2156 drains queued work;
 * PANEL.C F0346/F0347:1619-1657 keeps C040 as redraw owner while G0299 is
 * set; UTAMSCR.C F0077/F0078:141-150 bracket pointer/redraw updates; OBJECT.C
 * F0033:147-212 and F0038:395-423 preserve icon/slot identity.
 */
#ifndef FIRESTAFF_DM1_V1_MIRROR_CANDIDATE_RESURRECT_CONFIRM_INVENTORY_INTERRUPT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_MIRROR_CANDIDATE_RESURRECT_CONFIRM_INVENTORY_INTERRUPT_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_MIRROR_RCII_NONE_PC34_COMPAT (-1)
#define DM1_V1_MIRROR_RCII_C30_CHEST_SLOT_PC34_COMPAT 30
#define DM1_V1_MIRROR_RCII_C038_SLOT_BOX_PC34_COMPAT 38
#define DM1_V1_MIRROR_RCII_C040_PANEL_PC34_COMPAT 40
#define DM1_V1_MIRROR_RCII_C160_RESURRECT_PC34_COMPAT 160
#define DM1_V1_MIRROR_RCII_C161_REINCARNATE_PC34_COMPAT 161
#define DM1_V1_MIRROR_RCII_M568_PANEL_PC34_COMPAT 568

typedef enum Dm1V1MirrorRciiFinishPc34Compat {
    DM1_V1_MIRROR_RCII_FINISH_RESURRECT_PC34_COMPAT = 1,
    DM1_V1_MIRROR_RCII_FINISH_REINCARNATE_PC34_COMPAT = 2
} Dm1V1MirrorRciiFinishPc34Compat;

typedef struct Dm1V1MirrorRciiEvidencePc34Compat {
    int contractOnly;
    const char *reviveOpenAnchor;
    const char *reviveFinishAnchor;
    const char *chestAnchor;
    const char *championHandAnchor;
    const char *championSlotAnchor;
    const char *commandAnchor;
    const char *panelAnchor;
    const char *utamscrAnchor;
    const char *objectAnchor;
    const char *scope;
} Dm1V1MirrorRciiEvidencePc34Compat;

typedef struct Dm1V1MirrorRciiStatePc34Compat {
    int contractOnly;
    int candidateChampionOrdinal;
    int activePanelCandidateOrdinal;
    int selectedChampionOrdinal;
    int partyChampionCount;
    int leaderEmptyHanded;
    int leaderHandThing;
    int leaderHandIcon;
    int leaderHandObjectNameId;
    int sourceC30Thing;
    int sourceC30Icon;
    int chestSlot0Thing;
    int queuedCommand;
    int queuedSlot;
    int queuedThing;
    int queuedIcon;
    int pendingFinishCommand;
    int panelContent;
    int panelGraphic;
    int panelRedrawOwner;
    int panelRedrawSawCandidate;
    int panelRedrawSawThing;
    int f0280OpenCount;
    int f0282FinishCount;
    int f0297PutLeaderHandCount;
    int f0300RemoveSlotCount;
    int f0301AddSlotCount;
    int f0302SlotBoxCount;
    int f0333OpenChestCount;
    int f0334CloseChestCount;
    int f0346DrawC040Count;
    int f0347DrawPanelCount;
    int f0359PanelDispatchCount;
    int f0380QueueDrainCount;
    int f0077EnableCount;
    int f0078DisableCount;
    int f0033IconLookups;
    int f0038SlotDraws;
    int interruptQueuedCount;
    int interruptDispatchedAfterFinishCount;
    int resurrectFinishCount;
    int reincarnateFinishCount;
    int deterministicHash;
} Dm1V1MirrorRciiStatePc34Compat;

typedef struct Dm1V1MirrorRciiResultPc34Compat {
    int accepted;
    int queued;
    int dispatched;
    int candidateBefore;
    int candidateAfter;
    int pendingFinishBefore;
    int pendingFinishAfter;
    int leaderHandBefore;
    int leaderHandAfter;
    int leaderHandIconBefore;
    int leaderHandIconAfter;
    int sourceC30Before;
    int sourceC30After;
    int sourceC30IconBefore;
    int sourceC30IconAfter;
    int queuedCommandBefore;
    int queuedCommandAfter;
    int panelOwnerBefore;
    int panelOwnerAfter;
    int f0282Before;
    int f0282After;
    int candidateIdentityPreserved;
    int handMetadataPreserved;
    int panelRedrawOwnershipPreserved;
    int queuedMetadataPreserved;
    int deterministicHashAfter;
    const char *anchor;
} Dm1V1MirrorRciiResultPc34Compat;

void dm1_v1_mirror_candidate_rcii_init_pc34_compat(
    Dm1V1MirrorRciiStatePc34Compat *state);

int dm1_v1_mirror_candidate_rcii_begin_confirm_pc34_compat(
    Dm1V1MirrorRciiStatePc34Compat *state,
    Dm1V1MirrorRciiFinishPc34Compat finish,
    Dm1V1MirrorRciiResultPc34Compat *outResult);

int dm1_v1_mirror_candidate_rcii_inventory_interrupt_pc34_compat(
    Dm1V1MirrorRciiStatePc34Compat *state,
    Dm1V1MirrorRciiResultPc34Compat *outResult);

int dm1_v1_mirror_candidate_rcii_finish_confirm_pc34_compat(
    Dm1V1MirrorRciiStatePc34Compat *state,
    Dm1V1MirrorRciiResultPc34Compat *outResult);

const Dm1V1MirrorRciiEvidencePc34Compat *
dm1_v1_mirror_candidate_rcii_evidence_pc34_compat(void);

const char *
dm1_v1_mirror_candidate_rcii_source_evidence_pc34_compat(void);

int dm1_v1_mirror_candidate_rcii_run_self_test_pc34_compat(void);
int dm1_v1_mirror_candidate_rcii_assertions_pc34_compat(void);
int dm1_v1_mirror_candidate_rcii_failures_pc34_compat(void);
int dm1_v1_mirror_candidate_rcii_hash_pc34_compat(void);

#ifdef __cplusplus
}
#endif

#endif

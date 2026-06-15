#ifndef DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_NON_LEADER_PANEL_LIVE_PC34_COMPAT_H
#define DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_NON_LEADER_PANEL_LIVE_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB anchors: CHEST.C F0333:30-67 opens G0426 into C30+ visible
 * chest slots, and F0334:117-132 closes/relinks G0426; CHAMPION.C
 * F0297:243-268, F0298:270-298, F0300:511-584, F0301:606-660, and
 * F0302:662-713 own leader-hand and C30+ slot exchange; COMMAND.C
 * F0378:1973-1983 dispatches scroll pickup and F0380:2045-2159 preserves
 * queued identity; REVIVE.C F0280:124-132 opens G0299/C040 and
 * F0282:744-806 clears it; PANEL.C F0344/F0345 route panel clicks and
 * F0346/F0347:1619-1657 redraw C040; UTAMSCR.C F0077/F0078:141-150
 * brackets pointer redraw; OBJECT.C F0033:147-212 and BLITMASK.C
 * F0133:30-33 preserve icon/mask identity; DEFS.H:338-340, 810-817,
 * 1874-1878, 2085-2088, 2088-2096, 2200, 3001-3008, 5694, and
 * 5876-5881 name C162, C30..C37, C38, G0305, G0423, C040, M568/M569,
 * G0299, G0425, and G0426.
 */

#define DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_NON_LEADER_PANEL_LIVE_SLOT_COUNT_PC34_COMPAT 8
#define DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_NON_LEADER_PANEL_LIVE_PARTY_COUNT_PC34_COMPAT 4

enum {
    DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_NON_LEADER_PANEL_LIVE_NONE_PC34_COMPAT = 0,
    DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_NON_LEADER_PANEL_LIVE_C162_PC34_COMPAT = 162,
    DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_NON_LEADER_PANEL_LIVE_C30_PC34_COMPAT = 30,
    DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_NON_LEADER_PANEL_LIVE_C38_PC34_COMPAT = 38,
    DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_NON_LEADER_PANEL_LIVE_C040_PC34_COMPAT = 40,
    DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_NON_LEADER_PANEL_LIVE_C538_PC34_COMPAT = 538,
    DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_NON_LEADER_PANEL_LIVE_M568_PC34_COMPAT = 5,
    DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_NON_LEADER_PANEL_LIVE_M569_PC34_COMPAT = 4
};

typedef struct Dm1V1MirrorCandidateScrollPickupNonLeaderPanelLiveEvidencePc34Compat {
    int contractOnly;
    const char *chestOpenAnchor;
    const char *chestCloseAnchor;
    const char *leaderHandPutAnchor;
    const char *leaderHandRemoveAnchor;
    const char *slotRemoveAnchor;
    const char *slotAddAnchor;
    const char *slotDispatchAnchor;
    const char *scrollDispatchAnchor;
    const char *queueAnchor;
    const char *candidateOpenAnchor;
    const char *candidateCancelAnchor;
    const char *panelClickAnchor;
    const char *panelRedrawAnchor;
    const char *mouseAnchor;
    const char *objectAnchor;
    const char *blitMaskAnchor;
    const char *defsAnchor;
    const char *contractScope;
} Dm1V1MirrorCandidateScrollPickupNonLeaderPanelLiveEvidencePc34Compat;

typedef struct Dm1V1MirrorCandidateScrollPickupNonLeaderPanelLiveSpecPc34Compat {
    int partyCount;
    int leaderIndex;
    int partyTailChampion;
    unsigned int candidateOrdinal;
    int leaderOpenChestThing;
    int partyTailChestThing;
    int scrollThing;
    int nonLeaderSlotIndex;
    int nonLeaderSlotId;
    int nonLeaderSlotBox;
    int nonLeaderDisplayZone;
    int c040PanelGraphic;
    int resurrectPanelId;
    int chestPanelId;
    int cancelCommand;
} Dm1V1MirrorCandidateScrollPickupNonLeaderPanelLiveSpecPc34Compat;

typedef struct Dm1V1MirrorCandidateScrollPickupNonLeaderPanelLiveStatePc34Compat {
    int contractOnly;
    int partyCount;
    int leaderIndex;
    int partyTailChampion;
    unsigned int candidateOrdinal;
    unsigned int g0299CandidateOrdinal;
    int panelOpen;
    int panelGraphic;
    int panelRedrawable;
    int leaderHandThing;
    int leaderOpenChestThing;
    int partyTailChestThing;
    int openChestThing;
    int inventoryChampionOrdinal;
    int activeSlotBox;
    int nonLeaderSlotThing;
    int nonLeaderSlotClearedDuringPickup;
    int nonLeaderSlotReplacedDuringPickup;
    int followUpCancelRequested;
    int candidateClearedByCancel;
    int chestSlots
        [DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_NON_LEADER_PANEL_LIVE_SLOT_COUNT_PC34_COMPAT];
    int f0333OpenCount;
    int f0334CloseCount;
    int f0297PutCount;
    int f0298RemoveCount;
    int f0300SlotRemoveCount;
    int f0301SlotAddCount;
    int f0302DispatchCount;
    int f0378DispatchCount;
    int f0380QueueCount;
    int f0280OpenCount;
    int f0282CancelCount;
    int f0344PanelClickCount;
    int panelRedrawCount;
    int mouseEnableCount;
    int mouseDisableCount;
} Dm1V1MirrorCandidateScrollPickupNonLeaderPanelLiveStatePc34Compat;

typedef struct Dm1V1MirrorCandidateScrollPickupNonLeaderPanelLiveResultPc34Compat {
    const Dm1V1MirrorCandidateScrollPickupNonLeaderPanelLiveEvidencePc34Compat
        *evidence;
    const Dm1V1MirrorCandidateScrollPickupNonLeaderPanelLiveSpecPc34Compat
        *spec;
    int accepted;
    int f0333OpenCount;
    int f0334CloseCount;
    int f0297PutCount;
    int f0298RemoveCount;
    int f0300SlotRemoveCount;
    int f0301SlotAddCount;
    int f0302DispatchCount;
    int f0378DispatchCount;
    int f0380QueueCount;
    int f0280OpenCount;
    int f0282CancelCount;
    int f0344PanelClickCount;
    int panelRedrawCount;
    int mouseEnableCount;
    int mouseDisableCount;
    int leaderHandThingAfter;
    int nonLeaderSlotCleared;
    int nonLeaderSlotReplaced;
    unsigned int candidateOrdinal;
    int panelOpen;
    int openChestThing;
    int partyTailChampion;
    int followUpCancelClearsCandidate;
    int mutationGuardsOk;
    int candidateOrdinalBefore;
    int candidateOrdinalAfterPickup;
    int candidateOrdinalAfterCancel;
    int panelOpenBefore;
    int panelOpenAfterPickup;
    int panelOpenAfterCancel;
    int panelRedrawableAfterPickup;
    int openChestBefore;
    int openChestAfterPickup;
    int openChestAfterCancel;
    int partyTailChampionBefore;
    int partyTailChampionAfterPickup;
    int leaderOpenChestBefore;
    int leaderOpenChestAfterPickup;
    int partyTailChestBefore;
    int partyTailChestAfterPickup;
    int leaderHandBefore;
    int nonLeaderSlotBefore;
    int nonLeaderSlotAfterPickup;
    int activeSlotBoxBefore;
    int activeSlotBoxAfterPickup;
    int inventoryChampionOrdinalBefore;
    int inventoryChampionOrdinalAfterPickup;
    int chestSlotsBefore
        [DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_NON_LEADER_PANEL_LIVE_SLOT_COUNT_PC34_COMPAT];
    int chestSlotsAfterPickup
        [DM1_V1_MIRROR_CANDIDATE_SCROLL_PICKUP_NON_LEADER_PANEL_LIVE_SLOT_COUNT_PC34_COMPAT];
} Dm1V1MirrorCandidateScrollPickupNonLeaderPanelLiveResultPc34Compat;

void DM1_V1_MirrorCandidateScrollPickupNonLeaderPanelLive_InitPc34Compat(
    Dm1V1MirrorCandidateScrollPickupNonLeaderPanelLiveStatePc34Compat *state);

int DM1_V1_MirrorCandidateScrollPickupNonLeaderPanelLive_RunPc34Compat(
    Dm1V1MirrorCandidateScrollPickupNonLeaderPanelLiveStatePc34Compat *state,
    Dm1V1MirrorCandidateScrollPickupNonLeaderPanelLiveResultPc34Compat
        *outResult);

const Dm1V1MirrorCandidateScrollPickupNonLeaderPanelLiveEvidencePc34Compat *
DM1_V1_MirrorCandidateScrollPickupNonLeaderPanelLive_EvidencePc34Compat(void);

const Dm1V1MirrorCandidateScrollPickupNonLeaderPanelLiveSpecPc34Compat *
DM1_V1_MirrorCandidateScrollPickupNonLeaderPanelLive_SpecPc34Compat(void);

const char *
DM1_V1_MirrorCandidateScrollPickupNonLeaderPanelLive_SourceEvidencePc34Compat(
    void);

#ifdef __cplusplus
}
#endif

#endif

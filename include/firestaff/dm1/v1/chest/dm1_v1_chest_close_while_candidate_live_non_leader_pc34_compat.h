#ifndef FIRESTAFF_DM1_V1_CHEST_CLOSE_WHILE_CANDIDATE_LIVE_NON_LEADER_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_CLOSE_WHILE_CANDIDATE_LIVE_NON_LEADER_PC34_COMPAT_H

#include "dm1_v1_inventory_pc34_compat.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * DM1 V1 non-leader G0426 close while another champion owns a live C040
 * resurrect/reincarnate mirror candidate.
 *
 * ReDMCSB anchors:
 * - CHEST.C F0333:30-67 materializes the first eight G0425 visible links.
 * - CHEST.C F0334:113-132 clears G0426, rewrites non-empty visible slots,
 *   and truncates any hidden tail beyond the visible C30..C37 window.
 * - CHAMPION.C F0297:243-298 and F0298:270-298 own leader-hand state.
 * - CHAMPION.C F0300:511-515 and F0301:606-614 clear/write C30+ slots
 *   through G0425; F0302:662-714 routes C537..C544 slot-box clicks.
 * - REVIVE.C F0280:124-132 publishes a C040 candidate and F0282:744-806
 *   consumes/clears it only through resurrect/reincarnate/cancel commands.
 * - COMMAND.C F0359:1985-1990 routes live C040 panel commands.
 * - DEFS.H binds C040, C537..C544, C030, G0425, and G0426.
 */

enum {
    DM1_PC34_CCLNL_SLOT_COUNT = DM1_PC34_CHEST_SLOT_COUNT,
    DM1_PC34_CCLNL_HIDDEN_TAIL_COUNT = 2,
    DM1_PC34_CCLNL_CHAMPION_COUNT = 4,
    DM1_PC34_CCLNL_LEADER = 0,
    DM1_PC34_CCLNL_NON_LEADER_OWNER = 1,
    DM1_PC34_CCLNL_CANDIDATE_OWNER = 2,
    DM1_PC34_CCLNL_OTHER_CHAMPION = 3,
    DM1_PC34_CCLNL_OPEN_CHEST_THING = 0x7661,
    DM1_PC34_CCLNL_FIRST_ITEM = 0x7670,
    DM1_PC34_CCLNL_FIRST_WEIGHT = 11,
    DM1_PC34_CCLNL_FIRST_CHARGES = 41,
    DM1_PC34_CCLNL_C030_SOURCE_SLOT = DM1_PC34_SLOT_CHEST_1,
    DM1_PC34_CCLNL_C038_PANEL_CHROME = 38,
    DM1_PC34_CCLNL_C039_PANEL_CHROME = 39,
    DM1_PC34_CCLNL_C040_PANEL_CHROME = 40,
    DM1_PC34_CCLNL_C039_REJECTED_PANEL_CLICK = 39,
    DM1_PC34_CCLNL_C537_ZONE = 537,
    DM1_PC34_CCLNL_C540_ZONE = 540,
    DM1_PC34_CCLNL_C544_ZONE = 544,
    DM1_PC34_CCLNL_C537_SLOT_BOX = 38,
    DM1_PC34_CCLNL_C540_SLOT_BOX = 41,
    DM1_PC34_CCLNL_CLOSE_BUTTON_COMMAND = 45,
    DM1_PC34_CCLNL_CLOSE_BUTTON_ZONE = 503,
    DM1_PC34_CCLNL_M568_RESURRECT_PANEL = DM1_PC34_PANEL_RESURRECT_REINCARNATE
};

typedef struct {
    int type;
    int weight;
    int charges;
    int allowedSlots;
} DM1_V1_ChestCloseWhileCandidateLiveNonLeaderItemPc34;

typedef struct {
    const char* contractMarker;
    const char* chestOpenAnchor;
    const char* chestCloseAnchor;
    const char* championHandAnchor;
    const char* championSlotAnchor;
    const char* reviveOpenAnchor;
    const char* revivePendingAnchor;
    const char* commandAnchor;
    const char* defsAnchor;
    const char* disjointnessNote;
} DM1_V1_ChestCloseWhileCandidateLiveNonLeaderSpecPc34;

typedef struct {
    int sourceLockedContractOnly;
    int assetFree;
    int stepTrace[5];
    int stepCount;

    int leader;
    int nonLeaderOwner;
    int candidateOwner;
    int partyChampionCount;

    int openResult;
    int openChestOwnerBeforeClose;
    int openChestThingBeforeClose;
    int openChestThingAfterClose;
    int closeCommand;
    int closeButtonZone;
    int closeCount;
    int closeClearedOnlyOwnerG0426;
    int ownerClosedOnly;

    int panelBeforeClose;
    int panelAfterClose;
    int c038ChromeBeforeClose;
    int c038ChromeAfterClose;
    int c039ChromeBeforeClose;
    int c039ChromeAfterClose;
    int c040ChromeBeforeClose;
    int c040ChromeAfterClose;
    int c040PanelRoutePreserved;
    int c038C039C040ChromePreserved;

    int candidateOrdinalBeforeClose;
    int candidateOrdinalAfterClose;
    int candidateOwnerBeforeClose;
    int candidateOwnerAfterClose;
    int candidateSlotBeforeClose;
    int candidateSlotAfterClose;
    int candidateLiveBeforeClose;
    int candidateLiveAfterClose;
    int candidatePreservedAcrossClose;

    int rejectedPanelClickCommand;
    int rejectedPanelClickDuringClose;
    int rejectedPanelClickWouldHaveOpenedViaF0333;
    int f0333OpenCountBeforeRejectedClick;
    int f0333OpenCountAfterRejectedClick;
    int f0333OpenCountAfterClose;

    int c540Zone;
    int c540SlotBox;
    int c540Pc34Slot;
    DM1_V1_ChestCloseWhileCandidateLiveNonLeaderItemPc34 c540ItemBeforeClose;
    DM1_V1_ChestCloseWhileCandidateLiveNonLeaderItemPc34 c540ItemAfterClose;
    int c540PanelRoutePreserved;

    DM1_V1_ChestCloseWhileCandidateLiveNonLeaderItemPc34 visibleBefore[
        DM1_PC34_CCLNL_SLOT_COUNT];
    DM1_V1_ChestCloseWhileCandidateLiveNonLeaderItemPc34 closedChain[
        DM1_PC34_CCLNL_SLOT_COUNT];
    DM1_V1_ChestCloseWhileCandidateLiveNonLeaderItemPc34 hiddenTail[
        DM1_PC34_CCLNL_HIDDEN_TAIL_COUNT];
    int closedVisibleThingCount;
    int hiddenTailTruncated;
    int visibleSlotChainRewritten;

    int leaderHandBefore[DM1_PC34_CCLNL_CHAMPION_COUNT];
    int leaderHandAfter[DM1_PC34_CCLNL_CHAMPION_COUNT];
    uint32_t c030ChainHashBefore[DM1_PC34_CCLNL_CHAMPION_COUNT];
    uint32_t c030ChainHashAfter[DM1_PC34_CCLNL_CHAMPION_COUNT];
    int leaderHandC030ChainsPreserved;

    int f0333OpenCount;
    int f0334CloseCount;
    int f0297PutLeaderHandCount;
    int f0298RemoveLeaderHandCount;
    int f0300RemoveC030Count;
    int f0301AddC030Count;
    int f0302SlotBoxCount;
    int f0280CandidatePublishCount;
    int f0282CandidateConsumeCount;
    int f0359C040DispatchCount;

    uint32_t deterministicHash;
    int modelAssertions;
    int modelFailures;
} DM1_V1_ChestCloseWhileCandidateLiveNonLeaderProbePc34;

const char*
dm1_v1_chest_close_while_candidate_live_non_leader_source_evidence_pc34(void);

const DM1_V1_ChestCloseWhileCandidateLiveNonLeaderSpecPc34*
dm1_v1_chest_close_while_candidate_live_non_leader_spec_pc34(void);

int dm1_v1_chest_close_while_candidate_live_non_leader_run_pc34(
    DM1_V1_ChestCloseWhileCandidateLiveNonLeaderProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif

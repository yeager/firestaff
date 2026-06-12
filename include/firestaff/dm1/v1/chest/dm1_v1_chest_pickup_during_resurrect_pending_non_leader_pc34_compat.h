#ifndef FIRESTAFF_DM1_V1_CHEST_PICKUP_DURING_RESURRECT_PENDING_NON_LEADER_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_PICKUP_DURING_RESURRECT_PENDING_NON_LEADER_PC34_COMPAT_H

#include "dm1_v1_inventory_pc34_compat.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * DM1 V1 chest C537 pickup during resurrect-pending non-leader close gate.
 *
 * ReDMCSB anchors:
 * - CHEST.C F0333:30-67 opens G0426 and materializes C537..C544/G0425.
 * - CHEST.C F0334:113-132 closes G0426 and relinks non-empty G0425 slots.
 * - CHAMPION.C F0297:243-298 and F0298:270-298 own leader hand state.
 * - CHAMPION.C F0300:511-515 and F0301:606-614 clear/write C30+ slots.
 * - CHAMPION.C F0302:662-714 routes C537..C544 panel clicks.
 * - CHAMPION.C F0284:93-131 covers the party direction / leader context.
 * - REVIVE.C F0280:124-132 and F0282:744-806 own C040 pending candidate
 *   publish, confirm/cancel, and clear.
 * - PANEL.C F0344:113-145, F0345:155-200, and F0352:2111-2160 cover
 *   panel redraw / eye routing boundaries while C040 chrome is live.
 * - COMMAND.C F0359:1985-1990 and F0378:1973-1983 cover C040 and chest
 *   panel pointer dispatch.
 * - DEFS.H:2088 and 3906-3913 bind C10 and C537..C544; C30/G0425/G0426/
 *   G0423/G0305/M070/M516/C040 are the companion globals/constants.
 */

enum {
    DM1_PC34_CPRPNL_SLOT_COUNT = DM1_PC34_CHEST_SLOT_COUNT,
    DM1_PC34_CPRPNL_LEADER_BEFORE = 0,
    DM1_PC34_CPRPNL_NON_LEADER_OWNER = 1,
    DM1_PC34_CPRPNL_NEW_LEADER_AFTER_RESURRECT = 2,
    DM1_PC34_CPRPNL_CHAMPION_COUNT = 3,
    DM1_PC34_CPRPNL_CHEST_THING = 0x7650,
    DM1_PC34_CPRPNL_FIRST_ITEM = 0x7660,
    DM1_PC34_CPRPNL_FIRST_WEIGHT = 9,
    DM1_PC34_CPRPNL_FIRST_CHARGES = 31,
    DM1_PC34_CPRPNL_C537_ZONE = 537,
    DM1_PC34_CPRPNL_C544_ZONE = 544,
    DM1_PC34_CPRPNL_C537_SLOT_BOX = 38,
    DM1_PC34_CPRPNL_C30_SOURCE_SLOT = DM1_PC34_SLOT_CHEST_1,
    DM1_PC34_CPRPNL_C040_GRAPHIC = 40,
    DM1_PC34_CPRPNL_M568_RESURRECT_PANEL = DM1_PC34_PANEL_RESURRECT_REINCARNATE,
    DM1_PC34_CPRPNL_CLOSE_COMMAND_C045 = 45,
    DM1_PC34_CPRPNL_CLOSE_BUTTON_C503 = 503
};

typedef struct {
    const char* contractMarker;
    const char* chestOpenAnchor;
    const char* chestCloseAnchor;
    const char* championHandAnchor;
    const char* championSlotAnchor;
    const char* championDirectionAnchor;
    const char* reviveOpenAnchor;
    const char* revivePendingAnchor;
    const char* panelAnchor;
    const char* commandAnchor;
    const char* defsAnchor;
    const char* disjointnessNote;
} DM1_V1_ChestPickupDuringResurrectPendingNonLeaderSpecPc34;

typedef struct {
    int type;
    int weight;
    int charges;
    int allowedSlots;
} DM1_V1_ChestPickupDuringResurrectPendingNonLeaderItemPc34;

typedef struct {
    int sourceLockedContractOnly;
    int assetFree;
    int stepTrace[6];
    int stepCount;

    int leaderBefore;
    int nonLeaderOwner;
    int newLeaderAfterResurrect;
    int partyChampionCountBefore;
    int partyChampionCountAfterCommit;
    int partyDirectionBefore;
    int partyDirectionAfterCommit;

    int openResult;
    int openChestThingBeforePending;
    int panelAfterOpen;
    int c040PanelAfterPending;
    int c040ChromeBeforeClose;
    int c040ChromeAfterClose;
    int candidateOrdinalBeforeClose;
    int candidateOrdinalAfterClose;
    int candidateOrdinalAfterCommit;
    int candidateSlotBeforeClose;
    int candidateSlotAfterClose;
    int candidateSlotPreservedAcrossClose;
    int c040ChromePreservedAcrossClose;

    int queuedCommand;
    int queuedZone;
    int queuedSlotBox;
    int queuedPc34Slot;
    int queuedOwner;
    int queuedOpenChestThing;
    int queuedBeforeClose;
    int queueReservedC537;
    int queuePreservedAcrossClose;
    DM1_V1_ChestPickupDuringResurrectPendingNonLeaderItemPc34 queuedItem;

    int closeCommand;
    int closeButtonZone;
    int closeCount;
    int openChestThingAfterClose;
    int closeClearedG0426;
    int closeCompactedCleanly;
    int closedPickedCopies;
    DM1_V1_ChestPickupDuringResurrectPendingNonLeaderItemPc34 closedChain[
        DM1_PC34_CPRPNL_SLOT_COUNT];

    int resurrectCommitResult;
    int leaderAfterCommit;
    int f0282ClearedCandidate;
    int panelAfterCommit;
    int pickupResolveResult;
    int pickupResolvedAfterCommit;
    int pickupBlockedBeforeCommit;
    int pickupLandedInNewLeaderHand;
    int pickupLandedInLeaderC30Chain;
    int newLeaderHandType;
    int newLeaderHandWeight;
    int newLeaderHandCharges;
    int pickedCopiesIncludingHand;

    int f0333OpenCount;
    int f0334CloseCount;
    int f0300ReserveCount;
    int f0297PutCount;
    int f0302DispatchCount;
    int f0282CommitCount;
    int f0359PanelDispatchCount;
    int f0378ChestDispatchCount;

    uint32_t deterministicHash;
    int modelAssertions;
    int modelFailures;
} DM1_V1_ChestPickupDuringResurrectPendingNonLeaderProbePc34;

const char*
dm1_v1_chest_pickup_during_resurrect_pending_non_leader_source_evidence_pc34(
    void);
const DM1_V1_ChestPickupDuringResurrectPendingNonLeaderSpecPc34*
dm1_v1_chest_pickup_during_resurrect_pending_non_leader_spec_pc34(void);
int dm1_v1_chest_pickup_during_resurrect_pending_non_leader_run_pc34(
    DM1_V1_ChestPickupDuringResurrectPendingNonLeaderProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif

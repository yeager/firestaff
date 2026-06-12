#ifndef FIRESTAFF_DM1_V1_CHEST_C545_NON_LEADER_HAND_TO_MID_CAST_LEADER_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_C545_NON_LEADER_HAND_TO_MID_CAST_LEADER_PC34_COMPAT_H

/*
 * DM1 V1 C545 non-leader hand back to mid-cast leader runtime gate.
 *
 * ReDMCSB anchors:
 * - CHEST.C F0333:30-67 materializes G0425 from the open G0426 chest.
 * - CHEST.C F0334:113-132 rewrites the open chest from non-empty G0425.
 * - CHAMPION.C F0284:93-131 keeps champion ownership independent from
 *   party direction updates.
 * - CHAMPION.C F0297:243-298 and F0298:270-298 define leader-hand put/remove.
 * - CHAMPION.C F0300:511-515 clears C30+ and champion slots.
 * - CHAMPION.C F0301:606-614 writes C30+/champion slots.
 * - CHAMPION.C F0302:662-714 snapshots leader hand and selected slot before
 *   the remove/put exchange.
 * - PANEL.C F0344/F0345/F0352 and COMMAND.C F0359:1985-1990 are cited to
 *   pin the inactive food/water, eye, and resurrect-panel side routes.
 * - DEFS.H:2088, 5876-5881, 810-817, and 3906-3914 define C30, G0425,
 *   G0426, G0423, G0305, M070/M516 context, and C537..C545.
 */

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_PC34_C545_MIDCAST_SLOT_COUNT = 8,
    DM1_PC34_C545_MIDCAST_CHAMPION_COUNT = 4,
    DM1_PC34_C545_MIDCAST_NONE = 0xFFFF,
    DM1_PC34_C545_MIDCAST_END = 0xFFFE,
    DM1_PC34_C545_MIDCAST_C30 = 30,
    DM1_PC34_C545_MIDCAST_C37 = 37,
    DM1_PC34_C545_MIDCAST_C537 = 537,
    DM1_PC34_C545_MIDCAST_C540 = 540,
    DM1_PC34_C545_MIDCAST_C541 = 541,
    DM1_PC34_C545_MIDCAST_C544 = 544,
    DM1_PC34_C545_MIDCAST_C545 = 545,
    DM1_PC34_C545_MIDCAST_C070 = 70,
    DM1_PC34_C545_MIDCAST_M568 = 568,
    DM1_PC34_C545_MIDCAST_M569 = 569,
    DM1_PC34_C545_MIDCAST_LEADER_INDEX = 0,
    DM1_PC34_C545_MIDCAST_LEADER_ORDINAL = 1,
    DM1_PC34_C545_MIDCAST_OWNER_INDEX = 2,
    DM1_PC34_C545_MIDCAST_OWNER_ORDINAL = 3,
    DM1_PC34_C545_MIDCAST_OPEN_CHEST = 0x7460,
    DM1_PC34_C545_MIDCAST_SCROLL = 0x7461,
    DM1_PC34_C545_MIDCAST_CHEST_BASE = 0x7470,
    DM1_PC34_C545_MIDCAST_LOAD_MASK = 0x0200,
    DM1_PC34_C545_MIDCAST_PANEL_MASK = 0x0800,
    DM1_PC34_C545_MIDCAST_VIEWPORT_MASK = 0x4000,
    DM1_PC34_C545_MIDCAST_ACTION_HAND_MASK = 0x8000
};

typedef struct {
    const char* sourceEvidence;
    const char* f0333OpenAnchor;
    const char* f0334CloseAnchor;
    const char* f0284Anchor;
    const char* f0297Anchor;
    const char* f0298Anchor;
    const char* f0300Anchor;
    const char* f0301Anchor;
    const char* f0302Anchor;
    const char* f0344Anchor;
    const char* f0345Anchor;
    const char* f0352Anchor;
    const char* f0359Anchor;
    const char* defsAnchor;
} DM1_V1_ChestC545NonLeaderHandToMidCastLeaderSpecPc34;

typedef struct {
    int sourceLockedRuntimeGate;
    int noGameDataLoad;
    int partyChampionCount;
    int leaderIndex;
    int leaderOrdinal;
    int inventoryChampionIndex;
    int inventoryChampionOrdinal;
    int actingChampionOrdinalBefore;
    int actingChampionOrdinalAfter;
    int leaderMidCastBefore;
    int leaderMidCastAfter;
    int leaderSpellRuneCountBefore;
    int leaderSpellRuneCountAfter;
    int openChestThingBefore;
    int openChestThingAfter;
    int leaderHandBefore;
    int leaderHandAfter;
    int leaderHandIconAfter;
    int nonLeaderActionHandBefore;
    int nonLeaderActionHandAfter;
    int nonLeaderActionHandIconBefore;
    int nonLeaderActionHandClosedAfter;
    int leaderLoadBefore;
    int leaderLoadAfter;
    int nonLeaderLoadBefore;
    int nonLeaderLoadAfter;
    int leaderAttributesAfter;
    int nonLeaderAttributesAfter;
    int c545EventZone;
    int c545Command;
    int commandPanelBefore;
    int commandF0359ResurrectBlockedAfterPickup;
    int f0302LeaderSnapshot;
    int f0302SlotSnapshot;
    int f0302ChampionIndex;
    int f0302Pc34Slot;
    int f0302AllowedBySlotMask;
    int f0302EmptyEmptyRejected;
    int f0297PutCount;
    int f0298RemoveCount;
    int f0300ClearCount;
    int f0301WriteCount;
    int f0388ClearActingCount;
    int f0344FoodBarDrawCount;
    int f0345FoodWaterPanelDrawCount;
    int f0352EyePanelDrawCount;
    int panelRedrawAfterC545;
    int screenUpdateBalanced;
    int chestBefore[DM1_PC34_C545_MIDCAST_SLOT_COUNT];
    int chestAfter[DM1_PC34_C545_MIDCAST_SLOT_COUNT];
    int closedChest[DM1_PC34_C545_MIDCAST_SLOT_COUNT];
    int closedChestCount;
    int c540Preserved;
    int c541Preserved;
    int chestOrderPreserved;
    int negativeLeaderBusyRejected;
    int negativeLeaderHandAfter;
    int negativeNonLeaderHandAfter;
    int negativeActingChampionAfter;
    unsigned int deterministicHash;
} DM1_V1_ChestC545NonLeaderHandToMidCastLeaderProbePc34;

const DM1_V1_ChestC545NonLeaderHandToMidCastLeaderSpecPc34*
dm1_v1_chest_c545_non_leader_hand_to_mid_cast_leader_spec_pc34(void);

const char*
dm1_v1_chest_c545_non_leader_hand_to_mid_cast_leader_source_evidence_pc34(void);

int dm1_v1_chest_c545_non_leader_hand_to_mid_cast_leader_run_pc34(
    DM1_V1_ChestC545NonLeaderHandToMidCastLeaderProbePc34* out);

#ifdef __cplusplus
}
#endif

#endif

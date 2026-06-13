#ifndef FIRESTAFF_DM1_V1_CHAMPION_PANEL_DEAD_MEMBER_HAND_REFRESH_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_PANEL_DEAD_MEMBER_HAND_REFRESH_PC34_COMPAT_H

/*
 * DM1 V1 champion-panel dead-member hand refresh gate.
 *
 * Source-locked contract-only gate that pins the CHAMDRAW.C F0295/F0296
 * + ACTIDRAW.C F0386 dead-member action-hand refresh path. The lane
 * name is `dm1_v1_auto_champion_panel_dead_member_hand_refresh_gate`.
 *
 * ReDMCSB anchors:
 * - CHAMDRAW.C F0295_CHAMPION_HasObjectIconInSlotBoxChanged:1153-1182
 *   compares the slotbox's currently displayed icon against
 *   F0033_OBJECT_GetIconIndex(thing) for the slot at
 *   M070_HAND_SLOT_INDEX(slotBoxIndex) and redraws the slot via
 *   F0038_OBJECT_DrawIconInSlotBox when the icons differ. The mutable
 *   icon ranges are C000..C031 (junk), C148..C163 (potions), and the
 *   C195 empty flask.
 * - CHAMDRAW.C F0296_CHAMPION_DrawChangedObjectIcons:1184-1262 walks
 *   slotbox indices 0..(G0305_ui_PartyChampionCount << 1)-1 with
 *   L0885_i_ChampionIndex = slotBoxIndex >> 1 and
 *   M070_HAND_SLOT_INDEX(slotBoxIndex) == C01_SLOT_ACTION_HAND; for
 *   each action-hand slotbox where F0295 returns C1_TRUE, the loop
 *   calls F0386_MENUS_DrawActionIcon(L0885_i_ChampionIndex) at line
 *   1231. The F0296 walk never skips a dead champion.
 * - ACTIDRAW.C F0386_MENUS_DrawActionIcon:201-326 is the dead
 *   champion guard: lines 234-242 read
 *   L1183_ps_Champion->CurrentHealth, when zero fill the
 *   C089_ZONE_ACTION_AREA_CHAMPION_0_ACTION zone with
 *   C00_COLOR_BLACK and return; the action icon blit at lines 257-326
 *   is unreachable while CurrentHealth == 0.
 * - CHAMDRAW.C F0292_CHAMPION_DrawState:816-839 is the dead-status-box
 *   branch: it draws C008_GRAPHIC_STATUS_BOX_DEAD_CHAMPION into the
 *   C151+championIndex status-box zone and calls
 *   F0386_MENUS_DrawActionIcon(P0615_ui_ChampionIndex) at line 835
 *   before the T0292042 return. The F0292 call site for the
 *   dead champion goes through the F0386 dead-member guard.
 * - DEFS.H:781 C01_SLOT_ACTION_HAND, 1878 M070_HAND_SLOT_INDEX, 1950
 *   C195_ICON_POTION_EMPTY_FLASK, 1952 C201_ICON_ACTION_ICON_EMPTY_HAND,
 *   2078 C00_COLOR_BLACK, 2172 C008_GRAPHIC_STATUS_BOX_DEAD_CHAMPION,
 *   3770 C089_ZONE_ACTION_AREA_CHAMPION_0_ACTION pin the constants.
 *
 * Non-overlap marker: this gate covers the F0296 -> F0295 + F0386
 * dead-member hand refresh (a party of N champions with at least one
 * dead member, the dead member's action hand refresh writes a
 * C00_COLOR_BLACK fill to the action-area zone, the live members'
 * action hand refresh writes their respective icon blit), and the
 * F0292 dead-status-box branch that calls F0386 for the dead
 * champion. It is disjoint from champion_panel_portrait_box_blit_gate
 * (F0292+F0354 dispatch + F0254 secondary dispatch + F0293 dispatch
 * order, not F0296 dead-member hand refresh), from
 * champion_panel_portrait_box_redraw_states (F0296 changed-icon scan
 * + chest scan, no dead-champion handling), from
 * champion_panel_portrait_state_redraw (F0292 dead-status-box branch
 * state redraw order, not F0296 dead-member hand refresh), from
 * champion_panel_hand_slot_priority (CHAMPION.C F0302 dispatch, not
 * F0296 dead-member hand refresh), from
 * mirror_candidate_icon_refresh (F0296 leader hand candidate-icon
 * scan, not F0296 dead-member hand refresh), from
 * champion_panel_spell_area_overlay (F0394 dead-champion reject for
 * spell area, not F0296 dead-member hand refresh), from
 * mirror_candidate_c045_accept_dead_owner_guard (C045 accept while
 * owner dead, not F0296 dead-member hand refresh), from
 * inventory_champion_switch_hand_carry (dead target leaves leader
 * hand unchanged, not F0296 dead-member hand refresh), from the
 * F0107/F0108/chest-scroll-wheel/viewport/integrated family, and from
 * the per-state redraw + per-action-hand slot-box dispatch family.
 *
 * Contract only: this slice models the F0296/F0295/F0386 dead-member
 * action-hand refresh walk, the F0292 dead-status-box branch F0386
 * dispatch, the F0033_OBJECT_GetIconIndex / F0038_OBJECT_DrawIconInSlotBox
 * helper pair that F0295 invokes for the live members, the
 * C089+championIndex action-area zone fill, the action icon never
 * reaching the live-member blit for a dead member, the
 * F0077/F0078 mouse screen-update bracketing on the F0296 entry, and
 * the C00_COLOR_BLACK fill of the action-area zone for the dead
 * member across one or more F0296 invocations. It does not call real
 * M11 graphics and does not claim real-asset bitmap parity.
 */

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_DMHR_PARTY_COUNT_PC34 4
#define DM1_V1_DMHR_SLOTBOX_PER_CHAMPION_PC34 2
#define DM1_V1_DMHR_TRACE_COUNT_PC34 8
#define DM1_V1_DMHR_THING_NONE_PC34 0xffffu
#define DM1_V1_DMHR_C01_SLOT_ACTION_HAND_PC34 1
#define DM1_V1_DMHR_C089_ACTION_AREA_ZONE_BASE_PC34 89

typedef enum {
    DM1_V1_DMHR_PATH_INVALID_PC34 = 0,
    DM1_V1_DMHR_PATH_DEAD_MEMBER_F0296_REFRESH_PC34,
    DM1_V1_DMHR_PATH_DEAD_MEMBER_F0292_STATUS_BOX_REFRESH_PC34,
    DM1_V1_DMHR_PATH_LIVE_MEMBER_F0296_REFRESH_PC34,
    DM1_V1_DMHR_PATH_REJECTED_NO_DEAD_MEMBER_PC34
} Dm1V1ChampionPanelDeadMemberHandRefreshPathPc34;

typedef struct {
    const char *redrawF0295Anchor;
    const char *redrawF0296Anchor;
    const char *redrawF0386Anchor;
    const char *redrawF0292Anchor;
    const char *objectF0033Anchor;
    const char *objectF0038Anchor;
    const char *defsAnchor;
    const char *zoneAnchor;
    const char *contractScope;
    const char *noRealGraphicsClaim;
    const char *nonOverlap;
} Dm1V1ChampionPanelDeadMemberHandRefreshEvidencePc34;

typedef struct {
    int championIndex;
    int alive;
    int leader;
    uint16_t actionHandThing;
    int objectIconIndex;
    int slotBoxCurrentIcon;
    int iconChanged;
    int actionAreaZone;
    int blackFillCount;
    int actionIconBlitCount;
} Dm1V1ChampionPanelDeadMemberHandRefreshChampionPc34;

typedef struct {
    int contractOnly;
    int assetFree;
    int partyChampionCount;
    int leaderIndex;
    int deadMemberIndex;
    int aliveMembers;
    int f0296InvocationCount;
    int f0292InvocationCount;
    int f0295HasIconChangedCount;
    int f0295SameIconCount;
    int f0033GetIconIndexCount;
    int f0038DrawIconInSlotBoxCount;
    int f0386DrawActionIconCount;
    int f0077MouseEnableCount;
    int f0078MouseDisableCount;
    int leaderHandThingThing[DM1_V1_DMHR_PARTY_COUNT_PC34];
    int leaderHandObjectIcon[DM1_V1_DMHR_PARTY_COUNT_PC34];
    int leaderHandSlotBoxCurrentIcon[DM1_V1_DMHR_PARTY_COUNT_PC34];
    int leaderHandIconChanged[DM1_V1_DMHR_PARTY_COUNT_PC34];
    int leaderHandBlackFillCount[DM1_V1_DMHR_PARTY_COUNT_PC34];
    int leaderHandActionIconBlitCount[DM1_V1_DMHR_PARTY_COUNT_PC34];
    int f0296Invocation[DM1_V1_DMHR_TRACE_COUNT_PC34];
    int f0292Invocation[DM1_V1_DMHR_TRACE_COUNT_PC34];
    Dm1V1ChampionPanelDeadMemberHandRefreshChampionPc34
        champions[DM1_V1_DMHR_PARTY_COUNT_PC34];
} Dm1V1ChampionPanelDeadMemberHandRefreshStatePc34;

typedef struct {
    int accepted;
    int sourceAnchorsPresent;
    int deadMemberRecognized;
    int liveMembersUnchanged;
    int deadMemberBlackFillOnly;
    int deadMemberActionIconNeverBlits;
    int deadMemberZoneFilledAtCorrectIndex;
    int liveMemberActionIconBlits;
    int liveMemberIconChangeDetected;
    int f0296WalksEverySlotBox;
    int f0295SensesAllChampions;
    int f0386DispatchesForEachActionHand;
    int f0292DeadStatusBoxBranchReachesF0386;
    int mouseScreenUpdateBracketedPerF0296;
    int noF0296DispatchForAliveF0295False;
    int rejectsPartyWithoutDeadMember;
    int rejectsF0296WalkWithoutAliveMembers;
    int rejectsPartySizeZero;
    int rejectsNegativeLeaderIndex;
    int trace[DM1_V1_DMHR_TRACE_COUNT_PC34];
    int partyChampionCount;
    int deadMemberIndex;
    int aliveMembers;
    int f0296InvocationCount;
    int f0292InvocationCount;
    int f0295HasIconChangedCount;
    int f0295SameIconCount;
    int f0038DrawIconInSlotBoxCount;
    int f0386DrawActionIconCount;
    int f0077MouseEnableCount;
    int f0078MouseDisableCount;
    int deadMemberBlackFillCount;
    int liveMemberActionIconBlitCount;
    int deadMemberActionIconBlitCount;
    Dm1V1ChampionPanelDeadMemberHandRefreshPathPc34 path;
    uint32_t hash;
} Dm1V1ChampionPanelDeadMemberHandRefreshResultPc34;

void dm1_v1_champion_panel_dead_member_hand_refresh_init_pc34(
    Dm1V1ChampionPanelDeadMemberHandRefreshStatePc34 *state);

int dm1_v1_champion_panel_dead_member_hand_refresh_run_pc34(
    Dm1V1ChampionPanelDeadMemberHandRefreshStatePc34 *state,
    Dm1V1ChampionPanelDeadMemberHandRefreshResultPc34 *result);

const Dm1V1ChampionPanelDeadMemberHandRefreshEvidencePc34 *
dm1_v1_champion_panel_dead_member_hand_refresh_evidence_pc34(void);

const char *
dm1_v1_champion_panel_dead_member_hand_refresh_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CHAMPION_PANEL_DEAD_MEMBER_HAND_REFRESH_PC34_COMPAT_H */

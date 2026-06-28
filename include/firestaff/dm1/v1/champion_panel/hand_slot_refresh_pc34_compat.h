#ifndef FIRESTAFF_DM1_V1_CHAMPION_PANEL_HAND_SLOT_REFRESH_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_PANEL_HAND_SLOT_REFRESH_PC34_COMPAT_H

/*
 * DM1 V1 champion-panel hand-slot refresh walk-order gate.
 *
 * Source-locked contract-only gate that pins the CHAMDRAW.C F0296
 * walk-order + leader-hand icon refresh precedence + candidate
 * early-return + inventory-champion skip contract for the action-hand
 * slot refresh on a fully-alive 4-champion party. The lane name is
 * `champion_panel_hand_slot_refresh_gate`.
 *
 * ReDMCSB anchors (PC 3.4 EN, ReDMCSB WIP 20210206):
 * - CHAMDRAW.C F0296_CHAMPION_DrawChangedObjectIcons:1184-1262 walks
 *   slotbox indices 0..(G0305_ui_PartyChampionCount << 1)-1 with
 *   L0885_i_ChampionIndex = slotBoxIndex >> 1 and
 *   M070_HAND_SLOT_INDEX(slotBoxIndex) == C01_SLOT_ACTION_HAND.
 *   The action-hand slotbox indices are exactly the odd indices
 *   1, 3, 5, 7 (one per champion in champion-index order).
 * - CHAMDRAW.C F0296:1208-1210 has the candidate-champion ordinal
 *   early-return: if G0299_ui_CandidateChampionOrdinal != 0 and
 *   L0883_ui_InventoryChampionOrdinal == 0 then F0296 returns
 *   immediately without walking any slotbox.
 * - CHAMDRAW.C F0296:1217-1219 has the inventory-champion ordinal
 *   skip: if L0883_ui_InventoryChampionOrdinal ==
 *   M000_INDEX_TO_ORDINAL(L0885_i_ChampionIndex) the slotbox walk
 *   `continue`s to the next slotbox without invoking F0295 or F0386.
 * - CHAMDRAW.C F0296:1212-1216 leader-hand icon refresh precedes
 *   the slotbox walk: when G4055_s_LeaderHandObject.IconIndex is in
 *   a mutable range and differs from F0033_OBJECT_GetIconIndex
 *   (G4055_s_LeaderHandObject.Thing), F0296 calls F0077 +
 *   F0036_OBJECT_ExtractIconFromBitmap +
 *   F0068_MOUSE_SetPointerToObject +
 *   F0034_OBJECT_DrawLeaderHandObjectName in that order.
 * - CHAMDRAW.C F0296:1212 G0420_B_MousePointerHiddenToDrawChanged-
 *   ObjectIconOnScreen is reset to C0_FALSE at the entry of F0296.
 * - CHAMDRAW.C F0296:1248-1251 the F0078_MOUSE_DisableScreenUpdate
 *   is called exactly once at the end of F0296 when
 *   G0420_B_MousePointerHiddenToDrawChangedObjectIconOnScreen is
 *   still C1_TRUE (i.e. some F0295/F0036 path raised it during this
 *   walk).
 * - DEFS.H:781 C01_SLOT_ACTION_HAND, 1878 M070_HAND_SLOT_INDEX,
 *   1950 C195_ICON_POTION_EMPTY_FLASK, 1952
 *   C201_ICON_ACTION_ICON_EMPTY_HAND, 5694 G0299_ui_CandidateChampion-
 *   Ordinal, 5320 G0423_i_InventoryChampionOrdinal, 5322
 *   G0305_ui_PartyChampionCount pin the constants.
 *
 * Non-overlap marker: this gate covers the F0296 walk-order + leader-
 * hand icon refresh precedence + candidate early-return +
 * inventory-champion skip contract on a fully-alive 4-champion party.
 * It is disjoint from
 *   - champion_panel_dead_member_hand_refresh (F0296/F0295/F0386 walk
 *     with a dead member present, F0292 dead-status-box branch),
 *   - champion_panel_hand_slot_priority (CHAMPION.C F0302 input
 *     dispatch, not the F0296 redraw walk),
 *   - champion_panel_portrait_box_redraw_states (F0291/F0292/F0296
 *     event matrix for the portrait-box branch + status-box
 *     cascade, not the F0296 hand-slot walk-order),
 *   - champion_panel_portrait_state_redraw (F0292 state-redraw
 *     cascade, not the F0296 hand-slot walk-order),
 *   - mirror_candidate_icon_refresh (F0296 leader-hand icon refresh
 *     interaction with the candidate ordinal, no walk-order
 *     coverage),
 *   - mirror_candidate_c040 sibling family (candidate-panel state
 *     machine, no F0296 walk-order),
 *   - champion_panel_spell_area_overlay (F0394 dead-champion reject
 *     for spell area, not the F0296 hand-slot walk-order),
 *   - champion_panel_status_hand_rotation (F0284 leader rotation,
 *     not the F0296 hand-slot walk-order),
 *   - champion_panel_second_leader_hand_slot_priority (the 2nd
 *     leader's hand-slot priority path, not the F0296 walk-order),
 *   - the F0107/F0108/chest-scroll-wheel/viewport/integrated
 *     family, and
 *   - the per-state redraw + per-action-hand slot-box dispatch
 *     family.
 *
 * Contract only: this slice models the F0296 walk-order (action-hand
 * slotbox indices 1, 3, 5, 7 in champion-index order 0, 1, 2, 3),
 * the F0296 candidate-champion ordinal early-return, the F0296
 * inventory-champion ordinal skip, the F0296 leader-hand icon
 * refresh precedence (F0077 -> F0036 -> F0068 -> F0034), the F0295
 * per-slotbox sense-and-dispatch contract on a fully-alive party,
 * and the F0077/F0078 mouse-screen-update balance. It does not call
 * real M11 graphics and does not claim real-asset bitmap parity.
 */

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_DMHSR_PARTY_COUNT_PC34 4
#define DM1_V1_DMHSR_SLOTBOX_PER_CHAMPION_PC34 2
#define DM1_V1_DMHSR_TRACE_COUNT_PC34 8
#define DM1_V1_DMHSR_THING_NONE_PC34 0xffffu
#define DM1_V1_DMHSR_C01_SLOT_ACTION_HAND_PC34 1
#define DM1_V1_DMHSR_C00_SLOT_READY_HAND_PC34 0

typedef enum {
    DM1_V1_DMHSR_PATH_INVALID_PC34 = 0,
    DM1_V1_DMHSR_PATH_FULLY_ALIVE_F0296_WALK_PC34,
    DM1_V1_DMHSR_PATH_CANDIDATE_EARLY_RETURN_PC34,
    DM1_V1_DMHSR_PATH_INVENTORY_CHAMPION_SKIP_PC34,
    DM1_V1_DMHSR_PATH_REJECTED_DEAD_MEMBER_PC34,
    DM1_V1_DMHSR_PATH_REJECTED_PARTY_INCOMPLETE_PC34
} Dm1V1ChampionPanelHandSlotRefreshPathPc34;

typedef struct {
    const char *walkF0295Anchor;
    const char *walkF0296Anchor;
    const char *walkF0033Anchor;
    const char *walkF0038Anchor;
    const char *leaderHandAnchor;
    const char *candidateOrdinalAnchor;
    const char *inventoryChampionOrdinalAnchor;
    const char *mouseBracketAnchor;
    const char *defsAnchor;
    const char *contractScope;
    const char *noRealGraphicsClaim;
    const char *nonOverlap;
} Dm1V1ChampionPanelHandSlotRefreshEvidencePc34;

typedef struct {
    int championIndex;
    int alive;
    int leader;
    uint16_t actionHandThing;
    int actionHandIconIndex;
    int slotBoxCurrentIcon;
    int iconChanged;
    int slotBoxIndex;
    int f0295Dispatched;
    int f0038DrawIconInSlotBoxCount;
    int f0386DrawActionIconCount;
    int walkOrder;
    int inventoryChampionSkipHit;
} Dm1V1ChampionPanelHandSlotRefreshChampionPc34;

typedef struct {
    int contractOnly;
    int assetFree;
    int partyChampionCount;
    int leaderIndex;
    int candidateChampionOrdinal;
    int inventoryChampionOrdinal;
    int aliveMembers;
    int inventoryChampionIndex;
    int f0296InvocationCount;
    int f0295HasIconChangedCount;
    int f0295SameIconCount;
    int f0033GetIconIndexCount;
    int f0038DrawIconInSlotBoxCount;
    int f0036ExtractIconFromBitmapCount;
    int f0068SetPointerToObjectCount;
    int f0034DrawLeaderHandObjectNameCount;
    int f0386DrawActionIconCount;
    int f0077MouseEnableCount;
    int f0078MouseDisableCount;
    int leaderHandIconChanged;
    int leaderHandMousePointerHidden;
    int leaderHandIcon;
    int leaderHandThingThing;
    int leaderHandIconRefreshCount;
    int slotBoxWalkIndex[DM1_V1_DMHSR_PARTY_COUNT_PC34];
    int slotBoxWalkChampionIndex[DM1_V1_DMHSR_PARTY_COUNT_PC34];
    int slotBoxWalkIconChanged[DM1_V1_DMHSR_PARTY_COUNT_PC34];
    int slotBoxWalkF0295Dispatched[DM1_V1_DMHSR_PARTY_COUNT_PC34];
    int slotBoxWalkF0386Dispatched[DM1_V1_DMHSR_PARTY_COUNT_PC34];
    int slotBoxWalkInventorySkip[DM1_V1_DMHSR_PARTY_COUNT_PC34];
    int f0296Trace[DM1_V1_DMHSR_TRACE_COUNT_PC34];
    Dm1V1ChampionPanelHandSlotRefreshChampionPc34
        champions[DM1_V1_DMHSR_PARTY_COUNT_PC34];
} Dm1V1ChampionPanelHandSlotRefreshStatePc34;

typedef struct {
    int accepted;
    int sourceAnchorsPresent;
    int fullyAliveRecognized;
    int walkOrderChampionIndexAscending;
    int walkOrderActionHandIndicesOdd;
    int walkOrderChampionIndexPerSlotbox;
    int leaderHandPrecedesWalk;
    int leaderHandIconRefreshOncePerF0296;
    int leaderHandF0036F0068F0034Sequence;
    int inventoryChampionSkipAppliedPerSlotbox;
    int candidateEarlyReturnBeforeWalk;
    int f0296WalksExactlyN2Slotboxes;
    int f0295SenseContractOnMutableIcon;
    int f0295NoChangeSkipsF0386;
    int f0386DispatchedForChangedActionHand;
    int mouseScreenUpdateBalancedPerF0296;
    int mouseScreenUpdateNeverRaisedWithoutChange;
    int rejectsDeadMember;
    int rejectsPartySizeZero;
    int rejectsNegativeLeaderIndex;
    int rejectsF0296WhenCandidateNoInventory;
    int trace[DM1_V1_DMHSR_TRACE_COUNT_PC34];
    int partyChampionCount;
    int leaderIndex;
    int candidateChampionOrdinal;
    int inventoryChampionOrdinal;
    int f0296InvocationCount;
    int f0295HasIconChangedCount;
    int f0295SameIconCount;
    int f0038DrawIconInSlotBoxCount;
    int f0036ExtractIconFromBitmapCount;
    int f0068SetPointerToObjectCount;
    int f0034DrawLeaderHandObjectNameCount;
    int f0386DrawActionIconCount;
    int f0077MouseEnableCount;
    int f0078MouseDisableCount;
    int leaderHandIconRefreshCount;
    int leaderHandSlotBoxesWalked;
    int slotBoxWalkF0386Dispatched;
    int slotBoxWalkInventorySkip;
    Dm1V1ChampionPanelHandSlotRefreshPathPc34 path;
    uint32_t hash;
} Dm1V1ChampionPanelHandSlotRefreshResultPc34;

void dm1_v1_champion_panel_hand_slot_refresh_init_pc34(
    Dm1V1ChampionPanelHandSlotRefreshStatePc34 *state);

int dm1_v1_champion_panel_hand_slot_refresh_run_pc34(
    Dm1V1ChampionPanelHandSlotRefreshStatePc34 *state,
    Dm1V1ChampionPanelHandSlotRefreshResultPc34 *result);

const Dm1V1ChampionPanelHandSlotRefreshEvidencePc34 *
dm1_v1_champion_panel_hand_slot_refresh_evidence_pc34(void);

const char *
dm1_v1_champion_panel_hand_slot_refresh_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CHAMPION_PANEL_HAND_SLOT_REFRESH_PC34_COMPAT_H */

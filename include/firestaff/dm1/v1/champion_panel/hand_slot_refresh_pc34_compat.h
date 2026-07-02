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
 * - CHAMDRAW.C F0296_CHAMPION_DrawChangedObjectIcons:1184-1267 walks
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
 * - CHAMDRAW.C F0296:1242-1262 inventory-owner secondary walk.
 *   When L0883_ui_InventoryChampionOrdinal is non-zero, F0296 walks
 *   the inventory champion's internal slots
 *   C00_SLOT_READY_HAND..C29_SLOT_BACKPACK_LINE1_9 (30 slots) and for
 *   each internal slot `slotIndex` maps to the inventory slotbox at
 *   `slotIndex + C08_SLOT_BOX_INVENTORY_FIRST_SLOT`; the accumulated
 *   AL0884_B_DrawViewport flag is OR-ed with the per-slot F0295
 *   return, and F0386_MENUS_DrawActionIcon is dispatched only when
 *   the slot is the C01_SLOT_ACTION_HAND slot AND F0295 reports a
 *   change. After the internal walk, when the panel is the chest
 *   panel (`G2008_i_PanelContent == M569_PANEL_CHEST` on PC 3.4 /
 *   MEDIA720), F0296 walks the 8 chest slots and dispatches F0295
 *   with slotbox index `C38_SLOT_BOX_CHEST_FIRST_SLOT + chestSlot`.
 *   At the end of the inventory-owner block, when
 *   AL0884_B_DrawViewport is C1_TRUE, M008_SET sets
 *   MASK0x4000_VIEWPORT on the inventory champion's attributes, and
 *   F0292_CHAMPION_DrawState is dispatched for the inventory champion
 *   (M001_ORDINAL_TO_INDEX of the owner ordinal).
 * - CHAMDRAW.C F0296:1264-1267 the F0078_MOUSE_DisableScreenUpdate
 *   is called exactly once at the end of F0296 when
 *   G0420_B_MousePointerHiddenToDrawChangedObjectIconOnScreen is
 *   still C1_TRUE (i.e. some F0295/F0036 path raised it during this
 *   walk).
 * - DEFS.H:780 C00_SLOT_READY_HAND, 781 C01_SLOT_ACTION_HAND, 810
 *   C30_SLOT_CHEST_1, 1874 C08_SLOT_BOX_INVENTORY_FIRST_SLOT, 1876
 *   C38_SLOT_BOX_CHEST_FIRST_SLOT, 1878 M070_HAND_SLOT_INDEX, 1950
 *   C195_ICON_POTION_EMPTY_FLASK, 1952 C201_ICON_ACTION_ICON_EMPTY_
 *   HAND, 2995/3007 M569_PANEL_CHEST, 5320 G0423_i_InventoryChampion-
 *   Ordinal, 5322 G0305_ui_PartyChampionCount, 5694 G0299_ui_Candidate-
 *   ChampionOrdinal, 5877 G0424_i_PanelContent, 5878
 *   G0425_aT_ChestSlots[8], 6317 G2008_i_PanelContent, 7208
 *   M000_INDEX_TO_ORDINAL(index) == (index + 1), 7209
 *   M001_ORDINAL_TO_INDEX(ordinal) == (ordinal - 1), 7895
 *   F0292_CHAMPION_DrawState pin the constants.
 *
 * Non-overlap marker: this gate covers the F0296 walk-order + leader-
 * hand icon refresh precedence + candidate early-return +
 * inventory-champion skip contract on a fully-alive 4-champion party,
 * and the F0296:1242-1262 inventory-owner secondary walk contract
 * (30 internal slots walked; F0386 dispatch only on
 * C01_SLOT_ACTION_HAND when changed; per-slot AL0884_B_DrawViewport
 * accumulation; chest-panel 8-slot secondary walk when
 * G2008_i_PanelContent == M569_PANEL_CHEST; MASK0x4000_VIEWPORT set +
 * F0292 dispatch when AL0884_B_DrawViewport is C1_TRUE).
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
 * the F0296:1242-1262 inventory-owner secondary walk (30 internal
 * slots + chest-panel 8 slots + AL0884_B_DrawViewport accumulation
 * + MASK0x4000_VIEWPORT set + F0292 dispatch), and the F0077/F0078
 * mouse-screen-update balance. It does not call real M11 graphics
 * and does not claim real-asset bitmap parity.
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

/*
 * F0296:1242-1262 inventory-owner secondary walk constants.
 * C00_SLOT_READY_HAND .. C29_SLOT_BACKPACK_LINE1_9 (DEFS.H:780-809)
 * maps to inventory slotbox indices C08_SLOT_BOX_INVENTORY_FIRST_SLOT
 * (DEFS.H:1874) .. C37. The chest-panel secondary walk adds
 * C38_SLOT_BOX_CHEST_FIRST_SLOT (DEFS.H:1876) + 8 slots = C38..C45.
 */
#define DM1_V1_DMHSR_INVENTORY_INTERNAL_SLOT_COUNT_PC34 30
#define DM1_V1_DMHSR_INVENTORY_FIRST_SLOTBOX_PC34 8
#define DM1_V1_DMHSR_CHEST_SLOT_COUNT_PC34 8
#define DM1_V1_DMHSR_CHEST_FIRST_SLOTBOX_PC34 38
/*
 * PC 3.4 MEDIA720 panel content. DEFS.H:3005-3009 sets M569_PANEL_CHEST=6
 * for the I34E/I34M/A36M path (the G2008_i_PanelContent variable at
 * DEFS.H:6317 carries the chest content id 6 in PC 3.4).
 */
#define DM1_V1_DMHSR_PANEL_CONTENT_CHEST_PC34 6

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
    /*
     * F0296:1242-1262 inventory-owner secondary walk contract.
     *
     * `internalSlotWalkCount` records the number of inventory champion
     * internal slots walked (0..30; 30 = C00..C29 = READY_HAND..last
     * backpack slot).
     *
     * `internalSlotIconChanged[walkStep]` records the F0295
     * has-icon-changed return for each walked internal slot, in the
     * walked order. F0296 OR-accumulated into AL0884_B_DrawViewport.
     *
     * `internalSlotF0386ActionHandDispatched` records whether F0386
     * was dispatched during the internal walk (only when the C01
     * action-hand slot reports a change).
     *
     * `internalSlotWalkF0295Dispatched` records how many internal
     * slots returned F0295 = C1_TRUE (i.e. icon changed).
     *
     * `chestPanelContentId` records the G2008_i_PanelContent (PC 3.4
     * / MEDIA720) value used by the chest-panel secondary walk; a
     * value of DM1_V1_DMHSR_PANEL_CONTENT_CHEST_PC34 (6 on PC 3.4)
     * activates the chest walk.
     *
     * `chestSlotWalkActive` records whether the chest-panel secondary
     * walk fired (1) or was skipped because the panel was not the
     * chest panel (0).
     *
     * `chestSlotWalkCount` records the number of chest slots walked
     * (0 or 8 = DM1_V1_DMHSR_CHEST_SLOT_COUNT_PC34).
     *
     * `chestSlotF0295Dispatched` records how many chest slots
     * returned F0295 = C1_TRUE.
     *
     * `drawViewportAccumulated` records the final AL0884_B_DrawViewport
     * flag after the inventory-owner secondary walk (and any
     * chest-panel walk); the F0292 dispatch + MASK0x4000_VIEWPORT set
     * are gated on this flag being C1_TRUE.
     *
     * `attributesMask0x4000ViewportSet` records whether
     * M008_SET(Attributes, MASK0x4000_VIEWPORT) was applied during
     * the inventory-owner secondary walk.
     *
     * `f0292DrawStateDispatched` records whether
     * F0292_CHAMPION_DrawState was dispatched for the inventory
     * champion during the inventory-owner secondary walk
     * (`M001_ORDINAL_TO_INDEX(inventoryChampionOrdinal)`).
     */
    int internalSlotWalkCount;
    int internalSlotF0295Dispatched;
    int internalSlotF0386ActionHandDispatched;
    int internalSlotIconChanged[DM1_V1_DMHSR_INVENTORY_INTERNAL_SLOT_COUNT_PC34];
    int chestPanelContentId;
    int chestSlotWalkActive;
    int chestSlotWalkCount;
    int chestSlotF0295Dispatched;
    int chestSlotIconChanged[DM1_V1_DMHSR_CHEST_SLOT_COUNT_PC34];
    int drawViewportAccumulated;
    int attributesMask0x4000ViewportSet;
    int f0292DrawStateDispatched;
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
    /*
     * F0296:1242-1262 inventory-owner secondary walk invariants.
     *
     * `secondaryWalkSkippedWithoutInventoryOwner` records whether the
     * secondary walk was suppressed on the path where
     * `inventoryChampionOrdinal == 0` (the non-owner path skips the
     * inventory-owner secondary block entirely).
     *
     * `secondaryWalkFiredWithInventoryOwner` records whether the
     * secondary walk fired exactly once on every path where
     * `inventoryChampionOrdinal != 0` and the path is not the
     * candidate early-return path.
     *
     * `secondaryWalkThirtySlotsWhenFired` records that exactly
     * 30 inventory internal slots were walked when the secondary
     * walk fired; never a different count.
     *
     * `secondaryWalkF0295PerSlotAlways` records that F0295 was
     * dispatched on every internal-slot iteration (not selective).
     *
     * `secondaryWalkF0386OnlyOnActionHand` records that F0386 was
     * dispatched at most once during the internal walk, and only
     * when internal slot 1 (C01 action-hand) reported a change.
     *
     * `secondaryWalkDrawViewportAccumulated` records that the final
     * AL0884_B_DrawViewport flag equals the OR of every internal
     * (and chest, when active) F0295 return.
     *
     * `secondaryWalkMaskViewportSetOnlyWhenDrawViewport` records
     * that the MASK0x4000_VIEWPORT set was applied iff
     * AL0884_B_DrawViewport was C1_TRUE after the secondary walk.
     *
     * `secondaryWalkF0292DispatchedOnlyWhenDrawViewport` records
     * that F0292_CHAMPION_DrawState was dispatched iff
     * AL0884_B_DrawViewport was C1_TRUE after the secondary walk,
     * and that its champion ordinal argument matched
     * M001_ORDINAL_TO_INDEX(inventoryChampionOrdinal).
     *
     * `chestSecondaryWalkOnlyWhenPanelChest` records that the
     * chest-panel secondary walk fired iff
     * `chestPanelContentId == DM1_V1_DMHSR_PANEL_CONTENT_CHEST_PC34`,
     * and that 8 chest slots were walked in that case.
     *
     * `chestSecondaryWalkF0295PerSlotAlways` records that F0295
     * was dispatched on every chest-slot iteration when the chest
     * secondary walk fired.
     */
    int secondaryWalkSkippedWithoutInventoryOwner;
    int secondaryWalkFiredWithInventoryOwner;
    int secondaryWalkThirtySlotsWhenFired;
    int secondaryWalkF0295PerSlotAlways;
    int secondaryWalkF0386OnlyOnActionHand;
    int secondaryWalkDrawViewportAccumulated;
    int secondaryWalkMaskViewportSetOnlyWhenDrawViewport;
    int secondaryWalkF0292DispatchedOnlyWhenDrawViewport;
    int chestSecondaryWalkOnlyWhenPanelChest;
    int chestSecondaryWalkF0295PerSlotAlways;
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
    int internalSlotWalkCount;
    int internalSlotF0295Dispatched;
    int internalSlotF0386ActionHandDispatched;
    int chestSlotWalkActive;
    int chestSlotWalkCount;
    int chestSlotF0295Dispatched;
    int drawViewportAccumulated;
    int attributesMask0x4000ViewportSet;
    int f0292DrawStateDispatched;
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

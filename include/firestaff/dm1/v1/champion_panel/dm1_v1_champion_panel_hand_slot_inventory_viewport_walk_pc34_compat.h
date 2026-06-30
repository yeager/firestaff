#ifndef FIRESTAFF_DM1_V1_CHAMPION_PANEL_HAND_SLOT_INVENTORY_VIEWPORT_WALK_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_PANEL_HAND_SLOT_INVENTORY_VIEWPORT_WALK_PC34_COMPAT_H

/*
 * DM1 V1 champion-panel hand-slot inventory-viewport walk gate.
 *
 * Source-locked contract-only gate that pins the CHAMDRAW.C F0296
 * inventory-owner viewport sub-walk + chest sub-walk + inventory-
 * champion F0292 redraw tail for the action-hand slot refresh on a
 * single inventory-owner + 4-champion party. The lane name is
 * `champion_panel_hand_slot_inventory_viewport_walk_gate`.
 *
 * This gate is a sibling of `champion_panel_hand_slot_refresh_gate`
 * (which pins F0296:1184-1262 lines 1208-1251 outside the inventory
 * sub-walk). It owns the *second* half of F0296 that the lane title
 * names: lines 1234-1257, where the inventory-owner's slots, the
 * chest slots, and the F0292_CHAMPION_DrawState dispatch are walked.
 *
 * ReDMCSB anchors (PC 3.4 EN, ReDMCSB WIP 20210206):
 * - CHAMDRAW.C F0296_CHAMPION_DrawChangedObjectIcons:1234-1242 walks
 *   the inventory champion's slot indices [C00_SLOT_READY_HAND ..
 *   C30_SLOT_CHEST_1) (30 slots) when
 *   L0883_ui_InventoryChampionOrdinal != 0. For each slot index, the
 *   slotbox index is AL0882_ui_SlotIndex + C08_SLOT_BOX_INVENTORY_FIRST_SLOT
 *   (slotbox 8..37), and F0295_CHAMPION_HasObjectIconInSlotBoxChanged
 *   is called on the running L0886_pT_Thing pointer into
 *   M516_CHAMPIONS[M001_ORDINAL_TO_INDEX(L0883_ui_InventoryChampionOrdinal)].
 *   The
 *   AL0884_B_DrawViewport bit is OR-accumulated across the walk.
 * - CHAMDRAW.C F0296:1239-1241 dispatches
 *   F0386_MENUS_DrawActionIcon(M001_ORDINAL_TO_INDEX(L0883_ui_InventoryChampionOrdinal))
 *   when the changed slot index is C01_SLOT_ACTION_HAND. This is the
 *   *inventory-owner's* action hand icon refresh, which the
 *   top-row hand-slot walk skips at F0296:1228-1229 and this gate
 *   re-fires via the inventory-viewport path. It must fire exactly
 *   once per F0296 walk when the inventory-owner's action hand icon
 *   changed, and exactly zero times when the inventory-owner ordinal
 *   is 0 or when the action-hand icon matches.
 * - CHAMDRAW.C F0296:1244 (PC34 branch) gates the chest sub-walk on
 *   G0424_i_PanelContent == M569_PANEL_CHEST. (Saturn I34E/I34M
 *   uses G2008_i_PanelContent; this gate follows the PC34 path.)
 * - CHAMDRAW.C F0296:1249-1253 walks G0425_aT_ChestSlots[0..7]
 *   with the F0295 sense-and-OR-accumulate contract, mapping each
 *   chest index to slotbox C38_SLOT_BOX_CHEST_FIRST_SLOT +
 *   AL0882_ui_SlotIndex (slotboxes 38..45). The chest sub-walk does
 *   NOT call F0386 (chest slots never dispatch an action-hand icon).
 * - CHAMDRAW.C F0296:1254-1257 dispatches the inventory-owner's
 *   F0292_CHAMPION_DrawState path when AL0884_B_DrawViewport is true
 *   (i.e. some F0295 in the inventory or chest sub-walk reported a
 *   changed icon). It first sets
 *   M008_SET(L0887_ps_Champion->Attributes, MASK0x4000_VIEWPORT)
 *   so the next F0292 redraw covers the viewport, then calls
 *   F0292_CHAMPION_DrawState(M001_ORDINAL_TO_INDEX(L0883_ui_InventoryChampionOrdinal)).
 *   When AL0884_B_DrawViewport stays 0, neither the MASK0x4000_VIEWPORT
 *   set nor the F0292 dispatch fires.
 * - DEFS.H:780 C00_SLOT_READY_HAND, 781 C01_SLOT_ACTION_HAND,
 *   810 C30_SLOT_CHEST_1, 1874 C08_SLOT_BOX_INVENTORY_FIRST_SLOT,
 *   1876 C38_SLOT_BOX_CHEST_FIRST_SLOT, 1878 M070_HAND_SLOT_INDEX,
 *   3001 M569_PANEL_CHEST (PC34 branch), 5700
 *   G0305_ui_PartyChampionCount, 5876 G0423_i_InventoryChampionOrdinal,
 *   5877 G0424_i_PanelContent, 5878 G0425_aT_ChestSlots, and
 *   731 MASK0x4000_VIEWPORT pin the constants. COMPILE.H:1039 pins
 *   M001_ORDINAL_TO_INDEX.
 *
 * Non-overlap marker: this gate covers the F0296 inventory-owner
 * viewport sub-walk (1234-1242), the inventory-owner action-hand
 * F0386 dispatch (1239-1241), the chest sub-walk (1244-1253) gated
 * on G0424 == M569_PANEL_CHEST, and the F0292_CHAMPION_DrawState
 * tail + MASK0x4000_VIEWPORT set (1254-1257). It is disjoint from
 *   - champion_panel_hand_slot_refresh (F0296:1184-1262 walk-order
 *     + leader-hand icon refresh precedence + candidate early-return
 *     + inventory-champion ordinal skip on the *top-row status* slot
 *     boxes, not the inventory-viewport sub-walk),
 *   - champion_panel_dead_member_hand_refresh (F0296/F0295/F0386 walk
 *     with a dead member present, F0292 dead-status-box branch, not
 *     the inventory-viewport sub-walk),
 *   - champion_panel_hand_slot_priority (CHAMPION.C F0302 input
 *     dispatch, not the F0296 inventory-viewport sub-walk),
 *   - champion_panel_portrait_box_redraw_states (F0291/F0292/F0296
 *     event matrix for the portrait-box branch + status-box
 *     cascade, not the F0296 inventory-viewport sub-walk),
 *   - champion_panel_portrait_state_redraw (F0292 state-redraw
 *     cascade, not the F0296 inventory-viewport sub-walk),
 *   - mirror_candidate_icon_refresh (F0296 leader-hand icon refresh
 *     interaction with the candidate ordinal, no inventory-viewport
 *     sub-walk coverage),
 *   - mirror_candidate_c040 sibling family (candidate-panel state
 *     machine, no F0296 inventory-viewport sub-walk),
 *   - champion_panel_spell_area_overlay (F0394 dead-champion reject
 *     for spell area, not the F0296 inventory-viewport sub-walk),
 *   - champion_panel_status_hand_rotation (F0284 leader rotation,
 *     not the F0296 inventory-viewport sub-walk),
 *   - champion_panel_second_leader_hand_slot_priority (the 2nd
 *     leader's hand-slot priority path, not the F0296 inventory-
 *     viewport sub-walk),
 *   - the F0107/F0108/chest-scroll-wheel family (F0333/F0334 chest
 *     close path, not the F0296 inventory-viewport sub-walk), and
 *   - the inventory_slotbox_pc34_compat static slotbox count + zone
 *     index table (PC34 layout C507..C544, not the F0296 sub-walk
 *     dispatch contract).
 *
 * Contract only: this slice models the F0296 inventory-viewport
 * sub-walk (30 inventory slots + 8 chest slots gated on
 * G0424 == M569_PANEL_CHEST), the inventory-owner F0386 action-
 * hand dispatch, and the F0292_CHAMPION_DrawState + MASK0x4000_VIEWPORT
 * tail on a single inventory-owner + 4-champion party. It does not
 * call real M11 graphics and does not claim real-asset bitmap parity.
 */

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_HSIVW_INVENTORY_SLOT_COUNT_PC34 30
#define DM1_V1_HSIVW_CHEST_SLOT_COUNT_PC34 8
#define DM1_V1_HSIVW_PARTY_COUNT_PC34 4
#define DM1_V1_HSIVW_TRACE_COUNT_PC34 8
#define DM1_V1_HSIVW_THING_NONE_PC34 0xffffu

#define DM1_V1_HSIVW_C00_SLOT_READY_HAND_PC34 0
#define DM1_V1_HSIVW_C01_SLOT_ACTION_HAND_PC34 1
#define DM1_V1_HSIVW_C08_INVENTORY_FIRST_SLOTBOX_PC34 8
#define DM1_V1_HSIVW_C30_SLOT_CHEST_1_PC34 30
#define DM1_V1_HSIVW_C38_CHEST_FIRST_SLOTBOX_PC34 38

#define DM1_V1_HSIVW_M569_PANEL_CHEST_PC34 569
#define DM1_V1_HSIVW_MASK0x4000_VIEWPORT_PC34 0x4000u

typedef enum {
    DM1_V1_HSIVW_PATH_INVALID_PC34 = 0,
    DM1_V1_HSIVW_PATH_NO_INVENTORY_OWNER_PC34,
    DM1_V1_HSIVW_PATH_INVENTORY_WALK_NO_CHEST_PC34,
    DM1_V1_HSIVW_PATH_INVENTORY_WALK_WITH_CHEST_PC34,
    DM1_V1_HSIVW_PATH_REJECTED_PARTY_SIZE_ZERO_PC34
} Dm1V1ChampionPanelHandSlotInventoryViewportWalkPathPc34;

typedef struct {
    const char *walkF0295Anchor;
    const char *walkF0296InventoryAnchor;
    const char *walkF0296ChestAnchor;
    const char *walkF0296TailAnchor;
    const char *walkF0386Anchor;
    const char *walkF0292Anchor;
    const char *inventoryChampionOrdinalAnchor;
    const char *panelContentAnchor;
    const char *chestSlotsAnchor;
    const char *dirtyBitAnchor;
    const char *defsAnchor;
    const char *contractScope;
    const char *noRealGraphicsClaim;
    const char *nonOverlap;
} Dm1V1ChampionPanelHandSlotInventoryViewportWalkEvidencePc34;

typedef struct {
    uint16_t slotThing;
    int slotIconIndex;
    int slotBoxCurrentIcon;
} Dm1V1ChampionPanelHandSlotInventoryViewportWalkSlotPc34;

typedef struct {
    int championIndex;
    int alive;
    int inventoryOwner;
    Dm1V1ChampionPanelHandSlotInventoryViewportWalkSlotPc34
        slots[DM1_V1_HSIVW_INVENTORY_SLOT_COUNT_PC34];
} Dm1V1ChampionPanelHandSlotInventoryViewportWalkChampionPc34;

typedef struct {
    uint16_t chestSlotThing;
    int chestSlotIconIndex;
    int chestSlotBoxCurrentIcon;
} Dm1V1ChampionPanelHandSlotInventoryViewportWalkChestSlotPc34;

typedef struct {
    int contractOnly;
    int assetFree;
    int partyChampionCount;
    int inventoryChampionOrdinal;
    int inventoryChampionIndex;
    int panelContent;
    int dirtyBitViewportSet;
    int f0292DrawStateDispatchCount;
    int f0295InventoryHasIconChangedCount;
    int f0295InventorySameIconCount;
    int f0295ChestHasIconChangedCount;
    int f0295ChestSameIconCount;
    int f0386InventoryOwnerDispatchCount;
    int f0296InvocationCount;
    int inventorySlotWalkIndex[DM1_V1_HSIVW_INVENTORY_SLOT_COUNT_PC34];
    int inventorySlotWalkSlotboxIndex[DM1_V1_HSIVW_INVENTORY_SLOT_COUNT_PC34];
    int inventorySlotWalkHasIconChanged
        [DM1_V1_HSIVW_INVENTORY_SLOT_COUNT_PC34];
    int inventorySlotWalkDispatchedF0386
        [DM1_V1_HSIVW_INVENTORY_SLOT_COUNT_PC34];
    int chestSlotWalkIndex[DM1_V1_HSIVW_CHEST_SLOT_COUNT_PC34];
    int chestSlotWalkSlotboxIndex[DM1_V1_HSIVW_CHEST_SLOT_COUNT_PC34];
    int chestSlotWalkHasIconChanged
        [DM1_V1_HSIVW_CHEST_SLOT_COUNT_PC34];
    int f0296Trace[DM1_V1_HSIVW_TRACE_COUNT_PC34];
    Dm1V1ChampionPanelHandSlotInventoryViewportWalkChampionPc34
        champions[DM1_V1_HSIVW_PARTY_COUNT_PC34];
    Dm1V1ChampionPanelHandSlotInventoryViewportWalkChestSlotPc34
        chestSlots[DM1_V1_HSIVW_CHEST_SLOT_COUNT_PC34];
} Dm1V1ChampionPanelHandSlotInventoryViewportWalkStatePc34;

typedef struct {
    int accepted;
    int sourceAnchorsPresent;
    int inventoryOwnerRecognized;
    int inventoryWalkIndexRangeC00ToC29;
    int inventoryWalkSlotboxIndexRangeC08ToC37;
    int inventoryWalkStrictAscending;
    int inventoryWalkF0295ContractMutableIcon;
    int inventoryWalkF0295NoChangeSkipsF0386;
    int inventoryActionHandF0386DispatchContract;
    int inventoryActionHandF0386DispatchOncePerF0296;
    int chestWalkIndexRangeZeroToSeven;
    int chestWalkSlotboxIndexRangeC38ToC45;
    int chestWalkF0295ContractMutableIcon;
    int chestWalkGatedOnPanelContent569;
    int f0292TailDispatchedWhenAnyInventoryOrChestChanged;
    int f0292TailSuppressedWhenAllInventoryAndChestUnchanged;
    int mask4000ViewportSetWhenAnyInventoryOrChestChanged;
    int rejectsPartySizeZero;
    int rejectsPanelContentNot569ForChestWalk;
    int trace[DM1_V1_HSIVW_TRACE_COUNT_PC34];
    int partyChampionCount;
    int inventoryChampionOrdinal;
    int inventoryChampionIndex;
    int panelContent;
    int f0296InvocationCount;
    int f0295InventoryHasIconChangedCount;
    int f0295InventorySameIconCount;
    int f0295ChestHasIconChangedCount;
    int f0295ChestSameIconCount;
    int f0386InventoryOwnerDispatchCount;
    int f0292DrawStateDispatchCount;
    int dirtyBitViewportSet;
    int inventoryWalkIndex[DM1_V1_HSIVW_INVENTORY_SLOT_COUNT_PC34];
    int inventoryWalkSlotboxIndex
        [DM1_V1_HSIVW_INVENTORY_SLOT_COUNT_PC34];
    int inventoryWalkHasIconChanged
        [DM1_V1_HSIVW_INVENTORY_SLOT_COUNT_PC34];
    int inventoryWalkDispatchedF0386
        [DM1_V1_HSIVW_INVENTORY_SLOT_COUNT_PC34];
    int chestWalkIndex[DM1_V1_HSIVW_CHEST_SLOT_COUNT_PC34];
    int chestWalkSlotboxIndex[DM1_V1_HSIVW_CHEST_SLOT_COUNT_PC34];
    int chestWalkHasIconChanged
        [DM1_V1_HSIVW_CHEST_SLOT_COUNT_PC34];
    Dm1V1ChampionPanelHandSlotInventoryViewportWalkPathPc34 path;
    uint32_t hash;
} Dm1V1ChampionPanelHandSlotInventoryViewportWalkResultPc34;

void dm1_v1_champion_panel_hand_slot_inventory_viewport_walk_init_pc34(
    Dm1V1ChampionPanelHandSlotInventoryViewportWalkStatePc34 *state);

int dm1_v1_champion_panel_hand_slot_inventory_viewport_walk_run_pc34(
    Dm1V1ChampionPanelHandSlotInventoryViewportWalkStatePc34 *state,
    Dm1V1ChampionPanelHandSlotInventoryViewportWalkResultPc34 *result);

const Dm1V1ChampionPanelHandSlotInventoryViewportWalkEvidencePc34 *
dm1_v1_champion_panel_hand_slot_inventory_viewport_walk_evidence_pc34(void);

const char *
dm1_v1_champion_panel_hand_slot_inventory_viewport_walk_source_evidence_pc34(
    void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CHAMPION_PANEL_HAND_SLOT_INVENTORY_VIEWPORT_WALK_PC34_COMPAT_H */

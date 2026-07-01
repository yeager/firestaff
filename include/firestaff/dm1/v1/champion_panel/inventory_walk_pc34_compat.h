#ifndef FIRESTAFF_DM1_V1_CHAMPION_PANEL_INVENTORY_WALK_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_PANEL_INVENTORY_WALK_PC34_COMPAT_H

/*
 * DM1 V1 champion-panel inventory/chest redraw walk gate.
 *
 * Source-locked contract-only gate that pins the second half of
 * CHAMDRAW.C F0296_CHAMPION_DrawChangedObjectIcons — the inventory-
 * champion inventory-slot walk + the optional chest-slot walk — for
 * a fully-alive 4-champion party when G0423_i_InventoryChampionOrdinal
 * is non-zero. The lane name is `champion_panel_inventory_walk_gate`.
 *
 * ReDMCSB anchors (PC 3.4 EN, ReDMCSB WIP 20210206):
 *
 * - CHAMDRAW.C F0296:1233-1259 inventory-owner walk. After the
 *   action-hand slotbox walk on lines 1221-1231, F0296 branches
 *   into the inventory-owner walk when
 *   G0423_i_InventoryChampionOrdinal != 0. The walk iterates
 *   AL0882_ui_SlotIndex = C00_SLOT_READY_HAND .. C30_SLOT_CHEST_1-1
 *   (slot indices 0..29 inclusive, 30 slots), runs
 *   F0295_CHAMPION_HasObjectIconInSlotBoxChanged with the
 *   C08_SLOT_BOX_INVENTORY_FIRST_SLOT offset (line 1239) on each
 *   slot, ORs the changed result into AL0884_B_DrawViewport, and
 *   dispatches F0386_MENUS_DrawActionIcon once on the inventory
 *   champion when the changed slot is C01_SLOT_ACTION_HAND (line
 *   1241).
 *
 * - CHAMDRAW.C F0296:1244-1251 chest-slot walk. When
 *   G0424_i_PanelContent == M569_PANEL_CHEST, F0296 walks the 8
 *   chest slots 0..7 with the C38_SLOT_BOX_CHEST_FIRST_SLOT offset
 *   on lines 1247-1251, ORing each F0295 result into
 *   AL0884_B_DrawViewport but never dispatching F0386 on chest
 *   slots (chest slots are not action-hand slots).
 *
 * - CHAMDRAW.C F0296:1253-1256 F0292 dispatch + MASK0x4000_VIEWPORT
 *   set. When AL0884_B_DrawViewport is C1_TRUE (i.e. some
 *   inventory or chest slot raised a change), F0296 sets
 *   MASK0x4000_VIEWPORT on the inventory champion's Attributes
 *   (M008_SET) and calls F0292_CHAMPION_DrawState on the inventory
 *   champion index exactly once.
 *
 * - DEFS.H:780 C00_SLOT_READY_HAND = 0, 781 C01_SLOT_ACTION_HAND
 *   = 1, 810 C30_SLOT_CHEST_1 = 30, 1874
 *   C08_SLOT_BOX_INVENTORY_FIRST_SLOT = 8, 1876
 *   C38_SLOT_BOX_CHEST_FIRST_SLOT = 38, 731 MASK0x4000_VIEWPORT =
 *   0x4000, 873 M516_CHAMPIONS, 2995-3011 M569_PANEL_CHEST pin the
 *   constants. M008_SET is the canonical "set bit" macro on
 *   Attributes (DEFS.H:738).
 *
 * - DEFS.H:5877 G0424_i_PanelContent, 5878 G0425_aT_ChestSlots[8]
 *   pin the panel content + chest slot array storage. M516_CHAMPIONS
 *   indexes into the per-champion Slots[] array whose first 30
 *   entries are the inventory slots in canonical DM1 slot order
 *   (READY_HAND, ACTION_HAND, HEAD, NECK_1, NECK_2, TORSO_1,
 *   TORSO_2, ARM_1, ARM_2, HAND_1, HAND_2, FOOT_1, FOOT_2, QUIVER_1
 *   .. QUIVER_3, WEAPON_1 .. WEAPON_4, SHIELD, MISC_1 .. MISC_4,
 *   followed by CHEST_1 .. CHEST_8 at indices 30..37).
 *
 * Non-overlap marker: this gate covers the F0296 inventory-owner
 * walk (CHAMDRAW.C:1233-1259) — the inventory-slot walk, the
 * chest-slot walk when G0424_i_PanelContent == M569_PANEL_CHEST,
 * the F0292_CHAMPION_DrawState dispatch on the inventory champion,
 * and the MASK0x4000_VIEWPORT attribute set when any inventory or
 * chest slot changed. It is disjoint from
 *   - champion_panel_hand_slot_refresh (the F0296:1184-1231
 *     leader-hand + action-hand slotbox walk-order + candidate
 *     early-return + inventory-champion skip on the matched
 *     action-hand slotbox; this new sibling starts where that
 *     slice ends and owns the F0296:1233-1259 inventory-owner
 *     walk),
 *   - champion_panel_dead_member_hand_refresh (F0296 + F0292 with a
 *     dead member present + F0292 dead-status-box branch; this new
 *     slice models a fully-alive party),
 *   - champion_panel_hand_slot_priority (CHAMPION.C F0302 input
 *     dispatch, not the F0296 redraw walk),
 *   - champion_panel_portrait_box_redraw_states (F0291/F0292/F0296
 *     event matrix for the portrait-box branch + status-box
 *     cascade, not the F0296 inventory-owner walk),
 *   - champion_panel_portrait_state_redraw (F0292 state-redraw
 *     cascade on the portrait-box, not the F0296 inventory-owner
 *     walk),
 *   - mirror_candidate_icon_refresh (F0296 leader-hand icon
 *     refresh interaction with the candidate ordinal, no
 *     inventory-owner walk coverage),
 *   - mirror_candidate_c040 sibling family (candidate-panel state
 *     machine, no F0296 inventory-owner walk),
 *   - champion_panel_spell_area_overlay (F0394 dead-champion
 *     reject for spell area, not the F0296 inventory-owner walk),
 *   - champion_panel_status_hand_rotation (F0284 leader rotation,
 *     not the F0296 inventory-owner walk),
 *   - champion_panel_second_leader_hand_slot_priority (the 2nd
 *     leader's hand-slot priority path, not the F0296 walk),
 *   - the F0107/F0108/chest-scroll-wheel/viewport/integrated
 *     family, and
 *   - the per-state redraw + per-action-hand slot-box dispatch
 *     family (which targets the slotbox index, not the inventory-
 *     slot walk).
 *
 * Contract only: this slice models the F0296 inventory-owner walk
 * (30 inventory slots 0..29 with the C08 offset), the F0296
 * chest-slot walk (8 chest slots 0..7 with the C38 offset) gated
 * on G0424_i_PanelContent == M569_PANEL_CHEST, the F0292
 * dispatch + MASK0x4000_VIEWPORT set on the inventory champion
 * when AL0884_B_DrawViewport is raised, and the F0295 per-slot
 * sense contract on a fully-alive party. It does not call real
 * M11 graphics and does not claim real-asset bitmap parity.
 */

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_DMIW_PARTY_COUNT_PC34 4
#define DM1_V1_DMIW_INVENTORY_SLOT_COUNT_PC34 30
#define DM1_V1_DMIW_CHEST_SLOT_COUNT_PC34 8
#define DM1_V1_DMIW_TRACE_COUNT_PC34 8
#define DM1_V1_DMIW_THING_NONE_PC34 0xffffu
#define DM1_V1_DMIW_C00_SLOT_READY_HAND_PC34 0
#define DM1_V1_DMIW_C01_SLOT_ACTION_HAND_PC34 1
#define DM1_V1_DMIW_C30_SLOT_CHEST_1_PC34 30
#define DM1_V1_DMIW_C08_SLOT_BOX_INVENTORY_FIRST_SLOT_PC34 8
#define DM1_V1_DMIW_C38_SLOT_BOX_CHEST_FIRST_SLOT_PC34 38
#define DM1_V1_DMIW_MASK0x4000_VIEWPORT_PC34 0x4000u
#define DM1_V1_DMIW_M569_PANEL_CHEST_PC34 4
#define DM1_V1_DMIW_M569_PANEL_CHEST_V2X_PC34 6

typedef enum {
    DM1_V1_DMIW_PATH_INVALID_PC34 = 0,
    DM1_V1_DMIW_PATH_INVENTORY_WALK_NO_CHEST_PC34,
    DM1_V1_DMIW_PATH_INVENTORY_WALK_WITH_CHEST_PC34,
    DM1_V1_DMIW_PATH_INVENTORY_WALK_NO_DRAW_VIEWPORT_PC34,
    DM1_V1_DMIW_PATH_INVENTORY_WALK_DRAW_VIEWPORT_PC34,
    DM1_V1_DMIW_PATH_REJECTED_DEAD_MEMBER_PC34,
    DM1_V1_DMIW_PATH_REJECTED_PARTY_INCOMPLETE_PC34,
    DM1_V1_DMIW_PATH_REJECTED_NO_INVENTORY_OWNER_PC34,
    DM1_V1_DMIW_PATH_REJECTED_INVALID_PANEL_CONTENT_PC34
} Dm1V1ChampionPanelInventoryWalkPathPc34;

typedef struct {
    const char *walkF0295Anchor;
    const char *walkF0296Anchor;
    const char *walkF0292Anchor;
    const char *walkF0386Anchor;
    const char *inventoryChampionOrdinalAnchor;
    const char *panelContentAnchor;
    const char *chestSlotsAnchor;
    const char *maskViewportAnchor;
    const char *defsAnchor;
    const char *contractScope;
    const char *noRealGraphicsClaim;
    const char *nonOverlap;
} Dm1V1ChampionPanelInventoryWalkEvidencePc34;

typedef struct {
    int championIndex;
    int alive;
    int leader;
    int inventorySlots[DM1_V1_DMIW_INVENTORY_SLOT_COUNT_PC34];
    int inventoryCurrentIcon[DM1_V1_DMIW_INVENTORY_SLOT_COUNT_PC34];
    int inventoryObjectIcon[DM1_V1_DMIW_INVENTORY_SLOT_COUNT_PC34];
    int inventoryIconChanged[DM1_V1_DMIW_INVENTORY_SLOT_COUNT_PC34];
    int inventoryF0295Dispatched[DM1_V1_DMIW_INVENTORY_SLOT_COUNT_PC34];
    int attributesMask;
} Dm1V1ChampionPanelInventoryWalkChampionPc34;

typedef struct {
    int contractOnly;
    int assetFree;
    int partyChampionCount;
    int leaderIndex;
    int candidateChampionOrdinal;
    int inventoryChampionOrdinal;
    int inventoryChampionIndex;
    int panelContent;
    int chestSlots[DM1_V1_DMIW_CHEST_SLOT_COUNT_PC34];
    int chestCurrentIcon[DM1_V1_DMIW_CHEST_SLOT_COUNT_PC34];
    int chestObjectIcon[DM1_V1_DMIW_CHEST_SLOT_COUNT_PC34];
    int chestIconChanged[DM1_V1_DMIW_CHEST_SLOT_COUNT_PC34];
    int chestF0295Dispatched[DM1_V1_DMIW_CHEST_SLOT_COUNT_PC34];
    int f0296InvocationCount;
    int f0292InvocationCount;
    int f0295HasIconChangedCount;
    int f0295SameIconCount;
    int f0386DrawActionIconCount;
    int inventorySlotF0295Dispatched[DM1_V1_DMIW_INVENTORY_SLOT_COUNT_PC34];
    int inventorySlotF0386Dispatched[DM1_V1_DMIW_INVENTORY_SLOT_COUNT_PC34];
    int inventorySlotIconChanged[DM1_V1_DMIW_INVENTORY_SLOT_COUNT_PC34];
    int chestSlotF0295Dispatched[DM1_V1_DMIW_CHEST_SLOT_COUNT_PC34];
    int chestSlotIconChanged[DM1_V1_DMIW_CHEST_SLOT_COUNT_PC34];
    int drawViewportLatched;
    int mask0x4000ViewportSetCount;
    int f0296Trace[DM1_V1_DMIW_TRACE_COUNT_PC34];
    Dm1V1ChampionPanelInventoryWalkChampionPc34
        champions[DM1_V1_DMIW_PARTY_COUNT_PC34];
} Dm1V1ChampionPanelInventoryWalkStatePc34;

typedef struct {
    int accepted;
    int sourceAnchorsPresent;
    int fullyAliveRecognized;
    int inventoryWalkCoversThirtySlots;
    int inventoryWalkSlotboxOffsetApplied;
    int inventoryWalkActionHandDispatchesF0386;
    int inventoryWalkNonActionHandSkipsF0386;
    int inventoryWalkF0295DispatchedPerSlot;
    int chestWalkGatedOnPanelContentChest;
    int chestWalkGatedOffWhenPanelContentNotChest;
    int chestWalkCoversEightSlots;
    int chestWalkSlotboxOffsetApplied;
    int chestWalkF0295DispatchedPerSlot;
    int chestWalkNeverDispatchesF0386;
    int f0292DispatchedOnlyWhenDrawViewportLatched;
    int f0292DispatchedExactlyOncePerF0296;
    int f0292TargetsInventoryChampionIndex;
    int mask0x4000ViewportSetOnlyWhenDrawViewportLatched;
    int mask0x4000ViewportSetOnInventoryChampionAttributes;
    int rejectsDeadMember;
    int rejectsPartySizeZero;
    int rejectsNoInventoryOwner;
    int rejectsInvalidPanelContent;
    int trace[DM1_V1_DMIW_TRACE_COUNT_PC34];
    int partyChampionCount;
    int leaderIndex;
    int inventoryChampionOrdinal;
    int inventoryChampionIndex;
    int panelContent;
    int f0296InvocationCount;
    int f0292InvocationCount;
    int f0295HasIconChangedCount;
    int f0295SameIconCount;
    int f0386DrawActionIconCount;
    int inventorySlotF0295DispatchedTotal;
    int inventorySlotF0386DispatchedTotal;
    int inventorySlotIconChangedTotal;
    int chestSlotF0295DispatchedTotal;
    int chestSlotIconChangedTotal;
    int inventoryWalkSlotCount;
    int chestWalkSlotCount;
    int drawViewportLatched;
    int mask0x4000ViewportSetCount;
    Dm1V1ChampionPanelInventoryWalkPathPc34 path;
    uint32_t hash;
} Dm1V1ChampionPanelInventoryWalkResultPc34;

void dm1_v1_champion_panel_inventory_walk_init_pc34(
    Dm1V1ChampionPanelInventoryWalkStatePc34 *state);

int dm1_v1_champion_panel_inventory_walk_run_pc34(
    Dm1V1ChampionPanelInventoryWalkStatePc34 *state,
    Dm1V1ChampionPanelInventoryWalkResultPc34 *result);

const Dm1V1ChampionPanelInventoryWalkEvidencePc34 *
dm1_v1_champion_panel_inventory_walk_evidence_pc34(void);

const char *
dm1_v1_champion_panel_inventory_walk_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CHAMPION_PANEL_INVENTORY_WALK_PC34_COMPAT_H */

#ifndef FIRESTAFF_DM1_V1_CHAMPION_PANEL_INVENTORY_TAIL_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_PANEL_INVENTORY_TAIL_PC34_COMPAT_H

/*
 * DM1 V1 champion-panel F0296 inventory-owner tail gate.
 *
 * Source-locked contract-only companion to the hand-slot refresh gate.
 * This slice covers only the CHAMDRAW.C F0296 branch that runs after the
 * top-row status-hand walk when G0423_i_InventoryChampionOrdinal is non-zero:
 * inventory slotboxes C08..C37 are scanned, the inventory action hand may
 * redraw its action icon, optional open-chest slotboxes C38..C45 are scanned,
 * and any changed inventory/chest icon sets MASK0x4000_VIEWPORT before F0292
 * redraws the inventory owner.
 *
 * ReDMCSB anchors (WIP 20210206 PC 3.4 EN):
 * - CHAMDRAW.C F0296_CHAMPION_DrawChangedObjectIcons:1233-1247 scans
 *   inventory and optional chest slotboxes after the status-hand walk.
 * - CHAMDRAW.C F0295_CHAMPION_HasObjectIconInSlotBoxChanged:1153-1182
 *   owns mutable-icon comparison and F0038 redraw.
 * - CHAMDRAW.C F0296:1238-1240 redraws F0386 only for the inventory
 *   owner's inventory action-hand slot (C01).
 * - CHAMDRAW.C F0296:1242-1244 scans chest slotboxes only when panel
 *   content is M569_PANEL_CHEST; chest changes never call F0386 here.
 * - CHAMDRAW.C F0296:1245-1247 sets MASK0x4000_VIEWPORT and calls F0292
 *   once when any inventory/chest slot changed.
 * - DEFS.H:731 MASK0x4000_VIEWPORT, 781 C01_SLOT_ACTION_HAND,
 *   810 C30_SLOT_CHEST_1, 1874 C08_SLOT_BOX_INVENTORY_FIRST_SLOT,
 *   1876 C38_SLOT_BOX_CHEST_FIRST_SLOT, 3001/3007 M569_PANEL_CHEST.
 *
 * Contract only: no M11 renderer, no GRAPHICS.DAT/DUNGEON.DAT load, no
 * original-vs-Firestaff pixel parity claim.
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_CPIT_INVENTORY_SLOT_COUNT_PC34 30
#define DM1_V1_CPIT_CHEST_SLOT_COUNT_PC34 8
#define DM1_V1_CPIT_C08_SLOT_BOX_INVENTORY_FIRST_SLOT_PC34 8
#define DM1_V1_CPIT_C38_SLOT_BOX_CHEST_FIRST_SLOT_PC34 38
#define DM1_V1_CPIT_C30_SLOT_CHEST_1_PC34 30
#define DM1_V1_CPIT_C01_SLOT_ACTION_HAND_PC34 1
#define DM1_V1_CPIT_MASK0X4000_VIEWPORT_PC34 0x4000

typedef struct {
    const char *f0296InventoryTailAnchor;
    const char *f0295Anchor;
    const char *defsAnchor;
    const char *scope;
    const char *nonOverlap;
} Dm1V1ChampionPanelInventoryTailEvidencePc34;

typedef struct {
    int contractOnly;
    int assetFree;
    int inventoryChampionOrdinal;
    int inventoryChampionIndex;
    int panelContentIsChest;
    int inventoryCurrentIcon[DM1_V1_CPIT_INVENTORY_SLOT_COUNT_PC34];
    int inventoryObjectIcon[DM1_V1_CPIT_INVENTORY_SLOT_COUNT_PC34];
    int chestCurrentIcon[DM1_V1_CPIT_CHEST_SLOT_COUNT_PC34];
    int chestObjectIcon[DM1_V1_CPIT_CHEST_SLOT_COUNT_PC34];
} Dm1V1ChampionPanelInventoryTailStatePc34;

typedef struct {
    int accepted;
    int sourceAnchorsPresent;
    int inventoryOwnerRequired;
    int inventoryOwnerIndexMatchesOrdinal;
    int inventorySlotScanCount;
    int chestSlotScanCount;
    int inventoryFirstSlotBox;
    int inventoryLastSlotBox;
    int chestFirstSlotBox;
    int chestLastSlotBox;
    int inventoryChangedCount;
    int chestChangedCount;
    int f0038DrawIconInSlotBoxCount;
    int f0386DrawActionIconCount;
    int actionHandChangeDispatchesF0386;
    int nonActionInventoryChangeSkipsF0386;
    int chestChangeSkipsF0386;
    int viewportMaskSet;
    int f0292DrawStateCount;
    int f0292CalledOnceForAnyTailChange;
    int noChangeSkipsViewportCascade;
    uint32_t hash;
} Dm1V1ChampionPanelInventoryTailResultPc34;

void dm1_v1_champion_panel_inventory_tail_init_pc34(
    Dm1V1ChampionPanelInventoryTailStatePc34 *state);

int dm1_v1_champion_panel_inventory_tail_run_pc34(
    const Dm1V1ChampionPanelInventoryTailStatePc34 *state,
    Dm1V1ChampionPanelInventoryTailResultPc34 *result);

const Dm1V1ChampionPanelInventoryTailEvidencePc34 *
dm1_v1_champion_panel_inventory_tail_evidence_pc34(void);

const char *
dm1_v1_champion_panel_inventory_tail_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CHAMPION_PANEL_INVENTORY_TAIL_PC34_COMPAT_H */

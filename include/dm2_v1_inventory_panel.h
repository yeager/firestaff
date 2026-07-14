#ifndef FIRESTAFF_DM2_V1_INVENTORY_PANEL_H
#define FIRESTAFF_DM2_V1_INVENTORY_PANEL_H

/*
 * DM2 V1 inventory/item-panel view helper.
 *
 * This is a data-free bridge around the existing DM2 champion inventory,
 * leader-hand possession, and DB handle helpers. It does not dispatch input
 * or render pixels.
 *
 * Source anchors:
 *   ReDMCSB DEFS.H:779-810 slot indices, CM1 leader hand sentinel.
 *   ReDMCSB PANEL.C:1127-1200 object-description panel text/icon route.
 *   ReDMCSB PANEL.C:1658-1692 action-hand item panel fallback route.
 *   ReDMCSB CHAMPION.C:250-268/270-282 leader-hand put/remove state.
 *   ReDMCSB LOADSAVE.C:1535-1537/2744 leader-hand object persistence.
 */

#include "dm2_v1_save_load.h"
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM2_V1_INV_SLOT_LEADER_HAND      (-1)
#define DM2_V1_INV_SLOT_READY_HAND       0
#define DM2_V1_INV_SLOT_ACTION_HAND      1
#define DM2_V1_INV_SLOT_HEAD             2
#define DM2_V1_INV_SLOT_TORSO            3
#define DM2_V1_INV_SLOT_LEGS             4
#define DM2_V1_INV_SLOT_FEET             5
#define DM2_V1_INV_SLOT_NECK             10
#define DM2_V1_INV_SLOT_BACKPACK_FIRST   13
#define DM2_V1_INV_SLOT_COUNT            30

typedef struct {
    uint32_t object_id;
    const char *description;
} DM2_V1_InventoryPanelDescription;

typedef struct {
    int selected_slot;
    uint32_t object_id;
    int has_object;
    int db_resolved;
    uint8_t db_pool;
    uint32_t db_index;
    char description[64];
} DM2_V1_InventoryPanelItemView;

const char *dm2_v1_inventory_slot_label(int slot);
int dm2_v1_inventory_slot_is_equipment(int slot);

int dm2_v1_inventory_panel_select_item(
    const DM2_ChampionRecord *champion,
    const DM2_LeaderPossession *leader_hand,
    int selected_slot,
    const DM2_DB_State *db,
    const DM2_V1_InventoryPanelDescription *descriptions,
    size_t description_count,
    DM2_V1_InventoryPanelItemView *out);

const char *dm2_v1_inventory_panel_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_INVENTORY_PANEL_H */

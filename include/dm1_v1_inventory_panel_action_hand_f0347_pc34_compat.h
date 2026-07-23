#ifndef FIRESTAFF_DM1_V1_INVENTORY_PANEL_ACTION_HAND_F0347_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_INVENTORY_PANEL_ACTION_HAND_F0347_PC34_COMPAT_H

#include <stdint.h>

#include "memory_dungeon_dat_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* PANEL.C F0347 selects F0342 for a non-scroll/non-container action hand.
 * This narrow route admits only C05; C07/C09 and drawing stay separately owned. */
typedef struct DM1_V1_InventoryPanelActionHandReceiptF0347Pc34 {
    int valid;
    unsigned short actionHandThing;
    unsigned int weaponType;
    uint32_t rawFingerprint;
    const char *sourceAnchor;
} DM1_V1_InventoryPanelActionHandReceiptF0347Pc34;

int dm1_v1_inventory_panel_action_hand_admit_f0347_pc34(
    const struct DungeonThings_Compat *things,
    unsigned short actionHandThing,
    DM1_V1_InventoryPanelActionHandReceiptF0347Pc34 *outReceipt);

#ifdef __cplusplus
}
#endif

#endif

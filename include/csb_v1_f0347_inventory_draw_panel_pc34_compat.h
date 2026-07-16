#ifndef CSB_V1_F0347_INVENTORY_DRAW_PANEL_PC34_COMPAT_H
#define CSB_V1_F0347_INVENTORY_DRAW_PANEL_PC34_COMPAT_H

#include "csb_v1_boot.h"

/* CSB-owned ReDMCSB PANEL.C F0347 boundary.
 *
 * The startup/runtime session has already produced the concrete C017/C040
 * HUD-panel blit receipt. This adapter names the F0347 source step and
 * accepts only that source-owned receipt; it does not create pixels, replay
 * startup, or infer panel state from host geometry.
 */

typedef struct CSB_V1_F0347_InventoryDrawPanelReceipt_PC34 {
    int valid;
    int source_panel_receipt_valid;
    int c017_inventory_drawn;
    int c040_resurrect_drawn;
    int c040_optional_route;
    int no_legacy_wrappers;
    int no_synthetic_surface;
    unsigned int source_tick;
    unsigned int session_generation;
    unsigned int c017_pixel_hash;
    unsigned int c040_pixel_hash;
    unsigned int panel_hash;
    const char *source_evidence;
} CSB_V1_F0347_InventoryDrawPanelReceipt_PC34;

int csb_v1_f0347_inventory_draw_panel_pc34(
    const CSB_V1_StartupRuntimeHudPanelReceipt_PC34 *panel_receipt,
    int require_resurrect_panel,
    CSB_V1_F0347_InventoryDrawPanelReceipt_PC34 *out_receipt);

const char *csb_v1_f0347_inventory_draw_panel_source_evidence_pc34(void);

#endif

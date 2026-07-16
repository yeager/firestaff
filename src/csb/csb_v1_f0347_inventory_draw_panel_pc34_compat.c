#include "csb_v1_f0347_inventory_draw_panel_pc34_compat.h"

#include <string.h>

int csb_v1_f0347_inventory_draw_panel_pc34(
    const CSB_V1_StartupRuntimeHudPanelReceipt_PC34 *panel_receipt,
    int require_resurrect_panel,
    CSB_V1_F0347_InventoryDrawPanelReceipt_PC34 *out_receipt)
{
    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!panel_receipt || !out_receipt || !panel_receipt->valid ||
        !panel_receipt->real_asset_matched ||
        !panel_receipt->c017_presented ||
        !panel_receipt->no_legacy_wrappers ||
        !panel_receipt->no_synthetic_surface ||
        panel_receipt->source_tick == 0u ||
        panel_receipt->session_generation == 0u ||
        panel_receipt->c017_pixel_hash == 0u ||
        panel_receipt->panel_hash == 0u ||
        (require_resurrect_panel && !panel_receipt->c040_presented) ||
        (panel_receipt->c040_presented &&
         panel_receipt->c040_pixel_hash == 0u) ||
        (!panel_receipt->c040_presented &&
         panel_receipt->c040_pixel_hash != 0u)) {
        return 0;
    }

    out_receipt->valid = 1;
    out_receipt->source_panel_receipt_valid = 1;
    out_receipt->c017_inventory_drawn = 1;
    out_receipt->c040_resurrect_drawn =
        panel_receipt->c040_presented ? 1 : 0;
    out_receipt->c040_optional_route = require_resurrect_panel ? 0 : 1;
    out_receipt->no_legacy_wrappers = 1;
    out_receipt->no_synthetic_surface = 1;
    out_receipt->source_tick = panel_receipt->source_tick;
    out_receipt->session_generation = panel_receipt->session_generation;
    out_receipt->c017_pixel_hash = panel_receipt->c017_pixel_hash;
    out_receipt->c040_pixel_hash = panel_receipt->c040_pixel_hash;
    out_receipt->panel_hash = panel_receipt->panel_hash;
    out_receipt->source_evidence =
        csb_v1_f0347_inventory_draw_panel_source_evidence_pc34();
    return 1;
}

const char *csb_v1_f0347_inventory_draw_panel_source_evidence_pc34(void)
{
    return "ReDMCSB PANEL.C F0347_INVENTORY_DrawPanel lines 2376-2448; "
           "PANEL.C F0346_INVENTORY_DrawPanel_ResurrectReincarnate; "
           "CSB startup runtime C017/C040 HUD-panel receipt";
}

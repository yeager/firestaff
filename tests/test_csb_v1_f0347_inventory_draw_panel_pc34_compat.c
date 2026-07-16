#include "csb_v1_f0347_inventory_draw_panel_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_failures = 0;

static void check(int condition, const char *message)
{
    if (!condition) {
        ++g_failures;
        fprintf(stderr, "FAIL: %s\n", message);
    }
}

static CSB_V1_StartupRuntimeHudPanelReceipt_PC34 source_panel(int with_c040)
{
    CSB_V1_StartupRuntimeHudPanelReceipt_PC34 receipt;

    memset(&receipt, 0, sizeof(receipt));
    receipt.valid = 1;
    receipt.real_asset_matched = 1;
    receipt.c017_presented = 1;
    receipt.c040_presented = with_c040 ? 1 : 0;
    receipt.no_legacy_wrappers = 1;
    receipt.no_synthetic_surface = 1;
    receipt.source_tick = 7u;
    receipt.session_generation = 3u;
    receipt.c017_pixel_hash = 0x170017u;
    receipt.c040_pixel_hash = with_c040 ? 0x400040u : 0u;
    receipt.panel_hash = with_c040 ? 0x347040u : 0x347017u;
    receipt.source_evidence =
        "ReDMCSB PANEL.C F0347/F0346 lines 2376-2448";
    return receipt;
}

int main(void)
{
    CSB_V1_StartupRuntimeHudPanelReceipt_PC34 panel;
    CSB_V1_F0347_InventoryDrawPanelReceipt_PC34 receipt;

    panel = source_panel(1);
    check(csb_v1_f0347_inventory_draw_panel_pc34(&panel, 1, &receipt) == 1,
          "accepts source-owned C017 plus C040 panel receipt");
    check(receipt.valid && receipt.source_panel_receipt_valid,
          "publishes valid F0347 receipt");
    check(receipt.c017_inventory_drawn && receipt.c040_resurrect_drawn,
          "records both C017 and C040 source surfaces");
    check(receipt.no_legacy_wrappers && receipt.no_synthetic_surface,
          "keeps wrapper and synthetic routes closed");
    check(receipt.c017_pixel_hash == panel.c017_pixel_hash &&
              receipt.c040_pixel_hash == panel.c040_pixel_hash &&
              receipt.panel_hash == panel.panel_hash,
          "carries the source hashes unchanged");

    panel = source_panel(0);
    check(csb_v1_f0347_inventory_draw_panel_pc34(&panel, 0, &receipt) == 1,
          "accepts C017-only normal panel route");
    check(receipt.c017_inventory_drawn && !receipt.c040_resurrect_drawn &&
              receipt.c040_optional_route,
          "records C040 as optional when not required");
    check(receipt.c040_pixel_hash == 0u,
          "does not invent a C040 hash for the normal panel");

    panel = source_panel(0);
    check(csb_v1_f0347_inventory_draw_panel_pc34(&panel, 1, &receipt) == 0,
          "rejects missing C040 when resurrect panel is required");

    panel = source_panel(1);
    panel.no_synthetic_surface = 0;
    check(csb_v1_f0347_inventory_draw_panel_pc34(&panel, 1, &receipt) == 0,
          "rejects synthetic panel receipts");

    panel = source_panel(1);
    panel.c040_pixel_hash = 0u;
    check(csb_v1_f0347_inventory_draw_panel_pc34(&panel, 1, &receipt) == 0,
          "rejects C040 presentation without source pixels");

    panel = source_panel(0);
    panel.c040_pixel_hash = 0x400040u;
    check(csb_v1_f0347_inventory_draw_panel_pc34(&panel, 0, &receipt) == 0,
          "rejects stray C040 hash on C017-only panel");

    check(csb_v1_f0347_inventory_draw_panel_source_evidence_pc34() != 0,
          "source evidence string is available");

    if (g_failures != 0) return 1;
    puts("csb_v1_f0347_inventory_draw_panel_pc34_compat: ok");
    return 0;
}

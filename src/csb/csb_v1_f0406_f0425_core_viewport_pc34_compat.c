#include "csb_v1_f0406_f0425_core_viewport_pc34_compat.h"

#include <string.h>

int csb_v1_f0406_f0425_core_viewport_receipt_pc34(
    const CSB_V1_RuntimeProfile *profile,
    const CSB_V1_CSBGraphicsStartupPackage *package,
    CSB_V1_F0406F0425CoreViewportReceiptPc34 *out)
{
    CSB_V1_F0406F0425CoreViewportReceiptPc34 receipt;
    int square;

    if (!out) return 0;
    memset(&receipt, 0, sizeof(receipt));
    *out = receipt;

    if (!profile || !package || !profile->party_state_valid ||
        !profile->dungeon_handle || !profile->dungeon_handle->raw_data ||
        profile->dungeon_handle->square_bytes != 1 ||
        !profile->graphics_asset.path || !package->hud_ready ||
        !csb_v1_csbgraphics_startup_package_surface_draw_eligible(
            package, CSB_V1_CSBGRAPHICS_STARTUP_ASSET_HUD_INVENTORY) ||
        strcmp(profile->graphics_asset.path,
               package->palette_source.source_path) != 0) {
        return 0;
    }

    square = csb_v1_dungeon_get_raw_square(profile->dungeon_handle,
                                            profile->current_level,
                                            profile->party_x,
                                            profile->party_y);
    if (square < 0) return 0;

    receipt.valid = 1;
    receipt.map_index = profile->current_level;
    receipt.map_x = profile->party_x;
    receipt.map_y = profile->party_y;
    receipt.square_type = (square >> 5) & 7;
    receipt.source_bound_mask = CSB_V1_F0406_F0425_SOURCE_MASK;
    receipt.menu_action_owner_required = 1;
    receipt.save_owner_required = 1;
    receipt.dialog_owner_required = 1;
    receipt.source_evidence =
        "ReDMCSB MENUS.C F0406-F0412, SAVEUTIL.C F0414-F0423, "
        "DIALOG.C F0424-F0425; no menu action, save I/O, or dialog render side effect";
    *out = receipt;
    return 1;
}

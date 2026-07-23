#include "csb_v1_f0366_f0385_command_viewport_pc34_compat.h"
#include <string.h>

int csb_v1_f0366_f0385_command_viewport_receipt_pc34(
    const CSB_V1_RuntimeProfile *profile,
    const CSB_V1_CSBGraphicsStartupPackage *package,
    CSB_V1_F0366F0385CommandViewportReceiptPc34 *out)
{
    CSB_V1_F0366F0385CommandViewportReceiptPc34 r;
    int square;
    if (!out) return 0;
    memset(&r, 0, sizeof(r)); *out = r;
    if (!profile || !package || !profile->party_state_valid ||
        !profile->dungeon_handle || !profile->dungeon_handle->raw_data ||
        profile->dungeon_handle->square_bytes != 1 || !profile->graphics_asset.path ||
        !package->hud_ready || !csb_v1_csbgraphics_startup_package_surface_draw_eligible(
            package, CSB_V1_CSBGRAPHICS_STARTUP_ASSET_HUD_INVENTORY) ||
        strcmp(profile->graphics_asset.path, package->palette_source.source_path) != 0) return 0;
    square = csb_v1_dungeon_get_raw_square(profile->dungeon_handle, profile->current_level,
                                            profile->party_x, profile->party_y);
    if (square < 0) return 0;
    r.valid = 1; r.map_index = profile->current_level; r.map_x = profile->party_x;
    r.map_y = profile->party_y; r.square_type = (square >> 5) & 7;
    r.source_bound_mask = CSB_V1_F0366_F0385_SOURCE_MASK;
    r.command_owner_required = r.viewport_owner_required = r.menu_owner_required = 1;
    r.source_evidence = "ReDMCSB COMMAND.C F0366-F0380; MENUS.C F0381-F0385 raw PC34 admission; no command, UI, or render side effect";
    *out = r; return 1;
}

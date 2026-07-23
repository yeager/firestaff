#include "csb_v1_f0600_f0620_core_material_pc34_compat.h"

#include <string.h>

int csb_v1_f0600_f0620_core_material_receipt_pc34(
    const CSB_V1_RuntimeProfile *profile,
    const CSB_V1_CSBGraphicsDatRealCache *cache,
    const CSB_V1_CSBGraphicsStartupPackage *package,
    CSB_V1_F0600F0620CoreMaterialReceiptPc34 *out)
{
    CSB_V1_F0600F0620CoreMaterialReceiptPc34 receipt;
    const int role = CSB_V1_CSBGRAPHICS_STARTUP_ASSET_HUD_INVENTORY;
    int square;

    if (!out) return 0;
    memset(&receipt, 0, sizeof(receipt));
    *out = receipt;
    if (!profile || !cache || !package || !profile->party_state_valid ||
        !profile->dungeon_handle || !profile->dungeon_handle->raw_data ||
        profile->dungeon_handle->square_bytes != 1 ||
        !profile->graphics_asset.path || !cache->loaded || !cache->file_buffer ||
        cache->index.count == 0u || !cache->resolved_path[0] ||
        !cache->matched_md5[0] || !package->hud_ready ||
        !csb_v1_csbgraphics_startup_package_surface_draw_eligible(
            package, CSB_V1_CSBGRAPHICS_STARTUP_ASSET_HUD_INVENTORY) ||
        strcmp(profile->graphics_asset.path, cache->resolved_path) != 0 ||
        strcmp(cache->resolved_path, package->palette_source.source_path) != 0 ||
        strcmp(cache->matched_md5, package->palette_source.source_md5) != 0 ||
        strcmp(cache->resolved_path, package->image_sources[role].source_path) != 0 ||
        strcmp(cache->matched_md5, package->image_sources[role].source_md5) != 0) {
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
    receipt.graphics_entry_count = cache->index.count;
    receipt.source_bound_mask = CSB_V1_F0600_F0620_SOURCE_MASK;
    receipt.dialog_owner_required = 1;
    receipt.memory_owner_required = 1;
    receipt.viewport_owner_required = 1;
    receipt.action_name_owner_required = 1;
    receipt.source_evidence =
        "ReDMCSB DIALOG/MEMORY/PANEL/OBJECT/MENU F0600-F0620; "
        "no dialog, memory, bitmap, zone, slot-box, or action-name side effect";
    *out = receipt;
    return 1;
}

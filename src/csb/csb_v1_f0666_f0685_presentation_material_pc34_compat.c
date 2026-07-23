#include "csb_v1_f0666_f0685_presentation_material_pc34_compat.h"
#include <string.h>

int csb_v1_f0666_f0685_presentation_material_receipt_pc34(
    const CSB_V1_RuntimeProfile *profile,
    const CSB_V1_CSBGraphicsDatRealCache *cache,
    const CSB_V1_CSBGraphicsStartupPackage *package,
    CSB_V1_F0666F0685PresentationMaterialReceiptPc34 *out)
{
    CSB_V1_F0666F0685PresentationMaterialReceiptPc34 r;
    const int role = CSB_V1_CSBGRAPHICS_STARTUP_ASSET_HUD_INVENTORY;
    int square;
    if (!out) return 0;
    memset(&r, 0, sizeof(r)); *out = r;
    if (!profile || !cache || !package || !profile->party_state_valid ||
        !profile->dungeon_handle || !profile->dungeon_handle->raw_data ||
        profile->dungeon_handle->square_bytes != 1 || !profile->graphics_asset.path ||
        !cache->loaded || !cache->file_buffer || !cache->index.count ||
        !cache->resolved_path[0] || !cache->matched_md5[0] || !package->hud_ready ||
        !csb_v1_csbgraphics_startup_package_surface_draw_eligible(package, role) ||
        strcmp(profile->graphics_asset.path, cache->resolved_path) ||
        strcmp(cache->resolved_path, package->palette_source.source_path) ||
        strcmp(cache->matched_md5, package->palette_source.source_md5) ||
        strcmp(cache->resolved_path, package->image_sources[role].source_path) ||
        strcmp(cache->matched_md5, package->image_sources[role].source_md5)) return 0;
    square = csb_v1_dungeon_get_raw_square(profile->dungeon_handle, profile->current_level,
                                            profile->party_x, profile->party_y);
    if (square < 0) return 0;
    r.valid = 1; r.map_index = profile->current_level; r.map_x = profile->party_x;
    r.map_y = profile->party_y; r.square_type = (square >> 5) & 7;
    r.graphics_entry_count = cache->index.count; r.source_bound_mask = CSB_V1_F0666_F0685_SOURCE_MASK;
    r.endgame_owner_required = r.text_input_owner_required = 1;
    r.viewport_bitmap_owner_required = r.pixel_copy_owner_required = 1;
    r.source_evidence = "ReDMCSB ENDGAME/STRING/COMMAND/DUNVIEW/VIDEODRV/IMAGE F0666-F0685; no endgame, text, bitmap, or pixel-copy side effect";
    *out = r; return 1;
}

#include "csb_v1_f0706_f0725_package_admission_pc34_compat.h"

#include <string.h>

static int csb_v1_f0706_f0725_is_graphics_package_function(uint16_t function_id)
{
    return function_id == 707u || function_id == 720u || function_id == 721u;
}

static const char *csb_v1_f0706_f0725_evidence(uint16_t function_id)
{
    switch (function_id) {
    case 707u:
        return "ReDMCSB IO.C F0707: reset only an admitted graphics descriptor buffer";
    case 720u:
        return "ReDMCSB BLTSHRNK.C F0720: packed bitmap shrink source";
    case 721u:
        return "ReDMCSB BLTSHRNK.C F0721: palette-changing packed bitmap shrink source";
    case 706u:
    case 709u:
    case 710u:
    case 711u:
    case 712u:
    case 713u:
    case 714u:
    case 715u:
    case 716u:
    case 717u:
    case 718u:
    case 719u:
        return "ReDMCSB IO/IO2 F0706-F0719: no authenticated CSB package route";
    default:
        return NULL;
    }
}

int csb_v1_f0706_f0725_package_admit_pc34(
    const CSB_V1_RuntimeProfile *profile,
    const CSB_V1_CSBGraphicsDatRealCache *cache,
    const CSB_V1_CSBGraphicsStartupPackage *package,
    uint16_t function_id,
    CSB_V1_F0706F0725PackageReceiptPc34 *out)
{
    CSB_V1_F0706F0725PackageReceiptPc34 receipt;
    const int role = CSB_V1_CSBGRAPHICS_STARTUP_ASSET_TITLE;
    const char *evidence;

    if (!out) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    *out = receipt;

    evidence = csb_v1_f0706_f0725_evidence(function_id);
    if (!evidence) {
        return 0;
    }
    if (!csb_v1_f0706_f0725_is_graphics_package_function(function_id)) {
        /* I/O input, driver and audio routes lack a proven immutable PC34
         * package owner in CSB. They must not acquire a host substitute. */
        return 0;
    }
    if (!profile || !cache || !package || !profile->dungeon_handle ||
        !profile->dungeon_handle->raw_data || !profile->graphics_asset.path ||
        !cache->loaded || !cache->file_buffer || cache->index.count == 0u ||
        !cache->resolved_path[0] || !cache->matched_md5[0] ||
        !package->palette_material_complete ||
        !csb_v1_csbgraphics_startup_package_surface_draw_eligible(package, role) ||
        strcmp(profile->graphics_asset.path, cache->resolved_path) != 0 ||
        strcmp(cache->resolved_path, package->palette_source.source_path) != 0 ||
        strcmp(cache->matched_md5, package->palette_source.source_md5) != 0 ||
        strcmp(cache->resolved_path, package->image_sources[role].source_path) != 0 ||
        strcmp(cache->matched_md5, package->image_sources[role].source_md5) != 0) {
        return 0;
    }

    receipt.valid = 1;
    receipt.function_id = function_id;
    receipt.graphics_entry_index = package->assets[role].entry_index;
    receipt.graphics_entry_count = cache->index.count;
    receipt.graphics_package_admitted = 1;
    receipt.existing_owner_required = 1;
    receipt.runtime_execution_blocked = 1;
    receipt.source_evidence = evidence;
    *out = receipt;
    return 1;
}

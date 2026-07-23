#include "csb_v1_f0766_f0785_source_admission_pc34_compat.h"

#include <string.h>

static const char *csb_v1_f0766_f0785_evidence(uint16_t function_id)
{
    switch (function_id) {
    case 766u:
        return "ReDMCSB BASE.C F0766: PC34 title bitmap blit source";
    case 768u:
        return "ReDMCSB TEXT.C F0768: no authenticated CSB font/text package owner";
    case 770u:
    case 771u:
    case 772u:
    case 773u:
    case 774u:
    case 775u:
    case 776u:
    case 777u:
    case 778u:
    case 779u:
    case 780u:
        return "ReDMCSB FILE.C F0770-F0780: no authenticated writable PC34 package route";
    case 781u:
    case 783u:
    case 784u:
    case 785u:
        return "ReDMCSB IO.C F0781/F0783-F0785: no authenticated PC34 input package route";
    default:
        return NULL;
    }
}

int csb_v1_f0766_f0785_source_admit_pc34(
    const CSB_V1_RuntimeProfile *profile,
    const CSB_V1_CSBGraphicsDatRealCache *cache,
    const CSB_V1_CSBGraphicsStartupPackage *package,
    uint16_t function_id,
    CSB_V1_F0766F0785SourceReceiptPc34 *out)
{
    CSB_V1_F0766F0785SourceReceiptPc34 receipt;
    const int role = CSB_V1_CSBGRAPHICS_STARTUP_ASSET_TITLE;
    const char *evidence;

    if (!out) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    *out = receipt;

    evidence = csb_v1_f0766_f0785_evidence(function_id);
    if (!evidence || function_id != 766u) {
        return 0;
    }
    if (!profile || !cache || !package || !profile->graphics_asset.path ||
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
    receipt.source_entry_index = package->assets[role].entry_index;
    receipt.source_entry_count = cache->index.count;
    receipt.authentic_package_required = 1;
    receipt.existing_owner_required = 1;
    receipt.runtime_execution_blocked = 1;
    receipt.source_evidence = evidence;
    *out = receipt;
    return 1;
}

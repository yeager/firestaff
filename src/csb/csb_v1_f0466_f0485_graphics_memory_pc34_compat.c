#include "csb_v1_f0466_f0485_graphics_memory_pc34_compat.h"

#include <string.h>

int csb_v1_f0466_f0485_graphics_memory_receipt_pc34(
    const CSB_V1_CSBGraphicsDatRealCache *cache,
    const CSB_V1_CSBGraphicsStartupPackage *package,
    CSB_V1_F0466F0485GraphicsMemoryReceiptPc34 *out)
{
    CSB_V1_F0466F0485GraphicsMemoryReceiptPc34 receipt;
    const int role = CSB_V1_CSBGRAPHICS_STARTUP_ASSET_HUD_INVENTORY;

    if (!out) return 0;
    memset(&receipt, 0, sizeof(receipt));
    *out = receipt;

    if (!cache || !package || !cache->loaded || !cache->file_buffer ||
        cache->file_size == 0u || cache->index.count == 0u ||
        cache->index.payload_offset >= cache->file_size ||
        cache->index.payload_bytes_avail == 0u ||
        !cache->resolved_path[0] || !cache->matched_md5[0] ||
        !package->hud_ready ||
        !csb_v1_csbgraphics_startup_package_surface_draw_eligible(
            package, CSB_V1_CSBGRAPHICS_STARTUP_ASSET_HUD_INVENTORY) ||
        strcmp(cache->resolved_path, package->palette_source.source_path) != 0 ||
        strcmp(cache->matched_md5, package->palette_source.source_md5) != 0 ||
        strcmp(cache->resolved_path, package->image_sources[role].source_path) != 0 ||
        strcmp(cache->matched_md5, package->image_sources[role].source_md5) != 0 ||
        package->image_sources[role].entry_span.entry_index >= cache->index.count ||
        package->image_sources[role].entry_span.payload_offset <
            cache->index.payload_offset ||
        package->image_sources[role].entry_span.payload_offset +
            package->image_sources[role].entry_span.compressed_size > cache->file_size) {
        return 0;
    }

    receipt.valid = 1;
    receipt.graphics_entry_count = cache->index.count;
    receipt.payload_offset = cache->index.payload_offset;
    receipt.payload_bytes = cache->index.payload_bytes_avail;
    receipt.source_bound_mask = CSB_V1_F0466_F0485_SOURCE_MASK;
    receipt.expand_owner_required = 1;
    receipt.memory_owner_required = 1;
    receipt.cache_owner_required = 1;
    receipt.source_evidence =
        "ReDMCSB EXPAND.C F0466; MEMORY.C F0467-F0485; "
        "no expansion, allocation, cache, file-handle, or bitmap side effect";
    *out = receipt;
    return 1;
}

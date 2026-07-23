#include "csb_v1_f0466_f0485_graphics_memory_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int passed;
static int failed;

#define CHECK(condition, message) do { \
    if (condition) { ++passed; printf("PASS: %s\n", message); } \
    else { ++failed; printf("FAIL: %s\n", message); } \
} while (0)

int main(void)
{
    CSB_V1_CSBGraphicsDatRealCache cache;
    CSB_V1_CSBGraphicsStartupPackage package;
    CSB_V1_F0466F0485GraphicsMemoryReceiptPc34 receipt;
    uint8_t bytes[64];
    const int role = CSB_V1_CSBGRAPHICS_STARTUP_ASSET_HUD_INVENTORY;

    memset(&cache, 0, sizeof(cache));
    memset(&package, 0, sizeof(package));
    memset(bytes, 0, sizeof(bytes));
    cache.loaded = 1;
    cache.file_buffer = bytes;
    cache.file_size = sizeof(bytes);
    cache.index.count = 18;
    cache.index.payload_offset = 32;
    cache.index.payload_bytes_avail = 32;
    snprintf(cache.resolved_path, sizeof(cache.resolved_path), "%s",
             "/pc34/CSBgraphics.dat");
    snprintf(cache.matched_md5, sizeof(cache.matched_md5), "%s",
             "0123456789abcdef0123456789abcdef");
    package.valid = 1;
    package.hud_ready = 1;
    package.palette_material_complete = 1;
    package.palette_source.valid = 1;
    package.palette_source.source_kind =
        CSB_V1_CSBGRAPHICS_PALETTE_SOURCE_CSBGRAPHICS_DAT;
    snprintf(package.palette_source.source_path,
             sizeof(package.palette_source.source_path), "%s",
             cache.resolved_path);
    snprintf(package.palette_source.source_md5,
             sizeof(package.palette_source.source_md5), "%s",
             cache.matched_md5);
    package.assets[role].present = 1;
    package.assets[role].entry_index = 17;
    package.assets[role].decompressed_size = 1;
    package.image_sources[role].valid = 1;
    package.image_sources[role].source_kind =
        CSB_V1_CSBGRAPHICS_PALETTE_SOURCE_CSBGRAPHICS_DAT;
    snprintf(package.image_sources[role].source_path,
             sizeof(package.image_sources[role].source_path), "%s",
             cache.resolved_path);
    snprintf(package.image_sources[role].source_md5,
             sizeof(package.image_sources[role].source_md5), "%s",
             cache.matched_md5);
    package.image_sources[role].entry_span.entry_index = 17;
    package.image_sources[role].entry_span.payload_offset = 48;
    package.image_sources[role].entry_span.compressed_size = 1;
    package.image_sources[role].entry_span.decompressed_size = 1;

    CHECK(csb_v1_f0466_f0485_graphics_memory_receipt_pc34(
              &cache, &package, &receipt) &&
              receipt.source_bound_mask == CSB_V1_F0466_F0485_SOURCE_MASK &&
              receipt.expand_owner_required && receipt.memory_owner_required &&
              receipt.cache_owner_required,
          "F0466-F0485 hash-admitted graphics receipt retains original owners");

    package.image_sources[role].entry_span.payload_offset = 63;
    package.image_sources[role].entry_span.compressed_size = 2;
    CHECK(!csb_v1_f0466_f0485_graphics_memory_receipt_pc34(
              &cache, &package, &receipt),
          "out-of-range original graphics entry fails closed");

    printf("%d/%d\n", passed, passed + failed);
    return failed ? 1 : 0;
}

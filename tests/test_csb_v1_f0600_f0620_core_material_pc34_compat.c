#include "csb_v1_f0600_f0620_core_material_pc34_compat.h"

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
    CSB_V1_RuntimeProfile profile;
    CSB_V1_DungeonData dungeon;
    CSB_V1_CSBGraphicsDatRealCache cache;
    CSB_V1_CSBGraphicsStartupPackage package;
    CSB_V1_F0600F0620CoreMaterialReceiptPc34 receipt;
    uint8_t raw[80];
    uint8_t bytes[64];
    const int role = CSB_V1_CSBGRAPHICS_STARTUP_ASSET_HUD_INVENTORY;

    memset(&profile, 0, sizeof(profile));
    memset(&dungeon, 0, sizeof(dungeon));
    memset(&cache, 0, sizeof(cache));
    memset(&package, 0, sizeof(package));
    memset(raw, 0, sizeof(raw));
    memset(bytes, 0, sizeof(bytes));
    dungeon.raw_data = raw;
    dungeon.raw_size = sizeof(raw);
    dungeon.square_bytes = 1;
    dungeon.level_count = 1;
    dungeon.level_widths[0] = 1;
    dungeon.level_heights[0] = 1;
    profile.dungeon_handle = &dungeon;
    profile.party_state_valid = 1;
    profile.graphics_asset.path = "/pc34/CSBgraphics.dat";
    cache.loaded = 1;
    cache.file_buffer = bytes;
    cache.file_size = sizeof(bytes);
    cache.index.count = 18;
    snprintf(cache.resolved_path, sizeof(cache.resolved_path), "%s",
             profile.graphics_asset.path);
    snprintf(cache.matched_md5, sizeof(cache.matched_md5), "%s",
             "0123456789abcdef0123456789abcdef");
    package.valid = 1;
    package.hud_ready = 1;
    package.palette_material_complete = 1;
    package.palette_source.valid = 1;
    package.palette_source.source_kind = CSB_V1_CSBGRAPHICS_PALETTE_SOURCE_CSBGRAPHICS_DAT;
    snprintf(package.palette_source.source_path, sizeof(package.palette_source.source_path), "%s", cache.resolved_path);
    snprintf(package.palette_source.source_md5, sizeof(package.palette_source.source_md5), "%s", cache.matched_md5);
    package.assets[role].present = 1;
    package.assets[role].entry_index = 17;
    package.assets[role].decompressed_size = 1;
    package.image_sources[role].valid = 1;
    package.image_sources[role].source_kind = CSB_V1_CSBGRAPHICS_PALETTE_SOURCE_CSBGRAPHICS_DAT;
    snprintf(package.image_sources[role].source_path, sizeof(package.image_sources[role].source_path), "%s", cache.resolved_path);
    snprintf(package.image_sources[role].source_md5, sizeof(package.image_sources[role].source_md5), "%s", cache.matched_md5);
    package.image_sources[role].entry_span.entry_index = 17;
    package.image_sources[role].entry_span.compressed_size = 1;
    package.image_sources[role].entry_span.decompressed_size = 1;

    CHECK(csb_v1_f0600_f0620_core_material_receipt_pc34(
              &profile, &cache, &package, &receipt) &&
              receipt.source_bound_mask == CSB_V1_F0600_F0620_SOURCE_MASK &&
              receipt.dialog_owner_required && receipt.memory_owner_required &&
              receipt.viewport_owner_required && receipt.action_name_owner_required,
          "F0600-F0620 authentic PC34 material retains runtime owners");
    cache.matched_md5[0] = '\0';
    CHECK(!csb_v1_f0600_f0620_core_material_receipt_pc34(
              &profile, &cache, &package, &receipt),
          "missing graphics hash provenance fails closed");

    printf("%d/%d\n", passed, passed + failed);
    return failed ? 1 : 0;
}

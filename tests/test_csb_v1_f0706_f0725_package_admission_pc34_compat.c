#include "csb_v1_f0706_f0725_package_admission_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void check(int condition, const char *message)
{
    if (!condition) {
        ++failures;
        fprintf(stderr, "FAIL: %s\n", message);
    }
}

int main(void)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_DungeonData dungeon;
    CSB_V1_CSBGraphicsDatRealCache cache;
    CSB_V1_CSBGraphicsStartupPackage package;
    CSB_V1_F0706F0725PackageReceiptPc34 receipt;
    uint8_t dungeon_bytes[1] = { 0 };
    uint8_t graphics_bytes[64] = { 0 };
    const int role = CSB_V1_CSBGRAPHICS_STARTUP_ASSET_TITLE;

    memset(&profile, 0, sizeof(profile));
    memset(&dungeon, 0, sizeof(dungeon));
    memset(&cache, 0, sizeof(cache));
    memset(&package, 0, sizeof(package));

    dungeon.raw_data = dungeon_bytes;
    dungeon.raw_size = sizeof(dungeon_bytes);
    profile.dungeon_handle = &dungeon;
    profile.graphics_asset.path = "/pc34/CSBgraphics.dat";
    cache.loaded = 1;
    cache.file_buffer = graphics_bytes;
    cache.file_size = sizeof(graphics_bytes);
    cache.index.count = 18u;
    snprintf(cache.resolved_path, sizeof(cache.resolved_path), "%s",
             profile.graphics_asset.path);
    snprintf(cache.matched_md5, sizeof(cache.matched_md5), "%s",
             "0123456789abcdef0123456789abcdef");
    package.valid = 1;
    package.palette_material_complete = 1;
    package.palette_source.valid = 1;
    package.palette_source.source_kind =
        CSB_V1_CSBGRAPHICS_PALETTE_SOURCE_CSBGRAPHICS_DAT;
    snprintf(package.palette_source.source_path,
             sizeof(package.palette_source.source_path), "%s", cache.resolved_path);
    snprintf(package.palette_source.source_md5,
             sizeof(package.palette_source.source_md5), "%s", cache.matched_md5);
    package.assets[role].present = 1;
    package.assets[role].entry_index = 7u;
    package.assets[role].decompressed_size = 1u;
    package.image_sources[role].valid = 1;
    package.image_sources[role].source_kind =
        CSB_V1_CSBGRAPHICS_PALETTE_SOURCE_CSBGRAPHICS_DAT;
    snprintf(package.image_sources[role].source_path,
             sizeof(package.image_sources[role].source_path), "%s", cache.resolved_path);
    snprintf(package.image_sources[role].source_md5,
             sizeof(package.image_sources[role].source_md5), "%s", cache.matched_md5);
    package.image_sources[role].entry_span.entry_index = 7u;
    package.image_sources[role].entry_span.compressed_size = 1u;
    package.image_sources[role].entry_span.decompressed_size = 1u;

    check(csb_v1_f0706_f0725_package_admit_pc34(
              &profile, &cache, &package, 707u, &receipt) == 1,
          "F0707 admits only exact CSBgraphics package provenance");
    check(csb_v1_f0706_f0725_package_admit_pc34(
              &profile, &cache, &package, 720u, &receipt) == 1,
          "F0720 admits only exact CSBgraphics package provenance");
    check(receipt.graphics_package_admitted && receipt.existing_owner_required &&
              receipt.runtime_execution_blocked && receipt.graphics_entry_index == 7u,
          "admission remains read-only and preserves the existing owner");
    check(csb_v1_f0706_f0725_package_admit_pc34(
              &profile, &cache, &package, 721u, &receipt) == 1,
          "F0721 admits the same exact bitmap and palette provenance");
    package.image_sources[role].source_md5[0] = '\0';
    check(csb_v1_f0706_f0725_package_admit_pc34(
              &profile, &cache, &package, 721u, &receipt) == 0,
          "missing image provenance fails closed");
    check(csb_v1_f0706_f0725_package_admit_pc34(
              &profile, &cache, &package, 709u, &receipt) == 0,
          "sound route remains blocked without a proven CSB package owner");
    check(csb_v1_f0706_f0725_package_admit_pc34(
              &profile, &cache, &package, 722u, &receipt) == 0,
          "unnumbered F0722 route remains fail-closed");

    printf("csb_v1_f0706_f0725_package_admission: %s\n",
           failures ? "FAIL" : "PASS");
    return failures ? 1 : 0;
}

#include "csb_v1_f0766_f0785_source_admission_pc34_compat.h"

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
    CSB_V1_CSBGraphicsDatRealCache cache;
    CSB_V1_CSBGraphicsStartupPackage package;
    CSB_V1_F0766F0785SourceReceiptPc34 receipt;
    uint8_t graphics_bytes[64] = { 0 };
    const int role = CSB_V1_CSBGRAPHICS_STARTUP_ASSET_TITLE;

    memset(&profile, 0, sizeof(profile));
    memset(&cache, 0, sizeof(cache));
    memset(&package, 0, sizeof(package));
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
    package.assets[role].entry_index = 3u;
    package.assets[role].decompressed_size = 1u;
    package.image_sources[role].valid = 1;
    package.image_sources[role].source_kind =
        CSB_V1_CSBGRAPHICS_PALETTE_SOURCE_CSBGRAPHICS_DAT;
    snprintf(package.image_sources[role].source_path,
             sizeof(package.image_sources[role].source_path), "%s", cache.resolved_path);
    snprintf(package.image_sources[role].source_md5,
             sizeof(package.image_sources[role].source_md5), "%s", cache.matched_md5);
    package.image_sources[role].entry_span.entry_index = 3u;
    package.image_sources[role].entry_span.compressed_size = 1u;
    package.image_sources[role].entry_span.decompressed_size = 1u;

    check(csb_v1_f0766_f0785_source_admit_pc34(
              &profile, &cache, &package, 766u, &receipt) == 1,
          "F0766 admits exact title bitmap provenance");
    check(receipt.existing_owner_required && receipt.runtime_execution_blocked &&
              receipt.source_entry_index == 3u,
          "F0766 admission does not draw or create a host fallback");
    package.palette_source.source_md5[0] = '\0';
    check(csb_v1_f0766_f0785_source_admit_pc34(
              &profile, &cache, &package, 766u, &receipt) == 0,
          "palette provenance drift fails closed");
    check(csb_v1_f0766_f0785_source_admit_pc34(
              &profile, &cache, &package, 768u, &receipt) == 0,
          "F0768 text remains blocked without a real font package");
    check(csb_v1_f0766_f0785_source_admit_pc34(
              &profile, &cache, &package, 777u, &receipt) == 0,
          "F0777 file delete cannot acquire host file behavior");
    check(csb_v1_f0766_f0785_source_admit_pc34(
              &profile, &cache, &package, 781u, &receipt) == 0,
          "F0781 mouse handler cannot acquire synthetic input");
    check(csb_v1_f0766_f0785_source_admit_pc34(
              &profile, &cache, &package, 782u, &receipt) == 0,
          "unnumbered F0782 remains fail-closed");

    printf("csb_v1_f0766_f0785_source_admission: %s\n",
           failures ? "FAIL" : "PASS");
    return failures ? 1 : 0;
}

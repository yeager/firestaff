/* Emit a digest of the authenticated FM Towns SKULL menu for the native
 * post-present screenshot check. TITLE/0/4 and its palette stay in memory. */

#include "dm2_v1_asset_loader.h"
#include "dm2_v1_boot.h"

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    DM2_V1_BootProfile profile;
    DM2_V1_AssetLoader loader = {0};
    const uint8_t *palette;
    const uint8_t *pixels;
    size_t palette_size = 0u;
    size_t pixel_size = 0u;
    uint64_t digest = UINT64_C(14695981039346656037);
    int x, y, channel;
    int ok = 0;

    if (argc != 2) {
        fputs("usage: test_dm2_v1_fmtowns_menu_source_rgb <original-zip>\n",
              stderr);
        return 2;
    }
    dm2_v1_boot_profile_init(&profile);
    if (dm2_v1_boot_scan_assets(&profile, argv[1]) != 0 ||
        profile.platform != DM2_PLATFORM_FMTOWNS_JA ||
        !profile.assets_verified || !profile.graphics_mem ||
        !profile.graphics_mem_size ||
        dm2_v1_asset_loader_init(
            &loader, profile.graphics_mem, profile.graphics_mem_size) != 0) {
        fputs("FAIL: authentic FM Towns boot profile unavailable\n", stderr);
        goto cleanup;
    }
    palette = dm2_v1_asset_load_typed_sized(
        &loader, DM2_GDAT_CATEGORY_TITLE, 0, DM2_GDAT_ENTRY_TYPE_PAL_IRGB,
        4, &palette_size);
    /* HME-242 keeps SKULL's 320x200 TITLE/0/4 surface as the untyped
     * equivalent of SKProject SHOW_MENU_SCREEN's RAW7/4 source selection.
     * Read that exact source entry before New Game or dungeon loading. */
    pixels = dm2_v1_asset_load_sized(
        &loader, DM2_GDAT_CATEGORY_TITLE, 0, 4, &pixel_size);
    if (!palette || palette_size != 64u || !pixels ||
        pixel_size != 320u * 200u) {
        fputs("FAIL: authentic FM Towns SKULL menu unavailable\n", stderr);
        goto cleanup;
    }
    for (x = 0; x < 16; ++x) {
        if (palette[x * 4] != (uint8_t)x) {
            fputs("FAIL: FM Towns SKULL palette index mismatch\n", stderr);
            goto cleanup;
        }
    }
    /* Same RGB channel order as the M11 FM Towns presenter. The test hashes
     * pixels in display order; the screenshot reader reverses BMP's BGR. */
    for (y = 0; y < 200; ++y) {
        for (x = 0; x < 320; ++x) {
            uint8_t index = pixels[y * 320 + x];
            if (index >= 16u) {
                fputs("FAIL: FM Towns SKULL pixel exceeds source palette\n",
                      stderr);
                goto cleanup;
            }
            for (channel = 1; channel <= 3; ++channel) {
                digest ^= palette[index * 4 + channel];
                digest *= UINT64_C(1099511628211);
            }
        }
    }
    printf("%016" PRIx64 "\n", digest);
    ok = 1;
cleanup:
    dm2_v1_asset_loader_free(&loader);
    dm2_v1_boot_cleanup(&profile);
    return ok ? 0 : 1;
}

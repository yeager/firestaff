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
    const DM2_V1_AssetLoader *loader;
    const uint8_t *palette;
    uint8_t *pixels = NULL;
    size_t palette_size = 0u;
    uint64_t digest = UINT64_C(14695981039346656037);
    int width = 0, height = 0, stride = 0;
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
        !profile.assets_verified ||
        dm2_v1_boot_enter_game(&profile) != 0 ||
        !(loader = dm2_v1_boot_asset_loader(&profile))) {
        fputs("FAIL: authentic FM Towns boot profile unavailable\n", stderr);
        goto cleanup;
    }
    palette = dm2_v1_asset_load_typed_sized(
        loader, DM2_GDAT_CATEGORY_TITLE, 0, DM2_GDAT_ENTRY_TYPE_PAL_IRGB,
        4, &palette_size);
    /* The boot-owned GDAT API admits the authentic raw or packed TITLE/0/4
     * surface. enter_game mounts the verified loader; no New Game input is
     * sent to the native CLI whose presented image is checked separately. */
    if (!palette || palette_size != 64u ||
        dm2_v1_boot_gdat_image_asset_fetch(
            &profile, DM2_GDAT_CATEGORY_TITLE, 0, 4,
            &pixels, &width, &height, &stride) != 0 ||
        !pixels || width != 320 || height != 200 || stride < width) {
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
            uint8_t index = pixels[y * stride + x];
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
    dm2_v1_boot_gdat_image_asset_free(pixels);
    dm2_v1_boot_cleanup(&profile);
    return ok ? 0 : 1;
}

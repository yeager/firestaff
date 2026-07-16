#include "csb_v1_boot.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define CHECK(condition)                                                        \
    do {                                                                        \
        if (!(condition)) {                                                     \
            fprintf(stderr, "CHECK failed at %s:%d: %s\n", __FILE__, __LINE__, \
                    #condition);                                                \
            exit(1);                                                            \
        }                                                                       \
    } while (0)

static void test_release_null_is_safe(void)
{
    csb_v1_boot_startup_runtime_host_surface_receipt_release_pc34(NULL);
}

static void test_release_frees_owned_raster_and_clears_receipt(void)
{
    CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 receipt;
    unsigned char *pixels = (unsigned char *)malloc(4u);

    CHECK(pixels != NULL);
    pixels[0] = 1u;
    pixels[1] = 2u;
    pixels[2] = 3u;
    pixels[3] = 4u;

    receipt.valid = 1;
    receipt.real_asset_matched = 1;
    receipt.no_legacy_wrappers = 1;
    receipt.no_synthetic_surface = 1;
    receipt.host_surface = CSB_V1_STARTUP_RUNTIME_HOST_SURFACE_TITLE_PC34;
    receipt.host_surface_hash = 0x12345678u;
    receipt.raster.pixels = pixels;
    receipt.raster.width = 2;
    receipt.raster.height = 2;
    receipt.raster.valid = 1;
    receipt.raster.real_asset_matched = 1;
    receipt.raster.pixel_hash = 0x01020304u;
    receipt.source_evidence = "test";

    csb_v1_boot_startup_runtime_host_surface_receipt_release_pc34(&receipt);

    CHECK(receipt.valid == 0);
    CHECK(receipt.real_asset_matched == 0);
    CHECK(receipt.no_legacy_wrappers == 0);
    CHECK(receipt.no_synthetic_surface == 0);
    CHECK(receipt.host_surface == CSB_V1_STARTUP_RUNTIME_HOST_SURFACE_NONE_PC34);
    CHECK(receipt.host_surface_hash == 0u);
    CHECK(receipt.raster.pixels == NULL);
    CHECK(receipt.raster.valid == 0);
    CHECK(receipt.raster.pixel_hash == 0u);
    CHECK(receipt.source_evidence == NULL);
}

int main(void)
{
    test_release_null_is_safe();
    test_release_frees_owned_raster_and_clears_receipt();
    return 0;
}

#include "csb_v1_boot.h"
#include "csb_v1_fmtowns_portrait.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_pass = 0, g_fail = 0;

#define ASSERT(cond, msg) do { \
    if (cond) { g_pass++; } \
    else { g_fail++; printf("FAIL: %s (line %d)\n", msg, __LINE__); } \
} while (0)

static void test_probe_null(void) {
    ASSERT(csb_v1_fmtowns_portrait_probe(NULL, 0) == 0, "probe rejects NULL");
    uint8_t small[4] = {0};
    ASSERT(csb_v1_fmtowns_portrait_probe(small, 4) == 0, "probe rejects small");
}

static void test_f31_planar_decode(void) {
    uint8_t data[CSB_FMTOWNS_PORTRAIT_FILE_SIZE];
    uint8_t pixels[CSB_FMTOWNS_PORTRAIT_PIXEL_COUNT];

    memset(data, 0, sizeof(data));
    data[0] = 0x91u;
    data[1] = 0xa7u;
    data[6] = 0x00u;
    data[7] = 0x01u;
    /* First 16-pixel group: plane 0 and 2 set x=0, plane 1 and 3 set x=1.
     * PORTRAIT.C F7251 must therefore produce palette colours 5 and 10. */
    data[44] = 0x80u;
    data[46] = 0x40u;
    data[48] = 0x80u;
    data[50] = 0x40u;
    ASSERT(csb_v1_fmtowns_portrait_decode(data, sizeof(data), pixels,
                                            sizeof(pixels), NULL) == 1,
           "F31 portrait planar fixture decodes");
    ASSERT(pixels[0] == 5u && pixels[1] == 10u && pixels[2] == 0u,
           "F31 portrait preserves F7251 plane and pixel-pair order");
}

static void test_f31_c06_header_gate(void) {
    uint8_t data[CSB_FMTOWNS_PORTRAIT_FILE_SIZE];

    memset(data, 0, sizeof(data));
    data[0] = 0x91u;
    data[1] = 0xa7u;
    data[7] = 0x01u;
    ASSERT(csb_v1_fmtowns_portrait_probe(data, sizeof(data)) == 1,
           "F31 C06 accepts a native CMP header");
    data[7] = 0x00u;
    ASSERT(csb_v1_fmtowns_portrait_probe(data, sizeof(data)) == 0,
           "F31 C06 rejects CMP without its format marker");
    data[7] = 0x01u;
    data[8] = 0x80u;
    ASSERT(csb_v1_fmtowns_portrait_probe(data, sizeof(data)) == 0,
           "F31 C06 rejects disabled CMP flag");
}

static int test_real_portraits(void) {
    const char *archive = getenv("FIRESTAFF_CSB_FMTOWNS_ARCHIVE");
    const char *language = getenv("FIRESTAFF_CSB_FMTOWNS_GAME_LANGUAGE");
    CSB_V1_BootStartupLaunch_PC34 launch;
    const CSB_V1_BootProfile *profile;
    int decoded = 0, i;

    if (!archive || !archive[0]) {
        printf("SKIP: FIRESTAFF_CSB_FMTOWNS_ARCHIVE not set\n");
        return 77;
    }
    memset(&launch, 0, sizeof(launch));
    if (!csb_v1_boot_startup_launch_alloc_with_variant_pc34(
            archive, NULL, NULL, NULL, NULL,
            language && strcmp(language, "ja") == 0
                ? CSB_V1_VARIANT_FMTOWNS_JA : CSB_V1_VARIANT_FMTOWNS_EN,
            &launch)) {
        ASSERT(0, "packed F31 archive admits through the native boot path");
        csb_v1_boot_startup_launch_cleanup_pc34(&launch);
        return 0;
    }
    profile = launch.profile;
    ASSERT(profile && profile->fmtowns_portrait_count == 24u,
           "native packed F31 profile retains all 24 C06 portraits in RAM");
    if (!profile || profile->fmtowns_portrait_count != 24u) {
        csb_v1_boot_startup_launch_cleanup_pc34(&launch);
        return 0;
    }

    for (i = 0; i < 24; i++) {
        const uint8_t *data = profile->fmtowns_portrait_bytes[i];
        size_t size = profile->fmtowns_portrait_sizes[i];
        uint8_t pixels[CSB_FMTOWNS_PORTRAIT_PIXEL_COUNT];
        CSB_V1_FmtownsPortraitReceipt receipt;

        ASSERT(csb_v1_fmtowns_portrait_probe(data, size) == 1,
               "probe accepts portrait");

        if (csb_v1_fmtowns_portrait_decode(data, size, pixels,
                sizeof(pixels), &receipt)) {
            ASSERT(receipt.valid == 1, "receipt valid");
            ASSERT(receipt.pixel_fnv1a != 0, "pixel hash non-zero");
            ASSERT(strlen(receipt.name) > 0, "name non-empty");

            /* Verify meaningful pixel content */
            {
                size_t nonzero = 0, j;
                for (j = 0; j < CSB_FMTOWNS_PORTRAIT_PIXEL_COUNT; j++) {
                    if (pixels[j] != 0) nonzero++;
                    ASSERT(pixels[j] <= 15, "pixel is 4bpp");
                }
                ASSERT(nonzero > 100, "portrait has visible content");
            }
            decoded++;
            if (i < 4) {
                printf("  %s (%s): hash=%08x\n",
                       receipt.name, receipt.title, receipt.pixel_fnv1a);
            }
        }
    }

    printf("  Decoded %d/24 portraits\n", decoded);
    ASSERT(decoded == 24, "all 24 portraits decoded");
    csb_v1_boot_startup_launch_cleanup_pc34(&launch);
    return 0;
}

int main(void) {
    int real_portrait_result;
    test_probe_null();
    test_f31_planar_decode();
    test_f31_c06_header_gate();
    real_portrait_result = test_real_portraits();
    if (real_portrait_result == 77) return 77;
    printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail > 0 ? 1 : 0;
}

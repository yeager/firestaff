#include "dm1_v1_amiga_entrance_f0441.h"
#include "dm1_v1_amiga_graphics_dat.h"
#include "dm1_v1_amiga_swsh.h"
#include "dm1_v1_amiga_title_f0437.h"
#include "firestaff_zip_extract.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void md5_hex(const uint8_t md5[16], char out[33]) {
    static const char digits[] = "0123456789abcdef";
    unsigned int index;
    for (index = 0u; index < 16u; ++index) {
        out[index * 2u] = digits[md5[index] >> 4u];
        out[index * 2u + 1u] = digits[md5[index] & 0x0fu];
    }
    out[32] = '\0';
}

int main(void) {
    const char *archive = getenv("FIRESTAFF_DM1_AMIGA_DEMO_ARCHIVE");
    uint8_t *graphics = NULL;
    uint8_t *swoosh = NULL;
    size_t graphics_bytes = 0u;
    size_t swoosh_bytes = 0u;
    DM1_V1_AmigaGraphicsReceipt graphics_receipt;
    DM1_V1_AmigaSwshAssets swsh_receipt;
    DM1_V1_AmigaTitleF0437Receipt title_receipt;
    DM1_V1_AmigaEntranceF0441Receipt entrance_receipt;
    int result = 1;

    if (!archive || !archive[0]) {
        archive = getenv("HOME");
        if (archive && archive[0]) {
            static char default_archive[1024];
            (void)snprintf(default_archive, sizeof(default_archive),
                           "%s/.firestaff/data/dm1/dm-amiga-demo-en.zip",
                           archive);
            archive = default_archive;
        }
    }
    if (!archive || !archive[0]) {
        puts("SKIP: authentic DM1 Amiga demo archive is not staged");
        return 77;
    }
    if (firestaff_zip_extract_by_name(archive, "graphics.dat", &graphics,
                                      &graphics_bytes) != 0) {
        puts("SKIP: authentic DM1 Amiga demo archive is not staged");
        return 77;
    }
    if (firestaff_zip_extract_by_name(archive, "swoosh", &swoosh,
                                      &swoosh_bytes) != 0) {
        fprintf(stderr, "FAIL: authentic Amiga demo SWSH member is missing\n");
        goto cleanup;
    }
    if (dm1_v1_amiga_graphics_receipt(graphics, graphics_bytes,
                                      &graphics_receipt) != 0) {
        fprintf(stderr, "FAIL: demo GRAPHICS.DAT receipt rejected\n");
        goto cleanup;
    }
    if (!dm1_v1_amiga_swsh_parse(swoosh, swoosh_bytes, &swsh_receipt)) {
        fprintf(stderr, "FAIL: demo SWSH receipt rejected\n");
        goto cleanup;
    }
    {
        char graphics_md5[33];
        md5_hex(graphics_receipt.md5, graphics_md5);
        if (!graphics_receipt.is_amiga ||
            graphics_receipt.version != DM1_AMIGA_VER_DEMO ||
            graphics_receipt.lang != DM1_AMIGA_LANG_EN ||
            strcmp(swsh_receipt.executableMd5,
                   "28d406007e99b0ae8da0c6f7fc7f183b") != 0) {
            fprintf(stderr,
                    "FAIL: demo identities mismatch graphics=%s version=%d lang=%d swsh=%s\n",
                    graphics_md5, (int)graphics_receipt.version,
                    (int)graphics_receipt.lang, swsh_receipt.executableMd5);
            goto cleanup;
        }
    }
    if (!dm1_v1_amiga_title_f0437_receipt(
            graphics, graphics_bytes, swoosh, swoosh_bytes, &title_receipt)) {
        fprintf(stderr, "FAIL: authentic A20ED F0437 title receipt rejected\n");
        goto cleanup;
    }
    if (!dm1_v1_amiga_entrance_f0441_receipt(
            graphics, graphics_bytes, swoosh, swoosh_bytes,
            &entrance_receipt)) {
        fprintf(stderr, "FAIL: authentic A20ED F0441 entrance receipt rejected\n");
        goto cleanup;
    }
    if (strcmp(title_receipt.graphics_md5,
               "491ca939f9abb33ceeb26619b841fe91") != 0 ||
        strcmp(entrance_receipt.graphics_md5,
               "491ca939f9abb33ceeb26619b841fe91") != 0 ||
        strcmp(entrance_receipt.executable_md5,
               "28d406007e99b0ae8da0c6f7fc7f183b") != 0 ||
        entrance_receipt.opening_steps != 31u ||
        entrance_receipt.door_frame_count != 8u) {
        fprintf(stderr, "FAIL: A20ED startup receipt fields mismatch\n");
        goto cleanup;
    }
    puts("PASS: authentic DM1 Amiga demo F0437 title and F0441 entrance receipts");
    result = 0;

cleanup:
    free(graphics);
    free(swoosh);
    return result;
}

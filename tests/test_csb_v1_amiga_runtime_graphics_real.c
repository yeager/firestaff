/*
 * Real-media admission for the CSB Amiga C017/C040 runtime graphics.
 *
 * ReDMCSB PANEL.C F0346/F0347 installs C017 (inventory) and C040
 * (resurrect/reincarnate) before DUNVIEW.C F0128 draws the live page.
 * The Amiga package stores its DMCSB2 item table in big-endian byte order;
 * this test proves that the production decoder consumes those original
 * records rather than a PC fixture or a generated HUD substitute.
 *
 * Set FIRESTAFF_CSB_AMIGA_GRAPHICS_DAT to a user-supplied original
 * CSB Amiga GRAPHICS.DAT. Hosted CI intentionally has no licensed media and
 * reports SKIP.
 */

#include "csb_v1_amiga_graphics_dat.h"
#include "csb_v1_boot.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned char *read_file(const char *path, size_t *out_size)
{
    FILE *file = NULL;
    long size;
    unsigned char *bytes = NULL;

    if (out_size) {
        *out_size = 0u;
    }
    if (!path || !path[0] || !out_size || !(file = fopen(path, "rb")) ||
        fseek(file, 0L, SEEK_END) != 0 || (size = ftell(file)) <= 0L ||
        fseek(file, 0L, SEEK_SET) != 0 ||
        !(bytes = (unsigned char *)malloc((size_t)size)) ||
        fread(bytes, 1u, (size_t)size, file) != (size_t)size) {
        if (file) {
            fclose(file);
        }
        free(bytes);
        return NULL;
    }
    fclose(file);
    *out_size = (size_t)size;
    return bytes;
}

static int decode_source_hud_surface(const char *path, unsigned int graphic,
                                     const char *label)
{
    CSB_V1_StartupGraphicDecodeReceipt_PC34 receipt;
    unsigned char *pixels = NULL;
    int width = 0;
    int height = 0;
    int ok;

    memset(&receipt, 0, sizeof(receipt));
    ok = csb_v1_boot_decode_graphics_dat_asset_pc34(
             path, graphic, &pixels, &width, &height, &receipt) &&
         pixels != NULL && width > 0 && width <= 640 && height > 0 &&
         height <= 400 && receipt.valid && receipt.ended_at_record_boundary &&
         receipt.indexed_colors_are_4bit &&
         receipt.compressed_record_sha256[0] != '\0';
    free(pixels);
    if (!ok) {
        fprintf(stderr, "FAIL: Amiga %s did not decode from original data\n",
                label);
    }
    return ok;
}

int main(void)
{
    const char *path = getenv("FIRESTAFF_CSB_AMIGA_GRAPHICS_DAT");
    CSB_V1_AmigaGraphicsReceipt graphics;
    unsigned char *bytes;
    size_t size;

    if (!path || !path[0]) {
        puts("SKIP: set FIRESTAFF_CSB_AMIGA_GRAPHICS_DAT to original CSB Amiga GRAPHICS.DAT");
        return 0;
    }
    bytes = read_file(path, &size);
    if (!bytes || csb_v1_amiga_graphics_receipt(bytes, size, &graphics) != 0 ||
        !graphics.is_amiga || graphics.item_count < 700u ||
        graphics.item_count > 800u) {
        free(bytes);
        fputs("FAIL: FIRESTAFF_CSB_AMIGA_GRAPHICS_DAT is not a CSB Amiga DMCSB2 file\n",
              stderr);
        return 1;
    }
    free(bytes);

    if (!decode_source_hud_surface(path, 17u, "C017 inventory") ||
        !decode_source_hud_surface(path, 40u, "C040 panel")) {
        return 1;
    }
    puts("PASS: real CSB Amiga C017/C040 runtime graphics decode");
    return 0;
}

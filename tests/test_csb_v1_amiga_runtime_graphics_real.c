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

static int decode_source_viewport_field_surface(const char *path,
                                                unsigned int graphic,
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
        fprintf(stderr, "FAIL: Amiga viewport %s did not decode from original data\n",
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
        return 77;
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

    unsigned int graphic;

    if (!decode_source_hud_surface(path, 17u, "C017 inventory") ||
        !decode_source_hud_surface(path, 40u, "C040 panel") ||
        !decode_source_viewport_field_surface(path, 41u, "C041 Thieves Eye hole") ||
        !decode_source_viewport_field_surface(path, 70u, "C070 field mask D3L2") ||
        !decode_source_viewport_field_surface(path, 71u, "C071 field mask D3L") ||
        !decode_source_viewport_field_surface(path, 72u, "C072 field mask D2L2") ||
        !decode_source_viewport_field_surface(path, 73u, "C073 field mask D2L") ||
        !decode_source_viewport_field_surface(path, 74u, "C074 field mask D1L") ||
        !decode_source_viewport_field_surface(path, 75u, "C075 field mask D0L") ||
        !decode_source_viewport_field_surface(path, 76u, "C076 teleporter field") ||
        !decode_source_viewport_field_surface(path, 77u, "C077 fluxcage field")) {
        return 1;
    }
    /* ReDMCSB DUNVIEW.C F0104/F0112 (lines 3128-3242/4341-4458) consumes
     * C049--C062 floor-pit and C063--C069 ceiling-pit bitmaps for the
     * MEDIA720 Amiga route.  Every member needs a real decoder receipt: a
     * structurally valid field record cannot stand in for an absent pit.
     */
    for (graphic = 49u; graphic <= 69u; ++graphic) {
        if (!decode_source_viewport_field_surface(path, graphic,
                                                  "C049-C069 pit family")) {
            return 1;
        }
    }
    /* ReDMCSB DEFS.H MEDIA720_A31E_A31M_A33M_A35E_A35M declares
     * M645=108, C018=18 and M647=40.  DUNVIEW.C F0096 installs each
     * wall set's eighteen pre-scaled stair pictures at M645+WallSet*M647;
     * the authenticated CSB dungeon uses the four contiguous blocks
     * 108..245 before M633=246 starts the door pictures. */
    for (graphic = 108u; graphic < 246u; ++graphic) {
        if (!decode_source_viewport_field_surface(path, graphic,
                                                  "M645 stair family")) {
            return 1;
        }
    }
    /* ReDMCSB DEFS.H MEDIA720_A31E_A31M_A33M_A35E_A35M declares
     * M615=259 and M616=385.  DUNVIEW.C F0107 consumes the complete
     * M615..M616-1 wall-ornament family before it reaches F0115's thing
     * pass.  Prove the production Amiga decoder can admit every original
     * pre-scaled wall record, rather than allowing the runtime to replace
     * a missing torch/alcove/mirror with PC34-derived pixels. */
    for (graphic = 259u; graphic < 385u; ++graphic) {
        if (!decode_source_viewport_field_surface(path, graphic,
                                                  "M615 wall-ornament family")) {
            return 1;
        }
    }
    /* ReDMCSB DEFS.H MEDIA720_A31E_A31M_A33M_A35E_A35M declares
     * M616=385 and M649=439. DUNVIEW.C F0096:2733-2743 resolves each
     * active floor ornament to M616 + ornament * 6 and F0108 draws its
     * perspective bitmap. All 54 source records must decode natively;
     * admitting a wall ornament is not evidence that floor glyphs and
     * footprints are present. */
    for (graphic = 385u; graphic < 439u; ++graphic) {
        if (!decode_source_viewport_field_surface(path, graphic,
                                                  "M616 floor-ornament family")) {
            return 1;
        }
    }
    /* DUNVIEW.C F0111/F0110 consumes the two M649 masks, the twelve M617
     * door ornaments, then the lone M634 door button. MEDIA720 defines the
     * contiguous native 439..453 range; in particular M634=453 is not the
     * PC media C315 record. */
    for (graphic = 439u; graphic <= 453u; ++graphic) {
        if (!decode_source_viewport_field_surface(path, graphic,
                                                  "M649/M617/M634 door family")) {
            return 1;
        }
    }
    puts("PASS: real CSB Amiga HUD, pit, and F0113/F0127 graphics decode");
    return 0;
}

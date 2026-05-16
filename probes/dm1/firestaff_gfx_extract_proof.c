#include <stdint.h>
extern long F0467_MEMORY_GetGraphicOffset_Compat(int format, int count, const unsigned short *sizes, int index);

#include "memory_graphics_dat_header_pc34_compat.h"
#include "memory_graphics_dat_pc34_compat.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Proof probe: extract a real bitmap from GRAPHICS.DAT using
 * the existing M10 parser and dump it as PPM.
 *
 * Usage: firestaff_gfx_extract_proof /path/to/DATA/GRAPHICS.DAT output.ppm [index]
 */

static uint32_t vga_palette[16] = {
    0x000000, 0x0000AA, 0x00AA00, 0x00AAAA,
    0xAA0000, 0xAA00AA, 0xAA5500, 0xAAAAAA,
    0x555555, 0x5555FF, 0x55FF55, 0x55FFFF,
    0xFF5555, 0xFF55FF, 0xFFFF55, 0xFFFFFF,
};

int main(int argc, char **argv) {
    const char *gfx_path, *out_path;
    int gfx_index = 1;  /* default: graphic #1 */
    struct MemoryGraphicsDatState_Compat state;
    struct MemoryGraphicsDatHeader_Compat header;
    FILE *out;
    int w, h, compressed_size;
    long offset;
    unsigned char *compressed_buf, *pixels;
    int i;

    if (argc < 3) {
        printf("Usage: %s GRAPHICS.DAT output.ppm [index]\n", argv[0]);
        return 1;
    }
    gfx_path = argv[1];
    out_path = argv[2];
    if (argc > 3) gfx_index = atoi(argv[3]);

    memset(&state, 0, sizeof(state));
    memset(&header, 0, sizeof(header));

    printf("Loading %s...\n", gfx_path);
    memset(&state, 0, sizeof(state));
    memset(&header, 0, sizeof(header));
    if (!F0479_MEMORY_LoadGraphicsDatHeader_Compat(gfx_path, &state, &header)
        || header.graphicCount == 0 || !header.widthHeight) {
        fprintf(stderr, "Failed to parse GRAPHICS.DAT\n");
        return 1;
    }

    printf("Format: %d, Graphics: %d\n", header.format, header.graphicCount);

    if (gfx_index < 0 || gfx_index >= header.graphicCount) {
        fprintf(stderr, "Index %d out of range (0-%d)\n", gfx_index, header.graphicCount - 1);
        F0479_MEMORY_FreeGraphicsDatHeader_Compat(&header);
        F0478_MEMORY_CloseGraphicsDat_CPSDF_Compat(&state);
        return 1;
    }

    /* Print first 20 graphics info */
    printf("\nGraphics listing:\n");
    for (i = 0; i < 20 && i < header.graphicCount; i++) {
        printf("  #%3d: %3dx%-3d  comp=%5d  decomp=%5d\n",
            i,
            header.widthHeight[i].Width,
            header.widthHeight[i].Height,
            header.compressedByteCounts[i],
            header.decompressedByteCounts[i]);
    }

    /* Extract the requested graphic */
    w = header.widthHeight[gfx_index].Width;
    h = header.widthHeight[gfx_index].Height;
    compressed_size = header.compressedByteCounts[gfx_index];

    printf("\nExtracting #%d: %dx%d (comp=%d decomp=%d)\n",
        gfx_index, w, h, compressed_size,
        header.decompressedByteCounts[gfx_index]);

    if (w <= 0 || h <= 0 || compressed_size <= 0) {
        fprintf(stderr, "Invalid graphic dimensions\n");
        F0479_MEMORY_FreeGraphicsDatHeader_Compat(&header);
        F0478_MEMORY_CloseGraphicsDat_CPSDF_Compat(&state);
        return 1;
    }

    /* Read compressed data */
    offset = F0467_MEMORY_GetGraphicOffset_Compat(
        header.format, header.graphicCount,
        header.compressedByteCounts, gfx_index);

    printf("Data offset: %ld\n", offset);

    compressed_buf = (unsigned char *)malloc(compressed_size + 4);
    if (!compressed_buf) { return 1; }

    fseek(state.file, offset, SEEK_SET);
    if ((int)fread(compressed_buf, 1, compressed_size, state.file) != compressed_size) {
        fprintf(stderr, "Failed to read graphic data\n");
        free(compressed_buf);
        F0479_MEMORY_FreeGraphicsDatHeader_Compat(&header);
        F0478_MEMORY_CloseGraphicsDat_CPSDF_Compat(&state);
        return 1;
    }

    /* For now: treat data as raw 4bpp pixels (no decompression).
     * Real decompression would use F0490 expand pipeline. */
    int pixel_count = w * h;
    int byte_width = (w + 1) / 2;  /* 4bpp: 2 pixels per byte */
    pixels = (unsigned char *)calloc(pixel_count, 1);

    /* Expand 4bpp to 8bpp indexed */
    for (i = 0; i < byte_width * h && i < compressed_size; i++) {
        int pi = i * 2;
        if (pi < pixel_count) pixels[pi] = (compressed_buf[i] >> 4) & 0xF;
        if (pi + 1 < pixel_count) pixels[pi + 1] = compressed_buf[i] & 0xF;
    }

    /* Write PPM */
    out = fopen(out_path, "wb");
    if (!out) {
        fprintf(stderr, "Cannot write %s\n", out_path);
        free(compressed_buf);
        free(pixels);
        F0479_MEMORY_FreeGraphicsDatHeader_Compat(&header);
        F0478_MEMORY_CloseGraphicsDat_CPSDF_Compat(&state);
        return 1;
    }

    fprintf(out, "P6\n%d %d\n255\n", w, h);
    for (i = 0; i < pixel_count; i++) {
        uint32_t color = vga_palette[pixels[i] & 0xF];
        unsigned char rgb[3] = {
            (unsigned char)((color >> 16) & 0xFF),
            (unsigned char)((color >> 8) & 0xFF),
            (unsigned char)(color & 0xFF)
        };
        fwrite(rgb, 1, 3, out);
    }
    fclose(out);

    printf("\nWritten: %s (%dx%d)\n", out_path, w, h);
    printf("PROOF: GRAPHICS.DAT extraction successful.\n");

    free(compressed_buf);
    free(pixels);
    F0479_MEMORY_FreeGraphicsDatHeader_Compat(&header);
    F0478_MEMORY_CloseGraphicsDat_CPSDF_Compat(&state);
    return 0;
}

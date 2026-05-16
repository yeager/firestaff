
#include "firestaff_graphics_dat_reader.h"
#include <string.h>
#include <stdio.h>

/* Extract VGA palette from GRAPHICS.DAT.
 *
 * DM1 PC34 GRAPHICS.DAT stores the 256-color VGA palette
 * as the first resource (index 0). Format: 256 × RGB triplets
 * where each component is 0-63 (VGA 6-bit), scaled to 0-255. */

static uint32_t g_full_vga_palette[256];
static int g_palette_extracted = 0;

int fs_extract_vga_palette(const uint8_t *gfx_data, int gfx_size, uint32_t *palette_out) {
    int offset, count, i;
    const uint8_t *pal;

    if (!gfx_data || gfx_size < 1000 || !palette_out) return -1;

    /* PC34 new format: signature 0x8001 at offset 0-1 */
    if (gfx_data[0] != 0x01 || gfx_data[1] != 0x80) {
        printf("GRAPHICS.DAT: not PC34 new format\n");
        return -1;
    }

    /* Resource count at offset 2-3 (little-endian) */
    count = gfx_data[2] | (gfx_data[3] << 8);
    if (count < 1 || count > 1000) return -1;

    /* Header size: 4 + count*2 (comp sizes) + count*2 (decomp sizes) + count*4 (w/h) */
    /* First resource offset follows header */
    {
        int header_size = 4 + count * 2 + count * 2 + count * 4;
        /* First compressed size at offset 4 */
        int first_comp_size = gfx_data[4] | (gfx_data[5] << 8);

        /* Palette resource: if first resource is 768 bytes (256*3 = palette) */
        if (first_comp_size == 768 || first_comp_size == 0) {
            /* Try uncompressed palette at header_size */
            if (header_size + 768 <= gfx_size) {
                pal = gfx_data + header_size;
                for (i = 0; i < 256; i++) {
                    /* VGA 6-bit to 8-bit: multiply by 4 (or use (val << 2) | (val >> 4)) */
                    int r6 = pal[i*3+0] & 0x3F;
                    int g6 = pal[i*3+1] & 0x3F;
                    int b6 = pal[i*3+2] & 0x3F;
                    int r = (r6 << 2) | (r6 >> 4);
                    int g = (g6 << 2) | (g6 >> 4);
                    int b = (b6 << 2) | (b6 >> 4);
                    palette_out[i] = 0xFF000000 | (r << 16) | (g << 8) | b;
                }
                return 256;
            }
        }
    }

    /* Fallback: scan for 768-byte palette-like region */
    for (offset = 0; offset <= gfx_size - 768; offset++) {
        /* Heuristic: all values 0-63, reasonable distribution */
        int valid = 1, sum = 0;
        for (i = 0; i < 768 && valid; i++) {
            if (gfx_data[offset + i] > 63) valid = 0;
            sum += gfx_data[offset + i];
        }
        if (valid && sum > 100 && sum < 40000) {
            pal = gfx_data + offset;
            for (i = 0; i < 256; i++) {
                int r = (pal[i*3+0] & 0x3F) << 2;
                int g = (pal[i*3+1] & 0x3F) << 2;
                int b = (pal[i*3+2] & 0x3F) << 2;
                palette_out[i] = 0xFF000000 | (r << 16) | (g << 8) | b;
            }
            printf("VGA palette found at offset %d\n", offset);
            return 256;
        }
    }

    return -1;
}

void fs_dm1_get_full_palette(uint32_t *out256) {
    if (!out256) return;
    if (g_palette_extracted) {
        memcpy(out256, g_full_vga_palette, 256 * sizeof(uint32_t));
    } else {
        /* Default DM1 palette (first 16 + grays) */
        int i;
        memset(out256, 0, 256 * sizeof(uint32_t));
        out256[0]  = 0xFF000000;
        out256[1]  = 0xFF000088;
        out256[2]  = 0xFF008800;
        out256[3]  = 0xFF008888;
        out256[4]  = 0xFF880000;
        out256[5]  = 0xFF880088;
        out256[6]  = 0xFF885500;
        out256[7]  = 0xFFAAAAAA;
        out256[8]  = 0xFF555555;
        out256[9]  = 0xFF0000FF;
        out256[10] = 0xFF00FF00;
        out256[11] = 0xFF00FFFF;
        out256[12] = 0xFFFF0000;
        out256[13] = 0xFFFF00FF;
        out256[14] = 0xFFFFFF00;
        out256[15] = 0xFFFFFFFF;
        for (i = 16; i < 256; i++)
            out256[i] = 0xFF000000 | ((i & 0xFF) << 16) | ((i & 0xFF) << 8) | (i & 0xFF);
    }
}

int fs_dm1_load_palette_from_file(const char *gfx_path) {
    FILE *f = fopen(gfx_path, "rb");
    uint8_t *data;
    long size;
    int r;
    if (!f) return -1;
    fseek(f, 0, SEEK_END); size = ftell(f); fseek(f, 0, SEEK_SET);
    data = (uint8_t *)malloc(size);
    if (!data) { fclose(f); return -1; }
    fread(data, 1, size, f); fclose(f);
    r = fs_extract_vga_palette(data, (int)size, g_full_vga_palette);
    free(data);
    if (r > 0) g_palette_extracted = 1;
    return r;
}


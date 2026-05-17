
#include "firestaff_graphics_dat_reader.h"
#include <string.h>
#include <stdio.h>

/* GRAPHICS.DAT format (PC-34):
 * Offset 0: uint16 graphic_count
 * Offset 2: uint32[graphic_count] offset table
 * Per graphic at offset: uint16 width, uint16 height, packed pixels
 *
 * Source: ReDMCSB MEMORY.C F0467_MEMORY_GetGraphicOffset
 *         ReDMCSB MEMORY.C F0479_MEMORY_ReadGraphicsDatHeader */

static uint16_t r16(const uint8_t *p) { return (uint16_t)p[0] | ((uint16_t)p[1] << 8); }
static uint32_t r32(const uint8_t *p) { return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24); }

int fs_gfx_load(FS_GraphicsDat *gfx, const uint8_t *data, int size) {
    int i, off;
    uint16_t sig, count;
    if (!gfx || !data || size < 6) return -1;
    memset(gfx, 0, sizeof(*gfx));
    gfx->raw_data = data;
    gfx->raw_size = size;

    /* DM1 PC-34 GRAPHICS.DAT confirmed format:
     * New format (sig & 0x8000): sig(2) + count(2) + comp[count](2each) +
     *   decomp[count](2each) + wh[count](4each)
     * Old format: count(2) + comp[count](2each) then seek per graphic for w/h */
    sig = r16(data);
    if (sig & 0x8000) {
        /* New format */
        count = r16(data + 2);
        if (count > FS_GFX_MAX_GRAPHICS) count = FS_GFX_MAX_GRAPHICS;
        gfx->graphic_count = count;

        /* Read compressed sizes */
        off = 4;
        for (i = 0; i < count && off + 2 <= size; i++) {
            gfx->entries[i].compressed_size = r16(data + off);
            off += 2;
        }
        /* Skip decompressed sizes (same offset stride) */
        off += count * 2;
        /* Read width/height pairs */
        for (i = 0; i < count && off + 4 <= size; i++) {
            gfx->entries[i].width = r16(data + off);
            gfx->entries[i].height = r16(data + off + 2);
            off += 4;
        }
        /* Compute data offsets from compressed sizes */
        {
            int data_start = off;
            int cur = data_start;
            for (i = 0; i < count; i++) {
                gfx->entries[i].offset = cur;
                cur += gfx->entries[i].compressed_size;
            }
        }
    } else {
        /* Old format */
        count = sig;
        if (count > FS_GFX_MAX_GRAPHICS) count = FS_GFX_MAX_GRAPHICS;
        gfx->graphic_count = count;
        off = 2;
        for (i = 0; i < count && off + 2 <= size; i++) {
            gfx->entries[i].compressed_size = r16(data + off);
            off += 2;
        }
        /* Old format: w/h embedded at start of each graphic data */
        {
            int cur = off;
            for (i = 0; i < count; i++) {
                gfx->entries[i].offset = cur;
                if (cur + 4 <= size) {
                    gfx->entries[i].width = r16(data + cur);
                    gfx->entries[i].height = r16(data + cur + 2);
                }
                cur += gfx->entries[i].compressed_size;
            }
        }
    }

    gfx->loaded = 1;
    printf("GRAPHICS.DAT: %d graphics, format %s\n",
        count, (sig & 0x8000) ? "new" : "old");
    return count;
}


int fs_gfx_get_bitmap(const FS_GraphicsDat *gfx, int index,
    uint8_t *out_pixels, int max_size, int *out_w, int *out_h)
{
    /* BUG-010 fix: DM1 PC-34 GRAPHICS.DAT entries are LZW-compressed.
     * Delegate to fs_gfx_extract_bitmap which properly decompresses
     * via LZW then expands 4bpp to 8bpp indexed pixels.
     * Matches ReDMCSB MEMORY.C F0474 -> LZW decompress -> F0466 expand. */
    return fs_gfx_extract_bitmap(gfx, index, out_pixels, max_size, out_w, out_h);
}


int fs_gfx_get_palette(const FS_GraphicsDat *gfx, uint32_t *rgba_out) {
    if (!rgba_out) return -1;


    /* PC-34 VGA palette from ReDMCSB I34E DATA.C G0019_aui_Graphic560_Palette.
     * VGA DAC uses 6-bit values (0-63), scaled to 8-bit: val * 255 / 63. */
    static const uint32_t dm1_palette[16] = {
        0xFF000000,  /* 0: Black          (0,0,0)     */
        0xFF000044,  /* 1: Dark blue      (0,0,17)    */
        0xFF002200,  /* 2: Dark green     (0,9,0)     */
        0xFF662200,  /* 3: Brown          (25,9,0)    */
        0xFF440000,  /* 4: Dark red       (17,0,0)    */
        0xFF004400,  /* 5: Medium green   (0,17,0)    */
        0xFF226622,  /* 6: Olive green    (9,25,9)    */
        0xFF666666,  /* 7: Gray           (25,25,25)  */
        0xFF444444,  /* 8: Dark gray      (17,17,17)  */
        0xFF0044AA,  /* 9: Blue           (0,17,42)   */
        0xFF22AA22,  /* 10: Green         (9,42,9)    */
        0xFFAAAA44,  /* 11: Yellow-green  (42,42,17)  */
        0xFFAA4422,  /* 12: Orange-red    (42,17,9)   */
        0xFF44AAAA,  /* 13: Cyan          (17,42,42)  */
        0xFFAAAAAA,  /* 14: Light gray    (42,42,42)  */
        0xFFFFFFFF,  /* 15: White         (63,63,63)  */
    };

    memcpy(rgba_out, dm1_palette, 16 * sizeof(uint32_t));

    /* DM1 uses 4bpp (16 colors). Fill remaining 240 entries as dark variants
     * for V2 enhanced rendering effects */
    for (int i = 16; i < 256; i++) {
        /* Generate smooth ramp for extended palette */
        int base = i % 16;
        int bright = (i / 16) * 4;
        uint32_t c = dm1_palette[base];
        int r = ((c >> 16) & 0xFF) + bright;
        int g = ((c >> 8) & 0xFF) + bright;
        int b = (c & 0xFF) + bright;
        if (r > 255) r = 255;
        if (g > 255) g = 255;
        if (b > 255) b = 255;
        rgba_out[i] = 0xFF000000 | (r << 16) | (g << 8) | b;
    }

    /* If GRAPHICS.DAT is available, try to extract the actual palette
     * from the graphic data (graphic 562 area contains palette info) */
    if (gfx && gfx->loaded && gfx->graphic_count > 562) {
        /* Graphic 562 (15x13, 55 bytes) — contains palette modification data.
         * For now, the hardcoded palette above is the correct DM1 dungeon palette. */
    }

    return 16;
}



/* ══════════════════════════════════════════════════════════════════════
 * DM1 GRAPHICS.DAT bitmap decompression
 *
 * Source: ReDMCSB MEMORY.C F0474_MEMORY_LoadGraphic_CPSDF
 *
 * Format: each graphic is stored as either:
 *   1. Raw (uncompressed): width * height bytes, 4bpp packed to 8bpp
 *   2. RLE compressed: run-length encoded with escape byte
 *
 * The 4bpp format packs two pixels per byte (high nibble = left pixel).
 * PC-34 GRAPHICS.DAT uses a simple compression where:
 *   - Byte 0x00-0x7F: literal run of N+1 bytes
 *   - Byte 0x80-0xFF: repeat next byte (N-0x80+1) times
 * ══════════════════════════════════════════════════════════════════════ */

/* LZW decompressor is in dm1_v1_graphics_loader_pc34_compat.c */
#include "dm1_v1_graphics_loader_pc34_compat.h"

int fs_gfx_decompress_lzw(const uint8_t *src, int src_size,
    uint8_t *dst, int dst_size)
{
    M11_GFX_LZWState lzw;
    memset(&lzw, 0, sizeof(lzw));
    return m11_gfx_lzw_decompress(&lzw, src, (size_t)src_size,
        dst, (size_t)dst_size);
}

/* Expand 4bpp packed to 8bpp indexed (2 pixels per source byte) */
int fs_gfx_expand_4bpp(const uint8_t *packed, int packed_size,
    uint8_t *indexed, int max_pixels)
{
    int pi = 0, ii = 0;
    while (pi < packed_size && ii + 1 < max_pixels) {
        indexed[ii++] = (packed[pi] >> 4) & 0x0F;
        indexed[ii++] = packed[pi] & 0x0F;
        pi++;
    }
    return ii;
}

/* Full extraction: header + decompress + expand */
int fs_gfx_extract_bitmap(const FS_GraphicsDat *gfx, int index,
    uint8_t *out_indexed, int max_pixels,
    int *out_w, int *out_h)
{
    int w, h, off, data_off, compressed_size;
    uint8_t decomp_buf[65536];
    int decomp_size;

    if (!gfx || !gfx->loaded || index < 0 || index >= gfx->graphic_count)
        return -1;

    w = gfx->entries[index].width;
    h = gfx->entries[index].height;
    off = gfx->entries[index].offset;
    data_off = off; /* new format has w/h in header, old format embeds them */

    if (out_w) *out_w = w;
    if (out_h) *out_h = h;

    if (w <= 0 || h <= 0) return -1;

    /* Compressed data = comp_size from header */
    compressed_size = gfx->entries[index].compressed_size;
    if (compressed_size <= 0) {
        /* Try distance to next graphic */
        if (index + 1 < gfx->graphic_count)
            compressed_size = gfx->entries[index + 1].offset - data_off;
        else
            compressed_size = gfx->raw_size - data_off;
    }

    if (compressed_size <= 0 || data_off + compressed_size > gfx->raw_size)
        return -1;

    /* LZW decompression: output is 4bpp packed = (w*h+1)/2 bytes */
    {
        int expected_decomp = (w * h + 1) / 2;
        decomp_size = fs_gfx_decompress_lzw(
            gfx->raw_data + data_off, compressed_size,
            decomp_buf, expected_decomp < (int)sizeof(decomp_buf) ? expected_decomp : (int)sizeof(decomp_buf));
    }

    if (decomp_size <= 0) return -1;

    /* Check if data is 4bpp packed */
    int expected_4bpp = (w * h + 1) / 2;
    int expected_8bpp = w * h;

    if (!out_indexed) return expected_8bpp;

    if (decomp_size >= expected_8bpp) {
        /* Already 8bpp or raw */
        int copy = expected_8bpp < max_pixels ? expected_8bpp : max_pixels;
        memcpy(out_indexed, decomp_buf, copy);
        return copy;
    } else if (decomp_size >= expected_4bpp) {
        /* 4bpp packed — expand */
        return fs_gfx_expand_4bpp(decomp_buf, decomp_size,
            out_indexed, max_pixels);
    }

    /* Fallback: copy what we have */
    int copy = decomp_size < max_pixels ? decomp_size : max_pixels;
    memcpy(out_indexed, decomp_buf, copy);
    return copy;
}

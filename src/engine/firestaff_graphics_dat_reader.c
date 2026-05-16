
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
    int w, h, off, comp_size, byte_width, i;
    if (!gfx || !gfx->loaded || index < 0 || index >= gfx->graphic_count)
        return -1;

    w = gfx->entries[index].width;
    h = gfx->entries[index].height;
    off = gfx->entries[index].offset;
    comp_size = gfx->entries[index].compressed_size;

    if (out_w) *out_w = w;
    if (out_h) *out_h = h;
    if (w <= 0 || h <= 0 || comp_size <= 0) return -1;
    if (!out_pixels) return w * h;
    if (w * h > max_size) return -1;

    /* PC-34: data is 4bpp (2 pixels per byte), uncompressed.
     * byte_width = (w + 1) / 2; total = byte_width * h */
    byte_width = (w + 1) / 2;
    if (off + byte_width * h > gfx->raw_size) {
        /* Compressed data shorter than expected — use what we have */
        byte_width = comp_size / h;
        if (byte_width <= 0) return -1;
    }

    /* Expand 4bpp to 8bpp indexed */
    memset(out_pixels, 0, w * h);
    for (i = 0; i < byte_width * h && off + i < gfx->raw_size; i++) {
        int pi = i * 2;
        uint8_t byte = gfx->raw_data[off + i];
        if (pi < w * h) out_pixels[pi] = (byte >> 4) & 0x0F;
        if (pi + 1 < w * h) out_pixels[pi + 1] = byte & 0x0F;
    }

    return w * h;
}


int fs_gfx_get_palette(const FS_GraphicsDat *gfx, uint32_t *rgba_out) {
    /* DM1 VGA palette is at a fixed offset or embedded.
     * For now, use the standard DM1 palette. */
    (void)gfx;
    if (!rgba_out) return -1;
    /* Standard VGA 16-color + grayscale ramp */
    uint32_t pal16[] = {
        0xFF000000, 0xFF0000AA, 0xFF00AA00, 0xFF00AAAA,
        0xFFAA0000, 0xFFAA00AA, 0xFFAA5500, 0xFFAAAAAA,
        0xFF555555, 0xFF5555FF, 0xFF55FF55, 0xFF55FFFF,
        0xFFFF5555, 0xFFFF55FF, 0xFFFFFF55, 0xFFFFFFFF,
    };
    memcpy(rgba_out, pal16, 16 * 4);
    for (int i = 16; i < 256; i++) {
        uint8_t v = (uint8_t)(i);
        rgba_out[i] = 0xFF000000 | ((uint32_t)v<<16) | ((uint32_t)v<<8) | v;
    }
    return 256;
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

int fs_gfx_decompress_rle(const uint8_t *src, int src_size,
    uint8_t *dst, int dst_size)
{
    int si = 0, di = 0;
    while (si < src_size && di < dst_size) {
        uint8_t cmd = src[si++];
        if (cmd < 0x80) {
            /* Literal run: copy cmd+1 bytes */
            int count = cmd + 1;
            for (int i = 0; i < count && si < src_size && di < dst_size; i++)
                dst[di++] = src[si++];
        } else {
            /* Repeat run: repeat next byte (cmd-0x80+1) times */
            int count = (cmd & 0x7F) + 1;
            if (si >= src_size) break;
            uint8_t val = src[si++];
            for (int i = 0; i < count && di < dst_size; i++)
                dst[di++] = val;
        }
    }
    return di;
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
    data_off = off + 4; /* skip width/height */

    if (out_w) *out_w = w;
    if (out_h) *out_h = h;

    if (w <= 0 || h <= 0) return -1;

    /* Estimate compressed data size (until next graphic or EOF) */
    if (index + 1 < gfx->graphic_count)
        compressed_size = gfx->entries[index + 1].offset - data_off;
    else
        compressed_size = gfx->raw_size - data_off;

    if (compressed_size <= 0 || data_off + compressed_size > gfx->raw_size)
        return -1;

    /* Try RLE decompression */
    decomp_size = fs_gfx_decompress_rle(
        gfx->raw_data + data_off, compressed_size,
        decomp_buf, sizeof(decomp_buf));

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

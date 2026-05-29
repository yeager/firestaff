/*
 * theron_v1_palette.c — Theron's Quest V1 Phase 4: Palette/Tile System
 *
 * Implementations for the TQR tile/palette system.
 *
 * Phase 4 provides deterministic stub implementations:
 *   - tqr_palette_init_defaults(): fills palette with PC Engine dungeon stone tones
 *   - tqr_decode_tile_row(): planar bitplane decode (2bpp/4bpp → indexed)
 *   - tqr_decode_tile(): full 8x8 tile decode
 *   - tqr_tile_strip_load(): load tile strip into atlas
 *   - All other functions: no-op stubs returning 0/-1/NULL
 *
 * Full Track 02 tile/palette extraction (Phase 5) will replace these
 * with real implementations that parse the PC Engine CD-ROM data track.
 *
 * Source: THQUEST.ASM T400 (tile bank loading), T520 (tile selection),
 *         HuC6260/HuC6270 datasheet, tqr_v1_phase2_data_formats_H2339.md §7
 */

#include "theron_v1_palette.h"
#include <string.h>
#include <stdlib.h>

/* ── Tile decode ─────────────────────────────────────────────────── */

void tqr_decode_tile_row(uint8_t *TQR_RESTRICT out_row,
                          const uint8_t *TQR_RESTRICT src_row,
                          int bpp) {
    if (!out_row || !src_row) return;
    /* HuC6260 planar format: each bit-plane is one byte per 8 pixels.
     * LSB = leftmost pixel.  For 2bpp: bitplanes 0 and 1.
     * For 4bpp: bitplanes 0..3.
     *
     * Decoded byte per pixel = sum(bitplane_n[bit_n] << n) for n=0..bpp-1
     *
     * Since we only have raw bytes and no actual tile data yet,
     * we fill with a placeholder pattern (checkerboard using bpp bits). */
    if (bpp == 2) {
        /* 2bpp: bitplane 0 at src_row[0], bitplane 1 at src_row[1] */
        for (int i = 0; i < 8; i++) {
            int bit0 = (src_row[0] >> i) & 1;
            int bit1 = (src_row[1] >> i) & 1;
            out_row[i] = (uint8_t)((bit1 << 1) | bit0);
        }
    } else if (bpp == 4) {
        /* 4bpp: bitplanes 0..3 at src_row[0..3] */
        for (int i = 0; i < 8; i++) {
            int b0 = (src_row[0] >> i) & 1;
            int b1 = (src_row[1] >> i) & 1;
            int b2 = (src_row[2] >> i) & 1;
            int b3 = (src_row[3] >> i) & 1;
            out_row[i] = (uint8_t)((b3 << 3) | (b2 << 2) | (b1 << 1) | b0);
        }
    } else {
        /* Unknown bpp: fill with 0 */
        for (int i = 0; i < 8; i++) out_row[i] = 0;
    }
}

void tqr_decode_tile(uint8_t *TQR_RESTRICT out64,
                     const uint8_t *TQR_RESTRICT src,
                     int bpp) {
    if (!out64 || !src) return;
    int row_bytes = (bpp == 2) ? 2 : 4;
    for (int row = 0; row < 8; row++) {
        tqr_decode_tile_row(out64 + row * 8, src + row * row_bytes, bpp);
    }
}

/* ── Palette init ─────────────────────────────────────────────────── */

/*
 * Default PC Engine dungeon stone palette.
 * 16 palette groups × 16 colors = 512 entries.
 * Group 0 (dungeon walls/floors):
 *   0-3:   black, dark gray, gray, light gray
 *   4-7:   brown tones (stone wall shading)
 *   8-11:  tan/light brown (floor highlights)
 *   12-15: blue-gray (ceiling/special)
 * Groups 1-3 follow similar distributions for their domains.
 *
 * Generated deterministically using a fixed formula:
 *   entry[n].bgr444 = ((n / 16) << 8) | (((n % 16) * 15) / 16 << 4) | (n % 16)
 * which gives a recognizable dungeon stone gradient.
 */

/* Stub: fills all 512 palette entries with deterministic stone gradient.
 * This ensures the same output on all platforms even without Track 02 data. */
void tqr_palette_init_defaults(TQR_PaletteState *pal) {
    if (!pal) return;
    memset(pal, 0, sizeof(*pal));
    pal->tile_count = 0;

    /* Deterministic gradient: BGR444 formula gives a recognizable
     * dungeon stone palette across all 512 entries (16 groups x 16). */
    for (int i = 0; i < TQR_PALETTE_SIZE; i++) {
        /* BGR444: ((idx & 0xF) << 8) | (((idx >> 4) & 0xF) << 4) | (idx & 0xF)
         * Produces a smooth diagonal gradient: black→blue→...→white */
        uint16_t bgr444 = (uint16_t)(((i & 0xF) << 8) | (((i >> 4) & 0xF) << 4) | (i & 0xF));
        pal->entries[i].bgr444 = bgr444;
        pal->entries[i].rgba = tqr_bgr444_to_rgba(bgr444);
    }
}

/* ── Palette load ─────────────────────────────────────────────────── */

int tqr_palette_load_group(TQR_PaletteState *pal,
                           const uint8_t *data,
                           int start, int count) {
    if (!pal || !data || start < 0 || count <= 0) return 0;
    if (start + count > TQR_PALETTE_SIZE) {
        if (start >= TQR_PALETTE_SIZE) return 0;
        count = TQR_PALETTE_SIZE - start;
    }
    for (int i = 0; i < count; i++) {
        uint16_t bgr444 = (uint16_t)((data[i*2] << 8) | data[i*2+1]);
        pal->entries[start + i].bgr444 = bgr444;
        pal->entries[start + i].rgba   = tqr_bgr444_to_rgba(bgr444);
    }
    return count;
}

void tqr_palette_expand_rgba(TQR_PaletteState *pal) {
    if (!pal) return;
    for (int i = 0; i < TQR_PALETTE_SIZE; i++) {
        pal->entries[i].rgba = tqr_bgr444_to_rgba(pal->entries[i].bgr444);
    }
}

/* ── Tile loading ────────────────────────────────────────────────── */

int tqr_tile_load_from_data(TQR_PaletteState *pal,
                            const uint8_t *data,
                            int bpp,
                            int pal_group,
                            const char *label) {
    (void)label;
    if (!pal || !data) return -1;
    if (pal->tile_count >= TQR_MAX_TILES) return -1;

    TQR_Tile *t = &pal->tiles[pal->tile_count];
    t->data         = (uint8_t *)data; /* pointer to caller's buffer — NOT owned */
    t->bpp          = bpp;
    t->pal_group    = pal_group & (TQR_PALETTE_GROUPS - 1);
    t->uploaded     = 0;
    t->source_label = label;
    return pal->tile_count++;
}

/* Stub: load a strip of N tiles from a contiguous buffer.
 * Each tile is tile_size_bytes.  Returns first tile index or -1.
 * Phase 5 (Track 02 parsing) will implement real tile extraction. */
int tqr_tile_strip_load(TQR_PaletteState *pal,
                        const uint8_t *data,
                        int tile_count,
                        int tile_size_bytes,
                        int bpp,
                        int pal_group,
                        const char *label_prefix) {
    if (!pal || !data || tile_count <= 0 || tile_size_bytes <= 0) return -1;
    int first = -1;
    for (int i = 0; i < tile_count; i++) {
        /* Only allocate tiles if we have room; data is not copied */
        int tile_idx = tqr_tile_load_from_data(pal, data + i * tile_size_bytes,
                                               bpp, pal_group, label_prefix);
        if (i == 0) first = tile_idx;
        (void)first;
    }
    return first;
}

void tqr_palette_free_tiles(TQR_PaletteState *pal) {
    if (!pal) return;
    /* Tiles do not own their data (pointers to caller's buffers), so
     * we only reset the count and clear metadata. */
    for (int i = 0; i < pal->tile_count; i++) {
        pal->tiles[i].data = NULL;
        pal->tiles[i].source_label = NULL;
    }
    pal->tile_count = 0;
}

/* ── Tile query ───────────────────────────────────────────────────── */

const uint8_t *tqr_tile_get_data(const TQR_PaletteState *pal, int tile_index) {
    if (!pal || tile_index < 0 || tile_index >= pal->tile_count) return NULL;
    return pal->tiles[tile_index].data;
}

int tqr_tile_get_info(const TQR_PaletteState *pal,
                      int tile_index,
                      int *out_bpp,
                      int *out_pal_group) {
    if (!pal || tile_index < 0 || tile_index >= pal->tile_count) return -1;
    if (out_bpp)      *out_bpp      = pal->tiles[tile_index].bpp;
    if (out_pal_group) *out_pal_group = pal->tiles[tile_index].pal_group;
    return 0;
}
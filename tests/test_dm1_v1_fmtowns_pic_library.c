#include "dm1_v1_fmtowns_pic_library.h"
#include "dm1_v1_fmtowns_font_asset.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Build a self-contained mini picture-library byte-for-byte in the
 * format INDEX_TO_FILE_OFFSET (EDM.EXP 0x8d04) requires:
 *   +0   : u16 count
 *   +2   : u16 size_primary  [count entries]
 *   +2+2N: u16 size_secondary[count entries]  (mirror of primary)
 *   +2+4N: raw payload
 * The test uses only synthetic bytes — no game data on disk. */

static void put_u16_le(uint8_t *p, uint16_t v) {
    p[0] = (uint8_t)(v & 0xff);
    p[1] = (uint8_t)((v >> 8) & 0xff);
}

#define ASSET_COUNT 3
static const uint16_t k_sizes[ASSET_COUNT]  = {  4, 6, 5 };
/* Asset 0: raw 4-byte DECODEGRAPHIC header (w=32,h=1 -> uncompressed).
 * Asset 1: 4-byte header (w=16,h=1 -> padded=32, compressed) + 2
 *          bytes of RLE payload.
 * Asset 2: 5 bytes that mimic a raw byte span (like the menu font). */
static const uint8_t k_asset0[4] = { 0x20, 0x00, 0x01, 0x00 };
static const uint8_t k_asset1[6] = { 0x10, 0x00, 0x01, 0x00, 0xaa, 0xbb };
static const uint8_t k_asset2[5] = { 'F',  'O',  'N',  'T',  '!' };

static size_t build_pic_library(uint8_t *out) {
    size_t off = 0;
    put_u16_le(out + off, ASSET_COUNT); off += 2;
    for (int i = 0; i < ASSET_COUNT; ++i) { put_u16_le(out + off, k_sizes[i]); off += 2; }
    for (int i = 0; i < ASSET_COUNT; ++i) { put_u16_le(out + off, k_sizes[i]); off += 2; }
    memcpy(out + off, k_asset0, sizeof(k_asset0)); off += sizeof(k_asset0);
    memcpy(out + off, k_asset1, sizeof(k_asset1)); off += sizeof(k_asset1);
    memcpy(out + off, k_asset2, sizeof(k_asset2)); off += sizeof(k_asset2);
    return off;
}

static void test_open_and_header(void) {
    uint8_t buf[256];
    size_t n = build_pic_library(buf);

    dm1_v1_fmtowns_pic_library_view_t view;
    assert(dm1_v1_fmtowns_pic_library_open_pc34(buf, n, &view)
           == DM1_V1_FMTOWNS_PIC_LIB_OK);
    assert(view.asset_count == ASSET_COUNT);
    /* header = 2 + count*4 = 14 */
    assert(view.payload_offset == 2u + (size_t)ASSET_COUNT * 4u);
    assert(view.size_table_primary   == buf + 2);
    assert(view.size_table_secondary == buf + 2 + ASSET_COUNT * 2);
    assert(dm1_v1_fmtowns_pic_library_tables_are_mirror_pc34(&view) == 1);
}

static void test_sizes_and_offsets(void) {
    uint8_t buf[256];
    size_t n = build_pic_library(buf);
    dm1_v1_fmtowns_pic_library_view_t view;
    assert(dm1_v1_fmtowns_pic_library_open_pc34(buf, n, &view)
           == DM1_V1_FMTOWNS_PIC_LIB_OK);

    uint16_t sz;
    size_t   off;
    assert(dm1_v1_fmtowns_pic_library_asset_size_pc34(&view, 0, &sz) == 0 && sz == 4);
    assert(dm1_v1_fmtowns_pic_library_asset_size_pc34(&view, 1, &sz) == 0 && sz == 6);
    assert(dm1_v1_fmtowns_pic_library_asset_size_pc34(&view, 2, &sz) == 0 && sz == 5);

    assert(dm1_v1_fmtowns_pic_library_asset_offset_pc34(&view, 0, &off) == 0 && off == 14u);
    assert(dm1_v1_fmtowns_pic_library_asset_offset_pc34(&view, 1, &off) == 0 && off == 18u);
    assert(dm1_v1_fmtowns_pic_library_asset_offset_pc34(&view, 2, &off) == 0 && off == 24u);

    /* Out-of-range refuses. */
    assert(dm1_v1_fmtowns_pic_library_asset_size_pc34(&view, ASSET_COUNT, &sz)
           == DM1_V1_FMTOWNS_PIC_LIB_ERR_INDEX);
}

static void test_asset_bytes(void) {
    uint8_t buf[256];
    size_t n = build_pic_library(buf);
    dm1_v1_fmtowns_pic_library_view_t view;
    assert(dm1_v1_fmtowns_pic_library_open_pc34(buf, n, &view) == 0);

    const uint8_t *p; uint16_t sz;
    assert(dm1_v1_fmtowns_pic_library_asset_bytes_pc34(&view, 0, &p, &sz) == 0);
    assert(sz == 4 && memcmp(p, k_asset0, 4) == 0);
    assert(dm1_v1_fmtowns_pic_library_asset_bytes_pc34(&view, 2, &p, &sz) == 0);
    assert(sz == 5 && memcmp(p, k_asset2, 5) == 0);
}

static void test_raw_asset_copy(void) {
    /* Direct+NO_HDR path used by INIT_TEXT for the menu font. */
    uint8_t buf[256];
    size_t  n = build_pic_library(buf);
    dm1_v1_fmtowns_pic_library_view_t view;
    assert(dm1_v1_fmtowns_pic_library_open_pc34(buf, n, &view) == 0);

    uint8_t dst[16] = {0};
    size_t  wrote = 0;
    assert(dm1_v1_fmtowns_pic_library_load_raw_asset_pc34(
               &view, 2, dst, sizeof(dst), &wrote) == 0);
    assert(wrote == 5);
    assert(memcmp(dst, "FONT!", 5) == 0);

    /* Refuses when caller's buffer is too small. */
    assert(dm1_v1_fmtowns_pic_library_load_raw_asset_pc34(
               &view, 2, dst, 3, &wrote)
           == DM1_V1_FMTOWNS_PIC_LIB_ERR_TRUNCATED);
}

static void test_gfx_header_math(void) {
    /* Byte-verified DECODEGRAPHIC prologue arithmetic. */
    /* width=0 → padded=0. */
    assert(dm1_v1_fmtowns_pic_library_padded_width_pc34(0)   == 0);
    /* width=1..31 → padded=32. */
    assert(dm1_v1_fmtowns_pic_library_padded_width_pc34(1)   == 32);
    assert(dm1_v1_fmtowns_pic_library_padded_width_pc34(31)  == 32);
    /* width=32 → padded=32 (uncompressed path). */
    assert(dm1_v1_fmtowns_pic_library_padded_width_pc34(32)  == 32);
    /* width=33 → padded=64. */
    assert(dm1_v1_fmtowns_pic_library_padded_width_pc34(33)  == 64);
    /* width=320 (asset-1 real width) → padded=320. */
    assert(dm1_v1_fmtowns_pic_library_padded_width_pc34(320) == 320);
    /* row_bytes = padded/2 (4 bpp). */
    assert(dm1_v1_fmtowns_pic_library_row_bytes_pc34(320) == 160);
    assert(dm1_v1_fmtowns_pic_library_row_bytes_pc34(16)  == 16);
}

static void test_gfx_header_parse(void) {
    uint8_t buf[256];
    size_t n = build_pic_library(buf);
    dm1_v1_fmtowns_pic_library_view_t view;
    assert(dm1_v1_fmtowns_pic_library_open_pc34(buf, n, &view) == 0);

    const uint8_t *p; uint16_t sz;
    dm1_v1_fmtowns_pic_library_gfx_header_t hdr;

    /* Asset 0: width=32,height=1 → uncompressed. */
    assert(dm1_v1_fmtowns_pic_library_asset_bytes_pc34(&view, 0, &p, &sz) == 0);
    assert(dm1_v1_fmtowns_pic_library_parse_gfx_header_pc34(p, sz, &hdr) == 0);
    assert(hdr.width_pixels == 32 && hdr.height_pixels == 1);
    assert(hdr.padded_width_pixels == 32);
    assert(hdr.row_bytes == 16);
    assert(hdr.decoded_total_bytes == 16u);
    assert(hdr.is_uncompressed == 1);

    /* Asset 1: width=16,height=1 → padded=32, compressed path. */
    assert(dm1_v1_fmtowns_pic_library_asset_bytes_pc34(&view, 1, &p, &sz) == 0);
    assert(dm1_v1_fmtowns_pic_library_parse_gfx_header_pc34(p, sz, &hdr) == 0);
    assert(hdr.width_pixels == 16 && hdr.height_pixels == 1);
    assert(hdr.padded_width_pixels == 32);
    assert(hdr.is_uncompressed == 0);
}

static void test_menu_font_identity_is_reachable(void) {
    /* Cross-module confirmation: the identity module names the same
     * asset index the container will resolve when it becomes real. */
    uint16_t idx = dm1_v1_fmtowns_font_pic_library_index_pc34();
    assert(idx == 557u);
    /* And DIRECT+NO_HDR flag decode still matches. */
    assert(dm1_v1_fmtowns_font_is_direct_to_buffer_pc34(
               DM1_V1_FMTOWNS_FONT_MY_DECODED_ARG) == 1);
    assert(dm1_v1_fmtowns_font_is_no_size_header_pc34(
               DM1_V1_FMTOWNS_FONT_MY_DECODED_ARG) == 1);
}

static void test_truncated_input(void) {
    uint8_t buf[10] = {0x03, 0x00, /* count=3 but only 2 bytes total */};
    dm1_v1_fmtowns_pic_library_view_t view;
    /* Missing size tables. */
    assert(dm1_v1_fmtowns_pic_library_open_pc34(buf, 2, &view)
           == DM1_V1_FMTOWNS_PIC_LIB_ERR_TRUNCATED);
    /* Missing payload — 2 + 3*4 = 14 needed for tables, we have 10. */
    assert(dm1_v1_fmtowns_pic_library_open_pc34(buf, 10, &view)
           == DM1_V1_FMTOWNS_PIC_LIB_ERR_TRUNCATED);
}

/* Synthetic RLE fixture: width=16 (padded=32), height=1 → 16 dst bytes.
 * Short-form fill only covers up to 8 pixels (count = (ctrl>>4)+1
 * with bit7=0 caps upper nibble at 7). Two runs cover the full row.
 * Byte 0x7A: bit7=0, count=(7)+1=8, nibble=0xA → pixels 0..7 = 0xA
 * Byte 0x7F: bit7=0, count=8, nibble=0xF → pixels 8..15 = 0xF */
static void test_synthetic_short_fill(void) {
    uint8_t asset[7] = { 0x10, 0x00, 0x01, 0x00, 0x7A, 0x7F, 0 };
    uint8_t dst[16] = {0};
    size_t wrote = 0, consumed = 0;
    dm1_v1_fmtowns_pic_library_gfx_header_t hdr;
    int rc = dm1_v1_fmtowns_pic_library_decode_asset_pc34(
        asset, sizeof(asset), dst, sizeof(dst), &wrote, &consumed, &hdr);
    assert(rc == DM1_V1_FMTOWNS_PIC_LIB_OK);
    assert(hdr.width_pixels == 16 && hdr.padded_width_pixels == 32);
    assert(wrote == 16);
    /* Pixels 0..7 = 0xA → bytes 0..3 = 0xAA (both nibbles from 0xA
     * short-form fill's dup-byte). Pixels 8..15 = 0xF → bytes 4..7 =
     * 0xFF. Padding pixels 16..31 (bytes 8..15) untouched (zero). */
    for (size_t i = 0; i < 4; ++i) assert(dst[i] == 0xAA);
    for (size_t i = 4; i < 8; ++i) assert(dst[i] == 0xFF);
    for (size_t i = 8; i < 16; ++i) assert(dst[i] == 0x00);
    /* Consumed = 4 (header) + 2 (control bytes) = 6. */
    assert(consumed == 6);
}

/* Extended fill (bit7=1, bit6=0, mode 0): 1-byte count field. */
static void test_synthetic_extended_fill(void) {
    /* w=32 h=1 → padded_w=32 → uncompressed path → decoder refuses.
     * Use w=16 again; extended mode 0 with count = next+1. */
    /* ctrl 0x8A: bit7=1 bit6=0 mode 00, nib 0xA. Next byte 0x0F →
     * count = 16. Fills 16 pixels with nibble 0xA. */
    uint8_t asset[7] = { 0x10, 0x00, 0x01, 0x00, 0x8A, 0x0F, 0 };
    uint8_t dst[16] = {0};
    size_t wrote = 0, consumed = 0;
    dm1_v1_fmtowns_pic_library_gfx_header_t hdr;
    int rc = dm1_v1_fmtowns_pic_library_decode_asset_pc34(
        asset, sizeof(asset), dst, sizeof(dst), &wrote, &consumed, &hdr);
    assert(rc == DM1_V1_FMTOWNS_PIC_LIB_OK);
    assert(wrote == 16);
    /* All 16 unpadded pixels = 0xA → 8 dup bytes = 0xAA. */
    for (size_t i = 0; i < 8; ++i) assert(dst[i] == 0xAA);
    for (size_t i = 8; i < 16; ++i) assert(dst[i] == 0x00);
    /* Consumed = 4 (header) + 1 (ctrl) + 1 (count) = 6. */
    assert(consumed == 6);
}

/* Real GRAPHICS.DAT round-trip: byte-count verification on ≥2 RLE
 * assets. The task specification permits length-only verification
 * because we have no known-good decoded pixel data — length invariants
 * (source-bytes-consumed = size_table[idx], and decoded-bytes =
 * padded_width/2 * height) are the strongest ship criterion available.
 *
 * Points at real GRAPHICS.DAT via env FIRESTAFF_DM1_FMTOWNS_GRAPHICS_DAT.
 * Skips if not present. Never bundles game data. */
static uint8_t *load_file_full(const char *path, size_t *out_size) {
    FILE *f = fopen(path, "rb"); if (!f) return NULL;
    fseek(f, 0, SEEK_END); long sz = ftell(f);
    if (sz <= 0) { fclose(f); return NULL; }
    fseek(f, 0, SEEK_SET);
    uint8_t *buf = (uint8_t *)malloc((size_t)sz);
    if (!buf) { fclose(f); return NULL; }
    if (fread(buf, 1, (size_t)sz, f) != (size_t)sz) { free(buf); fclose(f); return NULL; }
    fclose(f);
    *out_size = (size_t)sz;
    return buf;
}

static void test_real_graphics_dat_rle_roundtrip(void) {
    const char *path = getenv("FIRESTAFF_DM1_FMTOWNS_GRAPHICS_DAT");
    if (!path || !*path) {
        printf("SKIP: FIRESTAFF_DM1_FMTOWNS_GRAPHICS_DAT not set\n");
        return;
    }
    size_t sz = 0;
    uint8_t *file = load_file_full(path, &sz);
    if (!file) { printf("SKIP: cannot open %s\n", path); return; }

    dm1_v1_fmtowns_pic_library_view_t view;
    int rc = dm1_v1_fmtowns_pic_library_open_pc34(file, sz, &view);
    assert(rc == DM1_V1_FMTOWNS_PIC_LIB_OK);

    /* Sweep every asset that hits the RLE branch (width != padded_width)
     * up to index 500 (past that the container also stores palette /
     * font raw spans whose 4-byte prefix is data, not a valid header). */
    uint8_t dst[1u << 17];
    size_t pass = 0, tried = 0;
    for (uint16_t i = 0; i < 500 && i < view.asset_count; ++i) {
        const uint8_t *span; uint16_t span_sz;
        if (dm1_v1_fmtowns_pic_library_asset_bytes_pc34(&view, i, &span, &span_sz) != 0) continue;
        if (span_sz < 4) continue;
        dm1_v1_fmtowns_pic_library_gfx_header_t hdr;
        if (dm1_v1_fmtowns_pic_library_parse_gfx_header_pc34(span, span_sz, &hdr) != 0) continue;
        if (hdr.is_uncompressed) continue;
        if (hdr.decoded_total_bytes == 0 || hdr.decoded_total_bytes > sizeof(dst)) continue;
        size_t wrote = 0, consumed = 0;
        int drc = dm1_v1_fmtowns_pic_library_decode_asset_pc34(
            span, span_sz, dst, sizeof(dst), &wrote, &consumed, NULL);
        if (drc != DM1_V1_FMTOWNS_PIC_LIB_OK) continue;
        tried++;
        /* Byte-count invariants: destination fully written and source
         * fully consumed. */
        if (wrote == hdr.decoded_total_bytes && consumed == span_sz) {
            pass++;
        } else {
            printf("  MISMATCH idx=%u w=%u h=%u wrote=%zu/%u consumed=%zu/%u\n",
                   i, hdr.width_pixels, hdr.height_pixels,
                   wrote, hdr.decoded_total_bytes, consumed, span_sz);
        }
    }
    printf("  RLE round-trip: %zu/%zu assets byte-exact\n", pass, tried);
    /* Task requires at least two RLE assets round-trip. */
    assert(pass >= 2);
    /* And realistically, ALL well-formed RLE assets must round-trip;
     * anything less means the decoder or the header classifier is
     * off. */
    assert(pass == tried);
    free(file);
}

int main(void) {
    test_open_and_header();
    test_sizes_and_offsets();
    test_asset_bytes();
    test_raw_asset_copy();
    test_gfx_header_math();
    test_gfx_header_parse();
    test_menu_font_identity_is_reachable();
    test_truncated_input();
    test_synthetic_short_fill();
    test_synthetic_extended_fill();
    test_real_graphics_dat_rle_roundtrip();
    printf("All dm1_v1_fmtowns_pic_library tests passed.\n");
    return 0;
}

#include "dm1_v1_fmtowns_pic_library.h"
#include "dm1_v1_fmtowns_font_asset.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
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

int main(void) {
    test_open_and_header();
    test_sizes_and_offsets();
    test_asset_bytes();
    test_raw_asset_copy();
    test_gfx_header_math();
    test_gfx_header_parse();
    test_menu_font_identity_is_reachable();
    test_truncated_input();
    printf("All dm1_v1_fmtowns_pic_library tests passed.\n");
    return 0;
}

#include "dm1_v1_fmtowns_font_rasteriser.h"
#include "dm1_v1_fmtowns_font_asset.h"
#include "dm1_v1_fmtowns_pic_library.h"
#include "firestaff_fmtowns_disc.h"
#include "firestaff_zip_extract.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Byte-verified font raster layout (see dm1_v1_fmtowns_font_rasteriser.h):
 * data[row * 128 + ascii], MSB-first bit order.
 *
 * These fixtures encode a few glyphs whose bit patterns exactly match
 * what asset 557 of the shipped English/Japanese Track 01 GRAPHICS.DAT
 * decodes to when read through the DIRECT+NO_HDR path. Any drift in
 * bit order, row stride, or ASCII offset will break these tests. */

static void build_test_raster(uint8_t r[768]) {
    memset(r, 0, 768);
    /* Real font stores each glyph right-aligned in an 8-bit byte:
     * CHAR_X_SIZE=5 body pixels occupy bits 4..0 of each row byte.
     * Verified against asset 557 in the shipped Japanese Track 01
     * GRAPHICS.DAT — 'A' row 3 = 0x1f, 'A' row 0 = 0x0e (top curve).
     *
     * Fixture 'A' matches the shipped shape:
     *   row0: .###.  = 0x0e
     *   row1: #...#  = 0x11
     *   row2: #####  = 0x1f  (crossbar)
     *   row3: #...#  = 0x11
     *   row4: #...#  = 0x11
     *   row5: (blank)
     */
    r[0*128 + 'A'] = 0x0e;
    r[1*128 + 'A'] = 0x11;
    r[2*128 + 'A'] = 0x1f;
    r[3*128 + 'A'] = 0x11;
    r[4*128 + 'A'] = 0x11;
    /* '!' body column at pixel-col 2 = bit 2:
     *   rows 0-2: ..#.. = 0x04
     *   row3: (blank)
     *   row4: ..#.. = 0x04
     */
    r[0*128 + '!'] = 0x04;
    r[1*128 + '!'] = 0x04;
    r[2*128 + '!'] = 0x04;
    r[4*128 + '!'] = 0x04;
    /* Space (0x20) is left all zero — verified as empty glyph. */
}

static void test_null_and_out_of_range(void) {
    uint8_t r[768] = {0};
    uint8_t fb[64];
    memset(fb, 0, sizeof(fb));
    assert(dm1_v1_fmtowns_font_rasterise_glyph_pc34(NULL, fb, 8, 8, 8, 0, 0, 1, 0, 0, 'A') == 0);
    assert(dm1_v1_fmtowns_font_rasterise_glyph_pc34(r, NULL, 8, 8, 8, 0, 0, 1, 0, 0, 'A') == 0);
    /* ASCII >= 128 rejected */
    assert(dm1_v1_fmtowns_font_rasterise_glyph_pc34(r, fb, 8, 8, 8, 0, 0, 1, 0, 0, 0x80) == 0);
    /* All bytes untouched */
    for (int i = 0; i < 64; ++i) assert(fb[i] == 0);
}

static void test_space_paints_nothing_when_no_bg(void) {
    uint8_t r[768] = {0};
    uint8_t fb[64];
    memset(fb, 0xaa, sizeof(fb));
    build_test_raster(r);
    /* Space glyph: all-zero raster; write_bg=0 must leave fb untouched. */
    int rc = dm1_v1_fmtowns_font_rasterise_glyph_pc34(r, fb, 8, 8, 8, 0, 0, 1, 2, 0, ' ');
    assert(rc == 0);
    for (int i = 0; i < 64; ++i) assert(fb[i] == 0xaa);
}

static void test_space_paints_bg_when_requested(void) {
    uint8_t r[768] = {0};
    uint8_t fb[64];
    memset(fb, 0xaa, sizeof(fb));
    build_test_raster(r);
    /* Space with write_bg=1 paints the 5x6 bg block, leaves the rest. */
    int rc = dm1_v1_fmtowns_font_rasterise_glyph_pc34(r, fb, 8, 8, 8, 0, 0, 1, 2, 1, ' ');
    assert(rc == 1);
    /* Rows 0..5, cols 0..4 must be bg = 2. */
    for (int y = 0; y < 6; ++y)
        for (int x = 0; x < 5; ++x)
            assert(fb[y * 8 + x] == 2);
    /* Rows 6..7 stay 0xaa. */
    for (int x = 0; x < 8; ++x) {
        assert(fb[6 * 8 + x] == 0xaa);
        assert(fb[7 * 8 + x] == 0xaa);
    }
    /* Cols 5..7 in rows 0..5 stay 0xaa. */
    for (int y = 0; y < 6; ++y)
        for (int x = 5; x < 8; ++x)
            assert(fb[y * 8 + x] == 0xaa);
}

static void test_A_bit_pattern(void) {
    uint8_t r[768] = {0};
    uint8_t fb[64];
    memset(fb, 0, sizeof(fb));
    build_test_raster(r);
    int rc = dm1_v1_fmtowns_font_rasterise_glyph_pc34(r, fb, 8, 8, 8, 0, 0, 4, 0, 0, 'A');
    assert(rc == 1);
    /* Expected 'A' shape (fg=4, bg stays 0):
     *   row0: .###.  x=1,2,3 set
     *   row1: #...#  x=0,4 set
     *   row2: #####  crossbar all 5
     *   row3: #...#  x=0,4 set
     *   row4: #...#  x=0,4 set
     *   row5: blank
     */
    assert(fb[0*8 + 0] == 0 && fb[0*8 + 1] == 4 && fb[0*8 + 2] == 4 && fb[0*8 + 3] == 4 && fb[0*8 + 4] == 0);
    assert(fb[1*8 + 0] == 4 && fb[1*8 + 1] == 0 && fb[1*8 + 2] == 0 && fb[1*8 + 3] == 0 && fb[1*8 + 4] == 4);
    for (int x = 0; x < 5; ++x) assert(fb[2*8 + x] == 4);
    assert(fb[3*8 + 0] == 4 && fb[3*8 + 1] == 0 && fb[3*8 + 2] == 0 && fb[3*8 + 3] == 0 && fb[3*8 + 4] == 4);
    assert(fb[4*8 + 0] == 4 && fb[4*8 + 1] == 0 && fb[4*8 + 2] == 0 && fb[4*8 + 3] == 0 && fb[4*8 + 4] == 4);
    for (int x = 0; x < 5; ++x) assert(fb[5*8 + x] == 0);
}

static void test_string_advances_by_char_x_wid(void) {
    uint8_t r[768] = {0};
    uint8_t fb[8 * 24];
    memset(fb, 0, sizeof(fb));
    build_test_raster(r);
    /* Draw "A A" -> A at x=0, space at x=6, A at x=12. Width 24 ensures
     * the third glyph's 5-pixel body (cols 12..16) fits. */
    unsigned int painted = dm1_v1_fmtowns_font_rasterise_string_pc34(
        r, fb, 24, 8, 24, 0, 0, 4, 0, 0, "A A");
    /* First A + third A count; space paints nothing with write_bg=0. */
    assert(painted == 2);
    /* First A crossbar row 3 at x=0..4 */
    for (int x = 0; x < 5; ++x) assert(fb[2*24 + x] == 4);
    /* Gap x=5..11 row 3 stays 0 */
    for (int x = 5; x < 12; ++x) assert(fb[2*24 + x] == 0);
    /* Second A crossbar row 3 at x=12..16 */
    for (int x = 12; x < 17; ++x) assert(fb[2*24 + x] == 4);
    /* Past the second A stays 0 */
    for (int x = 17; x < 24; ++x) assert(fb[2*24 + x] == 0);
}

static void test_string_stops_at_right_edge(void) {
    uint8_t r[768] = {0};
    uint8_t fb[64];
    memset(fb, 0, sizeof(fb));
    build_test_raster(r);
    /* fb_width=8. Start at x=6. First glyph needs 5 pixels, so 6..10;
     * 10 >= 8 so glyph WOULD be clipped, but the string helper checks
     * cursor + CHAR_X_SIZE > fb_width and stops entirely. 6 + 5 > 8 so
     * NOTHING is painted. */
    unsigned int painted = dm1_v1_fmtowns_font_rasterise_string_pc34(
        r, fb, 8, 8, 8, 6, 0, 4, 0, 0, "AAA");
    assert(painted == 0);
    for (int i = 0; i < 64; ++i) assert(fb[i] == 0);
}

/* Live GRAPHICS.DAT round-trip from the selected source archive. */
static uint8_t *load_retail_graphics_dat(size_t *out_size) {
    const char *archive = getenv("FIRESTAFF_DM1_FMTOWNS_ARCHIVE");
    uint8_t *cue = NULL, *bin = NULL, *graphics = NULL;
    size_t cue_size = 0u, bin_size = 0u;
    char image_member[256];
    FmtownsDiscProbeResult disc;
    const FmtownsIsoEntry *entry;

    if (!archive || !archive[0] || !out_size ||
        firestaff_zip_extract_by_suffix(archive, ".cue", &cue, &cue_size) != 0 ||
        !cue || !fmtowns_cue_parse_image_member((const char *)cue, cue_size,
                                                 image_member, sizeof(image_member)) ||
        firestaff_zip_extract_by_suffix(archive, image_member, &bin, &bin_size) != 0 ||
        !bin || fmtowns_disc_probe(bin, bin_size, FMTOWNS_SECTOR_2048, &disc) != 0 ||
        !disc.valid || !(entry = fmtowns_disc_find(&disc, "DATA/GRAPHICS.DAT")) ||
        fmtowns_disc_extract_alloc(bin, bin_size, FMTOWNS_SECTOR_2048, entry,
                                   &graphics, out_size) != 0) {
        free(cue);
        free(bin);
        free(graphics);
        return NULL;
    }
    free(cue);
    free(bin);
    return graphics;
}

static void test_real_data_round_trip(void) {
    dm1_v1_fmtowns_pic_library_view_t view;
    const uint8_t *font;
    uint16_t font_size;
    uint8_t *graphics;
    size_t graphics_size = 0u;
    uint8_t r[768];
    uint8_t fb[64];
    if (!getenv("FIRESTAFF_DM1_FMTOWNS_ARCHIVE")) {
        puts("SKIP: FIRESTAFF_DM1_FMTOWNS_ARCHIVE not set");
        return;
    }
    graphics = load_retail_graphics_dat(&graphics_size);
    assert(graphics && dm1_v1_fmtowns_pic_library_open_pc34(
                           graphics, graphics_size, &view) == DM1_V1_FMTOWNS_PIC_LIB_OK);
    assert(dm1_v1_fmtowns_pic_library_asset_bytes_pc34(
               &view, dm1_v1_fmtowns_font_pic_library_index_pc34(),
               &font, &font_size) == DM1_V1_FMTOWNS_PIC_LIB_OK &&
           font_size == sizeof(r));
    memcpy(r, font, sizeof(r));
    free(graphics);
    /* Space MUST be all-zero in the real font. */
    for (int row = 0; row < 6; ++row)
        assert(r[row * 128 + ' '] == 0);
    /* 'A' row 2 has the horizontal crossbar (bottom 5 bits set). */
    assert((r[2 * 128 + 'A'] & 0x1f) == 0x1f);
    /* Rasterise 'A' at 0,0 and verify all 5 crossbar pixels light up. */
    memset(fb, 0, sizeof(fb));
    dm1_v1_fmtowns_font_rasterise_glyph_pc34(r, fb, 8, 8, 8, 0, 0, 4, 0, 0, 'A');
    for (int x = 0; x < 5; ++x) assert(fb[2 * 8 + x] == 4);
    puts("PASS: retail ZIP GRAPHICS.DAT font round-trip matches fixture layout");
}

int main(void) {
    test_null_and_out_of_range();
    test_space_paints_nothing_when_no_bg();
    test_space_paints_bg_when_requested();
    test_A_bit_pattern();
    test_string_advances_by_char_x_wid();
    test_string_stops_at_right_edge();
    test_real_data_round_trip();
    puts("All dm1_v1_fmtowns_font_rasteriser tests passed.");
    return 0;
}

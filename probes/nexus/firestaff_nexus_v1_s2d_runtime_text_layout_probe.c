/*
 * firestaff_nexus_v1_s2d_runtime_text_layout_probe.c
 * ====================================================
 *
 * Probe for the Nexus V1 S2D runtime text-layout API declared in
 * include/nexus_v1_s2d_text_layout.h.
 *
 * Scope (deliberately bounded, data-free by default):
 *
 *   [1] nexus_v1_s2d_section_glyph_map() NULL-safety
 *   [2] nexus_v1_s2d_section_glyph_map() rejects bytes_per_glyph=0
 *   [3] Empty section table yields an empty map (range_count=0)
 *   [4] Single-populated section yields one range with
 *       char_start=0, char_count = section_size/bytes_per_glyph
 *   [5] Four-populated synthetic SCR walks to four ranges, each
 *       covering a contiguous slice of the character table.
 *   [6] Header char_count acts as a hard cap: a 4-section fixture
 *       with header char_count=100 caps the effective coverage at
 *       100 even when the section byte budget would cover more.
 *   [7] nexus_v1_s2d_glyph_range_lookup() returns -1 for
 *       out-of-coverage indices and the matching range index
 *       for in-coverage indices.
 *   [8] nexus_v1_s2d_glyph_map_total_chars() sums across ranges.
 *
 *   [9] nexus_v1_s2d_text_layout_init() NULL-safety + idempotency.
 *  [10] Reset cursor clears all counters.
 *  [11] draw_string on an empty string returns 0 and never paints.
 *  [12] draw_string on a 1-character string paints exactly one
 *       framebuffer cell of fg_index (and bg when configured).
 *  [13] draw_string advances the cursor by char_width +
 *       letter_spacing_x per glyph.
 *  [14] draw_string on a 5-character string paints 5 glyphs and
 *       advances the cursor to (5*(char_width+letter_spacing_x), 0).
 *  [15] '\n' increments the newline counter and resets cursor_x to 0
 *       while incrementing cursor_y by line_height.
 *  [16] A multi-line "screen" of text produces a deterministic
 *       FNV-1a framebuffer hash + a deterministic per-line receipt
 *       hash. The same input fed twice yields the same hash.
 *  [17] draw_string with max_chars cap stops drawing at the cap.
 *  [18] Out-of-coverage characters (e.g. a synthetic fixture whose
 *       header char_count is smaller than the input string length)
 *       increment chars_skipped and never paint.
 *  [19] Real FONT256.S2D optional path: when the operator has
 *       staged the verified 25,012-byte asset, the probe builds a
 *       section→glyph-range map from the real parser output and
 *       draws "NEXUS" through the real layout. This proves the
 *       chain real_parser → real_map → real_layout → real_fb
 *       agrees with the synthetic-only contract above. The branch
 *       is skip-safe: missing asset prints SKIP and returns 0.
 *  [20] The full S2D run on the real asset is deterministic: 5
 *       repetitions of the same draw script yield the same
 *       FNV-1a framebuffer hash and the same chars_drawn /
 *       chars_skipped / writes counters.
 *
 * Non-claim:
 *   This probe does NOT prove full Saturn SCR text-layout parity.
 *   It locks only the bounded ASCII-only layout contract plus an
 *   optional real-asset receipt. Capturing an actual Nexus screen
 *   using the real font is a separate gap-list row.
 *
 * Run:
 *   ./build/firestaff_nexus_v1_s2d_runtime_text_layout_probe
 *
 * CTest:
 *   ctest --test-dir build -R nexus_v1_s2d_runtime_text_layout --output-on-failure
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nexus_v1_s2d_text_layout.h"
#include "nexus_v1_saturn_font.h"

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do {                                  \
    if (cond) { fprintf(stderr, "  PASS: %s\n", msg); ++g_pass; }      \
    else      { fprintf(stderr, "  FAIL: %s\n", msg); ++g_fail; }      \
} while (0)

static uint64_t fnv1a64(const uint8_t *data, size_t size) {
    uint64_t h = UINT64_C(0xcbf29ce484222325);
    size_t i;
    for (i = 0; i < size; ++i) {
        h ^= data[i];
        h *= UINT64_C(0x100000001b3);
    }
    return h;
}

static uint8_t *read_entire_file(const char *path, long *out_size) {
    FILE *fp;
    long size;
    uint8_t *data;

    if (out_size) *out_size = 0;
    if (!path) return NULL;

    fp = fopen(path, "rb");
    if (!fp) return NULL;
    if (fseek(fp, 0, SEEK_END) != 0) { fclose(fp); return NULL; }
    size = ftell(fp);
    if (size <= 0) { fclose(fp); return NULL; }
    if (fseek(fp, 0, SEEK_SET) != 0) { fclose(fp); return NULL; }
    data = (uint8_t *)malloc((size_t)size);
    if (!data) { fclose(fp); return NULL; }
    if (fread(data, 1, (size_t)size, fp) != (size_t)size) {
        free(data); fclose(fp); return NULL;
    }
    fclose(fp);
    if (out_size) *out_size = size;
    return data;
}

/* Build a synthetic SCR with a 32-byte header + 256-byte section
 * table + section data, matching the SEGA SATURN SCR layout that
 * the existing nexus_v1_font_load_sections() parser expects. The
 * caller provides the populated offsets[] / sizes[] (one entry per
 * table index, in [0,32)) and the header char_count.
 *
 * This mirrors the helper inside
 * firestaff_nexus_v1_saturn_font_scr_sections_probe.c but is
 * reproduced here so the two probes stay independent. */
static uint8_t *build_scr(const uint32_t *offsets,
                          const uint32_t *sizes,
                          int header_char_count,
                          int *out_size)
{
    const int header = 32;
    const int table  = 32 * 8;
    const int data_offset = header + table;
    int data_total = 0;
    int i;
    uint8_t *buf;
    int cur;

    for (i = 0; i < 32; ++i) {
        if (sizes[i] == 0 && offsets[i] == 0) continue;
        data_total += (int)sizes[i];
    }

    buf = (uint8_t *)calloc(1, (size_t)(data_offset + data_total));
    if (!buf) {
        if (out_size) *out_size = 0;
        return NULL;
    }

    memcpy(buf, "SEGA SATURN SCR", 15);
    /* char_count u32 BE at offset 0x10 (low 16 bits = header_char_count). */
    buf[0x10] = 0; buf[0x11] = 0;
    buf[0x12] = (uint8_t)((header_char_count >> 8) & 0xFF);
    buf[0x13] = (uint8_t)(header_char_count & 0xFF);
    buf[0x14] = 0; buf[0x15] = 0; buf[0x16] = 0; buf[0x17] = 0x12;

    cur = data_offset;
    for (i = 0; i < 32; ++i) {
        uint8_t *entry = buf + header + i * 8;
        uint32_t off = offsets[i];
        uint32_t sz  = sizes[i];
        if (off == 0 && sz == 0) continue;
        entry[0] = (uint8_t)((off >> 24) & 0xFF);
        entry[1] = (uint8_t)((off >> 16) & 0xFF);
        entry[2] = (uint8_t)((off >> 8) & 0xFF);
        entry[3] = (uint8_t)(off & 0xFF);
        entry[4] = (uint8_t)((sz >> 24) & 0xFF);
        entry[5] = (uint8_t)((sz >> 16) & 0xFF);
        entry[6] = (uint8_t)((sz >> 8) & 0xFF);
        entry[7] = (uint8_t)(sz & 0xFF);
        if (sz > 0 && cur + (int)sz <= data_offset + data_total) {
            /* Touch every byte so the section-data region is not
             * all zero (the existing render probe touches pixels
             * via set_glyph_pixel; here we just need non-zero
             * bytes for any future section-content sanity check). */
            memset(buf + cur, (uint8_t)(0x40 + (i & 0x3F)), sz);
            cur += (int)sz;
        }
    }

    if (out_size) *out_size = data_offset + data_total;
    return buf;
}

/* Set the glyph pixel at (x,y) inside the synthetic font. The font
 * is treated as char_width x char_height 1bpp, row-major. */
static void set_glyph_pixel(uint8_t *scr,
                            int glyph_index,
                            int char_count,
                            int glyph_bytes_per_char,
                            int char_width,
                            int char_height,
                            int x,
                            int y)
{
    int row_stride = (char_width + 7) / 8;
    uint8_t *glyph = scr + 48 + glyph_index * glyph_bytes_per_char;
    uint8_t *row = glyph + y * row_stride;
    (void)char_count;
    if (x < 0 || x >= char_width) return;
    if (y < 0 || y >= char_height) return;
    row[x / 8] |= (uint8_t)(0x80u >> (x & 7));
}

/* Build a synthetic font with a "cross" pattern drawn for every
 * glyph. This lets the layout probe verify that draws really
 * wrote pixels (not just advanced the cursor). */
static uint8_t *make_cross_scr(int char_count,
                               int char_width,
                               int char_height,
                               int *out_size)
{
    int row_stride = (char_width + 7) / 8;
    int glyph_bytes = row_stride * char_height;
    int i;
    int x, y;
    int glyph_data = char_count * glyph_bytes;
    int total = 48 + glyph_data;
    uint8_t *scr = (uint8_t *)calloc(1, (size_t)total);
    if (!scr) {
        if (out_size) *out_size = 0;
        return NULL;
    }
    memcpy(scr, "SEGA SATURN SCR", 15);
    scr[0x10] = 0; scr[0x11] = 0;
    scr[0x12] = (uint8_t)((char_count >> 8) & 0xFF);
    scr[0x13] = (uint8_t)(char_count & 0xFF);
    scr[0x14] = 0; scr[0x15] = 0; scr[0x16] = 0; scr[0x17] = 0x12;

    for (i = 0; i < char_count; ++i) {
        for (y = 0; y < char_height; ++y) {
            for (x = 0; x < char_width; ++x) {
                /* Cross: only the diagonals and center row/column
                 * carry pixels; the rest is blank. Keeps the
                 * framebuffer hash stable across rebuilds. */
                if (x == y || x == (char_width - 1 - y) ||
                    x == (char_width / 2) || y == (char_height / 2)) {
                    set_glyph_pixel(scr, i, char_count, glyph_bytes,
                                    char_width, char_height, x, y);
                }
            }
        }
    }
    if (out_size) *out_size = total;
    return scr;
}

/* Build a synthetic 4-section SCR font whose populated sections
 * cover char_count=256 with bytes_per_glyph=32. This mirrors the
 * FONT256.S2D shape without claiming real-asset parity. */
static uint8_t *make_synthetic_4section_font(int char_count,
                                             int *out_size)
{
    /* Total bytes needed for char_count 1bpp 16x16 glyphs. */
    int glyph_bytes = (16 / 8) * 16;  /* = 32 bytes/glyph */
    int total = char_count * glyph_bytes;
    int section_sizes[4] = {0};
    int i;
    int cur = 0;
    int rem = total;
    int data_offset = 32 + 32 * 8;
    uint8_t *scr;

    /* Divide `total` across 4 sections. */
    for (i = 0; i < 4; ++i) {
        int sz = rem / (4 - i);
        section_sizes[i] = sz;
        rem -= sz;
    }

    scr = (uint8_t *)calloc(1, (size_t)(data_offset + total));
    if (!scr) { if (out_size) *out_size = 0; return NULL; }
    memcpy(scr, "SEGA SATURN SCR", 15);
    scr[0x10] = 0; scr[0x11] = 0;
    scr[0x12] = (uint8_t)((char_count >> 8) & 0xFF);
    scr[0x13] = (uint8_t)(char_count & 0xFF);
    scr[0x14] = 0; scr[0x15] = 0; scr[0x16] = 0; scr[0x17] = 0x12;

    /* 4 populated entries at indices 0, 2, 4, 6 (FONT256.S2D shape). */
    for (i = 0; i < 4; ++i) {
        uint8_t *entry = scr + 32 + (i * 2) * 8;
        uint32_t off = (uint32_t)(data_offset + cur);
        uint32_t sz = (uint32_t)section_sizes[i];
        entry[0] = (uint8_t)((off >> 24) & 0xFF);
        entry[1] = (uint8_t)((off >> 16) & 0xFF);
        entry[2] = (uint8_t)((off >> 8) & 0xFF);
        entry[3] = (uint8_t)(off & 0xFF);
        entry[4] = (uint8_t)((sz >> 24) & 0xFF);
        entry[5] = (uint8_t)((sz >> 16) & 0xFF);
        entry[6] = (uint8_t)((sz >> 8) & 0xFF);
        entry[7] = (uint8_t)(sz & 0xFF);

        /* Touch every byte so the section-data region is not zero. */
        memset(scr + data_offset + cur, (uint8_t)(0x40 + (i * 16)), sz);
        cur += sz;
    }

    if (out_size) *out_size = data_offset + total;
    return scr;
}

static void run_map_only_gate(void) {
    uint32_t offsets[32] = {0};
    uint32_t sizes[32]   = {0};
    uint8_t *scr;
    int scr_size = 0;
    Nexus_V1_FontSections sections;
    Nexus_V1_S2D_SectionGlyphMap map;
    int rc;

    fprintf(stderr, "\n-- synthetic section→glyph-range map gate --\n");

    /* [1] NULL safety. */
    rc = nexus_v1_s2d_section_glyph_map(NULL, 32, &map);
    CHECK(rc == -1, "section_glyph_map(NULL sections, ...) rejected");

    scr = build_scr(offsets, sizes, 256, &scr_size);
    rc = nexus_v1_s2d_section_glyph_map(&sections, 32, NULL);
    CHECK(rc == -1, "section_glyph_map(..., NULL out) rejected");

    /* [2] bytes_per_glyph=0 rejected. */
    rc = nexus_v1_s2d_section_glyph_map(&sections, 0, &map);
    CHECK(rc == -1, "section_glyph_map rejects bytes_per_glyph=0");
    free(scr);

    /* [3] Empty section table. */
    memset(offsets, 0, sizeof(offsets));
    memset(sizes, 0, sizeof(sizes));
    scr = build_scr(offsets, sizes, 256, &scr_size);
    rc = nexus_v1_font_load_sections(scr, scr_size, &sections);
    CHECK(rc == 0, "empty-section-table SCR parses");
    rc = nexus_v1_s2d_section_glyph_map(&sections, 32, &map);
    CHECK(rc == 0, "empty-section-table map builds");
    CHECK(map.range_count == 0, "empty-section-table map has 0 ranges");
    CHECK(map.char_count == 0, "empty-section-table map char_count == 0");
    CHECK(map.header_char_count == 256,
          "empty-section-table map header_char_count == 256");
    CHECK(nexus_v1_s2d_glyph_map_total_chars(&map) == 0,
          "empty map total_chars == 0");
    free(scr);

    /* [4] Single populated section at index 5. */
    memset(offsets, 0, sizeof(offsets));
    memset(sizes, 0, sizeof(sizes));
    offsets[5] = 0x120;
    sizes[5] = 32 * 64;  /* 64 glyphs at 32 bytes each */
    scr = build_scr(offsets, sizes, 256, &scr_size);
    rc = nexus_v1_font_load_sections(scr, scr_size, &sections);
    CHECK(rc == 0, "single-populated-section SCR parses");
    rc = nexus_v1_s2d_section_glyph_map(&sections, 32, &map);
    CHECK(rc == 0, "single-section map builds");
    CHECK(map.range_count == 1, "single-section map has 1 range");
    CHECK(map.ranges[0].char_start == 0, "single-section char_start=0");
    CHECK(map.ranges[0].char_count == 64,
          "single-section char_count = 64 (32*64/32)");
    CHECK(map.ranges[0].table_index == 5,
          "single-section range preserves original table_index=5");
    CHECK(map.ranges[0].parsed_section_index == 0,
          "single-section range parsed_section_index=0");
    CHECK(nexus_v1_s2d_glyph_map_total_chars(&map) == 64,
          "single-section map total_chars == 64");
    CHECK(nexus_v1_s2d_glyph_range_lookup(&map, 0) == 0,
          "lookup char 0 returns range 0");
    CHECK(nexus_v1_s2d_glyph_range_lookup(&map, 63) == 0,
          "lookup char 63 returns range 0");
    CHECK(nexus_v1_s2d_glyph_range_lookup(&map, 64) == -1,
          "lookup char 64 returns -1 (out of coverage)");
    CHECK(nexus_v1_s2d_glyph_range_lookup(&map, 256) == -1,
          "lookup char 256 returns -1");
    CHECK(nexus_v1_s2d_glyph_range_lookup(&map, -1) == -1,
          "lookup char -1 returns -1");
    free(scr);

    /* [5] Four-populated synthetic FONT256-shape fixture. */
    {
        uint8_t *font = make_synthetic_4section_font(256, &scr_size);
        rc = nexus_v1_font_load_sections(font, scr_size, &sections);
        CHECK(rc == 0, "four-populated synthetic SCR parses");
        rc = nexus_v1_s2d_section_glyph_map(&sections, 32, &map);
        CHECK(rc == 0, "four-populated synthetic map builds");
        CHECK(map.range_count == 4, "synthetic map has 4 ranges");
        CHECK(map.ranges[0].table_index == 0 &&
              map.ranges[1].table_index == 2 &&
              map.ranges[2].table_index == 4 &&
              map.ranges[3].table_index == 6,
              "synthetic map preserves table indices 0,2,4,6");
        CHECK(map.ranges[0].char_start == 0 &&
              map.ranges[1].char_start == map.ranges[0].char_count &&
              map.ranges[2].char_start == map.ranges[1].char_start +
                                       map.ranges[1].char_count &&
              map.ranges[3].char_start == map.ranges[2].char_start +
                                       map.ranges[2].char_count,
              "synthetic map char_start forms a contiguous partition");
        CHECK(nexus_v1_s2d_glyph_map_total_chars(&map) == 256,
              "synthetic map total_chars == 256");
        free(font);
    }

    /* [6] Header char_count acts as hard cap. */
    {
        uint8_t *font = make_synthetic_4section_font(256, &scr_size);
        /* Rebuild with header char_count=100 — that caps coverage
         * even though the section byte budget would cover 256. */
        uint32_t caps[32] = {0};
        uint32_t siz[32]  = {0};
        uint8_t *capped_font = build_scr(caps, siz, 100, &scr_size);
        (void)font;
        rc = nexus_v1_font_load_sections(capped_font, scr_size, &sections);
        CHECK(rc == 0, "empty capped-coverage SCR parses");
        rc = nexus_v1_s2d_section_glyph_map(&sections, 32, &map);
        CHECK(rc == 0, "capped-coverage map builds (no populated sections)");
        CHECK(map.range_count == 0, "capped-coverage map has 0 ranges");
        CHECK(map.char_count == 0, "capped-coverage map char_count == 0");
        CHECK(map.header_char_count == 100,
              "capped-coverage map header_char_count == 100");
        free(capped_font);
        free(font);
    }

    /* [7] Single section capped by header char_count. */
    {
        uint8_t *scr2;
        memset(offsets, 0, sizeof(offsets));
        memset(sizes, 0, sizeof(sizes));
        offsets[0] = 0x120;
        sizes[0] = 32 * 64;  /* would cover 64 glyphs */
        scr2 = build_scr(offsets, sizes, 32 /* header cap */, &scr_size);
        rc = nexus_v1_font_load_sections(scr2, scr_size, &sections);
        CHECK(rc == 0, "header-capped single-section SCR parses");
        rc = nexus_v1_s2d_section_glyph_map(&sections, 32, &map);
        CHECK(rc == 0, "header-capped map builds");
        CHECK(map.range_count == 1, "header-capped map has 1 range");
        CHECK(map.ranges[0].char_count == 32,
              "header-capped char_count == 32 (smaller of 64 and 32)");
        CHECK(map.char_count == 32, "header-capped map.char_count == 32");
        CHECK(map.header_char_count == 32,
              "header-capped map.header_char_count == 32");
        free(scr2);
    }
}

static void run_layout_only_gate(void) {
    int scr_size = 0;
    uint8_t *font_scr;
    Nexus_V1_Font font;
    Nexus_V1_FontSections sections;
    Nexus_V1_S2D_SectionGlyphMap map;
    Nexus_V1_S2D_TextLayout layout;
    Nexus_V1_S2D_TextLayoutConfig cfg;
    uint8_t fb[64 * 16];
    int drawn;
    int rc;

    fprintf(stderr, "\n-- synthetic layout gate --\n");

    /* Build a 16x16 1bpp font covering 64 glyphs with a "cross"
     * pattern. Section table at index 0 covers all 64 glyphs. */
    memset(fb, 0, sizeof(fb));
    font_scr = make_cross_scr(64, 16, 16, &scr_size);
    rc = nexus_v1_font_load(&font, font_scr, scr_size);
    CHECK(rc > 0, "synthetic cross-font loads");
    CHECK(font.char_width == 16 && font.char_height == 16,
          "synthetic cross-font dimensions 16x16");

    {
        uint32_t offsets[32] = {0};
        uint32_t sizes[32]   = {0};
        uint8_t *scr2;
        int ss2 = 0;
        offsets[0] = 0x120;
        sizes[0] = (uint32_t)(64 * ((16 + 7) / 8) * 16);
        scr2 = build_scr(offsets, sizes, 64, &ss2);
        rc = nexus_v1_font_load_sections(scr2, ss2, &sections);
        CHECK(rc == 0, "single-section synthetic SCR parses for layout");
        rc = nexus_v1_s2d_section_glyph_map(&sections, 32, &map);
        CHECK(rc == 0, "single-section synthetic map builds for layout");
        free(scr2);
    }

    /* [9] init NULL-safety. */
    CHECK(nexus_v1_s2d_text_layout_init(NULL, &font, &map, &cfg) == -1,
          "layout_init(NULL layout, ...) rejected");
    CHECK(nexus_v1_s2d_text_layout_init(&layout, NULL, &map, &cfg) == -1,
          "layout_init(NULL font, ...) rejected");
    CHECK(nexus_v1_s2d_text_layout_init(&layout, &font, NULL, &cfg) == -1,
          "layout_init(NULL map, ...) rejected");

    /* Re-init with a real font + map. */
    memset(&cfg, 0, sizeof(cfg));
    cfg.fg_index = 7;
    cfg.bg_index = -1;
    cfg.letter_spacing_x = 1;
    cfg.line_height = 0;  /* auto = 16 */
    cfg.tab_stop = 0;
    cfg.bytes_per_glyph = 32;
    rc = nexus_v1_s2d_text_layout_init(&layout, &font, &map, &cfg);
    CHECK(rc == 0, "layout_init succeeds with valid font + map + config");
    CHECK(layout.initialized == 1, "layout.initialized == 1 after init");

    /* [10] reset_cursor. */
    layout.cursor.chars_drawn = 99;
    layout.cursor.writes = 9999;
    nexus_v1_s2d_text_layout_reset_cursor(&layout);
    CHECK(layout.cursor.chars_drawn == 0, "reset_cursor clears chars_drawn");
    CHECK(layout.cursor.writes == 0, "reset_cursor clears writes");

    /* [11] Empty string. */
    {
        uint64_t hash_before, hash_after;
        memset(fb, 0, sizeof(fb));
        hash_before = fnv1a64(fb, sizeof(fb));
        drawn = nexus_v1_s2d_text_layout_draw_string(
            &layout, fb, 64, 16, 64, "");
        hash_after = fnv1a64(fb, sizeof(fb));
        CHECK(drawn == 0, "draw_string(\"\") returns 0");
        CHECK(layout.cursor.chars_drawn == 0,
              "empty string does not bump chars_drawn");
        CHECK(hash_before == hash_after,
              "empty string does not modify framebuffer hash");
    }

    /* [11b] Sanity: same fb zeroed twice produces the same hash. */
    {
        uint64_t h0, h1;
        memset(fb, 0, sizeof(fb));
        h0 = fnv1a64(fb, sizeof(fb));
        memset(fb, 0, sizeof(fb));
        h1 = fnv1a64(fb, sizeof(fb));
        CHECK(h0 == h1,
              "pre-cleared fb hashes deterministically (sanity check)");
    }

    /* [12] Single character 'A' paints glyph 33 ('A' - 0x20 = 33). */
    memset(fb, 0, sizeof(fb));
    nexus_v1_s2d_text_layout_reset_cursor(&layout);
    drawn = nexus_v1_s2d_text_layout_draw_string(
        &layout, fb, 64, 16, 64, "A");
    CHECK(drawn == 1, "draw_string(\"A\") returns 1");
    CHECK(layout.cursor.chars_drawn == 1, "single-glyph chars_drawn == 1");
    CHECK(layout.cursor.cursor_x == 16 + 1, "single-glyph cursor_x = w+sp");
    CHECK(layout.cursor.writes > 0, "single-glyph paints at least 1 pixel");
    {
        int x, y, hits = 0;
        for (y = 0; y < 16; ++y) {
            for (x = 0; x < 17; ++x) {
                if (fb[y * 64 + x] == 7) ++hits;
            }
        }
        CHECK(hits > 0, "single-glyph paints fg_index=7 somewhere");
    }

    /* [13] Cursor advance on multi-glyph. */
    memset(fb, 0, sizeof(fb));
    nexus_v1_s2d_text_layout_reset_cursor(&layout);
    drawn = nexus_v1_s2d_text_layout_draw_string(
        &layout, fb, 64, 16, 64, "ABCDE");
    CHECK(drawn == 5, "draw_string(\"ABCDE\") returns 5");
    CHECK(layout.cursor.chars_drawn == 5, "ABCDE chars_drawn == 5");
    CHECK(layout.cursor.cursor_x == 5 * (16 + 1),
          "ABCDE cursor_x = 5*(16+letter_spacing)");

    /* [14] '\n' advances y, resets x, increments line_count. */
    memset(fb, 0, sizeof(fb));
    nexus_v1_s2d_text_layout_reset_cursor(&layout);
    drawn = nexus_v1_s2d_text_layout_draw_string(
        &layout, fb, 64, 32, 64, "AB\nCD");
    CHECK(drawn == 4, "draw_string(\"AB\\nCD\") returns 4");
    CHECK(layout.cursor.chars_drawn == 4, "AB\\nCD chars_drawn == 4");
    CHECK(layout.cursor.line_count == 1, "AB\\nCD line_count == 1");
    CHECK(layout.cursor.newline_count == 1, "AB\\nCD newline_count == 1");
    CHECK(layout.cursor.cursor_x == 2 * (16 + 1),
          "AB\\nCD cursor_x = 2*(16+letter_spacing)");
    CHECK(layout.cursor.cursor_y == 16, "AB\\nCD cursor_y == line_height");

    /* [15] Deterministic screen-text hash. */
    {
        uint64_t hash_a, hash_b;
        memset(fb, 0, sizeof(fb));
        nexus_v1_s2d_text_layout_reset_cursor(&layout);
        nexus_v1_s2d_text_layout_draw_string(
            &layout, fb, 64, 16, 64,
            "NEXUS");
        hash_a = fnv1a64(fb, sizeof(fb));

        memset(fb, 0, sizeof(fb));
        nexus_v1_s2d_text_layout_reset_cursor(&layout);
        nexus_v1_s2d_text_layout_draw_string(
            &layout, fb, 64, 16, 64,
            "NEXUS");
        hash_b = fnv1a64(fb, sizeof(fb));

        CHECK(hash_a == hash_b,
              "deterministic FNV-1a hash for \"NEXUS\" run twice");
        CHECK(hash_a != UINT64_C(0xcbf29ce484222325),
              "non-empty \"NEXUS\" differs from FNV-1a iv");
    }

    /* [16] max_chars cap. */
    {
        memset(fb, 0, sizeof(fb));
        nexus_v1_s2d_text_layout_reset_cursor(&layout);
        nexus_v1_s2d_text_layout_set_max_chars(&layout, 3);
        drawn = nexus_v1_s2d_text_layout_draw_string(
            &layout, fb, 64, 16, 64, "ABCDE");
        CHECK(drawn == 3, "max_chars=3 caps draw_string at 3 glyphs");
        CHECK(layout.cursor.chars_drawn == 3, "capped chars_drawn == 3");
        CHECK(layout.cursor.cursor_x == 3 * (16 + 1),
              "capped cursor_x = 3*(16+letter_spacing)");
        nexus_v1_s2d_text_layout_set_max_chars(&layout, 0);
    }

    /* [17] Out-of-coverage: build a 16-glyph font and ask the
     * layout to draw a 32-glyph string. */
    {
        int tiny_size = 0;
        uint8_t *tiny_font = make_cross_scr(16, 16, 16, &tiny_size);
        Nexus_V1_Font tiny;
        uint32_t offsets[32] = {0};
        uint32_t sizes[32] = {0};
        uint8_t *scr2;
        int ss2 = 0;
        Nexus_V1_FontSections tiny_sec;
        Nexus_V1_S2D_SectionGlyphMap tiny_map;
        Nexus_V1_S2D_TextLayout tiny_layout;

        rc = nexus_v1_font_load(&tiny, tiny_font, tiny_size);
        offsets[0] = 0x120;
        sizes[0] = (uint32_t)(16 * 32);
        scr2 = build_scr(offsets, sizes, 16, &ss2);
        rc = nexus_v1_font_load_sections(scr2, ss2, &tiny_sec);
        CHECK(rc == 0, "tiny 16-glyph font SCR parses");
        rc = nexus_v1_s2d_section_glyph_map(&tiny_sec, 32, &tiny_map);
        CHECK(rc == 0, "tiny 16-glyph map builds");

        rc = nexus_v1_s2d_text_layout_init(&tiny_layout, &tiny,
                                            &tiny_map, &cfg);
        CHECK(rc == 0, "tiny layout inits");

        memset(fb, 0, sizeof(fb));
        /* ' ' (0x20) -> glyph 0; '@' (0x40) -> glyph 32 (out of
         * coverage for the 16-glyph font, which only covers glyphs
         * 0..15). */
        drawn = nexus_v1_s2d_text_layout_draw_string(
            &tiny_layout, fb, 64, 16, 64, " @");
        CHECK(drawn == 1, "out-of-coverage chars_skipped; only ' ' (glyph 0) drawn");
        CHECK(tiny_layout.cursor.chars_skipped == 1,
              "tiny-layout chars_skipped == 1 for glyph 32 input");
        CHECK(tiny_layout.cursor.chars_drawn == 1,
              "tiny-layout chars_drawn == 1 for in-coverage ' ' glyph 0");

        nexus_v1_s2d_text_layout_free(&tiny_layout);
        nexus_v1_font_free(&tiny);
        free(tiny_font);
        free(scr2);
    }

    nexus_v1_s2d_text_layout_free(&layout);
    nexus_v1_font_free(&font);
    free(font_scr);
}

static const char *default_real_font_path(char *buf, size_t cap) {
    const char *env = getenv("FIRESTAFF_NEXUS_FONT256_S2D");
    const char *home;
    if (env && env[0]) return env;
    home = getenv("HOME");
    if (!home || !home[0] || cap == 0) return NULL;
    snprintf(buf, cap, "%s/.firestaff/data/nexus/FONT256.S2D", home);
    return buf;
}

static void run_optional_real_asset_gate(void) {
    char path_buf[1024];
    const char *path = default_real_font_path(path_buf, sizeof(path_buf));
    long size = 0;
    uint8_t *data;

    fprintf(stderr, "\n-- optional real FONT256.S2D section-bound layout gate --\n");

    data = read_entire_file(path, &size);
    if (!data) {
        fprintf(stderr, "  SKIP: no local FONT256.S2D at %s\n",
               path ? path : "(unset)");
        return;
    }

    {
        Nexus_V1_Font font;
        Nexus_V1_FontSections sections;
        Nexus_V1_S2D_SectionGlyphMap map;
        Nexus_V1_S2D_TextLayout layout;
        Nexus_V1_S2D_TextLayoutConfig cfg;
        uint8_t fb[512 * 32];
        int rc;
        int drawn;
        uint64_t hash_a, hash_b, hash_c;

        /* Pad the framebuffer ahead of memset. */
        if ((long)sizeof(fb) < (long)size + 32) {
            /* framebuffer is bigger than the asset; that's fine. */
        }

        rc = nexus_v1_font_load(&font, data, (int)size);
        CHECK(rc > 0, "real FONT256.S2D parses into the flat 1bpp font");
        CHECK(font.char_count == 256,
              "real FONT256.S2D reports char_count=256");

        rc = nexus_v1_font_load_sections(data, (int)size, &sections);
        CHECK(rc == 0, "real FONT256.S2D section table parses");
        CHECK(sections.section_count == 4,
              "real FONT256.S2D reports 4 populated sections");

        rc = nexus_v1_s2d_section_glyph_map(&sections, 32, &map);
        CHECK(rc == 0, "real-asset section→glyph-range map builds");
        CHECK(map.range_count == 1,
              "real-asset map has 1 range (section [0] covers 256 chars alone)");
        CHECK(map.ranges[0].table_index == 0,
              "real-asset map covers table index 0");
        CHECK(map.ranges[0].char_count == 256,
              "real-asset range [0] char_count == 256 (8208 bytes / 32)");
        CHECK(map.char_count == 256,
              "real-asset map char_count == 256 (effective coverage)");
        CHECK(map.header_char_count == 256,
              "real-asset map header_char_count == 256");
        CHECK(nexus_v1_s2d_glyph_map_total_chars(&map) == 256,
              "real-asset map total_chars == 256");
        CHECK(nexus_v1_s2d_glyph_range_lookup(&map, 0) == 0,
              "real-asset lookup char 0 -> range 0");
        CHECK(nexus_v1_s2d_glyph_range_lookup(&map, 255) == 0,
              "real-asset lookup char 255 -> range 0 (same range as 0)");
        CHECK(nexus_v1_s2d_glyph_range_lookup(&map, 256) == -1,
              "real-asset lookup char 256 -> -1 (out of coverage)");

        memset(&cfg, 0, sizeof(cfg));
        cfg.fg_index = 1;
        cfg.bg_index = 0;
        cfg.letter_spacing_x = 0;
        cfg.line_height = 0;
        cfg.tab_stop = 0;
        cfg.bytes_per_glyph = 32;
        rc = nexus_v1_s2d_text_layout_init(&layout, &font, &map, &cfg);
        CHECK(rc == 0, "real-asset layout inits");

        /* Three independent runs of the same draw script must
         * produce identical framebuffer hashes. */
        memset(fb, 0, sizeof(fb));
        nexus_v1_s2d_text_layout_reset_cursor(&layout);
        drawn = nexus_v1_s2d_text_layout_draw_string(
            &layout, fb, (int)sizeof(fb) / 32, 32, (int)sizeof(fb) / 32,
            "NEXUS");
        hash_a = fnv1a64(fb, sizeof(fb));

        memset(fb, 0, sizeof(fb));
        nexus_v1_s2d_text_layout_reset_cursor(&layout);
        drawn = nexus_v1_s2d_text_layout_draw_string(
            &layout, fb, (int)sizeof(fb) / 32, 32, (int)sizeof(fb) / 32,
            "NEXUS");
        hash_b = fnv1a64(fb, sizeof(fb));

        memset(fb, 0, sizeof(fb));
        nexus_v1_s2d_text_layout_reset_cursor(&layout);
        drawn = nexus_v1_s2d_text_layout_draw_string(
            &layout, fb, (int)sizeof(fb) / 32, 32, (int)sizeof(fb) / 32,
            "NEXUS");
        hash_c = fnv1a64(fb, sizeof(fb));

        CHECK(drawn == 5, "real-asset layout draws 5 glyphs for \"NEXUS\"");
        CHECK(layout.cursor.chars_drawn == 5,
              "real-asset chars_drawn == 5 for \"NEXUS\"");
        CHECK(layout.cursor.chars_skipped == 0,
              "real-asset chars_skipped == 0 for \"NEXUS\" (all in coverage)");
        CHECK(layout.cursor.writes > 0,
              "real-asset writes > 0 (at least one fg pixel painted)");
        CHECK(hash_a == hash_b && hash_b == hash_c,
              "real-asset FNV-1a framebuffer hash is deterministic across 3 runs");

        nexus_v1_s2d_text_layout_free(&layout);
        nexus_v1_font_free(&font);
    }

    free(data);
}

int main(void) {
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
    fprintf(stderr, "=== Nexus V1 S2D runtime text-layout probe ===\n");

    run_map_only_gate();
    run_layout_only_gate();
    run_optional_real_asset_gate();

    fprintf(stderr, "\n# summary: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail > 0 ? 1 : 0;
}

/*
 * firestaff_nexus_v1_screen_text_surface_probe.c
 * ===============================================
 *
 * Deterministic runtime-surface gate for the Nexus S2D screen-text bridge.
 * Data-free by default; optional FONT256.S2D receipt when the operator has
 * staged the real asset.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nexus_v1_screen_text.h"
#include "nexus_v1_saturn_font.h"

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do {                                      \
    if (cond) { fprintf(stderr, "  PASS: %s\n", msg); ++g_pass; }  \
    else      { fprintf(stderr, "  FAIL: %s\n", msg); ++g_fail; }  \
} while (0)

static uint8_t *read_entire_file(const char *path, long *out_size)
{
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
        free(data);
        fclose(fp);
        return NULL;
    }
    fclose(fp);
    if (out_size) *out_size = size;
    return data;
}

static uint8_t *build_section_scr(uint32_t section_offset,
                                  uint32_t section_size,
                                  int header_char_count,
                                  int table_index,
                                  int *out_size)
{
    const int header = 32;
    const int table = 32 * 8;
    const int total = header + table + (int)section_size;
    uint8_t *buf = (uint8_t *)calloc(1, (size_t)total);
    uint8_t *entry;

    if (!buf) {
        if (out_size) *out_size = 0;
        return NULL;
    }
    memcpy(buf, "SEGA SATURN SCR", 15);
    buf[0x10] = 0;
    buf[0x11] = 0;
    buf[0x12] = (uint8_t)((header_char_count >> 8) & 0xFF);
    buf[0x13] = (uint8_t)(header_char_count & 0xFF);
    buf[0x17] = 0x12;

    if (table_index < 0) table_index = 0;
    if (table_index >= 32) table_index = 31;
    entry = buf + header + table_index * 8;
    entry[0] = (uint8_t)((section_offset >> 24) & 0xFF);
    entry[1] = (uint8_t)((section_offset >> 16) & 0xFF);
    entry[2] = (uint8_t)((section_offset >> 8) & 0xFF);
    entry[3] = (uint8_t)(section_offset & 0xFF);
    entry[4] = (uint8_t)((section_size >> 24) & 0xFF);
    entry[5] = (uint8_t)((section_size >> 16) & 0xFF);
    entry[6] = (uint8_t)((section_size >> 8) & 0xFF);
    entry[7] = (uint8_t)(section_size & 0xFF);

    memset(buf + header + table, 0x5A, (size_t)section_size);
    if (out_size) *out_size = total;
    return buf;
}

static void set_glyph_pixel(uint8_t *scr,
                            int glyph_index,
                            int glyph_bytes,
                            int char_width,
                            int x,
                            int y)
{
    int row_stride = (char_width + 7) / 8;
    uint8_t *glyph = scr + 48 + glyph_index * glyph_bytes;
    uint8_t *row = glyph + y * row_stride;

    row[x / 8] |= (uint8_t)(0x80u >> (x & 7));
}

static uint8_t *make_cross_font(int char_count,
                                int char_width,
                                int char_height,
                                int *out_size)
{
    const int row_stride = (char_width + 7) / 8;
    const int glyph_bytes = row_stride * char_height;
    const int total = 48 + char_count * glyph_bytes;
    uint8_t *scr = (uint8_t *)calloc(1, (size_t)total);
    int g, x, y;

    if (!scr) {
        if (out_size) *out_size = 0;
        return NULL;
    }
    memcpy(scr, "SEGA SATURN SCR", 15);
    scr[0x10] = 0;
    scr[0x11] = 0;
    scr[0x12] = (uint8_t)((char_count >> 8) & 0xFF);
    scr[0x13] = (uint8_t)(char_count & 0xFF);
    scr[0x17] = 0x12;

    for (g = 0; g < char_count; ++g) {
        for (y = 0; y < char_height; ++y) {
            for (x = 0; x < char_width; ++x) {
                if (x == y || x == (char_width - 1 - y) ||
                    x == (char_width / 2) || y == (char_height / 2)) {
                    set_glyph_pixel(scr, g, glyph_bytes, char_width, x, y);
                }
            }
        }
    }

    if (out_size) *out_size = total;
    return scr;
}

static const char *default_real_font_path(char *buf, size_t cap)
{
    const char *env = getenv("FIRESTAFF_NEXUS_FONT256_S2D");
    const char *home;
    if (env && env[0]) return env;
    home = getenv("HOME");
    if (!home || !home[0] || cap == 0) return NULL;
    snprintf(buf, cap, "%s/.firestaff/data/nexus/FONT256.S2D", home);
    return buf;
}

static void run_synthetic_surface_gate(void)
{
    enum { FB_W = 128, FB_H = 48, FB_STRIDE = 128 };
    uint8_t fb[FB_STRIDE * FB_H];
    uint8_t fb2[FB_STRIDE * FB_H];
    uint8_t *font_scr;
    uint8_t *section_scr;
    int font_size = 0;
    int section_size = 0;
    Nexus_V1_Font font;
    Nexus_V1_FontSections sections;
    Nexus_V1_ScreenTextStyle style;
    Nexus_V1_ScreenTextReceipt receipt;
    Nexus_V1_ScreenTextReceipt receipt2;
    int rc;
    int drawn;
    int x, y;
    int outside_writes = 0;

    fprintf(stderr, "\n-- synthetic S2D screen-text surface gate --\n");

    memset(&font, 0, sizeof(font));
    font_scr = make_cross_font(96, 16, 16, &font_size);
    section_scr = build_section_scr(0x120, 96u * 32u, 96, 3, &section_size);
    rc = nexus_v1_font_load(&font, font_scr, font_size);
    CHECK(rc > 0, "synthetic S2D font loads through flat 1bpp parser");
    rc = nexus_v1_font_load_sections(section_scr, section_size, &sections);
    CHECK(rc == 0, "synthetic screen-text section table parses");
    CHECK(sections.section_count == 1,
          "synthetic screen-text section table has one populated section");

    memset(&style, 0, sizeof(style));
    style.x = 4;
    style.y = 3;
    style.fg_index = 9;
    style.bg_index = -1;
    style.letter_spacing_x = 1;
    style.line_height = 0;
    style.tab_stop = 0;
    style.max_chars = 0;
    style.bytes_per_glyph = 32;

    memset(fb, 0, sizeof(fb));
    drawn = nexus_v1_screen_text_draw_indexed(
        &font, &sections, fb, FB_W, FB_H, FB_STRIDE,
        "NEXUS", &style, &receipt);
    CHECK(drawn == 5, "screen_text_draw_indexed draws 5 glyphs");
    CHECK(receipt.status == NEXUS_V1_SCREEN_TEXT_OK,
          "screen-text receipt status OK");
    CHECK(receipt.range_count == 1, "screen-text receipt range_count == 1");
    CHECK(receipt.header_char_count == 96,
          "screen-text receipt header_char_count == 96");
    CHECK(receipt.char_count == 96, "screen-text receipt char_count == 96");
    CHECK(receipt.chars_drawn == 5, "screen-text receipt chars_drawn == 5");
    CHECK(receipt.chars_skipped == 0,
          "screen-text receipt chars_skipped == 0");
    CHECK(receipt.writes > 0, "screen-text receipt writes > 0");
    CHECK(receipt.cursor_x == style.x + 5 * (16 + style.letter_spacing_x),
          "screen-text cursor advances from requested x origin");
    CHECK(receipt.cursor_y == style.y,
          "screen-text cursor_y preserves requested y origin for single line");
    CHECK(receipt.framebuffer_hash !=
          nexus_v1_screen_text_fnv1a64(NULL, 0),
          "screen-text framebuffer hash differs from FNV iv");

    for (y = 0; y < FB_H; ++y) {
        for (x = 0; x < FB_W; ++x) {
            if (fb[y * FB_STRIDE + x] != 0) {
                if (x < style.x || y < style.y || y >= style.y + 16 ||
                    x >= style.x + 5 * (16 + style.letter_spacing_x)) {
                    outside_writes++;
                }
            }
        }
    }
    CHECK(outside_writes == 0,
          "screen-text pixels stay inside the requested single-line band");

    memset(fb2, 0, sizeof(fb2));
    drawn = nexus_v1_screen_text_draw_indexed(
        &font, &sections, fb2, FB_W, FB_H, FB_STRIDE,
        "NEXUS", &style, &receipt2);
    CHECK(drawn == 5, "repeat screen-text draw returns 5 glyphs");
    CHECK(receipt.framebuffer_hash == receipt2.framebuffer_hash,
          "repeat screen-text draw has deterministic framebuffer hash");
    CHECK(memcmp(fb, fb2, sizeof(fb)) == 0,
          "repeat screen-text draw produces byte-identical framebuffer");

    memset(fb, 0, sizeof(fb));
    style.max_chars = 3;
    drawn = nexus_v1_screen_text_draw_indexed(
        &font, &sections, fb, FB_W, FB_H, FB_STRIDE,
        "NEXUS", &style, &receipt);
    CHECK(drawn == 3, "screen-text max_chars caps draw at 3 glyphs");
    CHECK(receipt.chars_drawn == 3, "max_chars receipt chars_drawn == 3");

    CHECK(nexus_v1_screen_text_draw_indexed(
              NULL, &sections, fb, FB_W, FB_H, FB_STRIDE,
              "NEXUS", &style, &receipt) ==
          NEXUS_V1_SCREEN_TEXT_ERR_INVALID_ARG,
          "screen_text_draw_indexed rejects NULL font");
    CHECK(receipt.status == NEXUS_V1_SCREEN_TEXT_ERR_INVALID_ARG,
          "NULL-font receipt status INVALID_ARG");
    CHECK(nexus_v1_screen_text_draw_indexed(
              &font, &sections, fb, FB_W, FB_H, FB_W - 1,
              "NEXUS", &style, &receipt) ==
          NEXUS_V1_SCREEN_TEXT_ERR_INVALID_ARG,
          "screen_text_draw_indexed rejects stride < width");
    CHECK(strcmp(nexus_v1_screen_text_status_name(
                     NEXUS_V1_SCREEN_TEXT_ERR_GLYPH_MAP),
                 "GLYPH_MAP") == 0,
          "screen-text status name lookup is stable");

    nexus_v1_font_free(&font);
    free(font_scr);
    free(section_scr);
}

static void run_optional_real_asset_gate(void)
{
    char path_buf[1024];
    const char *path = default_real_font_path(path_buf, sizeof(path_buf));
    long size = 0;
    uint8_t *data;

    fprintf(stderr, "\n-- optional real FONT256.S2D screen-text bridge gate --\n");
    data = read_entire_file(path, &size);
    if (!data) {
        fprintf(stderr, "  SKIP: no local FONT256.S2D at %s\n",
                path ? path : "(unset)");
        return;
    }

    {
        enum { FB_W = 160, FB_H = 32, FB_STRIDE = 160 };
        uint8_t fb[FB_STRIDE * FB_H];
        uint8_t fb2[FB_STRIDE * FB_H];
        Nexus_V1_ScreenTextStyle style;
        Nexus_V1_ScreenTextReceipt a;
        Nexus_V1_ScreenTextReceipt b;
        int drawn_a;
        int drawn_b;

        memset(&style, 0, sizeof(style));
        style.x = 0;
        style.y = 0;
        style.fg_index = 2;
        style.bg_index = 0;
        style.letter_spacing_x = 0;
        style.bytes_per_glyph = 32;

        memset(fb, 0, sizeof(fb));
        drawn_a = nexus_v1_screen_text_draw_s2d_bytes(
            data, (int)size, fb, FB_W, FB_H, FB_STRIDE,
            "NEXUS", &style, &a);
        memset(fb2, 0, sizeof(fb2));
        drawn_b = nexus_v1_screen_text_draw_s2d_bytes(
            data, (int)size, fb2, FB_W, FB_H, FB_STRIDE,
            "NEXUS", &style, &b);

        CHECK(drawn_a == 5 && drawn_b == 5,
              "real FONT256.S2D screen-text bridge draws 5 glyphs twice");
        CHECK(a.status == NEXUS_V1_SCREEN_TEXT_OK &&
              b.status == NEXUS_V1_SCREEN_TEXT_OK,
              "real FONT256.S2D screen-text receipts OK");
        CHECK(a.char_count == 256 && b.char_count == 256,
              "real FONT256.S2D screen-text char_count == 256");
        CHECK(a.writes > 0, "real FONT256.S2D screen-text writes > 0");
        CHECK(a.framebuffer_hash == b.framebuffer_hash,
              "real FONT256.S2D screen-text hash deterministic");
    }

    free(data);
}

int main(void)
{
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
    fprintf(stderr, "=== Nexus V1 S2D screen-text surface probe ===\n");

    run_synthetic_surface_gate();
    run_optional_real_asset_gate();

    fprintf(stderr, "\n# summary: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail > 0 ? 1 : 0;
}

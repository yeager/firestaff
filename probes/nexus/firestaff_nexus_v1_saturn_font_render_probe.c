/*
 * firestaff_nexus_v1_saturn_font_render_probe.c
 * =================================================
 *
 * Nexus V1 Saturn-font bitmap rendering probe.
 *
 * This is the bounded next step after the parser-only gate: it keeps the
 * data-free SCR fixture used by CI, expands one decoded 1bpp glyph into a
 * synthetic bitmap, draws it into an indexed framebuffer, and verifies
 * clipping, transparent/background behavior, deterministic framebuffer hash,
 * and invalid-input handling.
 *
 * The optional real-asset branch loads a local FONT256.S2D when present and
 * verifies parser-exposed glyph bytes can be rendered deterministically. It
 * does not claim full Saturn SCR section-table parity.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "nexus_v1_saturn_font.h"

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do {                                  \
    if (cond) { printf("  PASS: %s\n", msg); ++g_pass; }      \
    else      { printf("  FAIL: %s\n", msg); ++g_fail; }      \
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
    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return NULL;
    }
    size = ftell(fp);
    if (size <= 0) {
        fclose(fp);
        return NULL;
    }
    if (fseek(fp, 0, SEEK_SET) != 0) {
        fclose(fp);
        return NULL;
    }

    data = (uint8_t *)malloc((size_t)size);
    if (!data) {
        fclose(fp);
        return NULL;
    }
    if (fread(data, 1, (size_t)size, fp) != (size_t)size) {
        free(data);
        fclose(fp);
        return NULL;
    }
    fclose(fp);
    if (out_size) *out_size = size;
    return data;
}

static uint8_t *make_scr(int char_count, int glyph_bytes_per_char, int *out_size) {
    int glyph_bytes = char_count * glyph_bytes_per_char;
    int total = 48 + glyph_bytes;
    uint8_t *buf = (uint8_t *)calloc(1, (size_t)total);
    if (!buf) return NULL;

    memcpy(buf, "SEGA SATURN SCR", 15);
    buf[16] = 0;
    buf[17] = 0;
    buf[18] = (uint8_t)((char_count >> 8) & 0xFF);
    buf[19] = (uint8_t)(char_count & 0xFF);
    if (out_size) *out_size = total;
    return buf;
}

static void set_glyph_pixel(uint8_t *scr,
                            int glyph_index,
                            int glyph_bytes_per_char,
                            int x,
                            int y) {
    uint8_t *glyph = scr + 48 + glyph_index * glyph_bytes_per_char;
    uint8_t *row = glyph + y * 2;
    row[x / 8] |= (uint8_t)(0x80u >> (x & 7));
}

static uint8_t *make_cross_font(int *out_size) {
    uint8_t *scr = make_scr(4, 32, out_size);
    int y;
    if (!scr) return NULL;

    for (y = 0; y < 16; ++y) {
        set_glyph_pixel(scr, 2, 32, y, y);
        set_glyph_pixel(scr, 2, 32, 15 - y, y);
    }

    return scr;
}

static int count_bitmap_pixels(const uint8_t *bitmap, int width, int height, int stride) {
    int count = 0;
    int x, y;
    for (y = 0; y < height; ++y) {
        for (x = 0; x < width; ++x) {
            if (bitmap[y * stride + x]) ++count;
        }
    }
    return count;
}

static void run_synthetic_render_gate(void) {
    enum { FB_W = 20, FB_H = 20, FB_STRIDE = 24 };
    int sz;
    uint8_t *scr = make_cross_font(&sz);
    Nexus_V1_Font font;
    int rc;

    printf("\n-- synthetic SCR render gate --\n");
    CHECK(scr != NULL, "synthetic SCR fixture allocated");
    if (!scr) return;

    rc = nexus_v1_font_load(&font, scr, sz);
    CHECK(rc == 4, "synthetic font loads four glyphs");
    CHECK(font.char_width == 16 && font.char_height == 16,
          "synthetic glyph dimensions are 16x16");

    {
        uint8_t bitmap[16 * 18];
        int expand_rc;
        memset(bitmap, 0xEE, sizeof(bitmap));
        expand_rc = nexus_v1_font_expand_glyph_bitmap(&font, 2, bitmap, 16, 16, 18);
        CHECK(expand_rc == 1, "glyph expands into 0/1 bitmap with caller stride");
        CHECK(count_bitmap_pixels(bitmap, 16, 16, 18) == 32,
              "expanded cross glyph has 32 foreground pixels");
        CHECK(bitmap[0] == 1 && bitmap[15] == 1,
              "expanded top row keeps both diagonal endpoints");
        CHECK(bitmap[7 * 18 + 7] == 1 && bitmap[7 * 18 + 8] == 1,
              "expanded center rows preserve adjacent diagonal pixels");
        CHECK(bitmap[1] == 0 && bitmap[8 * 18 + 0] == 0,
              "expanded bitmap preserves transparent zero pixels");
    }

    {
        uint8_t fb[10 * 10];
        int writes;
        memset(fb, 7, sizeof(fb));
        writes = nexus_v1_font_draw_glyph_indexed(&font, fb, 10, 10, 10,
                                                  -4, -4, 2, 3, -1);
        CHECK(writes == 18, "transparent clipped draw writes only visible foreground pixels");
        CHECK(fb[0] == 3 && fb[9 * 10 + 9] == 3,
              "clipped draw reaches expected visible diagonal endpoints");
        CHECK(fb[1] == 7 && fb[9] == 7,
              "transparent zero pixels leave framebuffer background unchanged");
    }

    {
        uint8_t fb[FB_H * FB_STRIDE];
        int writes;
        uint64_t h;
        memset(fb, 7, sizeof(fb));
        writes = nexus_v1_font_draw_glyph_indexed(&font, fb, FB_W, FB_H, FB_STRIDE,
                                                  2, 2, 2, 5, 1);
        h = fnv1a64(fb, sizeof(fb));
        CHECK(writes == 256, "background draw writes the full unclipped glyph box");
        CHECK(fb[2 * FB_STRIDE + 2] == 5 && fb[2 * FB_STRIDE + 3] == 1,
              "foreground and background indices are both written");
        CHECK(fb[0] == 7 && fb[(FB_H - 1) * FB_STRIDE + (FB_W - 1)] == 7,
              "pixels outside the glyph box are preserved");
        CHECK(h == UINT64_C(0xcec17ab1a6b32d85),
              "indexed framebuffer hash is deterministic");
    }

    {
        uint8_t small[8 * 8];
        CHECK(nexus_v1_font_get_glyph_pixel(&font, 2, 0, 0) == 1,
              "glyph pixel reader returns set pixels");
        CHECK(nexus_v1_font_get_glyph_pixel(&font, 2, 1, 0) == 0,
              "glyph pixel reader returns clear pixels");
        CHECK(nexus_v1_font_get_glyph_pixel(&font, 2, -1, 0) == 0,
              "glyph pixel reader handles negative x");
        CHECK(nexus_v1_font_get_glyph_pixel(&font, 99, 0, 0) == 0,
              "glyph pixel reader handles out-of-range glyphs");
        CHECK(nexus_v1_font_expand_glyph_bitmap(NULL, 2, small, 8, 8, 8) == -1,
              "expand rejects NULL font");
        CHECK(nexus_v1_font_expand_glyph_bitmap(&font, 99, small, 8, 8, 8) == -1,
              "expand rejects out-of-range glyphs");
        CHECK(nexus_v1_font_draw_glyph_indexed(NULL, small, 8, 8, 8, 0, 0, 2, 1, -1) == -1,
              "draw rejects NULL font");
        CHECK(nexus_v1_font_draw_glyph_indexed(&font, NULL, 8, 8, 8, 0, 0, 2, 1, -1) == -1,
              "draw rejects NULL framebuffer");
        CHECK(nexus_v1_font_draw_glyph_indexed(&font, small, 8, 8, 7, 0, 0, 2, 1, -1) == -1,
              "draw rejects stride narrower than framebuffer width");
    }

    nexus_v1_font_free(&font);
    free(scr);

    {
        int tiny_sz;
        uint8_t *tiny = make_scr(4, 20, &tiny_sz);
        Nexus_V1_Font tiny_font;
        uint8_t tiny_out[8 * 8];
        if (tiny) {
            (void)nexus_v1_font_load(&tiny_font, tiny, tiny_sz);
            CHECK(tiny_font.char_width == 12 && tiny_font.char_height == 12,
                  "20 bytes/glyph remains parser-detected as 12x12");
            CHECK(nexus_v1_font_expand_glyph_bitmap(&tiny_font, 0, tiny_out, 8, 8, 8) == -1,
                  "expand rejects glyph payload shorter than inferred 12x12 1bpp bitmap");
            nexus_v1_font_free(&tiny_font);
            free(tiny);
        } else {
            CHECK(0, "undersized glyph fixture allocated");
        }
    }
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

    printf("\n-- optional real FONT256.S2D render gate --\n");
    data = read_entire_file(path, &size);
    if (!data) {
        printf("  SKIP: no local FONT256.S2D at %s\n", path ? path : "(unset)");
        return;
    }

    {
        Nexus_V1_Font font;
        uint8_t fb_a[32 * 32];
        uint8_t fb_b[32 * 32];
        int rc;
        int writes_a;
        int writes_b;
        uint64_t hash_a;
        uint64_t hash_b;

        rc = nexus_v1_font_load(&font, data, (int)size);
        CHECK(size == 25012, "local FONT256.S2D matches verified 25,012-byte asset size");
        CHECK(rc > 0, "local FONT256.S2D parser handoff succeeds");
        CHECK(font.char_count == 256, "local FONT256.S2D parser exposes 256 glyph slots");
        CHECK(font.char_width == 16 && font.char_height == 16,
              "local FONT256.S2D parser exposes 16x16 glyph dimensions");

        memset(fb_a, 0x44, sizeof(fb_a));
        memset(fb_b, 0x44, sizeof(fb_b));
        writes_a = nexus_v1_font_draw_glyph_indexed(&font, fb_a, 32, 32, 32,
                                                    8, 8, 0, 0xE0, 0x10);
        writes_b = nexus_v1_font_draw_glyph_indexed(&font, fb_b, 32, 32, 32,
                                                    8, 8, 0, 0xE0, 0x10);
        hash_a = fnv1a64(fb_a, sizeof(fb_a));
        hash_b = fnv1a64(fb_b, sizeof(fb_b));
        CHECK(writes_a == 256 && writes_b == 256,
              "local FONT256.S2D glyph draws into an indexed framebuffer");
        CHECK(hash_a == hash_b,
              "local FONT256.S2D parser-exposed glyph draw is deterministic");

        nexus_v1_font_free(&font);
    }

    free(data);
}

int main(void) {
    printf("=== Nexus V1 Saturn-font render probe ===\n");

    run_synthetic_render_gate();
    run_optional_real_asset_gate();

    printf("\n# summary: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail > 0 ? 1 : 0;
}

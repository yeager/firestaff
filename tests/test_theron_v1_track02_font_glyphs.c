#include <stdio.h>
#include <string.h>
#include "theron_v1_track02_font_glyphs.h"

static int g_pass, g_fail;
#define CHECK(msg, cond) do { \
    if (cond) { printf("  [PASS] %s\n", msg); g_pass++; } \
    else { printf("  [FAIL] %s\n", msg); g_fail++; } \
} while (0)

int main(void) {
    CHECK("glyph count is 120", theron_v1_track02_font_glyph_count() == 120);
    CHECK("glyph 0 (space) is all zeros",
          theron_v1_track02_font_glyph(0) != NULL &&
          memcmp(theron_v1_track02_font_glyph(0),
                 (uint8_t[]){0,0,0,0,0,0}, 6) == 0);
    CHECK("out-of-range returns NULL",
          theron_v1_track02_font_glyph(120) == NULL);

    /* UI decoration glyph 97 (box) */
    const uint8_t *box = theron_v1_track02_font_glyph(97);
    CHECK("UI box glyph exists", box != NULL);
    CHECK("UI box row 0 is 0xF8", box && box[0] == 0xF8);
    CHECK("UI box row 4 is 0xF8", box && box[4] == 0xF8);

    /* Verify '!' glyph (index 33) */
    const uint8_t *bang = theron_v1_track02_font_glyph(33);
    CHECK("'!' glyph exists", bang != NULL);
    CHECK("'!' row 0 is 0x20", bang && bang[0] == 0x20);
    CHECK("'!' row 3 is 0x00 (gap)", bang && bang[3] == 0x00);
    CHECK("'!' row 4 is 0x20 (dot)", bang && bang[4] == 0x20);

    /* Verify 'A' glyph (index 65) */
    const uint8_t *a_upper = theron_v1_track02_font_glyph(65);
    CHECK("'A' glyph exists", a_upper != NULL);
    CHECK("'A' row 0 is 0x70", a_upper && a_upper[0] == 0x70);
    CHECK("'A' row 2 is 0xF8 (crossbar)", a_upper && a_upper[2] == 0xF8);

    /* Verify lowercase 'a' (text alphabet index 1) */
    const uint8_t *a_lower = theron_v1_track02_font_glyph(1);
    CHECK("text-alphabet 'a' exists", a_lower != NULL);
    CHECK("text-alphabet 'a' row 0 is 0xF0", a_lower && a_lower[0] == 0xF0);

    /* Verify '0' digit (index 48) */
    const uint8_t *zero = theron_v1_track02_font_glyph(48);
    CHECK("'0' glyph exists", zero != NULL);
    CHECK("'0' row 0 is 0x70", zero && zero[0] == 0x70);

    printf("\nPASS: %d  FAIL: %d\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}

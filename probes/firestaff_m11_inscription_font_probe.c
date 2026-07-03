/*
 * firestaff_m11_inscription_font_probe.c
 *
 * DM1 V1 wall-inscription regression.  ReDMCSB DUNVIEW.C:3619 loads the
 * dedicated M648_GRAPHIC_INSCRIPTION_FONT for D1C inscriptions;
 * DUNVIEW.C:3627 centers with 112 - (characterCount << 2); and
 * DUNVIEW.C:3631-3637 blits 8x8 glyph cells with C10 transparency.
 *
 * This probe is intentionally asset-free: it guards the source constants and
 * glyph mapping used by m11_game_view.c before real GRAPHICS.DAT rendering.
 */

#include "dm1_v1_inscription_font_pc34_compat.h"
#include "font_m11.h"

#include <stdio.h>
#include <string.h>

static int g_failures = 0;

static void check_int(const char* label, int got, int expected) {
    if (got != expected) {
        printf("FAIL: %s got=%d expected=%d\n", label, got, expected);
        ++g_failures;
    } else {
        printf("PASS: %s = %d\n", label, got);
    }
}

static void check_true(const char* label, int condition) {
    if (!condition) {
        printf("FAIL: %s\n", label);
        ++g_failures;
    } else {
        printf("PASS: %s\n", label);
    }
}

static int inscription_draw_width(const char* text) {
    return DM1_V1_InscriptionTextWidth((int)strlen(text));
}

int main(void) {
    const char* samples[] = {
        "DUNGEON",
        "THE HALL",
        "SECRET DOOR",
        "DANGER.",
        "X"
    };
    int i;

    printf("=== DM1 V1 Inscription Font Probe ===\n");
    printf("Source: ReDMCSB DUNVIEW.C:3619-3638, COORD.C:1759, DEFS.H:2376/2395\n\n");

    check_int("inscription font GRAPHICS.DAT index",
              DM1_V1_INSCRIPTION_FONT_GRAPHIC_INDEX_PC34, 258);
    check_int("inscription font width",
              DM1_V1_INSCRIPTION_FONT_WIDTH_PC34, 288);
    check_int("inscription font height",
              DM1_V1_INSCRIPTION_FONT_HEIGHT_PC34, 8);
    check_int("inscription glyph width",
              DM1_V1_INSCRIPTION_GLYPH_WIDTH, 8);
    check_int("inscription glyph height",
              DM1_V1_INSCRIPTION_GLYPH_HEIGHT, 8);
    check_int("inscription transparent color",
              DM1_V1_INSCRIPTION_TRANSPARENT_COLOR, 10);
    check_int("inscription center x",
              DM1_V1_INSCRIPTION_CENTER_X, 112);

    check_true("wall inscription font is not generic M653 font",
               DM1_V1_INSCRIPTION_FONT_GRAPHIC_INDEX_PC34 !=
                   M11_FONT_GRAPHIC_INDEX_PC34);

    check_int("A maps to glyph 0",
              DM1_V1_InscriptionGlyphIndexFromAscii('A'), 0);
    check_int("Z maps to glyph 25",
              DM1_V1_InscriptionGlyphIndexFromAscii('Z'), 25);
    check_int("space maps to glyph 26",
              DM1_V1_InscriptionGlyphIndexFromAscii(' '), 26);
    check_int("period maps to glyph 27",
              DM1_V1_InscriptionGlyphIndexFromAscii('.'), 27);
    check_int("escape glyph 35 remains glyph 35",
              DM1_V1_InscriptionGlyphIndexFromAscii(35), 35);
    check_int("unsupported question mark is rejected",
              DM1_V1_InscriptionGlyphIndexFromAscii('?'), -1);

    for (i = 0; i < (int)(sizeof(samples) / sizeof(samples[0])); ++i) {
        const char* text = samples[i];
        int len = (int)strlen(text);
        int width = inscription_draw_width(text);
        int x = DM1_V1_InscriptionTextX(len);
        char label[128];

        snprintf(label, sizeof(label), "%s width", text);
        check_int(label, width, len * 8);
        snprintf(label, sizeof(label), "%s source x", text);
        check_int(label, x, 112 - (len << 2));
        snprintf(label, sizeof(label), "%s right edge source span", text);
        check_int(label, x + width - 1, 112 - (len << 2) + len * 8 - 1);
    }

    if (g_failures) {
        printf("\nFAIL: %d assertion(s)\n", g_failures);
        return 1;
    }
    printf("\nPASS: DM1 V1 wall inscriptions use source font 258 with 8-pixel centering and no generic font\n");
    return 0;
}

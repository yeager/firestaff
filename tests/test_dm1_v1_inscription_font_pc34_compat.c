#include "dm1_v1_inscription_font_pc34_compat.h"

#include <stdio.h>

static int g_tests;
static int g_passed;

static void check_int(const char* label, int got, int want)
{
    ++g_tests;
    if (got == want) {
        ++g_passed;
        printf("PASS %s=%d\n", label, got);
    } else {
        printf("FAIL %s got=%d want=%d\n", label, got, want);
    }
}

int main(void)
{
    const int compactWidth = DM1_V1_INSCRIPTION_FONT_WIDTH_PC34;
    const int realAsciiWidth = 128 * DM1_V1_INSCRIPTION_GLYPH_WIDTH;

    check_int("uppercase.A.compact",
              DM1_V1_InscriptionGlyphIndexForFontWidth('A', compactWidth),
              0);
    check_int("space.compact",
              DM1_V1_InscriptionGlyphIndexForFontWidth(' ', compactWidth),
              26);
    check_int("period.compact",
              DM1_V1_InscriptionGlyphIndexForFontWidth('.', compactWidth),
              27);
    check_int("escape.lowercase.real",
              DM1_V1_InscriptionGlyphIndexForFontWidth('a', realAsciiWidth),
              'a');
    check_int("escape.digit.real",
              DM1_V1_InscriptionGlyphIndexForFontWidth('0', realAsciiWidth),
              '0');
    check_int("escape.lowercase.compact.first",
              DM1_V1_InscriptionGlyphIndexForFontWidth('a', compactWidth),
              28);
    check_int("escape.lowercase.compact.last",
              DM1_V1_InscriptionGlyphIndexForFontWidth('h', compactWidth),
              35);
    check_int("escape.digit.compact.first",
              DM1_V1_InscriptionGlyphIndexForFontWidth('0', compactWidth),
              28);
    check_int("escape.digit.compact.last",
              DM1_V1_InscriptionGlyphIndexForFontWidth('7', compactWidth),
              35);
    check_int("escape.lowercase.compact.unavailable",
              DM1_V1_InscriptionGlyphIndexForFontWidth('i', compactWidth),
              -1);
    check_int("unsupported.question",
              DM1_V1_InscriptionGlyphIndexForFontWidth('?', realAsciiWidth),
              -1);

    printf("DM1 V1 inscription font glyph gate: %d/%d passed\n",
           g_passed, g_tests);
    return g_passed == g_tests ? 0 : 1;
}

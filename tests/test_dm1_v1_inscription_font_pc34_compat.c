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
    unsigned short textWords[2];
    struct DungeonThings_Compat things;
    struct DungeonTextString_Compat textStrings[2];
    unsigned short visibleWords[4];
    unsigned char glyphs[32];
    DM1_V1_InscriptionLinePlanPc34 linePlan;
    DM1_V1_InscriptionFrontWallLineDrawPlanPc34 drawPlan;
    int decodedLen;

    check_int("source.raw.A",
              DM1_V1_InscriptionGlyphIndexFromSourceByte(0),
              0);
    check_int("source.raw.Z",
              DM1_V1_InscriptionGlyphIndexFromSourceByte(25),
              25);
    check_int("source.raw.space",
              DM1_V1_InscriptionGlyphIndexFromSourceByte(26),
              26);
    check_int("source.raw.period",
              DM1_V1_InscriptionGlyphIndexFromSourceByte(27),
              27);
    check_int("source.raw.symbol.last",
              DM1_V1_InscriptionGlyphIndexFromSourceByte(35),
              35);
    check_int("source.raw.unsupported",
              DM1_V1_InscriptionGlyphIndexFromSourceByte(36),
              -1);
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
    check_int("decoded.lines.empty",
              DM1_V1_InscriptionDecodedLineCountPc34(""),
              0);
    check_int("decoded.lines.one",
              DM1_V1_InscriptionDecodedLineCountPc34("FUL YA"),
              1);
    check_int("decoded.lines.clamped",
              DM1_V1_InscriptionDecodedLineCountPc34("A\nB\nC\nD\nE"),
              DM1_V1_INSCRIPTION_MAX_LINES);

    textWords[0] = (unsigned short)((0 << 10) | (1 << 5) | 28);
    textWords[1] = (unsigned short)((29 << 10) | (2 << 5) | 31);
    decodedLen = DM1_V1_InscriptionDecodeRawGlyphsFromWordsPc34(
        textWords, 2, 0, glyphs, (int)sizeof(glyphs));
    check_int("decode.len", decodedLen, 8);
    check_int("decode.glyph.0", glyphs[0], 0);
    check_int("decode.glyph.1", glyphs[1], 1);
    check_int("decode.separator", glyphs[2], 0x80);
    check_int("decode.escape.T", glyphs[3], 19);
    check_int("decode.escape.H", glyphs[4], 7);
    check_int("decode.escape.E", glyphs[5], 4);
    check_int("decode.escape.space", glyphs[6], 26);
    check_int("decode.terminator", glyphs[7], 0x81);
    check_int("decode.bad.offset",
              DM1_V1_InscriptionDecodeRawGlyphsFromWordsPc34(
                  textWords, 2, 5, glyphs, (int)sizeof(glyphs)),
              0);
    (void)DM1_V1_InscriptionDecodeRawGlyphsFromWordsPc34(
        textWords, 2, 0, glyphs, (int)sizeof(glyphs));

    memset(&things, 0, sizeof(things));
    memset(textStrings, 0, sizeof(textStrings));
    visibleWords[0] = (unsigned short)((0 << 10) | (1 << 5) | 31);
    visibleWords[1] = (unsigned short)((2 << 10) | (3 << 5) | 31);
    textStrings[0].visible = 0;
    textStrings[0].textDataWordOffset = 0;
    textStrings[1].visible = 1;
    textStrings[1].textDataWordOffset = 1;
    things.textStrings = textStrings;
    things.textStringCount = 2;
    things.textData = visibleWords;
    things.textDataWordCount = 2;
    check_int("decode.visible.preferred",
              DM1_V1_InscriptionDecodeVisibleRawGlyphsPc34(
                  &things, 1, THING_ENDOFLIST, 0, 0, glyphs, (int)sizeof(glyphs)),
              1);
    check_int("decode.visible.preferred.glyph0", glyphs[0], 2);
    check_int("decode.visible.preferred.glyph1", glyphs[1], 3);
    check_int("decode.visible.hidden",
              DM1_V1_InscriptionDecodeVisibleRawGlyphsPc34(
                  &things, 0, THING_ENDOFLIST, 0, 0, glyphs, (int)sizeof(glyphs)),
              0);
    (void)DM1_V1_InscriptionDecodeRawGlyphsFromWordsPc34(
        textWords, 2, 0, glyphs, (int)sizeof(glyphs));

    check_int("unreadable.d3.side.1line",
              DM1_V1_InscriptionUnreadableBoxHeightPc34(3, -2, 1, 1),
              5);
    check_int("unreadable.d3.front.3line",
              DM1_V1_InscriptionUnreadableBoxHeightPc34(3, 0, 0, 3),
              20);
    check_int("unreadable.d2.side.2line",
              DM1_V1_InscriptionUnreadableBoxHeightPc34(2, 1, 1, 2),
              12);
    check_int("unreadable.d1.side.3line",
              DM1_V1_InscriptionUnreadableBoxHeightPc34(1, -1, 0, 3),
              33);
    check_int("unreadable.d1.front.none",
              DM1_V1_InscriptionUnreadableBoxHeightPc34(1, 0, 0, 2),
              0);

    check_int("line0.plan.ok",
              DM1_V1_InscriptionLinePlanFromRawGlyphsPc34(
                  glyphs, (int)sizeof(glyphs), 0, 0, &linePlan),
              1);
    check_int("line0.start", linePlan.glyphStart, 0);
    check_int("line0.count", linePlan.glyphCount, 2);
    check_int("line0.x", linePlan.textX, 104);
    check_int("line0.y", linePlan.textY, 41);
    check_int("line0.width", linePlan.textWidth, 16);
    check_int("line0.next", linePlan.nextCursor, 3);
    check_int("line0.done", linePlan.done, 0);
    check_int("line0.draw.ok",
              DM1_V1_InscriptionBuildFrontWallLineDrawPlanPc34(
                  glyphs, (int)sizeof(glyphs), 0, 0, 160, 111, &drawPlan),
              1);
    check_int("line0.draw.textX", drawPlan.textX, 104);
    check_int("line0.draw.textY", drawPlan.textY, 41);
    /* DUNVIEW.C F0107:3682 restores PC34 C735 once from its negative
     * D1C bitmap.  The line plan owns only the M648 source cell and its
     * destination zone; it must not invent a per-line backing patch. */
    check_int("line0.draw.shortWall.doesNotAlterM648Zone",
              DM1_V1_InscriptionBuildFrontWallLineDrawPlanPc34(
                  glyphs, (int)sizeof(glyphs), 0, 0, 80, 40, &drawPlan) &&
                  drawPlan.textX == 104 && drawPlan.textY == 41,
              1);
    check_int("line1.plan.ok",
              DM1_V1_InscriptionLinePlanFromRawGlyphsPc34(
                  glyphs, (int)sizeof(glyphs), linePlan.nextCursor, 1,
                  &linePlan),
              1);
    check_int("line1.start", linePlan.glyphStart, 3);
    check_int("line1.count", linePlan.glyphCount, 4);
    check_int("line1.x", linePlan.textX, 96);
    check_int("line1.y", linePlan.textY, 52);
    check_int("line1.width", linePlan.textWidth, 32);
    check_int("line1.done", linePlan.done, 1);
    check_int("font.supports.raw.line",
              DM1_V1_InscriptionRawGlyphLineSupportedByFontPc34(
                  glyphs,
                  2,
                  DM1_V1_INSCRIPTION_FONT_WIDTH_PC34,
                  DM1_V1_INSCRIPTION_FONT_HEIGHT_PC34),
              1);
    glyphs[0] = 36;
    check_int("font.rejects.unsupported.raw.glyph",
              DM1_V1_InscriptionRawGlyphLineSupportedByFontPc34(
                  glyphs,
                  1,
                  DM1_V1_INSCRIPTION_FONT_WIDTH_PC34,
                  DM1_V1_INSCRIPTION_FONT_HEIGHT_PC34),
              0);
    glyphs[0] = 0;
    check_int("font.rejects.short.width",
              DM1_V1_InscriptionRawGlyphLineSupportedByFontPc34(
                  glyphs,
                  1,
                  DM1_V1_INSCRIPTION_FONT_WIDTH_PC34 - 1,
                  DM1_V1_INSCRIPTION_FONT_HEIGHT_PC34),
              0);

    printf("DM1 V1 inscription font glyph gate: %d/%d passed\n",
           g_passed, g_tests);
    return g_passed == g_tests ? 0 : 1;
}

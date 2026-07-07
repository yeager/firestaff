#ifndef FIRESTAFF_DM1_V1_INSCRIPTION_FONT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_INSCRIPTION_FONT_PC34_COMPAT_H

/* DM1 PC34 wall inscriptions use a dedicated GRAPHICS.DAT font,
 * not the generic message/scroll font.  ReDMCSB DUNVIEW.C:3619 loads
 * M648_GRAPHIC_INSCRIPTION_FONT, DUNVIEW.C:3627 centers with count << 2,
 * and DUNVIEW.C:3631-3637 blits 8-pixel glyph cells with C10 transparency. */
#define DM1_V1_INSCRIPTION_FONT_GRAPHIC_INDEX_PC34 258
#define DM1_V1_INSCRIPTION_FONT_WIDTH_PC34 288
#define DM1_V1_INSCRIPTION_FONT_HEIGHT_PC34 8
#define DM1_V1_INSCRIPTION_GLYPH_WIDTH 8
#define DM1_V1_INSCRIPTION_GLYPH_HEIGHT 8
#define DM1_V1_INSCRIPTION_TRANSPARENT_COLOR 10
#define DM1_V1_INSCRIPTION_CENTER_X 112

static inline int DM1_V1_InscriptionGlyphIndexFromAscii(unsigned char ch) {
    if (ch >= 'A' && ch <= 'Z') {
        return (int)(ch - 'A');
    }
    if (ch == ' ') {
        return 26;
    }
    if (ch == '.') {
        return 27;
    }
    /* ReDMCSB DUNGEON.C:632-642 inscription escape replacements use glyph
     * indices 28..35 for special symbols. */
    if (ch >= 28 && ch <= 35) {
        return (int)ch;
    }
    return -1;
}

static inline int DM1_V1_InscriptionGlyphIndexForFontWidth(unsigned char ch,
                                                           int fontWidth) {
    int glyph = DM1_V1_InscriptionGlyphIndexFromAscii(ch);
    if (glyph >= 0) {
        return glyph;
    }
    /* ReDMCSB DUNGEON.C F0168 line ~2311 uses the G0256 escape-symbol
     * table directly, and DUNVIEW.C F0107 lines ~3631/~3704 blit M648 at
     * decodedByte << 3.  Real PC34 M648 has those ASCII-position symbol
     * cells; small synthetic fixtures may only expose compact 0..35 cells. */
    if ((ch >= 'a' && ch <= 'x') || (ch >= '0' && ch <= '7')) {
        int asciiGlyph = (int)ch;
        if (fontWidth >= (asciiGlyph + 1) * DM1_V1_INSCRIPTION_GLYPH_WIDTH) {
            return asciiGlyph;
        }
        if (ch >= 'a' && ch <= 'h') {
            return 28 + (int)(ch - 'a');
        }
        if (ch >= '0' && ch <= '7') {
            return 28 + (int)(ch - '0');
        }
    }
    return -1;
}

static inline int DM1_V1_InscriptionTextWidth(int characterCount) {
    return characterCount > 0
        ? characterCount * DM1_V1_INSCRIPTION_GLYPH_WIDTH
        : 0;
}

static inline int DM1_V1_InscriptionTextX(int characterCount) {
    return DM1_V1_INSCRIPTION_CENTER_X - (DM1_V1_InscriptionTextWidth(characterCount) / 2);
}

#endif /* FIRESTAFF_DM1_V1_INSCRIPTION_FONT_PC34_COMPAT_H */

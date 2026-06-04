#ifndef FIRESTAFF_DM1_V1_INSCRIPTION_FONT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_INSCRIPTION_FONT_PC34_COMPAT_H

/* DM1 PC34 readable wall inscriptions use a dedicated GRAPHICS.DAT font,
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
    if (ch >= 'a' && ch <= 'z') {
        return (int)(ch - 'a');
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

static inline int DM1_V1_InscriptionTextWidth(int characterCount) {
    return characterCount > 0
        ? characterCount * DM1_V1_INSCRIPTION_GLYPH_WIDTH
        : 0;
}

static inline int DM1_V1_InscriptionTextX(int characterCount) {
    return DM1_V1_INSCRIPTION_CENTER_X - (DM1_V1_InscriptionTextWidth(characterCount) / 2);
}

#endif /* FIRESTAFF_DM1_V1_INSCRIPTION_FONT_PC34_COMPAT_H */

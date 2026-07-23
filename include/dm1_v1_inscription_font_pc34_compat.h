#ifndef FIRESTAFF_DM1_V1_INSCRIPTION_FONT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_INSCRIPTION_FONT_PC34_COMPAT_H

/* DM1 PC34 wall inscriptions use a dedicated GRAPHICS.DAT font,
 * not the generic message/scroll font.  ReDMCSB DUNVIEW.C:3619 loads
 * M648_GRAPHIC_INSCRIPTION_FONT, DUNVIEW.C:3627 centers with count << 2,
 * and DUNVIEW.C:3631-3637 blits 8-pixel glyph cells with C10 transparency. */
/* `M648` is the ReDMCSB runtime pointer, not a direct G0018 value.  The
 * mandatory-load table's 120 is an internal preload slot; DUNVIEW.C F0107
 * reads the actual PC 3.4 GRAPHICS.DAT bitmap at ordinal 258. */
#define DM1_V1_INSCRIPTION_FONT_GRAPHIC_INDEX_PC34 258
#define DM1_V1_INSCRIPTION_FONT_WIDTH_PC34 288
#define DM1_V1_INSCRIPTION_FONT_HEIGHT_PC34 8
#define DM1_V1_INSCRIPTION_GLYPH_WIDTH 8
#define DM1_V1_INSCRIPTION_GLYPH_HEIGHT 8
#define DM1_V1_INSCRIPTION_TRANSPARENT_COLOR 10
#define DM1_V1_INSCRIPTION_CENTER_X 112
#define DM1_V1_INSCRIPTION_MAX_LINES 4

/* DUNVIEW.C F0107 places the baseline of each possible front-wall line at
 * these four distinct rows. Keep the positions source-owned rather than
 * letting a caller derive them from a host font's line height. */
static inline int DM1_V1_InscriptionFrontWallLineTextYPc34(int line) {
    static const int kLineBottomY[DM1_V1_INSCRIPTION_MAX_LINES] = {
        48, 59, 75, 86
    };
    if (line < 0 || line >= DM1_V1_INSCRIPTION_MAX_LINES) {
        return -1;
    }
    return kLineBottomY[line] - 7;
}

typedef struct DM1_V1_InscriptionLinePlanPc34 {
    int glyphStart;
    int glyphCount;
    int textX;
    int textY;
    int textWidth;
    int nextCursor;
    int done;
} DM1_V1_InscriptionLinePlanPc34;

static inline int DM1_V1_InscriptionGlyphIndexFromSourceByte(unsigned char ch) {
    /* ReDMCSB DUNVIEW.C F0107 lines ~3631/~3704 blit decoded inscription
     * bytes directly at source_x = decodedByte << 3.  Valid PC34 M648 cells
     * are 0..35: A..Z, space, period, and eight symbol cells. */
    return ch <= 35 ? (int)ch : -1;
}

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

static inline int DM1_V1_InscriptionDecodedLineCountPc34(const char* decoded) {
    int lines = 1;
    const char* p;
    if (!decoded || !decoded[0]) {
        return 0;
    }
    for (p = decoded; *p && lines < DM1_V1_INSCRIPTION_MAX_LINES; ++p) {
        if (*p == '\n') {
            ++lines;
        }
    }
    return lines;
}

static inline int DM1_V1_InscriptionRawGlyphLineSupportedByFontPc34(
        const unsigned char* glyphs,
        int glyphCount,
        int fontWidth,
        int fontHeight) {
    int i;
    if (!glyphs || glyphCount <= 0 ||
        fontWidth < DM1_V1_INSCRIPTION_FONT_WIDTH_PC34 ||
        fontHeight < DM1_V1_INSCRIPTION_FONT_HEIGHT_PC34) {
        return 0;
    }
    for (i = 0; i < glyphCount; ++i) {
        int glyph = DM1_V1_InscriptionGlyphIndexFromSourceByte(glyphs[i]);
        if (glyph < 0 ||
            (glyph + 1) * DM1_V1_INSCRIPTION_GLYPH_WIDTH > fontWidth) {
            return 0;
        }
    }
    return 1;
}

static inline int DM1_V1_InscriptionAppendRawGlyphPc34(unsigned char* outGlyphs,
                                                       int outGlyphCapacity,
                                                       int* ioPos,
                                                       int glyph) {
    if (!outGlyphs || !ioPos || glyph < 0 || glyph > 35) {
        return 0;
    }
    if (*ioPos >= outGlyphCapacity - 1) {
        return 0;
    }
    outGlyphs[(*ioPos)++] = (unsigned char)glyph;
    return 1;
}

static inline void DM1_V1_InscriptionAppendEscape29RawPc34(
        unsigned char* outGlyphs,
        int outGlyphCapacity,
        int* ioPos,
        int code) {
    static const unsigned char kThe[] = {19, 7, 4, 26};
    static const unsigned char kYou[] = {24, 14, 20, 26};
    const unsigned char* seq = 0;
    int seqLen = 0;
    int i;
    if (code == 0 || code == 1) {
        (void)DM1_V1_InscriptionAppendRawGlyphPc34(
            outGlyphs, outGlyphCapacity, ioPos, 28 + code);
        return;
    }
    if (code == 2) {
        seq = kThe;
        seqLen = (int)(sizeof(kThe) / sizeof(kThe[0]));
    } else if (code == 3) {
        seq = kYou;
        seqLen = (int)(sizeof(kYou) / sizeof(kYou[0]));
    } else if (code >= 4 && code <= 9) {
        (void)DM1_V1_InscriptionAppendRawGlyphPc34(
            outGlyphs, outGlyphCapacity, ioPos, 26 + code);
        return;
    }
    for (i = 0; i < seqLen; ++i) {
        (void)DM1_V1_InscriptionAppendRawGlyphPc34(
            outGlyphs, outGlyphCapacity, ioPos, seq[i]);
    }
}

static inline void DM1_V1_InscriptionAppendEscape30RawPc34(
        unsigned char* outGlyphs,
        int outGlyphCapacity,
        int* ioPos,
        int code) {
    static const unsigned char kThe[] = {19, 7, 4, 26};
    static const unsigned char kYou[] = {24, 14, 20, 26};
    const unsigned char* seq = 0;
    int seqLen = 0;
    int i;
    if (code == 2) {
        seq = kThe;
        seqLen = (int)(sizeof(kThe) / sizeof(kThe[0]));
    } else if (code == 3) {
        seq = kYou;
        seqLen = (int)(sizeof(kYou) / sizeof(kYou[0]));
    }
    for (i = 0; i < seqLen; ++i) {
        (void)DM1_V1_InscriptionAppendRawGlyphPc34(
            outGlyphs, outGlyphCapacity, ioPos, seq[i]);
    }
}

static inline int DM1_V1_InscriptionDecodeRawGlyphsFromWordsPc34(
        const unsigned short* textData,
        int textDataWordCount,
        int wordOffset,
        unsigned char* outGlyphs,
        int outGlyphCapacity) {
    int wi;
    int codeIdx = 0;
    int pos = 0;
    int escape = 0;
    unsigned short w = 0;
    if (!textData || wordOffset < 0 ||
        wordOffset >= textDataWordCount ||
        !outGlyphs || outGlyphCapacity < 2) {
        if (outGlyphs && outGlyphCapacity > 0) {
            outGlyphs[0] = 0x81U;
        }
        return 0;
    }
    wi = wordOffset;
    while (pos < outGlyphCapacity - 1 && wi < textDataWordCount) {
        int code;
        if (codeIdx == 0) {
            w = textData[wi];
            code = (w >> 10) & 0x1F;
        } else if (codeIdx == 1) {
            code = (w >> 5) & 0x1F;
        } else {
            code = w & 0x1F;
        }
        ++codeIdx;
        if (codeIdx >= 3) {
            codeIdx = 0;
            ++wi;
        }
        if (escape == 29) {
            DM1_V1_InscriptionAppendEscape29RawPc34(
                outGlyphs, outGlyphCapacity, &pos, code);
            escape = 0;
        } else if (escape == 30) {
            DM1_V1_InscriptionAppendEscape30RawPc34(
                outGlyphs, outGlyphCapacity, &pos, code);
            escape = 0;
        } else if (code < 28) {
            (void)DM1_V1_InscriptionAppendRawGlyphPc34(
                outGlyphs, outGlyphCapacity, &pos, code);
        } else if (code == 28) {
            if (pos < outGlyphCapacity - 1) {
                outGlyphs[pos++] = 0x80U;
            }
        } else if (code == 29 || code == 30) {
            escape = code;
        } else {
            break;
        }
    }
    if (pos < outGlyphCapacity) {
        outGlyphs[pos++] = 0x81U;
    }
    if (pos < outGlyphCapacity) {
        outGlyphs[pos] = 0;
    }
    return pos;
}

static inline int DM1_V1_InscriptionUnreadableBoxHeightPc34(
        int relForward,
        int relSide,
        int sideProjection,
        int lineCount) {
    static const unsigned char kUnreadableBoxHeight[5][3] = {
        {5, 8, 13},
        {7, 13, 20},
        {5, 12, 19},
        {10, 17, 27},
        {11, 22, 33}
    };
    int row = -1;
    if (lineCount <= 0 || lineCount >= 4) {
        return 0;
    }
    if (relForward == 3) {
        row = (relSide != 0 && sideProjection) ? 0 : 1;
    } else if (relForward == 2) {
        row = (relSide != 0 && sideProjection) ? 2 : 3;
    } else if (relForward == 1 && relSide != 0) {
        row = 4;
    }
    if (row < 0) {
        return 0;
    }
    return (int)kUnreadableBoxHeight[row][lineCount - 1];
}

static inline int DM1_V1_InscriptionLinePlanFromRawGlyphsPc34(
        const unsigned char* glyphs,
        int glyphCapacity,
        int cursor,
        int line,
        DM1_V1_InscriptionLinePlanPc34* outPlan) {
    int start;
    int glyphCount;
    if (!glyphs || !outPlan || glyphCapacity <= 0 ||
        cursor < 0 || cursor >= glyphCapacity ||
        line < 0 || line >= DM1_V1_INSCRIPTION_MAX_LINES) {
        return 0;
    }
    start = cursor;
    while (cursor < glyphCapacity &&
           glyphs[cursor] != 0x80U &&
           glyphs[cursor] != 0x81U) {
        ++cursor;
    }
    glyphCount = cursor - start;
    outPlan->glyphStart = start;
    outPlan->glyphCount = glyphCount;
    outPlan->textWidth = DM1_V1_InscriptionTextWidth(glyphCount);
    outPlan->textX = DM1_V1_InscriptionTextX(glyphCount);
    outPlan->textY = DM1_V1_InscriptionFrontWallLineTextYPc34(line);
    outPlan->done = (cursor >= glyphCapacity || glyphs[cursor] == 0x81U);
    outPlan->nextCursor = outPlan->done ? cursor : cursor + 1;
    return 1;
}

#endif /* FIRESTAFF_DM1_V1_INSCRIPTION_FONT_PC34_COMPAT_H */

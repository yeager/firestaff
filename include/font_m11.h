#ifndef FIRESTAFF_FONT_M11_H
#define FIRESTAFF_FONT_M11_H

/*
 * font_m11 — Original DM1 font rendering from GRAPHICS.DAT.
 *
 * Loads the original interface/scroll font graphic from GRAPHICS.DAT
 * as a 1-bit-per-pixel bitmap and renders text using the original DM1
 * character metrics. The concrete graphic index is media-dependent in
 * the ReDMCSB headers (`M653_GRAPHIC_FONT` resolves to 557 or 695 in the
 * local source dump), so the loader probes known source-backed
 * candidates instead of assuming a single literal slot number.
 *
 *   Font bitmap: 1024 x 6 pixels, 1bpp (768 bytes)
 *   Character cell: 8 pixels wide in bitmap (128 characters)
 *   Visible width: 6 pixels (G2087 = 6)
 *   Visible height: 6 pixels (G2083 = 6)
 *   Line height: 7 pixels (G2088 = 7)
 *   X offset: char_code * 8 + 3 (8 - G2082, where G2082 = 5)
 *
 * Falls back to the builtin hardcoded font when GRAPHICS.DAT
 * is unavailable or the font graphic cannot be loaded.
 */

#ifdef __cplusplus
extern "C" {
#endif

#define M11_FONT_BITMAP_WIDTH      1024
#define M11_FONT_BITMAP_HEIGHT     6
#define M11_FONT_BITMAP_BYTES      768
#define M11_FONT_CHAR_CELL_WIDTH   8
#define M11_FONT_CHAR_VISIBLE_W    6
#define M11_FONT_CHAR_VISIBLE_H    6
#define M11_FONT_LINE_HEIGHT       7
#define M11_FONT_X_OFFSET          3

#define M11_FONT_GRAPHIC_INDEX_PC34     695
#define M11_FONT_GRAPHIC_INDEX_LEGACY   557
#define M11_FONT_GRAPHIC_INDEX_FALLBACK 653
#define M11_FONT_GRAPHIC_CANDIDATE_COUNT 3

extern const int g_m11_font_graphic_candidates[M11_FONT_GRAPHIC_CANDIDATE_COUNT];

typedef struct {
    int loaded;
    int graphicIndex;
    unsigned char bitmap[M11_FONT_BITMAP_BYTES];
} M11_FontState;

void M11_Font_Init(M11_FontState* font);
int M11_Font_LoadFromGraphicsDat(
    M11_FontState* font,
    void* fileState,
    void* runtimeState);
int M11_Font_IsLoaded(const M11_FontState* font);
int M11_Font_FindGraphicIndex(void* runtimeState);
int M11_Font_ResolvedGraphicIndex(const M11_FontState* font);
int M11_Font_GetPixel(const M11_FontState* font, int x, int y);
int M11_Font_DrawChar(
    const M11_FontState* font,
    unsigned char* framebuffer,
    int fbWidth,
    int fbHeight,
    int dstX,
    int dstY,
    unsigned char ch,
    unsigned char fgColor,
    int bgColor,
    int scale
);
int M11_Font_DrawString(
    const M11_FontState* font,
    unsigned char* framebuffer,
    int fbWidth,
    int fbHeight,
    int dstX,
    int dstY,
    const char* text,
    unsigned char fgColor,
    int bgColor,
    int scale
);
int M11_Font_MeasureString(const char* text);

#ifdef __cplusplus
}
#endif

#endif

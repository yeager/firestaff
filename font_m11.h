#ifndef FIRESTAFF_FONT_M11_H
#define FIRESTAFF_FONT_M11_H

/*
 * font_m11 — Original DM1 font rendering from GRAPHICS.DAT.
 *
 * Loads graphic #653 (the interface font) as a 1-bit-per-pixel
 * bitmap and renders text using the original DM1 character metrics:
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

/* Original DM1 font constants */
#define M11_FONT_BITMAP_WIDTH      1024
#define M11_FONT_BITMAP_HEIGHT     6
#define M11_FONT_BITMAP_BYTES      768   /* 128 bytes/row * 6 rows */
#define M11_FONT_CHAR_CELL_WIDTH   8     /* pixels per character cell */
#define M11_FONT_CHAR_VISIBLE_W    6     /* G2087: rendered width */
#define M11_FONT_CHAR_VISIBLE_H    6     /* G2083: rendered height */
#define M11_FONT_LINE_HEIGHT       7     /* G2088: line spacing */
#define M11_FONT_X_OFFSET          3     /* 8 - G2082(5) = 3 */
#define M11_FONT_GRAPHIC_INDEX     653   /* M653_GRAPHIC_FONT */

typedef struct {
    int loaded;
    unsigned char bitmap[M11_FONT_BITMAP_BYTES];
} M11_FontState;

/* Initialize font state (zeroed, not loaded). */
void M11_Font_Init(M11_FontState* font);

/* Attempt to load the font from GRAPHICS.DAT via the M10 file reader.
 * Uses the same file/runtime state as the asset loader.
 * Returns 1 on success, 0 on failure. */
int M11_Font_LoadFromGraphicsDat(
    M11_FontState* font,
    void* fileState,       /* MemoryGraphicsDatState_Compat* */
    void* runtimeState     /* MemoryGraphicsDatRuntimeState_Compat* */
);

/* Check whether the font is loaded and ready. */
int M11_Font_IsLoaded(const M11_FontState* font);

/* Read a single pixel from the 1bpp font bitmap.
 * Returns 1 (foreground) or 0 (background). */
int M11_Font_GetPixel(const M11_FontState* font, int x, int y);

/* Draw a single character onto a 1-byte-per-pixel framebuffer.
 * ch: ASCII character code
 * fgColor: VGA palette index for foreground
 * bgColor: VGA palette index for background, or -1 for transparent bg
 * scale: integer scale factor (1 = original size)
 * Returns the horizontal advance in pixels (scaled). */
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

/* Draw a string onto a 1-byte-per-pixel framebuffer.
 * Returns the total width drawn in pixels. */
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

/* Measure the width of a string in pixels (before scaling). */
int M11_Font_MeasureString(const char* text);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_FONT_M11_H */

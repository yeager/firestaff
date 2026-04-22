/*
 * font_m11.c — Original DM1 font rendering from GRAPHICS.DAT graphic #653.
 *
 * The font is stored as a 1-bit-per-pixel bitmap (768 bytes) in
 * GRAPHICS.DAT. Unlike normal 4bpp graphics, the font entry uses
 * the NOT_EXPANDED flag in the original code, meaning the raw
 * decompressed file data IS the 1bpp bitmap.
 *
 * Layout: 1024 pixels wide x 6 pixels tall, MSB-first bit ordering.
 * Each character occupies an 8-pixel-wide cell. The visible portion
 * starts at offset 3 within each cell (8 - G2082 where G2082=5).
 */

#include "font_m11.h"
#include "memory_graphics_dat_pc34_compat.h"
#include "memory_graphics_dat_state_pc34_compat.h"
#include "memory_graphics_dat_select_pc34_compat.h"
#include "memory_graphics_dat_header_pc34_compat.h"
#include "memory_frontend_pc34_compat.h"

#include <string.h>
#include <stdlib.h>

void M11_Font_Init(M11_FontState* font) {
    if (!font) return;
    memset(font, 0, sizeof(*font));
}

int M11_Font_LoadFromGraphicsDat(
    M11_FontState* font,
    void* fileState,
    void* runtimeState)
{
    struct MemoryGraphicsDatState_Compat* fs;
    struct MemoryGraphicsDatRuntimeState_Compat* rt;
    struct MemoryGraphicsDatHeader_Compat tempHeader;
    struct MemoryGraphicsDatSelection_Compat selection;
    unsigned char* rawBuf;
    long offset;
    int readSize;

    if (!font || !fileState || !runtimeState) {
        return 0;
    }

    fs = (struct MemoryGraphicsDatState_Compat*)fileState;
    rt = (struct MemoryGraphicsDatRuntimeState_Compat*)runtimeState;

    if (M11_FONT_GRAPHIC_INDEX >= rt->graphicCount) {
        return 0;
    }

    /* Build temporary header to get file offset */
    memset(&tempHeader, 0, sizeof(tempHeader));
    tempHeader.format = rt->format;
    tempHeader.graphicCount = rt->graphicCount;
    tempHeader.compressedByteCounts = rt->compressedByteCounts;
    tempHeader.decompressedByteCounts = rt->decompressedByteCounts;
    tempHeader.widthHeight = rt->widthHeight;
    tempHeader.fileSize = rt->fileSize;

    memset(&selection, 0, sizeof(selection));
    if (!F0490_MEMORY_SelectGraphicFromHeader_Compat(
            &tempHeader, M11_FONT_GRAPHIC_INDEX, &selection)) {
        return 0;
    }

    offset = selection.offset;
    readSize = selection.compressedByteCount;

    /* The font at 1bpp should be exactly 768 bytes.
     * Accept sizes from 768 to ~1024 to tolerate minor format variation.
     * If compressed size is significantly different, the entry might be
     * IMG3-compressed — try to handle that too. */
    if (readSize <= 0 || readSize > 4096) {
        return 0;
    }

    rawBuf = (unsigned char*)calloc((size_t)readSize + 16, 1);
    if (!rawBuf) {
        return 0;
    }

    if (!F0474_MEMORY_LoadGraphic_CPSDF_Compat(offset, readSize, fs, rawBuf)) {
        free(rawBuf);
        return 0;
    }

    if (readSize == M11_FONT_BITMAP_BYTES) {
        /* Perfect match: raw 1bpp font data */
        memcpy(font->bitmap, rawBuf, M11_FONT_BITMAP_BYTES);
        font->loaded = 1;
        free(rawBuf);
        return 1;
    }

    /* If the raw data is larger, it might be IMG3-compressed.
     * Check for IMG3 header: first 4 bytes are width (LE) and height (LE).
     * For the font, we'd expect width near 1024 and height near 6.
     * If we find that, decompress via IMG3 and convert 4bpp to 1bpp. */
    if (readSize >= 8) {
        unsigned short hdrW = (unsigned short)(rawBuf[0] | (rawBuf[1] << 8));
        unsigned short hdrH = (unsigned short)(rawBuf[2] | (rawBuf[3] << 8));

        if (hdrW == M11_FONT_BITMAP_WIDTH && hdrH == M11_FONT_BITMAP_HEIGHT) {
            /* This looks like an IMG3 stream for a 1024x6 bitmap.
             * Decompress via the existing expand pipeline and convert
             * the resulting 4bpp data to 1bpp. */
            unsigned long packedSize =
                (unsigned long)((hdrW + 1) / 2) * (unsigned long)hdrH;
            unsigned char* packedBuf = (unsigned char*)calloc(packedSize + 64, 1);
            if (packedBuf) {
                struct GraphicWidthHeight_Compat sizeInfo;
                unsigned char* bitmapBase = packedBuf + 4;
                unsigned long ix, iy;
                int byteIdx, bitPos;

                sizeInfo.Width = hdrW;
                sizeInfo.Height = hdrH;
                F0488_MEMORY_ExpandGraphicToBitmap_Compat(rawBuf, bitmapBase, &sizeInfo);

                /* Convert 4bpp packed nibbles to 1bpp:
                 * nibble value 0 = bg (bit 0), nonzero = fg (bit 1) */
                memset(font->bitmap, 0, M11_FONT_BITMAP_BYTES);
                for (iy = 0; iy < (unsigned long)hdrH; iy++) {
                    for (ix = 0; ix < (unsigned long)hdrW; ix++) {
                        unsigned long nibbleIdx = iy * (unsigned long)hdrW + ix;
                        unsigned long packedByteIdx = nibbleIdx / 2;
                        unsigned char packed = bitmapBase[packedByteIdx];
                        unsigned char pixel;
                        if ((nibbleIdx & 1) == 0) {
                            pixel = (packed >> 4) & 0x0F;
                        } else {
                            pixel = packed & 0x0F;
                        }
                        if (pixel != 0) {
                            byteIdx = (int)(iy * 128 + ix / 8);
                            bitPos = 7 - (int)(ix % 8);
                            if (byteIdx >= 0 && byteIdx < M11_FONT_BITMAP_BYTES) {
                                font->bitmap[byteIdx] |=
                                    (unsigned char)(1 << bitPos);
                            }
                        }
                    }
                }
                free(packedBuf);
                font->loaded = 1;
                free(rawBuf);
                return 1;
            }
        }
    }

    /* Last resort: if compressed data is close to 768 bytes, try using
     * what we have (may contain minor padding/header). */
    if (readSize >= M11_FONT_BITMAP_BYTES) {
        /* Skip potential header bytes if present */
        int headerSkip = readSize - M11_FONT_BITMAP_BYTES;
        if (headerSkip <= 8) {
            memcpy(font->bitmap, rawBuf + headerSkip, M11_FONT_BITMAP_BYTES);
            font->loaded = 1;
            free(rawBuf);
            return 1;
        }
    }

    free(rawBuf);
    return 0;
}

int M11_Font_IsLoaded(const M11_FontState* font) {
    return font && font->loaded;
}

int M11_Font_GetPixel(const M11_FontState* font, int x, int y) {
    int byteIdx, bitPos;
    if (!font || !font->loaded) return 0;
    if (x < 0 || x >= M11_FONT_BITMAP_WIDTH) return 0;
    if (y < 0 || y >= M11_FONT_BITMAP_HEIGHT) return 0;
    byteIdx = y * 128 + x / 8;  /* 128 bytes per row = 1024/8 */
    bitPos = 7 - (x % 8);       /* MSB first */
    return (font->bitmap[byteIdx] >> bitPos) & 1;
}

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
    int scale)
{
    int fontX, row, col, sx, sy, px, py;
    int advance;

    if (!font || !font->loaded || !framebuffer || scale < 1) {
        return 0;
    }

    /* Original DM1 formula: fontX = charCode * 8 + (8 - G2082)
     * G2082 = 5, so offset = 3 */
    fontX = (int)ch * M11_FONT_CHAR_CELL_WIDTH + M11_FONT_X_OFFSET;
    advance = M11_FONT_CHAR_VISIBLE_W * scale;

    for (row = 0; row < M11_FONT_CHAR_VISIBLE_H; row++) {
        for (col = 0; col < M11_FONT_CHAR_VISIBLE_W; col++) {
            int pixel = M11_Font_GetPixel(font, fontX + col, row);
            for (sy = 0; sy < scale; sy++) {
                for (sx = 0; sx < scale; sx++) {
                    px = dstX + col * scale + sx;
                    py = dstY + row * scale + sy;
                    if (px < 0 || px >= fbWidth) continue;
                    if (py < 0 || py >= fbHeight) continue;
                    if (pixel) {
                        framebuffer[py * fbWidth + px] = fgColor;
                    } else if (bgColor >= 0) {
                        framebuffer[py * fbWidth + px] =
                            (unsigned char)bgColor;
                    }
                }
            }
        }
    }

    return advance;
}

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
    int scale)
{
    int totalWidth = 0;

    if (!font || !font->loaded || !text) return 0;
    if (scale < 1) scale = 1;

    while (*text) {
        unsigned char ch = (unsigned char)*text;
        /* Handle newline: move down one line, reset X */
        if (ch == '\n') {
            dstY += M11_FONT_LINE_HEIGHT * scale;
            dstX -= totalWidth; /* reset to original X */
            totalWidth = 0;
            text++;
            continue;
        }
        int advance = M11_Font_DrawChar(
            font, framebuffer, fbWidth, fbHeight,
            dstX + totalWidth, dstY, ch, fgColor, bgColor, scale);
        totalWidth += advance;
        text++;
    }

    return totalWidth;
}

int M11_Font_MeasureString(const char* text) {
    int width = 0;
    if (!text) return 0;
    while (*text) {
        if (*text != '\n') {
            width += M11_FONT_CHAR_VISIBLE_W;
        }
        text++;
    }
    return width;
}

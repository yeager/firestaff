#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include <string.h>
#include "screen_bitmap_present_pc34_compat.h"
#include "bitmap_copy_pc34_compat.h"

#define SCREEN_PRESENT_COMPAT_WIDTH 320
#define SCREEN_PRESENT_COMPAT_HEIGHT 200

static unsigned short screen_present_read_u16_le(const unsigned char* p) {
        return (unsigned short)(p[0] | ((unsigned short)p[1] << 8));
}

static void screen_present_write_u16_le(unsigned char* p, unsigned short value) {
        p[0] = (unsigned char)(value & 0xFF);
        p[1] = (unsigned char)((value >> 8) & 0xFF);
}

static unsigned long screen_present_byte_count(const unsigned char* bitmap) {
        unsigned short width;
        unsigned short height;
        unsigned short bytesPerRow;


        width = screen_present_read_u16_le(bitmap - 4);
        height = screen_present_read_u16_le(bitmap - 2);
        bytesPerRow = (unsigned short)(((width + 1) & 0xFFFE) >> 1);
        return (unsigned long)bytesPerRow * (unsigned long)height;
}

static unsigned char screen_present_get_nibble(const unsigned char* bitmap, unsigned short x, unsigned short y) {
        unsigned short width;
        unsigned short bytesPerRow;
        const unsigned char* row;
        unsigned char packed;


        width = screen_present_read_u16_le(bitmap - 4);
        bytesPerRow = (unsigned short)(((width + 1) & 0xFFFE) >> 1);
        row = bitmap + ((unsigned long)y * (unsigned long)bytesPerRow);
        packed = row[x >> 1];
        if (x & 1) {
                return packed & 0x0F;
        }
        return packed >> 4;
}

static void screen_present_set_nibble(unsigned char* bitmap, unsigned short x, unsigned short y, unsigned char color) {
        unsigned short width;
        unsigned short bytesPerRow;
        unsigned char* row;
        unsigned char packed;


        width = screen_present_read_u16_le(bitmap - 4);
        bytesPerRow = (unsigned short)(((width + 1) & 0xFFFE) >> 1);
        row = bitmap + ((unsigned long)y * (unsigned long)bytesPerRow);
        packed = row[x >> 1];
        color &= 0x0F;
        if (x & 1) {
                row[x >> 1] = (unsigned char)((packed & 0xF0) | color);
        } else {
                row[x >> 1] = (unsigned char)((packed & 0x0F) | (unsigned char)(color << 4));
        }
}

static void screen_present_init_blank_screen(unsigned char* screenBitmap) {
        unsigned short bytesPerRow;
        unsigned long totalBytes;


        bytesPerRow = (unsigned short)(((SCREEN_PRESENT_COMPAT_WIDTH + 1) & 0xFFFE) >> 1);
        totalBytes = (unsigned long)bytesPerRow * (unsigned long)SCREEN_PRESENT_COMPAT_HEIGHT;
        screen_present_write_u16_le(screenBitmap - 4, SCREEN_PRESENT_COMPAT_WIDTH);
        screen_present_write_u16_le(screenBitmap - 2, SCREEN_PRESENT_COMPAT_HEIGHT);
        memset(screenBitmap, 0, (size_t)totalBytes);
}

static int screen_present_overlay_bitmap(
const unsigned char*                      sourceBitmap     SEPARATOR
unsigned char*                            screenBitmap     SEPARATOR
unsigned char                             transparentColor SEPARATOR
struct ScreenBitmapPresentResult_Compat*  outResult        SEPARATOR
int                                       clearScreenFirst FINAL_SEPARATOR
{
        unsigned short sourceWidth;
        unsigned short sourceHeight;
        unsigned short composeX;
        unsigned short composeY;
        unsigned short x;
        unsigned short y;
        unsigned long composedPixelCount;


        if ((sourceBitmap == 0) || (screenBitmap == 0)) {
                return 0;
        }
        sourceWidth = screen_present_read_u16_le(sourceBitmap - 4);
        sourceHeight = screen_present_read_u16_le(sourceBitmap - 2);
        if ((sourceWidth > SCREEN_PRESENT_COMPAT_WIDTH) || (sourceHeight > SCREEN_PRESENT_COMPAT_HEIGHT)) {
                return 0;
        }
        if (clearScreenFirst) {
                screen_present_init_blank_screen(screenBitmap);
        }
        composeX = (unsigned short)((SCREEN_PRESENT_COMPAT_WIDTH - sourceWidth) / 2);
        composeY = (unsigned short)((SCREEN_PRESENT_COMPAT_HEIGHT - sourceHeight) / 2);
        composedPixelCount = 0;
        for (y = 0; y < sourceHeight; ++y) {
                for (x = 0; x < sourceWidth; ++x) {
                        unsigned char color;


                        color = screen_present_get_nibble(sourceBitmap, x, y);
                        if (color == (transparentColor & 0x0F)) {
                                continue;
                        }
                        screen_present_set_nibble(screenBitmap, (unsigned short)(composeX + x), (unsigned short)(composeY + y), color);
                        composedPixelCount++;
                }
        }
        if (outResult != 0) {
                memset(outResult, 0, sizeof(*outResult));
                outResult->sourceWidth = sourceWidth;
                outResult->sourceHeight = sourceHeight;
                outResult->screenWidth = SCREEN_PRESENT_COMPAT_WIDTH;
                outResult->screenHeight = SCREEN_PRESENT_COMPAT_HEIGHT;
                outResult->composedX = composeX;
                outResult->composedY = composeY;
                outResult->copiedByteCount = screen_present_byte_count(screenBitmap);
                outResult->composedPixelCount = composedPixelCount;
                outResult->sourceBitmap = (unsigned char*)sourceBitmap;
                outResult->screenBitmap = screenBitmap;
        }
        return 1;
}

int F9000_SCREEN_PresentBitmapToScreen_Compat(
const unsigned char*                      sourceBitmap SEPARATOR
unsigned char*                            screenBitmap SEPARATOR
struct ScreenBitmapPresentResult_Compat*  outResult    FINAL_SEPARATOR
{
        if ((sourceBitmap == 0) || (screenBitmap == 0)) {
                return 0;
        }
        F0616_CopyBitmap_Compat(sourceBitmap, screenBitmap);
        if (outResult != 0) {
                memset(outResult, 0, sizeof(*outResult));
                outResult->sourceWidth = screen_present_read_u16_le(sourceBitmap - 4);
                outResult->sourceHeight = screen_present_read_u16_le(sourceBitmap - 2);
                outResult->screenWidth = outResult->sourceWidth;
                outResult->screenHeight = outResult->sourceHeight;
                outResult->copiedByteCount = screen_present_byte_count(sourceBitmap);
                outResult->composedPixelCount = (unsigned long)outResult->sourceWidth * (unsigned long)outResult->sourceHeight;
                outResult->sourceBitmap = (unsigned char*)sourceBitmap;
                outResult->screenBitmap = screenBitmap;
        }
        return 1;
}

int F9004_SCREEN_ComposeBitmapToScreen_Compat(
const unsigned char*                      sourceBitmap      SEPARATOR
unsigned char*                            screenBitmap      SEPARATOR
unsigned char                             transparentColor  SEPARATOR
struct ScreenBitmapPresentResult_Compat*  outResult         FINAL_SEPARATOR
{
        return screen_present_overlay_bitmap(sourceBitmap, screenBitmap, transparentColor, outResult, 1);
}

int F9005_SCREEN_OverlayBitmapOnScreen_Compat(
const unsigned char*                      sourceBitmap      SEPARATOR
unsigned char*                            screenBitmap      SEPARATOR
unsigned char                             transparentColor  SEPARATOR
struct ScreenBitmapPresentResult_Compat*  outResult         FINAL_SEPARATOR
{
        return screen_present_overlay_bitmap(sourceBitmap, screenBitmap, transparentColor, outResult, 0);
}

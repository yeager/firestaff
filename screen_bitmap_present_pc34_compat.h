#ifndef REDMCSB_SCREEN_BITMAP_PRESENT_PC34_COMPAT_H
#define REDMCSB_SCREEN_BITMAP_PRESENT_PC34_COMPAT_H

struct ScreenBitmapPresentResult_Compat {
    unsigned short sourceWidth;
    unsigned short sourceHeight;
    unsigned short screenWidth;
    unsigned short screenHeight;
    unsigned short composedX;
    unsigned short composedY;
    unsigned long copiedByteCount;
    unsigned long composedPixelCount;
    unsigned char* sourceBitmap;
    unsigned char* screenBitmap;
};

int F9000_SCREEN_PresentBitmapToScreen_Compat(
    const unsigned char* sourceBitmap,
    unsigned char* screenBitmap,
    struct ScreenBitmapPresentResult_Compat* outResult);

int F9004_SCREEN_ComposeBitmapToScreen_Compat(
    const unsigned char* sourceBitmap,
    unsigned char* screenBitmap,
    unsigned char transparentColor,
    struct ScreenBitmapPresentResult_Compat* outResult);

int F9005_SCREEN_OverlayBitmapOnScreen_Compat(
    const unsigned char* sourceBitmap,
    unsigned char* screenBitmap,
    unsigned char transparentColor,
    struct ScreenBitmapPresentResult_Compat* outResult);

int F9006_SCREEN_OverlayViewportBitmapOnScreen_Compat(
    const unsigned char* sourceBitmap,
    unsigned char* screenBitmap,
    unsigned char transparentColor,
    struct ScreenBitmapPresentResult_Compat* outResult);

#endif

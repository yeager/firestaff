#ifndef REDMCSB_SCREEN_BITMAP_EXPORT_PPM_VGA_PC34_COMPAT_H
#define REDMCSB_SCREEN_BITMAP_EXPORT_PPM_VGA_PC34_COMPAT_H

struct ScreenBitmapExportPpmVgaResult_Compat {
    unsigned short width;
    unsigned short height;
    unsigned long pixelCount;
    unsigned int paletteLevel;
};

/* Export a 4-bit-per-pixel bitmap to PPM using the real VGA palette.
   paletteLevel: 0=brightest (title/menu), 5=darkest (deep dungeon). */
int F9011_SCREEN_ExportBitmapToPpmVga_Compat(
    const unsigned char* bitmap,
    const char* outputPath,
    unsigned int paletteLevel,
    struct ScreenBitmapExportPpmVgaResult_Compat* outResult);

#endif

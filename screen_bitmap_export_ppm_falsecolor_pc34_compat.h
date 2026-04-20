#ifndef REDMCSB_SCREEN_BITMAP_EXPORT_PPM_FALSECOLOR_PC34_COMPAT_H
#define REDMCSB_SCREEN_BITMAP_EXPORT_PPM_FALSECOLOR_PC34_COMPAT_H

struct ScreenBitmapExportPpmFalsecolorResult_Compat {
    unsigned short width;
    unsigned short height;
    unsigned long pixelCount;
};

int F9003_SCREEN_ExportBitmapToPpmFalsecolor_Compat(
    const unsigned char* bitmap,
    const char* outputPath,
    struct ScreenBitmapExportPpmFalsecolorResult_Compat* outResult);

#endif

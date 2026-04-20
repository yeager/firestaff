#ifndef REDMCSB_SCREEN_BITMAP_EXPORT_PGM_PC34_COMPAT_H
#define REDMCSB_SCREEN_BITMAP_EXPORT_PGM_PC34_COMPAT_H

struct ScreenBitmapExportPgmResult_Compat {
    unsigned short width;
    unsigned short height;
    unsigned long pixelCount;
};

int F9001_SCREEN_ExportBitmapToPgm_Compat(
    const unsigned char* bitmap,
    const char* outputPath,
    struct ScreenBitmapExportPgmResult_Compat* outResult);

#endif

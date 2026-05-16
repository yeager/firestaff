#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include <stdio.h>
#include <string.h>
#include "screen_bitmap_export_ppm_falsecolor_pc34_compat.h"

static unsigned short export_read_u16_le(const unsigned char* p) {
        return (unsigned short)(p[0] | ((unsigned short)p[1] << 8));
}

static const unsigned char G9003_auc_FalsecolorPalette_Compat[16][3] = {
        {0x00, 0x00, 0x00},
        {0x1f, 0x77, 0xb4},
        {0xff, 0x7f, 0x0e},
        {0x2c, 0xa0, 0x2c},
        {0xd6, 0x27, 0x28},
        {0x94, 0x67, 0xbd},
        {0x8c, 0x56, 0x4b},
        {0xe3, 0x77, 0xc2},
        {0x7f, 0x7f, 0x7f},
        {0xbc, 0xbd, 0x22},
        {0x17, 0xbe, 0xcf},
        {0xff, 0xbb, 0x78},
        {0x98, 0xdf, 0x8a},
        {0xff, 0x98, 0x96},
        {0xc5, 0xb0, 0xd5},
        {0xc7, 0xc7, 0xc7}
};

int F9003_SCREEN_ExportBitmapToPpmFalsecolor_Compat(
const unsigned char*                               bitmap     SEPARATOR
const char*                                        outputPath SEPARATOR
struct ScreenBitmapExportPpmFalsecolorResult_Compat* outResult FINAL_SEPARATOR
{
        FILE* file;
        unsigned short width;
        unsigned short height;
        unsigned short x;
        unsigned short y;
        unsigned short bytesPerRow;

        if ((bitmap == 0) || (outputPath == 0)) {
                return 0;
        }
        width = export_read_u16_le(bitmap - 4);
        height = export_read_u16_le(bitmap - 2);
        bytesPerRow = (unsigned short)(((width + 1) & 0xFFFE) >> 1);
        file = fopen(outputPath, "wb");
        if (file == 0) {
                return 0;
        }
        fprintf(file, "P6\n%u %u\n255\n", (unsigned int)width, (unsigned int)height);
        for (y = 0; y < height; ++y) {
                const unsigned char* row;
                row = bitmap + ((unsigned long)y * (unsigned long)bytesPerRow);
                for (x = 0; x < width; ++x) {
                        unsigned char packed;
                        unsigned char nibble;
                        packed = row[x >> 1];
                        nibble = (x & 1) ? (packed & 0x0F) : (packed >> 4);
                        fwrite(G9003_auc_FalsecolorPalette_Compat[nibble], 1, 3, file);
                }
        }
        fclose(file);
        if (outResult != 0) {
                memset(outResult, 0, sizeof(*outResult));
                outResult->width = width;
                outResult->height = height;
                outResult->pixelCount = (unsigned long)width * (unsigned long)height;
        }
        return 1;
}

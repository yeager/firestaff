#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include <stdio.h>
#include <string.h>
#include "screen_bitmap_export_pgm_pc34_compat.h"

static unsigned short export_read_u16_le(const unsigned char* p) {
        return (unsigned short)(p[0] | ((unsigned short)p[1] << 8));
}

int F9001_SCREEN_ExportBitmapToPgm_Compat(
const unsigned char*                        bitmap     SEPARATOR
const char*                                 outputPath SEPARATOR
struct ScreenBitmapExportPgmResult_Compat*  outResult  FINAL_SEPARATOR
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
        fprintf(file, "P5\n%u %u\n255\n", (unsigned int)width, (unsigned int)height);
        for (y = 0; y < height; ++y) {
                const unsigned char* row;
                row = bitmap + ((unsigned long)y * (unsigned long)bytesPerRow);
                for (x = 0; x < width; ++x) {
                        unsigned char packed;
                        unsigned char nibble;
                        unsigned char gray;


                        packed = row[x >> 1];
                        nibble = (x & 1) ? (packed & 0x0F) : (packed >> 4);
                        gray = (unsigned char)(nibble * 17);
                        fwrite(&gray, 1, 1, file);
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

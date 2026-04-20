#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include <stdio.h>
#include <string.h>
#include "screen_bitmap_export_ppm_vga_pc34_compat.h"
#include "vga_palette_pc34_compat.h"

static unsigned short export_vga_read_u16_le(const unsigned char* p) {
        return (unsigned short)(p[0] | ((unsigned short)p[1] << 8));
}

int F9011_SCREEN_ExportBitmapToPpmVga_Compat(
const unsigned char*                               bitmap       SEPARATOR
const char*                                        outputPath   SEPARATOR
unsigned int                                       paletteLevel SEPARATOR
struct ScreenBitmapExportPpmVgaResult_Compat*      outResult    FINAL_SEPARATOR
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
        if (paletteLevel >= 6U) {
                paletteLevel = 0;
        }
        width = export_vga_read_u16_le(bitmap - 4);
        height = export_vga_read_u16_le(bitmap - 2);
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
                        const unsigned char* rgb;
                        packed = row[x >> 1];
                        nibble = (x & 1) ? (packed & 0x0F) : (packed >> 4);
                        rgb = F9010_VGA_GetColorRgb_Compat(nibble, paletteLevel);
                        if (rgb != 0) {
                                fwrite(rgb, 1, 3, file);
                        } else {
                                unsigned char black[3] = {0, 0, 0};
                                fwrite(black, 1, 3, file);
                        }
                }
        }
        fclose(file);
        if (outResult != 0) {
                memset(outResult, 0, sizeof(*outResult));
                outResult->width = width;
                outResult->height = height;
                outResult->pixelCount = (unsigned long)width * (unsigned long)height;
                outResult->paletteLevel = paletteLevel;
        }
        return 1;
}

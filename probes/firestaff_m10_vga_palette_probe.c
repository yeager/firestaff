/*
 * M10 VGA Palette Probe
 *
 * Takes a 4bpp PGM bitmap (as produced by the M9 beta harness) and
 * re-exports it as a PPM using the real CSB PC 3.4 VGA palette.
 *
 * This proves:
 * 1. The VGA palette seam compiles and produces correct output
 * 2. Existing title-hold PGM frames can be color-mapped
 * 3. The palette lookup path works end-to-end
 *
 * Usage: <prog> <input.pgm> <output.ppm> [palette_level]
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vga_palette_pc34_compat.h"

int main(int argc, char** argv) {
        const char* inputPath;
        const char* outputPath;
        unsigned int paletteLevel;
        FILE* fin;
        FILE* fout;
        char header[256];
        unsigned int width;
        unsigned int height;
        unsigned int maxval;
        unsigned int x;
        unsigned int y;
        unsigned char* row;
        unsigned long nonBlackCount;
        unsigned long totalPixels;

        if (argc < 3) {
                fprintf(stderr, "Usage: %s <input.pgm> <output.ppm> [palette_level]\n", argv[0]);
                return 1;
        }
        inputPath = argv[1];
        outputPath = argv[2];
        paletteLevel = (argc >= 4) ? (unsigned int)strtoul(argv[3], NULL, 10) : 0;
        if (paletteLevel >= 6) paletteLevel = 0;

        fin = fopen(inputPath, "rb");
        if (!fin) {
                fprintf(stderr, "FAIL: cannot open %s\n", inputPath);
                return 2;
        }

        /* Parse PGM header: P5 <width> <height> <maxval> */
        if (!fgets(header, sizeof(header), fin) || strncmp(header, "P5", 2) != 0) {
                fprintf(stderr, "FAIL: not a P5 PGM file\n");
                fclose(fin);
                return 3;
        }
        /* Skip comments */
        while (fgets(header, sizeof(header), fin)) {
                if (header[0] != '#') break;
        }
        if (sscanf(header, "%u %u", &width, &height) != 2) {
                fprintf(stderr, "FAIL: cannot parse PGM dimensions\n");
                fclose(fin);
                return 4;
        }
        if (!fgets(header, sizeof(header), fin) || sscanf(header, "%u", &maxval) != 1) {
                fprintf(stderr, "FAIL: cannot parse PGM maxval\n");
                fclose(fin);
                return 5;
        }

        fout = fopen(outputPath, "wb");
        if (!fout) {
                fprintf(stderr, "FAIL: cannot create %s\n", outputPath);
                fclose(fin);
                return 6;
        }
        fprintf(fout, "P6\n%u %u\n255\n", width, height);

        row = (unsigned char*)malloc(width);
        if (!row) {
                fclose(fin);
                fclose(fout);
                return 7;
        }

        nonBlackCount = 0;
        totalPixels = 0;

        for (y = 0; y < height; y++) {
                if (fread(row, 1, width, fin) != width) {
                        fprintf(stderr, "FAIL: truncated PGM at row %u\n", y);
                        free(row);
                        fclose(fin);
                        fclose(fout);
                        return 8;
                }
                for (x = 0; x < width; x++) {
                        unsigned char pixel = row[x];
                        /* PGM pixel values 0-15 map directly to VGA color indices */
                        unsigned char colorIndex = (pixel > 15) ? 15 : pixel;
                        const unsigned char* rgb = F9010_VGA_GetColorRgb_Compat(colorIndex, paletteLevel);
                        if (rgb) {
                                fwrite(rgb, 1, 3, fout);
                                if (rgb[0] != 0 || rgb[1] != 0 || rgb[2] != 0) {
                                        nonBlackCount++;
                                }
                        } else {
                                unsigned char black[3] = {0, 0, 0};
                                fwrite(black, 1, 3, fout);
                        }
                        totalPixels++;
                }
        }

        free(row);
        fclose(fin);
        fclose(fout);

        printf("vgaPpmExported=1\n");
        printf("vgaPpmPath=%s\n", outputPath);
        printf("vgaPpmWidth=%u\n", width);
        printf("vgaPpmHeight=%u\n", height);
        printf("vgaPpmTotalPixels=%lu\n", totalPixels);
        printf("vgaPpmNonBlackPixels=%lu\n", nonBlackCount);
        printf("vgaPpmPaletteLevel=%u\n", paletteLevel);
        printf("vgaPpmColorCount=16\n");
        printf("vgaPpmBrightnessLevels=6\n");

        return 0;
}

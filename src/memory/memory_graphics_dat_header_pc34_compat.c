#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include <stdlib.h>
#include <string.h>
#include "memory_graphics_dat_header_pc34_compat.h"
#include "memory_graphics_dat_metadata_pc34_compat.h"

#define GRAPHICS_DAT_NEW_FORMAT_MASK 0x8000u
#define GRAPHICS_DAT_NEW_FORMAT_ID_MASK 0x7FFFu

static int read_u16_le(FILE* file, unsigned short* outValue) {
        unsigned char bytes[2];


        if (fread(bytes, 1, 2, file) != 2) {
                return 0;
        }
        *outValue = (unsigned short)(bytes[0] | ((unsigned short)bytes[1] << 8));
        return 1;
}

static int read_u16_be(FILE* file, unsigned short* outValue) {
        unsigned char bytes[2];


        if (fread(bytes, 1, 2, file) != 2) {
                return 0;
        }
        *outValue = (unsigned short)(((unsigned short)bytes[0] << 8) | bytes[1]);
        return 1;
}

static int read_u16_ordered(FILE* file,
                            unsigned short* outValue,
                            int bigEndian) {
        return bigEndian ? read_u16_be(file, outValue)
                         : read_u16_le(file, outValue);
}

static int read_u16_array(FILE* file,
                          unsigned short* outValues,
                          unsigned short count,
                          int bigEndian) {
        unsigned short i;


        for (i = 0; i < count; i++) {
                if (!read_u16_ordered(file, &outValues[i], bigEndian)) {
                        return 0;
                }
        }
        return 1;
}

static int read_width_height_array(FILE* file,
                                   struct GraphicWidthHeight_Compat* outValues,
                                   unsigned short count,
                                   int bigEndian) {
        unsigned short i;


        for (i = 0; i < count; i++) {
                if (!read_u16_ordered(file, &outValues[i].Width, bigEndian) ||
                    !read_u16_ordered(file, &outValues[i].Height, bigEndian)) {
                        return 0;
                }
        }
        return 1;
}

static int legacy_graphics_dat_extent_matches(FILE* file,
                                              unsigned short graphicCount,
                                              int bigEndian,
                                              long actualFileSize) {
        unsigned short i;
        unsigned short compressedByteCount;
        long expectedFileSize;

        if (file == 0 || graphicCount == 0) {
                return 0;
        }
        expectedFileSize = (long)sizeof(short) +
            (long)graphicCount * (long)(sizeof(short) * 2);
        if (expectedFileSize > actualFileSize ||
            fseek(file, (long)sizeof(short), SEEK_SET) != 0) {
                return 0;
        }
        for (i = 0; i < graphicCount; ++i) {
                if (!read_u16_ordered(file, &compressedByteCount, bigEndian) ||
                    compressedByteCount > actualFileSize - expectedFileSize) {
                        return 0;
                }
                expectedFileSize += compressedByteCount;
        }
        return expectedFileSize == actualFileSize;
}

void F0479_MEMORY_FreeGraphicsDatHeader_Compat(
struct MemoryGraphicsDatHeader_Compat* header FINAL_SEPARATOR
{
        free(header->compressedByteCounts);
        free(header->decompressedByteCounts);
        free(header->widthHeight);
        memset(header, 0, sizeof(*header));
}

int F0479_MEMORY_LoadGraphicsDatHeader_Compat(
const char*                          path   SEPARATOR
struct MemoryGraphicsDatState_Compat* state SEPARATOR
struct MemoryGraphicsDatHeader_Compat* header FINAL_SEPARATOR
{
        unsigned short signatureOrCount;
        unsigned short littleEndianCount;
        unsigned short bigEndianCount;
        unsigned short i;
        int bigEndian = 0;
        int littleEndianExtentMatches;
        int bigEndianExtentMatches;
        long actualFileSize;


        memset(header, 0, sizeof(*header));
        if (!F0477_MEMORY_OpenGraphicsDat_CPSDF_Compat(path, state)) {
                return 0;
        }
        if (fseek(state->file, 0, SEEK_SET) != 0) {
                F0478_MEMORY_CloseGraphicsDat_CPSDF_Compat(state);
                return 0;
        }
        if (!read_u16_le(state->file, &signatureOrCount)) {
                F0478_MEMORY_CloseGraphicsDat_CPSDF_Compat(state);
                return 0;
        }
        /* ReDMCSB MEMORY.C F0479 reads platform-native GRAPHICS.DAT tables.
         * PC DM1 stores the new-format marker as little-endian 0x8001
         * (`01 80`), while the verified CSB PC media stores the same marker
         * big-endian (`80 01`). Normalize both forms here so M11's shared V1
         * asset loader can bind CSB graphics without inventing a second path. */
        if (signatureOrCount == 0x0180u) {
                signatureOrCount = 0x8001u;
                bigEndian = 1;
        }
        if ((signatureOrCount & GRAPHICS_DAT_NEW_FORMAT_MASK) != 0) {
                header->format = (int)(signatureOrCount & GRAPHICS_DAT_NEW_FORMAT_ID_MASK);
                if ((header->format != 1) ||
                    !read_u16_ordered(state->file,
                                      &header->graphicCount,
                                      bigEndian)) {
                        F0478_MEMORY_CloseGraphicsDat_CPSDF_Compat(state);
                        return 0;
                }
        } else {
                /* Legacy CSB archives have no 0x8001 marker. Select the byte
                 * order only when its size table reaches the real file end. */
                if (fseek(state->file, 0, SEEK_END) != 0 ||
                    (actualFileSize = ftell(state->file)) < (long)sizeof(short)) {
                        F0478_MEMORY_CloseGraphicsDat_CPSDF_Compat(state);
                        return 0;
                }
                littleEndianCount = signatureOrCount;
                bigEndianCount = (unsigned short)(((signatureOrCount & 0x00ffu) << 8) |
                                                   ((signatureOrCount & 0xff00u) >> 8));
                littleEndianExtentMatches = legacy_graphics_dat_extent_matches(
                    state->file, littleEndianCount, 0, actualFileSize);
                bigEndianExtentMatches = legacy_graphics_dat_extent_matches(
                    state->file, bigEndianCount, 1, actualFileSize);
                if (littleEndianExtentMatches == bigEndianExtentMatches) {
                        F0478_MEMORY_CloseGraphicsDat_CPSDF_Compat(state);
                        return 0;
                }
                bigEndian = bigEndianExtentMatches;
                header->format = 0;
                header->graphicCount = bigEndian ? bigEndianCount : littleEndianCount;
                if (fseek(state->file, (long)sizeof(short), SEEK_SET) != 0) {
                        F0478_MEMORY_CloseGraphicsDat_CPSDF_Compat(state);
                        return 0;
                }
        }
        header->compressedByteCounts = (unsigned short*)calloc(header->graphicCount, sizeof(unsigned short));
        header->decompressedByteCounts = (unsigned short*)calloc(header->graphicCount, sizeof(unsigned short));
        header->widthHeight = (struct GraphicWidthHeight_Compat*)calloc(header->graphicCount, sizeof(struct GraphicWidthHeight_Compat));
        if ((header->compressedByteCounts == 0) || (header->decompressedByteCounts == 0) || (header->widthHeight == 0)) {
                F0479_MEMORY_FreeGraphicsDatHeader_Compat(header);
                F0478_MEMORY_CloseGraphicsDat_CPSDF_Compat(state);
                return 0;
        }
        if (!read_u16_array(state->file,
                            header->compressedByteCounts,
                            header->graphicCount,
                            bigEndian)
         || !read_u16_array(state->file,
                            header->decompressedByteCounts,
                            header->graphicCount,
                            bigEndian)) {
                F0479_MEMORY_FreeGraphicsDatHeader_Compat(header);
                F0478_MEMORY_CloseGraphicsDat_CPSDF_Compat(state);
                return 0;
        }
        if (header->format == 1) {
                if (!read_width_height_array(state->file,
                                             header->widthHeight,
                                             header->graphicCount,
                                             bigEndian)) {
                        F0479_MEMORY_FreeGraphicsDatHeader_Compat(header);
                        F0478_MEMORY_CloseGraphicsDat_CPSDF_Compat(state);
                        return 0;
                }
        } else {
                for (i = 0; i < header->graphicCount; i++) {
                        long offset = F0467_MEMORY_GetGraphicOffset_Compat(
                            header->format,
                            header->graphicCount,
                            header->compressedByteCounts,
                            i);
                        if ((fseek(state->file, offset, SEEK_SET) != 0)
                         || !read_u16_ordered(state->file,
                                              &header->widthHeight[i].Width,
                                              bigEndian)
                         || !read_u16_ordered(state->file,
                                              &header->widthHeight[i].Height,
                                              bigEndian)) {
                                F0479_MEMORY_FreeGraphicsDatHeader_Compat(header);
                                F0478_MEMORY_CloseGraphicsDat_CPSDF_Compat(state);
                                return 0;
                        }
                }
        }
        header->fileSize = F0467_MEMORY_GetGraphicOffset_Compat(
            header->format,
            header->graphicCount,
            header->compressedByteCounts,
            (unsigned int)(header->graphicCount - 1))
            + header->compressedByteCounts[header->graphicCount - 1];
        if (!F0478_MEMORY_CloseGraphicsDat_CPSDF_Compat(state)) {
                F0479_MEMORY_FreeGraphicsDatHeader_Compat(header);
                return 0;
        }
        return 1;
}

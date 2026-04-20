#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include <stdlib.h>
#include <string.h>
#include "memory_graphics_dat_header_pc34_compat.h"
#include "memory_graphics_dat_metadata_pc34_compat.h"

#define GRAPHICS_DAT_NEW_FORMAT_MASK 0x8000u
#define GRAPHICS_DAT_NEW_FORMAT_ID_MASK 0x7FFFu

static int read_u16(FILE* file, unsigned short* outValue) {
        unsigned char bytes[2];


        if (fread(bytes, 1, 2, file) != 2) {
                return 0;
        }
        *outValue = (unsigned short)(bytes[0] | ((unsigned short)bytes[1] << 8));
        return 1;
}

static int read_u16_array(FILE* file, unsigned short* outValues, unsigned short count) {
        unsigned short i;


        for (i = 0; i < count; i++) {
                if (!read_u16(file, &outValues[i])) {
                        return 0;
                }
        }
        return 1;
}

static int read_width_height_array(FILE* file, struct GraphicWidthHeight_Compat* outValues, unsigned short count) {
        unsigned short i;


        for (i = 0; i < count; i++) {
                if (!read_u16(file, &outValues[i].Width) || !read_u16(file, &outValues[i].Height)) {
                        return 0;
                }
        }
        return 1;
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
        unsigned short i;


        memset(header, 0, sizeof(*header));
        if (!F0477_MEMORY_OpenGraphicsDat_CPSDF_Compat(path, state)) {
                return 0;
        }
        if (fseek(state->file, 0, SEEK_SET) != 0) {
                F0478_MEMORY_CloseGraphicsDat_CPSDF_Compat(state);
                return 0;
        }
        if (!read_u16(state->file, &signatureOrCount)) {
                F0478_MEMORY_CloseGraphicsDat_CPSDF_Compat(state);
                return 0;
        }
        if ((signatureOrCount & GRAPHICS_DAT_NEW_FORMAT_MASK) != 0) {
                header->format = (int)(signatureOrCount & GRAPHICS_DAT_NEW_FORMAT_ID_MASK);
                if ((header->format != 1) || !read_u16(state->file, &header->graphicCount)) {
                        F0478_MEMORY_CloseGraphicsDat_CPSDF_Compat(state);
                        return 0;
                }
        } else {
                header->format = 0;
                header->graphicCount = signatureOrCount;
        }
        header->compressedByteCounts = (unsigned short*)calloc(header->graphicCount, sizeof(unsigned short));
        header->decompressedByteCounts = (unsigned short*)calloc(header->graphicCount, sizeof(unsigned short));
        header->widthHeight = (struct GraphicWidthHeight_Compat*)calloc(header->graphicCount, sizeof(struct GraphicWidthHeight_Compat));
        if ((header->compressedByteCounts == 0) || (header->decompressedByteCounts == 0) || (header->widthHeight == 0)) {
                F0479_MEMORY_FreeGraphicsDatHeader_Compat(header);
                F0478_MEMORY_CloseGraphicsDat_CPSDF_Compat(state);
                return 0;
        }
        if (!read_u16_array(state->file, header->compressedByteCounts, header->graphicCount)
         || !read_u16_array(state->file, header->decompressedByteCounts, header->graphicCount)) {
                F0479_MEMORY_FreeGraphicsDatHeader_Compat(header);
                F0478_MEMORY_CloseGraphicsDat_CPSDF_Compat(state);
                return 0;
        }
        if (header->format == 1) {
                if (!read_width_height_array(state->file, header->widthHeight, header->graphicCount)) {
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
                         || !read_u16(state->file, &header->widthHeight[i].Width)
                         || !read_u16(state->file, &header->widthHeight[i].Height)) {
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

#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include "memory_graphics_dat_metadata_pc34_compat.h"

long F0467_MEMORY_GetGraphicOffset_Compat(
int                   graphicsDatFileFormat SEPARATOR
unsigned int          graphicCount          SEPARATOR
const unsigned short* compressedByteCounts  SEPARATOR
unsigned int          graphicIndex          FINAL_SEPARATOR
{
        unsigned int i;
        long offset;


        if (graphicsDatFileFormat == 1) {
                offset = (long)(sizeof(short) * 2) + (long)graphicCount * (long)(sizeof(short) * 4);
        } else {
                offset = (long)sizeof(short) + (long)graphicCount * (long)(sizeof(short) * 2);
        }
        for (i = 0; i < graphicIndex; i++) {
                offset += compressedByteCounts[i];
        }
        return offset;
}

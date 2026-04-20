#ifndef REDMCSB_MEMORY_GRAPHICS_DAT_HEADER_PC34_COMPAT_H
#define REDMCSB_MEMORY_GRAPHICS_DAT_HEADER_PC34_COMPAT_H

#include "memory_frontend_pc34_compat.h"
#include "memory_graphics_dat_pc34_compat.h"

struct MemoryGraphicsDatHeader_Compat {
    int format;
    unsigned short graphicCount;
    unsigned short* compressedByteCounts;
    unsigned short* decompressedByteCounts;
    struct GraphicWidthHeight_Compat* widthHeight;
    long fileSize;
};

int F0479_MEMORY_LoadGraphicsDatHeader_Compat(
    const char* path,
    struct MemoryGraphicsDatState_Compat* state,
    struct MemoryGraphicsDatHeader_Compat* header);

void F0479_MEMORY_FreeGraphicsDatHeader_Compat(
    struct MemoryGraphicsDatHeader_Compat* header);

#endif

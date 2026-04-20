#ifndef REDMCSB_MEMORY_GRAPHICS_DAT_STATE_PC34_COMPAT_H
#define REDMCSB_MEMORY_GRAPHICS_DAT_STATE_PC34_COMPAT_H

#include "memory_graphics_dat_header_pc34_compat.h"

struct MemoryGraphicsDatRuntimeState_Compat {
    int initialized;
    int format;
    unsigned short graphicCount;
    unsigned short* compressedByteCounts;
    unsigned short* decompressedByteCounts;
    struct GraphicWidthHeight_Compat* widthHeight;
    long fileSize;
};

int F0479_MEMORY_InitializeGraphicsDatState_Compat(
    const char* path,
    struct MemoryGraphicsDatState_Compat* fileState,
    struct MemoryGraphicsDatRuntimeState_Compat* runtimeState);

void F0479_MEMORY_FreeGraphicsDatState_Compat(
    struct MemoryGraphicsDatRuntimeState_Compat* runtimeState);

#endif

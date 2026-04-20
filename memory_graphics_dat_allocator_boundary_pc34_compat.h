#ifndef REDMCSB_MEMORY_GRAPHICS_DAT_ALLOCATOR_BOUNDARY_PC34_COMPAT_H
#define REDMCSB_MEMORY_GRAPHICS_DAT_ALLOCATOR_BOUNDARY_PC34_COMPAT_H

#include "memory_graphics_dat_used_list_pc34_compat.h"
#include "memory_cache_allocation_boundary_pc34_compat.h"

unsigned char* F0489_MEMORY_GetOrLoadBitmapWithAllocatorBoundary_Compat(
    const char* graphicsDatPath,
    const struct MemoryGraphicsDatRuntimeState_Compat* runtimeState,
    struct MemoryGraphicsDatState_Compat* fileState,
    unsigned int graphicIndex,
    unsigned char* loadedGraphicBuffer,
    unsigned short* nativeBitmapBlockIndices,
    struct NativeBitmapBlock_Compat** blocks,
    int blockCapacity,
    struct NativeBitmapBlock_Compat* newBlock,
    struct MemoryCacheUsageState_Compat* usageState,
    struct MemoryGraphicsDatBitmapPathResult_Compat* outResult);

#endif

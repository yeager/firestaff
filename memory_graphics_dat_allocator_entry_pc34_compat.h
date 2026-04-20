#ifndef REDMCSB_MEMORY_GRAPHICS_DAT_ALLOCATOR_ENTRY_PC34_COMPAT_H
#define REDMCSB_MEMORY_GRAPHICS_DAT_ALLOCATOR_ENTRY_PC34_COMPAT_H

#include "memory_graphics_dat_allocator_boundary_pc34_compat.h"
#include "memory_cache_allocator_pc34_compat.h"

struct MemoryGraphicsDatAllocatorEntryResult_Compat {
    struct MemoryGraphicsDatBitmapPathResult_Compat bitmapPath;
    struct MemoryCacheAllocatorResult_Compat allocator;
    unsigned long normalizedRequestedSize;
};

unsigned char* F0489_MEMORY_GetOrLoadBitmapWithAllocatorEntry_Compat(
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
    struct MemoryCacheAllocateOrReuseState_Compat* allocatorState,
    struct MemoryCacheUnusedBlock_Compat** firstUnusedBlock,
    struct MemoryCacheUnusedBlock_Compat* splitRemainder,
    struct MemoryGraphicsDatAllocatorEntryResult_Compat* outResult);

#endif

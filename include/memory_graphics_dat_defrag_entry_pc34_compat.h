#ifndef REDMCSB_MEMORY_GRAPHICS_DAT_DEFRAG_ENTRY_PC34_COMPAT_H
#define REDMCSB_MEMORY_GRAPHICS_DAT_DEFRAG_ENTRY_PC34_COMPAT_H

#include "memory_graphics_dat_allocator_entry_pc34_compat.h"
#include "memory_cache_allocator_defrag_pc34_compat.h"

struct MemoryGraphicsDatDefragEntryResult_Compat {
    struct MemoryGraphicsDatBitmapPathResult_Compat bitmapPath;
    struct MemoryCacheAllocatorResult_Compat allocator;
    unsigned long normalizedRequestedSize;
    int defragApplied;
};

unsigned char* F0489_MEMORY_GetOrLoadBitmapWithDefragEntry_Compat(
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
    unsigned long compactedTopOffset,
    struct MemoryCacheAllocateOrReuseState_Compat* allocatorState,
    struct MemoryCacheDefragResultState_Compat* defragState,
    struct MemoryCacheUnusedBlock_Compat** firstUnusedBlock,
    struct MemoryCacheUnusedBlock_Compat* splitRemainder,
    struct MemoryGraphicsDatDefragEntryResult_Compat* outResult);

#endif

#ifndef REDMCSB_MEMORY_GRAPHICS_DAT_ALLOCATOR_ORCHESTRATOR_PC34_COMPAT_H
#define REDMCSB_MEMORY_GRAPHICS_DAT_ALLOCATOR_ORCHESTRATOR_PC34_COMPAT_H

#include "memory_graphics_dat_defrag_loop_pc34_compat.h"
#include "memory_cache_allocator_defrag_pc34_compat.h"

struct MemoryGraphicsDatAllocatorOrchestratorResult_Compat {
    struct MemoryGraphicsDatBitmapPathResult_Compat bitmapPath;
    struct MemoryCacheAllocatorResult_Compat allocator;
    struct MemoryCacheDefragOrchestratorResult_Compat orchestrator;
    unsigned long normalizedRequestedSize;
    int defragApplied;
};

unsigned char* F0489_MEMORY_GetOrLoadBitmapWithAllocatorOrchestrator_Compat(
    const char* graphicsDatPath,
    const struct MemoryGraphicsDatRuntimeState_Compat* runtimeState,
    struct MemoryGraphicsDatState_Compat* fileState,
    unsigned int graphicIndex,
    unsigned char* loadedGraphicBuffer,
    unsigned short* nativeBitmapBlockIndices,
    unsigned short* derivedBitmapBlockIndices,
    unsigned long* blockOffsets,
    struct NativeBitmapBlock_Compat** blocks,
    int blockCapacity,
    struct NativeBitmapBlock_Compat* newBlock,
    struct MemoryCacheUsageState_Compat* usageState,
    const struct MemoryCacheRawBlockHeader_Compat* headers,
    unsigned int headerCount,
    struct MemoryCacheDefragSegment_Compat* segments,
    unsigned int maxSegments,
    struct MemoryCacheAllocateOrReuseState_Compat* allocatorState,
    struct MemoryCacheDefragResultState_Compat* defragState,
    struct MemoryCacheUnusedBlock_Compat** firstUnusedBlock,
    struct MemoryCacheUnusedBlock_Compat* splitRemainder,
    struct MemoryGraphicsDatAllocatorOrchestratorResult_Compat* outResult);

#endif

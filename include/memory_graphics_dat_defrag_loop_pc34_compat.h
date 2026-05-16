#ifndef REDMCSB_MEMORY_GRAPHICS_DAT_DEFRAG_LOOP_PC34_COMPAT_H
#define REDMCSB_MEMORY_GRAPHICS_DAT_DEFRAG_LOOP_PC34_COMPAT_H

#include "memory_graphics_dat_defrag_entry_pc34_compat.h"
#include "memory_cache_defrag_orchestrator_pc34_compat.h"

struct MemoryGraphicsDatDefragLoopResult_Compat {
    struct MemoryGraphicsDatBitmapPathResult_Compat bitmapPath;
    struct MemoryCacheAllocatorResult_Compat allocator;
    struct MemoryCacheDefragOrchestratorResult_Compat orchestrator;
    unsigned long normalizedRequestedSize;
    int defragApplied;
};

unsigned char* F0489_MEMORY_GetOrLoadBitmapWithDefragLoop_Compat(
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
    unsigned long compactedTopOffset,
    struct MemoryCacheAllocateOrReuseState_Compat* allocatorState,
    struct MemoryCacheDefragResultState_Compat* defragState,
    struct MemoryCacheUnusedBlock_Compat** firstUnusedBlock,
    struct MemoryCacheUnusedBlock_Compat* splitRemainder,
    struct MemoryGraphicsDatDefragLoopResult_Compat* outResult);

#endif

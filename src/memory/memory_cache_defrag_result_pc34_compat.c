#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include "memory_cache_defrag_result_pc34_compat.h"

void MEMORY_CACHE_ApplyDefragResult_Compat(
unsigned long                           compactedTopOffset SEPARATOR
struct MemoryCacheDefragResultState_Compat* state         FINAL_SEPARATOR
{
        state->cacheMemoryTopOffset = compactedTopOffset;
        state->firstUnusedBlock = 0;
}

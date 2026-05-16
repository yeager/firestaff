#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include "memory_cache_allocator_pc34_compat.h"

int F0483_CACHE_GetNewBlock_Compat(
unsigned long                              requestedSize    SEPARATOR
unsigned long                              minimumSplitSize SEPARATOR
struct MemoryCacheAllocateOrReuseState_Compat* state        SEPARATOR
struct MemoryCacheUnusedBlock_Compat**     firstUnusedBlock SEPARATOR
struct MemoryCacheUnusedBlock_Compat*      splitRemainder   SEPARATOR
struct MemoryCacheAllocatorResult_Compat*  outResult        FINAL_SEPARATOR
{
        struct MemoryCacheDefragBranchResult_Compat branch;
        struct MemoryCacheAllocateOrReuseResult_Compat allocResult;


        branch = MEMORY_CACHE_CheckDefragBranch_Compat(
            requestedSize,
            state->cacheMemorySpan,
            *firstUnusedBlock);
        outResult->defragmentRequested = branch.shouldDefragment;
        if (branch.shouldDefragment) {
                if (state->cacheBytesAvailable < requestedSize) {
                        return 0;
                }
                outResult->blockOffset = state->cacheMemoryTopOffset;
                outResult->allocatedSize = requestedSize;
                outResult->allocatedFromTop = 1;
                outResult->reusedExistingBlock = 0;
                outResult->splitPerformed = 0;
                state->cacheMemoryTopOffset += requestedSize;
                state->cacheBytesAvailable -= requestedSize;
                state->cacheMemorySpan = 0;
                return 1;
        }
        if (!MEMORY_CACHE_AllocateOrReuseBlock_Compat(
                requestedSize,
                minimumSplitSize,
                state,
                firstUnusedBlock,
                splitRemainder,
                &allocResult)) {
                return 0;
        }
        outResult->blockOffset = allocResult.blockOffset;
        outResult->allocatedSize = allocResult.allocatedSize;
        outResult->allocatedFromTop = allocResult.allocatedFromTop;
        outResult->reusedExistingBlock = allocResult.reusedExistingBlock;
        outResult->splitPerformed = allocResult.splitPerformed;
        return 1;
}

/* ══════════════════════════════════════════════════════════════════════
 * Pass601 — Memory allocator source-lock (MEMORY.C:304-397)
 *
 * F0468_MEMORY_Allocate is the central heap allocator:
 *   - Two allocation types: PERMANENT (bottom-up) and TEMPORARY (top-down)
 *   - Permanent: bumps G0461_puc_HeapEnd upward
 *   - Temporary: bumps G0462_puc_HeapTopOfTemporary downward
 *   - Checks for overlap (out of memory) at MEMORY.C:358
 *   - Bitmap allocations (MASK0x8000) add 2*sizeof(int16_t) header
 *   - Returns pointer to usable memory after header
 *
 * F0602_GetMinimumOverheadMemoryChunk (MEMORY.C:222-253):
 *   - Multi-chunk variant: finds chunk with least overhead for allocation
 *
 * F0603_AllocateFromTopMemoryChunk (MEMORY.C:255-262):
 *   - Top-of-chunk allocation for temporary bitmaps
 *
 * F0604/F0605 Backup/Restore (MEMORY.C:264-301):
 *   - Save/restore heap state for transient operations
 * ══════════════════════════════════════════════════════════════════════ */

const char *dm1_memory_pass601_allocator_source_evidence(void)
{
    return
        "MEMORY.C:304-397 F0468_MEMORY_Allocate permanent/temporary heap\n"
        "MEMORY.C:222-253 F0602_GetMinimumOverheadMemoryChunk\n"
        "MEMORY.C:255-262 F0603_AllocateFromTopMemoryChunk\n"
        "MEMORY.C:264-301 F0604/F0605 backup/restore heap state\n"
        "MEMORY.C:358 overlap check (out of memory)\n"
        "MEMORY.C:339 MASK0x8000 bitmap header allocation\n";
}


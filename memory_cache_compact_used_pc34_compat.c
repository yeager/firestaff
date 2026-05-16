#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include "memory_cache_compact_used_pc34_compat.h"
#include "memory_cache_usage_pc34_compat.h"

static unsigned short compact_used_lookup_block_index(
int             bitmapIndex              SEPARATOR
int             isDerivedBitmap          SEPARATOR
unsigned short* nativeBitmapBlockIndices SEPARATOR
unsigned short* derivedBitmapBlockIndices SEPARATOR
unsigned short  nativeIndexCount         SEPARATOR
unsigned short  derivedIndexCount        FINAL_SEPARATOR
{
        if (!isDerivedBitmap) {
                if ((unsigned short)bitmapIndex >= nativeIndexCount) {
                        return MEMORY_CACHE_INDEX_NONE;
                }
                return nativeBitmapBlockIndices[bitmapIndex];
        }
        if ((unsigned short)bitmapIndex >= derivedIndexCount) {
                return MEMORY_CACHE_INDEX_NONE;
        }
        return derivedBitmapBlockIndices[bitmapIndex];
}

int MEMORY_CACHE_CompactUsedBlock_Compat(
int                                  bitmapIndex               SEPARATOR
int                                  isDerivedBitmap           SEPARATOR
unsigned long                        fromOffset                SEPARATOR
unsigned long                        toOffset                  SEPARATOR
unsigned short*                      nativeBitmapBlockIndices  SEPARATOR
unsigned short*                      derivedBitmapBlockIndices SEPARATOR
unsigned long*                       blockOffsets              SEPARATOR
struct MemoryCacheUsageState_Compat* usageState                SEPARATOR
unsigned short                       nativeIndexCount          SEPARATOR
unsigned short                       derivedIndexCount         SEPARATOR
struct MemoryCacheCompactUsedResult_Compat* outResult          FINAL_SEPARATOR
{
        unsigned short blockIndex;


        blockIndex = compact_used_lookup_block_index(
            bitmapIndex,
            isDerivedBitmap,
            nativeBitmapBlockIndices,
            derivedBitmapBlockIndices,
            nativeIndexCount,
            derivedIndexCount);
        if (blockIndex == MEMORY_CACHE_INDEX_NONE) {
                return 0;
        }
        outResult->blockIndex = blockIndex;
        outResult->movedFromOffset = fromOffset;
        outResult->movedToOffset = toOffset;
        outResult->moved = fromOffset != toOffset;
        if (!outResult->moved) {
                return 1;
        }
        blockOffsets[blockIndex] = toOffset;
        if ((unsigned long)usageState->firstUsedBlock == fromOffset) {
                usageState->firstUsedBlock = (struct NativeBitmapBlock_Compat*)toOffset;
        }
        if ((unsigned long)usageState->lastUsedBlock == fromOffset) {
                usageState->lastUsedBlock = (struct NativeBitmapBlock_Compat*)toOffset;
        }
        if ((unsigned long)usageState->firstReferencedUsedBlock == fromOffset) {
                usageState->firstReferencedUsedBlock = (struct NativeBitmapBlock_Compat*)toOffset;
        }
        return 1;
}

/* ══════════════════════════════════════════════════════════════════════
 * Pass601 — Cache release/free/defragment (MEMORY.C:1487-1721)
 *
 * F0480_CACHE_ReleaseBlock (MEMORY.C:1487-1586):
 *   - Releases a cache block back to unused pool
 *   - Updates block pointer in G0655_ppuc_Blocks[graphicIndex]
 *   - Calls F0472_CACHE_AddUnusedBlock for coalescing
 *   - If block was derived bitmap: clears derivation chain
 *
 * F0481_CACHE_FreeMemory (MEMORY.C:1588-1600):
 *   - Releases blocks until requested byte count is freed
 *   - Iterates used list from least recently used
 *   - Calls F0480_CACHE_ReleaseBlock for each candidate
 *
 * F0482_CACHE_Defragment (MEMORY.C:1602-1654):
 *   - Compacts all used blocks toward low memory
 *   - Moves each block to first-fit unused slot
 *   - Updates all pointers in G0655_ppuc_Blocks
 *   - Coalesces freed space at high end
 *
 * F0483_CACHE_GetNewBlock (MEMORY.C:1656-1721):
 *   - Allocates new block from unused pool
 *   - If no fit: calls F0481 to free, then F0482 to defrag
 *   - Returns NULL if cache exhausted after defrag
 * ══════════════════════════════════════════════════════════════════════ */

const char *dm1_memory_pass601_compact_source_evidence(void)
{
    return
        "MEMORY.C:1487-1586 F0480_CACHE_ReleaseBlock\n"
        "MEMORY.C:1588-1600 F0481_CACHE_FreeMemory LRU eviction\n"
        "MEMORY.C:1602-1654 F0482_CACHE_Defragment compact+coalesce\n"
        "MEMORY.C:1656-1721 F0483_CACHE_GetNewBlock alloc+evict+defrag\n";
}


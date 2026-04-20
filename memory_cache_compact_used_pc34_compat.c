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

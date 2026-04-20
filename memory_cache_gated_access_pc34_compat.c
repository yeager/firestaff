#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include "memory_cache_gated_access_pc34_compat.h"

unsigned char* F0489_MEMORY_GetNativeBitmapByIndexTimeGated_Compat(
unsigned long                        gameTime                 SEPARATOR
unsigned long*                       lastResetGameTime        SEPARATOR
int                                  graphicIndex             SEPARATOR
const unsigned char*                 graphic                  SEPARATOR
unsigned short*                      nativeBitmapBlockIndices SEPARATOR
struct NativeBitmapBlock_Compat**    blocks                   SEPARATOR
int                                  blockCapacity            SEPARATOR
struct NativeBitmapBlock_Compat*     newBlock                 SEPARATOR
struct MemoryCacheUsageState_Compat* usageState               SEPARATOR
const struct GraphicWidthHeight_Compat* sizeInfo              FINAL_SEPARATOR
{
        MEMORY_CACHE_CheckResetUsageCounts_Compat(gameTime, lastResetGameTime, usageState, blocks);
        return F0489_MEMORY_GetNativeBitmapByIndexAndTouch_Compat(
            graphicIndex,
            graphic,
            nativeBitmapBlockIndices,
            blocks,
            blockCapacity,
            newBlock,
            usageState,
            sizeInfo);
}

int F0491_CACHE_IsDerivedBitmapInCacheTimeGated_Compat(
unsigned long                        gameTime                 SEPARATOR
unsigned long*                       lastResetGameTime        SEPARATOR
int                                  derivedBitmapIndex       SEPARATOR
unsigned short*                      derivedBitmapBlockIndices SEPARATOR
struct NativeBitmapBlock_Compat**    blocks                   SEPARATOR
int                                  blockCapacity            SEPARATOR
struct NativeBitmapBlock_Compat*     newBlock                 SEPARATOR
struct MemoryCacheUsageState_Compat* usageState               FINAL_SEPARATOR
{
        MEMORY_CACHE_CheckResetUsageCounts_Compat(gameTime, lastResetGameTime, usageState, blocks);
        return F0491_CACHE_IsDerivedBitmapInCacheAndTouch_Compat(
            derivedBitmapIndex,
            derivedBitmapBlockIndices,
            blocks,
            blockCapacity,
            newBlock,
            usageState);
}

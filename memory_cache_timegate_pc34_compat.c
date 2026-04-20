#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include "memory_cache_timegate_pc34_compat.h"

int MEMORY_CACHE_CheckResetUsageCounts_Compat(
unsigned long                        gameTime           SEPARATOR
unsigned long*                       lastResetGameTime  SEPARATOR
struct MemoryCacheUsageState_Compat* usageState         SEPARATOR
struct NativeBitmapBlock_Compat**    blocks             FINAL_SEPARATOR
{
        if (gameTime == *lastResetGameTime) {
                return 0;
        }
        *lastResetGameTime = gameTime;
        F0485_CACHE_ResetUsageCounts_Compat(usageState, blocks);
        return 1;
}

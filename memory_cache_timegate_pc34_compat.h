#ifndef REDMCSB_MEMORY_CACHE_TIMEGATE_PC34_COMPAT_H
#define REDMCSB_MEMORY_CACHE_TIMEGATE_PC34_COMPAT_H

#include "memory_cache_usage_pc34_compat.h"

int MEMORY_CACHE_CheckResetUsageCounts_Compat(
    unsigned long gameTime,
    unsigned long* lastResetGameTime,
    struct MemoryCacheUsageState_Compat* usageState,
    struct NativeBitmapBlock_Compat** blocks);

#endif

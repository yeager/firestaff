#ifndef REDMCSB_MEMORY_CACHE_SEGMENT_SCAN_PC34_COMPAT_H
#define REDMCSB_MEMORY_CACHE_SEGMENT_SCAN_PC34_COMPAT_H

#include "memory_cache_defrag_loop_pc34_compat.h"

struct MemoryCacheRawBlockHeader_Compat {
    long signedBlockSize;
    int bitmapIndex;
    int isDerivedBitmap;
};

unsigned int MEMORY_CACHE_ScanSegments_Compat(
    const struct MemoryCacheRawBlockHeader_Compat* headers,
    unsigned int headerCount,
    struct MemoryCacheDefragSegment_Compat* outSegments,
    unsigned int maxSegments);

#endif

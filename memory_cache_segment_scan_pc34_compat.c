#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include "memory_cache_segment_scan_pc34_compat.h"

unsigned int MEMORY_CACHE_ScanSegments_Compat(
const struct MemoryCacheRawBlockHeader_Compat* headers      SEPARATOR
unsigned int                                   headerCount  SEPARATOR
struct MemoryCacheDefragSegment_Compat*         outSegments  SEPARATOR
unsigned int                                   maxSegments  FINAL_SEPARATOR
{
        unsigned int i;
        unsigned int outCount;
        long blockSize;


        outCount = 0;
        for (i = 0; i < headerCount; i++) {
                if (outCount >= maxSegments) {
                        break;
                }
                blockSize = headers[i].signedBlockSize;
                if (blockSize == 0) {
                        continue;
                }
                if (blockSize > 0) {
                        outSegments[outCount].isUsed = 0;
                        outSegments[outCount].bitmapIndex = 0;
                        outSegments[outCount].isDerivedBitmap = 0;
                        outSegments[outCount].blockSize = (unsigned long)blockSize;
                } else {
                        outSegments[outCount].isUsed = 1;
                        outSegments[outCount].bitmapIndex = headers[i].bitmapIndex;
                        outSegments[outCount].isDerivedBitmap = headers[i].isDerivedBitmap;
                        outSegments[outCount].blockSize = (unsigned long)(-blockSize);
                }
                outCount++;
        }
        return outCount;
}

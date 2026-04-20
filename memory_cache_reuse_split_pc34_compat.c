#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include "memory_cache_reuse_split_pc34_compat.h"

int MEMORY_CACHE_ReuseOrSplitUnusedBlock_Compat(
unsigned long                         requestedSize   SEPARATOR
unsigned long                         minimumSplitSize SEPARATOR
struct MemoryCacheUnusedBlock_Compat** firstUnusedBlock SEPARATOR
struct MemoryCacheUnusedBlock_Compat* splitRemainder  SEPARATOR
struct MemoryCacheReuseSplitResult_Compat* outResult  FINAL_SEPARATOR
{
        struct MemoryCacheUnusedBlock_Compat* selectedBlock;
        struct MemoryCacheUnusedBlock_Compat* current;
        unsigned long remainderSize;


        if (*firstUnusedBlock == 0) {
                return 0;
        }
        selectedBlock = *firstUnusedBlock;
        current = selectedBlock;
        for (;;) {
                if (current->blockSize == requestedSize) {
                        selectedBlock = current;
                        break;
                }
                if (current->blockSize < requestedSize || current->nextUnusedBlock == 0) {
                        selectedBlock = *firstUnusedBlock;
                        break;
                }
                current = current->nextUnusedBlock;
        }
        F0471_CACHE_RemoveUnusedBlock_Compat(selectedBlock, firstUnusedBlock);
        outResult->selectedBlock = selectedBlock;
        outResult->reusedExistingBlock = 1;
        outResult->exactFit = selectedBlock->blockSize == requestedSize;
        outResult->splitPerformed = 0;
        outResult->allocatedSize = selectedBlock->blockSize;
        remainderSize = selectedBlock->blockSize - requestedSize;
        if (selectedBlock->blockSize >= requestedSize && remainderSize >= minimumSplitSize) {
                outResult->allocatedSize = requestedSize;
                outResult->splitPerformed = MEMORY_CACHE_RegisterSplitUnusedRemainder_Compat(
                    splitRemainder,
                    remainderSize,
                    minimumSplitSize,
                    firstUnusedBlock);
        }
        return 1;
}

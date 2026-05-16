#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include "memory_cache_unused_blocks_pc34_compat.h"

void F0471_CACHE_RemoveUnusedBlock_Compat(
struct MemoryCacheUnusedBlock_Compat*  block            SEPARATOR
struct MemoryCacheUnusedBlock_Compat** firstUnusedBlock FINAL_SEPARATOR
{
        struct MemoryCacheUnusedBlock_Compat* previousUnusedBlock;
        struct MemoryCacheUnusedBlock_Compat* nextUnusedBlock;


        previousUnusedBlock = block->previousUnusedBlock;
        nextUnusedBlock = block->nextUnusedBlock;
        if (previousUnusedBlock == 0) {
                *firstUnusedBlock = nextUnusedBlock;
                if (nextUnusedBlock != 0) {
                        nextUnusedBlock->previousUnusedBlock = 0;
                }
        } else {
                previousUnusedBlock->nextUnusedBlock = nextUnusedBlock;
                if (nextUnusedBlock != 0) {
                        nextUnusedBlock->previousUnusedBlock = previousUnusedBlock;
                }
        }
        block->previousUnusedBlock = 0;
        block->nextUnusedBlock = 0;
}

void F0472_CACHE_AddUnusedBlock_Compat(
struct MemoryCacheUnusedBlock_Compat*  block            SEPARATOR
struct MemoryCacheUnusedBlock_Compat** firstUnusedBlock FINAL_SEPARATOR
{
        struct MemoryCacheUnusedBlock_Compat* current;


        if (*firstUnusedBlock == 0) {
                *firstUnusedBlock = block;
                block->previousUnusedBlock = 0;
                block->nextUnusedBlock = 0;
                return;
        }
        current = *firstUnusedBlock;
        if (block->blockSize >= current->blockSize) {
                *firstUnusedBlock = block;
                block->previousUnusedBlock = 0;
                block->nextUnusedBlock = current;
                current->previousUnusedBlock = block;
                return;
        }
        while (current->nextUnusedBlock != 0) {
                if (block->blockSize >= current->nextUnusedBlock->blockSize) {
                        block->previousUnusedBlock = current;
                        block->nextUnusedBlock = current->nextUnusedBlock;
                        current->nextUnusedBlock->previousUnusedBlock = block;
                        current->nextUnusedBlock = block;
                        return;
                }
                current = current->nextUnusedBlock;
        }
        block->previousUnusedBlock = current;
        block->nextUnusedBlock = 0;
        current->nextUnusedBlock = block;
}

int MEMORY_CACHE_RegisterSplitUnusedRemainder_Compat(
struct MemoryCacheUnusedBlock_Compat*  remainderBlock   SEPARATOR
unsigned long                          remainderSize    SEPARATOR
unsigned long                          minimumSplitSize SEPARATOR
struct MemoryCacheUnusedBlock_Compat** firstUnusedBlock FINAL_SEPARATOR
{
        if (remainderSize < minimumSplitSize) {
                return 0;
        }
        remainderBlock->blockSize = remainderSize;
        remainderBlock->previousUnusedBlock = 0;
        remainderBlock->nextUnusedBlock = 0;
        F0472_CACHE_AddUnusedBlock_Compat(remainderBlock, firstUnusedBlock);
        return 1;
}

#ifndef REDMCSB_MEMORY_CACHE_UNUSED_BLOCKS_PC34_COMPAT_H
#define REDMCSB_MEMORY_CACHE_UNUSED_BLOCKS_PC34_COMPAT_H

struct MemoryCacheUnusedBlock_Compat {
    unsigned long blockSize;
    struct MemoryCacheUnusedBlock_Compat* previousUnusedBlock;
    struct MemoryCacheUnusedBlock_Compat* nextUnusedBlock;
};

void F0471_CACHE_RemoveUnusedBlock_Compat(
    struct MemoryCacheUnusedBlock_Compat* block,
    struct MemoryCacheUnusedBlock_Compat** firstUnusedBlock);

void F0472_CACHE_AddUnusedBlock_Compat(
    struct MemoryCacheUnusedBlock_Compat* block,
    struct MemoryCacheUnusedBlock_Compat** firstUnusedBlock);

int MEMORY_CACHE_RegisterSplitUnusedRemainder_Compat(
    struct MemoryCacheUnusedBlock_Compat* remainderBlock,
    unsigned long remainderSize,
    unsigned long minimumSplitSize,
    struct MemoryCacheUnusedBlock_Compat** firstUnusedBlock);

#endif

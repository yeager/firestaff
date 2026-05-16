#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include "memory_cache_usage_pc34_compat.h"

static unsigned short memory_cache_usage_find_index(
const struct NativeBitmapBlock_Compat* block SEPARATOR
struct NativeBitmapBlock_Compat**       blocks SEPARATOR
int                                     blockCapacity FINAL_SEPARATOR
{
        int i;


        for (i = 0; i < blockCapacity; i++) {
                if (blocks[i] == block) {
                        return (unsigned short)i;
                }
        }
        return MEMORY_CACHE_INDEX_NONE;
}

void F0485_CACHE_ResetUsageCounts_Compat(
struct MemoryCacheUsageState_Compat* state  SEPARATOR
struct NativeBitmapBlock_Compat**    blocks FINAL_SEPARATOR
{
        struct NativeBitmapBlock_Compat* block;
        unsigned short index;


        state->firstReferencedUsedBlock = 0;
        block = state->lastUsedBlock;
        if (block == 0) {
                return;
        }
        while (block->usageCount != 0) {
                block->usageCount = 0;
                index = block->previousIndex;
                if (index == MEMORY_CACHE_INDEX_NONE) {
                        break;
                }
                block = blocks[index];
        }
}

void F0486_MEMORY_AddBlockToUsedList_Compat(
unsigned short                      blockIndex    SEPARATOR
struct MemoryCacheUsageState_Compat* state        SEPARATOR
struct NativeBitmapBlock_Compat**    blocks       SEPARATOR
int                                  blockCapacity FINAL_SEPARATOR
{
        struct NativeBitmapBlock_Compat* blockToAdd;
        struct NativeBitmapBlock_Compat* previousBlock;
        unsigned short previousIndex;
        (void)blockCapacity;


        blockToAdd = blocks[blockIndex];
        blockToAdd->usageCount = 1;
        if (state->firstReferencedUsedBlock == 0) {
                blockToAdd->nextIndex = MEMORY_CACHE_INDEX_NONE;
                if (state->lastUsedBlock == 0) {
                        blockToAdd->previousIndex = MEMORY_CACHE_INDEX_NONE;
                        state->firstUsedBlock = blockToAdd;
                } else {
                        previousIndex = memory_cache_usage_find_index(state->lastUsedBlock, blocks, blockCapacity);
                        state->lastUsedBlock->nextIndex = blockIndex;
                        blockToAdd->previousIndex = previousIndex;
                }
                state->lastUsedBlock = blockToAdd;
        } else {
                previousIndex = state->firstReferencedUsedBlock->previousIndex;
                state->firstReferencedUsedBlock->previousIndex = blockIndex;
                blockToAdd->previousIndex = previousIndex;
                if (previousIndex != MEMORY_CACHE_INDEX_NONE) {
                        previousBlock = blocks[previousIndex];
                        blockToAdd->nextIndex = previousBlock->nextIndex;
                        previousBlock->nextIndex = blockIndex;
                } else {
                        blockToAdd->nextIndex = memory_cache_usage_find_index(state->firstReferencedUsedBlock, blocks, blockCapacity);
                        state->firstUsedBlock = blockToAdd;
                }
        }
        state->firstReferencedUsedBlock = blockToAdd;
}

struct NativeBitmapBlock_Compat* F0487_CACHE_GetBlockAndIncrementUsageCount_Compat(
unsigned short                       blockIndex    SEPARATOR
struct MemoryCacheUsageState_Compat* state         SEPARATOR
struct NativeBitmapBlock_Compat**    blocks        SEPARATOR
int                                  blockCapacity FINAL_SEPARATOR
{
        struct NativeBitmapBlock_Compat* block;
        struct NativeBitmapBlock_Compat* nextBlock;
        struct NativeBitmapBlock_Compat* prevBlock;
        unsigned short previousIndex;
        unsigned short nextIndex;
        unsigned short usageCount;
        unsigned short insertAfterIndex;


        block = blocks[blockIndex];
        usageCount = block->usageCount;
        if (usageCount == 0) {
                nextIndex = block->nextIndex;
                if (nextIndex == MEMORY_CACHE_INDEX_NONE) {
                        state->firstReferencedUsedBlock = block;
                        block->usageCount = 1;
                        return block;
                }
                nextBlock = blocks[nextIndex];
                previousIndex = block->previousIndex;
                if (previousIndex == MEMORY_CACHE_INDEX_NONE) {
                        state->firstUsedBlock = nextBlock;
                        nextBlock->previousIndex = MEMORY_CACHE_INDEX_NONE;
                } else {
                        prevBlock = blocks[previousIndex];
                        prevBlock->nextIndex = nextIndex;
                        nextBlock->previousIndex = previousIndex;
                }
                F0486_MEMORY_AddBlockToUsedList_Compat(blockIndex, state, blocks, blockCapacity);
                return block;
        }
        usageCount = (unsigned short)(usageCount + 1);
        block->usageCount = usageCount;
        if (block == state->lastUsedBlock) {
            return block;
        }
        nextIndex = block->nextIndex;
        nextBlock = blocks[nextIndex];
        if (nextBlock->usageCount >= usageCount) {
                return block;
        }
        previousIndex = block->previousIndex;
        if (previousIndex == MEMORY_CACHE_INDEX_NONE) {
                state->firstUsedBlock = state->firstReferencedUsedBlock = nextBlock;
                nextBlock->previousIndex = MEMORY_CACHE_INDEX_NONE;
        } else {
                if (state->firstReferencedUsedBlock == block) {
                        state->firstReferencedUsedBlock = nextBlock;
                }
                prevBlock = blocks[previousIndex];
                prevBlock->nextIndex = nextIndex;
                nextBlock->previousIndex = previousIndex;
        }
        for (;;) {
                insertAfterIndex = nextIndex;
                block = nextBlock;
                nextIndex = block->nextIndex;
                if (nextIndex == MEMORY_CACHE_INDEX_NONE) {
                        block->nextIndex = blockIndex;
                        blocks[blockIndex]->previousIndex = insertAfterIndex;
                        blocks[blockIndex]->nextIndex = MEMORY_CACHE_INDEX_NONE;
                        state->lastUsedBlock = blocks[blockIndex];
                        return blocks[blockIndex];
                }
                nextBlock = blocks[nextIndex];
                if (nextBlock->usageCount >= usageCount) {
                        block->nextIndex = blockIndex;
                        blocks[blockIndex]->previousIndex = insertAfterIndex;
                        blocks[blockIndex]->nextIndex = nextIndex;
                        nextBlock->previousIndex = blockIndex;
                        return blocks[blockIndex];
                }
        }
}

/* ══════════════════════════════════════════════════════════════════════
 * Pass601 — Cache usage tracking (MEMORY.C:2308-2473)
 *
 * F0485_CACHE_ResetUsageCounts (MEMORY.C:2308-2329):
 *   - Resets all block usage counters to 0
 *   - Called at game loop boundaries (F0002_MAIN_GameLoop_CPSDF)
 *
 * F0486_MEMORY_AddBlockToUsedList (MEMORY.C:2331-2379):
 *   - Inserts block into used list sorted by usage count
 *   - Higher usage = more recently used = evicted last
 *
 * F0487_CACHE_GetBlockAndIncrementUsageCount (MEMORY.C:2381-2473):
 *   - Retrieves cached graphic block
 *   - Increments usage counter
 *   - If not cached: loads from GRAPHICS.DAT, decompresses, caches
 *   - Calls F0474_MEMORY_LoadGraphic_CPSDF for disk load
 *   - Calls F0483_CACHE_GetNewBlock for cache allocation
 * ══════════════════════════════════════════════════════════════════════ */

const char *dm1_memory_pass601_usage_source_evidence(void)
{
    return
        "MEMORY.C:2308-2329 F0485_CACHE_ResetUsageCounts\n"
        "MEMORY.C:2331-2379 F0486_MEMORY_AddBlockToUsedList sorted\n"
        "MEMORY.C:2381-2473 F0487_CACHE_GetBlockAndIncrementUsageCount\n"
        "MEMORY.C:707-831 F0474_MEMORY_LoadGraphic_CPSDF disk→cache\n";
}


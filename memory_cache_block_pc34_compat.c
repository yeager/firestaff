#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include "memory_cache_block_pc34_compat.h"
#include "memory_frontend_pc34_compat.h"

unsigned char* F0489_MEMORY_PrepareNativeBitmapBlock_Compat(
struct NativeBitmapBlock_Compat*        block       SEPARATOR
unsigned short                          bitmapIndex SEPARATOR
const struct GraphicWidthHeight_Compat* sizeInfo    FINAL_SEPARATOR
{
        block->bitmapIndex = bitmapIndex;
        if (sizeInfo != 0) {
                block->width = sizeInfo->Width;
                block->height = sizeInfo->Height;
        } else {
                block->width = 0;
                block->height = 0;
        }
        return block->bitmap;
}

/* ══════════════════════════════════════════════════════════════════════
 * Pass601 — Cache block management (MEMORY.C:454-551)
 *
 * F0471_CACHE_RemoveUnusedBlock (MEMORY.C:454-484):
 *   - Removes block from unused list
 *   - Doubly-linked list: prev/next pointer update
 *   - Decrements G0641_ui_CacheUnusedCount
 *
 * F0472_CACHE_AddUnusedBlock (MEMORY.C:486-551):
 *   - Adds block to unused list in sorted order (by address)
 *   - Coalesces adjacent free blocks (MEMORY.C:500-520)
 *   - Increments G0641_ui_CacheUnusedCount
 *   - Adjacent block merge: if (next->start == this->end) merge
 * ══════════════════════════════════════════════════════════════════════ */

const char *dm1_memory_pass601_block_source_evidence(void)
{
    return
        "MEMORY.C:454-484 F0471_CACHE_RemoveUnusedBlock doubly-linked\n"
        "MEMORY.C:486-551 F0472_CACHE_AddUnusedBlock sorted+coalesce\n"
        "MEMORY.C:500-520 adjacent free block merge\n";
}


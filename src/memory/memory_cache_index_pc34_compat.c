#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include "memory_cache_index_pc34_compat.h"
#include "memory_cache_block_pc34_compat.h"
#include "memory_cache_reuse_pc34_compat.h"

unsigned char* F0489_MEMORY_GetNativeBitmapByIndex_Compat(
int                                    graphicIndex            SEPARATOR
const unsigned char*                   graphic                 SEPARATOR
unsigned short*                        nativeBitmapBlockIndices SEPARATOR
struct NativeBitmapBlock_Compat**      blocks                  SEPARATOR
int                                    blockCapacity           SEPARATOR
struct NativeBitmapBlock_Compat*       newBlock                SEPARATOR
const struct GraphicWidthHeight_Compat* sizeInfo              FINAL_SEPARATOR
{
        unsigned short blockIndex;
        int i;
        unsigned char* bitmap;


        blockIndex = nativeBitmapBlockIndices[graphicIndex];
        if (blockIndex != 0xFFFF && blockIndex < (unsigned short)blockCapacity && blocks[blockIndex] != 0) {
                return blocks[blockIndex]->bitmap;
        }
        for (i = 0; i < blockCapacity; i++) {
                if (blocks[i] == 0) {
                        blocks[i] = newBlock;
                        nativeBitmapBlockIndices[graphicIndex] = (unsigned short)i;
                        bitmap = F0489_MEMORY_PrepareNativeBitmapBlock_Compat(newBlock, (unsigned short)graphicIndex, sizeInfo);
                        return F0489_MEMORY_GetNativeBitmapOrGraphic_Compat(graphic, 0, bitmap, sizeInfo);
                }
        }
        return 0;
}

/* ══════════════════════════════════════════════════════════════════════
 * Pass601 — Memory initialization (MEMORY.C:832-1211)
 *
 * F0475_MEMORY_Initialize (MEMORY.C:832-940):
 *   - Allocates core structures: events, timeline, active groups
 *   - Sets up heap boundaries (bottom=permanent, top=temporary)
 *   - Allocates floppy disk read buffer (1024-4520 bytes)
 *   - Allocates G0655_ppuc_Blocks pointer array
 *   - Allocates derived bitmap index + byte count arrays
 *
 * F0476_MEMORY_InitializeGraphicMemory (MEMORY.C:941-1211):
 *   - Allocates wall bitmap sets (flipped variants)
 *   - Platform-specific: PC/Amiga/Atari have different allocations
 *   - Calls F0461_START_AllocateFlippedWallBitmaps per platform
 *   - Sets up viewport bitmap pointers
 *
 * F0609_InitializeMemoryArea (MEMORY.C:1723-1733):
 *   - Initializes a memory area descriptor (base, size, free)
 *
 * F0610_DefragmentMemoryArea (MEMORY.C:1735-1766):
 *   - Compacts a memory area, returns pointer to free space
 * ══════════════════════════════════════════════════════════════════════ */

const char *dm1_memory_pass601_init_source_evidence(void)
{
    return
        "MEMORY.C:832-940 F0475_MEMORY_Initialize core structures\n"
        "MEMORY.C:941-1211 F0476_MEMORY_InitializeGraphicMemory wall/viewport\n"
        "MEMORY.C:1723-1733 F0609_InitializeMemoryArea\n"
        "MEMORY.C:1735-1766 F0610_DefragmentMemoryArea\n";
}


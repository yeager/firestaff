#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include <string.h>
#include "memory_graphics_dat_bitmap_path_pc34_compat.h"
#include "memory_load_expand_pc34_compat.h"

unsigned char* F0489_MEMORY_LoadNativeBitmapByIndex_Compat(
const char*                                      graphicsDatPath    SEPARATOR
const struct MemoryGraphicsDatRuntimeState_Compat* runtimeState     SEPARATOR
struct MemoryGraphicsDatState_Compat*            fileState          SEPARATOR
unsigned int                                     graphicIndex       SEPARATOR
unsigned char*                                   loadedGraphicBuffer SEPARATOR
unsigned char*                                   ownedBitmap        SEPARATOR
struct MemoryGraphicsDatBitmapPathResult_Compat* outResult          FINAL_SEPARATOR
{
        struct MemoryGraphicsDatTransactionResult_Compat transaction;
        struct MemoryGraphicsDatSelection_Compat selection;


        memset(&transaction, 0, sizeof(transaction));
        memset(&selection, 0, sizeof(selection));
        if (!F0490_MEMORY_LoadViewportGraphicByIndex_Compat(
                graphicsDatPath,
                runtimeState,
                fileState,
                graphicIndex,
                MEMORY_LOAD_EXPAND_FLAG_NOT_EXPANDED,
                0,
                loadedGraphicBuffer,
                &transaction,
                &selection)) {
                return 0;
        }
        if (outResult != 0) {
                memset(outResult, 0, sizeof(*outResult));
                outResult->transaction = transaction;
                outResult->selection = selection;
                outResult->loadedGraphic = loadedGraphicBuffer;
        }
        ownedBitmap = F0489_MEMORY_BuildNativeBitmapInPlace_Compat(
            loadedGraphicBuffer,
            ownedBitmap,
            &selection.widthHeight);
        if (outResult != 0) {
                outResult->bitmap = ownedBitmap;
        }
        return ownedBitmap;
}

/* ══════════════════════════════════════════════════════════════════════
 * Pass601 — Bitmap expand/native path (MEMORY.C:2474-2640)
 *
 * F0488_MEMORY_ExpandGraphicToBitmap (MEMORY.C:2474-2503):
 *   - Expands compressed graphic to full bitmap format
 *   - Handles palette conversion and bit-depth expansion
 *
 * F0489_MEMORY_GetNativeBitmapOrGraphic (MEMORY.C:2505-2582):
 *   - Returns cached bitmap if available
 *   - Otherwise loads from GRAPHICS.DAT, decompresses, caches
 *   - If block in cache: increment usage, return pointer
 *   - If not: F0487_CACHE_GetBlockAndIncrementUsageCount
 *   - KEY FUNCTION: all viewport/wall/creature rendering goes through this
 *
 * F0490_MEMORY_LoadDecompressAndExpandGraphic (MEMORY.C:2583-2640):
 *   - Full pipeline: open DAT → read header → decompress → expand → cache
 *   - Called for initial graphic loading and on-demand loading
 *   - Parameters control whether to copy dimensions, expand, or keep raw
 * ══════════════════════════════════════════════════════════════════════ */

const char *dm1_memory_pass601_bitmap_source_evidence(void)
{
    return
        "MEMORY.C:2474-2503 F0488_MEMORY_ExpandGraphicToBitmap\n"
        "MEMORY.C:2505-2582 F0489_MEMORY_GetNativeBitmapOrGraphic render path\n"
        "MEMORY.C:2583-2640 F0490_MEMORY_LoadDecompressAndExpandGraphic full pipeline\n";
}


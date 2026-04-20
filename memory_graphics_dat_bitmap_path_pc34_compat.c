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

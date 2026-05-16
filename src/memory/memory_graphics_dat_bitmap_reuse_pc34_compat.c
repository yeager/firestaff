#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include <string.h>
#include "memory_graphics_dat_bitmap_reuse_pc34_compat.h"

static int select_from_runtime_state(
const struct MemoryGraphicsDatRuntimeState_Compat* runtimeState SEPARATOR
unsigned int                                       graphicIndex SEPARATOR
struct MemoryGraphicsDatSelection_Compat*          selection    FINAL_SEPARATOR
{
        struct MemoryGraphicsDatHeader_Compat header;


        memset(&header, 0, sizeof(header));
        header.format = runtimeState->format;
        header.graphicCount = runtimeState->graphicCount;
        header.compressedByteCounts = runtimeState->compressedByteCounts;
        header.decompressedByteCounts = runtimeState->decompressedByteCounts;
        header.widthHeight = runtimeState->widthHeight;
        header.fileSize = runtimeState->fileSize;
        return F0490_MEMORY_SelectGraphicFromHeader_Compat(&header, graphicIndex, selection);
}

unsigned char* F0489_MEMORY_GetOrLoadNativeBitmapByIndex_Compat(
const char*                                      graphicsDatPath     SEPARATOR
const struct MemoryGraphicsDatRuntimeState_Compat* runtimeState      SEPARATOR
struct MemoryGraphicsDatState_Compat*            fileState           SEPARATOR
unsigned int                                     graphicIndex        SEPARATOR
unsigned char*                                   loadedGraphicBuffer SEPARATOR
unsigned char*                                   cachedBitmap        SEPARATOR
unsigned char*                                   targetBitmap        SEPARATOR
struct MemoryGraphicsDatBitmapPathResult_Compat* outResult           FINAL_SEPARATOR
{
        struct MemoryGraphicsDatSelection_Compat selection;


        memset(&selection, 0, sizeof(selection));
        if (!select_from_runtime_state(runtimeState, graphicIndex, &selection)) {
                return 0;
        }
        if (cachedBitmap != 0) {
                if (outResult != 0) {
                        memset(outResult, 0, sizeof(*outResult));
                        outResult->selection = selection;
                        outResult->bitmap = cachedBitmap;
                        outResult->loadedGraphic = loadedGraphicBuffer;
                }
                return F0489_MEMORY_GetNativeBitmapOrGraphic_Compat(
                    loadedGraphicBuffer,
                    cachedBitmap,
                    targetBitmap,
                    &selection.widthHeight);
        }
        return F0489_MEMORY_LoadNativeBitmapByIndex_Compat(
            graphicsDatPath,
            runtimeState,
            fileState,
            graphicIndex,
            loadedGraphicBuffer,
            targetBitmap,
            outResult);
}

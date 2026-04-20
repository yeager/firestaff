#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include <string.h>
#include "memory_load_session_pc34_compat.h"

void F0490_MEMORY_LoadGraphicSession_Compat(
int                                 graphicIndexFlags    SEPARATOR
const unsigned char*                loadedGraphic        SEPARATOR
unsigned long                       loadedByteCount      SEPARATOR
unsigned char*                      viewportGraphicBuffer SEPARATOR
unsigned char*                      destinationBitmap    SEPARATOR
const struct GraphicWidthHeight_Compat* sizeInfo         SEPARATOR
struct MemoryLoadSessionResult_Compat* outResult         FINAL_SEPARATOR
{
        if ((graphicIndexFlags & MEMORY_LOAD_EXPAND_FLAG_NOT_EXPANDED) != 0) {
                F0490_MEMORY_ApplyLoadedGraphic_Compat(
                    graphicIndexFlags,
                    loadedGraphic,
                    loadedByteCount,
                    destinationBitmap,
                    sizeInfo);
                outResult->loadTarget = destinationBitmap;
                outResult->usedViewportBuffer = 0;
                outResult->drawFloorAndCeilingRequested = 0;
                return;
        }
        memcpy(viewportGraphicBuffer, loadedGraphic, loadedByteCount);
        F0490_MEMORY_ApplyLoadedGraphic_Compat(
            graphicIndexFlags,
            viewportGraphicBuffer,
            loadedByteCount,
            destinationBitmap,
            sizeInfo);
        outResult->loadTarget = viewportGraphicBuffer;
        outResult->usedViewportBuffer = 1;
        outResult->drawFloorAndCeilingRequested = 1;
}

#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include "memory_graphics_dat_transaction_pc34_compat.h"
#include "memory_load_expand_pc34_compat.h"

int F0490_MEMORY_RunGraphicsDatTransaction_Compat(
const char*                              graphicsDatPath       SEPARATOR
long                                     graphicOffset         SEPARATOR
int                                      compressedByteCount   SEPARATOR
int                                      graphicIndexFlags     SEPARATOR
struct MemoryGraphicsDatState_Compat*    state                 SEPARATOR
unsigned char*                           viewportGraphicBuffer SEPARATOR
unsigned char*                           destinationBitmap     SEPARATOR
const struct GraphicWidthHeight_Compat*  sizeInfo              SEPARATOR
struct MemoryGraphicsDatTransactionResult_Compat* outResult    FINAL_SEPARATOR
{
        unsigned char* loadTarget;


        if (!F0477_MEMORY_OpenGraphicsDat_CPSDF_Compat(graphicsDatPath, state)) {
                return 0;
        }
        outResult->graphicsOpened = 1;
        loadTarget = ((graphicIndexFlags & MEMORY_LOAD_EXPAND_FLAG_NOT_EXPANDED) != 0) ? destinationBitmap : viewportGraphicBuffer;
        if (!F0474_MEMORY_LoadGraphic_CPSDF_Compat(graphicOffset, compressedByteCount, state, loadTarget)) {
                F0478_MEMORY_CloseGraphicsDat_CPSDF_Compat(state);
                outResult->graphicsClosed = 1;
                return 0;
        }
        if ((graphicIndexFlags & MEMORY_LOAD_EXPAND_FLAG_NOT_EXPANDED) != 0) {
                outResult->session.loadTarget = destinationBitmap;
                outResult->session.usedViewportBuffer = 0;
                outResult->session.drawFloorAndCeilingRequested = 0;
        } else {
                F0490_MEMORY_ApplyLoadedGraphic_Compat(
                    graphicIndexFlags,
                    viewportGraphicBuffer,
                    (unsigned long)compressedByteCount,
                    destinationBitmap,
                    sizeInfo);
                outResult->session.loadTarget = viewportGraphicBuffer;
                outResult->session.usedViewportBuffer = 1;
                outResult->session.drawFloorAndCeilingRequested = 1;
        }
        if (!F0478_MEMORY_CloseGraphicsDat_CPSDF_Compat(state)) {
                return 0;
        }
        outResult->graphicsClosed = 1;
        return 1;
}

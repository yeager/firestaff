#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include "memory_graphics_dat_viewport_path_pc34_compat.h"

int F0490_MEMORY_LoadViewportGraphicByIndex_Compat(
const char*                                     graphicsDatPath       SEPARATOR
const struct MemoryGraphicsDatRuntimeState_Compat* runtimeState       SEPARATOR
struct MemoryGraphicsDatState_Compat*           fileState             SEPARATOR
unsigned int                                    graphicIndex          SEPARATOR
int                                             graphicIndexFlags     SEPARATOR
unsigned char*                                  viewportGraphicBuffer SEPARATOR
unsigned char*                                  destinationBitmap     SEPARATOR
struct MemoryGraphicsDatTransactionResult_Compat* outResult           SEPARATOR
struct MemoryGraphicsDatSelection_Compat*       outSelection          FINAL_SEPARATOR
{
        return F0490_MEMORY_RunRuntimeGraphicsDatTransaction_Compat(
            graphicsDatPath,
            runtimeState,
            graphicIndex,
            graphicIndexFlags,
            fileState,
            viewportGraphicBuffer,
            destinationBitmap,
            outResult,
            outSelection);
}

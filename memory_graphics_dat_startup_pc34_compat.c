#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include <string.h>
#include "memory_graphics_dat_startup_pc34_compat.h"

void F0479_MEMORY_FreeStartupGraphicsChain_Compat(
struct MemoryGraphicsDatStartupResult_Compat* state FINAL_SEPARATOR
{
        F0479_MEMORY_FreeSpecialGraphics_Compat(&state->specials);
        F0479_MEMORY_FreeGraphicsDatState_Compat(&state->runtimeState);
        memset(state, 0, sizeof(*state));
}

int F0479_MEMORY_StartupGraphicsChain_Compat(
const char*                                  graphicsDatPath      SEPARATOR
struct MemoryGraphicsDatState_Compat*        fileState            SEPARATOR
unsigned int                                 dialogGraphicIndex   SEPARATOR
unsigned int                                 viewportGraphicIndex SEPARATOR
unsigned char*                               viewportGraphicBuffer SEPARATOR
unsigned char*                               viewportBitmap       SEPARATOR
struct MemoryGraphicsDatStartupResult_Compat* outResult          FINAL_SEPARATOR
{
        memset(outResult, 0, sizeof(*outResult));
        if (!F0479_MEMORY_InitializeGraphicsDatState_Compat(
                graphicsDatPath,
                fileState,
                &outResult->runtimeState)) {
                return 0;
        }
        outResult->runtimeInitialized = 1;
        if (!F0479_MEMORY_PreloadDialogBoxGraphic_Compat(
                graphicsDatPath,
                &outResult->runtimeState,
                fileState,
                dialogGraphicIndex,
                &outResult->specials)) {
                F0479_MEMORY_FreeStartupGraphicsChain_Compat(outResult);
                return 0;
        }
        outResult->dialogPreloaded = 1;
        if (!F0490_MEMORY_LoadViewportGraphicByIndex_Compat(
                graphicsDatPath,
                &outResult->runtimeState,
                fileState,
                viewportGraphicIndex,
                0,
                viewportGraphicBuffer,
                viewportBitmap,
                &outResult->viewportTransaction,
                &outResult->viewportSelection)) {
                F0479_MEMORY_FreeStartupGraphicsChain_Compat(outResult);
                return 0;
        }
        outResult->viewportLoaded = 1;
        return 1;
}

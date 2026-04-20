#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include <string.h>
#include "memory_graphics_dat_boot_pc34_compat.h"

void F0479_MEMORY_FreeBootInitMini_Compat(
struct MemoryGraphicsDatBootResult_Compat* result FINAL_SEPARATOR
{
        F0479_MEMORY_FreeStartupGraphicsChain_Compat(&result->startup);
        memset(result, 0, sizeof(*result));
}

int F0479_MEMORY_RunBootInitMini_Compat(
const char*                               graphicsDatPath      SEPARATOR
struct MemoryGraphicsDatState_Compat*     fileState            SEPARATOR
unsigned int                              dialogGraphicIndex   SEPARATOR
unsigned int                              viewportGraphicIndex SEPARATOR
unsigned char*                            viewportGraphicBuffer SEPARATOR
unsigned char*                            viewportBitmap       SEPARATOR
struct MemoryGraphicsDatBootResult_Compat* outResult          FINAL_SEPARATOR
{
        memset(outResult, 0, sizeof(*outResult));
        outResult->stage = MEMORY_GRAPHICS_DAT_BOOT_STAGE_NOT_STARTED;
        if (!F0479_MEMORY_StartupGraphicsChain_Compat(
                graphicsDatPath,
                fileState,
                dialogGraphicIndex,
                viewportGraphicIndex,
                viewportGraphicBuffer,
                viewportBitmap,
                &outResult->startup)) {
                return 0;
        }
        outResult->stage = MEMORY_GRAPHICS_DAT_BOOT_STAGE_RUNTIME_READY;
        if (outResult->startup.runtimeInitialized) {
                outResult->stage = MEMORY_GRAPHICS_DAT_BOOT_STAGE_RUNTIME_READY;
        }
        if (outResult->startup.dialogPreloaded) {
                outResult->stage = MEMORY_GRAPHICS_DAT_BOOT_STAGE_SPECIAL_READY;
        }
        if (outResult->startup.viewportLoaded) {
                outResult->stage = MEMORY_GRAPHICS_DAT_BOOT_STAGE_VIEWPORT_READY;
        }
        outResult->stage = MEMORY_GRAPHICS_DAT_BOOT_STAGE_COMPLETE;
        outResult->succeeded = 1;
        return 1;
}

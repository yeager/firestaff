#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include <string.h>
#include "memory_graphics_dat_startup_wiring_pc34_compat.h"

void F0479_MEMORY_FreeStartupWiringMini_Compat(
struct MemoryGraphicsDatStartupWiringResult_Compat* result FINAL_SEPARATOR
{
        F0479_MEMORY_FreeBootInitMini_Compat(&result->boot);
        memset(result, 0, sizeof(*result));
}

int F0479_MEMORY_RunStartupWiringMini_Compat(
const char*                                        graphicsDatPath      SEPARATOR
struct MemoryGraphicsDatState_Compat*              fileState            SEPARATOR
unsigned int                                       dialogGraphicIndex   SEPARATOR
unsigned int                                       viewportGraphicIndex SEPARATOR
unsigned char*                                     viewportGraphicBuffer SEPARATOR
unsigned char*                                     viewportBitmap       SEPARATOR
struct MemoryGraphicsDatStartupWiringResult_Compat* outResult          FINAL_SEPARATOR
{
        memset(outResult, 0, sizeof(*outResult));
        if (!F0479_MEMORY_RunBootInitMini_Compat(
                graphicsDatPath,
                fileState,
                dialogGraphicIndex,
                viewportGraphicIndex,
                viewportGraphicBuffer,
                viewportBitmap,
                &outResult->boot)) {
                return 0;
        }
        outResult->runtimeAssetsReady = outResult->boot.startup.runtimeInitialized;
        outResult->persistentSpecialsReady = outResult->boot.startup.dialogPreloaded;
        outResult->viewportPathReady = outResult->boot.startup.viewportLoaded;
        outResult->firstFramePrerequisitesReady =
            outResult->boot.succeeded &&
            outResult->runtimeAssetsReady &&
            outResult->persistentSpecialsReady &&
            outResult->viewportPathReady &&
            (outResult->boot.stage == MEMORY_GRAPHICS_DAT_BOOT_STAGE_COMPLETE);
        return outResult->firstFramePrerequisitesReady;
}

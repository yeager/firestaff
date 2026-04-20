#ifndef REDMCSB_MEMORY_GRAPHICS_DAT_STARTUP_WIRING_PC34_COMPAT_H
#define REDMCSB_MEMORY_GRAPHICS_DAT_STARTUP_WIRING_PC34_COMPAT_H

#include "memory_graphics_dat_boot_pc34_compat.h"

struct MemoryGraphicsDatStartupWiringResult_Compat {
    struct MemoryGraphicsDatBootResult_Compat boot;
    int runtimeAssetsReady;
    int persistentSpecialsReady;
    int viewportPathReady;
    int firstFramePrerequisitesReady;
};

int F0479_MEMORY_RunStartupWiringMini_Compat(
    const char* graphicsDatPath,
    struct MemoryGraphicsDatState_Compat* fileState,
    unsigned int dialogGraphicIndex,
    unsigned int viewportGraphicIndex,
    unsigned char* viewportGraphicBuffer,
    unsigned char* viewportBitmap,
    struct MemoryGraphicsDatStartupWiringResult_Compat* outResult);

void F0479_MEMORY_FreeStartupWiringMini_Compat(
    struct MemoryGraphicsDatStartupWiringResult_Compat* result);

#endif

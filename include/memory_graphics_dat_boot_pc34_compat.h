#ifndef REDMCSB_MEMORY_GRAPHICS_DAT_BOOT_PC34_COMPAT_H
#define REDMCSB_MEMORY_GRAPHICS_DAT_BOOT_PC34_COMPAT_H

#include "memory_graphics_dat_startup_pc34_compat.h"

enum MemoryGraphicsDatBootStage_Compat {
    MEMORY_GRAPHICS_DAT_BOOT_STAGE_NOT_STARTED = 0,
    MEMORY_GRAPHICS_DAT_BOOT_STAGE_RUNTIME_READY = 1,
    MEMORY_GRAPHICS_DAT_BOOT_STAGE_SPECIAL_READY = 2,
    MEMORY_GRAPHICS_DAT_BOOT_STAGE_VIEWPORT_READY = 3,
    MEMORY_GRAPHICS_DAT_BOOT_STAGE_COMPLETE = 4
};

struct MemoryGraphicsDatBootResult_Compat {
    enum MemoryGraphicsDatBootStage_Compat stage;
    struct MemoryGraphicsDatStartupResult_Compat startup;
    int succeeded;
};

int F0479_MEMORY_RunBootInitMini_Compat(
    const char* graphicsDatPath,
    struct MemoryGraphicsDatState_Compat* fileState,
    unsigned int dialogGraphicIndex,
    unsigned int viewportGraphicIndex,
    unsigned char* viewportGraphicBuffer,
    unsigned char* viewportBitmap,
    struct MemoryGraphicsDatBootResult_Compat* outResult);

void F0479_MEMORY_FreeBootInitMini_Compat(
    struct MemoryGraphicsDatBootResult_Compat* result);

#endif

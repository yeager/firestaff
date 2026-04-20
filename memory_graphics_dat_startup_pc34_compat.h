#ifndef REDMCSB_MEMORY_GRAPHICS_DAT_STARTUP_PC34_COMPAT_H
#define REDMCSB_MEMORY_GRAPHICS_DAT_STARTUP_PC34_COMPAT_H

#include "memory_graphics_dat_special_pc34_compat.h"
#include "memory_graphics_dat_viewport_path_pc34_compat.h"

struct MemoryGraphicsDatStartupResult_Compat {
    struct MemoryGraphicsDatRuntimeState_Compat runtimeState;
    struct MemoryGraphicsDatSpecials_Compat specials;
    struct MemoryGraphicsDatTransactionResult_Compat viewportTransaction;
    struct MemoryGraphicsDatSelection_Compat viewportSelection;
    int runtimeInitialized;
    int dialogPreloaded;
    int viewportLoaded;
};

int F0479_MEMORY_StartupGraphicsChain_Compat(
    const char* graphicsDatPath,
    struct MemoryGraphicsDatState_Compat* fileState,
    unsigned int dialogGraphicIndex,
    unsigned int viewportGraphicIndex,
    unsigned char* viewportGraphicBuffer,
    unsigned char* viewportBitmap,
    struct MemoryGraphicsDatStartupResult_Compat* outResult);

void F0479_MEMORY_FreeStartupGraphicsChain_Compat(
    struct MemoryGraphicsDatStartupResult_Compat* state);

#endif

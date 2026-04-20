#ifndef REDMCSB_MEMORY_GRAPHICS_DAT_DIALOG_PATH_PC34_COMPAT_H
#define REDMCSB_MEMORY_GRAPHICS_DAT_DIALOG_PATH_PC34_COMPAT_H

#include "memory_graphics_dat_special_pc34_compat.h"

int F0427_DIALOG_DrawPreloadedBackdrop_Compat(
    const char* graphicsDatPath,
    const struct MemoryGraphicsDatRuntimeState_Compat* runtimeState,
    struct MemoryGraphicsDatState_Compat* fileState,
    unsigned int dialogGraphicIndex,
    struct MemoryGraphicsDatSpecials_Compat* specials,
    unsigned char* viewportBitmap);

#endif

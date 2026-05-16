#ifndef REDMCSB_MEMORY_GRAPHICS_DAT_SPECIAL_PC34_COMPAT_H
#define REDMCSB_MEMORY_GRAPHICS_DAT_SPECIAL_PC34_COMPAT_H

#include "memory_graphics_dat_slots_pc34_compat.h"

struct MemoryGraphicsDatSpecials_Compat {
    unsigned char* dialogBoxGraphic;
    unsigned short dialogBoxAllocatedByteCount;
    unsigned short dialogBoxLoadedByteCount;
};

int F0479_MEMORY_PreloadDialogBoxGraphic_Compat(
    const char* graphicsDatPath,
    const struct MemoryGraphicsDatRuntimeState_Compat* runtimeState,
    struct MemoryGraphicsDatState_Compat* fileState,
    unsigned int dialogGraphicIndex,
    struct MemoryGraphicsDatSpecials_Compat* specials);

void F0479_MEMORY_FreeSpecialGraphics_Compat(
    struct MemoryGraphicsDatSpecials_Compat* specials);

#endif

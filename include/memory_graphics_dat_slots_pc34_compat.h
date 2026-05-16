#ifndef REDMCSB_MEMORY_GRAPHICS_DAT_SLOTS_PC34_COMPAT_H
#define REDMCSB_MEMORY_GRAPHICS_DAT_SLOTS_PC34_COMPAT_H

#include "memory_graphics_dat_runtime_transaction_pc34_compat.h"

struct MemoryGraphicsDatSlots_Compat {
    unsigned int graphicCount;
    unsigned char** graphics;
    unsigned short* loadedByteCounts;
};

int F0484_MEMORY_InitializeGraphicSlots_Compat(
    unsigned int graphicCount,
    struct MemoryGraphicsDatSlots_Compat* slots);

void F0484_MEMORY_FreeGraphicSlots_Compat(
    struct MemoryGraphicsDatSlots_Compat* slots);

unsigned char* F0484_MEMORY_PreloadGraphicSlot_Compat(
    const char* graphicsDatPath,
    const struct MemoryGraphicsDatRuntimeState_Compat* runtimeState,
    struct MemoryGraphicsDatState_Compat* fileState,
    unsigned int graphicIndex,
    struct MemoryGraphicsDatSlots_Compat* slots,
    struct MemoryGraphicsDatSelection_Compat* outSelection);

#endif

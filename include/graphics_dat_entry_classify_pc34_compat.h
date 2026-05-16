#ifndef REDMCSB_GRAPHICS_DAT_ENTRY_CLASSIFY_PC34_COMPAT_H
#define REDMCSB_GRAPHICS_DAT_ENTRY_CLASSIFY_PC34_COMPAT_H

#include "memory_graphics_dat_state_pc34_compat.h"

enum GraphicsDatEntryKind_Compat {
    GRAPHICS_DAT_ENTRY_BITMAP_SAFE = 0,
    GRAPHICS_DAT_ENTRY_BITMAP_SUSPICIOUS = 1,
    GRAPHICS_DAT_ENTRY_SPECIAL_NON_BITMAP = 2,
    GRAPHICS_DAT_ENTRY_EMPTY = 3,
    GRAPHICS_DAT_ENTRY_ZERO_SIZED_PLACEHOLDER = 4
};

struct GraphicsDatEntryClassificationResult_Compat {
    unsigned int graphicIndex;
    enum GraphicsDatEntryKind_Compat kind;
    int shouldUseBitmapPath;
    int shouldSkipBitmapExport;
};

int F9012_RUNTIME_ClassifyGraphicsDatEntry_Compat(
    const struct MemoryGraphicsDatRuntimeState_Compat* runtimeState,
    unsigned int graphicIndex,
    struct GraphicsDatEntryClassificationResult_Compat* outResult);

#endif

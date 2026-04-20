#ifndef REDMCSB_MEMORY_GRAPHICS_DAT_SELECT_PC34_COMPAT_H
#define REDMCSB_MEMORY_GRAPHICS_DAT_SELECT_PC34_COMPAT_H

#include "memory_graphics_dat_header_pc34_compat.h"

struct MemoryGraphicsDatSelection_Compat {
    long offset;
    unsigned short compressedByteCount;
    unsigned short decompressedByteCount;
    struct GraphicWidthHeight_Compat widthHeight;
};

int F0490_MEMORY_SelectGraphicFromHeader_Compat(
    const struct MemoryGraphicsDatHeader_Compat* header,
    unsigned int graphicIndex,
    struct MemoryGraphicsDatSelection_Compat* selection);

#endif

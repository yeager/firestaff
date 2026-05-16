#ifndef REDMCSB_MEMORY_LOAD_SESSION_PC34_COMPAT_H
#define REDMCSB_MEMORY_LOAD_SESSION_PC34_COMPAT_H

#include "memory_load_expand_pc34_compat.h"

struct GraphicWidthHeight_Compat;

struct MemoryLoadSessionResult_Compat {
    const unsigned char* loadTarget;
    int usedViewportBuffer;
    int drawFloorAndCeilingRequested;
};

void F0490_MEMORY_LoadGraphicSession_Compat(
    int graphicIndexFlags,
    const unsigned char* loadedGraphic,
    unsigned long loadedByteCount,
    unsigned char* viewportGraphicBuffer,
    unsigned char* destinationBitmap,
    const struct GraphicWidthHeight_Compat* sizeInfo,
    struct MemoryLoadSessionResult_Compat* outResult);

#endif

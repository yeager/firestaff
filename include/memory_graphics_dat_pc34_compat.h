#ifndef REDMCSB_MEMORY_GRAPHICS_DAT_PC34_COMPAT_H
#define REDMCSB_MEMORY_GRAPHICS_DAT_PC34_COMPAT_H

#include <stdio.h>

struct MemoryGraphicsDatState_Compat {
    FILE* file;
    int referenceCount;
    long fileSize;
    int cachedChunkIndex;
    int cacheContainsGraphicData;
    unsigned char chunkBuffer[1024];
};

int F0477_MEMORY_OpenGraphicsDat_CPSDF_Compat(
    const char* path,
    struct MemoryGraphicsDatState_Compat* state);

int F0478_MEMORY_CloseGraphicsDat_CPSDF_Compat(
    struct MemoryGraphicsDatState_Compat* state);

int F0474_MEMORY_LoadGraphic_CPSDF_Compat(
    long graphicOffset,
    int compressedByteCount,
    struct MemoryGraphicsDatState_Compat* state,
    unsigned char* destination);

#endif

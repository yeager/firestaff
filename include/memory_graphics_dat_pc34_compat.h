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

int F0477_MEMORY_OpenGraphicsDat_FromBuffer_Compat(
    const unsigned char *data, long size,
    struct MemoryGraphicsDatState_Compat* state);

/* ReDMCSB Amiga FLOPPY.C:F0534 adapter: report stream size while restoring
   its current cursor. FILE* is the host handle, not the Amiga BPTR ABI. */
long F0534_FLOPPY_GetFileSize_HostCompat(FILE* file);

/* ReDMCSB Amiga MEMORY.C:F0535: query the selected GRAPHICS.DAT. Uses the
   current stream when open; otherwise opens graphics_path for this query. */
long F0535_MEMORY_GetGraphicsDatFileSize_HostCompat(
    const char* graphics_path,
    struct MemoryGraphicsDatState_Compat* state);

int F0478_MEMORY_CloseGraphicsDat_CPSDF_Compat(
    struct MemoryGraphicsDatState_Compat* state);

int F0474_MEMORY_LoadGraphic_CPSDF_Compat(
    long graphicOffset,
    int compressedByteCount,
    struct MemoryGraphicsDatState_Compat* state,
    unsigned char* destination);

const char *dm1_memory_pass601_gfxdat_source_evidence(void);

#endif

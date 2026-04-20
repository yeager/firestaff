#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include <stdlib.h>
#include <string.h>
#include "memory_graphics_dat_state_pc34_compat.h"

void F0479_MEMORY_FreeGraphicsDatState_Compat(
struct MemoryGraphicsDatRuntimeState_Compat* runtimeState FINAL_SEPARATOR
{
        free(runtimeState->compressedByteCounts);
        free(runtimeState->decompressedByteCounts);
        free(runtimeState->widthHeight);
        memset(runtimeState, 0, sizeof(*runtimeState));
}

int F0479_MEMORY_InitializeGraphicsDatState_Compat(
const char*                               path         SEPARATOR
struct MemoryGraphicsDatState_Compat*     fileState    SEPARATOR
struct MemoryGraphicsDatRuntimeState_Compat* runtimeState FINAL_SEPARATOR
{
        struct MemoryGraphicsDatHeader_Compat header;


        memset(runtimeState, 0, sizeof(*runtimeState));
        if (!F0479_MEMORY_LoadGraphicsDatHeader_Compat(path, fileState, &header)) {
                return 0;
        }
        runtimeState->initialized = 1;
        runtimeState->format = header.format;
        runtimeState->graphicCount = header.graphicCount;
        runtimeState->compressedByteCounts = header.compressedByteCounts;
        runtimeState->decompressedByteCounts = header.decompressedByteCounts;
        runtimeState->widthHeight = header.widthHeight;
        runtimeState->fileSize = header.fileSize;
        return 1;
}

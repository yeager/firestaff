#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include <string.h>
#include "memory_graphics_dat_runtime_transaction_pc34_compat.h"

static void runtime_state_to_header(
const struct MemoryGraphicsDatRuntimeState_Compat* runtimeState SEPARATOR
struct MemoryGraphicsDatHeader_Compat*             header       FINAL_SEPARATOR
{
        memset(header, 0, sizeof(*header));
        header->format = runtimeState->format;
        header->graphicCount = runtimeState->graphicCount;
        header->compressedByteCounts = runtimeState->compressedByteCounts;
        header->decompressedByteCounts = runtimeState->decompressedByteCounts;
        header->widthHeight = runtimeState->widthHeight;
        header->fileSize = runtimeState->fileSize;
}

int F0490_MEMORY_RunRuntimeGraphicsDatTransaction_Compat(
const char*                                     graphicsDatPath       SEPARATOR
const struct MemoryGraphicsDatRuntimeState_Compat* runtimeState       SEPARATOR
unsigned int                                    graphicIndex          SEPARATOR
int                                             graphicIndexFlags     SEPARATOR
struct MemoryGraphicsDatState_Compat*           fileState             SEPARATOR
unsigned char*                                  viewportGraphicBuffer SEPARATOR
unsigned char*                                  destinationBitmap     SEPARATOR
struct MemoryGraphicsDatTransactionResult_Compat* outResult           SEPARATOR
struct MemoryGraphicsDatSelection_Compat*       outSelection          FINAL_SEPARATOR
{
        struct MemoryGraphicsDatHeader_Compat header;


        if ((runtimeState == 0) || !runtimeState->initialized) {
                return 0;
        }
        runtime_state_to_header(runtimeState, &header);
        return F0490_MEMORY_RunSelectedGraphicsDatTransaction_Compat(
            graphicsDatPath,
            &header,
            graphicIndex,
            graphicIndexFlags,
            fileState,
            viewportGraphicBuffer,
            destinationBitmap,
            outResult,
            outSelection);
}

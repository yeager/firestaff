#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include "memory_graphics_dat_composed_transaction_pc34_compat.h"

int F0490_MEMORY_RunSelectedGraphicsDatTransaction_Compat(
const char*                                   graphicsDatPath       SEPARATOR
const struct MemoryGraphicsDatHeader_Compat*  header                SEPARATOR
unsigned int                                  graphicIndex          SEPARATOR
int                                           graphicIndexFlags     SEPARATOR
struct MemoryGraphicsDatState_Compat*         state                 SEPARATOR
unsigned char*                                viewportGraphicBuffer SEPARATOR
unsigned char*                                destinationBitmap     SEPARATOR
struct MemoryGraphicsDatTransactionResult_Compat* outResult         SEPARATOR
struct MemoryGraphicsDatSelection_Compat*     outSelection          FINAL_SEPARATOR
{
        struct MemoryGraphicsDatSelection_Compat selection;


        if (!F0490_MEMORY_SelectGraphicFromHeader_Compat(header, graphicIndex, &selection)) {
                return 0;
        }
        if (!F0490_MEMORY_RunGraphicsDatTransaction_Compat(
                graphicsDatPath,
                selection.offset,
                selection.compressedByteCount,
                graphicIndexFlags,
                state,
                viewportGraphicBuffer,
                destinationBitmap,
                &selection.widthHeight,
                outResult)) {
                return 0;
        }
        if (outSelection != 0) {
                *outSelection = selection;
        }
        return 1;
}

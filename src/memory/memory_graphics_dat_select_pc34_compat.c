#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include <string.h>
#include "memory_graphics_dat_select_pc34_compat.h"
#include "memory_graphics_dat_metadata_pc34_compat.h"

int F0490_MEMORY_SelectGraphicFromHeader_Compat(
const struct MemoryGraphicsDatHeader_Compat* header    SEPARATOR
unsigned int                                 graphicIndex SEPARATOR
struct MemoryGraphicsDatSelection_Compat*    selection FINAL_SEPARATOR
{
        if ((header == 0) || (selection == 0) || (graphicIndex >= header->graphicCount)) {
                return 0;
        }
        memset(selection, 0, sizeof(*selection));
        selection->offset = F0467_MEMORY_GetGraphicOffset_Compat(
            header->format,
            header->graphicCount,
            header->compressedByteCounts,
            graphicIndex);
        selection->compressedByteCount = header->compressedByteCounts[graphicIndex];
        selection->decompressedByteCount = header->decompressedByteCounts[graphicIndex];
        selection->widthHeight = header->widthHeight[graphicIndex];
        return 1;
}

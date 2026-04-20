#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include <stdlib.h>
#include <string.h>
#include "memory_graphics_dat_special_pc34_compat.h"

static int select_from_runtime_state(
const struct MemoryGraphicsDatRuntimeState_Compat* runtimeState SEPARATOR
unsigned int                                       graphicIndex SEPARATOR
struct MemoryGraphicsDatSelection_Compat*          selection    FINAL_SEPARATOR
{
        struct MemoryGraphicsDatHeader_Compat header;


        memset(&header, 0, sizeof(header));
        header.format = runtimeState->format;
        header.graphicCount = runtimeState->graphicCount;
        header.compressedByteCounts = runtimeState->compressedByteCounts;
        header.decompressedByteCounts = runtimeState->decompressedByteCounts;
        header.widthHeight = runtimeState->widthHeight;
        header.fileSize = runtimeState->fileSize;
        return F0490_MEMORY_SelectGraphicFromHeader_Compat(&header, graphicIndex, selection);
}

void F0479_MEMORY_FreeSpecialGraphics_Compat(
struct MemoryGraphicsDatSpecials_Compat* specials FINAL_SEPARATOR
{
        free(specials->dialogBoxGraphic);
        memset(specials, 0, sizeof(*specials));
}

int F0479_MEMORY_PreloadDialogBoxGraphic_Compat(
const char*                                     graphicsDatPath    SEPARATOR
const struct MemoryGraphicsDatRuntimeState_Compat* runtimeState    SEPARATOR
struct MemoryGraphicsDatState_Compat*           fileState          SEPARATOR
unsigned int                                    dialogGraphicIndex SEPARATOR
struct MemoryGraphicsDatSpecials_Compat*        specials           FINAL_SEPARATOR
{
        struct MemoryGraphicsDatSelection_Compat selection;
        unsigned char* graphic;


        if ((runtimeState == 0) || !runtimeState->initialized || (specials == 0)) {
                return 0;
        }
        if (specials->dialogBoxGraphic != 0) {
                return 1;
        }
        if (!select_from_runtime_state(runtimeState, dialogGraphicIndex, &selection)) {
                return 0;
        }
        graphic = (unsigned char*)calloc(runtimeState->decompressedByteCounts[dialogGraphicIndex], 1);
        if (graphic == 0) {
                return 0;
        }
        if (!F0477_MEMORY_OpenGraphicsDat_CPSDF_Compat(graphicsDatPath, fileState)
         || !F0474_MEMORY_LoadGraphic_CPSDF_Compat(selection.offset, selection.compressedByteCount, fileState, graphic)
         || !F0478_MEMORY_CloseGraphicsDat_CPSDF_Compat(fileState)) {
                free(graphic);
                return 0;
        }
        specials->dialogBoxGraphic = graphic;
        specials->dialogBoxAllocatedByteCount = runtimeState->decompressedByteCounts[dialogGraphicIndex];
        specials->dialogBoxLoadedByteCount = selection.compressedByteCount;
        return 1;
}

#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include <stdlib.h>
#include <string.h>
#include "memory_graphics_dat_slots_pc34_compat.h"

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

int F0484_MEMORY_InitializeGraphicSlots_Compat(
unsigned int                          graphicCount SEPARATOR
struct MemoryGraphicsDatSlots_Compat* slots       FINAL_SEPARATOR
{
        memset(slots, 0, sizeof(*slots));
        slots->graphicCount = graphicCount;
        slots->graphics = (unsigned char**)calloc(graphicCount, sizeof(unsigned char*));
        slots->loadedByteCounts = (unsigned short*)calloc(graphicCount, sizeof(unsigned short));
        if ((slots->graphics == 0) || (slots->loadedByteCounts == 0)) {
                F0484_MEMORY_FreeGraphicSlots_Compat(slots);
                return 0;
        }
        return 1;
}

void F0484_MEMORY_FreeGraphicSlots_Compat(
struct MemoryGraphicsDatSlots_Compat* slots FINAL_SEPARATOR
{
        unsigned int i;


        if (slots->graphics != 0) {
                for (i = 0; i < slots->graphicCount; i++) {
                        free(slots->graphics[i]);
                }
        }
        free(slots->graphics);
        free(slots->loadedByteCounts);
        memset(slots, 0, sizeof(*slots));
}

unsigned char* F0484_MEMORY_PreloadGraphicSlot_Compat(
const char*                                   graphicsDatPath SEPARATOR
const struct MemoryGraphicsDatRuntimeState_Compat* runtimeState SEPARATOR
struct MemoryGraphicsDatState_Compat*         fileState       SEPARATOR
unsigned int                                  graphicIndex    SEPARATOR
struct MemoryGraphicsDatSlots_Compat*         slots           SEPARATOR
struct MemoryGraphicsDatSelection_Compat*     outSelection    FINAL_SEPARATOR
{
        struct MemoryGraphicsDatSelection_Compat selection;
        unsigned char* graphic;


        if ((slots == 0) || (runtimeState == 0) || !runtimeState->initialized || (graphicIndex >= slots->graphicCount)) {
                return 0;
        }
        if (slots->graphics[graphicIndex] != 0) {
                if (outSelection != 0) {
                        select_from_runtime_state(runtimeState, graphicIndex, outSelection);
                }
                return slots->graphics[graphicIndex];
        }
        if (!select_from_runtime_state(runtimeState, graphicIndex, &selection)) {
                return 0;
        }
        graphic = (unsigned char*)malloc(selection.compressedByteCount);
        if (graphic == 0) {
                return 0;
        }
        if (!F0477_MEMORY_OpenGraphicsDat_CPSDF_Compat(graphicsDatPath, fileState)
         || !F0474_MEMORY_LoadGraphic_CPSDF_Compat(selection.offset, selection.compressedByteCount, fileState, graphic)
         || !F0478_MEMORY_CloseGraphicsDat_CPSDF_Compat(fileState)) {
                free(graphic);
                return 0;
        }
        slots->graphics[graphicIndex] = graphic;
        slots->loadedByteCounts[graphicIndex] = selection.compressedByteCount;
        if (outSelection != 0) {
                *outSelection = selection;
        }
        return graphic;
}

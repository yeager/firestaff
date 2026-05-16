#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include <string.h>
#include "memory_graphics_dat_menu_state_pc34_compat.h"

void F0479_MEMORY_FreeMenuStateMini_Compat(
struct MemoryGraphicsDatMenuStateResult_Compat* result FINAL_SEPARATOR
{
        F0479_MEMORY_FreeEventDispatchMini_Compat(&result->dispatch);
        memset(result, 0, sizeof(*result));
}

int F0479_MEMORY_RunMenuStateMini_Compat(
const char*                                           graphicsDatPath       SEPARATOR
struct MemoryGraphicsDatState_Compat*                 fileState             SEPARATOR
unsigned int                                          dialogGraphicIndex    SEPARATOR
unsigned int                                          viewportGraphicIndex  SEPARATOR
unsigned char*                                        viewportGraphicBuffer SEPARATOR
unsigned char*                                        viewportBitmap        SEPARATOR
const enum MemoryGraphicsDatEvent_Compat*             events                SEPARATOR
unsigned int                                          eventCount            SEPARATOR
unsigned int                                          initialSelectionIndex SEPARATOR
unsigned int                                          selectionCount        SEPARATOR
struct MemoryGraphicsDatMenuStateResult_Compat*       outResult             FINAL_SEPARATOR
{
        unsigned int i;
        unsigned int currentSelection;

        memset(outResult, 0, sizeof(*outResult));
        if (selectionCount == 0U) {
                return 0;
        }
        if (!F0479_MEMORY_RunEventDispatchMini_Compat(
                graphicsDatPath,
                fileState,
                dialogGraphicIndex,
                viewportGraphicIndex,
                viewportGraphicBuffer,
                viewportBitmap,
                events,
                eventCount,
                &outResult->dispatch)) {
                return 0;
        }
        currentSelection = initialSelectionIndex % selectionCount;
        outResult->initialSelectionIndex = currentSelection;
        for (i = 0; i < eventCount; ++i) {
                if (events[i] == MEMORY_GRAPHICS_DAT_EVENT_ADVANCE) {
                        currentSelection = (currentSelection + 1U) % selectionCount;
                        outResult->advanceTransitionCount++;
                } else if (events[i] == MEMORY_GRAPHICS_DAT_EVENT_FRAME) {
                        outResult->frameCount++;
                }
        }
        outResult->finalSelectionIndex = currentSelection;
        return 1;
}

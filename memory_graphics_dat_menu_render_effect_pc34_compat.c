#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include <string.h>
#include "memory_graphics_dat_menu_render_effect_pc34_compat.h"

void F0479_MEMORY_FreeMenuRenderEffectMini_Compat(
struct MemoryGraphicsDatMenuRenderEffectResult_Compat* result FINAL_SEPARATOR
{
        F0479_MEMORY_FreeMenuStateMini_Compat(&result->menuState);
        memset(result, 0, sizeof(*result));
}

int F0479_MEMORY_RunMenuRenderEffectMini_Compat(
const char*                                                  graphicsDatPath         SEPARATOR
struct MemoryGraphicsDatState_Compat*                        fileState               SEPARATOR
unsigned int                                                 dialogGraphicIndex      SEPARATOR
unsigned int                                                 viewportGraphicIndex    SEPARATOR
unsigned char*                                               viewportGraphicBuffer   SEPARATOR
unsigned char*                                               viewportBitmap          SEPARATOR
const enum MemoryGraphicsDatEvent_Compat*                    events                  SEPARATOR
unsigned int                                                 eventCount              SEPARATOR
unsigned int                                                 initialSelectionIndex   SEPARATOR
unsigned int                                                 selectionCount          SEPARATOR
unsigned int                                                 highlightBaseGraphicIndex SEPARATOR
struct MemoryGraphicsDatMenuRenderEffectResult_Compat*       outResult               FINAL_SEPARATOR
{
        memset(outResult, 0, sizeof(*outResult));
        if (!F0479_MEMORY_RunMenuStateMini_Compat(
                graphicsDatPath,
                fileState,
                dialogGraphicIndex,
                viewportGraphicIndex,
                viewportGraphicBuffer,
                viewportBitmap,
                events,
                eventCount,
                initialSelectionIndex,
                selectionCount,
                &outResult->menuState)) {
                return 0;
        }
        outResult->renderVariantCount = selectionCount;
        outResult->selectedRenderVariant = outResult->menuState.finalSelectionIndex;
        outResult->highlightedGraphicIndex = highlightBaseGraphicIndex + outResult->selectedRenderVariant;
        return 1;
}

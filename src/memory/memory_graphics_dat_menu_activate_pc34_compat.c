#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include <string.h>
#include "memory_graphics_dat_menu_activate_pc34_compat.h"

void F0479_MEMORY_FreeMenuActivateMini_Compat(
struct MemoryGraphicsDatMenuActivateResult_Compat* result FINAL_SEPARATOR
{
        F0479_MEMORY_FreeMenuRenderEffectMini_Compat(&result->render);
        memset(result, 0, sizeof(*result));
}

int F0479_MEMORY_RunMenuActivateMini_Compat(
const char*                                             graphicsDatPath          SEPARATOR
struct MemoryGraphicsDatState_Compat*                   fileState                SEPARATOR
unsigned int                                            dialogGraphicIndex       SEPARATOR
unsigned int                                            viewportGraphicIndex     SEPARATOR
unsigned char*                                          viewportGraphicBuffer    SEPARATOR
unsigned char*                                          viewportBitmap           SEPARATOR
const enum MemoryGraphicsDatEvent_Compat*               events                   SEPARATOR
unsigned int                                            eventCount               SEPARATOR
unsigned int                                            initialSelectionIndex    SEPARATOR
unsigned int                                            selectionCount           SEPARATOR
unsigned int                                            highlightBaseGraphicIndex SEPARATOR
unsigned int                                            activateGraphicBaseIndex SEPARATOR
struct MemoryGraphicsDatMenuActivateResult_Compat*      outResult                FINAL_SEPARATOR
{
        unsigned int i;

        memset(outResult, 0, sizeof(*outResult));
        if (!F0479_MEMORY_RunMenuRenderEffectMini_Compat(
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
                highlightBaseGraphicIndex,
                &outResult->render)) {
                return 0;
        }
        outResult->activatedSelectionIndex = outResult->render.menuState.finalSelectionIndex;
        for (i = 0; i < eventCount; ++i) {
                if (events[i] == MEMORY_GRAPHICS_DAT_EVENT_ACTIVATE) {
                        outResult->activationTriggered = 1;
                }
        }
        if (outResult->activationTriggered) {
                outResult->activatedGraphicIndex = activateGraphicBaseIndex + outResult->activatedSelectionIndex;
        }
        return 1;
}

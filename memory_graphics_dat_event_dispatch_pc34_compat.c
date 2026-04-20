#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include <string.h>
#include "memory_graphics_dat_event_dispatch_pc34_compat.h"

void F0479_MEMORY_FreeEventDispatchMini_Compat(
struct MemoryGraphicsDatEventDispatchResult_Compat* result FINAL_SEPARATOR
{
        F0479_MEMORY_FreeInputCommandQueueMini_Compat(&result->inputQueue);
        memset(result, 0, sizeof(*result));
}

int F0479_MEMORY_RunEventDispatchMini_Compat(
const char*                                              graphicsDatPath       SEPARATOR
struct MemoryGraphicsDatState_Compat*                    fileState             SEPARATOR
unsigned int                                             dialogGraphicIndex    SEPARATOR
unsigned int                                             viewportGraphicIndex  SEPARATOR
unsigned char*                                           viewportGraphicBuffer SEPARATOR
unsigned char*                                           viewportBitmap        SEPARATOR
const enum MemoryGraphicsDatEvent_Compat*                events                SEPARATOR
unsigned int                                             eventCount            SEPARATOR
struct MemoryGraphicsDatEventDispatchResult_Compat*      outResult             FINAL_SEPARATOR
{
        enum MemoryGraphicsDatInputSignal_Compat inputs[32];
        unsigned int i;

        memset(outResult, 0, sizeof(*outResult));
        outResult->requestedEventCount = eventCount;
        if ((events == 0) && (eventCount != 0)) {
                return 0;
        }
        if (eventCount > 32U) {
                return 0;
        }
        for (i = 0; i < eventCount; ++i) {
                if (events[i] == MEMORY_GRAPHICS_DAT_EVENT_ADVANCE) {
                        inputs[i] = MEMORY_GRAPHICS_DAT_INPUT_SIGNAL_ADVANCE;
                        outResult->advanceEventCount++;
                } else {
                        inputs[i] = MEMORY_GRAPHICS_DAT_INPUT_SIGNAL_NONE;
                        if (events[i] == MEMORY_GRAPHICS_DAT_EVENT_FRAME) {
                                outResult->frameEventCount++;
                        } else if (events[i] == MEMORY_GRAPHICS_DAT_EVENT_ACTIVATE) {
                                outResult->activateEventCount++;
                        } else if (events[i] == MEMORY_GRAPHICS_DAT_EVENT_RETURN_TO_MENU) {
                                outResult->returnToMenuEventCount++;
                        } else {
                                outResult->idleEventCount++;
                        }
                }
                outResult->dispatchedEventCount++;
        }
        if (!F0479_MEMORY_RunInputCommandQueueMini_Compat(
                graphicsDatPath,
                fileState,
                dialogGraphicIndex,
                viewportGraphicIndex,
                viewportGraphicBuffer,
                viewportBitmap,
                inputs,
                eventCount,
                &outResult->inputQueue)) {
                return 0;
        }
        return outResult->inputQueue.typedQueue.queue.queueCompleted;
}

#ifndef REDMCSB_MEMORY_GRAPHICS_DAT_EVENT_DISPATCH_PC34_COMPAT_H
#define REDMCSB_MEMORY_GRAPHICS_DAT_EVENT_DISPATCH_PC34_COMPAT_H

#include "memory_graphics_dat_input_command_queue_pc34_compat.h"

enum MemoryGraphicsDatEvent_Compat {
    MEMORY_GRAPHICS_DAT_EVENT_IDLE = 0,
    MEMORY_GRAPHICS_DAT_EVENT_FRAME = 1,
    MEMORY_GRAPHICS_DAT_EVENT_ADVANCE = 2,
    MEMORY_GRAPHICS_DAT_EVENT_ACTIVATE = 3,
    MEMORY_GRAPHICS_DAT_EVENT_BACK = 4,
    MEMORY_GRAPHICS_DAT_EVENT_CANCEL = 5,
    MEMORY_GRAPHICS_DAT_EVENT_RETURN_TO_MENU = 6
};

struct MemoryGraphicsDatEventDispatchResult_Compat {
    struct MemoryGraphicsDatInputCommandQueueResult_Compat inputQueue;
    unsigned int requestedEventCount;
    unsigned int dispatchedEventCount;
    unsigned int frameEventCount;
    unsigned int advanceEventCount;
    unsigned int activateEventCount;
    unsigned int idleEventCount;
    unsigned int backEventCount;
    unsigned int cancelEventCount;
    unsigned int returnToMenuEventCount;
};

int F0479_MEMORY_RunEventDispatchMini_Compat(
    const char* graphicsDatPath,
    struct MemoryGraphicsDatState_Compat* fileState,
    unsigned int dialogGraphicIndex,
    unsigned int viewportGraphicIndex,
    unsigned char* viewportGraphicBuffer,
    unsigned char* viewportBitmap,
    const enum MemoryGraphicsDatEvent_Compat* events,
    unsigned int eventCount,
    struct MemoryGraphicsDatEventDispatchResult_Compat* outResult);

void F0479_MEMORY_FreeEventDispatchMini_Compat(
    struct MemoryGraphicsDatEventDispatchResult_Compat* result);

#endif

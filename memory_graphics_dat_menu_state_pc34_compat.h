#ifndef REDMCSB_MEMORY_GRAPHICS_DAT_MENU_STATE_PC34_COMPAT_H
#define REDMCSB_MEMORY_GRAPHICS_DAT_MENU_STATE_PC34_COMPAT_H

#include "memory_graphics_dat_event_dispatch_pc34_compat.h"

struct MemoryGraphicsDatMenuStateResult_Compat {
    struct MemoryGraphicsDatEventDispatchResult_Compat dispatch;
    unsigned int initialSelectionIndex;
    unsigned int finalSelectionIndex;
    unsigned int advanceTransitionCount;
    unsigned int frameCount;
};

int F0479_MEMORY_RunMenuStateMini_Compat(
    const char* graphicsDatPath,
    struct MemoryGraphicsDatState_Compat* fileState,
    unsigned int dialogGraphicIndex,
    unsigned int viewportGraphicIndex,
    unsigned char* viewportGraphicBuffer,
    unsigned char* viewportBitmap,
    const enum MemoryGraphicsDatEvent_Compat* events,
    unsigned int eventCount,
    unsigned int initialSelectionIndex,
    unsigned int selectionCount,
    struct MemoryGraphicsDatMenuStateResult_Compat* outResult);

void F0479_MEMORY_FreeMenuStateMini_Compat(
    struct MemoryGraphicsDatMenuStateResult_Compat* result);

#endif

#ifndef REDMCSB_MEMORY_GRAPHICS_DAT_MENU_ACTIVATE_PC34_COMPAT_H
#define REDMCSB_MEMORY_GRAPHICS_DAT_MENU_ACTIVATE_PC34_COMPAT_H

#include "memory_graphics_dat_menu_render_effect_pc34_compat.h"

struct MemoryGraphicsDatMenuActivateResult_Compat {
    struct MemoryGraphicsDatMenuRenderEffectResult_Compat render;
    int activationTriggered;
    unsigned int activatedSelectionIndex;
    unsigned int activatedGraphicIndex;
};

int F0479_MEMORY_RunMenuActivateMini_Compat(
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
    unsigned int highlightBaseGraphicIndex,
    unsigned int activateGraphicBaseIndex,
    struct MemoryGraphicsDatMenuActivateResult_Compat* outResult);

void F0479_MEMORY_FreeMenuActivateMini_Compat(
    struct MemoryGraphicsDatMenuActivateResult_Compat* result);

#endif

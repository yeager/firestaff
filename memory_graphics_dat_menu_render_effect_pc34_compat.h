#ifndef REDMCSB_MEMORY_GRAPHICS_DAT_MENU_RENDER_EFFECT_PC34_COMPAT_H
#define REDMCSB_MEMORY_GRAPHICS_DAT_MENU_RENDER_EFFECT_PC34_COMPAT_H

#include "memory_graphics_dat_menu_state_pc34_compat.h"

struct MemoryGraphicsDatMenuRenderEffectResult_Compat {
    struct MemoryGraphicsDatMenuStateResult_Compat menuState;
    unsigned int renderVariantCount;
    unsigned int selectedRenderVariant;
    unsigned int highlightedGraphicIndex;
};

int F0479_MEMORY_RunMenuRenderEffectMini_Compat(
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
    struct MemoryGraphicsDatMenuRenderEffectResult_Compat* outResult);

void F0479_MEMORY_FreeMenuRenderEffectMini_Compat(
    struct MemoryGraphicsDatMenuRenderEffectResult_Compat* result);

#endif

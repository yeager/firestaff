#ifndef REDMCSB_MEMORY_GRAPHICS_DAT_MENU_ACTIVATE_CONSEQUENCE_PC34_COMPAT_H
#define REDMCSB_MEMORY_GRAPHICS_DAT_MENU_ACTIVATE_CONSEQUENCE_PC34_COMPAT_H

#include "memory_graphics_dat_menu_activate_pc34_compat.h"

enum MemoryGraphicsDatMenuScreen_Compat {
    MEMORY_GRAPHICS_DAT_MENU_SCREEN_MENU = 0,
    MEMORY_GRAPHICS_DAT_MENU_SCREEN_SUBMENU = 1
};

enum MemoryGraphicsDatMenuGameStateClass_Compat {
    MEMORY_GRAPHICS_DAT_MENU_GAME_STATE_NONE = 0,
    MEMORY_GRAPHICS_DAT_MENU_GAME_STATE_NAVIGATED = 1,
    MEMORY_GRAPHICS_DAT_MENU_GAME_STATE_ACTIVATED = 2,
    MEMORY_GRAPHICS_DAT_MENU_GAME_STATE_ENTERED_SUBMENU = 3,
    MEMORY_GRAPHICS_DAT_MENU_GAME_STATE_HELD = 4
};

enum MemoryGraphicsDatMenuGameStateMask_Compat {
    MEMORY_GRAPHICS_DAT_MENU_GAME_STATE_MASK_NONE = 0,
    MEMORY_GRAPHICS_DAT_MENU_GAME_STATE_MASK_NAVIGATED = 1,
    MEMORY_GRAPHICS_DAT_MENU_GAME_STATE_MASK_ACTIVATED = 2,
    MEMORY_GRAPHICS_DAT_MENU_GAME_STATE_MASK_SCREEN_CHANGED = 4,
    MEMORY_GRAPHICS_DAT_MENU_GAME_STATE_MASK_RENDER_VARIANT_CHANGED = 8,
    MEMORY_GRAPHICS_DAT_MENU_GAME_STATE_MASK_HELD = 16
};

enum MemoryGraphicsDatMenuCommitHistoryClass_Compat {
    MEMORY_GRAPHICS_DAT_MENU_COMMIT_HISTORY_NONE = 0,
    MEMORY_GRAPHICS_DAT_MENU_COMMIT_HISTORY_SINGLE = 1,
    MEMORY_GRAPHICS_DAT_MENU_COMMIT_HISTORY_REPEATED_SAME = 2,
    MEMORY_GRAPHICS_DAT_MENU_COMMIT_HISTORY_REPEATED_VARIED = 3
};

enum MemoryGraphicsDatMenuCommitHistoryMask_Compat {
    MEMORY_GRAPHICS_DAT_MENU_COMMIT_HISTORY_MASK_NONE = 0,
    MEMORY_GRAPHICS_DAT_MENU_COMMIT_HISTORY_MASK_COMMITTED = 1,
    MEMORY_GRAPHICS_DAT_MENU_COMMIT_HISTORY_MASK_REPEATED = 2,
    MEMORY_GRAPHICS_DAT_MENU_COMMIT_HISTORY_MASK_VARIED = 4
};

struct MemoryGraphicsDatMenuActivateConsequenceResult_Compat {
    struct MemoryGraphicsDatMenuActivateResult_Compat activate;
    enum MemoryGraphicsDatMenuScreen_Compat initialScreen;
    enum MemoryGraphicsDatMenuScreen_Compat finalScreen;
    int screenChanged;
    int selectionChanged;
    int renderVariantChanged;
    int activationCommitted;
    int enteredSubmenu;
    int remainedInScreen;
    unsigned int initialSelectionIndex;
    unsigned int finalSelectionIndex;
    unsigned int selectionDelta;
    enum MemoryGraphicsDatMenuGameStateClass_Compat gameStateClass;
    unsigned int gameStateMask;
};

int F0479_MEMORY_RunMenuActivateConsequenceMini_Compat(
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
    enum MemoryGraphicsDatMenuScreen_Compat initialScreen,
    struct MemoryGraphicsDatMenuActivateConsequenceResult_Compat* outResult);

void F0479_MEMORY_FreeMenuActivateConsequenceMini_Compat(
    struct MemoryGraphicsDatMenuActivateConsequenceResult_Compat* result);

#endif

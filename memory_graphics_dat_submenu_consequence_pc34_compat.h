#ifndef REDMCSB_MEMORY_GRAPHICS_DAT_SUBMENU_CONSEQUENCE_PC34_COMPAT_H
#define REDMCSB_MEMORY_GRAPHICS_DAT_SUBMENU_CONSEQUENCE_PC34_COMPAT_H

#include "memory_graphics_dat_menu_activate_consequence_pc34_compat.h"

enum MemoryGraphicsDatSubmenuBehaviorClass_Compat {
    MEMORY_GRAPHICS_DAT_SUBMENU_BEHAVIOR_NONE = 0,
    MEMORY_GRAPHICS_DAT_SUBMENU_BEHAVIOR_REACTIVATE = 1,
    MEMORY_GRAPHICS_DAT_SUBMENU_BEHAVIOR_FRAME_HOLD = 2,
    MEMORY_GRAPHICS_DAT_SUBMENU_BEHAVIOR_IDLE_HOLD = 3,
    MEMORY_GRAPHICS_DAT_SUBMENU_BEHAVIOR_ADVANCE_SUPPRESSED_HOLD = 4,
    MEMORY_GRAPHICS_DAT_SUBMENU_BEHAVIOR_MIXED_HOLD = 5,
    MEMORY_GRAPHICS_DAT_SUBMENU_BEHAVIOR_BACK = 6,
    MEMORY_GRAPHICS_DAT_SUBMENU_BEHAVIOR_CANCEL = 7,
    MEMORY_GRAPHICS_DAT_SUBMENU_BEHAVIOR_RETURN_TO_MENU = 8
};

enum MemoryGraphicsDatSubmenuBehaviorMask_Compat {
    MEMORY_GRAPHICS_DAT_SUBMENU_BEHAVIOR_MASK_NONE = 0,
    MEMORY_GRAPHICS_DAT_SUBMENU_BEHAVIOR_MASK_REACTIVATE = 1,
    MEMORY_GRAPHICS_DAT_SUBMENU_BEHAVIOR_MASK_FRAME_HOLD = 2,
    MEMORY_GRAPHICS_DAT_SUBMENU_BEHAVIOR_MASK_IDLE_HOLD = 4,
    MEMORY_GRAPHICS_DAT_SUBMENU_BEHAVIOR_MASK_ADVANCE_SUPPRESSED_HOLD = 8,
    MEMORY_GRAPHICS_DAT_SUBMENU_BEHAVIOR_MASK_BACK = 16,
    MEMORY_GRAPHICS_DAT_SUBMENU_BEHAVIOR_MASK_CANCEL = 32,
    MEMORY_GRAPHICS_DAT_SUBMENU_BEHAVIOR_MASK_RETURN_TO_MENU = 64
};

enum MemoryGraphicsDatSubmenuExitClass_Compat {
    MEMORY_GRAPHICS_DAT_SUBMENU_EXIT_NONE = 0,
    MEMORY_GRAPHICS_DAT_SUBMENU_EXIT_RETURN = 1,
    MEMORY_GRAPHICS_DAT_SUBMENU_EXIT_CANCEL = 2,
    MEMORY_GRAPHICS_DAT_SUBMENU_EXIT_MIXED = 3
};

enum MemoryGraphicsDatSubmenuExitMask_Compat {
    MEMORY_GRAPHICS_DAT_SUBMENU_EXIT_MASK_NONE = 0,
    MEMORY_GRAPHICS_DAT_SUBMENU_EXIT_MASK_RETURN = 1,
    MEMORY_GRAPHICS_DAT_SUBMENU_EXIT_MASK_CANCEL = 2
};

enum MemoryGraphicsDatSubmenuInternalClass_Compat {
    MEMORY_GRAPHICS_DAT_SUBMENU_INTERNAL_NONE = 0,
    MEMORY_GRAPHICS_DAT_SUBMENU_INTERNAL_HOLD = 1,
    MEMORY_GRAPHICS_DAT_SUBMENU_INTERNAL_REACTIVATE = 2,
    MEMORY_GRAPHICS_DAT_SUBMENU_INTERNAL_EXIT = 3,
    MEMORY_GRAPHICS_DAT_SUBMENU_INTERNAL_NAVIGATE = 4,
    MEMORY_GRAPHICS_DAT_SUBMENU_INTERNAL_RETURN = 5,
    MEMORY_GRAPHICS_DAT_SUBMENU_INTERNAL_MIXED = 6
};

enum MemoryGraphicsDatSubmenuInternalMask_Compat {
    MEMORY_GRAPHICS_DAT_SUBMENU_INTERNAL_MASK_NONE = 0,
    MEMORY_GRAPHICS_DAT_SUBMENU_INTERNAL_MASK_HOLD = 1,
    MEMORY_GRAPHICS_DAT_SUBMENU_INTERNAL_MASK_REACTIVATE = 2,
    MEMORY_GRAPHICS_DAT_SUBMENU_INTERNAL_MASK_EXIT = 4,
    MEMORY_GRAPHICS_DAT_SUBMENU_INTERNAL_MASK_NAVIGATE = 8,
    MEMORY_GRAPHICS_DAT_SUBMENU_INTERNAL_MASK_RETURN = 16
};

struct MemoryGraphicsDatSubmenuConsequenceResult_Compat {
    struct MemoryGraphicsDatMenuActivateConsequenceResult_Compat consequence;
    unsigned int submenuEventCount;
    unsigned int submenuFrameEventCount;
    unsigned int submenuIdleEventCount;
    unsigned int submenuAdvanceSuppressedCount;
    unsigned int submenuReactivateCount;
    unsigned int submenuBackEventCount;
    unsigned int submenuCancelEventCount;
    unsigned int submenuReturnExitCount;
    unsigned int submenuCancelExitCount;
    int submenuModeActive;
    int submenuHoldApplied;
    int submenuExitApplied;
    int submenuReactivateHandled;
    enum MemoryGraphicsDatSubmenuBehaviorClass_Compat behaviorClass;
    enum MemoryGraphicsDatSubmenuBehaviorClass_Compat cumulativeBehaviorClass;
    unsigned int behaviorMask;
    enum MemoryGraphicsDatSubmenuExitClass_Compat exitClass;
    enum MemoryGraphicsDatSubmenuExitClass_Compat cumulativeExitClass;
    unsigned int exitMask;
    unsigned int submenuReturnToMenuCount;
    int submenuReturnToMenuApplied;
    enum MemoryGraphicsDatSubmenuInternalClass_Compat internalClass;
    unsigned int internalMask;
    unsigned int finalRenderGraphicIndex;
};

int F0479_MEMORY_RunSubmenuConsequenceMini_Compat(
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
    unsigned int initialRenderGraphicIndex,
    struct MemoryGraphicsDatSubmenuConsequenceResult_Compat* outResult);

void F0479_MEMORY_FreeSubmenuConsequenceMini_Compat(
    struct MemoryGraphicsDatSubmenuConsequenceResult_Compat* result);

#endif

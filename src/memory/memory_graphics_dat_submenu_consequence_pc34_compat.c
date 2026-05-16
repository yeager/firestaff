#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include <string.h>
#include "memory_graphics_dat_submenu_consequence_pc34_compat.h"

static enum MemoryGraphicsDatSubmenuBehaviorClass_Compat F0479_MEMORY_ClassifySubmenuBehaviorMask_Compat(
unsigned int behaviorMask FINAL_SEPARATOR
{
        switch (behaviorMask) {
        case MEMORY_GRAPHICS_DAT_SUBMENU_BEHAVIOR_MASK_NONE:
                return MEMORY_GRAPHICS_DAT_SUBMENU_BEHAVIOR_NONE;
        case MEMORY_GRAPHICS_DAT_SUBMENU_BEHAVIOR_MASK_REACTIVATE:
                return MEMORY_GRAPHICS_DAT_SUBMENU_BEHAVIOR_REACTIVATE;
        case MEMORY_GRAPHICS_DAT_SUBMENU_BEHAVIOR_MASK_FRAME_HOLD:
                return MEMORY_GRAPHICS_DAT_SUBMENU_BEHAVIOR_FRAME_HOLD;
        case MEMORY_GRAPHICS_DAT_SUBMENU_BEHAVIOR_MASK_IDLE_HOLD:
                return MEMORY_GRAPHICS_DAT_SUBMENU_BEHAVIOR_IDLE_HOLD;
        case MEMORY_GRAPHICS_DAT_SUBMENU_BEHAVIOR_MASK_ADVANCE_SUPPRESSED_HOLD:
                return MEMORY_GRAPHICS_DAT_SUBMENU_BEHAVIOR_ADVANCE_SUPPRESSED_HOLD;
        case MEMORY_GRAPHICS_DAT_SUBMENU_BEHAVIOR_MASK_BACK:
                return MEMORY_GRAPHICS_DAT_SUBMENU_BEHAVIOR_BACK;
        case MEMORY_GRAPHICS_DAT_SUBMENU_BEHAVIOR_MASK_CANCEL:
                return MEMORY_GRAPHICS_DAT_SUBMENU_BEHAVIOR_CANCEL;
        case MEMORY_GRAPHICS_DAT_SUBMENU_BEHAVIOR_MASK_RETURN_TO_MENU:
                return MEMORY_GRAPHICS_DAT_SUBMENU_BEHAVIOR_RETURN_TO_MENU;
        default:
                return MEMORY_GRAPHICS_DAT_SUBMENU_BEHAVIOR_MIXED_HOLD;
        }
}

static enum MemoryGraphicsDatSubmenuExitClass_Compat F0479_MEMORY_ClassifySubmenuExitMask_Compat(
unsigned int exitMask FINAL_SEPARATOR
{
        switch (exitMask) {
        case MEMORY_GRAPHICS_DAT_SUBMENU_EXIT_MASK_NONE:
                return MEMORY_GRAPHICS_DAT_SUBMENU_EXIT_NONE;
        case MEMORY_GRAPHICS_DAT_SUBMENU_EXIT_MASK_RETURN:
                return MEMORY_GRAPHICS_DAT_SUBMENU_EXIT_RETURN;
        case MEMORY_GRAPHICS_DAT_SUBMENU_EXIT_MASK_CANCEL:
                return MEMORY_GRAPHICS_DAT_SUBMENU_EXIT_CANCEL;
        default:
                return MEMORY_GRAPHICS_DAT_SUBMENU_EXIT_MIXED;
        }
}

void F0479_MEMORY_FreeSubmenuConsequenceMini_Compat(
struct MemoryGraphicsDatSubmenuConsequenceResult_Compat* result FINAL_SEPARATOR
{
        F0479_MEMORY_FreeMenuActivateConsequenceMini_Compat(&result->consequence);
        memset(result, 0, sizeof(*result));
}

int F0479_MEMORY_RunSubmenuConsequenceMini_Compat(
const char*                                                  graphicsDatPath           SEPARATOR
struct MemoryGraphicsDatState_Compat*                        fileState                 SEPARATOR
unsigned int                                                 dialogGraphicIndex        SEPARATOR
unsigned int                                                 viewportGraphicIndex      SEPARATOR
unsigned char*                                               viewportGraphicBuffer     SEPARATOR
unsigned char*                                               viewportBitmap            SEPARATOR
const enum MemoryGraphicsDatEvent_Compat*                    events                    SEPARATOR
unsigned int                                                 eventCount                SEPARATOR
unsigned int                                                 initialSelectionIndex     SEPARATOR
unsigned int                                                 selectionCount            SEPARATOR
unsigned int                                                 highlightBaseGraphicIndex SEPARATOR
unsigned int                                                 activateGraphicBaseIndex  SEPARATOR
enum MemoryGraphicsDatMenuScreen_Compat                      initialScreen             SEPARATOR
unsigned int                                                 initialRenderGraphicIndex SEPARATOR
struct MemoryGraphicsDatSubmenuConsequenceResult_Compat*     outResult                 FINAL_SEPARATOR
{
        unsigned int i;

        memset(outResult, 0, sizeof(*outResult));
        if (!F0479_MEMORY_RunMenuActivateConsequenceMini_Compat(
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
                activateGraphicBaseIndex,
                initialScreen,
                &outResult->consequence)) {
                return 0;
        }
        if (initialScreen == MEMORY_GRAPHICS_DAT_MENU_SCREEN_SUBMENU) {
                enum MemoryGraphicsDatSubmenuExitClass_Compat lastExitClass =
                        MEMORY_GRAPHICS_DAT_SUBMENU_EXIT_NONE;
                outResult->submenuModeActive = 1;
                outResult->submenuEventCount = eventCount;
                for (i = 0; i < eventCount; ++i) {
                        if (events[i] == MEMORY_GRAPHICS_DAT_EVENT_FRAME) {
                                outResult->submenuFrameEventCount++;
                                outResult->behaviorMask |= MEMORY_GRAPHICS_DAT_SUBMENU_BEHAVIOR_MASK_FRAME_HOLD;
                        } else if (events[i] == MEMORY_GRAPHICS_DAT_EVENT_IDLE) {
                                outResult->submenuIdleEventCount++;
                                outResult->behaviorMask |= MEMORY_GRAPHICS_DAT_SUBMENU_BEHAVIOR_MASK_IDLE_HOLD;
                        } else if (events[i] == MEMORY_GRAPHICS_DAT_EVENT_ADVANCE) {
                                outResult->submenuAdvanceSuppressedCount++;
                                outResult->behaviorMask |= MEMORY_GRAPHICS_DAT_SUBMENU_BEHAVIOR_MASK_ADVANCE_SUPPRESSED_HOLD;
                                outResult->submenuCancelExitCount++;
                                outResult->exitMask |= MEMORY_GRAPHICS_DAT_SUBMENU_EXIT_MASK_CANCEL;
                                lastExitClass = MEMORY_GRAPHICS_DAT_SUBMENU_EXIT_CANCEL;
                        } else if (events[i] == MEMORY_GRAPHICS_DAT_EVENT_ACTIVATE) {
                                outResult->submenuReactivateCount++;
                                outResult->behaviorMask |= MEMORY_GRAPHICS_DAT_SUBMENU_BEHAVIOR_MASK_REACTIVATE;
                                outResult->submenuReturnExitCount++;
                                outResult->exitMask |= MEMORY_GRAPHICS_DAT_SUBMENU_EXIT_MASK_RETURN;
                                lastExitClass = MEMORY_GRAPHICS_DAT_SUBMENU_EXIT_RETURN;
                        } else if (events[i] == MEMORY_GRAPHICS_DAT_EVENT_BACK) {
                                outResult->submenuBackEventCount++;
                                outResult->behaviorMask |= MEMORY_GRAPHICS_DAT_SUBMENU_BEHAVIOR_MASK_BACK;
                                outResult->submenuReturnExitCount++;
                                outResult->exitMask |= MEMORY_GRAPHICS_DAT_SUBMENU_EXIT_MASK_RETURN;
                                lastExitClass = MEMORY_GRAPHICS_DAT_SUBMENU_EXIT_RETURN;
                        } else if (events[i] == MEMORY_GRAPHICS_DAT_EVENT_CANCEL) {
                                outResult->submenuCancelEventCount++;
                                outResult->behaviorMask |= MEMORY_GRAPHICS_DAT_SUBMENU_BEHAVIOR_MASK_CANCEL;
                                outResult->submenuCancelExitCount++;
                                outResult->exitMask |= MEMORY_GRAPHICS_DAT_SUBMENU_EXIT_MASK_CANCEL;
                                lastExitClass = MEMORY_GRAPHICS_DAT_SUBMENU_EXIT_CANCEL;
                        } else if (events[i] == MEMORY_GRAPHICS_DAT_EVENT_RETURN_TO_MENU) {
                                outResult->submenuReturnToMenuCount++;
                                outResult->behaviorMask |= MEMORY_GRAPHICS_DAT_SUBMENU_BEHAVIOR_MASK_RETURN_TO_MENU;
                        }
                }
                outResult->exitClass = lastExitClass;
                outResult->cumulativeExitClass =
                        F0479_MEMORY_ClassifySubmenuExitMask_Compat(outResult->exitMask);
                outResult->submenuExitApplied =
                        (outResult->exitMask != MEMORY_GRAPHICS_DAT_SUBMENU_EXIT_MASK_NONE) ? 1 : 0;
                if (outResult->submenuReturnToMenuCount > 0U) {
                        outResult->submenuReturnToMenuApplied = 1;
                        outResult->consequence.finalScreen = MEMORY_GRAPHICS_DAT_MENU_SCREEN_MENU;
                        outResult->consequence.screenChanged =
                                (outResult->consequence.initialScreen != MEMORY_GRAPHICS_DAT_MENU_SCREEN_MENU) ? 1 : 0;
                        outResult->behaviorClass = MEMORY_GRAPHICS_DAT_SUBMENU_BEHAVIOR_RETURN_TO_MENU;
                        outResult->finalRenderGraphicIndex = outResult->consequence.activate.render.highlightedGraphicIndex;
                } else if (outResult->consequence.activate.activationTriggered) {
                        outResult->submenuReactivateHandled = 1;
                        outResult->behaviorClass = MEMORY_GRAPHICS_DAT_SUBMENU_BEHAVIOR_REACTIVATE;
                        outResult->finalRenderGraphicIndex = outResult->consequence.activate.activatedGraphicIndex;
                } else {
                        outResult->submenuHoldApplied = 1;
                        if ((outResult->submenuBackEventCount != 0U) &&
                            (outResult->submenuCancelEventCount == 0U) &&
                            (outResult->submenuFrameEventCount == 0U) &&
                            (outResult->submenuIdleEventCount == 0U) &&
                            (outResult->submenuAdvanceSuppressedCount == 0U)) {
                                outResult->behaviorClass = MEMORY_GRAPHICS_DAT_SUBMENU_BEHAVIOR_BACK;
                        } else if ((outResult->submenuCancelEventCount != 0U) &&
                                   (outResult->submenuBackEventCount == 0U) &&
                                   (outResult->submenuFrameEventCount == 0U) &&
                                   (outResult->submenuIdleEventCount == 0U) &&
                                   (outResult->submenuAdvanceSuppressedCount == 0U)) {
                                outResult->behaviorClass = MEMORY_GRAPHICS_DAT_SUBMENU_BEHAVIOR_CANCEL;
                        } else if ((outResult->submenuFrameEventCount != 0U) &&
                            (outResult->submenuIdleEventCount == 0U) &&
                            (outResult->submenuAdvanceSuppressedCount == 0U) &&
                            (outResult->submenuBackEventCount == 0U) &&
                            (outResult->submenuCancelEventCount == 0U)) {
                                outResult->behaviorClass = MEMORY_GRAPHICS_DAT_SUBMENU_BEHAVIOR_FRAME_HOLD;
                        } else if ((outResult->submenuFrameEventCount == 0U) &&
                                   (outResult->submenuIdleEventCount != 0U) &&
                                   (outResult->submenuAdvanceSuppressedCount == 0U) &&
                                   (outResult->submenuBackEventCount == 0U) &&
                                   (outResult->submenuCancelEventCount == 0U)) {
                                outResult->behaviorClass = MEMORY_GRAPHICS_DAT_SUBMENU_BEHAVIOR_IDLE_HOLD;
                        } else if ((outResult->submenuFrameEventCount == 0U) &&
                                   (outResult->submenuIdleEventCount == 0U) &&
                                   (outResult->submenuAdvanceSuppressedCount != 0U) &&
                                   (outResult->submenuBackEventCount == 0U) &&
                                   (outResult->submenuCancelEventCount == 0U)) {
                                outResult->behaviorClass = MEMORY_GRAPHICS_DAT_SUBMENU_BEHAVIOR_ADVANCE_SUPPRESSED_HOLD;
                        } else {
                                outResult->behaviorClass = MEMORY_GRAPHICS_DAT_SUBMENU_BEHAVIOR_MIXED_HOLD;
                        }
                        outResult->finalRenderGraphicIndex = initialRenderGraphicIndex;
                }
                outResult->cumulativeBehaviorClass =
                        F0479_MEMORY_ClassifySubmenuBehaviorMask_Compat(outResult->behaviorMask);
                /* submenu-internal activity classification */
                if (outResult->submenuFrameEventCount > 0U || outResult->submenuIdleEventCount > 0U) {
                        outResult->internalMask |= MEMORY_GRAPHICS_DAT_SUBMENU_INTERNAL_MASK_HOLD;
                }
                if (outResult->submenuReactivateCount > 0U) {
                        outResult->internalMask |= MEMORY_GRAPHICS_DAT_SUBMENU_INTERNAL_MASK_REACTIVATE;
                }
                if (outResult->submenuBackEventCount > 0U || outResult->submenuCancelEventCount > 0U ||
                    outResult->submenuAdvanceSuppressedCount > 0U) {
                        outResult->internalMask |= MEMORY_GRAPHICS_DAT_SUBMENU_INTERNAL_MASK_EXIT;
                }
                if (outResult->submenuReturnToMenuCount > 0U) {
                        outResult->internalMask |= MEMORY_GRAPHICS_DAT_SUBMENU_INTERNAL_MASK_RETURN;
                }
                switch (outResult->internalMask) {
                case MEMORY_GRAPHICS_DAT_SUBMENU_INTERNAL_MASK_NONE:
                        outResult->internalClass = MEMORY_GRAPHICS_DAT_SUBMENU_INTERNAL_NONE;
                        break;
                case MEMORY_GRAPHICS_DAT_SUBMENU_INTERNAL_MASK_HOLD:
                        outResult->internalClass = MEMORY_GRAPHICS_DAT_SUBMENU_INTERNAL_HOLD;
                        break;
                case MEMORY_GRAPHICS_DAT_SUBMENU_INTERNAL_MASK_REACTIVATE:
                        outResult->internalClass = MEMORY_GRAPHICS_DAT_SUBMENU_INTERNAL_REACTIVATE;
                        break;
                case MEMORY_GRAPHICS_DAT_SUBMENU_INTERNAL_MASK_EXIT:
                        outResult->internalClass = MEMORY_GRAPHICS_DAT_SUBMENU_INTERNAL_EXIT;
                        break;
                case MEMORY_GRAPHICS_DAT_SUBMENU_INTERNAL_MASK_RETURN:
                        outResult->internalClass = MEMORY_GRAPHICS_DAT_SUBMENU_INTERNAL_RETURN;
                        break;
                default:
                        outResult->internalClass = MEMORY_GRAPHICS_DAT_SUBMENU_INTERNAL_MIXED;
                        break;
                }
                if (outResult->finalRenderGraphicIndex == 0U) {
                        outResult->finalRenderGraphicIndex = outResult->consequence.activate.render.highlightedGraphicIndex;
                }
        } else {
                outResult->finalRenderGraphicIndex = outResult->consequence.activate.render.highlightedGraphicIndex;
                outResult->cumulativeBehaviorClass = MEMORY_GRAPHICS_DAT_SUBMENU_BEHAVIOR_NONE;
                outResult->exitClass = MEMORY_GRAPHICS_DAT_SUBMENU_EXIT_NONE;
                outResult->cumulativeExitClass = MEMORY_GRAPHICS_DAT_SUBMENU_EXIT_NONE;
                outResult->submenuExitApplied = 0;
        }
        return 1;
}

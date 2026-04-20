#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include <string.h>
#include "memory_graphics_dat_menu_activate_consequence_pc34_compat.h"

void F0479_MEMORY_FreeMenuActivateConsequenceMini_Compat(
struct MemoryGraphicsDatMenuActivateConsequenceResult_Compat* result FINAL_SEPARATOR
{
        F0479_MEMORY_FreeMenuActivateMini_Compat(&result->activate);
        memset(result, 0, sizeof(*result));
}

int F0479_MEMORY_RunMenuActivateConsequenceMini_Compat(
const char*                                                        graphicsDatPath          SEPARATOR
struct MemoryGraphicsDatState_Compat*                              fileState                SEPARATOR
unsigned int                                                       dialogGraphicIndex       SEPARATOR
unsigned int                                                       viewportGraphicIndex     SEPARATOR
unsigned char*                                                     viewportGraphicBuffer    SEPARATOR
unsigned char*                                                     viewportBitmap           SEPARATOR
const enum MemoryGraphicsDatEvent_Compat*                          events                   SEPARATOR
unsigned int                                                       eventCount               SEPARATOR
unsigned int                                                       initialSelectionIndex    SEPARATOR
unsigned int                                                       selectionCount           SEPARATOR
unsigned int                                                       highlightBaseGraphicIndex SEPARATOR
unsigned int                                                       activateGraphicBaseIndex SEPARATOR
enum MemoryGraphicsDatMenuScreen_Compat                           initialScreen            SEPARATOR
struct MemoryGraphicsDatMenuActivateConsequenceResult_Compat*      outResult                FINAL_SEPARATOR
{
        unsigned int finalSelectionIndex;
        unsigned int initialRenderVariant;

        memset(outResult, 0, sizeof(*outResult));
        outResult->initialScreen = initialScreen;
        outResult->finalScreen = initialScreen;
        outResult->initialSelectionIndex = initialSelectionIndex;
        if (!F0479_MEMORY_RunMenuActivateMini_Compat(
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
                &outResult->activate)) {
                return 0;
        }
        if (outResult->activate.activationTriggered) {
                outResult->finalScreen = MEMORY_GRAPHICS_DAT_MENU_SCREEN_SUBMENU;
                outResult->screenChanged = (outResult->finalScreen != outResult->initialScreen);
        }
        finalSelectionIndex = outResult->activate.render.menuState.finalSelectionIndex;
        outResult->finalSelectionIndex = finalSelectionIndex;
        outResult->selectionChanged =
                (finalSelectionIndex != initialSelectionIndex) ? 1 : 0;
        if (outResult->selectionChanged) {
                outResult->selectionDelta = (finalSelectionIndex >= initialSelectionIndex)
                        ? (finalSelectionIndex - initialSelectionIndex)
                        : (initialSelectionIndex - finalSelectionIndex);
        }
        initialRenderVariant = (selectionCount != 0U)
                ? (initialSelectionIndex % selectionCount)
                : 0U;
        outResult->renderVariantChanged =
                (outResult->activate.render.selectedRenderVariant != initialRenderVariant) ? 1 : 0;
        outResult->activationCommitted = outResult->activate.activationTriggered;
        outResult->enteredSubmenu =
                ((initialScreen == MEMORY_GRAPHICS_DAT_MENU_SCREEN_MENU) &&
                 (outResult->finalScreen == MEMORY_GRAPHICS_DAT_MENU_SCREEN_SUBMENU))
                ? 1 : 0;
        outResult->remainedInScreen = (outResult->screenChanged == 0) ? 1 : 0;
        if (outResult->selectionChanged) {
                outResult->gameStateMask |= MEMORY_GRAPHICS_DAT_MENU_GAME_STATE_MASK_NAVIGATED;
        }
        if (outResult->activationCommitted) {
                outResult->gameStateMask |= MEMORY_GRAPHICS_DAT_MENU_GAME_STATE_MASK_ACTIVATED;
        }
        if (outResult->screenChanged) {
                outResult->gameStateMask |= MEMORY_GRAPHICS_DAT_MENU_GAME_STATE_MASK_SCREEN_CHANGED;
        }
        if (outResult->renderVariantChanged) {
                outResult->gameStateMask |= MEMORY_GRAPHICS_DAT_MENU_GAME_STATE_MASK_RENDER_VARIANT_CHANGED;
        }
        if ((outResult->gameStateMask & (MEMORY_GRAPHICS_DAT_MENU_GAME_STATE_MASK_NAVIGATED |
                                         MEMORY_GRAPHICS_DAT_MENU_GAME_STATE_MASK_ACTIVATED |
                                         MEMORY_GRAPHICS_DAT_MENU_GAME_STATE_MASK_SCREEN_CHANGED |
                                         MEMORY_GRAPHICS_DAT_MENU_GAME_STATE_MASK_RENDER_VARIANT_CHANGED)) == 0U) {
                outResult->gameStateMask |= MEMORY_GRAPHICS_DAT_MENU_GAME_STATE_MASK_HELD;
        }
        if (outResult->enteredSubmenu) {
                outResult->gameStateClass = MEMORY_GRAPHICS_DAT_MENU_GAME_STATE_ENTERED_SUBMENU;
        } else if (outResult->activationCommitted) {
                outResult->gameStateClass = MEMORY_GRAPHICS_DAT_MENU_GAME_STATE_ACTIVATED;
        } else if (outResult->selectionChanged) {
                outResult->gameStateClass = MEMORY_GRAPHICS_DAT_MENU_GAME_STATE_NAVIGATED;
        } else if (outResult->gameStateMask & MEMORY_GRAPHICS_DAT_MENU_GAME_STATE_MASK_HELD) {
                outResult->gameStateClass = MEMORY_GRAPHICS_DAT_MENU_GAME_STATE_HELD;
        } else {
                outResult->gameStateClass = MEMORY_GRAPHICS_DAT_MENU_GAME_STATE_NONE;
        }
        return 1;
}

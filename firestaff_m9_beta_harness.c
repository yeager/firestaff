#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stateful_boot_plan_reachability_pc34_compat.h"
#include "memory_graphics_dat_header_pc34_compat.h"
#include "memory_graphics_dat_menu_activate_consequence_pc34_compat.h"
#include "memory_graphics_dat_submenu_consequence_pc34_compat.h"
#include "memory_graphics_dat_startup_pc34_compat.h"
#include "memory_graphics_dat_viewport_path_pc34_compat.h"
#include "dialog_frontend_pc34_compat.h"
#include "screen_bitmap_present_pc34_compat.h"
#include "host_video_pgm_backend_pc34_compat.h"
#include "startup_runtime_driver_pc34_compat.h"

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

static int parse_events(
    const char* text,
    enum MemoryGraphicsDatEvent_Compat* events,
    unsigned int capacity,
    unsigned int* outCount) {
    unsigned int count = 0;
    const unsigned char* p = (const unsigned char*)text;

    while (*p != '\0') {
        unsigned char c = (unsigned char)tolower(*p++);
        enum MemoryGraphicsDatEvent_Compat event;

        if ((c == ' ') || (c == '\t') || (c == '\n') || (c == '\r') || (c == ',') || (c == ';') || (c == '-')) {
            continue;
        }
        if (count >= capacity) {
            return 0;
        }
        switch (c) {
            case 'f': event = MEMORY_GRAPHICS_DAT_EVENT_FRAME; break;
            case 'n': event = MEMORY_GRAPHICS_DAT_EVENT_ADVANCE; break;
            case 'a': event = MEMORY_GRAPHICS_DAT_EVENT_ACTIVATE; break;
            case 'i': event = MEMORY_GRAPHICS_DAT_EVENT_IDLE; break;
            case 'b': event = MEMORY_GRAPHICS_DAT_EVENT_BACK; break;
            case 'c': event = MEMORY_GRAPHICS_DAT_EVENT_CANCEL; break;
            case 'r': event = MEMORY_GRAPHICS_DAT_EVENT_RETURN_TO_MENU; break;
            default: return 0;
        }
        events[count++] = event;
    }
    *outCount = count;
    return (count > 0U);
}

static const char* event_name(enum MemoryGraphicsDatEvent_Compat event) {
    switch (event) {
        case MEMORY_GRAPHICS_DAT_EVENT_IDLE: return "IDLE";
        case MEMORY_GRAPHICS_DAT_EVENT_FRAME: return "FRAME";
        case MEMORY_GRAPHICS_DAT_EVENT_ADVANCE: return "ADVANCE";
        case MEMORY_GRAPHICS_DAT_EVENT_ACTIVATE: return "ACTIVATE";
        case MEMORY_GRAPHICS_DAT_EVENT_BACK: return "BACK";
        case MEMORY_GRAPHICS_DAT_EVENT_CANCEL: return "CANCEL";
        case MEMORY_GRAPHICS_DAT_EVENT_RETURN_TO_MENU: return "RETURN_TO_MENU";
        default: return "UNKNOWN";
    }
}

static int run_prefix(
    const char* graphicsDatPath,
    const enum MemoryGraphicsDatEvent_Compat* events,
    unsigned int eventCount,
    unsigned int dialogGraphicIndex,
    unsigned int viewportGraphicIndex,
    unsigned int initialSelectionIndex,
    unsigned int selectionCount,
    unsigned int highlightBaseGraphicIndex,
    unsigned int activateGraphicBaseIndex,
    enum MemoryGraphicsDatMenuScreen_Compat initialScreen,
    struct MemoryGraphicsDatHeader_Compat* header,
    struct MemoryGraphicsDatMenuActivateConsequenceResult_Compat* outResult) {
    struct MemoryGraphicsDatState_Compat fileState;
    unsigned int viewportBytes;
    unsigned int bitmapBytes = 8192U;
    unsigned int i;
    unsigned char* viewportGraphicBuffer;
    unsigned char* viewportStorage;
    unsigned char* viewportBitmap;
    int ok;

    memset(&fileState, 0, sizeof(fileState));
    memset(outResult, 0, sizeof(*outResult));
    viewportBytes = header->compressedByteCounts[viewportGraphicIndex];
    if (viewportBytes < 16U) viewportBytes = 16U;
    if ((header != 0) && (header->graphicCount > 0)) {
        for (i = 0; i < header->graphicCount; ++i) {
            unsigned int width = header->widthHeight[i].Width;
            unsigned int height = header->widthHeight[i].Height;
            unsigned int candidate = (((width + 1U) & 0xFFFEU) >> 1U) * height;
            if (candidate > bitmapBytes) {
                bitmapBytes = candidate;
            }
        }
    }
    viewportGraphicBuffer = (unsigned char*)calloc((size_t)viewportBytes + 16U, 1);
    viewportStorage = (unsigned char*)calloc((size_t)bitmapBytes + 260U, 1);
    viewportBitmap = (viewportStorage == 0) ? 0 : (viewportStorage + 4);
    if ((viewportGraphicBuffer == 0) || (viewportStorage == 0)) {
        free(viewportGraphicBuffer);
        free(viewportStorage);
        return 0;
    }
    ok = F0479_MEMORY_RunMenuActivateConsequenceMini_Compat(
        graphicsDatPath,
        &fileState,
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
        outResult);
    free(viewportGraphicBuffer);
    free(viewportStorage);
    return ok;
}

struct BetaHarnessSessionState_Compat {
    unsigned int selectionIndex;
    unsigned int selectedRenderVariant;
    enum MemoryGraphicsDatMenuScreen_Compat screen;
    unsigned int renderGraphicIndex;
    int activationTriggered;
    int anyActivationTriggered;
    int lastScreenChanged;
    int anyScreenChanged;
    int lastSelectionChanged;
    int lastRenderVariantChanged;
    int lastActivationCommitted;
    int lastEnteredSubmenu;
    int lastRemainedInScreen;
    unsigned int lastGameStateClass;
    unsigned int lastGameStateMask;
    unsigned int selectionChangedCount;
    unsigned int renderVariantChangedCount;
    unsigned int activationCommittedCount;
    unsigned int enteredSubmenuCount;
    unsigned int gameStateMask;
    unsigned int gameStateCumulativeClass;
    int lastSubmenuHoldApplied;
    int lastSubmenuReactivateHandled;
    unsigned int lastSubmenuBehaviorClass;
    unsigned int lastSubmenuCumulativeBehaviorClass;
    unsigned int lastSubmenuBehaviorMask;
    unsigned int lastSubmenuFrameEventCount;
    unsigned int lastSubmenuIdleEventCount;
    unsigned int lastSubmenuAdvanceSuppressedCount;
    int lastSubmenuExitApplied;
    unsigned int lastSubmenuExitClass;
    unsigned int lastSubmenuCumulativeExitClass;
    unsigned int lastSubmenuExitMask;
    unsigned int lastSubmenuReturnExitCount;
    unsigned int lastSubmenuCancelExitCount;
    unsigned int submenuEventCount;
    unsigned int submenuHoldAppliedCount;
    unsigned int submenuReactivateHandledCount;
    unsigned int submenuBehaviorMask;
    unsigned int submenuCumulativeBehaviorClass;
    unsigned int submenuFrameEventCount;
    unsigned int submenuIdleEventCount;
    unsigned int submenuAdvanceSuppressedCount;
    unsigned int submenuBackEventCount;
    unsigned int submenuCancelEventCount;
    unsigned int submenuExitAppliedCount;
    unsigned int submenuExitMask;
    unsigned int submenuCumulativeExitClass;
    unsigned int submenuReturnExitCount;
    unsigned int submenuCancelExitCount;
    unsigned int activatedSelectionIndex;
    unsigned int activatedGraphicIndex;
    unsigned int totalEventCount;
    unsigned int commitHistoryClass;
    unsigned int commitHistoryMask;
    unsigned int distinctCommittedSelections;
    unsigned int firstCommittedSelectionIndex;
    int firstCommittedSelectionSet;
    int commitSelectionVaried;
    unsigned int submenuReturnToMenuCount;
    unsigned int submenuRoundTripCount;
    unsigned int lastSubmenuInternalClass;
    unsigned int lastSubmenuInternalMask;
    unsigned int submenuInternalMask;
    unsigned int submenuCumulativeInternalClass;
};

struct BetaHarnessPersistentRuntime_Compat {
    struct MemoryGraphicsDatState_Compat fileState;
    struct MemoryGraphicsDatStartupResult_Compat startup;
    unsigned char* viewportGraphicBuffer;
    unsigned char* viewportStorage;
    unsigned char* viewportBitmap;
    unsigned char* screenStorage;
    unsigned char* screenBitmap;
    unsigned int viewportBytes;
    unsigned int screenBytes;
    int initialized;
};

static int print_final_result(
    const struct BetaHarnessSessionState_Compat* session) {
    printf("totalEventCount=%u\n", session->totalEventCount);
    printf("finalSelectionIndex=%u\n", session->selectionIndex);
    printf("selectedRenderVariant=%u\n", session->selectedRenderVariant);
    printf("activationTriggered=%d\n", session->activationTriggered);
    printf("anyActivationTriggered=%d\n", session->anyActivationTriggered);
    printf("activatedSelectionIndex=%u\n", session->activatedSelectionIndex);
    printf("activatedGraphicIndex=%u\n", session->activatedGraphicIndex);
    printf("finalScreen=%u\n", (unsigned int)session->screen);
    printf("lastScreenChanged=%d\n", session->lastScreenChanged);
    printf("screenChanged=%d\n", session->anyScreenChanged);
    printf("lastSelectionChanged=%d\n", session->lastSelectionChanged);
    printf("lastRenderVariantChanged=%d\n", session->lastRenderVariantChanged);
    printf("lastActivationCommitted=%d\n", session->lastActivationCommitted);
    printf("lastEnteredSubmenu=%d\n", session->lastEnteredSubmenu);
    printf("lastRemainedInScreen=%d\n", session->lastRemainedInScreen);
    printf("lastGameStateClass=%u\n", session->lastGameStateClass);
    printf("lastGameStateMask=%u\n", session->lastGameStateMask);
    printf("selectionChangedCount=%u\n", session->selectionChangedCount);
    printf("renderVariantChangedCount=%u\n", session->renderVariantChangedCount);
    printf("activationCommittedCount=%u\n", session->activationCommittedCount);
    printf("enteredSubmenuCount=%u\n", session->enteredSubmenuCount);
    printf("gameStateMask=%u\n", session->gameStateMask);
    printf("gameStateCumulativeClass=%u\n", session->gameStateCumulativeClass);
    printf("lastSubmenuHoldApplied=%d\n", session->lastSubmenuHoldApplied);
    printf("lastSubmenuReactivateHandled=%d\n", session->lastSubmenuReactivateHandled);
    printf("lastSubmenuBehaviorClass=%u\n", session->lastSubmenuBehaviorClass);
    printf("lastSubmenuCumulativeBehaviorClass=%u\n", session->lastSubmenuCumulativeBehaviorClass);
    printf("lastSubmenuBehaviorMask=%u\n", session->lastSubmenuBehaviorMask);
    printf("lastSubmenuFrameEventCount=%u\n", session->lastSubmenuFrameEventCount);
    printf("lastSubmenuIdleEventCount=%u\n", session->lastSubmenuIdleEventCount);
    printf("lastSubmenuAdvanceSuppressedCount=%u\n", session->lastSubmenuAdvanceSuppressedCount);
    printf("lastSubmenuExitApplied=%d\n", session->lastSubmenuExitApplied);
    printf("lastSubmenuExitClass=%u\n", session->lastSubmenuExitClass);
    printf("lastSubmenuCumulativeExitClass=%u\n", session->lastSubmenuCumulativeExitClass);
    printf("lastSubmenuExitMask=%u\n", session->lastSubmenuExitMask);
    printf("lastSubmenuReturnExitCount=%u\n", session->lastSubmenuReturnExitCount);
    printf("lastSubmenuCancelExitCount=%u\n", session->lastSubmenuCancelExitCount);
    printf("submenuEventCount=%u\n", session->submenuEventCount);
    printf("submenuHoldAppliedCount=%u\n", session->submenuHoldAppliedCount);
    printf("submenuReactivateHandledCount=%u\n", session->submenuReactivateHandledCount);
    printf("submenuBehaviorMask=%u\n", session->submenuBehaviorMask);
    printf("submenuCumulativeBehaviorClass=%u\n", session->submenuCumulativeBehaviorClass);
    printf("submenuFrameEventCount=%u\n", session->submenuFrameEventCount);
    printf("submenuIdleEventCount=%u\n", session->submenuIdleEventCount);
    printf("submenuAdvanceSuppressedCount=%u\n", session->submenuAdvanceSuppressedCount);
    printf("submenuBackEventCount=%u\n", session->submenuBackEventCount);
    printf("submenuCancelEventCount=%u\n", session->submenuCancelEventCount);
    printf("submenuExitAppliedCount=%u\n", session->submenuExitAppliedCount);
    printf("submenuExitMask=%u\n", session->submenuExitMask);
    printf("submenuCumulativeExitClass=%u\n", session->submenuCumulativeExitClass);
    printf("submenuReturnExitCount=%u\n", session->submenuReturnExitCount);
    printf("submenuCancelExitCount=%u\n", session->submenuCancelExitCount);
    printf("commitHistoryClass=%u\n", session->commitHistoryClass);
    printf("commitHistoryMask=%u\n", session->commitHistoryMask);
    printf("distinctCommittedSelections=%u\n", session->distinctCommittedSelections);
    printf("submenuReturnToMenuCount=%u\n", session->submenuReturnToMenuCount);
    printf("submenuRoundTripCount=%u\n", session->submenuRoundTripCount);
    printf("lastSubmenuInternalClass=%u\n", session->lastSubmenuInternalClass);
    printf("lastSubmenuInternalMask=%u\n", session->lastSubmenuInternalMask);
    printf("submenuInternalMask=%u\n", session->submenuInternalMask);
    printf("submenuCumulativeInternalClass=%u\n", session->submenuCumulativeInternalClass);
    return 1;
}

static int run_single_event(
    const char* graphicsDatPath,
    unsigned int dialogGraphicIndex,
    unsigned int viewportGraphicIndex,
    unsigned int selectionCount,
    unsigned int highlightBaseGraphicIndex,
    unsigned int activateGraphicBaseIndex,
    struct MemoryGraphicsDatHeader_Compat* header,
    struct BetaHarnessSessionState_Compat* session,
    enum MemoryGraphicsDatEvent_Compat event,
    struct MemoryGraphicsDatSubmenuConsequenceResult_Compat* outResult) {
    struct MemoryGraphicsDatState_Compat fileState;
    unsigned int viewportBytes;
    unsigned int bitmapBytes = 8192U;
    unsigned int i;
    unsigned char* viewportGraphicBuffer;
    unsigned char* viewportStorage;
    unsigned char* viewportBitmap;
    int ok;

    memset(&fileState, 0, sizeof(fileState));
    memset(outResult, 0, sizeof(*outResult));
    viewportBytes = header->compressedByteCounts[viewportGraphicIndex];
    if (viewportBytes < 16U) viewportBytes = 16U;
    if ((header != 0) && (header->graphicCount > 0)) {
        for (i = 0; i < header->graphicCount; ++i) {
            unsigned int width = header->widthHeight[i].Width;
            unsigned int height = header->widthHeight[i].Height;
            unsigned int candidate = (((width + 1U) & 0xFFFEU) >> 1U) * height;
            if (candidate > bitmapBytes) {
                bitmapBytes = candidate;
            }
        }
    }
    viewportGraphicBuffer = (unsigned char*)calloc((size_t)viewportBytes + 16U, 1);
    viewportStorage = (unsigned char*)calloc((size_t)bitmapBytes + 260U, 1);
    viewportBitmap = (viewportStorage == 0) ? 0 : (viewportStorage + 4);
    if ((viewportGraphicBuffer == 0) || (viewportStorage == 0)) {
        free(viewportGraphicBuffer);
        free(viewportStorage);
        return 0;
    }
    ok = F0479_MEMORY_RunSubmenuConsequenceMini_Compat(
        graphicsDatPath,
        &fileState,
        dialogGraphicIndex,
        viewportGraphicIndex,
        viewportGraphicBuffer,
        viewportBitmap,
        &event,
        1U,
        session->selectionIndex,
        selectionCount,
        highlightBaseGraphicIndex,
        activateGraphicBaseIndex,
        session->screen,
        session->renderGraphicIndex,
        outResult);
    free(viewportGraphicBuffer);
    free(viewportStorage);
    return ok;
}

static void apply_single_event_result(
    struct BetaHarnessSessionState_Compat* session,
    const struct MemoryGraphicsDatSubmenuConsequenceResult_Compat* result) {
    session->screen = result->consequence.finalScreen;
    session->activationTriggered = result->consequence.activate.activationTriggered;
    session->lastScreenChanged = result->consequence.screenChanged;
    session->lastSelectionChanged = result->consequence.selectionChanged;
    session->lastRenderVariantChanged = result->consequence.renderVariantChanged;
    session->lastActivationCommitted = result->consequence.activationCommitted;
    session->lastEnteredSubmenu = result->consequence.enteredSubmenu;
    session->lastRemainedInScreen = result->consequence.remainedInScreen;
    session->lastGameStateClass = (unsigned int)result->consequence.gameStateClass;
    session->lastGameStateMask = result->consequence.gameStateMask;
    if (result->consequence.selectionChanged) {
        session->selectionChangedCount++;
    }
    if (result->consequence.renderVariantChanged) {
        session->renderVariantChangedCount++;
    }
    if (result->consequence.activationCommitted) {
        session->activationCommittedCount++;
        session->commitHistoryMask |= MEMORY_GRAPHICS_DAT_MENU_COMMIT_HISTORY_MASK_COMMITTED;
        if (!session->firstCommittedSelectionSet) {
            session->firstCommittedSelectionIndex = session->selectionIndex;
            session->firstCommittedSelectionSet = 1;
            session->distinctCommittedSelections = 1;
        } else {
            session->commitHistoryMask |= MEMORY_GRAPHICS_DAT_MENU_COMMIT_HISTORY_MASK_REPEATED;
            if (session->selectionIndex != session->firstCommittedSelectionIndex) {
                session->commitHistoryMask |= MEMORY_GRAPHICS_DAT_MENU_COMMIT_HISTORY_MASK_VARIED;
                if (!session->commitSelectionVaried) {
                    session->commitSelectionVaried = 1;
                    session->distinctCommittedSelections++;
                }
            }
        }
        if (session->commitHistoryMask & MEMORY_GRAPHICS_DAT_MENU_COMMIT_HISTORY_MASK_VARIED) {
            session->commitHistoryClass = MEMORY_GRAPHICS_DAT_MENU_COMMIT_HISTORY_REPEATED_VARIED;
        } else if (session->commitHistoryMask & MEMORY_GRAPHICS_DAT_MENU_COMMIT_HISTORY_MASK_REPEATED) {
            session->commitHistoryClass = MEMORY_GRAPHICS_DAT_MENU_COMMIT_HISTORY_REPEATED_SAME;
        } else {
            session->commitHistoryClass = MEMORY_GRAPHICS_DAT_MENU_COMMIT_HISTORY_SINGLE;
        }
    }
    if (result->consequence.enteredSubmenu) {
        if (session->enteredSubmenuCount > 0U) {
            session->submenuRoundTripCount++;
        }
        session->enteredSubmenuCount++;
    }
    session->gameStateMask |= result->consequence.gameStateMask;
    if (session->enteredSubmenuCount > 0U) {
        session->gameStateCumulativeClass = MEMORY_GRAPHICS_DAT_MENU_GAME_STATE_ENTERED_SUBMENU;
    } else if (session->activationCommittedCount > 0U) {
        session->gameStateCumulativeClass = MEMORY_GRAPHICS_DAT_MENU_GAME_STATE_ACTIVATED;
    } else if (session->selectionChangedCount > 0U) {
        session->gameStateCumulativeClass = MEMORY_GRAPHICS_DAT_MENU_GAME_STATE_NAVIGATED;
    } else if (session->gameStateMask & MEMORY_GRAPHICS_DAT_MENU_GAME_STATE_MASK_HELD) {
        session->gameStateCumulativeClass = MEMORY_GRAPHICS_DAT_MENU_GAME_STATE_HELD;
    } else {
        session->gameStateCumulativeClass = MEMORY_GRAPHICS_DAT_MENU_GAME_STATE_NONE;
    }
    session->lastSubmenuHoldApplied = result->submenuHoldApplied;
    session->lastSubmenuReactivateHandled = result->submenuReactivateHandled;
    session->lastSubmenuBehaviorClass = (unsigned int)result->behaviorClass;
    session->lastSubmenuCumulativeBehaviorClass = (unsigned int)result->cumulativeBehaviorClass;
    session->lastSubmenuBehaviorMask = result->behaviorMask;
    session->lastSubmenuFrameEventCount = result->submenuFrameEventCount;
    session->lastSubmenuIdleEventCount = result->submenuIdleEventCount;
    session->lastSubmenuAdvanceSuppressedCount = result->submenuAdvanceSuppressedCount;
    session->lastSubmenuExitApplied = result->submenuExitApplied;
    session->lastSubmenuExitClass = (unsigned int)result->exitClass;
    session->lastSubmenuCumulativeExitClass = (unsigned int)result->cumulativeExitClass;
    session->lastSubmenuExitMask = result->exitMask;
    session->lastSubmenuReturnExitCount = result->submenuReturnExitCount;
    session->lastSubmenuCancelExitCount = result->submenuCancelExitCount;
    if (result->consequence.activate.activationTriggered) {
        session->anyActivationTriggered = 1;
    }
    if (result->consequence.screenChanged) {
        session->anyScreenChanged = 1;
    }
    session->submenuEventCount += result->submenuEventCount;
    if (result->submenuHoldApplied) {
        session->submenuHoldAppliedCount++;
    }
    if (result->submenuReactivateHandled) {
        session->submenuReactivateHandledCount++;
    }
    session->submenuBehaviorMask |= result->behaviorMask;
    if (session->submenuBehaviorMask == MEMORY_GRAPHICS_DAT_SUBMENU_BEHAVIOR_MASK_NONE) {
        session->submenuCumulativeBehaviorClass = MEMORY_GRAPHICS_DAT_SUBMENU_BEHAVIOR_NONE;
    } else if (session->submenuBehaviorMask == MEMORY_GRAPHICS_DAT_SUBMENU_BEHAVIOR_MASK_REACTIVATE) {
        session->submenuCumulativeBehaviorClass = MEMORY_GRAPHICS_DAT_SUBMENU_BEHAVIOR_REACTIVATE;
    } else if (session->submenuBehaviorMask == MEMORY_GRAPHICS_DAT_SUBMENU_BEHAVIOR_MASK_FRAME_HOLD) {
        session->submenuCumulativeBehaviorClass = MEMORY_GRAPHICS_DAT_SUBMENU_BEHAVIOR_FRAME_HOLD;
    } else if (session->submenuBehaviorMask == MEMORY_GRAPHICS_DAT_SUBMENU_BEHAVIOR_MASK_IDLE_HOLD) {
        session->submenuCumulativeBehaviorClass = MEMORY_GRAPHICS_DAT_SUBMENU_BEHAVIOR_IDLE_HOLD;
    } else if (session->submenuBehaviorMask == MEMORY_GRAPHICS_DAT_SUBMENU_BEHAVIOR_MASK_ADVANCE_SUPPRESSED_HOLD) {
        session->submenuCumulativeBehaviorClass = MEMORY_GRAPHICS_DAT_SUBMENU_BEHAVIOR_ADVANCE_SUPPRESSED_HOLD;
    } else if (session->submenuBehaviorMask == MEMORY_GRAPHICS_DAT_SUBMENU_BEHAVIOR_MASK_BACK) {
        session->submenuCumulativeBehaviorClass = MEMORY_GRAPHICS_DAT_SUBMENU_BEHAVIOR_BACK;
    } else if (session->submenuBehaviorMask == MEMORY_GRAPHICS_DAT_SUBMENU_BEHAVIOR_MASK_CANCEL) {
        session->submenuCumulativeBehaviorClass = MEMORY_GRAPHICS_DAT_SUBMENU_BEHAVIOR_CANCEL;
    } else if (session->submenuBehaviorMask == MEMORY_GRAPHICS_DAT_SUBMENU_BEHAVIOR_MASK_RETURN_TO_MENU) {
        session->submenuCumulativeBehaviorClass = MEMORY_GRAPHICS_DAT_SUBMENU_BEHAVIOR_RETURN_TO_MENU;
    } else {
        session->submenuCumulativeBehaviorClass = MEMORY_GRAPHICS_DAT_SUBMENU_BEHAVIOR_MIXED_HOLD;
    }
    session->submenuFrameEventCount += result->submenuFrameEventCount;
    session->submenuIdleEventCount += result->submenuIdleEventCount;
    session->submenuAdvanceSuppressedCount += result->submenuAdvanceSuppressedCount;
    session->submenuBackEventCount += result->submenuBackEventCount;
    session->submenuCancelEventCount += result->submenuCancelEventCount;
    if (result->submenuExitApplied) {
        session->submenuExitAppliedCount++;
    }
    session->submenuExitMask |= result->exitMask;
    if (session->submenuExitMask == MEMORY_GRAPHICS_DAT_SUBMENU_EXIT_MASK_NONE) {
        session->submenuCumulativeExitClass = MEMORY_GRAPHICS_DAT_SUBMENU_EXIT_NONE;
    } else if (session->submenuExitMask == MEMORY_GRAPHICS_DAT_SUBMENU_EXIT_MASK_RETURN) {
        session->submenuCumulativeExitClass = MEMORY_GRAPHICS_DAT_SUBMENU_EXIT_RETURN;
    } else if (session->submenuExitMask == MEMORY_GRAPHICS_DAT_SUBMENU_EXIT_MASK_CANCEL) {
        session->submenuCumulativeExitClass = MEMORY_GRAPHICS_DAT_SUBMENU_EXIT_CANCEL;
    } else {
        session->submenuCumulativeExitClass = MEMORY_GRAPHICS_DAT_SUBMENU_EXIT_MIXED;
    }
    session->submenuReturnExitCount += result->submenuReturnExitCount;
    session->submenuCancelExitCount += result->submenuCancelExitCount;
    session->lastSubmenuInternalClass = (unsigned int)result->internalClass;
    session->lastSubmenuInternalMask = result->internalMask;
    session->submenuInternalMask |= result->internalMask;
    switch (session->submenuInternalMask) {
    case MEMORY_GRAPHICS_DAT_SUBMENU_INTERNAL_MASK_NONE:
        session->submenuCumulativeInternalClass = MEMORY_GRAPHICS_DAT_SUBMENU_INTERNAL_NONE; break;
    case MEMORY_GRAPHICS_DAT_SUBMENU_INTERNAL_MASK_HOLD:
        session->submenuCumulativeInternalClass = MEMORY_GRAPHICS_DAT_SUBMENU_INTERNAL_HOLD; break;
    case MEMORY_GRAPHICS_DAT_SUBMENU_INTERNAL_MASK_REACTIVATE:
        session->submenuCumulativeInternalClass = MEMORY_GRAPHICS_DAT_SUBMENU_INTERNAL_REACTIVATE; break;
    case MEMORY_GRAPHICS_DAT_SUBMENU_INTERNAL_MASK_EXIT:
        session->submenuCumulativeInternalClass = MEMORY_GRAPHICS_DAT_SUBMENU_INTERNAL_EXIT; break;
    case MEMORY_GRAPHICS_DAT_SUBMENU_INTERNAL_MASK_RETURN:
        session->submenuCumulativeInternalClass = MEMORY_GRAPHICS_DAT_SUBMENU_INTERNAL_RETURN; break;
    default:
        session->submenuCumulativeInternalClass = MEMORY_GRAPHICS_DAT_SUBMENU_INTERNAL_MIXED; break;
    }
    if (result->submenuReturnToMenuApplied) {
        session->submenuReturnToMenuCount += result->submenuReturnToMenuCount;
        session->renderGraphicIndex = result->finalRenderGraphicIndex;
        session->totalEventCount++;
        return;
    }
    if (result->submenuModeActive && result->submenuHoldApplied) {
        session->renderGraphicIndex = result->finalRenderGraphicIndex;
        session->totalEventCount++;
        return;
    }
    session->selectionIndex = result->consequence.activate.render.menuState.finalSelectionIndex;
    session->selectedRenderVariant = result->consequence.activate.render.selectedRenderVariant;
    if (result->consequence.activate.activationTriggered) {
        session->activatedSelectionIndex = result->consequence.activate.activatedSelectionIndex;
        session->activatedGraphicIndex = result->consequence.activate.activatedGraphicIndex;
    }
    session->renderGraphicIndex = result->finalRenderGraphicIndex;
    session->totalEventCount++;
}

static void free_persistent_runtime(
    struct BetaHarnessPersistentRuntime_Compat* runtime) {
    if (runtime->startup.runtimeInitialized || runtime->initialized) {
        F0479_MEMORY_FreeStartupGraphicsChain_Compat(&runtime->startup);
    }
    free(runtime->viewportGraphicBuffer);
    free(runtime->viewportStorage);
    free(runtime->screenStorage);
    memset(runtime, 0, sizeof(*runtime));
}

static int initialize_persistent_runtime(
    const char* graphicsDatPath,
    const struct MemoryGraphicsDatHeader_Compat* header,
    unsigned int dialogGraphicIndex,
    unsigned int initialViewportGraphicIndex,
    struct BetaHarnessPersistentRuntime_Compat* runtime) {
    unsigned int i;
    unsigned int maxCompressed = 16384U;
    unsigned int maxBitmapBytes = 8192U;

    memset(runtime, 0, sizeof(*runtime));
    if ((header != 0) && (header->graphicCount > 0)) {
        for (i = 0; i < header->graphicCount; ++i) {
            unsigned int width = header->widthHeight[i].Width;
            unsigned int height = header->widthHeight[i].Height;
            unsigned int bitmapBytes = (((width + 1U) & 0xFFFEU) >> 1U) * height;
            if (header->compressedByteCounts[i] > maxCompressed) {
                maxCompressed = header->compressedByteCounts[i];
            }
            if (bitmapBytes > maxBitmapBytes) {
                maxBitmapBytes = bitmapBytes;
            }
        }
    }
    runtime->viewportBytes = maxCompressed + 64U;
    runtime->screenBytes = 320U * 200U / 2U;
    runtime->viewportGraphicBuffer = (unsigned char*)calloc((size_t)runtime->viewportBytes, 1);
    runtime->viewportStorage = (unsigned char*)calloc((size_t)maxBitmapBytes + 64U, 1);
    runtime->screenStorage = (unsigned char*)calloc((size_t)runtime->screenBytes + 4U, 1);
    if ((runtime->viewportGraphicBuffer == 0) || (runtime->viewportStorage == 0) || (runtime->screenStorage == 0)) {
        free_persistent_runtime(runtime);
        return 0;
    }
    runtime->viewportBitmap = runtime->viewportStorage + 4;
    runtime->screenBitmap = runtime->screenStorage + 4;
    if (!F0479_MEMORY_StartupGraphicsChain_Compat(
            graphicsDatPath,
            &runtime->fileState,
            dialogGraphicIndex,
            initialViewportGraphicIndex,
            runtime->viewportGraphicBuffer,
            runtime->viewportBitmap,
            &runtime->startup)) {
        free_persistent_runtime(runtime);
        return 0;
    }
    runtime->initialized = 1;
    return 1;
}

static int render_graphic_with_runtime(
    const char* graphicsDatPath,
    const char* outputPath,
    unsigned int dialogGraphicIndex,
    unsigned int graphicIndex,
    unsigned int frameNumber,
    struct BetaHarnessPersistentRuntime_Compat* runtime) {
    struct ScreenBitmapPresentResult_Compat presentResult;
    struct HostVideoPgmBackendResult_Compat hostResult;
    struct MemoryGraphicsDatTransactionResult_Compat txResult;
    struct MemoryGraphicsDatSelection_Compat selection;
    const struct GraphicWidthHeight_Compat* sizeInfo;

    memset(&presentResult, 0, sizeof(presentResult));
    memset(&hostResult, 0, sizeof(hostResult));
    memset(&txResult, 0, sizeof(txResult));
    memset(&selection, 0, sizeof(selection));
    memset(runtime->viewportStorage, 0, (size_t)runtime->viewportBytes + 8192U);
    memset(runtime->screenStorage, 0, (size_t)runtime->screenBytes + 4U);
    if (!F0490_MEMORY_LoadViewportGraphicByIndex_Compat(
            graphicsDatPath,
            &runtime->startup.runtimeState,
            &runtime->fileState,
            graphicIndex,
            0,
            runtime->viewportGraphicBuffer,
            runtime->viewportBitmap,
            &txResult,
            &selection)) {
        return 0;
    }
    sizeInfo = &runtime->startup.runtimeState.widthHeight[dialogGraphicIndex];
    F0427_DIALOG_DrawBackdrop_Compat(runtime->startup.specials.dialogBoxGraphic, runtime->screenBitmap, sizeInfo);
    if (!F9005_SCREEN_OverlayBitmapOnScreen_Compat(runtime->viewportBitmap, runtime->screenBitmap, 0, &presentResult)) {
        return 0;
    }
    return F9002_HOSTVIDEO_PublishFrameToPgm_Compat(runtime->screenBitmap, frameNumber, outputPath, &hostResult);
}

static int publish_session_step(
    const char* graphicsDatPath,
    const char* outputPrefix,
    unsigned int dialogGraphicIndex,
    unsigned int frameNumber,
    unsigned int stepIndex,
    enum MemoryGraphicsDatEvent_Compat event,
    const struct BetaHarnessSessionState_Compat* session,
    int screenChanged,
    struct BetaHarnessPersistentRuntime_Compat* runtime) {
    char outputPath[1024];

    snprintf(outputPath, sizeof(outputPath), "%s_event_%04u.pgm", outputPrefix, frameNumber);
    if (!render_graphic_with_runtime(
            graphicsDatPath,
            outputPath,
            dialogGraphicIndex,
            session->renderGraphicIndex,
            frameNumber,
            runtime)) {
        return 0;
    }
    printf("event[%u]=%s\n", stepIndex, event_name(event));
    printf("frame[%u]=%u\n", stepIndex, frameNumber);
    printf("selection[%u]=%u\n", stepIndex, session->selectionIndex);
    printf("renderVariant[%u]=%u\n", stepIndex, session->selectedRenderVariant);
    printf("renderGraphic[%u]=%u\n", stepIndex, session->renderGraphicIndex);
    printf("activationTriggered[%u]=%d\n", stepIndex, session->activationTriggered);
    printf("anyActivationTriggered[%u]=%d\n", stepIndex, session->anyActivationTriggered);
    printf("screenChanged[%u]=%d\n", stepIndex, screenChanged);
    printf("anyScreenChanged[%u]=%d\n", stepIndex, session->anyScreenChanged);
    printf("submenuHoldApplied[%u]=%d\n", stepIndex, session->lastSubmenuHoldApplied);
    printf("submenuReactivateHandled[%u]=%d\n", stepIndex, session->lastSubmenuReactivateHandled);
    printf("submenuBehaviorClass[%u]=%u\n", stepIndex, session->lastSubmenuBehaviorClass);
    printf("submenuCumulativeBehaviorClass[%u]=%u\n", stepIndex, session->lastSubmenuCumulativeBehaviorClass);
    printf("submenuBehaviorMask[%u]=%u\n", stepIndex, session->lastSubmenuBehaviorMask);
    printf("submenuFrameEventCount[%u]=%u\n", stepIndex, session->lastSubmenuFrameEventCount);
    printf("submenuIdleEventCount[%u]=%u\n", stepIndex, session->lastSubmenuIdleEventCount);
    printf("submenuAdvanceSuppressedCount[%u]=%u\n", stepIndex, session->lastSubmenuAdvanceSuppressedCount);
    return 1;
}
static int publish_repeated_graphic_with_runtime(
    const char* graphicsDatPath,
    const char* outputPrefix,
    unsigned int dialogGraphicIndex,
    unsigned int graphicIndex,
    unsigned int repeatCount,
    unsigned int* ioFrameNumber,
    struct BetaHarnessPersistentRuntime_Compat* runtime) {
    unsigned int i;
    char outputPath[1024];

    for (i = 0; i < repeatCount; ++i) {
        snprintf(outputPath, sizeof(outputPath), "%s_hold_%04u.pgm", outputPrefix, *ioFrameNumber);
        if (!render_graphic_with_runtime(
                graphicsDatPath,
                outputPath,
                dialogGraphicIndex,
                graphicIndex,
                *ioFrameNumber,
                runtime)) {
            return 0;
        }
        (*ioFrameNumber)++;
    }
    return 1;
}

static int replay_boot_tail_with_runtime(
    const char* graphicsDatPath,
    const char* outputPrefix,
    unsigned int dialogGraphicIndex,
    const struct BootPlanScript_Compat* bootScript,
    unsigned int tailStepCount,
    unsigned int* ioFrameNumber,
    struct BetaHarnessPersistentRuntime_Compat* runtime) {
    unsigned int startIndex;
    unsigned int i;
    char outputPath[1024];

    if ((bootScript == 0) || (bootScript->stepCount == 0)) {
        return 0;
    }
    if (tailStepCount > bootScript->stepCount) {
        tailStepCount = bootScript->stepCount;
    }
    startIndex = bootScript->stepCount - tailStepCount;
    for (i = startIndex; i < bootScript->stepCount; ++i) {
        snprintf(outputPath, sizeof(outputPath), "%s_tail_%04u.pgm", outputPrefix, *ioFrameNumber);
        if (!render_graphic_with_runtime(
                graphicsDatPath,
                outputPath,
                dialogGraphicIndex,
                bootScript->steps[i].viewportGraphicIndex,
                *ioFrameNumber,
                runtime)) {
            return 0;
        }
        (*ioFrameNumber)++;
    }
    return 1;
}

static int process_events_persistently(
    const char* graphicsDatPath,
    const char* outputPrefix,
    unsigned int dialogGraphicIndex,
    unsigned int viewportGraphicIndex,
    unsigned int selectionCount,
    unsigned int highlightBaseGraphicIndex,
    unsigned int activateGraphicBaseIndex,
    struct MemoryGraphicsDatHeader_Compat* header,
    const enum MemoryGraphicsDatEvent_Compat* events,
    unsigned int startIndex,
    unsigned int endIndex,
    unsigned int* ioFrameNumber,
    struct BetaHarnessSessionState_Compat* session,
    struct BetaHarnessPersistentRuntime_Compat* runtime) {
    unsigned int i;

    for (i = startIndex; i < endIndex; ++i) {
        struct MemoryGraphicsDatSubmenuConsequenceResult_Compat stepResult;
        int screenChanged;

        memset(&stepResult, 0, sizeof(stepResult));
        if (!run_single_event(
                graphicsDatPath,
                dialogGraphicIndex,
                viewportGraphicIndex,
                selectionCount,
                highlightBaseGraphicIndex,
                activateGraphicBaseIndex,
                header,
                session,
                events[i],
                &stepResult)) {
            return 0;
        }
        screenChanged = stepResult.consequence.screenChanged;
        apply_single_event_result(session, &stepResult);
        if (!publish_session_step(
                graphicsDatPath,
                outputPrefix,
                dialogGraphicIndex,
                *ioFrameNumber,
                i,
                events[i],
                session,
                screenChanged,
                runtime)) {
            F0479_MEMORY_FreeSubmenuConsequenceMini_Compat(&stepResult);
            return 0;
        }
        F0479_MEMORY_FreeSubmenuConsequenceMini_Compat(&stepResult);
        (*ioFrameNumber)++;
    }
    return 1;
}

int main(int argc, char** argv) {
    const char* graphicsDatPath;
    const char* outputPrefix;
    char inputBuffer[512];
    const char* eventSpec = 0;
    const char* trustedSeamEventSpec = 0;
    const char* replayTailEventSpec = 0;
    const char* replayTailSessionEventSpec = 0;
    const char* unifiedBootEventSpec = 0;
    unsigned int unifiedBootStepLimit = 0;
    int titleHoldMode = 0;
    unsigned int titleHoldRepeatCount = 0;
    const char* scriptName = "m7_reachability_b";
    unsigned int dialogGraphicIndex = 1;
    unsigned int viewportGraphicIndex = 0;
    unsigned int replayTailStepCount = 0;
    unsigned int initialSelectionIndex = 0;
    unsigned int selectionCount = 3;
    unsigned int highlightBaseGraphicIndex = 1;
    unsigned int activateGraphicBaseIndex = 11;
    unsigned int firstFrameNumber = 1;
    unsigned int backdropStepCount = 24;
    unsigned int titleStepCount = 16;
    unsigned int menuStepCount = 16;
    unsigned int holdStepCount = 8;
    unsigned int holdCycleSize = 8;
    unsigned int eventCount = 0;
    unsigned int frameNumber;
    unsigned int bootFrameCount;
    unsigned int sessionMode = 0;
    enum MemoryGraphicsDatEvent_Compat events[256];
    const struct BootPlanScript_Compat* bootScript;
    struct BootPlanReachabilityResult_Compat bootResult;
    struct MemoryGraphicsDatState_Compat fileState;
    struct MemoryGraphicsDatHeader_Compat header;
    struct BetaHarnessSessionState_Compat session;
    struct BetaHarnessPersistentRuntime_Compat runtime;

    if (argc < 3) {
        fprintf(stderr, "usage: %s /path/to/GRAPHICS.DAT /path/to/output_prefix [event_spec|--session|--trusted-seam event_spec|--replay-tail step_count event_spec|--replay-tail-session step_count event_spec|--unified-boot event_spec|--unified-boot-stop step_count|--title-hold [repeat_count]]\n", argv[0]);
        fprintf(stderr, "event chars: f=frame n=advance a=activate i=idle\n");
        return 2;
    }
    graphicsDatPath = argv[1];
    outputPrefix = argv[2];
    if (argc >= 4) {
        if (strcmp(argv[3], "--session") == 0) {
            sessionMode = 1;
        } else if ((strcmp(argv[3], "--trusted-seam") == 0) && (argc >= 5)) {
            trustedSeamEventSpec = argv[4];
        } else if ((strcmp(argv[3], "--replay-tail") == 0) && (argc >= 6)) {
            replayTailStepCount = (unsigned int)strtoul(argv[4], 0, 10);
            replayTailEventSpec = argv[5];
        } else if ((strcmp(argv[3], "--replay-tail-session") == 0) && (argc >= 6)) {
            replayTailStepCount = (unsigned int)strtoul(argv[4], 0, 10);
            replayTailSessionEventSpec = argv[5];
        } else if ((strcmp(argv[3], "--unified-boot") == 0) && (argc >= 5)) {
            unifiedBootEventSpec = argv[4];
        } else if ((strcmp(argv[3], "--unified-boot-stop") == 0) && (argc >= 5)) {
            unifiedBootStepLimit = (unsigned int)strtoul(argv[4], 0, 10);
            unifiedBootEventSpec = "";
        } else if (strcmp(argv[3], "--title-hold") == 0) {
            titleHoldMode = 1;
            unifiedBootStepLimit = 50;
            unifiedBootEventSpec = "";
            if (argc >= 5) {
                titleHoldRepeatCount = (unsigned int)strtoul(argv[4], 0, 10);
            }
        } else {
            eventSpec = argv[3];
        }
    } else {
        printf("enter event sequence (f=frame, n=advance, a=activate, i=idle), example: fnnaf\n> ");
        fflush(stdout);
        if (fgets(inputBuffer, sizeof(inputBuffer), stdin) == 0) {
            fprintf(stderr, "failed: no event sequence provided\n");
            return 1;
        }
        eventSpec = inputBuffer;
    }

    bootScript = F9010_RUNTIME_GetBootPlanScript_Compat(scriptName);
    if (bootScript == 0) {
        fprintf(stderr, "failed: could not resolve boot script '%s'\n", scriptName);
        return 1;
    }

    memset(&bootResult, 0, sizeof(bootResult));
    if (unifiedBootEventSpec == 0) {
        if (!F9012_RUNTIME_RunStatefulBootPlanReachabilityScript_Compat(
                graphicsDatPath,
                outputPrefix,
                dialogGraphicIndex,
                firstFrameNumber,
                scriptName,
                backdropStepCount,
                titleStepCount,
                menuStepCount,
                holdStepCount,
                holdCycleSize,
                &bootResult)) {
            fprintf(stderr, "failed: boot-to-held-menu run did not complete\n");
            return 1;
        }
    }

    memset(&fileState, 0, sizeof(fileState));
    memset(&header, 0, sizeof(header));
    memset(&runtime, 0, sizeof(runtime));
    if (!F0479_MEMORY_LoadGraphicsDatHeader_Compat(graphicsDatPath, &fileState, &header)) {
        fprintf(stderr, "failed: could not load GRAPHICS.DAT header from %s\n", graphicsDatPath);
        return 1;
    }
    if (!initialize_persistent_runtime(
            graphicsDatPath,
            &header,
            dialogGraphicIndex,
            viewportGraphicIndex,
            &runtime)) {
        F0479_MEMORY_FreeGraphicsDatHeader_Compat(&header);
        fprintf(stderr, "failed: could not initialize persistent runtime\n");
        return 1;
    }

    if (unifiedBootEventSpec != 0) {
        bootFrameCount = 0;
        unsigned int bootReplayCount = (unifiedBootStepLimit > 0 && unifiedBootStepLimit < bootScript->stepCount) ? unifiedBootStepLimit : bootScript->stepCount;
        if (!replay_boot_tail_with_runtime(
                graphicsDatPath,
                outputPrefix,
                dialogGraphicIndex,
                bootScript,
                bootReplayCount,
                &firstFrameNumber,
                &runtime)) {
            free_persistent_runtime(&runtime);
            F0479_MEMORY_FreeGraphicsDatHeader_Compat(&header);
            fprintf(stderr, "failed: unified boot publish did not complete\n");
            return 1;
        }
        bootFrameCount = (unifiedBootStepLimit > 0 && unifiedBootStepLimit < bootScript->stepCount) ? unifiedBootStepLimit : bootScript->stepCount;
        bootResult.reachedMenuEstablished = (bootFrameCount >= (backdropStepCount + titleStepCount + menuStepCount));
        bootResult.reachedMenuHeld = (bootFrameCount >= (backdropStepCount + titleStepCount + menuStepCount + holdStepCount));
        frameNumber = firstFrameNumber;
    } else {
        bootFrameCount = bootResult.run.publishedFrameCount;
        if (!bootResult.reachedMenuHeld) {
            free_persistent_runtime(&runtime);
            F0479_MEMORY_FreeGraphicsDatHeader_Compat(&header);
            fprintf(stderr, "failed: boot run did not reach held menu\n");
            return 1;
        }
        frameNumber = firstFrameNumber + bootFrameCount;
    }
    printf("ok\n");
    printf("graphicsDatPath=%s\n", graphicsDatPath);
    printf("outputPrefix=%s\n", outputPrefix);
    printf("bootScript=%s\n", scriptName);
    printf("bootPublishedFrameCount=%u\n", bootFrameCount);
    printf("reachedMenuHeld=%d\n", bootResult.reachedMenuHeld);
    printf("sessionMode=%u\n", sessionMode);
    printf("unifiedBootMode=%d\n", unifiedBootEventSpec != 0);
    printf("titleHoldMode=%d\n", titleHoldMode);
    if ((trustedSeamEventSpec != 0) || (replayTailEventSpec != 0)) {
        struct MemoryGraphicsDatMenuActivateConsequenceResult_Compat seamResult;
        const char* seamEventSpec = (replayTailEventSpec != 0) ? replayTailEventSpec : trustedSeamEventSpec;
        unsigned int trustedEventCount = 0;

        if (!parse_events(seamEventSpec, events, 256U, &trustedEventCount)) {
            free_persistent_runtime(&runtime);
            F0479_MEMORY_FreeGraphicsDatHeader_Compat(&header);
            fprintf(stderr, "failed: could not parse trusted seam event sequence '%s'\n", seamEventSpec);
            return 1;
        }
        if (replayTailEventSpec != 0) {
            if (!replay_boot_tail_with_runtime(
                    graphicsDatPath,
                    outputPrefix,
                    dialogGraphicIndex,
                    bootScript,
                    replayTailStepCount,
                    &frameNumber,
                    &runtime)) {
                free_persistent_runtime(&runtime);
                F0479_MEMORY_FreeGraphicsDatHeader_Compat(&header);
                fprintf(stderr, "failed: runtime tail replay did not complete\n");
                return 1;
            }
            printf("replayTailStepCount=%u\n", replayTailStepCount);
        }

        memset(&seamResult, 0, sizeof(seamResult));
        if (!run_prefix(
                graphicsDatPath,
                events,
                trustedEventCount,
                dialogGraphicIndex,
                viewportGraphicIndex,
                initialSelectionIndex,
                selectionCount,
                highlightBaseGraphicIndex,
                activateGraphicBaseIndex,
                MEMORY_GRAPHICS_DAT_MENU_SCREEN_MENU,
                &header,
                &seamResult)) {
            free_persistent_runtime(&runtime);
            F0479_MEMORY_FreeGraphicsDatHeader_Compat(&header);
            fprintf(stderr, "failed: trusted seam run did not complete\n");
            return 1;
        }
        printf("trustedSeamEventCount=%u\n", trustedEventCount);
        printf("trustedSeamFinalSelectionIndex=%u\n", seamResult.activate.render.menuState.finalSelectionIndex);
        printf("trustedSeamSelectedRenderVariant=%u\n", seamResult.activate.render.selectedRenderVariant);
        printf("trustedSeamActivationTriggered=%d\n", seamResult.activate.activationTriggered);
        printf("trustedSeamActivatedSelectionIndex=%u\n", seamResult.activate.activatedSelectionIndex);
        printf("trustedSeamActivatedGraphicIndex=%u\n", seamResult.activate.activatedGraphicIndex);
        printf("trustedSeamFinalScreen=%u\n", (unsigned int)seamResult.finalScreen);
        printf("trustedSeamScreenChanged=%d\n", seamResult.screenChanged);
        F0479_MEMORY_FreeMenuActivateConsequenceMini_Compat(&seamResult);
        free_persistent_runtime(&runtime);
        F0479_MEMORY_FreeGraphicsDatHeader_Compat(&header);
        return 0;
    }

    memset(&session, 0, sizeof(session));
    session.selectionIndex = initialSelectionIndex;
    session.selectedRenderVariant = initialSelectionIndex % selectionCount;
    session.screen = MEMORY_GRAPHICS_DAT_MENU_SCREEN_MENU;
    session.renderGraphicIndex = highlightBaseGraphicIndex + (initialSelectionIndex % selectionCount);
    if (unifiedBootEventSpec != 0) {
        if (unifiedBootEventSpec[0] == '\0') {
            if (titleHoldMode && (titleHoldRepeatCount > 0)) {
                if (!publish_repeated_graphic_with_runtime(
                        graphicsDatPath,
                        outputPrefix,
                        dialogGraphicIndex,
                        313,
                        titleHoldRepeatCount,
                        &frameNumber,
                        &runtime)) {
                    free_persistent_runtime(&runtime);
                    F0479_MEMORY_FreeGraphicsDatHeader_Compat(&header);
                    fprintf(stderr, "failed: title hold repeat did not complete\n");
                    return 1;
                }
                printf("titleHoldRepeatCount=%u\n", titleHoldRepeatCount);
            }
            free_persistent_runtime(&runtime);
            F0479_MEMORY_FreeGraphicsDatHeader_Compat(&header);
            return 0;
        }
        if (!parse_events(unifiedBootEventSpec, events, 256U, &eventCount)) {
            free_persistent_runtime(&runtime);
            F0479_MEMORY_FreeGraphicsDatHeader_Compat(&header);
            fprintf(stderr, "failed: could not parse unified boot event sequence '%s'\n", unifiedBootEventSpec);
            return 1;
        }
        printf("eventCount=%u\n", eventCount);
        if (!process_events_persistently(
                graphicsDatPath,
                outputPrefix,
                dialogGraphicIndex,
                viewportGraphicIndex,
                selectionCount,
                highlightBaseGraphicIndex,
                activateGraphicBaseIndex,
                &header,
                events,
                0,
                eventCount,
                &frameNumber,
                &session,
                &runtime)) {
            free_persistent_runtime(&runtime);
            F0479_MEMORY_FreeGraphicsDatHeader_Compat(&header);
            fprintf(stderr, "failed: unified boot event sequence did not complete\n");
            return 1;
        }
        print_final_result(&session);
        free_persistent_runtime(&runtime);
        F0479_MEMORY_FreeGraphicsDatHeader_Compat(&header);
        return 0;
    }
    if (replayTailSessionEventSpec != 0) {
        if (!parse_events(replayTailSessionEventSpec, events, 256U, &eventCount)) {
            free_persistent_runtime(&runtime);
            F0479_MEMORY_FreeGraphicsDatHeader_Compat(&header);
            fprintf(stderr, "failed: could not parse replay tail session event sequence '%s'\n", replayTailSessionEventSpec);
            return 1;
        }
        if (!replay_boot_tail_with_runtime(
                graphicsDatPath,
                outputPrefix,
                dialogGraphicIndex,
                bootScript,
                replayTailStepCount,
                &frameNumber,
                &runtime)) {
            free_persistent_runtime(&runtime);
            F0479_MEMORY_FreeGraphicsDatHeader_Compat(&header);
            fprintf(stderr, "failed: runtime tail replay did not complete\n");
            return 1;
        }
        printf("replayTailStepCount=%u\n", replayTailStepCount);
        printf("eventCount=%u\n", eventCount);
        if (!process_events_persistently(
                graphicsDatPath,
                outputPrefix,
                dialogGraphicIndex,
                viewportGraphicIndex,
                selectionCount,
                highlightBaseGraphicIndex,
                activateGraphicBaseIndex,
                &header,
                events,
                0,
                eventCount,
                &frameNumber,
                &session,
                &runtime)) {
            free_persistent_runtime(&runtime);
            F0479_MEMORY_FreeGraphicsDatHeader_Compat(&header);
            fprintf(stderr, "failed: replay tail session event sequence did not complete\n");
            return 1;
        }
        print_final_result(&session);
        free_persistent_runtime(&runtime);
        F0479_MEMORY_FreeGraphicsDatHeader_Compat(&header);
        return 0;
    }
    if (sessionMode) {
        printf("sessionPrompt=f=frame n=advance a=activate i=idle, one chunk per line, EOF to finish\n");
        while (fgets(inputBuffer, sizeof(inputBuffer), stdin) != 0) {
            unsigned int chunkCount = 0;
            unsigned int startIndex = eventCount;

            if (!parse_events(inputBuffer, events + eventCount, 256U - eventCount, &chunkCount)) {
                printf("ignoredInput=%s", inputBuffer);
                continue;
            }
            eventCount += chunkCount;
            if (!process_events_persistently(
                    graphicsDatPath,
                    outputPrefix,
                    dialogGraphicIndex,
                    viewportGraphicIndex,
                    selectionCount,
                    highlightBaseGraphicIndex,
                    activateGraphicBaseIndex,
                    &header,
                    events,
                    startIndex,
                    eventCount,
                    &frameNumber,
                    &session,
                    &runtime)) {
                free_persistent_runtime(&runtime);
                F0479_MEMORY_FreeGraphicsDatHeader_Compat(&header);
                fprintf(stderr, "failed: session chunk did not complete\n");
                return 1;
            }
            printf("chunkEventCount=%u\n", chunkCount);
            print_final_result(&session);
        }
    } else {
        if (!parse_events(eventSpec, events, 256U, &eventCount)) {
            F0479_MEMORY_FreeGraphicsDatHeader_Compat(&header);
            fprintf(stderr, "failed: could not parse event sequence '%s'\n", eventSpec);
            return 1;
        }
        printf("eventCount=%u\n", eventCount);
        if (!process_events_persistently(
                graphicsDatPath,
                outputPrefix,
                dialogGraphicIndex,
                viewportGraphicIndex,
                selectionCount,
                highlightBaseGraphicIndex,
                activateGraphicBaseIndex,
                &header,
                events,
                0,
                eventCount,
                &frameNumber,
                &session,
                &runtime)) {
            free_persistent_runtime(&runtime);
            F0479_MEMORY_FreeGraphicsDatHeader_Compat(&header);
            fprintf(stderr, "failed: event sequence did not complete\n");
            return 1;
        }
        print_final_result(&session);
    }

    free_persistent_runtime(&runtime);
    F0479_MEMORY_FreeGraphicsDatHeader_Compat(&header);
    return 0;
}

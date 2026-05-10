#include "touch_pointer_input_pc34_compat.h"
#include <string.h>

/* Provider-neutral pointer/touch seam for DM1 V1 input.
 *
 * ReDMCSB source lock:
 * - COMMAND.C:1379-1449 F0358_COMMAND_GetCommandFromMouseInput_CPSC scans
 *   source MOUSE_INPUT records with the current X/Y/button mask.
 * - COMMAND.C:1452-1644 F0359_COMMAND_ProcessClick_CPSC queues mouse commands
 *   after primary mouse input fails over to secondary mouse input; screen
 *   taps use this same source-ordered primary-then-secondary lookup.
 * - COMMAND.C:396-405 is the movement/viewport/right-button mouse table.
 * - COMMAND.C:2296-2324 dispatches C083 inventory toggle, C111 action parent,
 *   and C080 dungeon-view click after the queue returns those command IDs.
 * - STARTUP2.C:1179-1182 installs primary interface + secondary movement
 *   mouse/keyboard tables; this shim deliberately emits click coordinates and
 *   button masks, not synthesized keys.
 * - CLIKMENU.C:519-585 resolves action-menu child clicks after C111 dispatch,
 *   so touch/click only feeds the original command IDs and does not touch
 *   dungeon movement/collision timing.
 */

static void clear_dispatch(TouchPointerDispatchPc34Compat* outDispatch) {
    if (outDispatch) memset(outDispatch, 0, sizeof(*outDispatch));
}

static void copy_zone_to_dispatch(int x,
                                  int y,
                                  unsigned int buttonMask,
                                  const TouchClickZonePc34Compat* zone,
                                  TouchPointerDispatchPc34Compat* outDispatch) {
    outDispatch->shouldDispatchClick = 1;
    outDispatch->screenX = x;
    outDispatch->screenY = y;
    outDispatch->buttonStatus = buttonMask;
    outDispatch->commandId = zone->commandId;
    outDispatch->zoneIndex = zone->zoneIndex;
    outDispatch->coordMode = zone->coordMode;
    outDispatch->groupName = zone->groupName;
}

int TOUCHPOINTER_Compat_EventFromScaledTap(int physicalX,
                                           int physicalY,
                                           int surfaceW,
                                           int surfaceH,
                                           unsigned int buttonMask,
                                           TouchPointerEventPc34Compat* outEvent) {
    if (!outEvent || buttonMask == 0u || surfaceW <= 0 || surfaceH <= 0) return 0;
    outEvent->action = TOUCH_POINTER_ACTION_CLICK_PC34_COMPAT;
    outEvent->space = TOUCH_POINTER_SPACE_SCALED_SCREEN_PC34_COMPAT;
    outEvent->x = physicalX;
    outEvent->y = physicalY;
    outEvent->surfaceW = surfaceW;
    outEvent->surfaceH = surfaceH;
    outEvent->buttonMask = buttonMask;
    return 1;
}

int TOUCHPOINTER_Compat_TranslateEvent(const TouchPointerEventPc34Compat* event,
                                       TouchPointerDispatchPc34Compat* outDispatch) {
    TouchClickZonePc34Compat zone;
    int screenX;
    int screenY;
    if (!event || !outDispatch) return 0;
    clear_dispatch(outDispatch);
    if (event->buttonMask == 0u) return 0;
    if (event->action != TOUCH_POINTER_ACTION_DOWN_PC34_COMPAT &&
        event->action != TOUCH_POINTER_ACTION_CLICK_PC34_COMPAT) {
        return 0;
    }

    switch (event->space) {
    case TOUCH_POINTER_SPACE_SCREEN_320X200_PC34_COMPAT:
        if (!TOUCHCLICK_Compat_HitTestPrimaryThenSecondary(event->x, event->y, event->buttonMask, &zone)) return 0;
        copy_zone_to_dispatch(event->x, event->y, event->buttonMask, &zone, outDispatch);
        return 1;

    case TOUCH_POINTER_SPACE_SCALED_SCREEN_PC34_COMPAT:
        if (!TOUCHCLICK_Compat_NormalizeScaledScreenPoint(event->x, event->y,
                                                          event->surfaceW, event->surfaceH,
                                                          &screenX, &screenY)) return 0;
        if (!TOUCHCLICK_Compat_HitTestPrimaryThenSecondary(screenX, screenY, event->buttonMask, &zone)) return 0;
        copy_zone_to_dispatch(screenX, screenY, event->buttonMask, &zone, outDispatch);
        return 1;

    case TOUCH_POINTER_SPACE_VIEWPORT_LOCAL_PC34_COMPAT:
    case TOUCH_POINTER_SPACE_SCALED_VIEWPORT_PC34_COMPAT: {
        TouchClickDispatchPc34Compat clickDispatch;
        int mapped;
        if (event->space == TOUCH_POINTER_SPACE_VIEWPORT_LOCAL_PC34_COMPAT) {
            mapped = TOUCHCLICK_Compat_MapViewportLocalPointToDispatch(event->x, event->y,
                                                                       event->buttonMask,
                                                                       &clickDispatch);
        } else {
            mapped = TOUCHCLICK_Compat_MapScaledViewportPointToDispatch(event->x, event->y,
                                                                        event->surfaceW, event->surfaceH,
                                                                        event->buttonMask,
                                                                        &clickDispatch);
        }
        if (!mapped) return 0;
        outDispatch->shouldDispatchClick = 1;
        outDispatch->screenX = clickDispatch.screenX;
        outDispatch->screenY = clickDispatch.screenY;
        outDispatch->buttonStatus = clickDispatch.buttonStatus;
        outDispatch->commandId = clickDispatch.commandId;
        outDispatch->zoneIndex = clickDispatch.zoneIndex;
        outDispatch->coordMode = clickDispatch.coordMode;
        outDispatch->groupName = clickDispatch.groupName;
        return 1;
    }

    default:
        return 0;
    }
}


int TOUCHPOINTER_Compat_EnqueueEventToInputCommandQueue(
    const TouchPointerEventPc34Compat* event,
    struct Dm1V1InputCommandQueuePc34Compat* queue,
    TouchPointerDispatchPc34Compat* outDispatch) {
    TouchPointerDispatchPc34Compat localDispatch;
    TouchPointerDispatchPc34Compat* dispatch = outDispatch ? outDispatch : &localDispatch;
    if (!queue) return 0;
    if (!TOUCHPOINTER_Compat_TranslateEvent(event, dispatch)) return 0;
    return DM1_V1_InputCommandQueue_EnqueueMouseCommandPc34Compat(
        queue,
        (int)dispatch->commandId,
        dispatch->screenX,
        dispatch->screenY,
        (int)dispatch->buttonStatus);
}

const char* TOUCHPOINTER_Compat_GetSourceEvidence(void) {
    return "ReDMCSB COMMAND.C:1379-1449 source mouse hit-test, 1452-1644 click queue primary-to-secondary search, 396-405 movement/viewport/right-button mouse table, 2296-2324 C083/C111/C080 dispatch; STARTUP2.C:1179-1182 installs primary interface and secondary movement mouse tables; COORD.C:1693-1722 source viewport origin/extent and 1915-1920 inclusive point-in-zone bounds; viewport-local and scaled-viewport touch events are promoted to original screen coordinates before queueing; touch bridge enqueues resolved mouse commands through DM1_V1_InputCommandQueue without changing keyboard routes; CLIKMENU.C:519-585 action-area child-click resolution unchanged";
}

#include "touch_pointer_input_pc34_compat.h"
#include <string.h>

/* Provider-neutral pointer/touch seam for DM1 V1 input.
 *
 * ReDMCSB source lock:
 * - COMMAND.C:2831-2915 F0359_COMMAND_ProcessClick_CPSC queues mouse commands
 *   after matching MOUSE_INPUT records against current coordinates/buttons.
 * - COMMAND.C:2922-2928 F0360_COMMAND_ProcessPendingClick replays pending clicks
 *   through the same click function.
 * - COMMAND.C:396-405 is the movement/viewport/right-button mouse table.
 * - COMMAND.C:1709-1765 F0361_COMMAND_ProcessKeyPress is the separate keyboard
 *   queue path; this shim deliberately emits click coordinates/buttons, not
 *   synthesized keys.
 * - CLIKMENU.C:142-174 and 180-347 execute turn/move commands after dispatch,
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
        if (!TOUCHCLICK_Compat_HitTestWithButton(event->x, event->y, event->buttonMask, &zone)) return 0;
        copy_zone_to_dispatch(event->x, event->y, event->buttonMask, &zone, outDispatch);
        return 1;

    case TOUCH_POINTER_SPACE_SCALED_SCREEN_PC34_COMPAT:
        if (!TOUCHCLICK_Compat_NormalizeScaledScreenPoint(event->x, event->y,
                                                          event->surfaceW, event->surfaceH,
                                                          &screenX, &screenY)) return 0;
        if (!TOUCHCLICK_Compat_HitTestWithButton(screenX, screenY, event->buttonMask, &zone)) return 0;
        copy_zone_to_dispatch(screenX, screenY, event->buttonMask, &zone, outDispatch);
        return 1;

    case TOUCH_POINTER_SPACE_VIEWPORT_LOCAL_PC34_COMPAT:
        if (!TOUCHCLICK_Compat_HitTestInCoordMode(event->x, event->y,
                                                  TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT,
                                                  event->buttonMask, &zone)) return 0;
        copy_zone_to_dispatch(event->x, event->y, event->buttonMask, &zone, outDispatch);
        return 1;

    default:
        return 0;
    }
}

const char* TOUCHPOINTER_Compat_GetSourceEvidence(void) {
    return "ReDMCSB COMMAND.C:2831-2915 click queue, 2922-2928 pending click, 396-405 movement/viewport mouse inputs, 1709-1765 keyboard-separate; CLIKMENU.C:142-174/180-347 movement execution unchanged";
}

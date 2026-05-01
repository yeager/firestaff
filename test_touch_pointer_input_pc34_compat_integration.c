#include <stdio.h>
#include <string.h>
#include "touch_pointer_input_pc34_compat.h"

static int expect_dispatch(const TouchPointerDispatchPc34Compat* d,
                           int x,
                           int y,
                           unsigned int buttonMask,
                           unsigned int commandId,
                           unsigned int zoneIndex,
                           TouchClickCoordModePc34Compat coordMode,
                           const char* groupName) {
    return d->shouldDispatchClick == 1 &&
           d->screenX == x && d->screenY == y &&
           d->buttonStatus == buttonMask &&
           d->commandId == commandId && d->zoneIndex == zoneIndex &&
           d->coordMode == coordMode &&
           strcmp(d->groupName, groupName) == 0;
}

int main(void) {
    TouchPointerEventPc34Compat event;
    TouchPointerDispatchPc34Compat dispatch;
    struct Dm1V1InputCommandQueuePc34Compat queue;
    struct Dm1V1QueuedCommandPc34Compat queued;
    struct Dm1V1InputQueueProcessResultPc34Compat processResult;
    int ok = 1;

    printf("probe=firestaff_touch_pointer_input\n");
    printf("sourceEvidence=%s\n", TOUCHPOINTER_Compat_GetSourceEvidence());

    event.action = TOUCH_POINTER_ACTION_DOWN_PC34_COMPAT;
    event.space = TOUCH_POINTER_SPACE_SCREEN_320X200_PC34_COMPAT;
    event.x = 264;
    event.y = 126;
    event.surfaceW = 0;
    event.surfaceH = 0;
    event.buttonMask = TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT;
    if (!TOUCHPOINTER_Compat_TranslateEvent(&event, &dispatch) ||
        !expect_dispatch(&dispatch, 264, 126, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,
                         3u, 70u, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT,
                         "movement.forward")) ok = 0;

    event.x = 25;
    event.y = 11;
    event.buttonMask = TOUCH_CLICK_BUTTON_RIGHT_PC34_COMPAT;
    if (!TOUCHPOINTER_Compat_TranslateEvent(&event, &dispatch) ||
        !expect_dispatch(&dispatch, 25, 11, TOUCH_CLICK_BUTTON_RIGHT_PC34_COMPAT,
                         7u, 151u, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT,
                         "champion0.toggle_box")) ok = 0;

    if (!TOUCHPOINTER_Compat_EventFromScaledTap(1056, 450, 1280, 720,
                                                TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,
                                                &event)) ok = 0;
    if (!TOUCHPOINTER_Compat_TranslateEvent(&event, &dispatch) ||
        !expect_dispatch(&dispatch, 275, 125, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,
                         3u, 70u, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT,
                         "movement.forward")) ok = 0;

    event.action = TOUCH_POINTER_ACTION_CLICK_PC34_COMPAT;
    event.space = TOUCH_POINTER_SPACE_VIEWPORT_LOCAL_PC34_COMPAT;
    event.x = 12;
    event.y = 13;
    event.surfaceW = 0;
    event.surfaceH = 0;
    event.buttonMask = TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT;
    if (!TOUCHPOINTER_Compat_TranslateEvent(&event, &dispatch) ||
        !expect_dispatch(&dispatch, 12, 46, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,
                         71u, 546u, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT,
                         "inventory.eye")) ok = 0;

    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    event.action = TOUCH_POINTER_ACTION_CLICK_PC34_COMPAT;
    event.space = TOUCH_POINTER_SPACE_SCALED_SCREEN_PC34_COMPAT;
    event.x = 1056;
    event.y = 450;
    event.surfaceW = 1280;
    event.surfaceH = 720;
    event.buttonMask = TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT;
    if (!TOUCHPOINTER_Compat_EnqueueEventToInputCommandQueue(&event, &queue, &dispatch)) ok = 0;
    if (queue.count != 1u || !DM1_V1_InputCommandQueue_PeekPc34Compat(&queue, &queued) ||
        queued.command != DM1_V1_COMMAND_MOVE_FORWARD || queued.x != 275 || queued.y != 125) ok = 0;

    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    event.action = TOUCH_POINTER_ACTION_CLICK_PC34_COMPAT;
    event.space = TOUCH_POINTER_SPACE_VIEWPORT_LOCAL_PC34_COMPAT;
    event.x = 12;
    event.y = 13;
    event.surfaceW = 0;
    event.surfaceH = 0;
    event.buttonMask = TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT;
    if (!TOUCHPOINTER_Compat_EnqueueEventToInputCommandQueue(&event, &queue, &dispatch)) ok = 0;
    if (queue.count != 1u || !DM1_V1_InputCommandQueue_PeekPc34Compat(&queue, &queued) ||
        queued.command != 71 || queued.x != 12 || queued.y != 46) ok = 0;

    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    queue.locked = 1;
    if (TOUCHPOINTER_Compat_EnqueueEventToInputCommandQueue(&event, &queue, &dispatch)) ok = 0;
    if (!queue.pendingClickPresent || queue.pendingClickCommand != 71) ok = 0;
    queue.locked = 0;
    processResult = DM1_V1_InputCommandQueue_ProcessOnePc34Compat(&queue, 0, 0, 0, 0);
    if (processResult.pendingReplayCount != 1 || queue.count != 1u ||
        !DM1_V1_InputCommandQueue_PeekPc34Compat(&queue, &queued) || queued.command != 71) ok = 0;

    event.action = TOUCH_POINTER_ACTION_MOVE_PC34_COMPAT;
    if (TOUCHPOINTER_Compat_TranslateEvent(&event, &dispatch) || dispatch.shouldDispatchClick) ok = 0;
    event.action = TOUCH_POINTER_ACTION_UP_PC34_COMPAT;
    if (TOUCHPOINTER_Compat_TranslateEvent(&event, &dispatch) || dispatch.shouldDispatchClick) ok = 0;
    event.action = TOUCH_POINTER_ACTION_CLICK_PC34_COMPAT;
    event.buttonMask = 0u;
    if (TOUCHPOINTER_Compat_TranslateEvent(&event, &dispatch) || dispatch.shouldDispatchClick) ok = 0;

    if (TOUCHPOINTER_Compat_EventFromScaledTap(20, 20, 0, 720,
                                               TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,
                                               &event)) ok = 0;

    printf("touchPointerInputInvariantOk=%u\n", ok ? 1u : 0u);
    return ok ? 0 : 1;
}

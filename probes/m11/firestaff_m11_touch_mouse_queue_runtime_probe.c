#include <stdio.h>
#include <string.h>

#include "touch_pointer_input_pc34_compat.h"

static int expect_queued(const struct Dm1V1InputCommandQueuePc34Compat* queue,
                         int command,
                         int x,
                         int y) {
    struct Dm1V1QueuedCommandPc34Compat queued;
    return queue->count == 1u &&
           DM1_V1_InputCommandQueue_PeekPc34Compat(queue, &queued) &&
           queued.command == command && queued.x == x && queued.y == y;
}

static int expect_pending_mouse_click(const struct Dm1V1InputCommandQueuePc34Compat* queue,
                                      int command,
                                      int x,
                                      int y,
                                      int buttons) {
    return queue->count == 0u &&
           queue->pendingClickPresent == 1 &&
           queue->pendingClickCommand == command &&
           queue->pendingClickX == x &&
           queue->pendingClickY == y &&
           queue->pendingClickButtons == buttons;
}

int main(void) {
    TouchPointerEventPc34Compat event;
    TouchPointerDispatchPc34Compat dispatch;
    struct Dm1V1InputCommandQueuePc34Compat queue;
    struct Dm1V1InputQueueProcessResultPc34Compat result;
    int ok = 1;

    printf("probe=firestaff_touch_mouse_queue_runtime\n");
    printf("sourceEvidence=%s\n", TOUCHPOINTER_Compat_GetSourceEvidence());
    printf("queueEvidence=%s\n", DM1_V1_InputCommandQueue_SourceEvidencePc34Compat());

    event.action = TOUCH_POINTER_ACTION_CLICK_PC34_COMPAT;
    event.space = TOUCH_POINTER_SPACE_SCALED_SCREEN_PC34_COMPAT;
    event.x = 1056;
    event.y = 450;
    event.surfaceW = 1280;
    event.surfaceH = 720;
    event.buttonMask = TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT;

    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    memset(&dispatch, 0, sizeof(dispatch));
    if (!TOUCHPOINTER_Compat_EnqueueEventToInputCommandQueue(&event, &queue, &dispatch)) ok = 0;
    if (!dispatch.shouldDispatchClick || dispatch.commandId != DM1_V1_COMMAND_MOVE_FORWARD ||
        dispatch.zoneIndex != 70u || dispatch.screenX != 275 || dispatch.screenY != 125) ok = 0;
    if (!expect_queued(&queue, DM1_V1_COMMAND_MOVE_FORWARD, 275, 125)) ok = 0;

    result = DM1_V1_InputCommandQueue_ProcessOnePc34Compat(&queue, 0, 0, 0, 0);
    if (!result.dequeued || !result.dispatchedMove || result.dispatchedTurn || queue.count != 0u) ok = 0;

    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    queue.locked = 1;
    memset(&dispatch, 0, sizeof(dispatch));
    if (TOUCHPOINTER_Compat_EnqueueEventToInputCommandQueue(&event, &queue, &dispatch)) ok = 0;
    if (!dispatch.shouldDispatchClick || dispatch.commandId != DM1_V1_COMMAND_MOVE_FORWARD ||
        !expect_pending_mouse_click(&queue, DM1_V1_COMMAND_MOVE_FORWARD, 275, 125,
                                    TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT)) ok = 0;

    queue.locked = 0;
    result = DM1_V1_InputCommandQueue_ProcessOnePc34Compat(&queue, 0, 0, 0, 0);
    if (result.dequeued || result.dispatchedMove || result.dispatchedTurn ||
        result.pendingReplayCount != 1 ||
        !expect_queued(&queue, DM1_V1_COMMAND_MOVE_FORWARD, 275, 125)) ok = 0;

    event.space = TOUCH_POINTER_SPACE_VIEWPORT_LOCAL_PC34_COMPAT;
    event.x = 12;
    event.y = 13;
    event.surfaceW = 0;
    event.surfaceH = 0;
    event.buttonMask = TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT;
    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    queue.locked = 1;
    memset(&dispatch, 0, sizeof(dispatch));
    if (TOUCHPOINTER_Compat_EnqueueEventToInputCommandQueue(&event, &queue, &dispatch)) ok = 0;
    if (!dispatch.shouldDispatchClick || dispatch.commandId != 71u || dispatch.zoneIndex != 546u ||
        !expect_pending_mouse_click(&queue, 71, 12, 46, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT)) ok = 0;

    event.action = TOUCH_POINTER_ACTION_MOVE_PC34_COMPAT;
    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    if (TOUCHPOINTER_Compat_EnqueueEventToInputCommandQueue(&event, &queue, &dispatch) ||
        queue.count != 0u || queue.pendingClickPresent) ok = 0;

    printf("touchMouseQueueRuntimeInvariantOk=%u\n", ok ? 1u : 0u);
    return ok ? 0 : 1;
}

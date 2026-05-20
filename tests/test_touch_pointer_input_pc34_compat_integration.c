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

typedef struct MovementTouchCasePc34Compat {
    const char* name;
    int x;
    int y;
    unsigned int commandId;
    unsigned int zoneIndex;
    const char* groupName;
} MovementTouchCasePc34Compat;

int main(void) {
    TouchPointerEventPc34Compat event;
    TouchPointerDispatchPc34Compat dispatch;
    struct Dm1V1InputCommandQueuePc34Compat queue;
    struct Dm1V1QueuedCommandPc34Compat queued;
    struct Dm1V1InputQueueProcessResultPc34Compat processResult;
    static const MovementTouchCasePc34Compat movementCases[] = {
        { "movement.turn_left",    234, 125, DM1_V1_COMMAND_TURN_LEFT,     68u, "movement.turn_left" },
        { "movement.forward",      263, 125, DM1_V1_COMMAND_MOVE_FORWARD,  70u, "movement.forward" },
        { "movement.turn_right",   291, 125, DM1_V1_COMMAND_TURN_RIGHT,    69u, "movement.turn_right" },
        { "movement.left",         234, 147, DM1_V1_COMMAND_MOVE_LEFT,     73u, "movement.left" },
        { "movement.backward",     263, 147, DM1_V1_COMMAND_MOVE_BACKWARD, 72u, "movement.backward" },
        { "movement.right",        291, 147, DM1_V1_COMMAND_MOVE_RIGHT,    71u, "movement.right" },
    };
    unsigned int i;
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

    for (i = 0; i < sizeof(movementCases) / sizeof(movementCases[0]); ++i) {
        event.action = TOUCH_POINTER_ACTION_DOWN_PC34_COMPAT;
        event.space = TOUCH_POINTER_SPACE_SCREEN_320X200_PC34_COMPAT;
        event.x = movementCases[i].x;
        event.y = movementCases[i].y;
        event.surfaceW = 0;
        event.surfaceH = 0;
        event.buttonMask = TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT;
        if (!TOUCHPOINTER_Compat_TranslateEvent(&event, &dispatch) ||
            !expect_dispatch(&dispatch, movementCases[i].x, movementCases[i].y,
                             TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,
                             movementCases[i].commandId,
                             movementCases[i].zoneIndex,
                             TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT,
                             movementCases[i].groupName)) ok = 0;
        DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
        if (!TOUCHPOINTER_Compat_EnqueueEventToInputCommandQueue(&event, &queue, &dispatch) ||
            queue.count != 1u || !DM1_V1_InputCommandQueue_PeekPc34Compat(&queue, &queued) ||
            queued.command != (int)movementCases[i].commandId ||
            queued.x != movementCases[i].x || queued.y != movementCases[i].y) ok = 0;
    }

    event.x = 25;
    event.y = 11;
    event.buttonMask = TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT;
    if (!TOUCHPOINTER_Compat_TranslateEvent(&event, &dispatch) ||
        !expect_dispatch(&dispatch, 25, 11, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,
                         12u, 151u, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT,
                         "champion0.status_box")) ok = 0;

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

    event.x = 104;
    event.y = 113;
    if (!TOUCHPOINTER_Compat_TranslateEvent(&event, &dispatch) ||
        !expect_dispatch(&dispatch, 104, 146, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,
                         162u, 573u, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT,
                         "panel.cancel")) ok = 0;

    event.space = TOUCH_POINTER_SPACE_DUNGEON_VIEWPORT_LOCAL_PC34_COMPAT;
    event.x = 112;
    event.y = 68;
    if (!TOUCHPOINTER_Compat_TranslateEvent(&event, &dispatch) ||
        !expect_dispatch(&dispatch, 112, 101, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,
                         80u, 7u, TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT,
                         "viewport.dungeon")) ok = 0;

    event.action = TOUCH_POINTER_ACTION_CLICK_PC34_COMPAT;
    event.space = TOUCH_POINTER_SPACE_SCALED_VIEWPORT_PC34_COMPAT;
    event.x = 68;
    event.y = 52;
    event.surfaceW = 448;
    event.surfaceH = 272;
    event.buttonMask = TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT;
    if (!TOUCHPOINTER_Compat_TranslateEvent(&event, &dispatch) ||
        !expect_dispatch(&dispatch, 34, 59, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,
                         30u, 509u, TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT,
                         "inventory.head")) ok = 0;

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
    processResult = DM1_V1_InputCommandQueue_ProcessOnePc34Compat(&queue, 0, 0, 0, 0);
    if (!processResult.dequeued || !processResult.dispatchedMove ||
        processResult.command != DM1_V1_COMMAND_MOVE_FORWARD || queue.count != 0u) ok = 0;

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
    event.action = TOUCH_POINTER_ACTION_CLICK_PC34_COMPAT;
    event.space = TOUCH_POINTER_SPACE_SCALED_VIEWPORT_PC34_COMPAT;
    event.x = 68;
    event.y = 52;
    event.surfaceW = 448;
    event.surfaceH = 272;
    event.buttonMask = TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT;
    if (!TOUCHPOINTER_Compat_EnqueueEventToInputCommandQueue(&event, &queue, &dispatch)) ok = 0;
    if (queue.count != 1u || !DM1_V1_InputCommandQueue_PeekPc34Compat(&queue, &queued) ||
        queued.command != 30 || queued.x != 34 || queued.y != 59) ok = 0;

    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    event.action = TOUCH_POINTER_ACTION_CLICK_PC34_COMPAT;
    event.space = TOUCH_POINTER_SPACE_DUNGEON_VIEWPORT_LOCAL_PC34_COMPAT;
    event.x = 112;
    event.y = 68;
    event.surfaceW = 0;
    event.surfaceH = 0;
    event.buttonMask = TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT;
    if (!TOUCHPOINTER_Compat_EnqueueEventToInputCommandQueue(&event, &queue, &dispatch)) ok = 0;
    if (queue.count != 1u || !DM1_V1_InputCommandQueue_PeekPc34Compat(&queue, &queued) ||
        queued.command != 80 || queued.x != 112 || queued.y != 101) ok = 0;

    event.action = TOUCH_POINTER_ACTION_CLICK_PC34_COMPAT;
    event.space = TOUCH_POINTER_SPACE_VIEWPORT_LOCAL_PC34_COMPAT;
    event.x = 12;
    event.y = 13;
    event.surfaceW = 0;
    event.surfaceH = 0;
    event.buttonMask = TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT;

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

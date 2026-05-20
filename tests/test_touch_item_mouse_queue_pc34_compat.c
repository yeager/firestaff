#include <stdio.h>
#include <string.h>

#include "touch_pointer_input_pc34_compat.h"

/* DM1 V1 touch item interaction source-lock probe.
 *
 * ReDMCSB evidence:
 * - COMMAND.C:412-451 G0449 maps inventory-mode item slots from
 *   viewport-relative mouse coordinates to C028..C057, C070, C071, and C081.
 * - COMMAND.C:498-506 G0456 maps open chest-panel slots to C058..C065.
 * - COMMAND.C:484-496 G0455 maps champion status-box hand/name child routes;
 *   CLIKCHAM.C:24-33 resolves those only after parent C012..C015 dispatch.
 * - COMMAND.C:1379-1449 F0358 performs the original mouse row hit-test.
 * - COMMAND.C:1452-1661 F0359 queues the resolved mouse command/X/Y.
 *
 * This does not add gestures. It proves touch points become the same mouse
 * commands and coordinates that the existing V1 command queue already accepts.
 */

typedef struct TouchItemQueueCasePc34Compat {
    const char* name;
    TouchPointerSpacePc34Compat space;
    int x;
    int y;
    int surfaceW;
    int surfaceH;
    unsigned int commandId;
    unsigned int zoneIndex;
    TouchClickCoordModePc34Compat coordMode;
    const char* groupName;
    int queuedX;
    int queuedY;
} TouchItemQueueCasePc34Compat;

static int expect_queued(const struct Dm1V1InputCommandQueuePc34Compat* queue,
                         int command,
                         int x,
                         int y) {
    struct Dm1V1QueuedCommandPc34Compat queued;
    return queue->count == 1u &&
           DM1_V1_InputCommandQueue_PeekPc34Compat(queue, &queued) &&
           queued.command == command && queued.x == x && queued.y == y;
}

static int expect_dispatch(const TouchPointerDispatchPc34Compat* dispatch,
                           const TouchItemQueueCasePc34Compat* tc) {
    return dispatch->shouldDispatchClick == 1 &&
           dispatch->screenX == tc->queuedX &&
           dispatch->screenY == tc->queuedY &&
           dispatch->buttonStatus == TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT &&
           dispatch->commandId == tc->commandId &&
           dispatch->zoneIndex == tc->zoneIndex &&
           dispatch->coordMode == tc->coordMode &&
           dispatch->groupName && strcmp(dispatch->groupName, tc->groupName) == 0;
}

static TouchPointerEventPc34Compat make_touch_event(const TouchItemQueueCasePc34Compat* tc) {
    TouchPointerEventPc34Compat event;
    event.action = TOUCH_POINTER_ACTION_CLICK_PC34_COMPAT;
    event.space = tc->space;
    event.x = tc->x;
    event.y = tc->y;
    event.surfaceW = tc->surfaceW;
    event.surfaceH = tc->surfaceH;
    event.buttonMask = TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT;
    return event;
}

static int expect_touch_queues_item_mouse_command(const TouchItemQueueCasePc34Compat* tc) {
    TouchPointerEventPc34Compat event = make_touch_event(tc);
    TouchPointerDispatchPc34Compat dispatch;
    struct Dm1V1InputCommandQueuePc34Compat queue;
    int ok = 1;

    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    memset(&dispatch, 0, sizeof(dispatch));
    if (!TOUCHPOINTER_Compat_EnqueueEventToInputCommandQueue(&event, &queue, &dispatch)) {
        ok = 0;
    }
    if (!expect_dispatch(&dispatch, tc)) {
        ok = 0;
    }
    if (!expect_queued(&queue, (int)tc->commandId, tc->queuedX, tc->queuedY)) {
        ok = 0;
    }

    printf("case=%s ok=%u command=%u zone=%u queued=%d,%d group=%s\n",
           tc->name,
           ok ? 1u : 0u,
           tc->commandId,
           tc->zoneIndex,
           tc->queuedX,
           tc->queuedY,
           tc->groupName);
    return ok;
}

static int expect_locked_touch_becomes_pending_inventory_click(void) {
    TouchItemQueueCasePc34Compat tc = {
        "locked inventory action hand touch queues pending C029",
        TOUCH_POINTER_SPACE_VIEWPORT_LOCAL_PC34_COMPAT,
        62, 53, 0, 0,
        29u, 508u,
        TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT,
        "inventory.action_hand",
        62, 86
    };
    TouchPointerEventPc34Compat event = make_touch_event(&tc);
    TouchPointerDispatchPc34Compat dispatch;
    struct Dm1V1InputCommandQueuePc34Compat queue;
    int ok = 1;

    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    queue.locked = 1;
    memset(&dispatch, 0, sizeof(dispatch));
    if (TOUCHPOINTER_Compat_EnqueueEventToInputCommandQueue(&event, &queue, &dispatch)) {
        ok = 0;
    }
    if (!expect_dispatch(&dispatch, &tc)) {
        ok = 0;
    }
    if (queue.count != 0u || !queue.pendingClickPresent ||
        queue.pendingClickCommand != (int)tc.commandId ||
        queue.pendingClickX != tc.queuedX || queue.pendingClickY != tc.queuedY ||
        queue.pendingClickButtons != TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT) {
        ok = 0;
    }
    printf("case=%s ok=%u\n", tc.name, ok ? 1u : 0u);
    return ok;
}

int main(void) {
    static const TouchItemQueueCasePc34Compat cases[] = {
        {
            "status hand touch queues parent C012",
            TOUCH_POINTER_SPACE_SCREEN_320X200_PC34_COMPAT,
            24, 10, 0, 0,
            DM1_V1_COMMAND_CLICK_CHAMPION_STATUS_0, 151u,
            TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT,
            "champion0.status_box",
            24, 10
        },
        {
            "inventory action hand touch queues C029",
            TOUCH_POINTER_SPACE_VIEWPORT_LOCAL_PC34_COMPAT,
            62, 53, 0, 0,
            29u, 508u,
            TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT,
            "inventory.action_hand",
            62, 86
        },
        {
            "backpack touch queues C041",
            TOUCH_POINTER_SPACE_VIEWPORT_LOCAL_PC34_COMPAT,
            66, 33, 0, 0,
            41u, 520u,
            TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT,
            "inventory.backpack_line1_1",
            66, 66
        },
        {
            "chest panel touch queues C058",
            TOUCH_POINTER_SPACE_VIEWPORT_LOCAL_PC34_COMPAT,
            117, 59, 0, 0,
            58u, 537u,
            TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT,
            "inventory.chest_1",
            117, 92
        },
        {
            "scaled viewport inventory action hand touch queues C029",
            TOUCH_POINTER_SPACE_SCALED_VIEWPORT_PC34_COMPAT,
            124, 106, 448, 272,
            29u, 508u,
            TOUCH_CLICK_COORD_VIEWPORT_RELATIVE_PC34_COMPAT,
            "inventory.action_hand",
            62, 86
        },
        {
            "dungeon viewport mode does not reinterpret inventory coordinates",
            TOUCH_POINTER_SPACE_DUNGEON_VIEWPORT_LOCAL_PC34_COMPAT,
            62, 53, 0, 0,
            80u, 7u,
            TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT,
            "viewport.dungeon",
            62, 86
        }
    };
    size_t i;
    int ok = 1;

    for (i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        if (!expect_touch_queues_item_mouse_command(&cases[i])) {
            ok = 0;
        }
    }

    if (!expect_locked_touch_becomes_pending_inventory_click()) {
        ok = 0;
    }

    printf("touchItemMouseQueuePc34CompatOk=%u\n", ok ? 1u : 0u);
    return ok ? 0 : 1;
}

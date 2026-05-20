#include <stdio.h>
#include <string.h>
#include "touch_pointer_input_pc34_compat.h"

/* Source lock for the narrow Touch Support movement/turning slice.
 *
 * ReDMCSB WIP20210206, Toolchains/Common/Source:
 * - COMMAND.C:396-405 maps the six movement arrow mouse zones to C001/C003/
 *   C002/C006/C005/C004 with left-button input only.
 * - COMMAND.C:1379-1449 F0358_COMMAND_GetCommandFromMouseInput_CPSC tests
 *   source mouse zones and button masks without synthesizing keyboard input.
 * - COMMAND.C:2045-2156 F0380_COMMAND_ProcessQueue_CPSC gates only movement
 *   commands when G0310/G0311 block movement, then dispatches turns to F0365
 *   and step commands to F0366.
 * - CLIKMENU.C:142-174 F0365 handles turns; CLIKMENU.C:180-347 F0366 handles
 *   movement. This test proves touch/tap input reaches those original command
 *   IDs through the same queue model; it does not add gesture behavior.
 */

typedef struct TouchMovementRouteCasePc34Compat {
    const char* name;
    int x;
    int y;
    unsigned int command;
    unsigned int zone;
    int isTurn;
} TouchMovementRouteCasePc34Compat;

static int expect_int(const char* label, int actual, int expected) {
    if (actual != expected) {
        printf("FAIL %s actual=%d expected=%d\n", label, actual, expected);
        return 0;
    }
    return 1;
}

static int expect_uint(const char* label, unsigned int actual, unsigned int expected) {
    if (actual != expected) {
        printf("FAIL %s actual=%u expected=%u\n", label, actual, expected);
        return 0;
    }
    return 1;
}

static int expect_name(const char* label, const char* actual, const char* expected) {
    if (!actual || strcmp(actual, expected) != 0) {
        printf("FAIL %s actual=%s expected=%s\n", label, actual ? actual : "(null)", expected);
        return 0;
    }
    return 1;
}

static int enqueue_touch_case(const TouchMovementRouteCasePc34Compat* route,
                              struct Dm1V1InputCommandQueuePc34Compat* queue,
                              TouchPointerDispatchPc34Compat* dispatch) {
    TouchPointerEventPc34Compat event;
    event.action = TOUCH_POINTER_ACTION_CLICK_PC34_COMPAT;
    event.space = TOUCH_POINTER_SPACE_SCREEN_320X200_PC34_COMPAT;
    event.x = route->x;
    event.y = route->y;
    event.surfaceW = 0;
    event.surfaceH = 0;
    event.buttonMask = TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT;
    return TOUCHPOINTER_Compat_EnqueueEventToInputCommandQueue(&event, queue, dispatch);
}

int main(void) {
    static const TouchMovementRouteCasePc34Compat routes[] = {
        { "movement.turn_left",    234, 125, DM1_V1_COMMAND_TURN_LEFT,     68u, 1 },
        { "movement.forward",      263, 125, DM1_V1_COMMAND_MOVE_FORWARD,  70u, 0 },
        { "movement.turn_right",   291, 125, DM1_V1_COMMAND_TURN_RIGHT,    69u, 1 },
        { "movement.left",         234, 147, DM1_V1_COMMAND_MOVE_LEFT,     73u, 0 },
        { "movement.backward",     263, 147, DM1_V1_COMMAND_MOVE_BACKWARD, 72u, 0 },
        { "movement.right",        291, 147, DM1_V1_COMMAND_MOVE_RIGHT,    71u, 0 },
    };
    unsigned int i;
    int ok = 1;

    printf("probe=touch_movement_turn_routes_pc34_compat\n");
    printf("sourceEvidence=%s\n", TOUCHPOINTER_Compat_GetSourceEvidence());

    for (i = 0; i < sizeof(routes) / sizeof(routes[0]); ++i) {
        struct Dm1V1InputCommandQueuePc34Compat queue;
        struct Dm1V1QueuedCommandPc34Compat queued;
        struct Dm1V1InputQueueProcessResultPc34Compat result;
        TouchPointerDispatchPc34Compat dispatch;

        DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
        if (!enqueue_touch_case(&routes[i], &queue, &dispatch)) {
            printf("FAIL %s enqueue_touch_case returned 0\n", routes[i].name);
            ok = 0;
            continue;
        }

        ok &= expect_int("dispatch.shouldDispatchClick", dispatch.shouldDispatchClick, 1);
        ok &= expect_int("dispatch.screenX", dispatch.screenX, routes[i].x);
        ok &= expect_int("dispatch.screenY", dispatch.screenY, routes[i].y);
        ok &= expect_uint("dispatch.buttonStatus", dispatch.buttonStatus,
                          TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT);
        ok &= expect_uint("dispatch.commandId", dispatch.commandId, routes[i].command);
        ok &= expect_uint("dispatch.zoneIndex", dispatch.zoneIndex, routes[i].zone);
        ok &= expect_int("dispatch.coordMode", dispatch.coordMode,
                         TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT);
        ok &= expect_name("dispatch.groupName", dispatch.groupName, routes[i].name);

        ok &= expect_uint("queue.count", queue.count, 1u);
        ok &= expect_int("queue.peek", DM1_V1_InputCommandQueue_PeekPc34Compat(&queue, &queued), 1);
        ok &= expect_int("queued.command", queued.command, (int)routes[i].command);
        ok &= expect_int("queued.x", queued.x, routes[i].x);
        ok &= expect_int("queued.y", queued.y, routes[i].y);

        result = DM1_V1_InputCommandQueue_ProcessOnePc34Compat(&queue, 0, 0, 0, 0);
        ok &= expect_int("process.dequeued", result.dequeued, 1);
        ok &= expect_int("process.command", result.command, (int)routes[i].command);
        ok &= expect_int("process.dispatchedTurn", result.dispatchedTurn, routes[i].isTurn ? 1 : 0);
        ok &= expect_int("process.dispatchedMove", result.dispatchedMove, routes[i].isTurn ? 0 : 1);
        ok &= expect_uint("queue.empty.after.process", queue.count, 0u);

        DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
        if (!enqueue_touch_case(&routes[i], &queue, &dispatch)) {
            printf("FAIL %s enqueue_touch_case gate setup returned 0\n", routes[i].name);
            ok = 0;
            continue;
        }
        result = DM1_V1_InputCommandQueue_ProcessOnePc34Compat(&queue, 0, 7, 0, 0);
        if (routes[i].isTurn) {
            ok &= expect_int("turn.bypasses.disabledMovementTicks", result.movementDisabledGate, 0);
            ok &= expect_int("turn.dispatched", result.dispatchedTurn, 1);
            ok &= expect_uint("turn.queue.empty", queue.count, 0u);
        } else {
            ok &= expect_int("move.respects.disabledMovementTicks", result.movementDisabledGate, 1);
            ok &= expect_int("move.not.dequeued.while.gated", result.dequeued, 0);
            ok &= expect_uint("move.queue.retained", queue.count, 1u);
            ok &= expect_int("move.retained.peek", DM1_V1_InputCommandQueue_PeekPc34Compat(&queue, &queued), 1);
            ok &= expect_int("move.retained.command", queued.command, (int)routes[i].command);
        }
    }

    printf("touchMovementTurnRoutesInvariantOk=%u\n", ok ? 1u : 0u);
    return ok ? 0 : 1;
}

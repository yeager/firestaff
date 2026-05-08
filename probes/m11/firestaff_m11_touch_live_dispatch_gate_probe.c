#include <stdio.h>
#include <string.h>

#include "touch_pointer_input_pc34_compat.h"

/* pass350: live M11/game-view touch dispatch gate.
 *
 * This probe exercises the runtime touch-pointer integration seam that M11 uses
 * for game-view pointer dispatch: touch event -> source-ordered screen hit-test
 * -> DM1 V1 input command queue.  It proves that pass347's abstraction is not
 * only table data: dispatch and queued/pending click state preserve the
 * original ReDMCSB command, screen coordinates, and button mask.
 *
 * ReDMCSB anchors audited for this gate:
 * - STARTUP2.C:1179-1182 installs primary interface before secondary movement.
 * - COMMAND.C:375-405 defines the primary interface and secondary movement
 *   mouse tables and original button masks.
 * - COMMAND.C:1379-1449 F0358 scans source mouse records in table order.
 * - COMMAND.C:1641-1661 F0359 probes primary first, then secondary, and queues
 *   command/x/y for a hit.
 * - COMMAND.C:1693-1707 F0360 replays a pending click with stored x/y/buttons.
 * - COORD.C:1693-1722 locks PC/I34E 320x200 and viewport x=0 y=33 w=224 h=136.
 * - COORD.C:1915-1921 defines inclusive point-in-zone checks.
 * - DEFS.H:197-216 defines MOUSE_INPUT and masks 0x0001 right, 0x0002 left.
 */

typedef struct TouchLiveCasePc34Compat {
    const char* name;
    TouchPointerSpacePc34Compat space;
    int x;
    int y;
    int surfaceW;
    int surfaceH;
    unsigned int buttonMask;
    unsigned int expectedCommand;
    unsigned int expectedZone;
    int expectedScreenX;
    int expectedScreenY;
    const char* expectedGroup;
} TouchLiveCasePc34Compat;

static void fill_event(const TouchLiveCasePc34Compat* c, TouchPointerEventPc34Compat* event) {
    event->action = TOUCH_POINTER_ACTION_CLICK_PC34_COMPAT;
    event->space = c->space;
    event->x = c->x;
    event->y = c->y;
    event->surfaceW = c->surfaceW;
    event->surfaceH = c->surfaceH;
    event->buttonMask = c->buttonMask;
}

static int dispatch_matches(const TouchPointerDispatchPc34Compat* dispatch,
                            const TouchLiveCasePc34Compat* c) {
    return dispatch->shouldDispatchClick == 1 &&
           dispatch->screenX == c->expectedScreenX &&
           dispatch->screenY == c->expectedScreenY &&
           dispatch->buttonStatus == c->buttonMask &&
           dispatch->commandId == c->expectedCommand &&
           dispatch->zoneIndex == c->expectedZone &&
           dispatch->coordMode == TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT &&
           dispatch->groupName && strcmp(dispatch->groupName, c->expectedGroup) == 0;
}

static int run_dispatch_case(const TouchLiveCasePc34Compat* c) {
    TouchPointerEventPc34Compat event;
    TouchPointerDispatchPc34Compat dispatch;
    struct Dm1V1InputCommandQueuePc34Compat queue;
    struct Dm1V1QueuedCommandPc34Compat queued;
    int ok = 1;

    memset(&dispatch, 0, sizeof(dispatch));
    memset(&queued, 0, sizeof(queued));
    fill_event(c, &event);
    if (!TOUCHPOINTER_Compat_TranslateEvent(&event, &dispatch) ||
        !dispatch_matches(&dispatch, c)) {
        printf("case=%s dispatch=fail command=%u zone=%u screen=%d,%d button=0x%04x group=%s\n",
               c->name,
               dispatch.commandId,
               dispatch.zoneIndex,
               dispatch.screenX,
               dispatch.screenY,
               dispatch.buttonStatus,
               dispatch.groupName ? dispatch.groupName : "(null)");
        ok = 0;
    }

    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    if (!TOUCHPOINTER_Compat_EnqueueEventToInputCommandQueue(&event, &queue, &dispatch) ||
        queue.count != 1u ||
        !DM1_V1_InputCommandQueue_PeekPc34Compat(&queue, &queued) ||
        queued.command != (int)c->expectedCommand ||
        queued.x != c->expectedScreenX || queued.y != c->expectedScreenY ||
        dispatch.buttonStatus != c->buttonMask) {
        printf("case=%s queue=fail count=%u queuedCommand=%d queuedXY=%d,%d button=0x%04x\n",
               c->name,
               queue.count,
               queued.command,
               queued.x,
               queued.y,
               dispatch.buttonStatus);
        ok = 0;
    }

    printf("case=%s command=%u zone=%u screen=%d,%d button=0x%04x group=%s result=%s\n",
           c->name,
           c->expectedCommand,
           c->expectedZone,
           c->expectedScreenX,
           c->expectedScreenY,
           c->buttonMask,
           c->expectedGroup,
           ok ? "pass" : "fail");
    return ok;
}

int main(void) {
    static const TouchLiveCasePc34Compat cases[] = {
        { "primary_left_beats_status_box_child", TOUCH_POINTER_SPACE_SCREEN_320X200_PC34_COMPAT,
          25, 11, 0, 0, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 12u, 151u, 25, 11, "champion0.status_box" },
        { "primary_right_status_box_toggle", TOUCH_POINTER_SPACE_SCREEN_320X200_PC34_COMPAT,
          25, 11, 0, 0, TOUCH_CLICK_BUTTON_RIGHT_PC34_COMPAT, 7u, 151u, 25, 11, "champion0.toggle_box" },
        { "secondary_left_movement_fallback", TOUCH_POINTER_SPACE_SCREEN_320X200_PC34_COMPAT,
          264, 126, 0, 0, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, 3u, 70u, 264, 126, "movement.forward" },
        { "secondary_right_screen_toggle", TOUCH_POINTER_SPACE_SCREEN_320X200_PC34_COMPAT,
          264, 126, 0, 0, TOUCH_CLICK_BUTTON_RIGHT_PC34_COMPAT, 83u, 2u, 264, 126, "inventory.toggle_leader" },
        { "scaled_primary_right_preserves_original_screen", TOUCH_POINTER_SPACE_SCALED_SCREEN_PC34_COMPAT,
          100, 39, 1280, 800, TOUCH_CLICK_BUTTON_RIGHT_PC34_COMPAT, 7u, 151u, 25, 9, "champion0.toggle_box" }
    };
    unsigned int i;
    int ok = 1;
    TouchPointerEventPc34Compat event;
    TouchPointerDispatchPc34Compat dispatch;
    struct Dm1V1InputCommandQueuePc34Compat queue;
    struct Dm1V1QueuedCommandPc34Compat queued;
    struct Dm1V1InputQueueProcessResultPc34Compat processResult;

    memset(&dispatch, 0, sizeof(dispatch));
    memset(&queued, 0, sizeof(queued));
    printf("probe=pass350_dm1_v1_touch_live_dispatch_gate\n");
    printf("sourceEvidence=%s\n", TOUCHPOINTER_Compat_GetSourceEvidence());

    for (i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        if (!run_dispatch_case(&cases[i])) ok = 0;
    }

    fill_event(&cases[0], &event);
    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    queue.locked = 1;
    if (TOUCHPOINTER_Compat_EnqueueEventToInputCommandQueue(&event, &queue, &dispatch) ||
        !dispatch_matches(&dispatch, &cases[0]) ||
        !queue.pendingClickPresent ||
        queue.pendingClickCommand != (int)cases[0].expectedCommand ||
        queue.pendingClickX != cases[0].expectedScreenX ||
        queue.pendingClickY != cases[0].expectedScreenY ||
        queue.pendingClickButtons != (int)cases[0].buttonMask) {
        printf("case=pending_replay_original_click_state result=fail pending=%d command=%d xy=%d,%d buttons=0x%04x\n",
               queue.pendingClickPresent,
               queue.pendingClickCommand,
               queue.pendingClickX,
               queue.pendingClickY,
               queue.pendingClickButtons);
        ok = 0;
    }
    queue.locked = 0;
    processResult = DM1_V1_InputCommandQueue_ProcessOnePc34Compat(&queue, 0, 0, 0, 0);
    if (processResult.pendingReplayCount != 1 || queue.count != 1u ||
        !DM1_V1_InputCommandQueue_PeekPc34Compat(&queue, &queued) ||
        queued.command != (int)cases[0].expectedCommand ||
        queued.x != cases[0].expectedScreenX || queued.y != cases[0].expectedScreenY) {
        printf("case=pending_replay_queue_result result=fail replay=%d count=%u command=%d xy=%d,%d\n",
               processResult.pendingReplayCount,
               queue.count,
               queued.command,
               queued.x,
               queued.y);
        ok = 0;
    } else {
        printf("case=pending_replay_original_click_state command=%u screen=%d,%d button=0x%04x result=pass\n",
               cases[0].expectedCommand,
               cases[0].expectedScreenX,
               cases[0].expectedScreenY,
               cases[0].buttonMask);
    }

    printf("primaryBeforeSecondaryClickRoutingOk=%u\n", ok ? 1u : 0u);
    printf("originalButtonMasksAndCoordinatesOk=%u\n", ok ? 1u : 0u);
    printf("pass350TouchLiveDispatchGateOk=%u\n", ok ? 1u : 0u);
    return ok ? 0 : 1;
}

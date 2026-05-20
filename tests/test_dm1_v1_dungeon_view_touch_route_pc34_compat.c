#include <stdio.h>
#include <string.h>

#include "touch_pointer_input_pc34_compat.h"

static int g_failures;

static void check_int(const char *name, int actual, int expected)
{
    if (actual != expected) {
        printf("FAIL %s actual=%d expected=%d\n", name, actual, expected);
        ++g_failures;
    }
}

static void check_str(const char *name, const char *actual, const char *expected)
{
    if (!actual || strcmp(actual, expected) != 0) {
        printf("FAIL %s actual=%s expected=%s\n", name,
               actual ? actual : "(null)", expected);
        ++g_failures;
    }
}

static void check_contains(const char *name, const char *haystack, const char *needle)
{
    if (!haystack || !strstr(haystack, needle)) {
        printf("FAIL %s missing=%s\n", name, needle);
        ++g_failures;
    }
}

static void check_viewport_zone_route(void)
{
    TouchClickZonePc34Compat zone;
    TouchClickDispatchPc34Compat dispatch;

    check_int("viewport.hit.left",
              TOUCHCLICK_Compat_HitTestPrimaryThenSecondary(
                  112, 101, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, &zone),
              1);
    check_int("viewport.hit.command", (int)zone.commandId, 80);
    check_int("viewport.hit.zone", (int)zone.zoneIndex, 7);
    check_str("viewport.hit.group", zone.groupName, "viewport.dungeon");
    check_contains("viewport.hit.evidence", zone.sourceEvidence, "COMMAND.C:403");

    check_int("viewport.right.button.route",
              TOUCHCLICK_Compat_HitTestPrimaryThenSecondary(
                  112, 101, TOUCH_CLICK_BUTTON_RIGHT_PC34_COMPAT, &zone),
              1);
    check_int("viewport.right.command", (int)zone.commandId, 83);
    check_int("viewport.right.zone", (int)zone.zoneIndex, 2);
    check_str("viewport.right.group", zone.groupName, "inventory.toggle_leader");

    check_int("viewport.local.dispatch",
              TOUCHCLICK_Compat_MapDungeonViewportLocalPointToDispatch(
                  112, 68, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, &dispatch),
              1);
    check_int("viewport.local.screen_x", dispatch.screenX, 112);
    check_int("viewport.local.screen_y", dispatch.screenY, 101);
    check_int("viewport.local.command", (int)dispatch.commandId, 80);
    check_int("viewport.local.zone", (int)dispatch.zoneIndex, 7);
    check_str("viewport.local.group", dispatch.groupName, "viewport.dungeon");

    check_int("viewport.edge.local.dispatch",
              TOUCHCLICK_Compat_MapDungeonViewportLocalPointToDispatch(
                  223, 135, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, &dispatch),
              1);
    check_int("viewport.edge.screen_x", dispatch.screenX, 223);
    check_int("viewport.edge.screen_y", dispatch.screenY, 168);
    check_int("viewport.outside.local.dispatch",
              TOUCHCLICK_Compat_MapDungeonViewportLocalPointToDispatch(
                  224, 135, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, &dispatch),
              0);
}

static void check_touch_pointer_queue_route(void)
{
    TouchPointerEventPc34Compat event;
    TouchPointerDispatchPc34Compat dispatch;
    struct Dm1V1InputCommandQueuePc34Compat queue;
    struct Dm1V1QueuedCommandPc34Compat queued;
    struct Dm1V1InputQueueProcessResultPc34Compat processed;

    event.action = TOUCH_POINTER_ACTION_CLICK_PC34_COMPAT;
    event.space = TOUCH_POINTER_SPACE_DUNGEON_VIEWPORT_LOCAL_PC34_COMPAT;
    event.x = 112;
    event.y = 68;
    event.surfaceW = 0;
    event.surfaceH = 0;
    event.buttonMask = TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT;

    check_int("pointer.translate.viewport",
              TOUCHPOINTER_Compat_TranslateEvent(&event, &dispatch), 1);
    check_int("pointer.translate.should_dispatch", dispatch.shouldDispatchClick, 1);
    check_int("pointer.translate.command", (int)dispatch.commandId, 80);
    check_int("pointer.translate.screen_x", dispatch.screenX, 112);
    check_int("pointer.translate.screen_y", dispatch.screenY, 101);

    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    check_int("pointer.enqueue.viewport",
              TOUCHPOINTER_Compat_EnqueueEventToInputCommandQueue(
                  &event, &queue, &dispatch),
              1);
    check_int("pointer.enqueue.count", (int)queue.count, 1);
    check_int("pointer.enqueue.peek",
              DM1_V1_InputCommandQueue_PeekPc34Compat(&queue, &queued), 1);
    check_int("pointer.enqueue.command", queued.command, 80);
    check_int("pointer.enqueue.x", queued.x, 112);
    check_int("pointer.enqueue.y", queued.y, 101);

    processed = DM1_V1_InputCommandQueue_ProcessOnePc34Compat(&queue, 0, 99, 0, 0);
    check_int("viewport.not_keyboard_move_gate", processed.movementDisabledGate, 0);
    check_int("viewport.not_dispatched_move", processed.dispatchedMove, 0);
    check_int("viewport.not_dispatched_turn", processed.dispatchedTurn, 0);
    check_int("viewport.dequeued", processed.dequeued, 1);
    check_int("viewport.processed.command", processed.command, 80);
}

static void check_main_ui_and_keyboard_guard(void)
{
    TouchPointerEventPc34Compat event;
    TouchPointerDispatchPc34Compat dispatch;
    struct Dm1V1InputCommandQueuePc34Compat queue;
    struct Dm1V1InputEventPc34Compat key;
    struct Dm1V1QueuedCommandPc34Compat queued;
    struct Dm1V1InputQueueProcessResultPc34Compat processed;

    event.action = TOUCH_POINTER_ACTION_CLICK_PC34_COMPAT;
    event.space = TOUCH_POINTER_SPACE_SCREEN_320X200_PC34_COMPAT;
    event.x = 286;
    event.y = 78;
    event.surfaceW = 0;
    event.surfaceH = 0;
    event.buttonMask = TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT;
    check_int("main_ui.action_parent.translate",
              TOUCHPOINTER_Compat_TranslateEvent(&event, &dispatch), 1);
    check_int("main_ui.action_parent.command", (int)dispatch.commandId, 111);
    check_int("main_ui.action_parent.zone", (int)dispatch.zoneIndex, 11);
    check_str("main_ui.action_parent.group", dispatch.groupName, "action.parent");

    event.x = 25;
    event.y = 11;
    event.buttonMask = TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT;
    check_int("main_ui.status_left.translate",
              TOUCHPOINTER_Compat_TranslateEvent(&event, &dispatch), 1);
    check_int("main_ui.status_left.command", (int)dispatch.commandId, 12);
    check_int("main_ui.status_left.zone", (int)dispatch.zoneIndex, 151);

    event.buttonMask = TOUCH_CLICK_BUTTON_RIGHT_PC34_COMPAT;
    check_int("main_ui.status_right.translate",
              TOUCHPOINTER_Compat_TranslateEvent(&event, &dispatch), 1);
    check_int("main_ui.status_right.command", (int)dispatch.commandId, 7);
    check_int("main_ui.status_right.zone", (int)dispatch.zoneIndex, 151);

    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    memset(&key, 0, sizeof(key));
    key.kind = DM1_V1_INPUT_KIND_KEY;
    key.keyCode = 0x004C;
    check_int("keyboard.forward.enqueue",
              DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue, key), 1);
    check_int("keyboard.forward.peek",
              DM1_V1_InputCommandQueue_PeekPc34Compat(&queue, &queued), 1);
    check_int("keyboard.forward.command", queued.command, DM1_V1_COMMAND_MOVE_FORWARD);
    processed = DM1_V1_InputCommandQueue_ProcessOnePc34Compat(&queue, 0, 99, 0, 0);
    check_int("keyboard.forward.movement_gate", processed.movementDisabledGate, 1);
    check_int("keyboard.forward.not_dequeued", processed.dequeued, 0);
}

int main(void)
{
    printf("probe=dm1_v1_dungeon_view_touch_route_pc34_compat\n");
    printf("touchClickEvidence=%s\n", TOUCHCLICK_Compat_GetSourceEvidence());
    printf("touchPointerEvidence=%s\n", TOUCHPOINTER_Compat_GetSourceEvidence());

    check_viewport_zone_route();
    check_touch_pointer_queue_route();
    check_main_ui_and_keyboard_guard();

    if (g_failures) {
        printf("FAIL dm1_v1_dungeon_view_touch_route_pc34_compat failures=%d\n", g_failures);
        return 1;
    }
    printf("PASS dm1_v1_dungeon_view_touch_route_pc34_compat\n");
    return 0;
}

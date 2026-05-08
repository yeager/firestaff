#include <stdio.h>
#include "dm1_v1_input_command_queue_pc34_compat.h"

static int expect_int(const char* label, int got, int want)
{
    if (got != want) {
        fprintf(stderr, "FAIL %s got=%d want=%d\n", label, got, want);
        return 0;
    }
    return 1;
}

int main(void)
{
    struct Dm1V1InputCommandQueuePc34Compat queue;
    struct Dm1V1InputQueueProcessResultPc34Compat result;
    struct Dm1V1QueuedCommandPc34Compat peek;
    int ok = 1;

    printf("probe=dm1_v1_input_command_queue_pc34_compat\n");
    printf("sourceEvidence=%s\n", DM1_V1_InputCommandQueue_SourceEvidencePc34Compat());

    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    ok &= expect_int("enqueue keypad forward", DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
        (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_KEY, 0xAB35, 0, 0, 0 }), 1);
    ok &= expect_int("count after key", (int)queue.count, 1);

    queue.locked = 1;
    ok &= expect_int("locked mouse becomes pending", DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
        (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_MOUSE, 0, 240, 130, DM1_V1_BUTTON_LEFT }), 0);
    ok &= expect_int("pending present", queue.pendingClickPresent, 1);
    ok &= expect_int("pending command locked mouse", queue.pendingClickCommand, DM1_V1_COMMAND_TURN_LEFT);
    queue.locked = 0;

    result = DM1_V1_InputCommandQueue_ProcessOnePc34Compat(&queue, 0, 1, 0, 0);
    ok &= expect_int("movement gate set", result.movementDisabledGate, 1);
    ok &= expect_int("movement not dequeued", result.dequeued, 0);
    ok &= expect_int("pending replayed", result.pendingReplayCount, 1);
    ok &= expect_int("forward plus replayed turn queued", (int)queue.count, 2);
    ok &= expect_int("front still forward", DM1_V1_InputCommandQueue_PeekPc34Compat(&queue, &peek), 1);
    ok &= expect_int("front command", peek.command, DM1_V1_COMMAND_MOVE_FORWARD);

    result = DM1_V1_InputCommandQueue_ProcessOnePc34Compat(&queue, 0, 0, 0, 0);
    ok &= expect_int("forward dequeued after gate clears", result.dequeued, 1);
    ok &= expect_int("forward dispatched move", result.dispatchedMove, 1);
    ok &= expect_int("turn remains", (int)queue.count, 1);
    ok &= expect_int("front turn", DM1_V1_InputCommandQueue_PeekPc34Compat(&queue, &peek), 1);
    ok &= expect_int("turn command", peek.command, DM1_V1_COMMAND_TURN_LEFT);

    result = DM1_V1_InputCommandQueue_ProcessOnePc34Compat(&queue, 0, 0, 0, 0);
    ok &= expect_int("turn dequeued", result.dequeued, 1);
    ok &= expect_int("turn dispatched", result.dispatchedTurn, 1);
    ok &= expect_int("queue empty", (int)queue.count, 0);

    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    ok &= expect_int("enqueue turn while movement disabled", DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
        (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_KEY, 0xAB36, 0, 0, 0 }), 1);
    result = DM1_V1_InputCommandQueue_ProcessOnePc34Compat(&queue, 0, 7, 3, 0);
    ok &= expect_int("turn bypasses movement disabled gate", result.movementDisabledGate, 0);
    ok &= expect_int("turn dequeued despite movement disabled", result.dequeued, 1);
    ok &= expect_int("turn dispatched despite movement disabled", result.dispatchedTurn, 1);
    ok &= expect_int("turn-disabled-gate queue empty", (int)queue.count, 0);

    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    queue.locked = 1;
    ok &= expect_int("locked direct touch inventory pending",
        DM1_V1_InputCommandQueue_EnqueueMouseCommandPc34Compat(&queue, 71, 12, 13, DM1_V1_BUTTON_LEFT), 0);
    ok &= expect_int("locked direct touch command stored", queue.pendingClickCommand, 71);
    queue.locked = 0;
    result = DM1_V1_InputCommandQueue_ProcessOnePc34Compat(&queue, 0, 0, 0, 0);
    ok &= expect_int("direct touch pending replayed", result.pendingReplayCount, 1);
    ok &= expect_int("direct touch queued", (int)queue.count, 1);
    ok &= expect_int("direct touch peek", DM1_V1_InputCommandQueue_PeekPc34Compat(&queue, &peek), 1);
    ok &= expect_int("direct touch command", peek.command, 71);

    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    ok &= expect_int("primary action area beats secondary viewport",
        DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
        (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_MOUSE, 0, 250, 90, DM1_V1_BUTTON_LEFT }), 1);
    ok &= expect_int("primary action queued", DM1_V1_InputCommandQueue_PeekPc34Compat(&queue, &peek), 1);
    ok &= expect_int("primary action command", peek.command, DM1_V1_COMMAND_CLICK_IN_ACTION_AREA);
    result = DM1_V1_InputCommandQueue_ProcessOnePc34Compat(&queue, 0, 0, 0, 0);
    ok &= expect_int("primary action dequeued", result.dequeued, 1);
    ok &= expect_int("primary action result command", result.command, DM1_V1_COMMAND_CLICK_IN_ACTION_AREA);

    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    ok &= expect_int("primary spell area beats secondary viewport",
        DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
        (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_MOUSE, 0, 250, 60, DM1_V1_BUTTON_LEFT }), 1);
    ok &= expect_int("primary spell queued", DM1_V1_InputCommandQueue_PeekPc34Compat(&queue, &peek), 1);
    ok &= expect_int("primary spell command", peek.command, DM1_V1_COMMAND_CLICK_IN_SPELL_AREA);

    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    queue.locked = 1;
    ok &= expect_int("locked primary action pending",
        DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
        (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_MOUSE, 0, 250, 90, DM1_V1_BUTTON_LEFT }), 0);
    ok &= expect_int("locked primary action pending command", queue.pendingClickCommand, DM1_V1_COMMAND_CLICK_IN_ACTION_AREA);
    queue.locked = 0;
    result = DM1_V1_InputCommandQueue_ProcessOnePc34Compat(&queue, 0, 0, 0, 0);
    ok &= expect_int("locked primary action replayed", result.pendingReplayCount, 1);
    ok &= expect_int("locked primary action replay command", DM1_V1_InputCommandQueue_PeekPc34Compat(&queue, &peek), 1);
    ok &= expect_int("locked primary action queued", peek.command, DM1_V1_COMMAND_CLICK_IN_ACTION_AREA);

    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    ok &= expect_int("left button bar graph source-order toggle",
        DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
        (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_MOUSE, 0, 50, 10, DM1_V1_BUTTON_LEFT }), 1);
    ok &= expect_int("left button bar graph command", DM1_V1_InputCommandQueue_PeekPc34Compat(&queue, &peek), 1);
    ok &= expect_int("left button bar graph toggle command", peek.command, DM1_V1_COMMAND_TOGGLE_INVENTORY_CHAMPION_0);

    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    ok &= expect_int("left button name-hand status command",
        DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
        (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_MOUSE, 0, 25, 10, DM1_V1_BUTTON_LEFT }), 1);
    ok &= expect_int("left button status command", DM1_V1_InputCommandQueue_PeekPc34Compat(&queue, &peek), 1);
    ok &= expect_int("left button champion status command", peek.command, DM1_V1_COMMAND_CLICK_CHAMPION_STATUS_0);

    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    ok &= expect_int("right button top primary champion toggle",
        DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
        (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_MOUSE, 0, 25, 11, DM1_V1_BUTTON_RIGHT }), 1);
    ok &= expect_int("right button primary command", DM1_V1_InputCommandQueue_PeekPc34Compat(&queue, &peek), 1);
    ok &= expect_int("right button primary champion command", peek.command, DM1_V1_COMMAND_TOGGLE_INVENTORY_CHAMPION_0);

    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    ok &= expect_int("right button secondary full-screen leader toggle",
        DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
        (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_MOUSE, 0, 250, 180, DM1_V1_BUTTON_RIGHT }), 1);
    ok &= expect_int("right button secondary command", DM1_V1_InputCommandQueue_PeekPc34Compat(&queue, &peek), 1);
    ok &= expect_int("right button secondary leader command", peek.command, DM1_V1_COMMAND_TOGGLE_INVENTORY_LEADER);

    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    ok &= expect_int("pc34 table left arrow turns left", DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
        (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_KEY, 0x004B, 0, 0, 0 }), 1);
    result = DM1_V1_InputCommandQueue_ProcessOnePc34Compat(&queue, 0, 0, 0, 0);
    ok &= expect_int("pc34 table left arrow command", result.command, DM1_V1_COMMAND_TURN_LEFT);
    ok &= expect_int("pc34 table left arrow dispatches turn", result.dispatchedTurn, 1);

    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    ok &= expect_int("pc34 table up arrow moves forward", DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
        (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_KEY, 0x004C, 0, 0, 0 }), 1);
    result = DM1_V1_InputCommandQueue_ProcessOnePc34Compat(&queue, 0, 0, 0, 0);
    ok &= expect_int("pc34 table up arrow command", result.command, DM1_V1_COMMAND_MOVE_FORWARD);
    ok &= expect_int("pc34 table up arrow dispatches move", result.dispatchedMove, 1);

    /* Source lock: ReDMCSB COMMAND.C:677-684 is the I34E movement
     * keyboard table. It is keypad/scancode-shaped, not WASD-shaped:
     * 0x4B/0x4C/0x4D/0x4F/0x50/0x51 are left/forward/right/
     * strafe-left/back/strafe-right. This probe keeps the full table
     * pinned so SDL NumLock-on keypad routing in main_loop_m11.c cannot
     * collapse to arrow-only movement again. */
    {
        const int keyCodes[6] = { 0x004B, 0x004C, 0x004D, 0x004F, 0x0050, 0x0051 };
        const int commands[6] = {
            DM1_V1_COMMAND_TURN_LEFT,
            DM1_V1_COMMAND_MOVE_FORWARD,
            DM1_V1_COMMAND_TURN_RIGHT,
            DM1_V1_COMMAND_MOVE_LEFT,
            DM1_V1_COMMAND_MOVE_BACKWARD,
            DM1_V1_COMMAND_MOVE_RIGHT
        };
        const char* labels[6] = {
            "i34e keypad 4 turns left",
            "i34e keypad 5 moves forward",
            "i34e keypad 6 turns right",
            "i34e keypad 1 strafes left",
            "i34e keypad 2 moves backward",
            "i34e keypad 3 strafes right"
        };
        int i;
        for (i = 0; i < 6; ++i) {
            DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
            ok &= expect_int(labels[i], DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
                (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_KEY, keyCodes[i], 0, 0, 0 }), 1);
            result = DM1_V1_InputCommandQueue_ProcessOnePc34Compat(&queue, 0, 0, 0, 0);
            ok &= expect_int(labels[i], result.command, commands[i]);
        }
    }

    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    ok &= expect_int("pc34 io2 shifted up arrow normalizes to forward", DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
        (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_KEY, 0x004C, 0, 0, 0 }), 1);
    result = DM1_V1_InputCommandQueue_ProcessOnePc34Compat(&queue, 0, 0, 0, 0);
    ok &= expect_int("pc34 io2 shifted up arrow command", result.command, DM1_V1_COMMAND_MOVE_FORWARD);
    ok &= expect_int("pc34 io2 shifted up arrow dispatches move", result.dispatchedMove, 1);

    ok &= expect_int("pc34 shifted del turns left", DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
        (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_KEY, 0x9BFF, 0, 0, 0 }), 1);
    result = DM1_V1_InputCommandQueue_ProcessOnePc34Compat(&queue, 0, 0, 0, 0);
    ok &= expect_int("pc34 shifted del command", result.command, DM1_V1_COMMAND_TURN_LEFT);
    ok &= expect_int("pc34 shifted del dispatches turn", result.dispatchedTurn, 1);

    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    ok &= expect_int("pc34 shifted help turns right", DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
        (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_KEY, 0x9B6F, 0, 0, 0 }), 1);
    result = DM1_V1_InputCommandQueue_ProcessOnePc34Compat(&queue, 0, 0, 0, 0);
    ok &= expect_int("pc34 shifted help command", result.command, DM1_V1_COMMAND_TURN_RIGHT);
    ok &= expect_int("pc34 shifted help dispatches turn", result.dispatchedTurn, 1);

    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    ok &= expect_int("pc34 shifted backward arrow strafes right", DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
        (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_KEY, 0x9B60, 0, 0, 0 }), 1);
    result = DM1_V1_InputCommandQueue_ProcessOnePc34Compat(&queue, 0, 0, 0, 0);
    ok &= expect_int("pc34 shifted backward arrow command", result.command, DM1_V1_COMMAND_MOVE_RIGHT);
    ok &= expect_int("pc34 shifted backward arrow dispatches move", result.dispatchedMove, 1);

    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    ok &= expect_int("queue accepts first command", DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
        (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_KEY, 0xAB35, 0, 0, 0 }), 1);
    ok &= expect_int("queue accepts second command", DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
        (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_KEY, 0xAB35, 0, 0, 0 }), 1);
    ok &= expect_int("queue accepts third command", DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
        (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_KEY, 0xAB35, 0, 0, 0 }), 1);
    ok &= expect_int("queue accepts fourth command", DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
        (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_KEY, 0xAB35, 0, 0, 0 }), 1);
    ok &= expect_int("redmcsb queue holds four before C5 limit", (int)queue.count, 4);
    ok &= expect_int("redmcsb fifth command accepted before C5 limit", DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
        (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_KEY, 0xAB35, 0, 0, 0 }), 1);
    ok &= expect_int("redmcsb queue holds five commands", (int)queue.count, 5);
    ok &= expect_int("redmcsb sixth command is dropped at C5 limit", DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
        (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_KEY, 0xAB35, 0, 0, 0 }), 0);
    ok &= expect_int("redmcsb sixth command counted dropped", (int)queue.droppedFullCount, 1);

    ok &= expect_int("release command uses first reserved slot",
        DM1_V1_InputCommandQueue_EnqueueMouseCommandPc34Compat(&queue,
            DM1_V1_COMMAND_RELEASE_CHAMPION_ICON, 11, 12, 0), 1);
    ok &= expect_int("release slot raises queue to six", (int)queue.count, 6);
    ok &= expect_int("stop pressing uses second reserved slot",
        DM1_V1_InputCommandQueue_EnqueueMouseCommandPc34Compat(&queue,
            DM1_V1_COMMAND_STOP_PRESSING_EYE_MOUTH_WALL, 13, 14, 0), 1);
    ok &= expect_int("reserved queue holds seven commands", (int)queue.count, 7);
    ok &= expect_int("third reserved release is dropped at C7 limit",
        DM1_V1_InputCommandQueue_EnqueueMouseCommandPc34Compat(&queue,
            DM1_V1_COMMAND_RELEASE_CHAMPION_ICON, 15, 16, 0), 0);
    ok &= expect_int("reserved overflow counted dropped", (int)queue.droppedFullCount, 2);

    /* Source lock: ReDMCSB COMMAND.C:2095-2100 gates only movement
     * commands whose relative direction matches the pending projectile
     * disable direction: normalize(partyDirection + command - C003).
     * The command remains queued on a matching projectile gate and is
     * dequeued on a non-matching projectile direction. */
    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    ok &= expect_int("projectile-gated forward enqueue", DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
        (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_KEY, 0x004C, 0, 0, 0 }), 1);
    result = DM1_V1_InputCommandQueue_ProcessOnePc34Compat(&queue, 1, 0, 4, 1);
    ok &= expect_int("projectile matching direction gates move", result.movementDisabledGate, 1);
    ok &= expect_int("projectile matching direction keeps command queued", result.dequeued, 0);
    ok &= expect_int("projectile gated command still queued", (int)queue.count, 1);

    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    ok &= expect_int("projectile-mismatched forward enqueue", DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
        (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_KEY, 0x004C, 0, 0, 0 }), 1);
    result = DM1_V1_InputCommandQueue_ProcessOnePc34Compat(&queue, 1, 0, 4, 2);
    ok &= expect_int("projectile mismatched direction does not gate", result.movementDisabledGate, 0);
    ok &= expect_int("projectile mismatched direction dequeues", result.dequeued, 1);
    ok &= expect_int("projectile mismatched direction dispatches move", result.dispatchedMove, 1);

    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    ok &= expect_int("projectile-gated strafe-left enqueue", DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
        (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_KEY, 0x004F, 0, 0, 0 }), 1);
    result = DM1_V1_InputCommandQueue_ProcessOnePc34Compat(&queue, 0, 0, 4, 3);
    ok &= expect_int("projectile gates normalized strafe-left direction", result.movementDisabledGate, 1);
    ok &= expect_int("projectile gated strafe-left stays queued", (int)queue.count, 1);

    printf("dm1V1InputCommandQueueInvariantOk=%u\n", ok ? 1u : 0u);
    return ok ? 0 : 1;
}

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

    printf("dm1V1InputCommandQueueInvariantOk=%u\n", ok ? 1u : 0u);
    return ok ? 0 : 1;
}

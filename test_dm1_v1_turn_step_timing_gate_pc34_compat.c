#include <stdio.h>

#include "dm1_v1_input_command_queue_pc34_compat.h"
#include "dm1_v1_movement_timing_pc34_compat.h"

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
    int disabledTicks = 2;
    int projectileTicks = 0;
    int ok = 1;

    printf("probe=dm1_v1_turn_step_timing_gate_pc34_compat\n");
    printf("queueSourceEvidence=%s\n", DM1_V1_InputCommandQueue_SourceEvidencePc34Compat());
    printf("timingSourceEvidence=%s\n", DM1_V1_MovementTiming_SourceEvidencePc34Compat());

    ok &= expect_int("pc34 input wait window is 12 vblanks",
        DM1_V1_MovementTiming_InputWaitMaxVBlanksPc34Compat(), 12);
    ok &= expect_int("input wait does not stop before twelfth vblank",
        DM1_V1_MovementTiming_InputWaitStopsAfterVBlanksPc34Compat(11), 0);
    ok &= expect_int("input wait stops at twelfth vblank",
        DM1_V1_MovementTiming_InputWaitStopsAfterVBlanksPc34Compat(12), 1);
    ok &= expect_int("vblank wait does not decrement movement cooldown",
        DM1_V1_MovementTiming_VBlankWaitDecrementsMovementCooldownPc34Compat(), 0);

    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    ok &= expect_int("queued keyboard turn accepted while step cooldown active",
        DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
        (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_KEY, 0x004D, 0, 0, 0 }), 1);
    result = DM1_V1_InputCommandQueue_ProcessOnePc34Compat(&queue, 0, disabledTicks, projectileTicks, 0);
    ok &= expect_int("queued keyboard turn bypasses movement-disabled gate",
        result.movementDisabledGate, 0);
    ok &= expect_int("queued keyboard turn dequeues during wait loop",
        result.dequeued, 1);
    ok &= expect_int("queued keyboard turn dispatches turn path",
        result.dispatchedTurn, 1);
    ok &= expect_int("queued keyboard turn is not step path",
        result.dispatchedMove, 0);
    ok &= expect_int("turn dispatch leaves cooldown for game-loop tick",
        disabledTicks, 2);

    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    ok &= expect_int("queued keyboard forward accepted while step cooldown active",
        DM1_V1_InputCommandQueue_EnqueueEventPc34Compat(&queue,
        (struct Dm1V1InputEventPc34Compat){ DM1_V1_INPUT_KIND_KEY, 0x004C, 0, 0, 0 }), 1);
    result = DM1_V1_InputCommandQueue_ProcessOnePc34Compat(&queue, 0, disabledTicks, projectileTicks, 0);
    ok &= expect_int("queued keyboard forward hits movement-disabled gate",
        result.movementDisabledGate, 1);
    ok &= expect_int("queued keyboard forward remains queued while gated",
        result.dequeued, 0);
    ok &= expect_int("queued keyboard forward waits through vblank window",
        DM1_V1_InputCommandQueue_PeekPc34Compat(&queue, &peek), 1);
    ok &= expect_int("front command is still forward after vblank wait gate",
        peek.command, DM1_V1_COMMAND_MOVE_FORWARD);
    ok &= expect_int("vblank wait left movement cooldown unchanged",
        disabledTicks, 2);

    DM1_V1_MovementTiming_DecrementCooldownsPc34Compat(&disabledTicks, &projectileTicks);
    ok &= expect_int("game loop tick decrements movement cooldown once",
        disabledTicks, 1);
    result = DM1_V1_InputCommandQueue_ProcessOnePc34Compat(&queue, 0, disabledTicks, projectileTicks, 0);
    ok &= expect_int("one remaining cooldown tick still gates forward",
        result.movementDisabledGate, 1);
    ok &= expect_int("forward still queued after second gated pass",
        (int)queue.count, 1);

    DM1_V1_MovementTiming_DecrementCooldownsPc34Compat(&disabledTicks, &projectileTicks);
    ok &= expect_int("second game loop tick clears movement cooldown",
        disabledTicks, 0);
    result = DM1_V1_InputCommandQueue_ProcessOnePc34Compat(&queue, 0, disabledTicks, projectileTicks, 0);
    ok &= expect_int("cleared cooldown releases queued forward",
        result.dequeued, 1);
    ok &= expect_int("released queued forward dispatches step path",
        result.dispatchedMove, 1);
    ok &= expect_int("released queued forward is not turn path",
        result.dispatchedTurn, 0);

    printf("dm1V1TurnStepTimingGateInvariantOk=%u\n", ok ? 1u : 0u);
    return ok ? 0 : 1;
}

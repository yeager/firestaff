#include <stdio.h>
#include <string.h>

#include "dm1_v1_host_input_bridge_pc34_compat.h"

static int failures;

static void expect_int(const char* label, int actual, int expected)
{
    if (actual != expected) {
        printf("FAIL %s actual=%d expected=%d\n", label, actual, expected);
        failures++;
    }
}

static Dm1V1HostInputPolicyPc34Compat active_policy(void)
{
    Dm1V1HostInputPolicyPc34Compat policy;
    memset(&policy, 0, sizeof(policy));
    policy.gameplayActive = 1;
    policy.fullscreenSurfaceW = 1280;
    policy.fullscreenSurfaceH = 800;
    return policy;
}

int main(void)
{
    struct Dm1V1InputCommandQueuePc34Compat queue;
    struct Dm1V1QueuedCommandPc34Compat queued;
    Dm1V1HostInputPolicyPc34Compat policy = active_policy();
    Dm1V1HostInputEventPc34Compat event;
    Dm1V1HostInputResultPc34Compat result;
    unsigned int i;

    memset(&event, 0, sizeof(event));
    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    event.kind = DM1_V1_HOST_INPUT_CONTROLLER_PC34_COMPAT;
    event.controller = DM1_V1_CONTROLLER_AFFORDANCE_DPAD_UP_PC34_COMPAT;
    expect_int("controller up accepted", DM1_V1_HostInputBridge_EnqueuePc34Compat(
        &policy, &event, &queue, &result), 1);
    expect_int("controller up command", result.commandId, DM1_V1_COMMAND_MOVE_FORWARD);
    expect_int("controller queue count", (int)queue.count, 1);
    expect_int("controller queue peek", DM1_V1_InputCommandQueue_PeekPc34Compat(&queue, &queued), 1);
    expect_int("controller source command", queued.command, DM1_V1_COMMAND_MOVE_FORWARD);

    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    event.controller = DM1_V1_CONTROLLER_AFFORDANCE_STRAFE_LEFT_PC34_COMPAT;
    expect_int("controller strafe accepted", DM1_V1_HostInputBridge_EnqueuePc34Compat(
        &policy, &event, &queue, &result), 1);
    expect_int("controller strafe is C006", result.commandId, DM1_V1_COMMAND_MOVE_LEFT);

    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    policy.modalActive = 1;
    expect_int("modal rejects controller", DM1_V1_HostInputBridge_EnqueuePc34Compat(
        &policy, &event, &queue, &result), 0);
    expect_int("modal reason", result.decision, DM1_V1_HOST_INPUT_REJECT_MODAL_PC34_COMPAT);
    expect_int("modal leaves queue", (int)queue.count, 0);
    policy.modalActive = 0;

    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    memset(&event, 0, sizeof(event));
    event.kind = DM1_V1_HOST_INPUT_TOUCH_PC34_COMPAT;
    /* 1280x800 fullscreen -> 320x200 source.  (1052,500) -> C003 arrow. */
    event.physicalX = 1052;
    event.physicalY = 500;
    event.buttonMask = DM1_V1_BUTTON_LEFT;
    expect_int("scaled touch accepted", DM1_V1_HostInputBridge_EnqueuePc34Compat(
        &policy, &event, &queue, &result), 1);
    expect_int("scaled touch forward", result.commandId, DM1_V1_COMMAND_MOVE_FORWARD);
    expect_int("scaled touch x", result.sourceX, 263);
    expect_int("scaled touch y", result.sourceY, 125);

    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    event.kind = DM1_V1_HOST_INPUT_POINTER_PC34_COMPAT;
    event.physicalX = 0;
    event.physicalY = 0;
    expect_int("pointer primary route", DM1_V1_HostInputBridge_EnqueuePc34Compat(
        &policy, &event, &queue, &result), 1);
    expect_int("pointer champion source command", result.commandId,
               DM1_V1_COMMAND_CLICK_CHAMPION_STATUS_0);

    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    memset(&event, 0, sizeof(event));
    event.kind = DM1_V1_HOST_INPUT_KEY_COMMAND_PC34_COMPAT;
    event.commandId = DM1_V1_COMMAND_TURN_RIGHT;
    expect_int("key source command accepted", DM1_V1_HostInputBridge_EnqueuePc34Compat(
        &policy, &event, &queue, &result), 1);
    expect_int("key source command", result.commandId, DM1_V1_COMMAND_TURN_RIGHT);

    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    for (i = 0; i < 5u; ++i) {
        (void)DM1_V1_InputCommandQueue_EnqueueCommandPc34Compat(
            &queue, DM1_V1_COMMAND_MOVE_FORWARD, 0, 0);
    }
    expect_int("full queue rejected", DM1_V1_HostInputBridge_EnqueuePc34Compat(
        &policy, &event, &queue, &result), 0);
    expect_int("full queue reason", result.decision,
               DM1_V1_HOST_INPUT_REJECT_QUEUE_FULL_PC34_COMPAT);

    if (strstr(DM1_V1_HostInputBridge_SourceEvidencePc34Compat(), "F0380") == NULL) {
        printf("FAIL source evidence\n");
        failures++;
    }
    printf("dm1_v1_host_input_bridge_pc34_compat failures=%d\n", failures);
    return failures ? 1 : 0;
}

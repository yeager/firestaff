#include "dm1_v1_host_input_bridge_pc34_compat.h"

#include <string.h>

static int dm1_v1_is_navigation_command(int commandId)
{
    return commandId >= DM1_V1_COMMAND_TURN_LEFT &&
           commandId <= DM1_V1_COMMAND_MOVE_LEFT;
}

static void dm1_v1_host_input_clear(Dm1V1HostInputResultPc34Compat* result)
{
    if (result) {
        memset(result, 0, sizeof(*result));
        result->decision = DM1_V1_HOST_INPUT_REJECT_INVALID_PC34_COMPAT;
    }
}

int DM1_V1_HostInputBridge_ControllerCommandPc34Compat(
    Dm1V1ControllerAffordancePc34Compat affordance)
{
    switch (affordance) {
    case DM1_V1_CONTROLLER_AFFORDANCE_DPAD_LEFT_PC34_COMPAT:
        return DM1_V1_COMMAND_TURN_LEFT;
    case DM1_V1_CONTROLLER_AFFORDANCE_DPAD_UP_PC34_COMPAT:
        return DM1_V1_COMMAND_MOVE_FORWARD;
    case DM1_V1_CONTROLLER_AFFORDANCE_DPAD_RIGHT_PC34_COMPAT:
        return DM1_V1_COMMAND_TURN_RIGHT;
    case DM1_V1_CONTROLLER_AFFORDANCE_DPAD_DOWN_PC34_COMPAT:
        return DM1_V1_COMMAND_MOVE_BACKWARD;
    case DM1_V1_CONTROLLER_AFFORDANCE_STRAFE_LEFT_PC34_COMPAT:
        return DM1_V1_COMMAND_MOVE_LEFT;
    case DM1_V1_CONTROLLER_AFFORDANCE_STRAFE_RIGHT_PC34_COMPAT:
        return DM1_V1_COMMAND_MOVE_RIGHT;
    default:
        return DM1_V1_COMMAND_NONE;
    }
}

static int dm1_v1_host_input_enqueue_command(
    struct Dm1V1InputCommandQueuePc34Compat* queue,
    int commandId,
    int sourceX,
    int sourceY,
    Dm1V1HostInputResultPc34Compat* result)
{
    if (!dm1_v1_is_navigation_command(commandId)) {
        result->decision = DM1_V1_HOST_INPUT_REJECT_UNMAPPED_PC34_COMPAT;
        return 0;
    }
    if (!DM1_V1_InputCommandQueue_EnqueueCommandPc34Compat(
            queue, commandId, sourceX, sourceY)) {
        result->decision = DM1_V1_HOST_INPUT_REJECT_QUEUE_FULL_PC34_COMPAT;
        return 0;
    }
    result->decision = DM1_V1_HOST_INPUT_ACCEPTED_PC34_COMPAT;
    result->enqueued = 1;
    result->commandId = commandId;
    result->sourceX = sourceX;
    result->sourceY = sourceY;
    return 1;
}

int DM1_V1_HostInputBridge_EnqueuePc34Compat(
    const Dm1V1HostInputPolicyPc34Compat* policy,
    const Dm1V1HostInputEventPc34Compat* event,
    struct Dm1V1InputCommandQueuePc34Compat* queue,
    Dm1V1HostInputResultPc34Compat* outResult)
{
    Dm1V1HostInputResultPc34Compat localResult;
    Dm1V1HostInputResultPc34Compat* result =
        outResult ? outResult : &localResult;
    TouchPointerEventPc34Compat pointerEvent;
    TouchPointerDispatchPc34Compat dispatch;
    int commandId;

    dm1_v1_host_input_clear(result);
    if (!policy || !event || !queue) {
        return 0;
    }
    if (!policy->gameplayActive) {
        result->decision = DM1_V1_HOST_INPUT_REJECT_INACTIVE_PC34_COMPAT;
        return 0;
    }
    /* A host overlay must own input before it can reach COMMAND.C. */
    if (policy->modalActive) {
        result->decision = DM1_V1_HOST_INPUT_REJECT_MODAL_PC34_COMPAT;
        return 0;
    }

    switch (event->kind) {
    case DM1_V1_HOST_INPUT_KEY_COMMAND_PC34_COMPAT:
        return dm1_v1_host_input_enqueue_command(queue, event->commandId, 0, 0,
                                                  result);
    case DM1_V1_HOST_INPUT_CONTROLLER_PC34_COMPAT:
        commandId = DM1_V1_HostInputBridge_ControllerCommandPc34Compat(
            event->controller);
        return dm1_v1_host_input_enqueue_command(queue, commandId, 0, 0, result);
    case DM1_V1_HOST_INPUT_POINTER_PC34_COMPAT:
    case DM1_V1_HOST_INPUT_TOUCH_PC34_COMPAT:
        if (policy->fullscreenSurfaceW <= 0 || policy->fullscreenSurfaceH <= 0 ||
            event->buttonMask == 0u) {
            result->decision = DM1_V1_HOST_INPUT_REJECT_INVALID_PC34_COMPAT;
            return 0;
        }
        if (!TOUCHPOINTER_Compat_EventFromScaledTap(
                event->physicalX, event->physicalY,
                policy->fullscreenSurfaceW, policy->fullscreenSurfaceH,
                event->buttonMask, &pointerEvent) ||
            !TOUCHPOINTER_Compat_TranslateEvent(&pointerEvent, &dispatch)) {
            result->decision = DM1_V1_HOST_INPUT_REJECT_UNMAPPED_PC34_COMPAT;
            return 0;
        }
        result->pointerDispatch = dispatch;
        if (!DM1_V1_InputCommandQueue_EnqueueMouseCommandPc34Compat(
                queue, (int)dispatch.commandId, dispatch.screenX,
                dispatch.screenY, (int)dispatch.buttonStatus)) {
            result->decision = DM1_V1_HOST_INPUT_REJECT_QUEUE_FULL_PC34_COMPAT;
            return 0;
        }
        result->decision = DM1_V1_HOST_INPUT_ACCEPTED_PC34_COMPAT;
        result->enqueued = 1;
        result->commandId = (int)dispatch.commandId;
        result->sourceX = dispatch.screenX;
        result->sourceY = dispatch.screenY;
        return 1;
    default:
        result->decision = DM1_V1_HOST_INPUT_REJECT_UNMAPPED_PC34_COMPAT;
        return 0;
    }
}

const char* DM1_V1_HostInputBridge_SourceEvidencePc34Compat(void)
{
    return "ReDMCSB COMMAND.C:375-405 G0448 C001/C003/C002/C006/C005/C004 mouse rows; COMMAND.C:1379-1449 F0358 source-zone lookup; COMMAND.C:1452-1661 F0359 click admission; COMMAND.C:1709-1813 F0361 key admission; COMMAND.C:2045-2156 F0380 queue dispatch; CLIKMENU.C:142-174 F0365 turn and 180-347 F0366 movement; INPUT.C:641-664 raw button forwarding. Host controller directions are aliases only for those C001-C006 commands, and pointer/touch retains F0358 coordinate lookup after fullscreen normalization.";
}

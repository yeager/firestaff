#ifndef DM1_V1_HOST_INPUT_BRIDGE_PC34_COMPAT_H
#define DM1_V1_HOST_INPUT_BRIDGE_PC34_COMPAT_H

#include "dm1_v1_input_command_queue_pc34_compat.h"
#include "touch_pointer_input_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * One host-input admission point for DM1 V1 gameplay.  It deliberately
 * converts host devices only to ReDMCSB's existing COMMAND.C command ids;
 * it never creates a parallel movement or click route.
 *
 * Source lock: ReDMCSB COMMAND.C G0447/G0448, F0358/F0359/F0361/F0380;
 * CLIKMENU.C F0365/F0366; INPUT.C F0540.  Controller affordances are host
 * aliases for those source commands, while touch/pointer input still passes
 * through the source mouse-zone matcher.
 */
typedef enum Dm1V1HostInputKindPc34Compat {
    DM1_V1_HOST_INPUT_NONE_PC34_COMPAT = 0,
    DM1_V1_HOST_INPUT_KEY_COMMAND_PC34_COMPAT,
    DM1_V1_HOST_INPUT_CONTROLLER_PC34_COMPAT,
    DM1_V1_HOST_INPUT_POINTER_PC34_COMPAT,
    DM1_V1_HOST_INPUT_TOUCH_PC34_COMPAT
} Dm1V1HostInputKindPc34Compat;

typedef enum Dm1V1ControllerAffordancePc34Compat {
    DM1_V1_CONTROLLER_AFFORDANCE_NONE_PC34_COMPAT = 0,
    DM1_V1_CONTROLLER_AFFORDANCE_DPAD_LEFT_PC34_COMPAT,
    DM1_V1_CONTROLLER_AFFORDANCE_DPAD_UP_PC34_COMPAT,
    DM1_V1_CONTROLLER_AFFORDANCE_DPAD_RIGHT_PC34_COMPAT,
    DM1_V1_CONTROLLER_AFFORDANCE_DPAD_DOWN_PC34_COMPAT,
    DM1_V1_CONTROLLER_AFFORDANCE_STRAFE_LEFT_PC34_COMPAT,
    DM1_V1_CONTROLLER_AFFORDANCE_STRAFE_RIGHT_PC34_COMPAT
} Dm1V1ControllerAffordancePc34Compat;

typedef enum Dm1V1HostInputRejectPc34Compat {
    DM1_V1_HOST_INPUT_ACCEPTED_PC34_COMPAT = 0,
    DM1_V1_HOST_INPUT_REJECT_INVALID_PC34_COMPAT,
    DM1_V1_HOST_INPUT_REJECT_INACTIVE_PC34_COMPAT,
    DM1_V1_HOST_INPUT_REJECT_MODAL_PC34_COMPAT,
    DM1_V1_HOST_INPUT_REJECT_UNMAPPED_PC34_COMPAT,
    DM1_V1_HOST_INPUT_REJECT_QUEUE_FULL_PC34_COMPAT
} Dm1V1HostInputRejectPc34Compat;

typedef struct Dm1V1HostInputPolicyPc34Compat {
    int gameplayActive;
    int modalActive;
    int fullscreenSurfaceW;
    int fullscreenSurfaceH;
} Dm1V1HostInputPolicyPc34Compat;

typedef struct Dm1V1HostInputEventPc34Compat {
    Dm1V1HostInputKindPc34Compat kind;
    int commandId; /* keyboard host mapping has already resolved to C001..C006 */
    Dm1V1ControllerAffordancePc34Compat controller;
    int physicalX;
    int physicalY;
    unsigned int buttonMask;
} Dm1V1HostInputEventPc34Compat;

typedef struct Dm1V1HostInputResultPc34Compat {
    Dm1V1HostInputRejectPc34Compat decision;
    int enqueued;
    int commandId;
    int sourceX;
    int sourceY;
    TouchPointerDispatchPc34Compat pointerDispatch;
} Dm1V1HostInputResultPc34Compat;

/* Maps a controller affordance to one of the source-owned C001..C006 ids. */
int DM1_V1_HostInputBridge_ControllerCommandPc34Compat(
    Dm1V1ControllerAffordancePc34Compat affordance);

/*
 * Normalizes pointer/touch coordinates through the existing F0358-equivalent
 * source-zone matcher, or maps keyboard/controller input to the same C001..C006
 * queue. Modal/inactive gameplay rejects before queue mutation.
 */
int DM1_V1_HostInputBridge_EnqueuePc34Compat(
    const Dm1V1HostInputPolicyPc34Compat* policy,
    const Dm1V1HostInputEventPc34Compat* event,
    struct Dm1V1InputCommandQueuePc34Compat* queue,
    Dm1V1HostInputResultPc34Compat* outResult);

const char* DM1_V1_HostInputBridge_SourceEvidencePc34Compat(void);

#ifdef __cplusplus
}
#endif

#endif

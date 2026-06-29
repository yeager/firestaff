#ifndef FIRESTAFF_THERON_V2_TOUCH_CONTROLLER_AFFORDANCE_H
#define FIRESTAFF_THERON_V2_TOUCH_CONTROLLER_AFFORDANCE_H

#include "dm1_v2_movement_command_adapter_pc34.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Theron V2 Phase 6 touch/controller affordances.
 *
 * Presentation-only input labels for Theron's Quest.  They map onto the
 * source-locked C001-C006 movement command ids used by the shared DM1-family
 * command queue, while preserving V1 input parity when V2 presentation is off.
 *
 * Source-lock anchors:
 *   THQUEST.ASM T520/T560/T600  Theron party placement, viewport, UI zones
 *   ReDMCSB DEFS.H:238-243      C001-C006 movement command ids
 *   ReDMCSB COMMAND.C:2045-2155 F0380 command queue dispatch
 *   ReDMCSB CLIKMENU.C:142      F0365 turn-party dispatch
 *   ReDMCSB CLIKMENU.C:180      F0366 move-party dispatch
 *   ReDMCSB GAMELOOP.C:164-219  V1 input wait/command queue loop
 */

typedef enum {
    THERON_V2_AFFORDANCE_NONE = 0,

    THERON_V2_AFFORDANCE_TOUCH_SWIPE_UP,
    THERON_V2_AFFORDANCE_TOUCH_SWIPE_DOWN,
    THERON_V2_AFFORDANCE_TOUCH_SWIPE_LEFT,
    THERON_V2_AFFORDANCE_TOUCH_SWIPE_RIGHT,
    THERON_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_LEFT,
    THERON_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_RIGHT,

    THERON_V2_AFFORDANCE_CONTROLLER_DPAD_UP,
    THERON_V2_AFFORDANCE_CONTROLLER_DPAD_DOWN,
    THERON_V2_AFFORDANCE_CONTROLLER_DPAD_LEFT,
    THERON_V2_AFFORDANCE_CONTROLLER_DPAD_RIGHT,
    THERON_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_UP,
    THERON_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_DOWN,
    THERON_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_LEFT,
    THERON_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_RIGHT,
    THERON_V2_AFFORDANCE_CONTROLLER_RIGHT_STICK_LEFT,
    THERON_V2_AFFORDANCE_CONTROLLER_RIGHT_STICK_RIGHT
} Theron_V2_TouchControllerAffordance;

typedef enum {
    THERON_V2_AFFORDANCE_INPUT_NONE = 0,
    THERON_V2_AFFORDANCE_INPUT_TOUCH = 1,
    THERON_V2_AFFORDANCE_INPUT_CONTROLLER = 2
} Theron_V2_TouchControllerInputKind;

typedef struct {
    int accepted;
    int v2Only;
    Theron_V2_TouchControllerInputKind inputKind;
    Theron_V2_TouchControllerAffordance affordance;
    DM1_V2_MovementCommand movementCommand;
    DM1_V2_MovementCommandRoute route;
} Theron_V2_TouchControllerAffordanceRoute;

DM1_V2_MovementCommand
theron_v2_touch_controller_affordance_movement_command(
    Theron_V2_TouchControllerAffordance affordance);

Theron_V2_TouchControllerInputKind
theron_v2_touch_controller_affordance_input_kind(
    Theron_V2_TouchControllerAffordance affordance);

Theron_V2_TouchControllerAffordanceRoute
theron_v2_touch_controller_affordance_route(
    int v2PresentationEnabled,
    Theron_V2_TouchControllerAffordance affordance);

const char *
theron_v2_touch_controller_affordance_name(
    Theron_V2_TouchControllerAffordance affordance);

const char *theron_v2_touch_controller_affordance_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_THERON_V2_TOUCH_CONTROLLER_AFFORDANCE_H */

#ifndef FIRESTAFF_DM1_V2_TOUCH_CONTROLLER_AFFORDANCE_PC34_H
#define FIRESTAFF_DM1_V2_TOUCH_CONTROLLER_AFFORDANCE_PC34_H

#include "dm1_v2_movement_command_adapter_pc34.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DM1_V2_AFFORDANCE_NONE = 0,
    DM1_V2_AFFORDANCE_TOUCH_SWIPE_UP,
    DM1_V2_AFFORDANCE_TOUCH_SWIPE_DOWN,
    DM1_V2_AFFORDANCE_TOUCH_SWIPE_LEFT,
    DM1_V2_AFFORDANCE_TOUCH_SWIPE_RIGHT,
    DM1_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_LEFT,
    DM1_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_RIGHT,
    DM1_V2_AFFORDANCE_CONTROLLER_DPAD_UP,
    DM1_V2_AFFORDANCE_CONTROLLER_DPAD_DOWN,
    DM1_V2_AFFORDANCE_CONTROLLER_DPAD_LEFT,
    DM1_V2_AFFORDANCE_CONTROLLER_DPAD_RIGHT,
    DM1_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_UP,
    DM1_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_DOWN,
    DM1_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_LEFT,
    DM1_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_RIGHT,
    DM1_V2_AFFORDANCE_CONTROLLER_RIGHT_STICK_LEFT,
    DM1_V2_AFFORDANCE_CONTROLLER_RIGHT_STICK_RIGHT,
    DM1_V2_AFFORDANCE_CONTROLLER_LEFT_BUMPER,
    DM1_V2_AFFORDANCE_CONTROLLER_RIGHT_BUMPER,
    /* Right stick up/down: right stick already handles turn left/right via
     * LEFT/RIGHT_STICK_{LEFT,RIGHT}. Adding UP/DOWN for forward/backward
     * gives full right-stick movement parity with left stick. */
    DM1_V2_AFFORDANCE_CONTROLLER_RIGHT_STICK_UP,
    DM1_V2_AFFORDANCE_CONTROLLER_RIGHT_STICK_DOWN,
    /* V2-only pinch-to-zoom: two-finger spread on minimap viewport */
    DM1_V2_AFFORDANCE_TOUCH_PINCH_ZOOM_IN,
    DM1_V2_AFFORDANCE_TOUCH_PINCH_ZOOM_OUT,
    /* V2-only double-tap: two quick taps in the lower dungeon viewport area
     * triggers quick forward movement. ReDMCSB GAMELOOP.C:164-219 has no
     * double-tap state; this is a V2-only gesture synthesized from SDL3
     * SDL_FINGERMULTIGESTURE events. */
    DM1_V2_AFFORDANCE_TOUCH_DOUBLE_TAP,
    /* V2-only long-press: hold on an object/creature/inscription triggers
     * examine mode. ReDMCSB GAMELOOP.C:164-219 has no held-touch state;
     * this is a V2-only gesture using SDL3 SDL_FINGERMOTION dwell detection. */
    DM1_V2_AFFORDANCE_TOUCH_LONG_PRESS
} DM1_V2_TouchControllerAffordance;

typedef enum {
    DM1_V2_AFFORDANCE_INPUT_NONE = 0,
    DM1_V2_AFFORDANCE_INPUT_TOUCH = 1,
    DM1_V2_AFFORDANCE_INPUT_CONTROLLER = 2
} DM1_V2_TouchControllerInputKind;

typedef struct {
    int accepted;
    int v2Only;
    DM1_V2_TouchControllerInputKind inputKind;
    DM1_V2_TouchControllerAffordance affordance;
    DM1_V2_MovementCommand movementCommand;
    DM1_V2_MovementCommandRoute route;
} DM1_V2_TouchControllerAffordanceRoute;

DM1_V2_MovementCommand dm1_v2_touch_controller_affordance_movement_command(
    DM1_V2_TouchControllerAffordance affordance);
DM1_V2_TouchControllerInputKind dm1_v2_touch_controller_affordance_input_kind(
    DM1_V2_TouchControllerAffordance affordance);
DM1_V2_TouchControllerAffordanceRoute dm1_v2_touch_controller_affordance_route(
    int v2PresentationEnabled,
    DM1_V2_TouchControllerAffordance affordance);
const char* dm1_v2_touch_controller_affordance_name(
    DM1_V2_TouchControllerAffordance affordance);
const char* dm1_v2_touch_controller_affordance_source_lock_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V2_TOUCH_CONTROLLER_AFFORDANCE_PC34_H */

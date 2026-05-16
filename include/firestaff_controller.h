
#ifndef FIRESTAFF_CONTROLLER_H
#define FIRESTAFF_CONTROLLER_H
#include <stdint.h>

/* Gamepad/controller support.
 * Maps controller buttons to DM1 actions.
 *
 * Default mapping:
 *   D-pad / Left stick: Movement (forward/back/turn)
 *   A/Cross:   Attack/Confirm
 *   B/Circle:  Back/Cancel
 *   X/Square:  Cast spell
 *   Y/Triangle: Inventory
 *   LB/L1:     Prev champion
 *   RB/R1:     Next champion
 *   Start:     Menu/Pause
 *   Select:    Map/Minimap toggle
 *   Left trigger:  Strafe left
 *   Right trigger: Strafe right */

typedef enum {
    CTRL_NONE = 0,
    CTRL_MOVE_FORWARD,
    CTRL_MOVE_BACK,
    CTRL_TURN_LEFT,
    CTRL_TURN_RIGHT,
    CTRL_STRAFE_LEFT,
    CTRL_STRAFE_RIGHT,
    CTRL_ATTACK,
    CTRL_SPELL,
    CTRL_INVENTORY,
    CTRL_PREV_CHAMPION,
    CTRL_NEXT_CHAMPION,
    CTRL_MENU,
    CTRL_MAP,
    CTRL_CONFIRM,
    CTRL_CANCEL,
} FS_ControllerAction;

typedef struct {
    int connected;
    int id;
    char name[64];
    /* Current state */
    int buttons[16];
    float left_x, left_y;
    float right_x, right_y;
    float left_trigger, right_trigger;
    /* Dead zone */
    float dead_zone;
} FS_Controller;

typedef struct {
    FS_Controller controllers[4];
    int count;
    int active;        /* primary controller index */
} FS_ControllerManager;

void fs_ctrl_init(FS_ControllerManager *mgr);
void fs_ctrl_poll(FS_ControllerManager *mgr);
FS_ControllerAction fs_ctrl_get_action(const FS_ControllerManager *mgr);
int fs_ctrl_is_connected(const FS_ControllerManager *mgr);
const char *fs_ctrl_action_name(FS_ControllerAction action);

#endif



#include "firestaff_controller.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

void fs_ctrl_init(FS_ControllerManager *mgr) {
    if (!mgr) return;
    memset(mgr, 0, sizeof(*mgr));
    for (int i = 0; i < 4; i++)
        mgr->controllers[i].dead_zone = 0.2f;
}

/* Poll controllers via SDL (when available) */
void fs_ctrl_poll(FS_ControllerManager *mgr) {
    (void)mgr;
    /* SDL_GameControllerUpdate() called externally.
     * This reads current state from SDL controller API.
     * Stub: actual SDL integration in firestaff_sdl_bridge.c */
}

FS_ControllerAction fs_ctrl_get_action(const FS_ControllerManager *mgr) {
    const FS_Controller *c;
    float dz;
    if (!mgr || mgr->count == 0) return CTRL_NONE;
    c = &mgr->controllers[mgr->active];
    if (!c->connected) return CTRL_NONE;
    dz = c->dead_zone;

    /* D-pad / left stick */
    if (c->left_y < -dz || c->buttons[0]) return CTRL_MOVE_FORWARD; /* up */
    if (c->left_y > dz || c->buttons[1])  return CTRL_MOVE_BACK;    /* down */
    if (c->left_x < -dz || c->buttons[2]) return CTRL_TURN_LEFT;    /* left */
    if (c->left_x > dz || c->buttons[3])  return CTRL_TURN_RIGHT;   /* right */

    /* Triggers */
    if (c->left_trigger > 0.5f)  return CTRL_STRAFE_LEFT;
    if (c->right_trigger > 0.5f) return CTRL_STRAFE_RIGHT;

    /* Face buttons */
    if (c->buttons[4])  return CTRL_ATTACK;         /* A/Cross */
    if (c->buttons[5])  return CTRL_CANCEL;          /* B/Circle */
    if (c->buttons[6])  return CTRL_SPELL;           /* X/Square */
    if (c->buttons[7])  return CTRL_INVENTORY;       /* Y/Triangle */
    if (c->buttons[8])  return CTRL_PREV_CHAMPION;   /* LB */
    if (c->buttons[9])  return CTRL_NEXT_CHAMPION;   /* RB */
    if (c->buttons[10]) return CTRL_MENU;            /* Start */
    if (c->buttons[11]) return CTRL_MAP;             /* Select */

    return CTRL_NONE;
}

int fs_ctrl_is_connected(const FS_ControllerManager *mgr) {
    return mgr && mgr->count > 0 && mgr->controllers[mgr->active].connected;
}

const char *fs_ctrl_action_name(FS_ControllerAction a) {
    switch (a) {
    case CTRL_MOVE_FORWARD: return "Forward";
    case CTRL_MOVE_BACK: return "Back";
    case CTRL_TURN_LEFT: return "Turn Left";
    case CTRL_TURN_RIGHT: return "Turn Right";
    case CTRL_STRAFE_LEFT: return "Strafe Left";
    case CTRL_STRAFE_RIGHT: return "Strafe Right";
    case CTRL_ATTACK: return "Attack";
    case CTRL_SPELL: return "Cast Spell";
    case CTRL_INVENTORY: return "Inventory";
    case CTRL_PREV_CHAMPION: return "Prev Champion";
    case CTRL_NEXT_CHAMPION: return "Next Champion";
    case CTRL_MENU: return "Menu";
    case CTRL_MAP: return "Map";
    case CTRL_CONFIRM: return "Confirm";
    case CTRL_CANCEL: return "Cancel";
    default: return "None";
    }
}



#include "firestaff_touch.h"
#include <stdio.h>

void fs_touch_init(FS_TouchConfig *cfg, int screen_w, int screen_h) {
    if (!cfg) return;
    cfg->screen_w = screen_w;
    cfg->screen_h = screen_h;
    /* Viewport occupies left 70% of screen, full height */
    cfg->viewport_x = 0;
    cfg->viewport_y = 0;
    cfg->viewport_w = screen_w * 70 / 100;
    cfg->viewport_h = screen_h;
    cfg->enabled = 1;
}

FS_TouchAction fs_touch_resolve(const FS_TouchConfig *cfg, int tx, int ty) {
    float rx, ry;
    if (!cfg || !cfg->enabled) return TOUCH_NONE;

    /* Normalize touch coordinates */
    rx = (float)tx / cfg->screen_w;
    ry = (float)ty / cfg->screen_h;

    /* Right panel: champion selection (rightmost 30%) */
    if (rx > 0.7f) {
        if (ry < 0.25f) return TOUCH_CHAMPION_1;
        if (ry < 0.50f) return TOUCH_CHAMPION_2;
        if (ry < 0.75f) return TOUCH_CHAMPION_3;
        return TOUCH_CHAMPION_4;
    }

    /* Viewport area: movement and actions */
    /* Top third: forward */
    if (ry < 0.33f) {
        if (rx < 0.25f) return TOUCH_TURN_LEFT;
        if (rx > 0.45f) return TOUCH_TURN_RIGHT;
        return TOUCH_MOVE_FORWARD;
    }

    /* Middle third: strafe or attack */
    if (ry < 0.66f) {
        if (rx < 0.2f) return TOUCH_STRAFE_LEFT;
        if (rx > 0.5f) return TOUCH_STRAFE_RIGHT;
        return TOUCH_ATTACK;
    }

    /* Bottom third: backward or spells */
    if (rx < 0.25f) return TOUCH_CAST_SPELL;
    if (rx > 0.45f) return TOUCH_INVENTORY;
    return TOUCH_MOVE_BACK;
}

const char *fs_touch_action_name(FS_TouchAction action) {
    switch (action) {
    case TOUCH_MOVE_FORWARD: return "Forward";
    case TOUCH_MOVE_BACK: return "Back";
    case TOUCH_TURN_LEFT: return "Turn Left";
    case TOUCH_TURN_RIGHT: return "Turn Right";
    case TOUCH_STRAFE_LEFT: return "Strafe Left";
    case TOUCH_STRAFE_RIGHT: return "Strafe Right";
    case TOUCH_ATTACK: return "Attack";
    case TOUCH_CAST_SPELL: return "Cast Spell";
    case TOUCH_INVENTORY: return "Inventory";
    case TOUCH_MAP: return "Map";
    case TOUCH_CHAMPION_1: return "Champion 1";
    case TOUCH_CHAMPION_2: return "Champion 2";
    case TOUCH_CHAMPION_3: return "Champion 3";
    case TOUCH_CHAMPION_4: return "Champion 4";
    default: return "None";
    }
}


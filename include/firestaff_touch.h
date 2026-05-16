
#ifndef FIRESTAFF_TOUCH_H
#define FIRESTAFF_TOUCH_H

/* Touchscreen/mouse click → game command translation.
 * Maps screen regions to DM1 actions. */

typedef enum {
    TOUCH_NONE = 0,
    TOUCH_MOVE_FORWARD,
    TOUCH_MOVE_BACK,
    TOUCH_TURN_LEFT,
    TOUCH_TURN_RIGHT,
    TOUCH_STRAFE_LEFT,
    TOUCH_STRAFE_RIGHT,
    TOUCH_ATTACK,
    TOUCH_CAST_SPELL,
    TOUCH_INVENTORY,
    TOUCH_MAP,
    TOUCH_CHAMPION_1,
    TOUCH_CHAMPION_2,
    TOUCH_CHAMPION_3,
    TOUCH_CHAMPION_4,
} FS_TouchAction;

typedef struct {
    int viewport_x, viewport_y, viewport_w, viewport_h;
    int screen_w, screen_h;
    int enabled;
} FS_TouchConfig;

void fs_touch_init(FS_TouchConfig *cfg, int screen_w, int screen_h);
FS_TouchAction fs_touch_resolve(const FS_TouchConfig *cfg, int touch_x, int touch_y);
const char *fs_touch_action_name(FS_TouchAction action);

#endif


/*
 * firestaff_touch.c — Touch-to-mouse abstraction layer for DM1 V1.
 *
 * Maps SDL touch/finger events to:
 *   - Synthesized mouse-button events → M11_GameView_HandlePointerButton()
 *   - Swipe gestures → fs_input_queue_push() → movement command pipeline
 *
 * All input converges on the same V1 handler; no duplicate routing.
 *
 * Gestures:
 *   Single tap          -> left click  (viewport/UI hit-test)
 *   Long press (>500ms) -> right click (inventory panel)
 *   Swipe up            -> move forward
 *   Swipe down          -> move backward
 *   Swipe left          -> turn left
 *   Swipe right         -> turn right
 *   Drag (movement >24px) -> no click fired
 *
 * Enabled via FIRESTAFF_TOUCH_ENABLED=1.
 *
 * Source lock: ReDMCSB COMMAND.C:375-405 active mouse tables define the
 * active in-game hit zones; firestaff_touch feeds normalised 320x200 screen
 * coordinates to the same click dispatcher via mouse-button synthesis.
 * Swipe commands use FS_InputQueue / fs_game_tick_v1() pipeline, identical
 * to arrow-key / WASD movement — preserving V1 cooldowns and collision gates.
 */

#include "firestaff_touch.h"
#include "firestaff_input.h"
#include <string.h>
#include <stdlib.h>

/* extern declarations — swipes are bridged to the FS_InputQueue */
extern void fs_input_queue_push(FS_InputQueue *q, FS_Command cmd, int p1, int p2);

/* ------------------------------------------------------------------
 * Swipe direction constants
 * ------------------------------------------------------------------ */

typedef enum {
    FS_SWIPE_NONE    = -1,
    FS_SWIPE_FORWARD =  0,   /* up on screen  — move forward    */
    FS_SWIPE_BACK    =  1,   /* down on screen — move backward  */
    FS_SWIPE_RIGHT   =  2,   /* right on screen — turn right    */
    FS_SWIPE_LEFT    =  3    /* left on screen  — turn left     */
} FS_SwipeDir;

/* Map swipe direction → FS_Command (matches firestaff_input.h order) */
static FS_Command fs_swipe_dir_to_command(FS_SwipeDir dir) {
    switch (dir) {
    case FS_SWIPE_FORWARD: return FS_CMD_MOVE_FORWARD;
    case FS_SWIPE_BACK:     return FS_CMD_MOVE_BACKWARD;
    case FS_SWIPE_LEFT:     return FS_CMD_TURN_LEFT;
    case FS_SWIPE_RIGHT:    return FS_CMD_TURN_RIGHT;
    default:               return FS_CMD_NONE;
    }
}

/* Bridge a recognised swipe command to the FS_InputQueue so it travels the
 * same fs_game_tick_v1() pipeline as keyboard/WASD movement commands. */
static void fs_touch_emit_swipe_command(FS_SwipeDir swipe) {
    FS_Command cmd = fs_swipe_dir_to_command(swipe);
    if (cmd == FS_CMD_NONE) return;
    /* Queue pointer is supplied by firestaff_game_loop.c via extern */
    extern FS_InputQueue *fs_g_input_queue_get(void);
    FS_InputQueue *q = fs_g_input_queue_get();
    if (q) fs_input_queue_push(q, cmd, 0, 0);
}

/* ------------------------------------------------------------------
 * Swipe gesture detection
 * ------------------------------------------------------------------ */

/* Minimum distance for a swipe to be recognised.
 * Threshold is in framebuffer (320x200) pixel units;
 * mapped from screen pixels via the letterbox normaliser. */
#ifndef FIRESTAFF_TOUCH_SWIPE_THRESHOLD_PX
#define FIRESTAFF_TOUCH_SWIPE_THRESHOLD_PX 40
#endif

/* Detect whether a finger path from (startX,startY) to (endX,endY)
 * constitutes a directional swipe.  Dominant axis wins.
 * Returns 1 and sets *outDirection on a valid swipe.
 * Returns 0 if the total travel is below the threshold. */
int firestaff_touch_detect_swipe(int startX, int startY,
                                int endX,   int endY,
                                int       *outDirection) {
    int dx      = endX - startX;
    int dy      = endY - startY;
    int absDx   = dx < 0 ? -dx : dx;
    int absDy   = dy < 0 ? -dy : dy;
    FS_SwipeDir dir = FS_SWIPE_NONE;

    if (outDirection) *outDirection = -1;

    if (absDx < FIRESTAFF_TOUCH_SWIPE_THRESHOLD_PX &&
        absDy < FIRESTAFF_TOUCH_SWIPE_THRESHOLD_PX) {
        return 0; /* Too small — not a swipe */
    }

    if (absDx > absDy) {
        dir = (dx > 0) ? FS_SWIPE_RIGHT : FS_SWIPE_LEFT;
    } else {
        dir = (dy > 0) ? FS_SWIPE_BACK : FS_SWIPE_FORWARD;
    }

    if (outDirection) *outDirection = (int)dir;
    return 1;
}

/* Convenience: given start/end finger positions in framebuffer space,
 * detect a swipe and immediately emit its command onto the input queue. */
void firestaff_touch_emit_swipe(int startX, int startY,
                                int endX,   int endY) {
    int dir = -1;
    if (firestaff_touch_detect_swipe(startX, startY, endX, endY, &dir)) {
        fs_touch_emit_swipe_command((FS_SwipeDir)dir);
    }
}

/* ------------------------------------------------------------------
 * Tap / long-press / drag detection helpers
 * ------------------------------------------------------------------ */

/* tap_tolerance_px — movement within this many fb pixels is still a tap */
#ifndef FIRESTAFF_TOUCH_TAP_TOLERANCE_PX
#define FIRESTAFF_TOUCH_TAP_TOLERANCE_PX 24
#endif

/* long_press_ms — held this long without moving triggers right-click */
#ifndef FIRESTAFF_TOUCH_LONG_PRESS_MS
#define FIRESTAFF_TOUCH_LONG_PRESS_MS 500
#endif

/* Returns 1 if (x,y) is within FIRESTAFF_TOUCH_TAP_TOLERANCE_PX
 * of (ox,oy) in framebuffer space. */
static int fs_touch_within_tap_tolerance(int x, int y, int ox, int oy) {
    int dx = x - ox; if (dx < 0) dx = -dx;
    int dy = y - oy; if (dy < 0) dy = -dy;
    return dx <= FIRESTAFF_TOUCH_TAP_TOLERANCE_PX &&
           dy <= FIRESTAFF_TOUCH_TAP_TOLERANCE_PX;
}

/* Re-export the movement tolerance so callers can easily check drags. */
int fs_touch_tap_tolerance_px(void) {
    return FIRESTAFF_TOUCH_TAP_TOLERANCE_PX;
}

/* Re-export long-press threshold so callers can use it in timeout checks. */
int fs_touch_long_press_ms(void) {
    return FIRESTAFF_TOUCH_LONG_PRESS_MS;
}

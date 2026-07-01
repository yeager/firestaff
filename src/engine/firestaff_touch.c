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
 *   Edge-strafe zones   -> strafe (V2 only, left/right 20% of screen)
 *   Drag (movement >24px) -> no click fired
 *
 * V2 presentation edge-strafe: DM1_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_LEFT/RIGHT
 * (ReDMCSB DEFS.H:238-243 C005/C006 strafe commands) are emitted when the
 * touch begins in the left/right edge zone. V1 path ignores edge-zones.
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

static FS_Command fs_touch_command_from_runtime_code(int commandCode) {
    switch (commandCode) {
    case FS_CMD_MOVE_FORWARD:
    case FS_CMD_MOVE_BACKWARD:
    case FS_CMD_TURN_LEFT:
    case FS_CMD_TURN_RIGHT:
    case FS_CMD_STRAFE_LEFT:
    case FS_CMD_STRAFE_RIGHT:
        return (FS_Command)commandCode;
    default:
        return FS_CMD_NONE;
    }
}

/* Edge-zone threshold as fraction of framebuffer width.
 * A touch that begins within this fraction of the left or right edge
 * is classified as an edge-strafe attempt (V2-only affordance).
 * ReDMCSB DEFS.H:238-243 maps C005/C006 to left/right strafe. */
#ifndef FIRESTAFF_TOUCH_EDGE_ZONE_FRAC
#define FIRESTAFF_TOUCH_EDGE_ZONE_FRAC  0.20f   /* 20% from each edge */
#endif

/* Returns 1 if x is in the left edge zone at framebuffer width fbW. */
int fs_touch_in_left_edge(int x, int fbW) {
    return x < (int)(fbW * FIRESTAFF_TOUCH_EDGE_ZONE_FRAC);
}

/* Returns 1 if x is in the right edge zone at framebuffer width fbW. */
int fs_touch_in_right_edge(int x, int fbW) {
    return x >= (int)(fbW * (1.0f - FIRESTAFF_TOUCH_EDGE_ZONE_FRAC));
}

static int fs_touch_emit_runtime_command(
    const FirestaffRuntimeGestureNavResult* result) {

    FS_Command cmd;
    FS_InputQueue *q;

    if (!result) return 0;
    cmd = fs_touch_command_from_runtime_code(result->commandCode);
    if (cmd == FS_CMD_NONE) return 0;

    /* Queue pointer is supplied by firestaff_game_loop.c via extern. */
    extern FS_InputQueue *fs_g_input_queue_get(void);
    q = fs_g_input_queue_get();
    if (!q) return 0;
    fs_input_queue_push(q, cmd, 0, 0);
    return 1;
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
    FirestaffRuntimeGestureNavPolicy policy;
    memset(&policy, 0, sizeof(policy));
    policy.accessibilityTouchEnabled = 1;
    (void)firestaff_touch_emit_swipe_runtime(startX, startY, endX, endY,
                                             &policy, NULL);
}

int firestaff_touch_evaluate_swipe_runtime(
    int startX, int startY,
    int endX, int endY,
    const FirestaffRuntimeGestureNavPolicy* policy,
    FirestaffRuntimeGestureNavResult* outResult) {

    FirestaffRuntimeGestureNavEvent event;
    FirestaffRuntimeGestureNavResult localResult;

    memset(&event, 0, sizeof(event));
    memset(&localResult, 0, sizeof(localResult));

    event.kind = FIRESTAFF_RUNTIME_GESTURE_NAV_KIND_SWIPE;
    event.startX = startX;
    event.startY = startY;
    event.endX = endX;
    event.endY = endY;
    event.fbW = 320;
    event.thresholdPx = FIRESTAFF_TOUCH_SWIPE_THRESHOLD_PX;
    event.tapTolerancePx = FIRESTAFF_TOUCH_TAP_TOLERANCE_PX;

    /* ReDMCSB COMMAND.C F0380 consumes the same C001..C004 movement
     * commands that keyboard input queues; the runtime gesture gate
     * decides whether a finger path may enqueue one of those commands. */
    if (!FirestaffRuntimeGestureNav_Evaluate(
            &event, policy, outResult ? outResult : &localResult)) {
        return 0;
    }

    if (!outResult) {
        outResult = &localResult;
    }

    return fs_touch_command_from_runtime_code(outResult->commandCode) != FS_CMD_NONE;
}

int firestaff_touch_emit_swipe_runtime(
    int startX, int startY,
    int endX, int endY,
    const FirestaffRuntimeGestureNavPolicy* policy,
    FirestaffRuntimeGestureNavResult* outResult) {

    FirestaffRuntimeGestureNavResult localResult;
    FirestaffRuntimeGestureNavResult* result = outResult ? outResult : &localResult;

    memset(&localResult, 0, sizeof(localResult));
    if (!firestaff_touch_evaluate_swipe_runtime(startX, startY, endX, endY,
                                                policy, result)) {
        return 0;
    }
    return fs_touch_emit_runtime_command(result);
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
static int __attribute__((unused)) fs_touch_within_tap_tolerance (int x, int y, int ox, int oy) {
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

/* ── Edge-strafe helpers ───────────────────────────────────────────── */

/* Emit a strafe command when the touch starts in an edge zone.
 * V2-only: DM1_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_LEFT/RIGHT are forwarded
 * to fs_input_queue as FS_CMD_STRAFE_LEFT/RIGHT (source-locked C005/C006).
 * No-op when the global queue is unavailable. */
void fs_touch_emit_edge_strafe(int startX, int fbW) {
    FirestaffRuntimeGestureNavPolicy policy;
    memset(&policy, 0, sizeof(policy));
    policy.accessibilityTouchEnabled = 1;
    policy.v2PresentationEnabled = 1;
    policy.v1ParityPreserve = 0;
    (void)fs_touch_emit_edge_strafe_runtime(startX, fbW, &policy, NULL);
}

int fs_touch_emit_edge_strafe_runtime(
    int startX,
    int fbW,
    const FirestaffRuntimeGestureNavPolicy* policy,
    FirestaffRuntimeGestureNavResult* outResult) {

    FirestaffRuntimeGestureNavEvent event;
    FirestaffRuntimeGestureNavResult localResult;
    FirestaffRuntimeGestureNavResult* result = outResult ? outResult : &localResult;

    memset(&event, 0, sizeof(event));
    memset(&localResult, 0, sizeof(localResult));

    event.kind = FIRESTAFF_RUNTIME_GESTURE_NAV_KIND_EDGE_STRAFE;
    event.startX = startX;
    event.fbW = fbW;
    event.thresholdPx = FIRESTAFF_TOUCH_SWIPE_THRESHOLD_PX;
    event.tapTolerancePx = FIRESTAFF_TOUCH_TAP_TOLERANCE_PX;

    if (!FirestaffRuntimeGestureNav_Evaluate(&event, policy, result)) {
        return 0;
    }
    return fs_touch_emit_runtime_command(result);
}

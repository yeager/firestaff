/*
 * firestaff_touch.c — Touch-to-mouse abstraction layer for DM1 V1.
 *
 * Maps SDL touch/finger events to synthesized mouse-button events
 * that flow through the existing M11_GameView_HandlePointerButton()
 * mouse-routing path.  No duplicate click logic — all clicks
 * (touch or physical) converge on the same V1 handler.
 *
 * Gestures:
 *   Single tap        -> left click  (button 1)
 *   Long press (>500ms) -> right click (button 3, inventory panel)
 *   Movement beyond tolerance -> drag (no click fired)
 *
 * Enabled via FIRESTAFF_TOUCH_ENABLED=1.
 */

#include "firestaff_touch.h"
#include <string.h>
#include <stdlib.h>

/* ------------------------------------------------------------------
 * Per-finger gesture state
 * ------------------------------------------------------------------ */

typedef struct {
    int16_t   id;                 /* finger / point id                      */
    int16_t   active;             /* 1 while tracked (down or recently up)  */
    int16_t   down;               /* 1 when finger is currently held        */
    int16_t   fired_right;        /* guard: don't re-fire right-click       */
    uint

/* ── Swipe gesture detection ──
 * Track finger movement from down to up.  If distance exceeds
 * threshold and direction is predominantly one axis, emit a
 * swipe command mapped to DM1 movement arrows.
 *
 * FIRESTAFF_TOUCH_SWIPE_THRESHOLD_PX: minimum distance for a swipe.
 * Swipe up = move forward, down = move back, left = turn left,
 * right = turn right. */

#ifndef FIRESTAFF_TOUCH_SWIPE_THRESHOLD_PX
#define FIRESTAFF_TOUCH_SWIPE_THRESHOLD_PX 40
#endif

int firestaff_touch_detect_swipe(int startX, int startY,
                                  int endX, int endY,
                                  int* outDirection) {
    int dx = endX - startX;
    int dy = endY - startY;
    int absDx = dx < 0 ? -dx : dx;
    int absDy = dy < 0 ? -dy : dy;

    if (outDirection) *outDirection = -1;

    if (absDx < FIRESTAFF_TOUCH_SWIPE_THRESHOLD_PX &&
        absDy < FIRESTAFF_TOUCH_SWIPE_THRESHOLD_PX) {
        return 0; /* Too short — not a swipe */
    }

    if (absDx > absDy) {
        /* Horizontal swipe */
        if (outDirection) *outDirection = (dx > 0) ? 2 : 3; /* 2=right, 3=left */
    } else {
        /* Vertical swipe */
        if (outDirection) *outDirection = (dy > 0) ? 1 : 0; /* 0=up/forward, 1=down/back */
    }
    return 1;
}

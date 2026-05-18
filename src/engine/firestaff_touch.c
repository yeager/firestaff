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

#ifndef FIRESTAFF_TOUCH_H
#define FIRESTAFF_TOUCH_H

#include <stdint.h>

/* Maximum number of simultaneous touch points to track */
#define FIRESTAFF_TOUCH_MAX_FINGERS 5

/* Long-press threshold in milliseconds — triggers a right-click */
#define FIRESTAFF_TOUCH_LONG_PRESS_MS 500

/* Tap movement tolerance (pixels at framebuffer resolution).
 * A touch that moves beyond this while down is cancelled
 * (treated as a drag, not a tap). */
#define FIRESTAFF_TOUCH_TAP_TOLERANCE_PX 24

/* Per-finger touch state */
typedef struct {
    int16_t id;             /* touch point / finger id            */
    int16_t active;         /* 1 while finger is down             */
    int16_t down;           /* 1 = currently held down            */
    uint32_t down_time_ms;  /* SDL timestamp when finger went down */
    int32_t down_fb_x;      /* framebuffer x at touch-down        */
    int32_t down_fb_y;      /* framebuffer y at touch-down        */
    uint32_t last_event_ms; /* timestamp of most recent event     */
} FS_TouchFinger;

/* Global touch state — maintained per game loop iteration */
typedef struct {
    int enabled;                  /* 1 when FIRESTAFF_TOUCH_ENABLED is set  */
    int screen_w;                 /* window width in pixels               */
    int screen_h;                 /* window height in pixels              */
    FS_TouchFinger fingers[FIRESTAFF_TOUCH_MAX_FINGERS];
} FS_TouchState;

/* Initialise / reset touch state for a frame.
 * Must be called once at startup with the actual window size. */
void fs_touch_init(FS_TouchState *state, int screen_w, int screen_h);

/* Process a single SDL finger event and push a corresponding
 * mouse button event (LEFT or RIGHT) into the SDL event queue
 * when a tap / long-press gesture is recognised.
 *
 * @param state     pointer to the global FS_TouchState
 * @param ev        pointer to the SDL_TouchFingerEvent (SDL3) or
 *                  SDL_FingerEvent (SDL2)
 * @param fb_w      framebuffer / game logical width
 * @param fb_h      framebuffer / game logical height
 */
void fs_touch_process_event(FS_TouchState *state,
                            const void *ev,
                            int fb_w,
                            int fb_h);

/* Called at the end of every pump / frame to fire delayed
 * right-clicks from fingers that crossed the long-press
 * threshold while idle (no motion). */
void fs_touch_process_held(FS_TouchState *state,
                           uint32_t now_ms,
                           int fb_w,
                           int fb_h);

/* Reset the touch state so held fingers are cleared — call
 * after a full reset of the game view or when switching
 * menus. */
void fs_touch_reset(FS_TouchState *state);

#endif /* FIRESTAFF_TOUCH_H */

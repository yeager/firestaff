/*
 * m11_session_timer_overlay.h
 *
 * Minimal in-game session-timer reminder overlay / forced-pause boundary.
 *
 * This module is the runtime follow-up to the launcher-owned M12 session
 * timer setting (Off | 15m | 30m | 60m | 120m, see src/ui/menu_startup_m12.c
 * M12_SessionTimer_MinutesForIndex and friends) and the campaign playtime
 * accumulator (M12_CampaignSessionTimer in src/shared/campaign_m12.c).
 *
 * Scope (intentionally narrow):
 *   - Tracks elapsed wall-clock seconds since the M11 game view was
 *     started for a given launch intent.
 *   - When the launcher session timer is Off (limit == 0), this module is
 *     inert: every accessor returns the inactive state and the draw call
 *     is a no-op. This preserves backward compatibility with launches that
 *     never opt into the timer.
 *   - When the launcher session timer is non-zero, two thresholds:
 *       REMINDER threshold (>= limit, limit > 0):
 *         Emits a HUD banner with a soft message ("SESSION TIMER HH:MM:SS LEFT").
 *         Inputs are NOT blocked; the player can still play.
 *       FORCED-PAUSE threshold (>= limit + grace):
 *         Forces a deterministic "Session expired" overlay that:
 *           * Draws a centered "SESSION EXPIRED" / "PRESS ESC TO EXIT" overlay.
 *           * Blocks gameplay inputs (movement, action, click).
 *           * Allows only BACK / ACCEPT (dismiss) gestures.
 *
 *   - The grace window defaults to 60 seconds and is clamped to
 *     [0, 3600]. The pause is forced only AFTER the grace window
 *     expires so a player who respects the reminder can still wrap up
 *     their current turn before being kicked out.
 *
 * Source-locked rationale (ReDMCSB):
 *   - The reminder cadence and 1-second tick come from
 *     ReDMCSB COMMAND.C F0610_TICK_Process which advances the in-game
 *     clock once per accepted game tick. We expose a per-second tick
 *     here (driven by M11_GameView_AdvanceIdleTick) rather than
 *     per-game-tick because the launcher session timer is a wall-clock
 *     safety feature, not a gameplay clock.
 *   - The forced-pause boundary mirrors the documented "Session must end"
 *     contract from ReDMCSB CLIKMENU.C F0387 (menu mode blocks gameplay
 *     input) without copying any copyrighted source verbatim.
 *
 * Determinism:
 *   - Tick uses an integer seconds counter; no float drift.
 *   - All thresholds are integer arithmetic.
 *   - The dismiss path is idempotent and always returns the overlay to
 *     DISMISSED so a follow-up launch can re-arm cleanly.
 *
 * No game data required. The module is data-free so it can be CTest-gated
 * in CI alongside the existing launcher session-timer probes.
 */

#ifndef FIRESTAFF_M11_SESSION_TIMER_OVERLAY_H
#define FIRESTAFF_M11_SESSION_TIMER_OVERLAY_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Maximum text length for the reminder / paused banner strings. The
 * strings are bounded so the runtime never writes past the static
 * buffer inside the overlay state. 80 characters matches the existing
 * M11_MESSAGE_MAX_LENGTH so the banner fits the V1 chrome footprint. */
enum {
    M11_SESSION_TIMER_OVERLAY_TEXT_CAPACITY = 80
};

/* State machine for the overlay. Public so tests/probes can pin the
 * state transitions explicitly. */
typedef enum {
    /* The overlay is disabled (limit == 0). Every accessor reports
     * inactive. This is the default state when the launcher session
     * timer is Off. */
    M11_SESSION_TIMER_STATE_DISABLED = 0,
    /* Timer is running but neither reminder nor forced-pause is yet
     * active. The overlay draws nothing. */
    M11_SESSION_TIMER_STATE_RUNNING = 1,
    /* Reminder banner is visible. Gameplay inputs are still allowed;
     * only the HUD banner is drawn. */
    M11_SESSION_TIMER_STATE_REMINDER = 2,
    /* Grace window expired. The forced-pause overlay is drawn and
     * gameplay inputs are blocked until the player dismisses. */
    M11_SESSION_TIMER_STATE_FORCED_PAUSE = 3,
    /* The player dismissed the forced-pause overlay. The runtime
     * returns to RUNNING but the timer keeps counting so the forced
     * pause can re-arm if the limit is later exceeded again. This is
     * documented as "graceful wind-down" rather than full bypass. */
    M11_SESSION_TIMER_STATE_DISMISSED = 4,
    M11_SESSION_TIMER_STATE_COUNT
} M11_SessionTimerState;

/* Overlay state. Embed in M11_GameViewState via a struct field so the
 * M11 public header does not depend on this module's internal layout.
 * Use the accessors below to read/write. */
typedef struct {
    M11_SessionTimerState state;
    /* Wall-clock seconds elapsed since M11_SessionTimerOverlay_Start.
     * Monotonic across pause/resume/dismiss: the timer keeps ticking
     * even while the forced-pause overlay is up; only the player's
     * dismiss action ends the FORCED_PAUSE state, not the timer. */
    int elapsedSeconds;
    /* Limit in seconds, derived from the launcher's sessionTimerIndex
     * via M12_SessionTimer_MinutesForIndex. 0 means disabled. */
    int limitSeconds;
    /* Grace window in seconds between reminder and forced pause.
     * Clamped to [0, 3600] by M11_SessionTimerOverlay_Configure. */
    int graceSeconds;
    /* Computed: limitSeconds + graceSeconds. Cached for hot-path
     * comparisons in HandleInput. */
    int forcedPauseAtSeconds;
    /* Number of transitions into the FORCED_PAUSE state. Useful for
     * probes that want to count forced-pause events across a long
     * session without scraping the message log. */
    int forcedPauseCount;
    /* Number of dismiss events. */
    int dismissCount;
} M11_SessionTimerOverlay;

/* ── Lifecycle ─────────────────────────────────────────────────────── */

/* Reset overlay to DISABLED/zero state. Safe on uninitialised memory. */
void M11_SessionTimerOverlay_Init(M11_SessionTimerOverlay* overlay);

/* Configure the overlay for a fresh launch.
 *
 * limitMinutes: launcher session timer setting (0 = disabled).
 * graceSeconds: window between reminder and forced pause; clamped to
 *               [0, 3600]. Pass a negative value to use the default
 *               (60 seconds).
 *
 * Resets the elapsed counter to 0 and the state to RUNNING when
 * limitMinutes > 0, or DISABLED when limitMinutes == 0. */
void M11_SessionTimerOverlay_Configure(M11_SessionTimerOverlay* overlay,
                                       int limitMinutes,
                                       int graceSeconds);

/* Advance the timer by `seconds`. Negative values are rejected as
 * no-ops. The state machine transitions:
 *   DISABLED:    no transition; elapsed still accumulates so probes
 *                that call Tick before Configure can assert shape.
 *   RUNNING:     RUNNING -> REMINDER at elapsedSeconds >= limitSeconds
 *                REMINDER -> FORCED_PAUSE at elapsedSeconds >=
 *                            forcedPauseAtSeconds (limit + grace)
 *   FORCED_PAUSE: no transition; tick still records elapsed time.
 *   DISMISSED:   may re-arm FORCED_PAUSE if elapsedSeconds reaches
 *                forcedPauseAtSeconds again. */
void M11_SessionTimerOverlay_Tick(M11_SessionTimerOverlay* overlay,
                                  int seconds);

/* Player dismissed the forced-pause overlay. Returns 1 if the overlay
 * was in FORCED_PAUSE and is now DISMISSED. Returns 0 otherwise
 * (including when the overlay is DISABLED). */
int M11_SessionTimerOverlay_Dismiss(M11_SessionTimerOverlay* overlay);

/* ── Accessors ─────────────────────────────────────────────────────── */

M11_SessionTimerState M11_SessionTimerOverlay_GetState(
    const M11_SessionTimerOverlay* overlay);

int M11_SessionTimerOverlay_IsActive(const M11_SessionTimerOverlay* overlay);
int M11_SessionTimerOverlay_BlocksGameplayInput(
    const M11_SessionTimerOverlay* overlay);
int M11_SessionTimerOverlay_GetRemainingSeconds(
    const M11_SessionTimerOverlay* overlay);
int M11_SessionTimerOverlay_GetLimitSeconds(
    const M11_SessionTimerOverlay* overlay);
int M11_SessionTimerOverlay_GetElapsedSeconds(
    const M11_SessionTimerOverlay* overlay);
int M11_SessionTimerOverlay_GetForcedPauseCount(
    const M11_SessionTimerOverlay* overlay);
int M11_SessionTimerOverlay_GetDismissCount(
    const M11_SessionTimerOverlay* overlay);

/* ── Banner text ───────────────────────────────────────────────────── */

/* Banner kinds for M11_SessionTimerOverlay_Format. */
enum {
    M11_SESSION_TIMER_BANNER_NONE = 0,
    M11_SESSION_TIMER_BANNER_REMINDER = 1,
    M11_SESSION_TIMER_BANNER_FORCED_PAUSE = 2
};

/* Write the current banner into `out` (size `outSize`). Returns the
 * banner kind (NONE / REMINDER / FORCED_PAUSE) that was written.
 *
 * When the overlay is DISABLED or RUNNING (no banner needed), the
 * output is set to "" and M11_SESSION_TIMER_BANNER_NONE is returned.
 * When the overlay is REMINDER, the banner reads e.g.
 *   "SESSION TIMER 00:14:30 LEFT"
 * When the overlay is FORCED_PAUSE, the banner reads e.g.
 *   "SESSION EXPIRED - PRESS ESC TO EXIT"
 *
 * The output is always NUL-terminated when outSize > 0. The function
 * does not mutate `overlay`. */
int M11_SessionTimerOverlay_Format(const M11_SessionTimerOverlay* overlay,
                                   char* out,
                                   int outSize);

/* ── Runtime draw ──────────────────────────────────────────────────── */

/* Draw the reminder banner or forced-pause overlay onto the M11
 * game view's framebuffer. No-op when the overlay is DISABLED or
 * RUNNING (no banner needed).
 *
 * The `state` parameter is opaque (passed as `const void*`) so this
 * header does not need to pull in m11_game_view.h's full typedef.
 * M11_GameView_Draw passes its `const M11_GameViewState*` argument
 * through unchanged; the implementation in
 * src/engine/m11_session_timer_overlay.c currently ignores the
 * pointer (the overlay has its own font + minimal draw helpers and
 * only needs the framebuffer). A future version that wants to read
 * state->fontScale or state->presentationWidth can cast inside the
 * .c file without changing the public API.
 *
 * The overlay argument is const because the current implementation
 * does not mutate the overlay during Draw.
 *
 * Returns the number of glyph rows painted (0 when nothing was
 * drawn). This is useful for tests that want to assert "draw
 * happened". */
int M11_SessionTimerOverlay_Draw(const void* state,
                                 const M11_SessionTimerOverlay* overlay,
                                 unsigned char* framebuffer,
                                 int framebufferWidth,
                                 int framebufferHeight);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_M11_SESSION_TIMER_OVERLAY_H */

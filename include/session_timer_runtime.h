/*
 * Session Timer Runtime
 * ---------------------
 * Minimal deterministic in-game handoff boundary for the launcher-owned
 * Session Timer setting (see menu_startup_m12.h M12_SessionTimer_* and
 * M12_CampaignSessionTimer in campaign_m12.h).
 *
 * The M12 launcher persists a session-timer limit (Off / 15m / 30m / 60m /
 * 120m).  Until this module landed there was no in-game hook: the value
 * lived in M12_StartupMenu_SessionTimerRemainingSeconds() only, and
 * gameplay ran without consulting it.  This module owns the *runtime*
 * state machine that consults the limit during gameplay and surfaces two
 * deterministic handoff events:
 *
 *   - REMINDER_DUE:  triggered exactly once per reminder window when
 *                    remaining time falls below the reminder threshold.
 *                    The M11 main loop reads this and shows a one-line
 *                    overlay (or a localized dialog plaque).  Acknowledge
 *                    clears the pending flag and re-arms the next window.
 *
 *   - FORCED_PAUSE:  latched when the limit is reached.  The M11 main
 *                    loop reads this to surface a return-to-menu confirm
 *                    dialog similar to the existing quit-guard flow.
 *                    Further ticks while in FORCED_PAUSE are no-ops.
 *
 * Scope:
 *   - Data-free, deterministic, headless.  No real assets, no SDL,
 *     no GRAPHICS.DAT/DUNGEON.DAT load.  All state is plain integers.
 *   - Does not own the M12 settings row or the campaign slot flush
 *     path.  Those still live in menu_startup_m12.c and campaign_m12.c.
 *   - Does not claim gameplay parity with any original DM/CSB/DM2/Nexus/
 *     Theron title-screen timer (the originals had no such UI on PC 3.4
 *     either).  This is a Firestaff-specific runtime/UX boundary.
 *
 * Source-lock:
 *   - M12 launcher rows: menu_startup_m12.c M12_SETTINGS_ROW_SESSION_TIMER
 *     + M12_SessionTimer_* + M12_StartupMenu_SessionTimerLimitMinutes /
 *     M12_StartupMenu_SessionTimerRemainingSeconds.
 *   - M12 Campaign timer: campaign_m12.h M12_CampaignSessionTimer_*.
 *   - The REMINDER threshold (default 5 minutes) and the reminder window
 *     (default 5 minutes) are documented constants, not ReDMCSB-derived.
 *
 * Companion tests/probes:
 *   - tests/test_session_timer_runtime.c          (CTest unit)
 *   - probes/runtime/firestaff_session_timer_runtime_probe.c
 *     (CTest integration: M11 <-> M12 hand-off smoke)
 */

#ifndef FIRESTAFF_SESSION_TIMER_RUNTIME_H
#define FIRESTAFF_SESSION_TIMER_RUNTIME_H

#ifdef __cplusplus
extern "C" {
#endif

/* Default reminder threshold: surface the in-game reminder when fewer
 * than this many seconds remain.  Documented behavior, not a ReDMCSB
 * constant.  Equal to 5 minutes. */
#define SESSION_TIMER_RUNTIME_DEFAULT_REMINDER_SECONDS (5 * 60)

/* Default reminder re-arm window: once the user acknowledges, the next
 * reminder fires after this many additional seconds (or at FORCED_PAUSE,
 * whichever comes first).  Documented behavior.  Equal to 5 minutes. */
#define SESSION_TIMER_RUNTIME_DEFAULT_REMINDER_WINDOW_SECONDS (5 * 60)

/* Maximum text buffer for SessionTimerRuntime_FormatRemaining(),
 * including the terminating NUL.  Format is "HH:MM:SS" (8 chars + NUL). */
#define SESSION_TIMER_RUNTIME_TEXT_CAPACITY 16

/* In-game runtime handoff events reported by SessionTimerRuntime_Poll(). */
typedef enum {
    /* Limit is 0 (Off) or no event has fired yet. */
    SESSION_TIMER_RUNTIME_EVENT_RUNNING = 0,
    /* Remaining time just crossed the reminder threshold and the overlay
     * has not been acknowledged yet.  Surfaces once per reminder window. */
    SESSION_TIMER_RUNTIME_EVENT_REMINDER_DUE = 1,
    /* Elapsed time reached or exceeded the limit.  Latched.  The M11
     * main loop turns this into a forced-pause confirm dialog. */
    SESSION_TIMER_RUNTIME_EVENT_FORCED_PAUSE = 2
} SessionTimerRuntimeEvent;

typedef struct {
    /* Limit in minutes.  <= 0 means "Off" -- no events ever fire. */
    int limitMinutes;
    /* Limit converted to seconds (cached for fast comparison).  0 if Off. */
    int limitSeconds;
    /* Wall-clock seconds elapsed since init.  Accumulated via Tick(). */
    int elapsedSeconds;
    /* Remaining seconds; -1 when the limit is Off. */
    int remainingSeconds;
    /* Threshold at which the next reminder event fires. */
    int reminderThresholdSeconds;
    /* Re-arm window applied after SessionTimerRuntime_Acknowledge(). */
    int reminderWindowSeconds;
    /* Last value at which REMINDER_DUE was reported (monotonic).  -1
     * means "no reminder has fired yet".  After acknowledge, this is
     * reset so the next reminder re-arms relative to the current time
     * plus the reminder window. */
    int lastReminderFiredAtElapsed;
    /* Pending reminder: 1 while REMINDER_DUE has been reported but not
     * acknowledged.  Cleared by SessionTimerRuntime_Acknowledge(). */
    int reminderPending;
    /* Forced-pause latch: 1 once elapsed >= limit.  Sticky until Reset(). */
    int forcedPauseLatched;
} SessionTimerRuntime;

/* ── Lifecycle ─────────────────────────────────────────────────── */

/* Reset all state.  limitMinutes <= 0 selects the "Off" mode; the limit
 * is then treated as 0 seconds and no event ever fires. */
void SessionTimerRuntime_Init(SessionTimerRuntime* rt, int limitMinutes);

/* Reset the elapsed/pending/latched fields while keeping the configured
 * limit and reminder thresholds.  Use when returning to the launcher
 * and immediately re-launching the same game. */
void SessionTimerRuntime_Reset(SessionTimerRuntime* rt);

/* ── Time accumulation ─────────────────────────────────────────── */

/* Add elapsed wall-clock seconds.  Negative inputs are clamped to 0.
 * No-op once the forced-pause latch is set. */
void SessionTimerRuntime_Tick(SessionTimerRuntime* rt, int seconds);

/* ── Event polling ─────────────────────────────────────────────── */

/* Returns the most recent event state.  REMINDER_DUE is reported at
 * most once per reminder window (latched in reminderPending until
 * acknowledged).  FORCED_PAUSE is reported as long as the latch is set;
 * the caller decides whether to clear or honor the boundary. */
SessionTimerRuntimeEvent SessionTimerRuntime_Poll(
    const SessionTimerRuntime* rt);

/* Clear the pending reminder latch and re-arm the next reminder to
 * fire (remainingSeconds + reminderWindowSeconds) seconds from now.
 * No-op when no reminder is pending. */
void SessionTimerRuntime_Acknowledge(SessionTimerRuntime* rt);

/* Force-clear the forced-pause latch so the user can resume gameplay
 * after acknowledging the boundary.  Used by the M11 main loop when
 * the user picks "Continue" / "Return to menu" in the confirm dialog. */
void SessionTimerRuntime_ClearForcedPause(SessionTimerRuntime* rt);

/* ── Query helpers ─────────────────────────────────────────────── */

/* Remaining seconds, or -1 when the timer is Off.  Matches
 * M12_StartupMenu_SessionTimerRemainingSeconds semantics. */
int SessionTimerRuntime_RemainingSeconds(const SessionTimerRuntime* rt);

/* Format the remaining seconds as "HH:MM:SS" into outBuf.  Writes
 * "Off" when the limit is 0.  Returns the number of bytes written
 * (excluding NUL).  outBuf must be at least SESSION_TIMER_RUNTIME_TEXT_CAPACITY
 * bytes. */
int SessionTimerRuntime_FormatRemaining(const SessionTimerRuntime* rt,
                                       char* outBuf,
                                       int outBufSize);

/* ── Defaults exposed for testing ──────────────────────────────── */

int SessionTimerRuntime_DefaultReminderSeconds(void);
int SessionTimerRuntime_DefaultReminderWindowSeconds(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_SESSION_TIMER_RUNTIME_H */

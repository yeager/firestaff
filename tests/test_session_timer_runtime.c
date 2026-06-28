/*
 * Unit test for the Session Timer Runtime module.
 *
 * Pure data-free; uses only the runtime module's public API.  No
 * SDL, no real assets, no graphics.  Covers:
 *
 *   - Init defaults (Off mode, Off -> 0 limit)
 *   - Tick boundary behavior (negative ticks ignored, accumulated math)
 *   - REMINDER_DUE latch + ack re-arm
 *   - FORCED_PAUSE latch + ClearForcedPause + post-clear tick resumption
 *   - Reset clears elapsed/latches but keeps the configured limit
 *   - FormatRemaining covers Off / HH:MM:SS / negative-input / null-buffer
 *   - Default-exposure sanity check
 *   - Acknowledging without a pending reminder is a no-op
 *   - The remaining-seconds helper matches M12 math (Off -> -1)
 */

#include "session_timer_runtime.h"

#include <stdio.h>
#include <string.h>

static int failures = 0;

static void check(int ok, const char* name) {
    if (!ok) {
        ++failures;
        printf("FAIL %s\n", name);
    } else {
        printf("PASS %s\n", name);
    }
}

int main(void) {
    SessionTimerRuntime rt;
    char buf[SESSION_TIMER_RUNTIME_TEXT_CAPACITY];
    int n;

    /* Off mode (limitMinutes <= 0) -- no events fire, remaining is -1,
     * FormatRemaining prints "Off", Tick is a no-op. */
    SessionTimerRuntime_Init(&rt, 0);
    check(rt.limitSeconds == 0 && rt.forcedPauseLatched == 0 &&
              rt.reminderPending == 0,
          "init Off: zero limit, no latches");
    check(SessionTimerRuntime_RemainingSeconds(&rt) == -1,
          "Off: remaining helper returns -1");
    check(SessionTimerRuntime_Poll(&rt) == SESSION_TIMER_RUNTIME_EVENT_RUNNING,
          "Off: Poll returns RUNNING");
    SessionTimerRuntime_Tick(&rt, 600);
    check(rt.elapsedSeconds == 0 &&
              SessionTimerRuntime_Poll(&rt) == SESSION_TIMER_RUNTIME_EVENT_RUNNING,
          "Off: Tick is a no-op");

    /* Negative limit minutes also yields Off mode (defensive against
     * future launcher settings rows). */
    SessionTimerRuntime_Init(&rt, -5);
    check(rt.limitSeconds == 0,
          "negative limit minutes collapses to Off mode");

    /* Standard 60-minute session.  Reminder threshold defaults to 5
     * minutes (300 s), so the reminder should fire once elapsed
     * crosses 55 minutes (3300 s). */
    SessionTimerRuntime_Init(&rt, 60);
    check(rt.limitMinutes == 60 && rt.limitSeconds == 3600,
          "60-min init: limit cached as 3600s");
    check(rt.remainingSeconds == 3600,
          "60-min init: remaining starts at 3600s");
    check(SessionTimerRuntime_Poll(&rt) == SESSION_TIMER_RUNTIME_EVENT_RUNNING,
          "60-min init: no reminder at start");

    /* Walk up to 5 minutes before the limit (elapsed = 3300, remaining
     * = 300).  At this point the reminder threshold (300s) has been
     * met exactly, so REMINDER_DUE must surface. */
    SessionTimerRuntime_Tick(&rt, 3300);
    check(rt.elapsedSeconds == 3300 && rt.remainingSeconds == 300,
          "60-min: tick to 3300/3600 leaves 300s remaining");
    check(SessionTimerRuntime_Poll(&rt) == SESSION_TIMER_RUNTIME_EVENT_REMINDER_DUE,
          "60-min: Poll reports REMINDER_DUE at threshold");

    /* Idempotent Poll: REMINDER_DUE stays latched until Acknowledge(). */
    SessionTimerRuntime_Poll(&rt);
    check(SessionTimerRuntime_Poll(&rt) == SESSION_TIMER_RUNTIME_EVENT_REMINDER_DUE,
          "60-min: REMINDER_DUE remains latched across repeated polls");

    /* Acknowledging the reminder clears the latch and re-arms the
     * next window (we are still below the limit; subsequent ticks
     * below the threshold should NOT re-arm until enough additional
     * time elapses). */
    SessionTimerRuntime_Acknowledge(&rt);
    check(rt.reminderPending == 0,
          "60-min: Acknowledge clears reminderPending");
    /* After ack, lastReminderFiredAtElapsed records the elapsed time
     * of the ack so the runtime can re-arm after the next window. */
    check(rt.lastReminderFiredAtElapsed == 3300,
          "60-min: Acknowledge stamps lastReminderFiredAtElapsed");

    /* A handful of additional ticks still does not re-arm. */
    SessionTimerRuntime_Tick(&rt, 60);
    check(rt.elapsedSeconds == 3360 && rt.remainingSeconds == 240,
          "60-min: tick after ack accumulates to 3360");
    check(SessionTimerRuntime_Poll(&rt) == SESSION_TIMER_RUNTIME_EVENT_RUNNING,
          "60-min: no reminder re-arm within the same window");

    /* Walking to the limit (elapsed = 3600) latches FORCED_PAUSE and
     * overrides any pending reminder state. */
    SessionTimerRuntime_Tick(&rt, 240);
    check(rt.elapsedSeconds == 3600 && rt.remainingSeconds == 0,
          "60-min: tick to limit caps elapsed at 3600");
    check(SessionTimerRuntime_Poll(&rt) == SESSION_TIMER_RUNTIME_EVENT_FORCED_PAUSE,
          "60-min: Poll reports FORCED_PAUSE at limit");
    check(rt.forcedPauseLatched == 1,
          "60-min: forcedPauseLatched is set");

    /* Further ticks while latched are no-ops. */
    SessionTimerRuntime_Tick(&rt, 999);
    check(rt.elapsedSeconds == 3600,
          "60-min: post-limit ticks are no-ops");

    /* ClearForcedPause resets the latch and reopens the runtime.  Elapsed
     * stays at the limit; a subsequent Tick pushes elapsed past the limit
     * and re-latches FORCED_PAUSE (the user dismissed the dialog without
     * leaving, so the boundary re-fires on the next tick).  Tick clamps
     * elapsed back to limitSeconds when re-latching. */
    SessionTimerRuntime_ClearForcedPause(&rt);
    check(rt.forcedPauseLatched == 0 && rt.reminderPending == 0 &&
              rt.lastReminderFiredAtElapsed == -1,
          "60-min: ClearForcedPause clears all latches");
    SessionTimerRuntime_Tick(&rt, 1);
    check(rt.elapsedSeconds == 3600 && rt.forcedPauseLatched == 1 &&
              SessionTimerRuntime_Poll(&rt) == SESSION_TIMER_RUNTIME_EVENT_FORCED_PAUSE,
          "60-min: post-clear Tick past limit re-latches FORCED_PAUSE");

    /* Reset() preserves the configured limit but resets elapsed /
     * latches -- this is the launcher->game->menu->game flow when
     * the user backs out without changing settings. */
    SessionTimerRuntime_Init(&rt, 30);
    SessionTimerRuntime_Tick(&rt, 1740); /* 29 minutes -> 60s remaining */
    check(rt.elapsedSeconds == 1740 && rt.remainingSeconds == 60,
          "30-min: tick to 1740 leaves 60s remaining");
    SessionTimerRuntime_Reset(&rt);
    check(rt.limitMinutes == 30 && rt.elapsedSeconds == 0 &&
              rt.remainingSeconds == 1800 && rt.forcedPauseLatched == 0 &&
              rt.reminderPending == 0 && rt.lastReminderFiredAtElapsed == -1,
          "30-min: Reset keeps the limit but clears elapsed + latches");

    /* Negative tick is clamped to 0 (no negative elapsed). */
    SessionTimerRuntime_Init(&rt, 15);
    SessionTimerRuntime_Tick(&rt, -50);
    check(rt.elapsedSeconds == 0 && rt.remainingSeconds == 900,
          "15-min: negative Tick clamped, no underflow");

    /* Acknowledging without a pending reminder is a no-op (no negative
     * side effects on lastReminderFiredAtElapsed). */
    SessionTimerRuntime_Init(&rt, 60);
    SessionTimerRuntime_Acknowledge(&rt);
    check(rt.lastReminderFiredAtElapsed == -1,
          "60-min: stray Acknowledge does not arm reminder window");

    /* FormatRemaining covers Off, normal HH:MM:SS, and a zero/expired
     * boundary that still reports "00:00:00" (never a negative number). */
    SessionTimerRuntime_Init(&rt, 0);
    n = SessionTimerRuntime_FormatRemaining(&rt, buf, sizeof(buf));
    check(strcmp(buf, "Off") == 0 && n == 3,
          "FormatRemaining: Off mode reports 'Off'");

    SessionTimerRuntime_Init(&rt, 60);
    SessionTimerRuntime_Tick(&rt, 60);
    n = SessionTimerRuntime_FormatRemaining(&rt, buf, sizeof(buf));
    check(strcmp(buf, "00:59:00") == 0 && n == 8,
          "FormatRemaining: 59 minutes left formats as '00:59:00'");

    SessionTimerRuntime_Tick(&rt, 3540); /* total 3600 -> limit */
    n = SessionTimerRuntime_FormatRemaining(&rt, buf, sizeof(buf));
    check(strcmp(buf, "00:00:00") == 0 && n == 8,
          "FormatRemaining: at-limit formats as '00:00:00'");

    /* Null-buffer safety: FormatRemaining writes nothing and returns 0. */
    n = SessionTimerRuntime_FormatRemaining(&rt, NULL, sizeof(buf));
    check(n == 0,
          "FormatRemaining: NULL outBuf returns 0");
    n = SessionTimerRuntime_FormatRemaining(&rt, buf, 0);
    check(n == 0,
          "FormatRemaining: zero-sized buffer returns 0");

    /* Defaults are exposed and equal the documented 5-minute values. */
    check(SessionTimerRuntime_DefaultReminderSeconds() == 300,
          "default reminder threshold = 300s");
    check(SessionTimerRuntime_DefaultReminderWindowSeconds() == 300,
          "default reminder re-arm window = 300s");

    /* Null-pointer tolerance. */
    SessionTimerRuntime_Init(NULL, 30);
    SessionTimerRuntime_Tick(NULL, 30);
    SessionTimerRuntime_Acknowledge(NULL);
    SessionTimerRuntime_ClearForcedPause(NULL);
    SessionTimerRuntime_Reset(NULL);
    check(SessionTimerRuntime_RemainingSeconds(NULL) == -1 &&
              SessionTimerRuntime_Poll(NULL) == SESSION_TIMER_RUNTIME_EVENT_RUNNING,
          "null-pointer inputs are safe no-ops");

    /* Boundary case: limitMinutes exactly 1.  Reminder should fire at
     * elapsed = 60 - 300 = -240? No -- we cannot go below 0, so the
     * reminder is armed at init time when the limit is <= the
     * threshold.  In that edge case Poll() reports REMINDER_DUE
     * immediately, which is the correct UX: a 1-minute session timer
     * is below the 5-minute reminder threshold so the user must see
     * the reminder up front. */
    SessionTimerRuntime_Init(&rt, 1);
    SessionTimerRuntime_Tick(&rt, 0);
    check(SessionTimerRuntime_Poll(&rt) == SESSION_TIMER_RUNTIME_EVENT_REMINDER_DUE,
          "1-min limit: REMINDER_DUE fires immediately (limit <= threshold)");

    if (failures) {
        printf("test_session_timer_runtime: FAIL %d\n", failures);
        return 1;
    }
    puts("test_session_timer_runtime: PASS");
    (void)n;
    return 0;
}

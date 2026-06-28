/*
 * Session Timer Runtime -- implementation.
 *
 * See include/session_timer_runtime.h for the module contract.
 *
 * Source-lock:
 *   - M12 launcher rows: include/menu_startup_m12.h M12_SessionTimer_*
 *     + M12_StartupMenu_SessionTimerLimitMinutes /
 *     M12_StartupMenu_SessionTimerRemainingSeconds (src/ui/menu_startup_m12.c).
 *   - M12 Campaign timer: include/campaign_m12.h M12_CampaignSessionTimer_*
 *     + src/shared/campaign_m12.c M12_CampaignSessionTimer_Tick (line 802).
 *   - This module deliberately does not touch M12_CampaignSessionTimer;
 *     the campaign flush lives in M12_CampaignSessionTimer_FlushToSlot.
 *     The runtime handoff boundary here is gameplay-side and the
 *     campaign flush is launcher-side accounting.  Both can coexist
 *     (the runtime reports remaining time; the campaign flushes elapsed
 *     time into playTimeSeconds), but they are independent state.
 *
 * Deterministic / data-free / no ReDMCSB-derived constants: this is a
 * Firestaff UX boundary, not a source-locked gameplay feature.
 */

#include "session_timer_runtime.h"

#include <stdio.h>
#include <string.h>

void SessionTimerRuntime_Init(SessionTimerRuntime* rt, int limitMinutes) {
    if (!rt) {
        return;
    }
    memset(rt, 0, sizeof(*rt));
    rt->limitMinutes = limitMinutes > 0 ? limitMinutes : 0;
    rt->limitSeconds = rt->limitMinutes * 60;
    rt->remainingSeconds = -1;
    rt->reminderThresholdSeconds = SESSION_TIMER_RUNTIME_DEFAULT_REMINDER_SECONDS;
    rt->reminderWindowSeconds =
        SESSION_TIMER_RUNTIME_DEFAULT_REMINDER_WINDOW_SECONDS;
    rt->lastReminderFiredAtElapsed = -1;
    rt->reminderPending = 0;
    rt->forcedPauseLatched = 0;
    rt->elapsedSeconds = 0;
    if (rt->limitSeconds > 0) {
        rt->remainingSeconds = rt->limitSeconds;
        /* When the limit is smaller than the reminder threshold (e.g.
         * a 15-minute limit with a 5-minute threshold), the user
         * would otherwise never see the reminder before FORCED_PAUSE
         * latches.  Pre-arm the reminder latch at init so the next
         * Poll() reports REMINDER_DUE immediately.  The Acknowledge()
         * re-arm window still applies: the runtime won't re-fire on
         * the next single tick. */
        if (rt->limitSeconds <= rt->reminderThresholdSeconds) {
            rt->reminderPending = 1;
        }
    }
}

void SessionTimerRuntime_Reset(SessionTimerRuntime* rt) {
    if (!rt) {
        return;
    }
    rt->elapsedSeconds = 0;
    rt->remainingSeconds = rt->limitSeconds > 0 ? rt->limitSeconds : -1;
    rt->lastReminderFiredAtElapsed = -1;
    rt->reminderPending = 0;
    rt->forcedPauseLatched = 0;
}

void SessionTimerRuntime_Tick(SessionTimerRuntime* rt, int seconds) {
    if (!rt) {
        return;
    }
    /* Off mode (no limit) is a no-op for elapsed bookkeeping; the caller
     * can still call Poll() and will see RUNNING.  We deliberately do
     * not accumulate so an "Off" session does not blow up the campaign
     * playTimeSeconds later if the launcher flips the timer on mid-run
     * without a Reset() -- the runtime is driven by an explicit Reset,
     * not by an Off-tick stream. */
    if (rt->limitSeconds <= 0) {
        return;
    }
    if (seconds <= 0) {
        return;
    }
    if (rt->forcedPauseLatched) {
        /* Once we have latched FORCED_PAUSE we stop accumulating; the
         * M11 main loop owns the boundary and must explicitly
         * ClearForcedPause() before normal ticking resumes. */
        return;
    }
    rt->elapsedSeconds += seconds;
    if (rt->elapsedSeconds >= rt->limitSeconds) {
        rt->elapsedSeconds = rt->limitSeconds;
        rt->remainingSeconds = 0;
        rt->forcedPauseLatched = 1;
        /* Make sure the next Poll reports FORCED_PAUSE even if no
         * reminder was acknowledged: clear the pending flag so the
         * main loop does not try to first surface a stale reminder
         * and then forced-pause on the same tick. */
        rt->reminderPending = 0;
        return;
    }
    rt->remainingSeconds = rt->limitSeconds - rt->elapsedSeconds;
    /* Arm the reminder latch when remaining time drops to or below the
     * reminder threshold.  Latch stays set until Acknowledge() so a
     * single boundary crossing surfaces exactly once. */
    if (!rt->reminderPending &&
        rt->remainingSeconds <= rt->reminderThresholdSeconds &&
        rt->lastReminderFiredAtElapsed < 0) {
        rt->reminderPending = 1;
    }
}

SessionTimerRuntimeEvent SessionTimerRuntime_Poll(
    const SessionTimerRuntime* rt) {
    if (!rt) {
        return SESSION_TIMER_RUNTIME_EVENT_RUNNING;
    }
    if (rt->forcedPauseLatched) {
        return SESSION_TIMER_RUNTIME_EVENT_FORCED_PAUSE;
    }
    if (rt->reminderPending) {
        return SESSION_TIMER_RUNTIME_EVENT_REMINDER_DUE;
    }
    return SESSION_TIMER_RUNTIME_EVENT_RUNNING;
}

void SessionTimerRuntime_Acknowledge(SessionTimerRuntime* rt) {
    if (!rt || !rt->reminderPending) {
        return;
    }
    rt->reminderPending = 0;
    /* Re-arm the next reminder window relative to the current elapsed
     * time so the user gets a steady cadence of reminders instead of
     * one every tick at the threshold edge. */
    rt->lastReminderFiredAtElapsed = rt->elapsedSeconds;
}

void SessionTimerRuntime_ClearForcedPause(SessionTimerRuntime* rt) {
    if (!rt) {
        return;
    }
    rt->forcedPauseLatched = 0;
    rt->reminderPending = 0;
    rt->lastReminderFiredAtElapsed = -1;
}

int SessionTimerRuntime_RemainingSeconds(const SessionTimerRuntime* rt) {
    if (!rt) {
        return -1;
    }
    if (rt->limitSeconds <= 0) {
        return -1;
    }
    return rt->remainingSeconds;
}

int SessionTimerRuntime_FormatRemaining(const SessionTimerRuntime* rt,
                                       char* outBuf,
                                       int outBufSize) {
    int remaining;
    int hours;
    int minutes;
    int seconds;
    int written = 0;
    if (!outBuf || outBufSize <= 0) {
        return 0;
    }
    outBuf[0] = '\0';
    if (!rt) {
        written = snprintf(outBuf, (size_t)outBufSize, "Off");
        return written > 0 ? written : 0;
    }
    if (rt->limitSeconds <= 0) {
        written = snprintf(outBuf, (size_t)outBufSize, "Off");
        return written > 0 ? written : 0;
    }
    remaining = rt->remainingSeconds;
    if (remaining < 0) {
        remaining = 0;
    }
    hours = remaining / 3600;
    minutes = (remaining / 60) % 60;
    seconds = remaining % 60;
    written = snprintf(outBuf, (size_t)outBufSize, "%02d:%02d:%02d",
                       hours, minutes, seconds);
    return written > 0 ? written : 0;
}

int SessionTimerRuntime_DefaultReminderSeconds(void) {
    return SESSION_TIMER_RUNTIME_DEFAULT_REMINDER_SECONDS;
}

int SessionTimerRuntime_DefaultReminderWindowSeconds(void) {
    return SESSION_TIMER_RUNTIME_DEFAULT_REMINDER_WINDOW_SECONDS;
}

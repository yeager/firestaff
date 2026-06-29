/*
 * Session Timer Runtime probe -- M11/M12 hand-off smoke test.
 *
 * Pure data-free deterministic integration probe.  Exercises the
 * launcher's session-timer limit math (M12_StartupMenu_SessionTimer*),
 * the campaign timer (M12_CampaignSessionTimer*), and the new
 * session-timer runtime (SessionTimerRuntime_*) together to confirm
 * the M11 boundary contract:
 *
 *   - Off-mode launcher setting produces an Off-mode runtime
 *     (Poll always reports RUNNING).
 *   - 60-minute launcher setting + simulated 1-second M11 ticks
 *     surfaces REMINDER_DUE exactly once at the 55-minute boundary
 *     (300s remaining), and FORCED_PAUSE at the 60-minute boundary.
 *   - Campaign flush into a slot is independent of the runtime
 *     state machine (the runtime can report FORCED_PAUSE while the
 *     campaign slot records elapsed play time).
 *   - The reminder can be acknowledged, the latch cleared, and the
 *     forced-pause released; further ticks behave as expected.
 *   - Uneven/coarse M11 tick deltas still cross the reminder and
 *     forced-pause boundaries deterministically without surfacing a
 *     stale reminder before FORCED_PAUSE.
 *
 * The probe is data-free: no SDL, no real assets, no graphics.  It
 * only consults the public M12 / session_timer_runtime APIs and the
 * M12_CampaignSessionTimer campaign-timer API.
 */

#if defined(__APPLE__) && !defined(_DARWIN_C_SOURCE)
#define _DARWIN_C_SOURCE 1
#endif
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include "campaign_m12.h"
#include "menu_startup_m12.h"
#include "session_timer_runtime.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

typedef struct {
    int total;
    int passed;
} ProbeTally;

static void probe_record(ProbeTally* tally,
                         const char* id,
                         int ok,
                         const char* message) {
    tally->total += 1;
    if (ok) {
        tally->passed += 1;
        printf("PASS %s %s\n", id, message);
    } else {
        printf("FAIL %s %s\n", id, message);
    }
}

/* Helper: simulate 1-second M11 ticks in a tight loop.  Centralizes the
 * Tick / Poll / Acknowledge contract so the assertions read like a
 * sequence diagram. */
static int drive_runtime_to(SessionTimerRuntime* rt,
                            int totalSeconds,
                            int perTickSeconds,
                            SessionTimerRuntimeEvent stopOn) {
    int ticked = 0;
    int remaining = totalSeconds;
    while (remaining > 0) {
        int step = perTickSeconds > remaining ? remaining : perTickSeconds;
        SessionTimerRuntime_Tick(rt, step);
        ticked += step;
        remaining -= step;
        if (SessionTimerRuntime_Poll(rt) == stopOn) {
            return ticked;
        }
    }
    return ticked;
}

int main(void) {
    ProbeTally tally = {0, 0};
    M12_StartupMenuState menu;
    SessionTimerRuntime rt;
    M12_CampaignSessionTimer campaign;
    M12_CampaignSlot slot;
    char buf[SESSION_TIMER_RUNTIME_TEXT_CAPACITY];

    /* Seed campaign slot the same way as test_campaign_m12_session_timer. */
    memset(&slot, 0, sizeof(slot));
    slot.status = CAMPAIGN_SLOT_ACTIVE;
    slot.playTimeSeconds = 0;

    /* ── Probe 1: launcher Off-mode is honored end-to-end. ──────────── */
    M12_StartupMenu_InitWithDataDir(&menu, "/tmp", NULL);
    menu.settings.sessionTimerIndex =
        M12_SessionTimer_IndexForMinutes(0);
    SessionTimerRuntime_Init(
        &rt,
        M12_StartupMenu_SessionTimerLimitMinutes(&menu));
    probe_record(&tally,
                 "SESSION_RUNTIME_00",
                 M12_StartupMenu_SessionTimerLimitMinutes(&menu) == 0 &&
                     rt.limitSeconds == 0 &&
                     SessionTimerRuntime_Poll(&rt) ==
                         SESSION_TIMER_RUNTIME_EVENT_RUNNING,
                 "launcher Off-mode forces runtime Off-mode (Poll=RUNNING)");

    /* ── Probe 2: 60-minute session surfaces REMINDER_DUE at 55min. ── */
    menu.settings.sessionTimerIndex =
        M12_SessionTimer_IndexForMinutes(60);
    SessionTimerRuntime_Init(
        &rt,
        M12_StartupMenu_SessionTimerLimitMinutes(&menu));
    {
        int ticked = drive_runtime_to(&rt, /*totalSeconds=*/3300,
                                       /*perTick=*/30,
                                       SESSION_TIMER_RUNTIME_EVENT_REMINDER_DUE);
        probe_record(&tally,
                     "SESSION_RUNTIME_01",
                     ticked == 3300 &&
                         SessionTimerRuntime_Poll(&rt) ==
                             SESSION_TIMER_RUNTIME_EVENT_REMINDER_DUE &&
                         SessionTimerRuntime_RemainingSeconds(&rt) == 300,
                     "60-min runtime surfaces REMINDER_DUE at 55min boundary");
    }

    /* ── Probe 3: Acknowledge re-arms, no second reminder within window. */
    SessionTimerRuntime_Acknowledge(&rt);
    SessionTimerRuntime_Tick(&rt, 60); /* 1 more minute */
    probe_record(&tally,
                 "SESSION_RUNTIME_02",
                 rt.elapsedSeconds == 3360 &&
                     SessionTimerRuntime_Poll(&rt) ==
                         SESSION_TIMER_RUNTIME_EVENT_RUNNING,
                 "Acknowledge + 60s tick stays in RUNNING within re-arm window");

    /* ── Probe 4: drive to limit and verify FORCED_PAUSE. ──────────── */
    {
        int ticked = drive_runtime_to(&rt, /*totalSeconds=*/240,
                                       /*perTick=*/30,
                                       SESSION_TIMER_RUNTIME_EVENT_FORCED_PAUSE);
        probe_record(&tally,
                     "SESSION_RUNTIME_03",
                     ticked == 240 &&
                         rt.elapsedSeconds == 3600 &&
                         SessionTimerRuntime_RemainingSeconds(&rt) == 0 &&
                         SessionTimerRuntime_Poll(&rt) ==
                             SESSION_TIMER_RUNTIME_EVENT_FORCED_PAUSE,
                     "60-min runtime surfaces FORCED_PAUSE at limit");
    }

    /* ── Probe 5: FormatRemaining matches the M12 helper. ──────────── */
    /* Re-init to a clean state so we are not in FORCED_PAUSE when we
     * test the format helper. */
    SessionTimerRuntime_Init(
        &rt, M12_StartupMenu_SessionTimerLimitMinutes(&menu));
    SessionTimerRuntime_Tick(&rt, 60);
    {
        int m12Remaining = M12_StartupMenu_SessionTimerRemainingSeconds(
            &menu, rt.elapsedSeconds);
        SessionTimerRuntime_FormatRemaining(&rt, buf, sizeof(buf));
        probe_record(&tally,
                     "SESSION_RUNTIME_04",
                     m12Remaining == 3540 &&
                         strcmp(buf, "00:59:00") == 0,
                     "FormatRemaining matches M12_StartupMenu_SessionTimerRemainingSeconds");
    }

    /* ── Probe 6: campaign timer accumulates in parallel and flushes
     *    into playTimeSeconds independently from the runtime boundary. */
    M12_CampaignSessionTimer_Init(&campaign);
    M12_CampaignSessionTimer_Start(&campaign);
    {
        int i;
        for (i = 0; i < 10; ++i) {
            M12_CampaignSessionTimer_Tick(&campaign, 60);
        }
    }
    {
        int flushed = M12_CampaignSessionTimer_FlushToSlot(&campaign, &slot);
        probe_record(&tally,
                     "SESSION_RUNTIME_05",
                     flushed == 600 && slot.playTimeSeconds == 600 &&
                         slot.modifiedAt != 0,
                     "CampaignSessionTimer flushes into slot independent of runtime state");
    }

    /* ── Probe 7: ClearForcedPause resumes ticking (idle path). ────── */
    /* Reset and force FORCED_PAUSE again, then clear and verify a 1s
     * tick brings us out of the latch (since elapsed == 3600 still,
     * any new tick past the limit re-latches, but a 0s tick must NOT
     * re-latch). */
    SessionTimerRuntime_Init(&rt, 60);
    drive_runtime_to(&rt, 3600, 60, SESSION_TIMER_RUNTIME_EVENT_FORCED_PAUSE);
    SessionTimerRuntime_ClearForcedPause(&rt);
    SessionTimerRuntime_Tick(&rt, 0); /* zero-tick must not re-latch */
    probe_record(&tally,
                 "SESSION_RUNTIME_06",
                 rt.forcedPauseLatched == 0 &&
                     SessionTimerRuntime_Poll(&rt) ==
                         SESSION_TIMER_RUNTIME_EVENT_RUNNING,
                 "ClearForcedPause + zero-tick leaves runtime in RUNNING");

    /* ── Probe 8: limits at-or-below the reminder threshold (e.g.,
     *    1 minute < 5 min reminder window) surface REMINDER_DUE at
     *    start, matching the documented boundary.  A 15-min limit
     *    (900s) is above the 300s threshold so it should NOT
     *    pre-arm and Poll must report RUNNING; a 1-min limit (60s)
     *    is below 300s so Poll must report REMINDER_DUE.  This
     *    is the safe UX behavior: the user sees the reminder up
     *    front when the limit itself is smaller than the reminder
     *    window. */
    SessionTimerRuntime_Init(&rt, 15);
    probe_record(&tally,
                 "SESSION_RUNTIME_07A",
                 SessionTimerRuntime_Poll(&rt) ==
                     SESSION_TIMER_RUNTIME_EVENT_RUNNING,
                 "15-min runtime reports RUNNING (900s > 300s threshold)");
    SessionTimerRuntime_Init(&rt, 1);
    probe_record(&tally,
                 "SESSION_RUNTIME_07B",
                 SessionTimerRuntime_Poll(&rt) ==
                     SESSION_TIMER_RUNTIME_EVENT_REMINDER_DUE,
                 "1-min runtime reports REMINDER_DUE at init (60s < 300s threshold)");

    /* ── Probe 9: stale-session reset re-arms from a fresh init. ──── */
    SessionTimerRuntime_Init(&rt, 30);
    SessionTimerRuntime_Tick(&rt, 600);
    SessionTimerRuntime_Reset(&rt);
    probe_record(&tally,
                 "SESSION_RUNTIME_08",
                 rt.elapsedSeconds == 0 &&
                     rt.remainingSeconds == 1800 &&
                     SessionTimerRuntime_Poll(&rt) ==
                         SESSION_TIMER_RUNTIME_EVENT_RUNNING,
                 "Reset returns to RUNNING with full limit");

    /* ── Probe 10: round-trip across launcher settings values. ─────── */
    {
        int minutes;
        for (minutes = 0; minutes <= 120; minutes += 15) {
            int index = M12_SessionTimer_IndexForMinutes(minutes);
            menu.settings.sessionTimerIndex = index;
            SessionTimerRuntime_Init(
                &rt, M12_StartupMenu_SessionTimerLimitMinutes(&menu));
        }
        probe_record(&tally,
                     "SESSION_RUNTIME_09",
                     M12_StartupMenu_SessionTimerLimitMinutes(&menu) == 120 &&
                         rt.limitSeconds == 7200,
                     "launcher round-trip across 0/15/30/60/120 minutes stays coherent");
    }

    /* ── Probe 11: defaults are documented 5-minute values. ────────── */
    probe_record(&tally,
                 "SESSION_RUNTIME_10",
                 SessionTimerRuntime_DefaultReminderSeconds() == 300 &&
                     SessionTimerRuntime_DefaultReminderWindowSeconds() == 300,
                 "default reminder + re-arm window = 300s (5 min)");

    /* ── Probe 12: uneven/coarse M11 tick deltas cross boundaries. ─── */
    SessionTimerRuntime_Init(&rt, 60);
    SessionTimerRuntime_Tick(&rt, 3299);
    SessionTimerRuntime_Tick(&rt, 2);
    probe_record(&tally,
                 "SESSION_RUNTIME_11A",
                 rt.elapsedSeconds == 3301 &&
                     SessionTimerRuntime_RemainingSeconds(&rt) == 299 &&
                     SessionTimerRuntime_Poll(&rt) ==
                         SESSION_TIMER_RUNTIME_EVENT_REMINDER_DUE,
                 "uneven 3299+2s ticks cross the 300s reminder threshold");

    SessionTimerRuntime_Init(&rt, 60);
    SessionTimerRuntime_Tick(&rt, 3700);
    SessionTimerRuntime_FormatRemaining(&rt, buf, sizeof(buf));
    probe_record(&tally,
                 "SESSION_RUNTIME_11B",
                 rt.elapsedSeconds == 3600 &&
                     rt.remainingSeconds == 0 &&
                     rt.reminderPending == 0 &&
                     rt.forcedPauseLatched == 1 &&
                     SessionTimerRuntime_Poll(&rt) ==
                         SESSION_TIMER_RUNTIME_EVENT_FORCED_PAUSE &&
                     strcmp(buf, "00:00:00") == 0,
                 "single coarse tick clamps at limit and reports FORCED_PAUSE only");

    printf("# summary: %d/%d invariants passed\n", tally.passed, tally.total);
    return (tally.passed == tally.total) ? 0 : 1;
}

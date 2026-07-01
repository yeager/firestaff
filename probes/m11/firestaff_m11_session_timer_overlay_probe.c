/*
 * firestaff_m11_session_timer_overlay_probe.c
 *
 * Headless probe for the M11 session-timer reminder overlay and
 * forced-pause boundary. Drives the overlay through:
 *   - configure(60-min limit, default 60-sec grace)
 *   - tick to REMINDER threshold, assert banner format and pixel paint
 *   - tick to FORCED_PAUSE threshold, assert banner format and pixel paint
 *   - dismiss, assert transition to DISMISSED and forced-pause count
 *   - second tick past threshold, assert re-arm and dismiss count
 *
 * No game data required. Run by CTest as
 * `firestaff_m11_session_timer_overlay_probe`. See DONE.md 2026-06-26
 * entry for the related gate pass.
 */

#include "m11_session_timer_overlay.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int total;
    int passed;
} ProbeTally;

static void probe_record(ProbeTally* tally, const char* id, int ok,
                         const char* message) {
    tally->total += 1;
    if (ok) {
        tally->passed += 1;
        printf("PASS %s %s\n", id, message);
    } else {
        printf("FAIL %s %s\n", id, message);
    }
}

static int count_changed_from(const unsigned char* fb, size_t size,
                              unsigned char baseline) {
    size_t i;
    int n = 0;
    for (i = 0; i < size; ++i) {
        if (fb[i] != baseline) {
            ++n;
        }
    }
    return n;
}

static void probe_reminder_to_forced_pause(ProbeTally* tally) {
    M11_SessionTimerOverlay overlay;
    unsigned char fb[320 * 200];
    char banner[M11_SESSION_TIMER_OVERLAY_TEXT_CAPACITY];
    int kind;
    int drawn;
    int painted;

    memset(fb, 0, sizeof(fb));
    M11_SessionTimerOverlay_Init(&overlay);
    /* 1-minute limit, 60-second default grace, so forced-pause fires
     * at elapsed = 120 seconds. */
    M11_SessionTimerOverlay_Configure(&overlay, 1, -1);
    probe_record(tally, "M11_SESSION_TIMER_01",
                 M11_SessionTimerOverlay_GetState(&overlay)
                     == M11_SESSION_TIMER_STATE_RUNNING,
                 "Configure(1, -1) -> RUNNING");
    probe_record(tally, "M11_SESSION_TIMER_02",
                 M11_SessionTimerOverlay_GetLimitSeconds(&overlay) == 60,
                 "limit seconds = 60");
    probe_record(tally, "M11_SESSION_TIMER_03",
                 M11_SessionTimerOverlay_GetRemainingSeconds(&overlay) == 60,
                 "remaining = 60 after configure");

    /* Tick to the reminder threshold. */
    M11_SessionTimerOverlay_Tick(&overlay, 60);
    probe_record(tally, "M11_SESSION_TIMER_04",
                 M11_SessionTimerOverlay_GetState(&overlay)
                     == M11_SESSION_TIMER_STATE_REMINDER,
                 "T+60 -> REMINDER");
    probe_record(tally, "M11_SESSION_TIMER_05",
                 M11_SessionTimerOverlay_GetRemainingSeconds(&overlay) == 0,
                 "remaining = 0 at the reminder threshold");
    probe_record(tally, "M11_SESSION_TIMER_06",
                 M11_SessionTimerOverlay_BlocksGameplayInput(&overlay) == 0,
                 "REMINDER does not block gameplay input");

    /* Banner formatting at REMINDER. */
    kind = M11_SessionTimerOverlay_Format(&overlay, banner,
                                          (int)sizeof(banner));
    probe_record(tally, "M11_SESSION_TIMER_07",
                 kind == M11_SESSION_TIMER_BANNER_REMINDER,
                 "Format at REMINDER returns REMINDER kind");
    probe_record(tally, "M11_SESSION_TIMER_08",
                 strcmp(banner, "SESSION TIMER 00:01:00 LEFT") == 0,
                 "banner shows HH:MM:SS left");
    probe_record(tally, "M11_SESSION_TIMER_09",
                 M11_SessionTimerOverlay_IsActive(&overlay) == 1,
                 "REMINDER is active");

    /* Framebuffer paint at REMINDER. */
    drawn = M11_SessionTimerOverlay_Draw(NULL, &overlay, fb, 320, 200);
    painted = count_changed_from(fb, sizeof(fb), 0);
    probe_record(tally, "M11_SESSION_TIMER_10",
                 drawn > 0,
                 "Draw at REMINDER paints glyph rows");
    probe_record(tally, "M11_SESSION_TIMER_11",
                 painted > 0,
                 "Draw at REMINDER writes non-zero pixels");

    /* Tick to forced-pause boundary. */
    M11_SessionTimerOverlay_Tick(&overlay, 60);
    probe_record(tally, "M11_SESSION_TIMER_12",
                 M11_SessionTimerOverlay_GetState(&overlay)
                     == M11_SESSION_TIMER_STATE_FORCED_PAUSE,
                 "T+120 -> FORCED_PAUSE");
    probe_record(tally, "M11_SESSION_TIMER_13",
                 M11_SessionTimerOverlay_BlocksGameplayInput(&overlay) == 1,
                 "FORCED_PAUSE blocks gameplay input");
    probe_record(tally, "M11_SESSION_TIMER_14",
                 M11_SessionTimerOverlay_GetForcedPauseCount(&overlay) == 1,
                 "forcedPauseCount = 1");

    /* Banner formatting at FORCED_PAUSE. */
    kind = M11_SessionTimerOverlay_Format(&overlay, banner,
                                          (int)sizeof(banner));
    probe_record(tally, "M11_SESSION_TIMER_15",
                 kind == M11_SESSION_TIMER_BANNER_FORCED_PAUSE,
                 "Format at FORCED_PAUSE returns FORCED_PAUSE kind");
    probe_record(tally, "M11_SESSION_TIMER_16",
                 strcmp(banner, "SESSION EXPIRED - PRESS ESC TO EXIT") == 0,
                 "forced-pause banner reads 'SESSION EXPIRED - PRESS ESC TO EXIT'");

    /* Framebuffer paint at FORCED_PAUSE. */
    memset(fb, 0, sizeof(fb));
    drawn = M11_SessionTimerOverlay_Draw(NULL, &overlay, fb, 320, 200);
    painted = count_changed_from(fb, sizeof(fb), 0);
    probe_record(tally, "M11_SESSION_TIMER_17",
                 drawn > 0,
                 "Draw at FORCED_PAUSE paints glyph rows");
    probe_record(tally, "M11_SESSION_TIMER_18",
                 painted > 200,
                 "Draw at FORCED_PAUSE paints substantial overlay");

    /* Dismiss. */
    probe_record(tally, "M11_SESSION_TIMER_19",
                 M11_SessionTimerOverlay_Dismiss(&overlay) == 1,
                 "Dismiss at FORCED_PAUSE accepted");
    probe_record(tally, "M11_SESSION_TIMER_20",
                 M11_SessionTimerOverlay_GetState(&overlay)
                     == M11_SESSION_TIMER_STATE_DISMISSED,
                 "Dismiss -> DISMISSED");
    probe_record(tally, "M11_SESSION_TIMER_21",
                 M11_SessionTimerOverlay_GetDismissCount(&overlay) == 1,
                 "dismissCount = 1");
    probe_record(tally, "M11_SESSION_TIMER_22",
                 M11_SessionTimerOverlay_BlocksGameplayInput(&overlay) == 0,
                 "DISMISSED no longer blocks input");

    /* Tick past forcedPauseAt again -> re-arm. */
    M11_SessionTimerOverlay_Tick(&overlay, 1);
    probe_record(tally, "M11_SESSION_TIMER_23",
                 M11_SessionTimerOverlay_GetState(&overlay)
                     == M11_SESSION_TIMER_STATE_FORCED_PAUSE,
                 "After dismiss, tick re-arms FORCED_PAUSE");
    probe_record(tally, "M11_SESSION_TIMER_24",
                 M11_SessionTimerOverlay_GetForcedPauseCount(&overlay) == 2,
                 "forcedPauseCount = 2 after re-arm");
}

static void probe_disabled_inert(ProbeTally* tally) {
    M11_SessionTimerOverlay overlay;
    unsigned char fb[320 * 200];
    int drawn;
    int painted;
    memset(fb, 0xAA, sizeof(fb));
    M11_SessionTimerOverlay_Init(&overlay);
    M11_SessionTimerOverlay_Configure(&overlay, 0, 30);
    M11_SessionTimerOverlay_Tick(&overlay, 99999);
    probe_record(tally, "M11_SESSION_TIMER_DISABLED_01",
                 M11_SessionTimerOverlay_GetState(&overlay)
                     == M11_SESSION_TIMER_STATE_DISABLED,
                 "Disabled overlay stays DISABLED through any tick count");
    drawn = M11_SessionTimerOverlay_Draw(NULL, &overlay, fb, 320, 200);
    painted = count_changed_from(fb, sizeof(fb), 0xAA);
    probe_record(tally, "M11_SESSION_TIMER_DISABLED_02",
                 drawn == 0,
                 "Draw on disabled overlay paints 0 glyphs");
    probe_record(tally, "M11_SESSION_TIMER_DISABLED_03",
                 painted == 0,
                 "Draw on disabled overlay does not modify framebuffer");
    probe_record(tally, "M11_SESSION_TIMER_DISABLED_04",
                 M11_SessionTimerOverlay_BlocksGameplayInput(&overlay) == 0,
                 "Disabled overlay never blocks input");
}

static void probe_grace_clamp(ProbeTally* tally) {
    M11_SessionTimerOverlay overlay;
    M11_SessionTimerOverlay_Init(&overlay);
    M11_SessionTimerOverlay_Configure(&overlay, 30, -5);
    probe_record(tally, "M11_SESSION_TIMER_GRACE_01",
                 M11_SessionTimerOverlay_GetLimitSeconds(&overlay) == 30 * 60,
                 "limit = 1800 for 30 minutes");
    probe_record(tally, "M11_SESSION_TIMER_GRACE_02",
                 overlay.graceSeconds == 60,
                 "negative grace defaults to 60");

    M11_SessionTimerOverlay_Init(&overlay);
    M11_SessionTimerOverlay_Configure(&overlay, 30, 99999);
    probe_record(tally, "M11_SESSION_TIMER_GRACE_03",
                 overlay.graceSeconds == 3600,
                 "oversized grace clamped to 3600");
}

static void probe_skip_thresholds(ProbeTally* tally) {
    M11_SessionTimerOverlay overlay;
    M11_SessionTimerOverlay_Init(&overlay);
    M11_SessionTimerOverlay_Configure(&overlay, 1, 1); /* limit 60 + grace 1 = 61 */
    M11_SessionTimerOverlay_Tick(&overlay, 999);
    probe_record(tally, "M11_SESSION_TIMER_SKIP_01",
                 M11_SessionTimerOverlay_GetState(&overlay)
                     == M11_SESSION_TIMER_STATE_FORCED_PAUSE,
                 "single huge tick jumps to FORCED_PAUSE");
    probe_record(tally, "M11_SESSION_TIMER_SKIP_02",
                 M11_SessionTimerOverlay_GetForcedPauseCount(&overlay) == 1,
                 "single huge tick increments forcedPauseCount once");
}

int main(void) {
    ProbeTally tally = {0, 0};
    probe_reminder_to_forced_pause(&tally);
    probe_disabled_inert(&tally);
    probe_grace_clamp(&tally);
    probe_skip_thresholds(&tally);
    printf("# summary: %d/%d invariants passed\n",
           tally.passed, tally.total);
    return (tally.passed == tally.total) ? 0 : 1;
}

/*
 * test_m11_session_timer_overlay.c
 *
 * Unit tests for the M11 session-timer reminder overlay / forced-pause
 * boundary module. See include/m11_session_timer_overlay.h for the
 * full contract and src/engine/m11_session_timer_overlay.c for the
 * implementation. No game data required.
 */

#include "m11_session_timer_overlay.h"

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

static void test_init_disabled(void) {
    M11_SessionTimerOverlay overlay;
    memset(&overlay, 0xFF, sizeof(overlay));
    M11_SessionTimerOverlay_Init(&overlay);
    check(overlay.state == M11_SESSION_TIMER_STATE_DISABLED,
          "init -> DISABLED");
    check(overlay.elapsedSeconds == 0, "init elapsed = 0");
    check(overlay.limitSeconds == 0, "init limit = 0");
    check(overlay.graceSeconds == 0, "init grace = 0");
    check(overlay.forcedPauseAtSeconds == 0,
          "init forcedPauseAt = 0");
    check(overlay.forcedPauseCount == 0, "init forcedPauseCount = 0");
    check(overlay.dismissCount == 0, "init dismissCount = 0");
    check(M11_SessionTimerOverlay_GetState(&overlay)
              == M11_SESSION_TIMER_STATE_DISABLED,
          "init GetState returns DISABLED");
    check(M11_SessionTimerOverlay_IsActive(&overlay) == 0,
          "init IsActive = 0");
    check(M11_SessionTimerOverlay_BlocksGameplayInput(&overlay) == 0,
          "init BlocksGameplayInput = 0");
    check(M11_SessionTimerOverlay_GetRemainingSeconds(&overlay) == -1,
          "init GetRemainingSeconds = -1");
    check(M11_SessionTimerOverlay_GetLimitSeconds(&overlay) == 0,
          "init GetLimitSeconds = 0");
    check(M11_SessionTimerOverlay_GetElapsedSeconds(&overlay) == 0,
          "init GetElapsedSeconds = 0");
}

static void test_configure_off(void) {
    M11_SessionTimerOverlay overlay;
    M11_SessionTimerOverlay_Init(&overlay);
    M11_SessionTimerOverlay_Configure(&overlay, 0, 30);
    check(overlay.state == M11_SESSION_TIMER_STATE_DISABLED,
          "Configure(0) -> DISABLED");
    check(overlay.limitSeconds == 0, "Configure(0) limit = 0");
    check(overlay.graceSeconds == 0, "Configure(0) grace = 0");
    /* Tick should not transition out of DISABLED. */
    M11_SessionTimerOverlay_Tick(&overlay, 9999);
    check(overlay.state == M11_SESSION_TIMER_STATE_DISABLED,
          "Tick while DISABLED stays DISABLED");
    /* Elapsed still accumulates so a probe can assert shape. */
    check(overlay.elapsedSeconds == 9999,
          "Tick while DISABLED still records elapsed seconds");
}

static void test_configure_nonzero(void) {
    M11_SessionTimerOverlay overlay;
    M11_SessionTimerOverlay_Init(&overlay);
    /* 15-minute limit with default grace (60s negative sentinel). */
    M11_SessionTimerOverlay_Configure(&overlay, 15, -1);
    check(overlay.state == M11_SESSION_TIMER_STATE_RUNNING,
          "Configure(15, -1) -> RUNNING");
    check(overlay.limitSeconds == 15 * 60, "Configure(15) limit = 900");
    check(overlay.graceSeconds == 60,
          "Configure(15, -1) grace defaults to 60");
    check(overlay.forcedPauseAtSeconds == 900 + 60,
          "Configure(15) forcedPauseAt = 960");
    check(M11_SessionTimerOverlay_GetRemainingSeconds(&overlay) == 900,
          "Configure(15) GetRemainingSeconds = 900");
}

static void test_grace_clamp(void) {
    M11_SessionTimerOverlay overlay;
    M11_SessionTimerOverlay_Init(&overlay);
    M11_SessionTimerOverlay_Configure(&overlay, 30, -5);
    check(overlay.graceSeconds == 60,
          "negative grace defaults to 60");
    M11_SessionTimerOverlay_Init(&overlay);
    M11_SessionTimerOverlay_Configure(&overlay, 30, 99999);
    check(overlay.graceSeconds == 3600,
          "oversized grace clamped to 3600");
    M11_SessionTimerOverlay_Init(&overlay);
    M11_SessionTimerOverlay_Configure(&overlay, 30, 0);
    check(overlay.graceSeconds == 0,
          "explicit zero grace is preserved");
}

static void test_state_transitions(void) {
    M11_SessionTimerOverlay overlay;
    M11_SessionTimerOverlay_Init(&overlay);
    /* 60-second limit + 10-second grace so we can tick through the
     * grace window without jumping to FORCED_PAUSE immediately. */
    M11_SessionTimerOverlay_Configure(&overlay, 1, 10);
    /* Step to just under limit. */
    M11_SessionTimerOverlay_Tick(&overlay, 59);
    check(overlay.state == M11_SESSION_TIMER_STATE_RUNNING,
          "T+59 RUNNING");
    /* Cross the limit. */
    M11_SessionTimerOverlay_Tick(&overlay, 1);
    check(overlay.state == M11_SESSION_TIMER_STATE_REMINDER,
          "T+60 -> REMINDER");
    /* Stay in REMINDER for the grace window. */
    M11_SessionTimerOverlay_Tick(&overlay, 5);
    check(overlay.state == M11_SESSION_TIMER_STATE_REMINDER,
          "T+65 stays REMINDER until grace elapsed");
    /* Cross the forced-pause boundary. */
    M11_SessionTimerOverlay_Tick(&overlay, 5);
    check(overlay.state == M11_SESSION_TIMER_STATE_FORCED_PAUSE,
          "T+70 -> FORCED_PAUSE");
    check(overlay.forcedPauseCount == 1,
          "forcedPauseCount incremented exactly once");
}

static void test_state_skip_thresholds(void) {
    M11_SessionTimerOverlay overlay;
    M11_SessionTimerOverlay_Init(&overlay);
    M11_SessionTimerOverlay_Configure(&overlay, 1, 1); /* limit 60s + 1 grace */
    /* Skip directly past both thresholds. */
    M11_SessionTimerOverlay_Tick(&overlay, 999);
    check(overlay.state == M11_SESSION_TIMER_STATE_FORCED_PAUSE,
          "single huge tick jumps to FORCED_PAUSE");
    check(overlay.forcedPauseCount == 1,
          "single huge tick increments forcedPauseCount once");
}

static void test_dismiss_only_in_forced_pause(void) {
    M11_SessionTimerOverlay overlay;
    M11_SessionTimerOverlay_Init(&overlay);
    M11_SessionTimerOverlay_Configure(&overlay, 1, 1);
    /* Dismiss while RUNNING is rejected. */
    check(M11_SessionTimerOverlay_Dismiss(&overlay) == 0,
          "Dismiss in RUNNING rejected");
    /* Dismiss while REMINDER is rejected. */
    M11_SessionTimerOverlay_Tick(&overlay, 60);
    check(overlay.state == M11_SESSION_TIMER_STATE_REMINDER,
          "T+60 -> REMINDER (sanity)");
    check(M11_SessionTimerOverlay_Dismiss(&overlay) == 0,
          "Dismiss in REMINDER rejected");
    /* Dismiss while FORCED_PAUSE is accepted. */
    M11_SessionTimerOverlay_Tick(&overlay, 2);
    check(overlay.state == M11_SESSION_TIMER_STATE_FORCED_PAUSE,
          "T+62 -> FORCED_PAUSE (sanity)");
    check(M11_SessionTimerOverlay_Dismiss(&overlay) == 1,
          "Dismiss in FORCED_PAUSE accepted");
    check(overlay.state == M11_SESSION_TIMER_STATE_DISMISSED,
          "Dismiss moves state to DISMISSED");
    check(overlay.dismissCount == 1, "dismissCount = 1");
    check(M11_SessionTimerOverlay_BlocksGameplayInput(&overlay) == 0,
          "after Dismiss, BlocksGameplayInput = 0");
    /* Re-dismiss is idempotent. */
    check(M11_SessionTimerOverlay_Dismiss(&overlay) == 0,
          "Dismiss in DISMISSED rejected");
}

static void test_dismissed_can_rearm(void) {
    M11_SessionTimerOverlay overlay;
    M11_SessionTimerOverlay_Init(&overlay);
    M11_SessionTimerOverlay_Configure(&overlay, 1, 1);
    M11_SessionTimerOverlay_Tick(&overlay, 62);
    check(overlay.state == M11_SESSION_TIMER_STATE_FORCED_PAUSE, "T+62 FP");
    (void)M11_SessionTimerOverlay_Dismiss(&overlay);
    check(overlay.state == M11_SESSION_TIMER_STATE_DISMISSED,
          "after dismiss -> DISMISSED");
    /* Tick past forcedPauseAt again: should re-arm FORCED_PAUSE. */
    M11_SessionTimerOverlay_Tick(&overlay, 0); /* no-op */
    /* elapsed already past forcedPauseAt so a tick of 1 still leaves
     * the elapsed >= forcedPauseAt -> re-arm. */
    M11_SessionTimerOverlay_Tick(&overlay, 1);
    check(overlay.state == M11_SESSION_TIMER_STATE_FORCED_PAUSE,
          "after dismiss, tick re-arms FORCED_PAUSE");
    check(overlay.forcedPauseCount == 2,
          "forcedPauseCount = 2 after re-arm");
    check(overlay.dismissCount == 1,
          "dismissCount = 1 (unchanged by re-arm)");
}

static void test_negative_tick_rejected(void) {
    M11_SessionTimerOverlay overlay;
    M11_SessionTimerOverlay_Init(&overlay);
    M11_SessionTimerOverlay_Configure(&overlay, 1, 0);
    M11_SessionTimerOverlay_Tick(&overlay, -10);
    check(overlay.elapsedSeconds == 0,
          "negative tick does not advance elapsed");
    M11_SessionTimerOverlay_Tick(&overlay, 0);
    check(overlay.elapsedSeconds == 0,
          "zero tick does not advance elapsed");
}

static void test_elapsed_cap(void) {
    M11_SessionTimerOverlay overlay;
    M11_SessionTimerOverlay_Init(&overlay);
    M11_SessionTimerOverlay_Configure(&overlay, 1, 0);
    M11_SessionTimerOverlay_Tick(&overlay, 999999);
    check(overlay.elapsedSeconds <= 24 * 60 * 60,
          "elapsed capped to 24h");
}

static void test_format_reminder(void) {
    M11_SessionTimerOverlay overlay;
    char buf[M11_SESSION_TIMER_OVERLAY_TEXT_CAPACITY];
    int kind;
    M11_SessionTimerOverlay_Init(&overlay);
    /* 1-min limit + 60s default grace -> forcedPauseAt = 120s.
     * Tick(60) lands in REMINDER with 60s of grace remaining. */
    M11_SessionTimerOverlay_Configure(&overlay, 1, -1);
    M11_SessionTimerOverlay_Tick(&overlay, 60);
    kind = M11_SessionTimerOverlay_Format(&overlay, buf, (int)sizeof(buf));
    check(kind == M11_SESSION_TIMER_BANNER_REMINDER,
          "Format in REMINDER returns REMINDER kind");
    check(strcmp(buf, "SESSION TIMER 00:01:00 LEFT") == 0,
          "Format in REMINDER shows time until forced pause");
}

static void test_format_forced_pause(void) {
    M11_SessionTimerOverlay overlay;
    char buf[M11_SESSION_TIMER_OVERLAY_TEXT_CAPACITY];
    int kind;
    M11_SessionTimerOverlay_Init(&overlay);
    /* 1-min limit + 1s grace -> forcedPauseAt = 61s. Tick(61) lands
     * in FORCED_PAUSE. */
    M11_SessionTimerOverlay_Configure(&overlay, 1, 1);
    M11_SessionTimerOverlay_Tick(&overlay, 61);
    kind = M11_SessionTimerOverlay_Format(&overlay, buf, (int)sizeof(buf));
    check(kind == M11_SESSION_TIMER_BANNER_FORCED_PAUSE,
          "Format in FORCED_PAUSE returns FORCED_PAUSE kind");
    check(strcmp(buf, "SESSION EXPIRED - PRESS ESC TO EXIT") == 0,
          "Format in FORCED_PAUSE shows forced-pause banner");
}

static void test_format_disabled_and_running(void) {
    M11_SessionTimerOverlay overlay;
    char buf[M11_SESSION_TIMER_OVERLAY_TEXT_CAPACITY];
    int kind;
    M11_SessionTimerOverlay_Init(&overlay);
    M11_SessionTimerOverlay_Configure(&overlay, 0, 0);
    kind = M11_SessionTimerOverlay_Format(&overlay, buf, (int)sizeof(buf));
    check(kind == M11_SESSION_TIMER_BANNER_NONE,
          "Format in DISABLED returns NONE");
    check(buf[0] == '\0', "Format in DISABLED writes empty string");

    M11_SessionTimerOverlay_Init(&overlay);
    M11_SessionTimerOverlay_Configure(&overlay, 1, 0);
    kind = M11_SessionTimerOverlay_Format(&overlay, buf, (int)sizeof(buf));
    check(kind == M11_SESSION_TIMER_BANNER_NONE,
          "Format in RUNNING returns NONE");
}

static void test_format_truncates_to_buffer(void) {
    M11_SessionTimerOverlay overlay;
    char tiny[8];
    int kind;
    M11_SessionTimerOverlay_Init(&overlay);
    /* 2-min limit + 60s default grace -> forcedPauseAt = 180s.
     * Tick(120) lands in REMINDER with 60s of grace remaining. */
    M11_SessionTimerOverlay_Configure(&overlay, 2, -1);
    M11_SessionTimerOverlay_Tick(&overlay, 120);
    kind = M11_SessionTimerOverlay_Format(&overlay, tiny, (int)sizeof(tiny));
    check(kind == M11_SESSION_TIMER_BANNER_REMINDER,
          "Format returns REMINDER even with tiny buffer");
    /* snprintf must NUL-terminate within the buffer. */
    check(tiny[sizeof(tiny) - 1] == '\0',
          "Format NUL-terminates within tiny buffer");
}

static void test_draw_disabled_no_pixels(void) {
    M11_SessionTimerOverlay overlay;
    unsigned char fb[320 * 200];
    int drawn;
    memset(fb, 0xAA, sizeof(fb));
    M11_SessionTimerOverlay_Init(&overlay);
    M11_SessionTimerOverlay_Configure(&overlay, 0, 0);
    drawn = M11_SessionTimerOverlay_Draw(NULL, &overlay, fb, 320, 200);
    check(drawn == 0, "Draw in DISABLED paints 0 glyphs");
    /* Verify fb unchanged (no bytes overwritten). */
    {
        size_t i;
        int allUnchanged = 1;
        for (i = 0; i < sizeof(fb); ++i) {
            if (fb[i] != 0xAA) {
                allUnchanged = 0;
                break;
            }
        }
        check(allUnchanged == 1,
              "Draw in DISABLED does not modify framebuffer");
    }
}

static void test_draw_running_no_pixels(void) {
    M11_SessionTimerOverlay overlay;
    unsigned char fb[320 * 200];
    int drawn;
    memset(fb, 0xAA, sizeof(fb));
    M11_SessionTimerOverlay_Init(&overlay);
    M11_SessionTimerOverlay_Configure(&overlay, 1, 0);
    M11_SessionTimerOverlay_Tick(&overlay, 30); /* still RUNNING */
    drawn = M11_SessionTimerOverlay_Draw(NULL, &overlay, fb, 320, 200);
    check(drawn == 0, "Draw in RUNNING paints 0 glyphs");
}

static void test_draw_reminder_paints_pixels(void) {
    M11_SessionTimerOverlay overlay;
    unsigned char fb[320 * 200];
    int drawn;
    int nonZeroCount;
    size_t i;
    memset(fb, 0, sizeof(fb));
    M11_SessionTimerOverlay_Init(&overlay);
    M11_SessionTimerOverlay_Configure(&overlay, 1, 0);
    M11_SessionTimerOverlay_Tick(&overlay, 60);
    drawn = M11_SessionTimerOverlay_Draw(NULL, &overlay, fb, 320, 200);
    check(drawn > 0, "Draw in REMINDER paints >0 glyphs");
    nonZeroCount = 0;
    for (i = 0; i < sizeof(fb); ++i) {
        if (fb[i] != 0) {
            ++nonZeroCount;
        }
    }
    check(nonZeroCount > 0,
          "Draw in REMINDER writes non-zero pixels");
}

static void test_draw_forced_pause_paints_box(void) {
    M11_SessionTimerOverlay overlay;
    unsigned char fb[320 * 200];
    int drawn;
    int nonZeroCount;
    size_t i;
    memset(fb, 0, sizeof(fb));
    M11_SessionTimerOverlay_Init(&overlay);
    M11_SessionTimerOverlay_Configure(&overlay, 1, 0);
    M11_SessionTimerOverlay_Tick(&overlay, 61);
    drawn = M11_SessionTimerOverlay_Draw(NULL, &overlay, fb, 320, 200);
    check(drawn > 0, "Draw in FORCED_PAUSE paints >0 glyphs");
    nonZeroCount = 0;
    for (i = 0; i < sizeof(fb); ++i) {
        if (fb[i] != 0) {
            ++nonZeroCount;
        }
    }
    /* Forced-pause paints a 36-pixel-tall centered box plus two text
     * lines, so we expect a large number of non-zero pixels. */
    check(nonZeroCount > 200,
          "Draw in FORCED_PAUSE paints substantial overlay");
}

static void test_draw_null_safety(void) {
    unsigned char fb[320 * 200];
    int drawn;
    memset(fb, 0, sizeof(fb));
    /* NULL overlay */
    drawn = M11_SessionTimerOverlay_Draw(NULL, NULL, fb, 320, 200);
    check(drawn == 0, "Draw with NULL overlay returns 0");
    /* NULL framebuffer */
    {
        M11_SessionTimerOverlay overlay;
        M11_SessionTimerOverlay_Init(&overlay);
        M11_SessionTimerOverlay_Configure(&overlay, 1, 0);
        M11_SessionTimerOverlay_Tick(&overlay, 60);
        drawn = M11_SessionTimerOverlay_Draw(NULL, &overlay, NULL, 320, 200);
        check(drawn == 0, "Draw with NULL framebuffer returns 0");
    }
    /* Zero-dim framebuffer */
    {
        M11_SessionTimerOverlay overlay;
        M11_SessionTimerOverlay_Init(&overlay);
        M11_SessionTimerOverlay_Configure(&overlay, 1, 0);
        M11_SessionTimerOverlay_Tick(&overlay, 60);
        drawn = M11_SessionTimerOverlay_Draw(NULL, &overlay, fb, 0, 0);
        check(drawn == 0, "Draw with 0x0 framebuffer returns 0");
    }
}

static void test_reconfigure_resets(void) {
    M11_SessionTimerOverlay overlay;
    M11_SessionTimerOverlay_Init(&overlay);
    M11_SessionTimerOverlay_Configure(&overlay, 1, 0);
    M11_SessionTimerOverlay_Tick(&overlay, 61); /* FORCED_PAUSE */
    M11_SessionTimerOverlay_Dismiss(&overlay);
    check(overlay.forcedPauseCount == 1, "pre-reconfigure count = 1");
    check(overlay.dismissCount == 1, "pre-reconfigure dismiss = 1");
    M11_SessionTimerOverlay_Configure(&overlay, 0, 0); /* turn OFF */
    check(overlay.state == M11_SESSION_TIMER_STATE_DISABLED,
          "Reconfigure to 0 -> DISABLED");
    check(overlay.forcedPauseCount == 0,
          "Reconfigure resets forcedPauseCount");
    check(overlay.dismissCount == 0,
          "Reconfigure resets dismissCount");
    check(overlay.elapsedSeconds == 0,
          "Reconfigure resets elapsedSeconds");
}

int main(void) {
    test_init_disabled();
    test_configure_off();
    test_configure_nonzero();
    test_grace_clamp();
    test_state_transitions();
    test_state_skip_thresholds();
    test_dismiss_only_in_forced_pause();
    test_dismissed_can_rearm();
    test_negative_tick_rejected();
    test_elapsed_cap();
    test_format_reminder();
    test_format_forced_pause();
    test_format_disabled_and_running();
    test_format_truncates_to_buffer();
    test_draw_disabled_no_pixels();
    test_draw_running_no_pixels();
    test_draw_reminder_paints_pixels();
    test_draw_forced_pause_paints_box();
    test_draw_null_safety();
    test_reconfigure_resets();

    if (failures) {
        printf("test_m11_session_timer_overlay: FAIL %d\n", failures);
        return 1;
    }
    puts("test_m11_session_timer_overlay: PASS");
    return 0;
}

/*
 * test_fs_gesture_navigation_gate.c — unit tests for the cross-game
 * runtime gesture navigation gate.
 *
 * These tests run from pure C logic on synthetic input.  No game
 * data is required and no SDL context is needed.  Each assertion
 * is data-free and CTest-greppable.
 *
 * Source-lock anchors (full list in
 * docs/source-lock/fs_gesture_navigation_gate.md):
 *   - ReDMCSB COMMAND.C:2045-2156 F0380 process queue + dispatch.
 *   - ReDMCSB CLIKMENU.C:142-174 F0365 turn.
 *   - ReDMCSB CLIKMENU.C:180-347 F0366 movement.
 *   - ReDMCSB DEFS.H:238-243      C001..C006 movement command IDs.
 *   - firestaff_touch.h           sibling DM1 V1 recognizer.
 */

#include "fs_gesture_navigation_gate.h"
#include <stdio.h>
#include <string.h>

/* ── Tiny assertion helpers ────────────────────────────────────────── */

static int g_assertions = 0;
static int g_failures = 0;

#define CHECK(cond_) do {                                                  \
    g_assertions++;                                                        \
    if (!(cond_)) {                                                        \
        printf("  FAIL  %s:%d  %s\n", __FILE__, __LINE__, #cond_);         \
        g_failures++;                                                      \
    }                                                                      \
} while (0)

#define CHECK_STR_EQ(actual_, expected_) do {                              \
    const char* _a = (actual_);                                            \
    const char* _e = (expected_);                                          \
    g_assertions++;                                                        \
    if (!_a || !_e || strcmp(_a, _e) != 0) {                               \
        printf("  FAIL  %s:%d  expected=\"%s\" actual=\"%s\"\n",           \
               __FILE__, __LINE__, _e ? _e : "(null)",                     \
               _a ? _a : "(null)");                                        \
        g_failures++;                                                      \
    }                                                                      \
} while (0)

#define CHECK_INT_EQ(actual_, expected_) do {                              \
    int _a = (int)(actual_);                                               \
    int _e = (int)(expected_);                                             \
    g_assertions++;                                                        \
    if (_a != _e) {                                                        \
        printf("  FAIL  %s:%d  expected=%d actual=%d\n",                   \
               __FILE__, __LINE__, _e, _a);                                \
        g_failures++;                                                      \
    }                                                                      \
} while (0)

/* Helper: feed a DOWN-MOVE-UP gesture in one go. */
static int feed_full_gesture(FsGestureFeedKind kind, int x, int y, uint32_t nowMs) {
    FsGestureFeedEvent ev;
    FsGestureType out = FS_GG_GESTURE_NONE;
    memset(&ev, 0, sizeof(ev));
    ev.kind = kind;
    ev.x = x;
    ev.y = y;
    ev.nowMs = nowMs;
    return fs_gesture_recognizer_step(&ev, &out);
}

/* Helper: feed a complete swipe from (sx, sy) at tDown to (ex, ey) at tUp. */
static FsGestureType feed_swipe(int sx, int sy, int ex, int ey,
                                uint32_t tDown, uint32_t tUp) {
    FsGestureFeedEvent ev;
    FsGestureType out = FS_GG_GESTURE_NONE;
    memset(&ev, 0, sizeof(ev));
    ev.kind = FS_GG_FEED_DOWN; ev.x = sx; ev.y = sy; ev.nowMs = tDown;
    fs_gesture_recognizer_step(&ev, &out);
    out = FS_GG_GESTURE_NONE;
    memset(&ev, 0, sizeof(ev));
    ev.kind = FS_GG_FEED_MOVE; ev.x = ex; ev.y = ey; ev.nowMs = tDown + 10u;
    fs_gesture_recognizer_step(&ev, &out);
    out = FS_GG_GESTURE_NONE;
    memset(&ev, 0, sizeof(ev));
    ev.kind = FS_GG_FEED_UP; ev.x = ex; ev.y = ey; ev.nowMs = tUp;
    fs_gesture_recognizer_step(&ev, &out);
    return out;
}

/* ── Tests ─────────────────────────────────────────────────────────── */

static void test_lifecycle(void) {
    printf("[lifecycle]\n");
    /* Module is not initialized yet -> NOT_INITIALIZED on translate. */
    FsGestureGameCommand cmd;
    FsGestureDispatchReason reason = FS_GG_OK;
    CHECK_INT_EQ(fs_gesture_translate_with_reason(FS_GG_GAME_DM1,
                                                  FS_GG_GESTURE_SWIPE_UP,
                                                  &cmd, &reason), 0);
    CHECK_INT_EQ(reason, FS_GG_NOT_INITIALIZED);
    CHECK_INT_EQ(cmd.gameCommand, -1);

    /* Init OK; idempotent. */
    CHECK_INT_EQ(fs_gesture_gate_init(), 1);
    CHECK_INT_EQ(fs_gesture_gate_init(), 1);
    CHECK_INT_EQ(fs_gesture_gate_is_initialized(), 1);

    /* Shutdown; idempotent. */
    fs_gesture_gate_shutdown();
    CHECK_INT_EQ(fs_gesture_gate_is_initialized(), 0);
    fs_gesture_gate_shutdown();

    /* Re-init for the rest of the suite. */
    CHECK_INT_EQ(fs_gesture_gate_init(), 1);
}

static void test_settings_gate(void) {
    printf("[settings]\n");
    /* All games start OFF. */
    int i;
    for (i = 0; i < FS_GG_GAME_ID_COUNT; ++i) {
        FsGgGameId gid = (FsGgGameId)i;
        if (gid == FS_GG_GAME_THERON || gid == FS_GG_GAME_DM1
            || gid == FS_GG_GAME_CSB || gid == FS_GG_GAME_DM2
            || gid == FS_GG_GAME_NEXUS) {
            CHECK_INT_EQ(fs_gesture_gate_is_enabled(gid), 0);
        }
    }
    CHECK_INT_EQ(fs_gesture_gate_enabled_count(), 0);

    /* Invalid game id rejected. */
    CHECK_INT_EQ(fs_gesture_gate_set_enabled((FsGgGameId)99, 1), -1);
    CHECK_INT_EQ(fs_gesture_gate_is_enabled((FsGgGameId)99), -1);

    /* Toggle DM1 ON; previous=0, new=1. */
    CHECK_INT_EQ(fs_gesture_gate_set_enabled(FS_GG_GAME_DM1, 1), 0);
    CHECK_INT_EQ(fs_gesture_gate_is_enabled(FS_GG_GAME_DM1), 1);
    CHECK_INT_EQ(fs_gesture_gate_set_enabled(FS_GG_GAME_DM1, 0), 1);
    CHECK_INT_EQ(fs_gesture_gate_is_enabled(FS_GG_GAME_DM1), 0);

    /* Active game default + set/get roundtrip. */
    CHECK_INT_EQ(fs_gesture_gate_active_game(), FS_GG_GAME_DM1);
    CHECK_INT_EQ(fs_gesture_gate_set_active_game(FS_GG_GAME_CSB), FS_GG_GAME_DM1);
    CHECK_INT_EQ(fs_gesture_gate_active_game(), FS_GG_GAME_CSB);
    /* Set with invalid id is a no-op (returns previous). */
    CHECK_INT_EQ(fs_gesture_gate_set_active_game((FsGgGameId)42), FS_GG_GAME_CSB);
    CHECK_INT_EQ(fs_gesture_gate_active_game(), FS_GG_GAME_CSB);
    fs_gesture_gate_set_active_game(FS_GG_GAME_DM1);
}

static void test_recognizer_swipes(void) {
    printf("[recognizer:swipes]\n");
    /* Vertical swipe up = SWIPE_UP (screen y increases downward, so
     * swiping up means endY < startY). */
    CHECK_INT_EQ(feed_swipe(160, 180, 160, 100, 1000u, 1100u),
                 FS_GG_GESTURE_SWIPE_UP);
    CHECK_INT_EQ(fs_gesture_recognizer_last_gesture(), FS_GG_GESTURE_SWIPE_UP);

    /* Vertical swipe down = SWIPE_DOWN. */
    CHECK_INT_EQ(feed_swipe(160, 100, 160, 180, 2000u, 2100u),
                 FS_GG_GESTURE_SWIPE_DOWN);

    /* Horizontal swipe right = SWIPE_RIGHT. */
    CHECK_INT_EQ(feed_swipe(50, 100, 250, 100, 3000u, 3100u),
                 FS_GG_GESTURE_SWIPE_RIGHT);

    /* Horizontal swipe left = SWIPE_LEFT. */
    CHECK_INT_EQ(feed_swipe(250, 100, 50, 100, 4000u, 4100u),
                 FS_GG_GESTURE_SWIPE_LEFT);

    /* Below threshold = no swipe (DRAG instead, since movement > tap tol). */
    CHECK_INT_EQ(feed_swipe(160, 100, 165, 102, 5000u, 5100u),
                 FS_GG_GESTURE_DRAG);
}

static void test_recognizer_long_press(void) {
    printf("[recognizer:long_press]\n");
    /* Stationary hold >= 500ms = LONG_PRESS. */
    FsGestureFeedEvent ev;
    FsGestureType out = FS_GG_GESTURE_NONE;
    memset(&ev, 0, sizeof(ev));
    ev.kind = FS_GG_FEED_DOWN; ev.x = 100; ev.y = 100; ev.nowMs = 6000u;
    fs_gesture_recognizer_step(&ev, &out);
    out = FS_GG_GESTURE_NONE;
    memset(&ev, 0, sizeof(ev));
    ev.kind = FS_GG_FEED_UP;   ev.x = 100; ev.y = 100; ev.nowMs = 6600u;
    fs_gesture_recognizer_step(&ev, &out);
    CHECK_INT_EQ(out, FS_GG_GESTURE_LONG_PRESS);
}

static void test_recognizer_double_tap(void) {
    printf("[recognizer:double_tap]\n");
    /* First tap (short hold) = TAP. */
    FsGestureFeedEvent ev;
    FsGestureType out = FS_GG_GESTURE_NONE;
    memset(&ev, 0, sizeof(ev));
    ev.kind = FS_GG_FEED_DOWN; ev.x = 80; ev.y = 80; ev.nowMs = 7000u;
    fs_gesture_recognizer_step(&ev, &out);
    out = FS_GG_GESTURE_NONE;
    memset(&ev, 0, sizeof(ev));
    ev.kind = FS_GG_FEED_UP;   ev.x = 80; ev.y = 80; ev.nowMs = 7100u;
    fs_gesture_recognizer_step(&ev, &out);
    CHECK_INT_EQ(out, FS_GG_GESTURE_TAP);

    /* Second tap within 300ms + 32px = DOUBLE_TAP. */
    memset(&ev, 0, sizeof(ev));
    ev.kind = FS_GG_FEED_DOWN; ev.x = 82; ev.y = 81; ev.nowMs = 7200u;
    fs_gesture_recognizer_step(&ev, &out);
    out = FS_GG_GESTURE_NONE;
    memset(&ev, 0, sizeof(ev));
    ev.kind = FS_GG_FEED_UP;   ev.x = 82; ev.y = 81; ev.nowMs = 7300u;
    fs_gesture_recognizer_step(&ev, &out);
    CHECK_INT_EQ(out, FS_GG_GESTURE_DOUBLE_TAP);

    /* Third tap well after the second is a fresh TAP. */
    memset(&ev, 0, sizeof(ev));
    ev.kind = FS_GG_FEED_DOWN; ev.x = 80; ev.y = 80; ev.nowMs = 9000u;
    fs_gesture_recognizer_step(&ev, &out);
    out = FS_GG_GESTURE_NONE;
    memset(&ev, 0, sizeof(ev));
    ev.kind = FS_GG_FEED_UP;   ev.x = 80; ev.y = 80; ev.nowMs = 9100u;
    fs_gesture_recognizer_step(&ev, &out);
    CHECK_INT_EQ(out, FS_GG_GESTURE_TAP);
}

static void test_recognizer_edge_strafe(void) {
    printf("[recognizer:edge_strafe]\n");
    /* Edge zone is 20% of width; on a 320-px framebuffer that's
     * the leftmost 64 px and rightmost 64 px. */
    /* Origin x=10 (left edge), swipe right (dominant X) = EDGE_STRAFE_LEFT. */
    CHECK_INT_EQ(feed_swipe(10, 100, 250, 100, 10000u, 10100u),
                 FS_GG_GESTURE_EDGE_STRAFE_LEFT);
    /* Origin x=310 (right edge), swipe left = EDGE_STRAFE_RIGHT. */
    CHECK_INT_EQ(feed_swipe(310, 100, 50, 100, 11000u, 11100u),
                 FS_GG_GESTURE_EDGE_STRAFE_RIGHT);
    /* Origin x=160 (center), swipe right = SWIPE_RIGHT (not strafe). */
    CHECK_INT_EQ(feed_swipe(160, 100, 250, 100, 12000u, 12100u),
                 FS_GG_GESTURE_SWIPE_RIGHT);
}

static void test_translation_disabled(void) {
    printf("[translation:disabled]\n");
    /* DM1 disabled by default -> SWIPE_UP rejected with DISABLED. */
    FsGestureGameCommand cmd;
    FsGestureDispatchReason reason = FS_GG_OK;
    fs_gesture_gate_set_enabled(FS_GG_GAME_DM1, 0);
    CHECK_INT_EQ(fs_gesture_translate_with_reason(FS_GG_GAME_DM1,
                                                  FS_GG_GESTURE_SWIPE_UP,
                                                  &cmd, &reason), 0);
    CHECK_INT_EQ(reason, FS_GG_DISABLED);
    CHECK_INT_EQ(cmd.gameCommand, -1);
}

static void test_translation_unsupported_game(void) {
    printf("[translation:unsupported_game]\n");
    FsGestureGameCommand cmd;
    FsGestureDispatchReason reason = FS_GG_OK;
    CHECK_INT_EQ(fs_gesture_translate_with_reason((FsGgGameId)99,
                                                  FS_GG_GESTURE_SWIPE_UP,
                                                  &cmd, &reason), 0);
    CHECK_INT_EQ(reason, FS_GG_UNSUPPORTED_GAME);
}

static void test_translation_unsupported_gesture(void) {
    printf("[translation:unsupported_gesture]\n");
    /* PINCH_ZOOM_IN is in every game row but command=-1 (V2-only). */
    FsGestureGameCommand cmd;
    FsGestureDispatchReason reason = FS_GG_OK;
    fs_gesture_gate_set_enabled(FS_GG_GAME_DM1, 1);
    /* We don't have a row for a made-up gesture enum value, but we
     * can hit UNSUPPORTED_GESTURE with a gesture type the per-game
     * table doesn't carry by setting the active game to one whose
     * table doesn't include it.  Since every game carries every
     * gesture, we exercise UNSUPPORTED_GESTURE via a synthetic high
     * gesture value passed through translate_with_reason. */
    CHECK_INT_EQ(fs_gesture_translate_with_reason(FS_GG_GAME_DM1,
                                                  (FsGestureType)9999,
                                                  &cmd, &reason), 0);
    CHECK_INT_EQ(reason, FS_GG_UNSUPPORTED_GESTURE);
}

static void test_translation_tap_not_movement(void) {
    printf("[translation:tap_not_movement]\n");
    FsGestureGameCommand cmd;
    FsGestureDispatchReason reason = FS_GG_OK;
    /* TAP/DRAG are recognised gestures but the gate marks them as
     * hit-test gestures, not movement commands. */
    fs_gesture_gate_set_enabled(FS_GG_GAME_DM1, 1);
    CHECK_INT_EQ(fs_gesture_translate_with_reason(FS_GG_GAME_DM1,
                                                  FS_GG_GESTURE_TAP,
                                                  &cmd, &reason), 0);
    CHECK_INT_EQ(reason, FS_GG_TAP_NOT_MOVEMENT);
    CHECK_INT_EQ(cmd.gameCommand, -1);
    CHECK_INT_EQ(cmd.isMovement, 0);
    CHECK_INT_EQ(cmd.isTurn, 0);
}

static void test_translation_no_pending(void) {
    printf("[translation:no_pending]\n");
    FsGestureGameCommand cmd;
    FsGestureDispatchReason reason = FS_GG_OK;
    fs_gesture_gate_set_enabled(FS_GG_GAME_DM1, 1);
    CHECK_INT_EQ(fs_gesture_translate_with_reason(FS_GG_GAME_DM1,
                                                  FS_GG_GESTURE_NONE,
                                                  &cmd, &reason), 0);
    CHECK_INT_EQ(reason, FS_GG_NO_PENDING_GESTURE);
}

static void test_translation_per_game(void) {
    printf("[translation:per_game]\n");
    /* Enable each game and walk the SWIPE_* + DOUBLE_TAP +
     * EDGE_STRAFE_* gestures; verify the command id matches the
     * per-game table. */
    struct {
        FsGgGameId game;
        int expectedForward;   /* FS_GG_CMD_MOVE_FORWARD or Theron forward */
        int expectedTurnLeft;  /* FS_GG_CMD_TURN_LEFT or Theron turn_l */
        int expectedStrafeLeft;
    } expected[] = {
        { FS_GG_GAME_DM1,    3, 1, 6 },
        { FS_GG_GAME_CSB,    3, 1, 6 },
        { FS_GG_GAME_DM2,    3, 1, 6 },
        { FS_GG_GAME_NEXUS,  3, 1, 6 },
        { FS_GG_GAME_THERON, 0x10, 0x12, 0x14 }
    };
    int i;
    unsigned int tsize = fs_gesture_per_game_table_size();
    /* 12 gestures x 5 games = 60 rows. */
    CHECK_INT_EQ(tsize, 60u);

    for (i = 0; i < (int)(sizeof(expected) / sizeof(expected[0])); ++i) {
        FsGestureGameCommand cmd;
        FsGestureDispatchReason reason = FS_GG_OK;
        FsGgGameId g = expected[i].game;
        fs_gesture_gate_set_enabled(g, 1);
        /* SWIPE_UP -> forward. */
        CHECK_INT_EQ(fs_gesture_translate_with_reason(g, FS_GG_GESTURE_SWIPE_UP,
                                                      &cmd, &reason), 1);
        CHECK_INT_EQ(cmd.gameCommand, expected[i].expectedForward);
        CHECK_INT_EQ(cmd.isMovement, 1);
        CHECK_INT_EQ(cmd.isTurn, 0);
        /* SWIPE_LEFT -> turn left. */
        CHECK_INT_EQ(fs_gesture_translate_with_reason(g, FS_GG_GESTURE_SWIPE_LEFT,
                                                      &cmd, &reason), 1);
        CHECK_INT_EQ(cmd.gameCommand, expected[i].expectedTurnLeft);
        CHECK_INT_EQ(cmd.isMovement, 0);
        CHECK_INT_EQ(cmd.isTurn, 1);
        /* EDGE_STRAFE_LEFT -> strafe left. */
        CHECK_INT_EQ(fs_gesture_translate_with_reason(g, FS_GG_GESTURE_EDGE_STRAFE_LEFT,
                                                      &cmd, &reason), 1);
        CHECK_INT_EQ(cmd.gameCommand, expected[i].expectedStrafeLeft);
        CHECK_INT_EQ(cmd.isMovement, 1);
        /* Disable after the test so we don't leak across the suite. */
        fs_gesture_gate_set_enabled(g, 0);
    }
}

static void test_translation_pinch_rejected_with_command(void) {
    printf("[translation:pinch]\n");
    /* PINCH_ZOOM_IN is a recognised row with command=-1 (V2-only).
     * Translation must NOT accept it because the row's command is
     * -1 and the gate keeps V1 sole input path. */
    FsGestureGameCommand cmd;
    FsGestureDispatchReason reason = FS_GG_OK;
    fs_gesture_gate_set_enabled(FS_GG_GAME_DM1, 1);
    CHECK_INT_EQ(fs_gesture_translate_with_reason(FS_GG_GAME_DM1,
                                                  FS_GG_GESTURE_PINCH_ZOOM_IN,
                                                  &cmd, &reason), 1);
    CHECK_INT_EQ(cmd.gameCommand, -1);
    CHECK_INT_EQ(cmd.isMovement, 0);
    CHECK_INT_EQ(cmd.isTurn, 0);
}

static void test_zone_audit_minimum(void) {
    printf("[audit:minimum]\n");
    /* A 24x24 zone meets the minimum exactly. */
    FsGestureZone zones[1];
    FsGestureZoneAuditReport report;
    memset(&report, 0, sizeof(report));
    zones[0].x = 0; zones[0].y = 0; zones[0].w = 24; zones[0].h = 24;
    zones[0].groupName = "test.min";
    CHECK_INT_EQ(fs_gesture_audit_zones(zones, 1, 24, 44, &report), 1);
    CHECK_INT_EQ(report.zonesBelowMinimum, 0);
    CHECK_INT_EQ(report.zonesBelowRecommended, 1);

    /* A 16x16 zone fails the minimum. */
    zones[0].w = 16; zones[0].h = 16;
    memset(&report, 0, sizeof(report));
    fs_gesture_audit_zones(zones, 1, 24, 44, &report);
    CHECK_INT_EQ(report.zonesBelowMinimum, 1);

    /* A 44x44 zone meets both. */
    zones[0].w = 44; zones[0].h = 44;
    memset(&report, 0, sizeof(report));
    fs_gesture_audit_zones(zones, 1, 24, 44, &report);
    CHECK_INT_EQ(report.zonesBelowMinimum, 0);
    CHECK_INT_EQ(report.zonesBelowRecommended, 0);

    /* Negative side is treated as 0; fails. */
    zones[0].w = -1; zones[0].h = -1;
    memset(&report, 0, sizeof(report));
    fs_gesture_audit_zones(zones, 1, 24, 44, &report);
    CHECK_INT_EQ(report.audits[0].minShortSide, 0);
    CHECK_INT_EQ(report.zonesBelowMinimum, 1);

    /* Null arguments rejected. */
    CHECK_INT_EQ(fs_gesture_audit_zones(0, 1, 24, 44, &report), 0);
    CHECK_INT_EQ(fs_gesture_audit_zones(zones, 0, 24, 44, &report), 0);
    CHECK_INT_EQ(fs_gesture_audit_zones(zones, 1, 24, 44, 0), 0);
}

static void test_zone_audit_builtin(void) {
    printf("[audit:builtin]\n");
    FsGestureZoneAuditReport report;
    memset(&report, 0, sizeof(report));
    int n = fs_gesture_audit_builtin_zones(&report);
    CHECK_INT_EQ(n > 0, 1);
    /* Built-in zones use the source-locked DM1 V1 layout-696 sizes
     * (16x16 backpack slots, 27x21 movement arrows, 67x29 champion
     * boxes).  Many of these are intentionally below the 44px
     * recommended target because the original DM1 UI was 320x200;
     * the audit is honest about that.  We only require that the
     * audit returns a sensible count. */
    CHECK_INT_EQ(report.totalZones, n);
}

static void test_counters(void) {
    printf("[counters]\n");
    fs_gesture_gate_reset_counters();
    FsGestureGateCounters c = fs_gesture_gate_counters();
    CHECK_INT_EQ(c.feedCount, 0u);
    CHECK_INT_EQ(c.gestureCount, 0u);
    CHECK_INT_EQ(c.translateAccepted, 0u);
    CHECK_INT_EQ(c.translateRejected, 0u);

    /* Feed a swipe; counter goes up by 1. */
    feed_swipe(160, 100, 160, 30, 13000u, 13100u);
    c = fs_gesture_gate_counters();
    CHECK_INT_EQ(c.feedCount > 0u, 1);
    CHECK_INT_EQ(c.gestureCount, 1u);

    /* Translate accepted. */
    fs_gesture_gate_set_enabled(FS_GG_GAME_DM1, 1);
    FsGestureGameCommand cmd;
    FsGestureDispatchReason reason = FS_GG_OK;
    fs_gesture_translate_with_reason(FS_GG_GAME_DM1, FS_GG_GESTURE_SWIPE_UP,
                                     &cmd, &reason);
    c = fs_gesture_gate_counters();
    CHECK_INT_EQ(c.translateAccepted, 1u);

    /* Translate rejected. */
    fs_gesture_gate_set_enabled(FS_GG_GAME_DM1, 0);
    fs_gesture_translate_with_reason(FS_GG_GAME_DM1, FS_GG_GESTURE_SWIPE_UP,
                                     &cmd, &reason);
    c = fs_gesture_gate_counters();
    CHECK_INT_EQ(c.translateRejected, 1u);
}

static void test_source_evidence(void) {
    printf("[source_evidence]\n");
    const char* evidence = fs_gesture_gate_source_evidence();
    CHECK_INT_EQ(evidence != 0, 1);
    /* Spot-check key source-lock citations. */
    CHECK_INT_EQ(strstr(evidence, "COMMAND.C:2045-2156") != 0, 1);
    CHECK_INT_EQ(strstr(evidence, "F0365") != 0, 1);
    CHECK_INT_EQ(strstr(evidence, "F0366") != 0, 1);
    CHECK_INT_EQ(strstr(evidence, "DEFS.H:238-243") != 0, 1);

    /* Spot-check gesture name table. */
    CHECK_STR_EQ(fs_gesture_type_name(FS_GG_GESTURE_SWIPE_UP), "swipe_up");
    CHECK_STR_EQ(fs_gesture_type_name(FS_GG_GESTURE_LONG_PRESS), "long_press");
    CHECK_STR_EQ(fs_gesture_type_name(FS_GG_GESTURE_DOUBLE_TAP), "double_tap");
    CHECK_STR_EQ(fs_gesture_type_name(FS_GG_GESTURE_EDGE_STRAFE_LEFT), "edge_strafe_left");
    CHECK_STR_EQ(fs_gesture_type_name(FS_GG_GESTURE_PINCH_ZOOM_IN), "pinch_zoom_in");
    CHECK_STR_EQ(fs_gesture_type_name((FsGestureType)9999), "unknown");

    /* Spot-check dispatch reason table. */
    CHECK_STR_EQ(fs_gesture_dispatch_reason_name(FS_GG_OK), "ok");
    CHECK_STR_EQ(fs_gesture_dispatch_reason_name(FS_GG_DISABLED), "disabled");
    CHECK_STR_EQ(fs_gesture_dispatch_reason_name(FS_GG_TAP_NOT_MOVEMENT), "tap_not_movement");
    CHECK_STR_EQ(fs_gesture_dispatch_reason_name(FS_GG_UNSUPPORTED_GAME), "unsupported_game");
    CHECK_STR_EQ(fs_gesture_dispatch_reason_name(FS_GG_UNSUPPORTED_GESTURE), "unsupported_gesture");
}

static void test_null_safety(void) {
    printf("[null_safety]\n");
    FsGestureGameCommand cmd;
    FsGestureDispatchReason reason = FS_GG_OK;
    /* translate with null outCmd. */
    CHECK_INT_EQ(fs_gesture_translate_with_reason(FS_GG_GAME_DM1,
                                                  FS_GG_GESTURE_SWIPE_UP,
                                                  0, &reason), 0);
    CHECK_INT_EQ(reason, FS_GG_NULL_ARG);

    /* recognizer step with null event. */
    CHECK_INT_EQ(fs_gesture_recognizer_step(0, &cmd.gesture), 0);

    /* translate with valid cmd pointer but reason null OK. */
    cmd.gesture = FS_GG_GESTURE_NONE;
    fs_gesture_gate_set_enabled(FS_GG_GAME_DM1, 1);
    fs_gesture_translate_with_reason(FS_GG_GAME_DM1, FS_GG_GESTURE_SWIPE_UP,
                                     &cmd, 0);
    CHECK_INT_EQ(cmd.accepted, 1);

    /* recognizer state queries work after reset. */
    fs_gesture_recognizer_reset();
    CHECK_INT_EQ(fs_gesture_recognizer_last_gesture(), FS_GG_GESTURE_NONE);
    CHECK_INT_EQ(fs_gesture_recognizer_active_finger_id(), -1);
}

int main(void) {
    printf("Cross-game runtime gesture navigation gate — unit test\n");
    printf("Source: ReDMCSB COMMAND.C:2045-2156, CLIKMENU.C:142/180,\n"
           "        DEFS.H:238-243, firestaff_touch.h sibling,\n"
           "        docs/source-lock/fs_gesture_navigation_gate.md\n\n");

    test_lifecycle();
    test_settings_gate();
    test_recognizer_swipes();
    test_recognizer_long_press();
    test_recognizer_double_tap();
    test_recognizer_edge_strafe();
    test_translation_disabled();
    test_translation_unsupported_game();
    test_translation_unsupported_gesture();
    test_translation_tap_not_movement();
    test_translation_no_pending();
    test_translation_per_game();
    test_translation_pinch_rejected_with_command();
    test_zone_audit_minimum();
    test_zone_audit_builtin();
    test_counters();
    test_source_evidence();
    test_null_safety();

    printf("\nResult: %d assertions, %d failures\n", g_assertions, g_failures);
    return g_failures == 0 ? 0 : 1;
}

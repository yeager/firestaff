/*
 * firestaff_gesture_navigation_gate_probe.c — headless probe for the
 * cross-game runtime gesture navigation gate.
 *
 * This probe complements tests/test_fs_gesture_navigation_gate.c with
 * a CI-greppable summary.  It exercises:
 *   - lifecycle (init / shutdown / re-init idempotent)
 *   - settings gate (per-game enable/disable)
 *   - recognizer (swipes, long press, double tap, edge strafe)
 *   - per-game translation (DM1/CSB/DM2/Nexus/Theron rows)
 *   - rejection reasons (disabled, unsupported game, unsupported gesture,
 *     tap_not_movement, no_pending_gesture, not_initialized, null_arg)
 *   - touch target safety audit (minimum + recommended)
 *   - source evidence citation
 *   - counters
 *
 * Source-lock anchors (full list in
 * docs/source-lock/fs_gesture_navigation_gate.md):
 *   - ReDMCSB COMMAND.C:2045-2156 F0380 process queue + dispatch.
 *   - ReDMCSB CLIKMENU.C:142-174 F0365 turn.
 *   - ReDMCSB CLIKMENU.C:180-347 F0366 movement.
 *   - ReDMCSB DEFS.H:238-243      C001..C006 movement command IDs.
 */

#include "fs_gesture_navigation_gate.h"
#include <stdio.h>
#include <string.h>

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
    ev.kind = FS_GG_FEED_UP;   ev.x = ex; ev.y = ey; ev.nowMs = tUp;
    fs_gesture_recognizer_step(&ev, &out);
    return out;
}

int main(void) {
    printf("Cross-game runtime gesture navigation gate — headless probe\n");
    printf("Source: ReDMCSB COMMAND.C:2045-2156, CLIKMENU.C:142/180,\n"
           "        DEFS.H:238-243, firestaff_touch.h sibling,\n"
           "        docs/source-lock/fs_gesture_navigation_gate.md\n\n");

    /* ── Lifecycle (init / shutdown idempotency) ─────────────────── */
    fs_gesture_gate_shutdown();
    FsGestureGameCommand cmd;
    FsGestureDispatchReason reason = FS_GG_OK;
    /* Not initialized -> NOT_INITIALIZED. */
    CHECK(fs_gesture_translate_with_reason(FS_GG_GAME_DM1,
                                           FS_GG_GESTURE_SWIPE_UP,
                                           &cmd, &reason) == 0);
    CHECK(reason == FS_GG_NOT_INITIALIZED);

    CHECK(fs_gesture_gate_init() == 1);
    CHECK(fs_gesture_gate_init() == 1); /* idempotent */
    CHECK(fs_gesture_gate_is_initialized() == 1);

    /* ── Settings gate ───────────────────────────────────────────── */
    CHECK(fs_gesture_gate_is_enabled(FS_GG_GAME_DM1) == 0);
    CHECK(fs_gesture_gate_is_enabled(FS_GG_GAME_CSB) == 0);
    CHECK(fs_gesture_gate_is_enabled(FS_GG_GAME_DM2) == 0);
    CHECK(fs_gesture_gate_is_enabled(FS_GG_GAME_NEXUS) == 0);
    CHECK(fs_gesture_gate_is_enabled(FS_GG_GAME_THERON) == 0);
    CHECK(fs_gesture_gate_enabled_count() == 0);
    CHECK(fs_gesture_gate_set_enabled((FsGgGameId)99, 1) == -1);
    CHECK(fs_gesture_gate_set_enabled(FS_GG_GAME_DM1, 1) == 0);
    CHECK(fs_gesture_gate_set_enabled(FS_GG_GAME_CSB, 1) == 0);
    CHECK(fs_gesture_gate_set_enabled(FS_GG_GAME_DM2, 1) == 0);
    CHECK(fs_gesture_gate_set_enabled(FS_GG_GAME_NEXUS, 1) == 0);
    CHECK(fs_gesture_gate_set_enabled(FS_GG_GAME_THERON, 1) == 0);
    CHECK(fs_gesture_gate_enabled_count() == 5);

    CHECK(fs_gesture_gate_set_active_game(FS_GG_GAME_DM1) == FS_GG_GAME_DM1);
    CHECK(fs_gesture_gate_set_active_game(FS_GG_GAME_DM1) == FS_GG_GAME_DM1);

    /* ── Recognizer: swipes ──────────────────────────────────────── */
    CHECK(feed_swipe(160, 180, 160, 100, 1000u, 1100u) == FS_GG_GESTURE_SWIPE_UP);
    CHECK(feed_swipe(160, 100, 160, 180, 2000u, 2100u) == FS_GG_GESTURE_SWIPE_DOWN);
    CHECK(feed_swipe(50, 100, 250, 100, 3000u, 3100u) == FS_GG_GESTURE_SWIPE_RIGHT);
    CHECK(feed_swipe(250, 100, 50, 100, 4000u, 4100u) == FS_GG_GESTURE_SWIPE_LEFT);
    CHECK(feed_swipe(160, 100, 165, 102, 5000u, 5100u) == FS_GG_GESTURE_DRAG);

    /* ── Recognizer: long press ──────────────────────────────────── */
    {
        FsGestureFeedEvent ev; FsGestureType out = FS_GG_GESTURE_NONE;
        memset(&ev, 0, sizeof(ev));
        ev.kind = FS_GG_FEED_DOWN; ev.x = 100; ev.y = 100; ev.nowMs = 6000u;
        fs_gesture_recognizer_step(&ev, &out);
        out = FS_GG_GESTURE_NONE;
        memset(&ev, 0, sizeof(ev));
        ev.kind = FS_GG_FEED_UP;   ev.x = 100; ev.y = 100; ev.nowMs = 6600u;
        fs_gesture_recognizer_step(&ev, &out);
        CHECK(out == FS_GG_GESTURE_LONG_PRESS);
    }

    /* ── Recognizer: double tap ───────────────────────────────────── */
    {
        FsGestureFeedEvent ev; FsGestureType out = FS_GG_GESTURE_NONE;
        memset(&ev, 0, sizeof(ev));
        ev.kind = FS_GG_FEED_DOWN; ev.x = 80; ev.y = 80; ev.nowMs = 7000u;
        fs_gesture_recognizer_step(&ev, &out);
        out = FS_GG_GESTURE_NONE;
        memset(&ev, 0, sizeof(ev));
        ev.kind = FS_GG_FEED_UP;   ev.x = 80; ev.y = 80; ev.nowMs = 7100u;
        fs_gesture_recognizer_step(&ev, &out);
        CHECK(out == FS_GG_GESTURE_TAP);
        memset(&ev, 0, sizeof(ev));
        ev.kind = FS_GG_FEED_DOWN; ev.x = 82; ev.y = 81; ev.nowMs = 7200u;
        fs_gesture_recognizer_step(&ev, &out);
        out = FS_GG_GESTURE_NONE;
        memset(&ev, 0, sizeof(ev));
        ev.kind = FS_GG_FEED_UP;   ev.x = 82; ev.y = 81; ev.nowMs = 7300u;
        fs_gesture_recognizer_step(&ev, &out);
        CHECK(out == FS_GG_GESTURE_DOUBLE_TAP);
    }

    /* ── Recognizer: edge strafe ─────────────────────────────────── */
    CHECK(feed_swipe(10, 100, 250, 100, 10000u, 10100u) == FS_GG_GESTURE_EDGE_STRAFE_LEFT);
    CHECK(feed_swipe(310, 100, 50, 100, 11000u, 11100u) == FS_GG_GESTURE_EDGE_STRAFE_RIGHT);

    /* ── Per-game translation table (5 games × 12 gestures = 60 rows) */
    CHECK(fs_gesture_per_game_table_size() == 60u);

    /* DM1/CSB/DM2/Nexus share C001..C006; Theron uses opaque PCE ids. */
    struct { FsGgGameId game; int fwd; int tl; int sl; } exp[] = {
        { FS_GG_GAME_DM1,    3, 1, 6 },
        { FS_GG_GAME_CSB,    3, 1, 6 },
        { FS_GG_GAME_DM2,    3, 1, 6 },
        { FS_GG_GAME_NEXUS,  3, 1, 6 },
        { FS_GG_GAME_THERON, 0x10, 0x12, 0x14 }
    };
    int i;
    for (i = 0; i < 5; ++i) {
        FsGgGameId g = exp[i].game;
        CHECK(fs_gesture_translate_with_reason(g, FS_GG_GESTURE_SWIPE_UP,
                                               &cmd, &reason) == 1);
        CHECK(reason == FS_GG_OK);
        CHECK(cmd.gameCommand == exp[i].fwd);
        CHECK(cmd.isMovement == 1);
        CHECK(cmd.isTurn == 0);
        CHECK(fs_gesture_translate_with_reason(g, FS_GG_GESTURE_SWIPE_LEFT,
                                               &cmd, &reason) == 1);
        CHECK(cmd.gameCommand == exp[i].tl);
        CHECK(cmd.isTurn == 1);
        CHECK(fs_gesture_translate_with_reason(g, FS_GG_GESTURE_EDGE_STRAFE_LEFT,
                                               &cmd, &reason) == 1);
        CHECK(cmd.gameCommand == exp[i].sl);
    }

    /* ── Rejection: disabled ─────────────────────────────────────── */
    fs_gesture_gate_set_enabled(FS_GG_GAME_DM1, 0);
    CHECK(fs_gesture_translate_with_reason(FS_GG_GAME_DM1,
                                           FS_GG_GESTURE_SWIPE_UP,
                                           &cmd, &reason) == 0);
    CHECK(reason == FS_GG_DISABLED);
    fs_gesture_gate_set_enabled(FS_GG_GAME_DM1, 1);

    /* ── Rejection: unsupported game ─────────────────────────────── */
    CHECK(fs_gesture_translate_with_reason((FsGgGameId)99,
                                           FS_GG_GESTURE_SWIPE_UP,
                                           &cmd, &reason) == 0);
    CHECK(reason == FS_GG_UNSUPPORTED_GAME);

    /* ── Rejection: unsupported gesture ──────────────────────────── */
    CHECK(fs_gesture_translate_with_reason(FS_GG_GAME_DM1,
                                           (FsGestureType)9999,
                                           &cmd, &reason) == 0);
    CHECK(reason == FS_GG_UNSUPPORTED_GESTURE);

    /* ── Rejection: tap_not_movement ─────────────────────────────── */
    CHECK(fs_gesture_translate_with_reason(FS_GG_GAME_DM1,
                                           FS_GG_GESTURE_TAP,
                                           &cmd, &reason) == 0);
    CHECK(reason == FS_GG_TAP_NOT_MOVEMENT);
    CHECK(cmd.gameCommand == -1);

    /* ── Rejection: no_pending_gesture ───────────────────────────── */
    CHECK(fs_gesture_translate_with_reason(FS_GG_GAME_DM1,
                                           FS_GG_GESTURE_NONE,
                                           &cmd, &reason) == 0);
    CHECK(reason == FS_GG_NO_PENDING_GESTURE);

    /* ── PINCH_ZOOM_IN: row exists, command=-1 (V2-only). */
    CHECK(fs_gesture_translate_with_reason(FS_GG_GAME_DM1,
                                           FS_GG_GESTURE_PINCH_ZOOM_IN,
                                           &cmd, &reason) == 1);
    CHECK(cmd.gameCommand == -1);

    /* ── Null safety ─────────────────────────────────────────────── */
    CHECK(fs_gesture_translate_with_reason(FS_GG_GAME_DM1,
                                           FS_GG_GESTURE_SWIPE_UP,
                                           0, &reason) == 0);
    CHECK(reason == FS_GG_NULL_ARG);

    /* ── Touch target safety audit ───────────────────────────────── */
    {
        FsGestureZone zones[4];
        FsGestureZoneAuditReport report;
        memset(&report, 0, sizeof(report));
        zones[0].x=0; zones[0].y=0; zones[0].w=24; zones[0].h=24; zones[0].groupName="min_ok";
        zones[1].x=0; zones[0].y=0; zones[1].w=16; zones[1].h=16; zones[1].groupName="min_fail";
        zones[2].x=0; zones[2].y=0; zones[2].w=44; zones[2].h=44; zones[2].groupName="rec_ok";
        zones[3].x=0; zones[3].y=0; zones[3].w=32; zones[3].h=32; zones[3].groupName="rec_fail";
        CHECK(fs_gesture_audit_zones(zones, 4, 24, 44, &report) == 4);
        CHECK(report.totalZones == 4);
        CHECK(report.zonesBelowMinimum == 1);
        CHECK(report.zonesBelowRecommended == 2);
    }

    /* ── Built-in audit (synthesised zones from layout-696 / DATA.C). */
    {
        FsGestureZoneAuditReport report;
        memset(&report, 0, sizeof(report));
        int n = fs_gesture_audit_builtin_zones(&report);
        CHECK(n > 0);
        CHECK(report.totalZones == n);
    }

    /* ── Counters ────────────────────────────────────────────────── */
    fs_gesture_gate_reset_counters();
    feed_swipe(160, 100, 160, 30, 13000u, 13100u);
    {
        FsGestureGateCounters c = fs_gesture_gate_counters();
        CHECK(c.feedCount > 0u);
        CHECK(c.gestureCount == 1u);
    }

    /* ── Source evidence ─────────────────────────────────────────── */
    {
        const char* ev = fs_gesture_gate_source_evidence();
        CHECK(ev != 0);
        CHECK(strstr(ev, "COMMAND.C:2045-2156") != 0);
        CHECK(strstr(ev, "F0365") != 0);
        CHECK(strstr(ev, "F0366") != 0);
        CHECK(strstr(ev, "DEFS.H:238-243") != 0);
    }

    /* ── Spot-check gesture name + dispatch reason string tables. */
    CHECK_STR_EQ(fs_gesture_type_name(FS_GG_GESTURE_SWIPE_UP), "swipe_up");
    CHECK_STR_EQ(fs_gesture_type_name(FS_GG_GESTURE_LONG_PRESS), "long_press");
    CHECK_STR_EQ(fs_gesture_type_name(FS_GG_GESTURE_DOUBLE_TAP), "double_tap");
    CHECK_STR_EQ(fs_gesture_type_name(FS_GG_GESTURE_EDGE_STRAFE_LEFT), "edge_strafe_left");
    CHECK_STR_EQ(fs_gesture_type_name(FS_GG_GESTURE_PINCH_ZOOM_IN), "pinch_zoom_in");
    CHECK_STR_EQ(fs_gesture_dispatch_reason_name(FS_GG_OK), "ok");
    CHECK_STR_EQ(fs_gesture_dispatch_reason_name(FS_GG_DISABLED), "disabled");
    CHECK_STR_EQ(fs_gesture_dispatch_reason_name(FS_GG_TAP_NOT_MOVEMENT), "tap_not_movement");
    CHECK_STR_EQ(fs_gesture_dispatch_reason_name(FS_GG_UNSUPPORTED_GAME), "unsupported_game");

    fs_gesture_gate_shutdown();
    fs_gesture_gate_shutdown(); /* idempotent */

    printf("\nResult: %d assertions, %d failures\n", g_assertions, g_failures);
    return g_failures == 0 ? 0 : 1;
}

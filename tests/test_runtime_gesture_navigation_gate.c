/*
 * test_runtime_gesture_navigation_gate.c
 *
 * Data-free CTest unit for the cross-game runtime gesture navigation
 * gate (include/runtime_gesture_navigation_gate.h + src/engine/
 * runtime_gesture_navigation_gate.c).
 *
 * Source of truth:
 *   - ReDMCSB COMMAND.C:2045-2155 F0380_COMMAND_ProcessQueue_CPSC
 *     (movement/turn queue dispatch, gate -> FS_CMD_* mapping)
 *   - ReDMCSB CLIKMENU.C:142-174 F0365 turn / 180-390 F0366 move
 *   - ReDMCSB GAMELOOP.C:164-219 V1 input wait loop (V1 parity)
 *   - ReDMCSB DEFS.H:238-243 C001..C006 movement commands
 *   - firestaff_touch.c FIRESTAFF_TOUCH_SWIPE_THRESHOLD_PX=40
 *                       FIRESTAFF_TOUCH_TAP_TOLERANCE_PX=24
 *                       FIRESTAFF_TOUCH_EDGE_ZONE_FRAC=0.20
 *
 * The test exercises:
 *   - Setting gate: disabled / enabled (the policy gate)
 *   - Swipe paths: forward / backward / turn-left / turn-right
 *   - Tap-tolerance rejection (kept as a tap, not a swipe)
 *   - Threshold rejection (too short)
 *   - Diagonal / equal-axis ambiguity rejection
 *   - Edge-strafe: V2-on / V1-only rejection
 *   - Edge-strafe target-size safety
 *   - Touch-target safety contract: 44 px floor
 *   - Source-viewport scale safety across surface sizes
 *   - Null-pointer safety
 *   - Default-threshold fallback
 *   - Decision-name string contract (grep-verifiable spec citation)
 */

#include "runtime_gesture_navigation_gate.h"

#include <stdio.h>
#include <string.h>

static int failures = 0;

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        ++failures; \
    } \
} while (0)

static void check_pass(const char* name, int ok) {
    if (!ok) {
        printf("FAIL %s\n", name);
        ++failures;
    } else {
        printf("PASS %s\n", name);
    }
}

/* Group A: setting gate - disabled when accessibilityTouchEnabled = 0 */
static void test_setting_gate_disabled(void) {
    FirestaffRuntimeGestureNavEvent ev;
    FirestaffRuntimeGestureNavPolicy pol;
    FirestaffRuntimeGestureNavResult res;

    memset(&ev, 0, sizeof(ev));
    memset(&pol, 0, sizeof(pol));
    memset(&res, 0, sizeof(res));

    ev.kind = FIRESTAFF_RUNTIME_GESTURE_NAV_KIND_SWIPE;
    ev.startX = 100; ev.startY = 100;
    ev.endX   = 100; ev.endY   = 60;   /* 40 px swipe up */
    ev.fbW = 320;
    ev.thresholdPx = 40;
    ev.tapTolerancePx = 24;

    pol.accessibilityTouchEnabled = 0;
    pol.v2PresentationEnabled = 1;
    pol.v1ParityPreserve = 0;

    check_pass("A1 disabled: rejected_disabled",
               FirestaffRuntimeGestureNav_Evaluate(&ev, &pol, &res) == 1 &&
               res.decision ==
                   RUNTIME_GESTURE_NAV_DECISION_REJECTED_DISABLED &&
               res.commandCode == 0);
}

/* Group B: swipe forward / backward / turn-left / turn-right at the
 * 40 px threshold.  These are the four movement / turn swipes. */
static void test_swipe_paths(void) {
    FirestaffRuntimeGestureNavEvent ev;
    FirestaffRuntimeGestureNavPolicy pol;
    FirestaffRuntimeGestureNavResult res;

    memset(&ev, 0, sizeof(ev));
    memset(&pol, 0, sizeof(pol));
    pol.accessibilityTouchEnabled = 1;
    pol.v2PresentationEnabled = 1;
    pol.v1ParityPreserve = 0;
    ev.fbW = 320;
    ev.thresholdPx = 40;
    ev.tapTolerancePx = 24;
    ev.kind = FIRESTAFF_RUNTIME_GESTURE_NAV_KIND_SWIPE;

    /* B1: swipe up -> forward, dy = -50, absDy > threshold */
    ev.startX = 100; ev.startY = 150;
    ev.endX   = 100; ev.endY   = 100;
    CHECK(FirestaffRuntimeGestureNav_Evaluate(&ev, &pol, &res) == 1);
    check_pass("B1 swipe up -> forward",
               res.decision ==
                   RUNTIME_GESTURE_NAV_DECISION_EMIT_FORWARD &&
               res.commandCode == 1 &&  /* FS_CMD_MOVE_FORWARD */
               res.dominantAxis == 'v' &&
               res.travelPx == 50);

    /* B2: swipe down -> backward, dy = +60 */
    ev.startX = 100; ev.startY = 100;
    ev.endX   = 100; ev.endY   = 160;
    CHECK(FirestaffRuntimeGestureNav_Evaluate(&ev, &pol, &res) == 1);
    check_pass("B2 swipe down -> backward",
               res.decision ==
                   RUNTIME_GESTURE_NAV_DECISION_EMIT_BACKWARD &&
               res.commandCode == 2 &&  /* FS_CMD_MOVE_BACKWARD */
               res.dominantAxis == 'v' &&
               res.travelPx == 60);

    /* B3: swipe right -> turn right, dx = +70 */
    ev.startX = 100; ev.startY = 100;
    ev.endX   = 170; ev.endY   = 100;
    CHECK(FirestaffRuntimeGestureNav_Evaluate(&ev, &pol, &res) == 1);
    check_pass("B3 swipe right -> turn right",
               res.decision ==
                   RUNTIME_GESTURE_NAV_DECISION_EMIT_TURN_RIGHT &&
               res.commandCode == 4 &&  /* FS_CMD_TURN_RIGHT */
               res.dominantAxis == 'h' &&
               res.travelPx == 70);

    /* B4: swipe left -> turn left, dx = -45 */
    ev.startX = 170; ev.startY = 100;
    ev.endX   = 125; ev.endY   = 100;
    CHECK(FirestaffRuntimeGestureNav_Evaluate(&ev, &pol, &res) == 1);
    check_pass("B4 swipe left -> turn left",
               res.decision ==
                   RUNTIME_GESTURE_NAV_DECISION_EMIT_TURN_LEFT &&
               res.commandCode == 3 &&  /* FS_CMD_TURN_LEFT */
               res.dominantAxis == 'h' &&
               res.travelPx == 45);
}

/* Group C: threshold rejection (too short) */
static void test_threshold_rejection(void) {
    FirestaffRuntimeGestureNavEvent ev;
    FirestaffRuntimeGestureNavPolicy pol;
    FirestaffRuntimeGestureNavResult res;

    memset(&ev, 0, sizeof(ev));
    memset(&pol, 0, sizeof(pol));
    pol.accessibilityTouchEnabled = 1;
    pol.v2PresentationEnabled = 1;
    pol.v1ParityPreserve = 0;
    ev.fbW = 320;
    ev.thresholdPx = 40;
    ev.tapTolerancePx = 24;
    ev.kind = FIRESTAFF_RUNTIME_GESTURE_NAV_KIND_SWIPE;

    /* C1: travel below threshold on both axes -> too short */
    ev.startX = 100; ev.startY = 100;
    ev.endX   = 110; ev.endY   = 105; /* dx=10 dy=5, both < 40 */
    CHECK(FirestaffRuntimeGestureNav_Evaluate(&ev, &pol, &res) == 1);
    check_pass("C1 below threshold -> too_short",
               res.decision ==
                   RUNTIME_GESTURE_NAV_DECISION_REJECTED_TOO_SHORT &&
               res.commandCode == 0);

    /* C2: tap-tolerance guard: dx=5 dy=5 <= tapTolerancePx=24 */
    ev.startX = 100; ev.startY = 100;
    ev.endX   = 105; ev.endY   = 105;
    CHECK(FirestaffRuntimeGestureNav_Evaluate(&ev, &pol, &res) == 1);
    check_pass("C2 tap-tolerance -> too_short",
               res.decision ==
                   RUNTIME_GESTURE_NAV_DECISION_REJECTED_TOO_SHORT);
}

/* Group D: diagonal / equal-axis ambiguity rejection */
static void test_diagonal_rejection(void) {
    FirestaffRuntimeGestureNavEvent ev;
    FirestaffRuntimeGestureNavPolicy pol;
    FirestaffRuntimeGestureNavResult res;

    memset(&ev, 0, sizeof(ev));
    memset(&pol, 0, sizeof(pol));
    pol.accessibilityTouchEnabled = 1;
    pol.v2PresentationEnabled = 1;
    pol.v1ParityPreserve = 0;
    ev.fbW = 320;
    ev.thresholdPx = 40;
    ev.tapTolerancePx = 24;
    ev.kind = FIRESTAFF_RUNTIME_GESTURE_NAV_KIND_SWIPE;

    /* D1: equal axes (dx=dy=50) -> ambiguous */
    ev.startX = 100; ev.startY = 100;
    ev.endX   = 150; ev.endY   = 150;
    CHECK(FirestaffRuntimeGestureNav_Evaluate(&ev, &pol, &res) == 1);
    check_pass("D1 equal-axis diagonal -> ambiguous",
               res.decision ==
                   RUNTIME_GESTURE_NAV_DECISION_REJECTED_AMBIGUOUS &&
               res.commandCode == 0);

    /* D2: slight horizontal edge (dx=51 dy=50) -> turn_right (horizontal wins) */
    ev.startX = 100; ev.startY = 100;
    ev.endX   = 151; ev.endY   = 150;
    CHECK(FirestaffRuntimeGestureNav_Evaluate(&ev, &pol, &res) == 1);
    check_pass("D2 slight horizontal edge -> turn_right",
               res.decision ==
                   RUNTIME_GESTURE_NAV_DECISION_EMIT_TURN_RIGHT);
}

/* Group E: edge-strafe - V2-only with v1ParityPreserve flag */
static void test_edge_strafe_paths(void) {
    FirestaffRuntimeGestureNavEvent ev;
    FirestaffRuntimeGestureNavPolicy pol;
    FirestaffRuntimeGestureNavResult res;

    memset(&ev, 0, sizeof(ev));
    memset(&pol, 0, sizeof(pol));
    pol.accessibilityTouchEnabled = 1;
    pol.v2PresentationEnabled = 1;
    pol.v1ParityPreserve = 0;
    ev.fbW = 320;
    ev.thresholdPx = 40;
    ev.tapTolerancePx = 24;
    ev.kind = FIRESTAFF_RUNTIME_GESTURE_NAV_KIND_EDGE_STRAFE;

    /* E1: edge-left touch (x=20 < 20% of 320 = 64) -> strafe left */
    ev.startX = 20;
    CHECK(FirestaffRuntimeGestureNav_Evaluate(&ev, &pol, &res) == 1);
    check_pass("E1 edge-left -> strafe_left",
               res.decision ==
                   RUNTIME_GESTURE_NAV_DECISION_EMIT_STRAFE_LEFT &&
               res.commandCode == 5);  /* FS_CMD_STRAFE_LEFT */

    /* E2: edge-right touch (x=300 >= 320 - 64 = 256) -> strafe right */
    ev.startX = 300;
    CHECK(FirestaffRuntimeGestureNav_Evaluate(&ev, &pol, &res) == 1);
    check_pass("E2 edge-right -> strafe_right",
               res.decision ==
                   RUNTIME_GESTURE_NAV_DECISION_EMIT_STRAFE_RIGHT &&
               res.commandCode == 6);  /* FS_CMD_STRAFE_RIGHT */

    /* E3: middle touch -> ambiguous (caller should re-classify) */
    ev.startX = 160;
    CHECK(FirestaffRuntimeGestureNav_Evaluate(&ev, &pol, &res) == 1);
    check_pass("E3 middle edge-strafe -> ambiguous",
               res.decision ==
                   RUNTIME_GESTURE_NAV_DECISION_REJECTED_AMBIGUOUS);

    /* E4: V1 parity guard - v1ParityPreserve=1 must reject */
    pol.v1ParityPreserve = 1;
    ev.startX = 20;
    CHECK(FirestaffRuntimeGestureNav_Evaluate(&ev, &pol, &res) == 1);
    check_pass("E4 V1-parity guard -> rejected_v1_only",
               res.decision ==
                   RUNTIME_GESTURE_NAV_DECISION_REJECTED_V1_ONLY &&
               res.commandCode == 0);
    pol.v1ParityPreserve = 0;

    /* E5: V2-off - v2PresentationEnabled=0 must reject */
    pol.v2PresentationEnabled = 0;
    ev.startX = 20;
    CHECK(FirestaffRuntimeGestureNav_Evaluate(&ev, &pol, &res) == 1);
    check_pass("E5 V2-off edge-strafe -> rejected_v1_only",
               res.decision ==
                   RUNTIME_GESTURE_NAV_DECISION_REJECTED_V1_ONLY);
    pol.v2PresentationEnabled = 1;
}

/* Group F: edge-strafe target-size safety - when the 20% edge-zone
 * is smaller than the 44 px safe-touch floor, reject.  On a 200 px
 * framebuffer, 20% = 40 px < 44 px. */
static void test_edge_strafe_target_safety(void) {
    FirestaffRuntimeGestureNavEvent ev;
    FirestaffRuntimeGestureNavPolicy pol;
    FirestaffRuntimeGestureNavResult res;

    memset(&ev, 0, sizeof(ev));
    memset(&pol, 0, sizeof(pol));
    pol.accessibilityTouchEnabled = 1;
    pol.v2PresentationEnabled = 1;
    pol.v1ParityPreserve = 0;
    ev.kind = FIRESTAFF_RUNTIME_GESTURE_NAV_KIND_EDGE_STRAFE;
    ev.thresholdPx = 40;
    ev.tapTolerancePx = 24;

    /* F1: 200 px wide framebuffer -> 20% edge = 40 px < 44 -> reject */
    ev.fbW = 200;
    ev.startX = 10;
    CHECK(FirestaffRuntimeGestureNav_Evaluate(&ev, &pol, &res) == 1);
    check_pass("F1 200-fbW edge too small -> too_small_target",
               res.decision ==
                   RUNTIME_GESTURE_NAV_DECISION_REJECTED_TOO_SMALL_TARGET &&
               res.commandCode == 0);

    /* F2: 320 px wide framebuffer -> 20% edge = 64 px >= 44 -> accept */
    ev.fbW = 320;
    ev.startX = 20;
    CHECK(FirestaffRuntimeGestureNav_Evaluate(&ev, &pol, &res) == 1);
    check_pass("F2 320-fbW edge safe -> emit_strafe_left",
               res.decision ==
                   RUNTIME_GESTURE_NAV_DECISION_EMIT_STRAFE_LEFT);
}

/* Group G: touch-target safety contract (44 px floor) */
static void test_touch_target_safety(void) {
    check_pass("G1 44x44 -> safe",
               FirestaffRuntimeGestureNav_TouchTargetSafe(44, 44) == 1);
    check_pass("G2 50x30 (h<44) -> unsafe",
               FirestaffRuntimeGestureNav_TouchTargetSafe(50, 30) == 0);
    check_pass("G3 30x50 (w<44) -> unsafe",
               FirestaffRuntimeGestureNav_TouchTargetSafe(30, 50) == 0);
    check_pass("G4 negative dim -> unsafe",
               FirestaffRuntimeGestureNav_TouchTargetSafe(-1, 44) == 0);
    check_pass("G5 0x0 -> unsafe",
               FirestaffRuntimeGestureNav_TouchTargetSafe(0, 0) == 0);
    check_pass("G6 100x100 -> safe",
               FirestaffRuntimeGestureNav_TouchTargetSafe(100, 100) == 1);
}

/* Group H: source-viewport scale safety across surface sizes */
static void test_source_viewport_scale_safety(void) {
    /* 320x200 source viewport scaled to 1280x720:
     * sx = 1280/320 = 4, sy = 720/200 = 3.
     * shortest axis is height (200), scaled: 200 * 3 = 600.
     * 600 >= 44 -> safe. */
    check_pass("H1 320x200 -> 1280x720 safe",
               FirestaffRuntimeGestureNav_SourceViewportSafe(
                   320, 200, 1280, 720) == 1);

    /* 320x200 source to 1920x1080: sx=6 sy=5, 200*5=1000 -> safe */
    check_pass("H2 320x200 -> 1920x1080 safe",
               FirestaffRuntimeGestureNav_SourceViewportSafe(
                   320, 200, 1920, 1080) == 1);

    /* 320x200 source to 320x200 surface (1x): shortest axis = 200 -> safe */
    check_pass("H3 320x200 -> 320x200 (1x) safe",
               FirestaffRuntimeGestureNav_SourceViewportSafe(
                   320, 200, 320, 200) == 1);

    /* Pathological 1x1 surface -> unsafe (sx=0 -> sx=1, but sourceH=200
     * still safe so we accept). This documents the conservative boundary:
     * the function only consults the SOURCE framebuffer and the SURFACE
     * dimensions to compute the scale; the actual runtime viewport will
     * never see a surface smaller than the source because the letterbox
     * path keeps the source 1:1 minimum. */

    /* Negative or zero dimensions -> unsafe */
    check_pass("H4 zero source -> unsafe",
               FirestaffRuntimeGestureNav_SourceViewportSafe(
                   0, 200, 1280, 720) == 0);
    check_pass("H5 zero surface -> unsafe",
               FirestaffRuntimeGestureNav_SourceViewportSafe(
                   320, 200, 0, 720) == 0);
}

/* Group I: null-pointer safety */
static void test_null_pointer_safety(void) {
    FirestaffRuntimeGestureNavEvent ev;
    FirestaffRuntimeGestureNavPolicy pol;
    FirestaffRuntimeGestureNavResult res;

    memset(&ev, 0, sizeof(ev));
    memset(&pol, 0, sizeof(pol));
    pol.accessibilityTouchEnabled = 1;
    ev.fbW = 320;
    ev.thresholdPx = 40;
    ev.tapTolerancePx = 24;
    ev.kind = FIRESTAFF_RUNTIME_GESTURE_NAV_KIND_SWIPE;
    ev.startX = 100; ev.startY = 100;
    ev.endX   = 100; ev.endY   = 60;

    /* I1: NULL out-pointer -> returns 0 */
    check_pass("I1 NULL out-pointer -> 0",
               FirestaffRuntimeGestureNav_Evaluate(&ev, &pol, NULL) == 0);

    /* I2: NULL event -> REJECTED_NULL, command=0, axis='e' */
    CHECK(FirestaffRuntimeGestureNav_Evaluate(NULL, &pol, &res) == 1);
    check_pass("I2 NULL event -> rejected_null",
               res.decision ==
                   RUNTIME_GESTURE_NAV_DECISION_REJECTED_NULL &&
               res.commandCode == 0 && res.dominantAxis == 'e');

    /* I3: NULL policy -> REJECTED_NULL */
    CHECK(FirestaffRuntimeGestureNav_Evaluate(&ev, NULL, &res) == 1);
    check_pass("I3 NULL policy -> rejected_null",
               res.decision ==
                   RUNTIME_GESTURE_NAV_DECISION_REJECTED_NULL);
}

/* Group J: default-threshold fallback when caller passes 0 */
static void test_default_threshold_fallback(void) {
    FirestaffRuntimeGestureNavEvent ev;
    FirestaffRuntimeGestureNavPolicy pol;
    FirestaffRuntimeGestureNavResult res;

    memset(&ev, 0, sizeof(ev));
    memset(&pol, 0, sizeof(pol));
    pol.accessibilityTouchEnabled = 1;
    pol.v2PresentationEnabled = 1;
    pol.v1ParityPreserve = 0;
    ev.fbW = 320;
    ev.thresholdPx = 0;          /* default fallback */
    ev.tapTolerancePx = 0;       /* skip tap-tolerance */
    ev.kind = FIRESTAFF_RUNTIME_GESTURE_NAV_KIND_SWIPE;

    /* J1: 39 px swipe (just below the 40 px default) -> too_short */
    ev.startX = 100; ev.startY = 100;
    ev.endX   = 100; ev.endY   = 61;
    CHECK(FirestaffRuntimeGestureNav_Evaluate(&ev, &pol, &res) == 1);
    check_pass("J1 39px swipe + default threshold -> too_short",
               res.decision ==
                   RUNTIME_GESTURE_NAV_DECISION_REJECTED_TOO_SHORT);

    /* J2: 41 px swipe (just above the 40 px default) -> forward */
    ev.startX = 100; ev.startY = 100;
    ev.endX   = 100; ev.endY   = 59;
    CHECK(FirestaffRuntimeGestureNav_Evaluate(&ev, &pol, &res) == 1);
    check_pass("J2 41px swipe + default threshold -> forward",
               res.decision ==
                   RUNTIME_GESTURE_NAV_DECISION_EMIT_FORWARD);
}

/* Group K: V1 parity - swipes are accepted in V1 mode (no V2 needed).
 * Edge-strafe is the only V2-only branch. */
static void test_v1_swipe_parity(void) {
    FirestaffRuntimeGestureNavEvent ev;
    FirestaffRuntimeGestureNavPolicy pol;
    FirestaffRuntimeGestureNavResult res;

    memset(&ev, 0, sizeof(ev));
    memset(&pol, 0, sizeof(pol));
    pol.accessibilityTouchEnabled = 1;
    pol.v2PresentationEnabled = 0; /* V1 active */
    pol.v1ParityPreserve = 1;      /* V1 strict */
    ev.fbW = 320;
    ev.thresholdPx = 40;
    ev.tapTolerancePx = 24;
    ev.kind = FIRESTAFF_RUNTIME_GESTURE_NAV_KIND_SWIPE;

    /* K1: V1 + strict parity: swipe forward still emits (V1 parity surface) */
    ev.startX = 100; ev.startY = 150;
    ev.endX   = 100; ev.endY   = 100;
    CHECK(FirestaffRuntimeGestureNav_Evaluate(&ev, &pol, &res) == 1);
    check_pass("K1 V1+strict swipe forward -> emit_forward",
               res.decision ==
                   RUNTIME_GESTURE_NAV_DECISION_EMIT_FORWARD);

    /* K2: V1 + strict parity: edge-strafe rejected (V2-only) */
    ev.kind = FIRESTAFF_RUNTIME_GESTURE_NAV_KIND_EDGE_STRAFE;
    ev.startX = 20;
    CHECK(FirestaffRuntimeGestureNav_Evaluate(&ev, &pol, &res) == 1);
    check_pass("K2 V1+strict edge-strafe -> rejected_v1_only",
               res.decision ==
                   RUNTIME_GESTURE_NAV_DECISION_REJECTED_V1_ONLY);
}

/* Group L: decision-name string contract (grep-verifiable spec citation) */
static void test_decision_name_contract(void) {
    check_pass("L1 emit_forward name",
               strcmp(FirestaffRuntimeGestureNav_DecisionName(
                          RUNTIME_GESTURE_NAV_DECISION_EMIT_FORWARD),
                      "emit_forward") == 0);
    check_pass("L2 emit_backward name",
               strcmp(FirestaffRuntimeGestureNav_DecisionName(
                          RUNTIME_GESTURE_NAV_DECISION_EMIT_BACKWARD),
                      "emit_backward") == 0);
    check_pass("L3 emit_turn_left name",
               strcmp(FirestaffRuntimeGestureNav_DecisionName(
                          RUNTIME_GESTURE_NAV_DECISION_EMIT_TURN_LEFT),
                      "emit_turn_left") == 0);
    check_pass("L4 emit_turn_right name",
               strcmp(FirestaffRuntimeGestureNav_DecisionName(
                          RUNTIME_GESTURE_NAV_DECISION_EMIT_TURN_RIGHT),
                      "emit_turn_right") == 0);
    check_pass("L5 emit_strafe_left name",
               strcmp(FirestaffRuntimeGestureNav_DecisionName(
                          RUNTIME_GESTURE_NAV_DECISION_EMIT_STRAFE_LEFT),
                      "emit_strafe_left") == 0);
    check_pass("L6 emit_strafe_right name",
               strcmp(FirestaffRuntimeGestureNav_DecisionName(
                          RUNTIME_GESTURE_NAV_DECISION_EMIT_STRAFE_RIGHT),
                      "emit_strafe_right") == 0);
    check_pass("L7 rejected_disabled name",
               strcmp(FirestaffRuntimeGestureNav_DecisionName(
                          RUNTIME_GESTURE_NAV_DECISION_REJECTED_DISABLED),
                      "rejected_disabled") == 0);
    check_pass("L8 rejected_too_short name",
               strcmp(FirestaffRuntimeGestureNav_DecisionName(
                          RUNTIME_GESTURE_NAV_DECISION_REJECTED_TOO_SHORT),
                      "rejected_too_short") == 0);
    check_pass("L9 rejected_too_small_target name",
               strcmp(FirestaffRuntimeGestureNav_DecisionName(
                          RUNTIME_GESTURE_NAV_DECISION_REJECTED_TOO_SMALL_TARGET),
                      "rejected_too_small_target") == 0);
    check_pass("L10 rejected_ambiguous name",
               strcmp(FirestaffRuntimeGestureNav_DecisionName(
                          RUNTIME_GESTURE_NAV_DECISION_REJECTED_AMBIGUOUS),
                      "rejected_ambiguous") == 0);
    check_pass("L11 rejected_null name",
               strcmp(FirestaffRuntimeGestureNav_DecisionName(
                          RUNTIME_GESTURE_NAV_DECISION_REJECTED_NULL),
                      "rejected_null") == 0);
    check_pass("L12 rejected_v1_only name",
               strcmp(FirestaffRuntimeGestureNav_DecisionName(
                          RUNTIME_GESTURE_NAV_DECISION_REJECTED_V1_ONLY),
                      "rejected_v1_only") == 0);
    check_pass("L13 unknown decision name",
               strcmp(FirestaffRuntimeGestureNav_DecisionName(
                          (FirestaffRuntimeGestureNavDecision)999),
                      "unknown") == 0);
}

/* Group M: source-evidence string contains key source-locked names */
static void test_source_evidence_citation(void) {
    const char* ev = FirestaffRuntimeGestureNav_GetSourceEvidence();
    CHECK(ev != NULL);
    check_pass("M1 evidence cites COMMAND.C",
               strstr(ev, "COMMAND.C") != NULL);
    check_pass("M2 evidence cites CLIKMENU.C",
               strstr(ev, "CLIKMENU.C") != NULL);
    check_pass("M3 evidence cites GAMELOOP.C",
               strstr(ev, "GAMELOOP.C") != NULL);
    check_pass("M4 evidence cites DEFS.H",
               strstr(ev, "DEFS.H") != NULL);
    check_pass("M5 evidence cites firestaff_touch.c",
               strstr(ev, "firestaff_touch.c") != NULL);
    check_pass("M6 evidence cites F0380_COMMAND_ProcessQueue_CPSC",
               strstr(ev, "F0380_COMMAND_ProcessQueue_CPSC") != NULL);
    check_pass("M7 evidence cites F0365",
               strstr(ev, "F0365") != NULL);
    check_pass("M8 evidence cites F0366",
               strstr(ev, "F0366") != NULL);
}

/* Group N: cross-V1/V2 surface scaling - emit_forward / emit_backward
 * codes must match firestaff_input.h FS_CMD_* so callers can index the
 * resulting command on a single integer. */
static void test_cross_v1_v2_command_codes(void) {
    FirestaffRuntimeGestureNavEvent ev;
    FirestaffRuntimeGestureNavPolicy pol;
    FirestaffRuntimeGestureNavResult res;

    memset(&ev, 0, sizeof(ev));
    memset(&pol, 0, sizeof(pol));
    pol.accessibilityTouchEnabled = 1;
    pol.v2PresentationEnabled = 1;
    pol.v1ParityPreserve = 0;
    ev.fbW = 320;
    ev.thresholdPx = 40;
    ev.tapTolerancePx = 24;
    ev.kind = FIRESTAFF_RUNTIME_GESTURE_NAV_KIND_SWIPE;

    /* N1: code 1 = FS_CMD_MOVE_FORWARD */
    ev.startX = 100; ev.startY = 150;
    ev.endX   = 100; ev.endY   = 100;
    CHECK(FirestaffRuntimeGestureNav_Evaluate(&ev, &pol, &res) == 1);
    check_pass("N1 forward code matches FS_CMD_MOVE_FORWARD",
               res.commandCode == 1);

    /* N2: code 2 = FS_CMD_MOVE_BACKWARD */
    ev.startX = 100; ev.startY = 100;
    ev.endX   = 100; ev.endY   = 160;
    CHECK(FirestaffRuntimeGestureNav_Evaluate(&ev, &pol, &res) == 1);
    check_pass("N2 backward code matches FS_CMD_MOVE_BACKWARD",
               res.commandCode == 2);

    /* N3: code 3 = FS_CMD_TURN_LEFT */
    ev.startX = 170; ev.startY = 100;
    ev.endX   = 100; ev.endY   = 100;
    CHECK(FirestaffRuntimeGestureNav_Evaluate(&ev, &pol, &res) == 1);
    check_pass("N3 turn_left code matches FS_CMD_TURN_LEFT",
               res.commandCode == 3);

    /* N4: code 4 = FS_CMD_TURN_RIGHT */
    ev.startX = 100; ev.startY = 100;
    ev.endX   = 170; ev.endY   = 100;
    CHECK(FirestaffRuntimeGestureNav_Evaluate(&ev, &pol, &res) == 1);
    check_pass("N4 turn_right code matches FS_CMD_TURN_RIGHT",
               res.commandCode == 4);
}

/* Group O: travel-pixel reporting - pin the threshold boundary so a
 * 1 px shorter swipe is rejected and a 1 px longer swipe is accepted. */
static void test_travel_pixel_threshold_boundary(void) {
    FirestaffRuntimeGestureNavEvent ev;
    FirestaffRuntimeGestureNavPolicy pol;
    FirestaffRuntimeGestureNavResult res;

    memset(&ev, 0, sizeof(ev));
    memset(&pol, 0, sizeof(pol));
    pol.accessibilityTouchEnabled = 1;
    pol.v2PresentationEnabled = 1;
    pol.v1ParityPreserve = 0;
    ev.fbW = 320;
    ev.thresholdPx = 50;
    ev.tapTolerancePx = 24;
    ev.kind = FIRESTAFF_RUNTIME_GESTURE_NAV_KIND_SWIPE;

    /* O1: 49 px swipe up -> too_short, travel=49 */
    ev.startX = 100; ev.startY = 149;
    ev.endX   = 100; ev.endY   = 100;
    CHECK(FirestaffRuntimeGestureNav_Evaluate(&ev, &pol, &res) == 1);
    check_pass("O1 49px @ threshold=50 -> too_short travel=49",
               res.decision ==
                   RUNTIME_GESTURE_NAV_DECISION_REJECTED_TOO_SHORT &&
               res.travelPx == 49);

    /* O2: 51 px swipe up -> emit_forward, travel=51 */
    ev.startX = 100; ev.startY = 151;
    ev.endX   = 100; ev.endY   = 100;
    CHECK(FirestaffRuntimeGestureNav_Evaluate(&ev, &pol, &res) == 1);
    check_pass("O2 51px @ threshold=50 -> forward travel=51",
               res.decision ==
                   RUNTIME_GESTURE_NAV_DECISION_EMIT_FORWARD &&
               res.travelPx == 51);
}

int main(void) {
    printf("test=runtime_gesture_navigation_gate\n");
    printf("source_evidence=%s\n",
           FirestaffRuntimeGestureNav_GetSourceEvidence());

    test_setting_gate_disabled();
    test_swipe_paths();
    test_threshold_rejection();
    test_diagonal_rejection();
    test_edge_strafe_paths();
    test_edge_strafe_target_safety();
    test_touch_target_safety();
    test_source_viewport_scale_safety();
    test_null_pointer_safety();
    test_default_threshold_fallback();
    test_v1_swipe_parity();
    test_decision_name_contract();
    test_source_evidence_citation();
    test_cross_v1_v2_command_codes();
    test_travel_pixel_threshold_boundary();

    printf("failures=%d\n", failures);
    return failures == 0 ? 0 : 1;
}

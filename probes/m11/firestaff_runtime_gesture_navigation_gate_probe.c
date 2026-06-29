/*
 * firestaff_runtime_gesture_navigation_gate_probe.c
 *
 * Headless CI probe for the cross-game runtime gesture navigation gate.
 *
 * Scope:
 *   - Mirrors the unit-test invariants that touch the public gate API.
 *   - Adds a 50-iteration determinism probe so the gate's decision
 *     sequence is byte-stable across runs (CI-friendly).
 *   - Pin the four movement/turn swipes + two edge-strafes + the four
 *     reject paths + the 44 px touch-target floor in one PASS/FAIL row
 *     so CI can answer the open G2 gap with a single command.
 *
 * Source lock (grep-verifiable):
 *   ReDMCSB COMMAND.C:2045-2155 F0380_COMMAND_ProcessQueue_CPSC
 *   ReDMCSB CLIKMENU.C:142-174 F0365 turn
 *   ReDMCSB CLIKMENU.C:180-390 F0366 move
 *   ReDMCSB GAMELOOP.C:164-219 V1 input wait loop
 *   ReDMCSB DEFS.H:238-243 C001..C006 movement commands
 *   firestaff_touch.c FIRESTAFF_TOUCH_SWIPE_THRESHOLD_PX=40
 *                       FIRESTAFF_TOUCH_TAP_TOLERANCE_PX=24
 *                       FIRESTAFF_TOUCH_EDGE_ZONE_FRAC=0.20
 *
 * The probe is data-free, header-only, and links only against the
 * M11 lib (which carries the gate via runtime_gesture_navigation_
 * gate.c). No SDL, no graphics, no game data.
 */

#include "runtime_gesture_navigation_gate.h"

#include <stdio.h>
#include <string.h>

static int ok = 1;

static void check(int cond, const char* name) {
    if (!cond) {
        printf("FAIL %s\n", name);
        ok = 0;
    } else {
        printf("PASS %s\n", name);
    }
}

/* Group A: setting gate */
static void probe_setting_gate(void) {
    FirestaffRuntimeGestureNavEvent ev;
    FirestaffRuntimeGestureNavPolicy pol;
    FirestaffRuntimeGestureNavResult res;

    memset(&ev, 0, sizeof(ev));
    memset(&pol, 0, sizeof(pol));
    memset(&res, 0, sizeof(res));

    pol.accessibilityTouchEnabled = 0;
    pol.v2PresentationEnabled = 1;
    ev.kind = FIRESTAFF_RUNTIME_GESTURE_NAV_KIND_SWIPE;
    ev.startX = 100; ev.startY = 100;
    ev.endX   = 100; ev.endY   = 60;
    ev.fbW = 320;
    ev.thresholdPx = 40;
    ev.tapTolerancePx = 24;

    check(FirestaffRuntimeGestureNav_Evaluate(&ev, &pol, &res) == 1 &&
          res.decision ==
              RUNTIME_GESTURE_NAV_DECISION_REJECTED_DISABLED,
          "A1 disabled policy rejects swipe");
}

/* Group B: four movement/turn swipes */
static void probe_swipe_paths(void) {
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

    /* B1 forward */
    ev.startX = 100; ev.startY = 150;
    ev.endX   = 100; ev.endY   = 100;
    check(FirestaffRuntimeGestureNav_Evaluate(&ev, &pol, &res) == 1 &&
          res.decision ==
              RUNTIME_GESTURE_NAV_DECISION_EMIT_FORWARD &&
          res.commandCode == 1 &&
          res.dominantAxis == 'v' &&
          res.travelPx == 50,
          "B1 swipe up -> forward (cmd=1, axis=v, travel=50)");

    /* B2 backward */
    ev.startX = 100; ev.startY = 100;
    ev.endX   = 100; ev.endY   = 160;
    check(FirestaffRuntimeGestureNav_Evaluate(&ev, &pol, &res) == 1 &&
          res.decision ==
              RUNTIME_GESTURE_NAV_DECISION_EMIT_BACKWARD &&
          res.commandCode == 2,
          "B2 swipe down -> backward (cmd=2)");

    /* B3 turn right */
    ev.startX = 100; ev.startY = 100;
    ev.endX   = 170; ev.endY   = 100;
    check(FirestaffRuntimeGestureNav_Evaluate(&ev, &pol, &res) == 1 &&
          res.decision ==
              RUNTIME_GESTURE_NAV_DECISION_EMIT_TURN_RIGHT &&
          res.commandCode == 4 &&
          res.dominantAxis == 'h',
          "B3 swipe right -> turn right (cmd=4, axis=h)");

    /* B4 turn left */
    ev.startX = 170; ev.startY = 100;
    ev.endX   = 125; ev.endY   = 100;
    check(FirestaffRuntimeGestureNav_Evaluate(&ev, &pol, &res) == 1 &&
          res.decision ==
              RUNTIME_GESTURE_NAV_DECISION_EMIT_TURN_LEFT &&
          res.commandCode == 3,
          "B4 swipe left -> turn left (cmd=3)");
}

/* Group C: reject paths */
static void probe_reject_paths(void) {
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

    /* C1 too_short */
    ev.startX = 100; ev.startY = 100;
    ev.endX   = 110; ev.endY   = 105;
    check(FirestaffRuntimeGestureNav_Evaluate(&ev, &pol, &res) == 1 &&
          res.decision ==
              RUNTIME_GESTURE_NAV_DECISION_REJECTED_TOO_SHORT,
          "C1 below threshold -> too_short");

    /* C2 ambiguous diagonal */
    ev.startX = 100; ev.startY = 100;
    ev.endX   = 150; ev.endY   = 150;
    check(FirestaffRuntimeGestureNav_Evaluate(&ev, &pol, &res) == 1 &&
          res.decision ==
              RUNTIME_GESTURE_NAV_DECISION_REJECTED_AMBIGUOUS,
          "C2 equal-axis diagonal -> ambiguous");

    /* C3 null safety */
    check(FirestaffRuntimeGestureNav_Evaluate(&ev, &pol, NULL) == 0,
          "C3 NULL out-pointer -> 0");
    check(FirestaffRuntimeGestureNav_Evaluate(NULL, &pol, &res) == 1 &&
          res.decision ==
              RUNTIME_GESTURE_NAV_DECISION_REJECTED_NULL,
          "C3b NULL event -> rejected_null");
}

/* Group D: edge-strafe */
static void probe_edge_strafe_paths(void) {
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

    ev.startX = 20;
    check(FirestaffRuntimeGestureNav_Evaluate(&ev, &pol, &res) == 1 &&
          res.decision ==
              RUNTIME_GESTURE_NAV_DECISION_EMIT_STRAFE_LEFT &&
          res.commandCode == 5,
          "D1 edge-left (x=20) -> strafe_left (cmd=5)");

    ev.startX = 300;
    check(FirestaffRuntimeGestureNav_Evaluate(&ev, &pol, &res) == 1 &&
          res.decision ==
              RUNTIME_GESTURE_NAV_DECISION_EMIT_STRAFE_RIGHT &&
          res.commandCode == 6,
          "D2 edge-right (x=300) -> strafe_right (cmd=6)");

    /* D3 V1-only rejection */
    pol.v2PresentationEnabled = 0;
    pol.v1ParityPreserve = 1;
    ev.startX = 20;
    check(FirestaffRuntimeGestureNav_Evaluate(&ev, &pol, &res) == 1 &&
          res.decision ==
              RUNTIME_GESTURE_NAV_DECISION_REJECTED_V1_ONLY,
          "D3 V1+strict edge-strafe -> rejected_v1_only");
    pol.v2PresentationEnabled = 1;
    pol.v1ParityPreserve = 0;

    /* D4 target-size safety: 200 px fbW -> 20% edge = 40 px < 44 */
    ev.fbW = 200;
    ev.startX = 10;
    check(FirestaffRuntimeGestureNav_Evaluate(&ev, &pol, &res) == 1 &&
          res.decision ==
              RUNTIME_GESTURE_NAV_DECISION_REJECTED_TOO_SMALL_TARGET,
          "D4 200-fbW edge -> too_small_target (40px < 44px floor)");
}

/* Group E: touch-target safety contract */
static void probe_touch_target_safety(void) {
    check(FirestaffRuntimeGestureNav_TouchTargetSafe(44, 44) == 1,
          "E1 44x44 -> safe (Apple HIG floor)");
    check(FirestaffRuntimeGestureNav_TouchTargetSafe(50, 30) == 0,
          "E2 50x30 -> unsafe (h<44)");
    check(FirestaffRuntimeGestureNav_TouchTargetSafe(100, 100) == 1,
          "E3 100x100 -> safe");
    check(FirestaffRuntimeGestureNav_TouchTargetSafe(0, 0) == 0,
          "E4 0x0 -> unsafe");
    check(FirestaffRuntimeGestureNav_TouchTargetSafe(-1, 44) == 0,
          "E5 negative dim -> unsafe");
}

/* Group F: source-viewport scale safety */
static void probe_source_viewport_scale(void) {
    check(FirestaffRuntimeGestureNav_SourceViewportSafe(
              320, 200, 1280, 720) == 1,
          "F1 320x200 -> 1280x720 safe");
    check(FirestaffRuntimeGestureNav_SourceViewportSafe(
              320, 200, 1920, 1080) == 1,
          "F2 320x200 -> 1920x1080 safe");
    check(FirestaffRuntimeGestureNav_SourceViewportSafe(
              320, 200, 320, 200) == 1,
          "F3 320x200 -> 320x200 (1x) safe");
    check(FirestaffRuntimeGestureNav_SourceViewportSafe(
              320, 200, 70, 44) == 0,
          "F4 320x200 -> 70x44 fits as 70x43 unsafe");
    check(FirestaffRuntimeGestureNav_SourceViewportSafe(
              320, 200, 71, 44) == 1,
          "F5 320x200 -> 71x44 fits as 71x44 safe");
    check(FirestaffRuntimeGestureNav_SourceViewportSafe(
              320, 200, 43, 43) == 0,
          "F6 320x200 -> 43x43 unsafe");
    check(FirestaffRuntimeGestureNav_SourceViewportSafe(
              0, 200, 1280, 720) == 0,
          "F7 zero source -> unsafe");
    check(FirestaffRuntimeGestureNav_SourceViewportSafe(
              320, 200, 0, 720) == 0,
          "F8 zero surface -> unsafe");
}

/* Group G: 50-iteration determinism - every probe-state evaluation
 * must produce byte-stable decisions so the gate can be a CI lock. */
static void probe_determinism(void) {
    FirestaffRuntimeGestureNavEvent ev;
    FirestaffRuntimeGestureNavPolicy pol;
    FirestaffRuntimeGestureNavResult res_first;
    FirestaffRuntimeGestureNavResult res_iter;
    int i;

    memset(&ev, 0, sizeof(ev));
    memset(&pol, 0, sizeof(pol));
    pol.accessibilityTouchEnabled = 1;
    pol.v2PresentationEnabled = 1;
    pol.v1ParityPreserve = 0;
    ev.fbW = 320;
    ev.thresholdPx = 40;
    ev.tapTolerancePx = 24;
    ev.kind = FIRESTAFF_RUNTIME_GESTURE_NAV_KIND_SWIPE;
    ev.startX = 100; ev.startY = 150;
    ev.endX   = 100; ev.endY   = 100;

    FirestaffRuntimeGestureNav_Evaluate(&ev, &pol, &res_first);
    for (i = 0; i < 50; ++i) {
        FirestaffRuntimeGestureNav_Evaluate(&ev, &pol, &res_iter);
        check(res_iter.decision == res_first.decision &&
              res_iter.commandCode == res_first.commandCode &&
              res_iter.dominantAxis == res_first.dominantAxis &&
              res_iter.travelPx == res_first.travelPx,
              "G1 determinism iteration");
    }
}

/* Group H: source-evidence citation contract */
static void probe_source_evidence_contract(void) {
    const char* ev = FirestaffRuntimeGestureNav_GetSourceEvidence();
    check(ev != NULL, "H1 evidence non-null");
    check(strstr(ev, "COMMAND.C") != NULL,
          "H2 evidence cites COMMAND.C");
    check(strstr(ev, "CLIKMENU.C") != NULL,
          "H3 evidence cites CLIKMENU.C");
    check(strstr(ev, "GAMELOOP.C") != NULL,
          "H4 evidence cites GAMELOOP.C");
    check(strstr(ev, "DEFS.H") != NULL,
          "H5 evidence cites DEFS.H");
    check(strstr(ev, "F0380_COMMAND_ProcessQueue_CPSC") != NULL,
          "H6 evidence cites F0380_COMMAND_ProcessQueue_CPSC");
    check(strstr(ev, "F0365") != NULL &&
          strstr(ev, "F0366") != NULL,
          "H7 evidence cites F0365 + F0366 (turn + move dispatch)");
}

/* Group I: cross-V1/V2 command code parity (FS_InputQueue compat) */
static void probe_cross_v1_v2_command_codes(void) {
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

    ev.startX = 100; ev.startY = 150;
    ev.endX   = 100; ev.endY   = 100;
    FirestaffRuntimeGestureNav_Evaluate(&ev, &pol, &res);
    check(res.commandCode == 1, "I1 forward code = 1 (FS_CMD_MOVE_FORWARD)");

    ev.startX = 100; ev.startY = 100;
    ev.endX   = 100; ev.endY   = 160;
    FirestaffRuntimeGestureNav_Evaluate(&ev, &pol, &res);
    check(res.commandCode == 2, "I2 backward code = 2 (FS_CMD_MOVE_BACKWARD)");

    ev.startX = 170; ev.startY = 100;
    ev.endX   = 100; ev.endY   = 100;
    FirestaffRuntimeGestureNav_Evaluate(&ev, &pol, &res);
    check(res.commandCode == 3, "I3 turn_left code = 3 (FS_CMD_TURN_LEFT)");

    ev.startX = 100; ev.startY = 100;
    ev.endX   = 170; ev.endY   = 100;
    FirestaffRuntimeGestureNav_Evaluate(&ev, &pol, &res);
    check(res.commandCode == 4, "I4 turn_right code = 4 (FS_CMD_TURN_RIGHT)");
}

int main(void) {
    printf("probe=firestaff_runtime_gesture_navigation_gate\n");
    printf("source_evidence=%s\n",
           FirestaffRuntimeGestureNav_GetSourceEvidence());

    probe_setting_gate();
    probe_swipe_paths();
    probe_reject_paths();
    probe_edge_strafe_paths();
    probe_touch_target_safety();
    probe_source_viewport_scale();
    probe_determinism();
    probe_source_evidence_contract();
    probe_cross_v1_v2_command_codes();

    printf("status=%s\n", ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}

/*
 * runtime_gesture_navigation_gate.c
 *
 * Implementation of the cross-game runtime gesture navigation gate
 * declared in include/runtime_gesture_navigation_gate.h.
 *
 * Scope:
 *   - Wraps the existing firestaff_touch.c swipe + edge-zone primitives
 *     into a deterministic contract: input event + policy -> decision.
 *   - Pins the touch-target safety floor (RUNTIME_GESTURE_NAV_MIN_TARGET_PX).
 *   - Source-locks the policy branch on ReDMCSB COMMAND.C:2045-2155
 *     F0380_COMMAND_ProcessQueue_CPSC, CLIKMENU.C:142-174 F0365 turn /
 *     CLIKMENU.C:180-390 F0366 move, and GAMELOOP.C:164-219 V1 input.
 *
 * The gate is pure data-free logic - no SDL, no graphics, no game data.
 * All callers pass structs in / structs out; every branch is reachable
 * from a test harness.  This keeps the contract lockable while leaving
 * the SDL3 finger-event dispatcher and FS_InputQueue push where they
 * already live (firestaff_touch.c).
 */

#include "runtime_gesture_navigation_gate.h"

#include <string.h>

/* FS_Command enum mirrors firestaff_input.h so we don't pull that
 * header into this data-free module.  Order matches firestaff_input.h:
 *   FS_CMD_NONE = 0
 *   FS_CMD_MOVE_FORWARD = 1
 *   FS_CMD_MOVE_BACKWARD = 2
 *   FS_CMD_TURN_LEFT = 3
 *   FS_CMD_TURN_RIGHT = 4
 *   FS_CMD_STRAFE_LEFT = 5
 *   FS_CMD_STRAFE_RIGHT = 6
 * Keeping the codes here lets the gate be a leaf module that probes
 * can link without the FS_InputQueue machinery.
 */
#define FS_CMD_NONE_VALUE          0
#define FS_CMD_MOVE_FORWARD_VALUE  1
#define FS_CMD_MOVE_BACKWARD_VALUE 2
#define FS_CMD_TURN_LEFT_VALUE     3
#define FS_CMD_TURN_RIGHT_VALUE    4
#define FS_CMD_STRAFE_LEFT_VALUE   5
#define FS_CMD_STRAFE_RIGHT_VALUE  6

/* Internal axis-dominant classification helper.  Returns:
 *   'h' if horizontal travel dominates, 'v' if vertical, 'e' on edge.
 * Caller is responsible for the threshold gate.
 */
static char fs_runtime_gesture_axis(int dx, int dy) {
    int absDx = dx < 0 ? -dx : dx;
    int absDy = dy < 0 ? -dy : dy;
    if (absDx == 0 && absDy == 0) return 'e';
    if (absDx >= absDy) return 'h';
    return 'v';
}

/* Internal: clamp a default threshold when caller passed 0.  The
 * 40 px floor mirrors FIRESTAFF_TOUCH_SWIPE_THRESHOLD_PX in
 * firestaff_touch.h. We deliberately do not include firestaff_touch.h
 * here so the gate stays a leaf module - the threshold value is part
 * of the source-locked contract, not an include dependency.
 */
#define RUNTIME_GESTURE_NAV_DEFAULT_SWIPE_THRESHOLD_PX 40

int FirestaffRuntimeGestureNav_Evaluate(
    const FirestaffRuntimeGestureNavEvent* event,
    const FirestaffRuntimeGestureNavPolicy* policy,
    FirestaffRuntimeGestureNavResult* result) {

    int dx, dy, absDx, absDy;
    int threshold;
    FirestaffRuntimeGestureNavDecision decision;
    int command;
    char axis;
    int travel;

    if (!result) return 0;
    memset(result, 0, sizeof(*result));

    if (!event || !policy) {
        decision = RUNTIME_GESTURE_NAV_DECISION_REJECTED_NULL;
        result->decision = decision;
        result->commandCode = FS_CMD_NONE_VALUE;
        result->dominantAxis = 'e';
        result->travelPx = 0;
        return 1;
    }

    /* Setting gate: when touch is off in launcher, gestures are off. */
    if (!policy->accessibilityTouchEnabled) {
        result->decision = RUNTIME_GESTURE_NAV_DECISION_REJECTED_DISABLED;
        result->commandCode = FS_CMD_NONE_VALUE;
        result->dominantAxis = 'e';
        result->travelPx = 0;
        return 1;
    }

    /* Resolve the effective threshold: caller-supplied takes priority,
     * else fall back to the 40 px source-locked default.  A negative
     * caller threshold is treated as "use default". */
    if (event->thresholdPx > 0) {
        threshold = event->thresholdPx;
    } else {
        threshold = RUNTIME_GESTURE_NAV_DEFAULT_SWIPE_THRESHOLD_PX;
    }

    dx = event->endX - event->startX;
    dy = event->endY - event->startY;
    absDx = dx < 0 ? -dx : dx;
    absDy = dy < 0 ? -dy : dy;

    axis = fs_runtime_gesture_axis(dx, dy);

    if (event->kind == FIRESTAFF_RUNTIME_GESTURE_NAV_KIND_EDGE_STRAFE) {
        /* Edge-strafe: V2-only. */
        int fbW = event->fbW > 0 ? event->fbW : 320;
        int edgePx = (fbW * 20) / 100; /* 20% from each edge, matches
                                         FIRESTAFF_TOUCH_EDGE_ZONE_FRAC
                                         in firestaff_touch.c */
        int safeLeft = event->startX < edgePx;
        int safeRight = event->startX >= fbW - edgePx;
        int safeWidth = edgePx;
        if (safeWidth < RUNTIME_GESTURE_NAV_MIN_TARGET_PX) {
            /* The edge-strafe zone itself is too small to count as a
             * safe touch target on the running surface. */
            result->decision =
                RUNTIME_GESTURE_NAV_DECISION_REJECTED_TOO_SMALL_TARGET;
            result->commandCode = FS_CMD_NONE_VALUE;
            result->dominantAxis = 'e';
            result->travelPx = safeWidth;
            return 1;
        }
        if (!safeLeft && !safeRight) {
            /* Touch started inside the viewport but not in either edge
             * zone - caller should re-classify as a swipe. */
            result->decision = RUNTIME_GESTURE_NAV_DECISION_REJECTED_AMBIGUOUS;
            result->commandCode = FS_CMD_NONE_VALUE;
            result->dominantAxis = 'e';
            result->travelPx = 0;
            return 1;
        }
        /* V2 must be on; V1 parity guard must be off. */
        if (!policy->v2PresentationEnabled ||
            policy->v1ParityPreserve) {
            result->decision =
                RUNTIME_GESTURE_NAV_DECISION_REJECTED_V1_ONLY;
            result->commandCode = FS_CMD_NONE_VALUE;
            result->dominantAxis = 'e';
            result->travelPx = safeWidth;
            return 1;
        }
        if (safeLeft) {
            decision = RUNTIME_GESTURE_NAV_DECISION_EMIT_STRAFE_LEFT;
            command = FS_CMD_STRAFE_LEFT_VALUE;
        } else {
            decision = RUNTIME_GESTURE_NAV_DECISION_EMIT_STRAFE_RIGHT;
            command = FS_CMD_STRAFE_RIGHT_VALUE;
        }
        result->decision = decision;
        result->commandCode = command;
        result->dominantAxis = 'e';
        result->travelPx = safeWidth;
        return 1;
    }

    /* Swipe path. The tap-tolerance guard preserves V1 click fidelity
     * - a touch that moves <= tapTolerancePx is a tap, not a swipe. */
    if (event->tapTolerancePx > 0 &&
        absDx <= event->tapTolerancePx &&
        absDy <= event->tapTolerancePx) {
        result->decision = RUNTIME_GESTURE_NAV_DECISION_REJECTED_TOO_SHORT;
        result->commandCode = FS_CMD_NONE_VALUE;
        result->dominantAxis = axis;
        result->travelPx = (absDx > absDy) ? absDx : absDy;
        return 1;
    }

    if (absDx < threshold && absDy < threshold) {
        result->decision = RUNTIME_GESTURE_NAV_DECISION_REJECTED_TOO_SHORT;
        result->commandCode = FS_CMD_NONE_VALUE;
        result->dominantAxis = axis;
        result->travelPx = (absDx > absDy) ? absDx : absDy;
        return 1;
    }

    /* Both axes dominate the threshold but neither dominates the other:
     * ambiguous (diagonal) swipe is rejected so the caller can fall back
     * to V1 keyboard input rather than picking a wrong axis. */
    if (absDx >= threshold && absDy >= threshold && absDx == absDy) {
        result->decision = RUNTIME_GESTURE_NAV_DECISION_REJECTED_AMBIGUOUS;
        result->commandCode = FS_CMD_NONE_VALUE;
        result->dominantAxis = axis;
        result->travelPx = absDx;
        return 1;
    }

    /* V1 parity: swipes are safe in both V1 and V2 because they push
     * the same FS_CMD_MOVE_FORWARD / FS_CMD_TURN_RIGHT codes that the
     * V1 keyboard path already uses. The v2PresentationEnabled flag is
     * therefore intentionally NOT consulted here - swipes are the
     * cross-V1/V2 path. Edge-strafe is the V2-only branch above. */
    if (axis == 'h') {
        travel = absDx;
        if (dx > 0) {
            decision = RUNTIME_GESTURE_NAV_DECISION_EMIT_TURN_RIGHT;
            command = FS_CMD_TURN_RIGHT_VALUE;
        } else {
            decision = RUNTIME_GESTURE_NAV_DECISION_EMIT_TURN_LEFT;
            command = FS_CMD_TURN_LEFT_VALUE;
        }
    } else {
        travel = absDy;
        if (dy > 0) {
            decision = RUNTIME_GESTURE_NAV_DECISION_EMIT_BACKWARD;
            command = FS_CMD_MOVE_BACKWARD_VALUE;
        } else {
            decision = RUNTIME_GESTURE_NAV_DECISION_EMIT_FORWARD;
            command = FS_CMD_MOVE_FORWARD_VALUE;
        }
    }

    result->decision = decision;
    result->commandCode = command;
    result->dominantAxis = axis;
    result->travelPx = travel;
    return 1;
}

int FirestaffRuntimeGestureNav_TouchTargetSafe(int widthPx, int heightPx) {
    if (widthPx < 0 || heightPx < 0) return 0;
    if (widthPx < RUNTIME_GESTURE_NAV_MIN_TARGET_PX) return 0;
    if (heightPx < RUNTIME_GESTURE_NAV_MIN_TARGET_PX) return 0;
    return 1;
}

int FirestaffRuntimeGestureNav_SourceViewportSafe(
    int sourceW, int sourceH, int surfaceW, int surfaceH) {
    int sx, sy;

    if (sourceW <= 0 || sourceH <= 0) return 0;
    if (surfaceW <= 0 || surfaceH <= 0) return 0;

    /* Compute the integer-nearest scale factor for the shortest
     * framebuffer axis, mirroring the M11 letterbox math used by
     * m11_letterbox_render_pc34_compat (smallest integer scale that
     * fits the surface). */
    sx = surfaceW / sourceW;
    sy = surfaceH / sourceH;
    if (sx <= 0) sx = 1;
    if (sy <= 0) sy = 1;

    /* Safe = the source framebuffer's shortest axis scales to >= MIN_TARGET_PX. */
    if (sourceW < sourceH) {
        /* sourceW is the shortest axis */
        if (sx >= 1) {
            return sourceW * sx >= RUNTIME_GESTURE_NAV_MIN_TARGET_PX;
        }
        return sourceW >= RUNTIME_GESTURE_NAV_MIN_TARGET_PX;
    } else {
        if (sy >= 1) {
            return sourceH * sy >= RUNTIME_GESTURE_NAV_MIN_TARGET_PX;
        }
        return sourceH >= RUNTIME_GESTURE_NAV_MIN_TARGET_PX;
    }
}

const char* FirestaffRuntimeGestureNav_GetSourceEvidence(void) {
    return "ReDMCSB COMMAND.C:2045-2155 F0380_COMMAND_ProcessQueue_CPSC "
           "(movement/turn queue dispatch); CLIKMENU.C:142-174 F0365 turn "
           "(turn dispatch); CLIKMENU.C:180-390 F0366 move (step dispatch); "
           "GAMELOOP.C:164-219 V1 input wait loop (V1 parity surface); "
           "DEFS.H:238-243 C001..C006 movement commands; "
           "DEFS.H:197-211 C003/C005 strafe owner; "
           "src/engine/firestaff_touch.c FIRESTAFF_TOUCH_SWIPE_THRESHOLD_PX=40, "
           "FIRESTAFF_TOUCH_TAP_TOLERANCE_PX=24, FIRESTAFF_TOUCH_LONG_PRESS_MS=500, "
           "FIRESTAFF_TOUCH_EDGE_ZONE_FRAC=0.20; "
           "src/dm1v2/dm1_v2_touch_controller_affordance_pc34.c "
           "DM1_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_LEFT/RIGHT (V2-only edge-strafe); "
           "include/config_m12.h dm1V2AccessibilityTouchEnabled (setting gate). "
           "Touch-target floor 44 px: Apple HIG 44x44 pt + Material Design 48 dp + "
           "WCAG 2.5.5 44 px / iOS HIG.";
}

const char* FirestaffRuntimeGestureNav_DecisionName(
    FirestaffRuntimeGestureNavDecision decision) {
    switch (decision) {
    case RUNTIME_GESTURE_NAV_DECISION_REJECTED_DISABLED:
        return "rejected_disabled";
    case RUNTIME_GESTURE_NAV_DECISION_REJECTED_TOO_SHORT:
        return "rejected_too_short";
    case RUNTIME_GESTURE_NAV_DECISION_REJECTED_TOO_SMALL_TARGET:
        return "rejected_too_small_target";
    case RUNTIME_GESTURE_NAV_DECISION_REJECTED_AMBIGUOUS:
        return "rejected_ambiguous";
    case RUNTIME_GESTURE_NAV_DECISION_REJECTED_NULL:
        return "rejected_null";
    case RUNTIME_GESTURE_NAV_DECISION_REJECTED_V1_ONLY:
        return "rejected_v1_only";
    case RUNTIME_GESTURE_NAV_DECISION_EMIT_FORWARD:
        return "emit_forward";
    case RUNTIME_GESTURE_NAV_DECISION_EMIT_BACKWARD:
        return "emit_backward";
    case RUNTIME_GESTURE_NAV_DECISION_EMIT_TURN_LEFT:
        return "emit_turn_left";
    case RUNTIME_GESTURE_NAV_DECISION_EMIT_TURN_RIGHT:
        return "emit_turn_right";
    case RUNTIME_GESTURE_NAV_DECISION_EMIT_STRAFE_LEFT:
        return "emit_strafe_left";
    case RUNTIME_GESTURE_NAV_DECISION_EMIT_STRAFE_RIGHT:
        return "emit_strafe_right";
    default:
        return "unknown";
    }
}

/*
 * runtime_gesture_navigation_gate.h
 *
 * Cross-game runtime gesture navigation gate for movement / turning.
 *
 * This module wraps the existing `firestaff_touch.c` swipe + edge-strafe
 * detection primitives into a single, deterministic, data-free contract
 * that the M11 / per-game V1 game loops consult before emitting a
 * gesture-driven FS_Command onto the FS_InputQueue.
 *
 * Scope (gap `Gesture navigation for runtime movement/turning` from
 * docs/FIRESTAFF_GAP_LIST.md G2 + TODO.md 2026-06-28 lane):
 *   - Input translation:
 *       swipe up    -> FS_CMD_MOVE_FORWARD
 *       swipe down  -> FS_CMD_MOVE_BACKWARD
 *       swipe left  -> FS_CMD_TURN_LEFT
 *       swipe right -> FS_CMD_TURN_RIGHT
 *       edge-left   -> FS_CMD_STRAFE_LEFT  (V2 only)
 *       edge-right  -> FS_CMD_STRAFE_RIGHT (V2 only)
 *   - Touch-target safety:
 *       pin a deterministic minimum framebuffer-space target size
 *       (44 px, Apple HIG + Material Design 48 dp floor) so the
 *       framebuffer-space edge zones, swipe thresholds, and tap
 *       tolerances never collapse below ergonomic limits on a
 *       320x200 source viewport.
 *   - Setting gate:
 *       gestures only emit when `accessibilityTouchEnabled` is on
 *       (V2 only - preserves V1 keyboard/mouse parity when off).
 *
 * Source lock:
 *   ReDMCSB COMMAND.C:2045-2155 F0380_COMMAND_ProcessQueue_CPSC
 *     consumes the same C001..C006 movement IDs that the FS_InputQueue
 *     flushes; CLIKMENU.C:142-174 F0365 turn / CLIKMENU.C:180-390 F0366
 *     step dispatch them. ReDMCSB GAMELOOP.C:164-219 V1 input wait loop
 *     is the only consumer that matters for V1 parity.
 *   DEFS.H:238-243 C005/C006 strafe commands are DM1-side; CSB/DM2/
 *     Nexus use the same source-locked C005/C006 IDs.
 *   The V2-only edge-strafe affordances are DM1_V2_AFFORDANCE_TOUCH_
 *     EDGE_STRAFE_LEFT/RIGHT (src/dm1v2/dm1_v2_touch_controller_
 *     affordance_pc34.c).
 *   firestaff_touch.c owns the existing swipe detect / edge zone
 *     primitives. This module is the gate that the game loops consult
 *     before forwarding those primitives into the FS_InputQueue.
 *
 * Data-free: no game data, no SDL events, no V1/V2 state - every input
 * is a struct field, every output is a struct field, every check is
 * deterministic.
 */

#ifndef FIRESTAFF_RUNTIME_GESTURE_NAVIGATION_GATE_H
#define FIRESTAFF_RUNTIME_GESTURE_NAVIGATION_GATE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Minimum framebuffer-space target size for touch-safe UI affordances.
 * 44 px is Apple HIG (44x44 pt @ 1x) and the smallest of the Material
 * Design 48 dp / WCAG 2.5.5 44 px / iOS 44 pt / Android 48 dp targets.
 * We pin 44 px because that is the smallest documented recommendation
 * the gate must respect on a 320x200 source viewport. */
#define RUNTIME_GESTURE_NAV_MIN_TARGET_PX 44

/* Decision codes reported by the gate. The numerical ordering is part
 * of the public contract - callers index switch tables by it. */
typedef enum {
    RUNTIME_GESTURE_NAV_DECISION_REJECTED_DISABLED = 0,
    RUNTIME_GESTURE_NAV_DECISION_REJECTED_TOO_SHORT = 1,
    RUNTIME_GESTURE_NAV_DECISION_REJECTED_TOO_SMALL_TARGET = 2,
    RUNTIME_GESTURE_NAV_DECISION_REJECTED_AMBIGUOUS = 3,
    RUNTIME_GESTURE_NAV_DECISION_REJECTED_NULL = 4,
    RUNTIME_GESTURE_NAV_DECISION_REJECTED_V1_ONLY = 5,
    RUNTIME_GESTURE_NAV_DECISION_EMIT_FORWARD = 10,
    RUNTIME_GESTURE_NAV_DECISION_EMIT_BACKWARD = 11,
    RUNTIME_GESTURE_NAV_DECISION_EMIT_TURN_LEFT = 12,
    RUNTIME_GESTURE_NAV_DECISION_EMIT_TURN_RIGHT = 13,
    RUNTIME_GESTURE_NAV_DECISION_EMIT_STRAFE_LEFT = 14,
    RUNTIME_GESTURE_NAV_DECISION_EMIT_STRAFE_RIGHT = 15
} FirestaffRuntimeGestureNavDecision;

/* Gesture kind derived from a finger path or edge-zone hit-test.
 * Caller classifies the input; the gate applies the policy. */
typedef enum {
    FIRESTAFF_RUNTIME_GESTURE_NAV_KIND_NONE = 0,
    FIRESTAFF_RUNTIME_GESTURE_NAV_KIND_SWIPE = 1,
    FIRESTAFF_RUNTIME_GESTURE_NAV_KIND_EDGE_STRAFE = 2
} FirestaffRuntimeGestureNavKind;

/* A single gesture candidate to evaluate.
 * Coordinates are framebuffer-space (320x200 source).
 *
 * For SWIPE:    startX/startY -> endX/endY, thresholdPx = swipe threshold.
 * For EDGE:     startX is enough (startY ignored); endX/endY are 0.
 *               The framebuffer width `fbW` is required so the edge-zone
 *               check can classify left vs right.
 */
typedef struct {
    FirestaffRuntimeGestureNavKind kind;
    int startX;
    int startY;
    int endX;
    int endY;
    int fbW;             /* framebuffer width (320 on V1 source)         */
    int thresholdPx;     /* minimum total travel for a swipe             */
    int tapTolerancePx;  /* max travel that still counts as a tap        */
} FirestaffRuntimeGestureNavEvent;

/* Policy / setting flags the gate consults. The fields map directly
 * onto the existing launcher / V2 settings so callers don't need a
 * cross-module dependency:
 *   accessibilityTouchEnabled  -- config_m12.dm1V2AccessibilityTouchEnabled
 *   v2PresentationEnabled      -- per-game V2 presentation gate
 *   v1ParityPreserve           -- 1 = reject edge-strafe (V1 parity)
 */
typedef struct {
    int accessibilityTouchEnabled;
    int v2PresentationEnabled;
    int v1ParityPreserve;
} FirestaffRuntimeGestureNavPolicy;

/* Result of evaluating one event under one policy.
 *
 * `decision` is one of the FIRESTAFF_RUNTIME_GESTURE_NAV_DECISION_*
 * values. `commandCode` is the FS_Command that the gate would push
 * onto the FS_InputQueue when decision is one of the EMIT_* values;
 * it is 0 (FS_CMD_NONE) for every REJECTED_* value. `dominantAxis`
 * is 'h' for horizontal-dominant swipes, 'v' for vertical-dominant,
 * and 'e' for edge-strafe hits. `travelPx` is the integer euclidean
 * proxy (|dx| or |dy| whichever dominates) so probes can verify the
 * threshold boundary.
 */
typedef struct {
    FirestaffRuntimeGestureNavDecision decision;
    int commandCode;     /* FS_CMD_* mapped value (0 = none) */
    char dominantAxis;   /* 'h', 'v', or 'e' */
    int travelPx;
} FirestaffRuntimeGestureNavResult;

/* Public API */

/* Evaluate one event under one policy and fill *result. Returns 1 on
 * success, 0 on hard error (NULL out-pointer). Never reads past the
 * end of either struct. Safe to call with any combination of fields
 * including all-zero inputs. */
int FirestaffRuntimeGestureNav_Evaluate(
    const FirestaffRuntimeGestureNavEvent* event,
    const FirestaffRuntimeGestureNavPolicy* policy,
    FirestaffRuntimeGestureNavResult* result);

/* Touch-target safety contract: returns 1 when the framebuffer-space
 * zone of size (widthPx, heightPx) meets the RUNTIME_GESTURE_NAV_MIN_
 * TARGET_PX minimum on both axes. 0 otherwise. Always returns 0 when
 * either dimension is negative. */
int FirestaffRuntimeGestureNav_TouchTargetSafe(int widthPx, int heightPx);

/* Touch-target safety on the source viewport (320x200): returns 1 when
 * the framebuffer itself, scaled to the running presentation surface,
 * exposes at least RUNTIME_GESTURE_NAV_MIN_TARGET_PX for the shortest
 * touch-zone dimension.  Used to gate V2 launch when the surface
 * would shrink the touch targets below safe limits. */
int FirestaffRuntimeGestureNav_SourceViewportSafe(
    int sourceW, int sourceH, int surfaceW, int surfaceH);

/* Public source-evidence string for grep-verifiable spec citation. */
const char* FirestaffRuntimeGestureNav_GetSourceEvidence(void);

/* Decision-name helper (string form, e.g. for log output). */
const char* FirestaffRuntimeGestureNav_DecisionName(
    FirestaffRuntimeGestureNavDecision decision);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_RUNTIME_GESTURE_NAVIGATION_GATE_H */

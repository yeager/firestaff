#ifndef FIRESTAFF_RUNTIME_GESTURE_NAVIGATION_GATE_PC34_COMPAT_H
#define FIRESTAFF_RUNTIME_GESTURE_NAVIGATION_GATE_PC34_COMPAT_H

/*
 * Cross-game runtime gesture navigation gate for movement/turning.
 *
 * This gate sits ABOVE the existing touch pointer / mouse click pipeline and
 * the swipe detection layer (firestaff_touch.c / firestaff_input.c) and
 * sits BEHIND the launcher-side touch/controller settings
 * (M12 settings.inputModeIndex / settings.touchControlsIndex).
 *
 * The gate is data-free and cross-game: it does NOT decode game data, it
 * only validates that the input-translation contract from a finger path
 * (or a synthesised tap) reaches the canonical V1 turn/move commands
 * safely. The gate does not add any gesture behaviour beyond what the
 * existing touch/click zone pipeline already exposes - it pins down
 * that contract with bounded, machine-checkable invariants so future
 * touch-affordance work (large touch targets, custom gestures, etc.)
 * can build on top of the gate without regressing the basic route.
 *
 * Source lock for the underlying movement/turning command IDs:
 * - ReDMCSB COMMAND.C:396-405 (movement arrow mouse table).
 * - ReDMCSB COMMAND.C:2045-2156 F0380 (queue dispatch to F0365 turn /
 *   F0366 move).
 * - ReDMCSB COMMAND.C:2296-2324 (C083 inventory / C111 action / C080 click).
 * - ReDMCSB CLIKMENU.C:142-179 F0365 (turn handling).
 * - ReDMCSB CLIKMENU.C:180-347 F0366 (movement handling).
 * - firestaff_touch.c (firestaff_touch_detect_swipe, FIRESTAFF_TOUCH_SWIPE_THRESHOLD_PX).
 * - firestaff_input.c (FS_Command enum, fs_input_queue_push/pop).
 * - src/ui/menu_startup_m12.c (settings.inputModeIndex A/K/T/G labels,
 *   settings.touchControlsIndex O/M/F/L labels).
 */

#include <stdint.h>
#include "firestaff_input.h"
#include "touch_pointer_input_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* M12 input-mode indices. Mirrors `g_inputModeLabels[]` order in
 * src/ui/menu_startup_m12.c. We use raw integers (not the upstream enum
 * which is private to M12) so the gate stays M12-decoupled. */
typedef enum GestureNavInputModePc34Compat {
    GESTURE_NAV_INPUT_MODE_AUTO_PC34_COMPAT            = 0,
    GESTURE_NAV_INPUT_MODE_KEYBOARD_MOUSE_PC34_COMPAT  = 1,
    GESTURE_NAV_INPUT_MODE_TOUCH_PC34_COMPAT           = 2,
    GESTURE_NAV_INPUT_MODE_GAMEPAD_PC34_COMPAT         = 3
} GestureNavInputModePc34Compat;

/* M12 touch-controls index. Mirrors `g_touchControlsLabels[]` order. */
typedef enum GestureNavTouchLevelPc34Compat {
    GESTURE_NAV_TOUCH_LEVEL_OFF_PC34_COMPAT     = 0,
    GESTURE_NAV_TOUCH_LEVEL_MINIMAL_PC34_COMPAT = 1,
    GESTURE_NAV_TOUCH_LEVEL_FULL_PC34_COMPAT    = 2,
    GESTURE_NAV_TOUCH_LEVEL_LARGE_PC34_COMPAT   = 3
} GestureNavTouchLevelPc34Compat;

/* Cross-game enumeration. Used by the gate to assert that the
 * movement/turning route is identical across all five supported games
 * (DM1, CSB, DM2, Nexus, Theron). The gate is intentionally agnostic:
 * no per-game logic - the gate only cares that the input pipeline
 * routes to the canonical V1 turn/move command IDs which the per-game
 * game-loop layer dispatches identically. */
typedef enum GestureNavGameIdPc34Compat {
    GESTURE_NAV_GAME_DM1_PC34_COMPAT    = 0,
    GESTURE_NAV_GAME_CSB_PC34_COMPAT    = 1,
    GESTURE_NAV_GAME_DM2_PC34_COMPAT    = 2,
    GESTURE_NAV_GAME_NEXUS_PC34_COMPAT  = 3,
    GESTURE_NAV_GAME_THERON_PC34_COMPAT = 4
} GestureNavGameIdPc34Compat;

/* Touch-target safety classification for a single navigation gesture.
 * The DMWeb / Accessibility / M12 touch-target audit guidance requires
 * that any touch target in a gesture-driven navigation surface must be
 * at least FIRESTAFF_GESTURE_NAV_MIN_TARGET_PX pixels wide AND tall
 * in the source framebuffer (320x200) so that a fat-finger operator
 * does not accidentally activate an adjacent gesture. */
typedef enum GestureNavTargetSafetyPc34Compat {
    GESTURE_NAV_TARGET_SAFETY_UNKNOWN_PC34_COMPAT     = 0,
    GESTURE_NAV_TARGET_SAFETY_SAFE_PC34_COMPAT        = 1,
    GESTURE_NAV_TARGET_SAFETY_UNDER_THRESHOLD_PC34_COMPAT = 2,
    GESTURE_NAV_TARGET_SAFETY_OUT_OF_BOUNDS_PC34_COMPAT   = 3,
    GESTURE_NAV_TARGET_SAFETY_INPUT_DISABLED_PC34_COMPAT  = 4
} GestureNavTargetSafetyPc34Compat;

/* Per-direction movement/turning navigation target. The gate consumes
 * the existing touch_click_zone_matrix entry for the canonical V1
 * command ID (TURN_LEFT/TURN_RIGHT/MOVE_FORWARD/MOVE_BACKWARD) and
 * classifies its framebuffer size. */
typedef struct GestureNavDirectionTargetPc34Compat {
    int                     directionOrdinal; /* 0..3 in source-locked order */
    unsigned int            commandId;
    FS_Command              fsCommand;
    const char*             label;
    TouchClickZonePc34Compat zone;
    int                     zoneFound;
    GestureNavTargetSafetyPc34Compat safety;
} GestureNavDirectionTargetPc34Compat;

/* Full per-frame snapshot of the gesture navigation gate state. */
typedef struct GestureNavSnapshotPc34Compat {
    GestureNavInputModePc34Compat   inputMode;
    GestureNavTouchLevelPc34Compat  touchLevel;
    GestureNavGameIdPc34Compat      gameId;
    int                             gateActive;          /* 1 if touch nav is allowed right now */
    int                             swipeThresholdPx;
    int                             tapTolerancePx;
    int                             longPressMs;
    int                             edgeZonePercent;     /* 0..100, V2-only strafe zones */
    int                             targetSafeCount;
    int                             targetUnderCount;
    int                             targetDisabledCount;
    int                             targetOutOfBoundsCount;
    GestureNavDirectionTargetPc34Compat targets[4];
} GestureNavSnapshotPc34Compat;

/* Configuration applied by the launcher / runtime when touch controls
 * are first activated. Defaults are the existing firestaff_touch.c
 * thresholds. The gate does NOT mutate these during runtime - the
 * launcher owns persistence. */
typedef struct GestureNavConfigPc34Compat {
    GestureNavInputModePc34Compat   inputMode;
    GestureNavTouchLevelPc34Compat  touchLevel;
    GestureNavGameIdPc34Compat      gameId;
    int                             framebufferW;     /* 320 by default */
    int                             framebufferH;     /* 200 by default */
    int                             enableEdgeStrafe; /* 0/1, V2 only */
} GestureNavConfigPc34Compat;

/* Built-in defaults, also captured here for the test/probe contract. */
#define FIRESTAFF_GESTURE_NAV_MIN_TARGET_PX      16
#define FIRESTAFF_GESTURE_NAV_RECOMMENDED_PX     24
#define FIRESTAFF_GESTURE_NAV_DEFAULT_FB_W       320
#define FIRESTAFF_GESTURE_NAV_DEFAULT_FB_H       200
#define FIRESTAFF_GESTURE_NAV_EDGE_ZONE_PERCENT  20
#define FIRESTAFF_GESTURE_NAV_DEFAULT_SWIPE_PX   40
#define FIRESTAFF_GESTURE_NAV_DEFAULT_TAP_PX     24
#define FIRESTAFF_GESTURE_NAV_DEFAULT_LONG_MS    500

/* Reset the snapshot to a known zero state. */
void GESTURENAV_Compat_ResetSnapshotPc34Compat(GestureNavSnapshotPc34Compat* snapshot);

/* Apply the launcher/runtime config into the snapshot. Returns 1 if the
 * gate is active (touch nav allowed) for this config, 0 otherwise.
 * The gate is active iff touchLevel >= MINIMAL AND inputMode is
 * AUTO or TOUCH. KEYBOARD+MOUSE-only and GAMEPAD-only both disable
 * the gate (the keyboard/gamepad paths still work via their own
 * command queues). */
int GESTURENAV_Compat_ApplyConfigPc34Compat(const GestureNavConfigPc34Compat* config,
                                            GestureNavSnapshotPc34Compat* snapshot);

/* Compute the touch-target safety snapshot for the four canonical
 * movement/turning directions (TURN_LEFT, TURN_RIGHT, MOVE_FORWARD,
 * MOVE_BACKWARD) using the live touch_click_zone_matrix. Returns the
 * number of SAFE targets after the call. */
int GESTURENAV_Compat_ClassifyTouchTargetsPc34Compat(GestureNavSnapshotPc34Compat* snapshot);

/* Resolve a finger-path swipe to an FS_Command under the gate. The
 * swipe is in framebuffer pixel space (320x200). Returns FS_CMD_NONE
 * when the gate is not active, when the path is below threshold, or
 * when the path is degenerate (zero-length). When the gate is active,
 * the FS_Command always matches the source-locked V1 movement/turning
 * command IDs (FS_CMD_MOVE_FORWARD/BACKWARD/TURN_LEFT/TURN_RIGHT). */
FS_Command GESTURENAV_Compat_ResolveSwipePc34Compat(const GestureNavSnapshotPc34Compat* snapshot,
                                                    int startX, int startY,
                                                    int endX, int endY,
                                                    int* outDirection);

/* Resolve a single-tap (left-button) touch input through the existing
 * touch_pointer_input pipeline under the gate. Returns 1 if the tap
 * dispatches a mouse command, 0 otherwise. When the gate is active,
 * the dispatch always resolves to the canonical V1 command IDs. When
 * the gate is not active, this returns 0 so the caller's input is
 * dropped (KEYBOARD+MOUSE / GAMEPAD users use their own paths). */
int GESTURENAV_Compat_ResolveTapPc34Compat(const GestureNavSnapshotPc34Compat* snapshot,
                                           int screenX, int screenY,
                                           unsigned int buttonMask,
                                           TouchPointerDispatchPc34Compat* outDispatch);

/* Push a swipe-resolved FS_Command into a real FS_InputQueue (the same
 * queue that fs_game_tick_v1() drains). Returns 1 if pushed, 0 if the
 * queue is full or the gate refused the input. */
int GESTURENAV_Compat_EnqueueSwipePc34Compat(const GestureNavSnapshotPc34Compat* snapshot,
                                             FS_InputQueue* queue,
                                             int startX, int startY,
                                             int endX, int endY);

/* Touch-target safety predicate. Returns 1 if a framebuffer-space
 * touch target with the given width/height is "safe" (>= MIN_TARGET_PX
 * on both axes), 0 otherwise. */
int GESTURENAV_Compat_IsTargetSafePc34Compat(int widthPx, int heightPx);

/* Edge-strafe helper. Returns 1 if startX is in the left or right edge
 * strafe zone at the given framebuffer width, only when the gate has
 * enableEdgeStrafe=1 (V2-only). Returns 0 otherwise. */
int GESTURENAV_Compat_IsInEdgeStrafeZonePc34Compat(const GestureNavSnapshotPc34Compat* snapshot,
                                                   int startX);

/* Emit a strafe command when the touch starts in an edge zone and the
 * gate has enableEdgeStrafe=1. The strafe command is forwarded to the
 * queue using FS_CMD_STRAFE_LEFT/RIGHT. Returns 1 if pushed, 0 if
 * the gate refused (V1-only or non-edge). */
int GESTURENAV_Compat_EnqueueEdgeStrafePc34Compat(const GestureNavSnapshotPc34Compat* snapshot,
                                                  FS_InputQueue* queue,
                                                  int startX, int framebufferW);

/* Cross-game parity helper. Runs GESTURENAV_Compat_ApplyConfigPc34Compat
 * with the same config against every supported game id and records
 * whether the gate activates the same way. Returns the number of
 * games for which the gate activated (0..5). */
int GESTURENAV_Compat_VerifyCrossGameActivationPc34Compat(
    const GestureNavConfigPc34Compat* baseConfig,
    int perGameActivation[5]);

/* Source evidence citation. */
const char* GESTURENAV_Compat_GetSourceEvidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_RUNTIME_GESTURE_NAVIGATION_GATE_PC34_COMPAT_H */

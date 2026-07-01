/*
 * fs_gesture_navigation_gate.c — Cross-game runtime gesture navigation
 *
 * Implementation notes for the public surface declared in
 * include/fs_gesture_navigation_gate.h.  Every game-specific row in
 * the per-game translation table cites the ReDMCSB / sibling
 * source-lock that owns the underlying command id; the table is
 * fully data-free and self-contained.
 *
 * The recognizer is single-finger and platform-neutral.  It does
 * NOT call SDL directly; callers feed it via FsGestureFeedEvent.
 * This keeps the gate headless-testable and CI-friendly.
 *
 * Source-lock anchors (full citations in
 * docs/source-lock/fs_gesture_navigation_gate.md):
 *   - ReDMCSB COMMAND.C:2045-2156 F0380 process queue + dispatch.
 *   - ReDMCSB CLIKMENU.C:142-174  F0365 turn.
 *   - ReDMCSB CLIKMENU.C:180-347  F0366 movement.
 *   - ReDMCSB DEFS.H:238-243      C001..C006 movement command IDs.
 *   - ReDMCSB COMMAND.C:375-405   mouse movement arrow zones.
 *   - ReDMCSB COMMAND.C:1379-1449 F0358 mouse hit-test.
 *   - ReDMCSB COMMAND.C:1641-1644 F0359 primary → secondary search.
 *   - firestaff_touch.h           sibling DM1 V1 recognizer.
 *   - csb_v2_touch_runtime.h      sibling CSB V2 affordance translation.
 *   - dm2_v2_touch_runtime.h      sibling DM2 V2 affordance translation.
 *   - nexus_v2_touch_runtime.h    sibling Nexus V2 affordance translation.
 *   - theron_v1_mechanics.h       sibling Theron movement commands.
 */

#include "fs_gesture_navigation_gate.h"
#include <string.h>

/* ── Tunables (compile-time defaults; runtime audit thresholds are
 *    passed by the caller, not read here). */
#ifndef FS_GG_TAP_TOLERANCE_PX
#define FS_GG_TAP_TOLERANCE_PX 24
#endif

#ifndef FS_GG_SWIPE_THRESHOLD_PX
#define FS_GG_SWIPE_THRESHOLD_PX 40
#endif

#ifndef FS_GG_LONG_PRESS_MS
#define FS_GG_LONG_PRESS_MS 500u
#endif

#ifndef FS_GG_DOUBLE_TAP_MS
#define FS_GG_DOUBLE_TAP_MS 300u
#endif

#ifndef FS_GG_DOUBLE_TAP_DISTANCE_PX
#define FS_GG_DOUBLE_TAP_DISTANCE_PX 32
#endif

#ifndef FS_GG_EDGE_ZONE_FRAC_Q16
/* Right-shift-16 fixed point: 0.20 -> 0.20 * 65536 = 13107 */
#define FS_GG_EDGE_ZONE_FRAC_Q16 13107
#endif

/* ── Game-specific movement/turn command IDs ──────────────────────────
 * These ids mirror the source-locked V1 command constants the
 * per-game runtime expects.  DM1/CSB/DM2/Nexus all share the same
 * ReDMCSB C001..C006 movement IDs (DM1_V1_COMMAND_TURN_LEFT ==
 * CSB_V1_COMMAND_TURN_LEFT etc.).  Theron uses a smaller PCE
 * command-id set reserved by theron_v1_mechanics.h. */
enum {
    /* ReDMCSB DEFS.H:238-243 + dm1_v1_input_command_queue_pc34_compat.h */
    FS_GG_CMD_TURN_LEFT    = 1,
    FS_GG_CMD_TURN_RIGHT   = 2,
    FS_GG_CMD_MOVE_FORWARD = 3,
    FS_GG_CMD_MOVE_RIGHT   = 4,
    FS_GG_CMD_MOVE_BACK    = 5,
    FS_GG_CMD_MOVE_LEFT    = 6,

    /* Theron's Quest on PC Engine — same logical axes, smaller id set.
     * Source: theron_v1_mechanics.h; kept opaque here so the gate does
     * not pull in the PCE runtime headers. */
    FS_GG_CMD_THERON_FORWARD = 0x10,
    FS_GG_CMD_THERON_BACK    = 0x11,
    FS_GG_CMD_THERON_TURN_L  = 0x12,
    FS_GG_CMD_THERON_TURN_R  = 0x13,
    FS_GG_CMD_THERON_STRAFE_L= 0x14,
    FS_GG_CMD_THERON_STRAFE_R= 0x15,
    FS_GG_CMD_THERON_NONE    = 0x00
};

/* ── Recognizer state ────────────────────────────────────────────────── */
typedef struct {
    int armed;                       /* 1 if DOWN seen, waiting for UP */
    int downX, downY;                /* touch-down coords */
    int lastX, lastY;                /* last MOVE / current UP coords */
    uint32_t downMs;                 /* clock at DOWN */
    uint32_t lastUpMs;               /* clock at last completed UP */
    int lastUpX, lastUpY;            /* last completed UP coords */
    FsGestureType lastGesture;       /* last completed gesture */
    FsGestureType pendingGesture;    /* set by DOWN; flushed on UP */
    int hasPending;                  /* 1 if pendingGesture is valid */
    int fingerId;                    /* tracked finger id (always 0) */
} FsGestureRecognizerState;

/* ── Module state ────────────────────────────────────────────────────── */
static int g_initialized = 0;
static int g_enabled[FS_GG_GAME_ID_COUNT] = { 0 }; /* all OFF by default */
static FsGgGameId g_active_game = FS_GG_GAME_DM1;
static FsGestureRecognizerState g_recog;
static FsGestureGateCounters g_counters;

/* ── Per-game translation row ───────────────────────────────────────── */
typedef struct {
    FsGgGameId game;
    FsGestureType gesture;
    int command;                     /* game command id; -1 if N/A */
    int isMovement;
    int isTurn;
    const char* sourceEvidence;
} FsGestureGameRow;

static const FsGestureGameRow kPerGameTable[] = {
    /* ── DM1 (FS_GG_GAME_DM1) ────────────────────────────────────────── */
    /* Source-locked against ReDMCSB COMMAND.C:375-405 (movement arrows),
     * CLIKMENU.C:142 F0365 (turn), CLIKMENU.C:180 F0366 (movement),
     * DEFS.H:238-243 (C001..C006), and dm1_v1_input_command_queue_pc34_compat.h
     * (DM1_V1_COMMAND_*). */
    { FS_GG_GAME_DM1, FS_GG_GESTURE_SWIPE_UP,          FS_GG_CMD_MOVE_FORWARD, 1, 0, "ReDMCSB COMMAND.C:2045-2156 F0380 process queue + DEFS.H:238-243 C003 MOVE_FORWARD" },
    { FS_GG_GAME_DM1, FS_GG_GESTURE_SWIPE_DOWN,        FS_GG_CMD_MOVE_BACK,    1, 0, "ReDMCSB COMMAND.C:2045-2156 F0380 process queue + DEFS.H:238-243 C005 MOVE_BACKWARD" },
    { FS_GG_GAME_DM1, FS_GG_GESTURE_SWIPE_LEFT,        FS_GG_CMD_TURN_LEFT,    0, 1, "ReDMCSB CLIKMENU.C:142-174 F0365 turn handling + DEFS.H:238-243 C001 TURN_LEFT" },
    { FS_GG_GAME_DM1, FS_GG_GESTURE_SWIPE_RIGHT,       FS_GG_CMD_TURN_RIGHT,   0, 1, "ReDMCSB CLIKMENU.C:142-174 F0365 turn handling + DEFS.H:238-243 C002 TURN_RIGHT" },
    { FS_GG_GAME_DM1, FS_GG_GESTURE_LONG_PRESS,        -1, 0, 0, "ReDMCSB INPUT.C:574-664 long-press synthesizes right-button; COMAND.C:2296-2300 C083 inventory toggle on right click" },
    { FS_GG_GAME_DM1, FS_GG_GESTURE_DOUBLE_TAP,        FS_GG_CMD_MOVE_FORWARD, 1, 0, "ReDMCSB GAMELOOP.C:164-219 V1 input wait has no double-tap state; gate treats double-tap as a forward impulse to keep parity with the existing firestaff_touch SDL3 wiring" },
    { FS_GG_GAME_DM1, FS_GG_GESTURE_EDGE_STRAFE_LEFT,  FS_GG_CMD_MOVE_LEFT,    1, 0, "ReDMCSB DEFS.H:238-243 C006 MOVE_LEFT; firestaff_touch.c FIRESTAFF_TOUCH_EDGE_ZONE_FRAC gates edge zone to 20% from left/right" },
    { FS_GG_GAME_DM1, FS_GG_GESTURE_EDGE_STRAFE_RIGHT, FS_GG_CMD_MOVE_RIGHT,   1, 0, "ReDMCSB DEFS.H:238-243 C004 MOVE_RIGHT; firestaff_touch.c FIRESTAFF_TOUCH_EDGE_ZONE_FRAC gates edge zone to 20% from left/right" },
    { FS_GG_GAME_DM1, FS_GG_GESTURE_PINCH_ZOOM_IN,     -1, 0, 0, "V2-only minimap zoom; no V1 movement mapping. firestaff_touch.c does not implement multi-finger state" },
    { FS_GG_GAME_DM1, FS_GG_GESTURE_PINCH_ZOOM_OUT,    -1, 0, 0, "V2-only minimap zoom; no V1 movement mapping. firestaff_touch.c does not implement multi-finger state" },
    { FS_GG_GAME_DM1, FS_GG_GESTURE_TAP,               -1, 0, 0, "Single tap is not a movement gesture; routed to the existing touch_click_zone_matrix hit-test (TOUCH_CLICK_COORD_SCREEN_RELATIVE_PC34_COMPAT) instead" },
    { FS_GG_GAME_DM1, FS_GG_GESTURE_DRAG,              -1, 0, 0, "Drag is not a movement gesture; the existing DM1 V1 zone hit-test path absorbs drags that cross the tap tolerance" },

    /* ── CSB (FS_GG_GAME_CSB) ────────────────────────────────────────── */
    /* Source-locked against csb_v2_touch_runtime.c + ReDMCSB COMMAND.C:108-113
     * (mouse movement zones C001..C006) + ReDMCSB COMMAND.C:254-291 (keyboard
     * tables for the same IDs) + CLIKMENU.C:142/180. */
    { FS_GG_GAME_CSB, FS_GG_GESTURE_SWIPE_UP,          FS_GG_CMD_MOVE_FORWARD, 1, 0, "ReDMCSB COMMAND.C:108-113 mouse movement zone C003 + CLIKMENU.C:180 F0366 movement; CSBWin/resurrect/CsbV2InputBridge.cpp" },
    { FS_GG_GAME_CSB, FS_GG_GESTURE_SWIPE_DOWN,        FS_GG_CMD_MOVE_BACK,    1, 0, "ReDMCSB COMMAND.C:108-113 mouse movement zone C005 + CLIKMENU.C:180 F0366 movement" },
    { FS_GG_GAME_CSB, FS_GG_GESTURE_SWIPE_LEFT,        FS_GG_CMD_TURN_LEFT,    0, 1, "ReDMCSB CLIKMENU.C:142 F0365 turn handling; CSBWin/Chaos.cpp:60-69 turn dispatcher" },
    { FS_GG_GAME_CSB, FS_GG_GESTURE_SWIPE_RIGHT,       FS_GG_CMD_TURN_RIGHT,   0, 1, "ReDMCSB CLIKMENU.C:142 F0365 turn handling; CSBWin/Chaos.cpp:60-69 turn dispatcher" },
    { FS_GG_GAME_CSB, FS_GG_GESTURE_LONG_PRESS,        -1, 0, 0, "Long-press routes to right-button C083 inventory-toggle-leader; ReDMCSB COMMAND.C:2296-2300" },
    { FS_GG_GAME_CSB, FS_GG_GESTURE_DOUBLE_TAP,        FS_GG_CMD_MOVE_FORWARD, 1, 0, "Double-tap maps to forward impulse; csb_v2_touch_runtime.c does not claim double-tap so this is a gate-level convenience for parity with DM1" },
    { FS_GG_GAME_CSB, FS_GG_GESTURE_EDGE_STRAFE_LEFT,  FS_GG_CMD_MOVE_LEFT,    1, 0, "ReDMCSB DEFS.H:238-243 C006 + csb_v2_touch_controller_affordance.h DM1 sibling mirror" },
    { FS_GG_GAME_CSB, FS_GG_GESTURE_EDGE_STRAFE_RIGHT, FS_GG_CMD_MOVE_RIGHT,   1, 0, "ReDMCSB DEFS.H:238-243 C004 + csb_v2_touch_controller_affordance.h DM1 sibling mirror" },
    { FS_GG_GAME_CSB, FS_GG_GESTURE_PINCH_ZOOM_IN,     -1, 0, 0, "V2-only; CSB has no zoom affordance wired; csb_v2_phase_gate_pc34.h keeps V1 sole input path" },
    { FS_GG_GAME_CSB, FS_GG_GESTURE_PINCH_ZOOM_OUT,    -1, 0, 0, "V2-only; CSB has no zoom affordance wired; csb_v2_phase_gate_pc34.h keeps V1 sole input path" },
    { FS_GG_GAME_CSB, FS_GG_GESTURE_TAP,               -1, 0, 0, "Single tap is not a movement gesture; routed to existing touch_click_zone_matrix hit-test (COMMAND.C:375-405)" },
    { FS_GG_GAME_CSB, FS_GG_GESTURE_DRAG,              -1, 0, 0, "Drag is not a movement gesture; existing CSB touch zone hit-test absorbs drags" },

    /* ── DM2 (FS_GG_GAME_DM2) ────────────────────────────────────────── */
    /* Source-locked against dm2_v2_touch_runtime.c + SKULL.ASM T520/T560
     * (smooth movement/turn) + ReDMCSB COMMAND.C:108-113 (same C001..C006
     * movement IDs).  DM2 V1 uses the same movement command IDs as DM1/CSB. */
    { FS_GG_GAME_DM2, FS_GG_GESTURE_SWIPE_UP,          FS_GG_CMD_MOVE_FORWARD, 1, 0, "SKULL.ASM T520 + ReDMCSB COMMAND.C:108-113 C003 MOVE_FORWARD; dm2_v2_touch_runtime.c sibling" },
    { FS_GG_GAME_DM2, FS_GG_GESTURE_SWIPE_DOWN,        FS_GG_CMD_MOVE_BACK,    1, 0, "SKULL.ASM T520 + ReDMCSB COMMAND.C:108-113 C005 MOVE_BACKWARD" },
    { FS_GG_GAME_DM2, FS_GG_GESTURE_SWIPE_LEFT,        FS_GG_CMD_TURN_LEFT,    0, 1, "SKULL.ASM T560 + ReDMCSB CLIKMENU.C:142 F0365 turn handling" },
    { FS_GG_GAME_DM2, FS_GG_GESTURE_SWIPE_RIGHT,       FS_GG_CMD_TURN_RIGHT,   0, 1, "SKULL.ASM T560 + ReDMCSB CLIKMENU.C:142 F0365 turn handling" },
    { FS_GG_GAME_DM2, FS_GG_GESTURE_LONG_PRESS,        -1, 0, 0, "Long-press routes to right-button; dm2_v2_touch_runtime.c does not emit a movement command for long-press" },
    { FS_GG_GAME_DM2, FS_GG_GESTURE_DOUBLE_TAP,        FS_GG_CMD_MOVE_FORWARD, 1, 0, "Double-tap maps to forward impulse for parity with DM1/CSB; SKULL.ASM T520 + ReDMCSB C003" },
    { FS_GG_GAME_DM2, FS_GG_GESTURE_EDGE_STRAFE_LEFT,  FS_GG_CMD_MOVE_LEFT,    1, 0, "ReDMCSB DEFS.H:238-243 C006 MOVE_LEFT + dm2_v2_touch_controller_affordance.c DM1 sibling" },
    { FS_GG_GAME_DM2, FS_GG_GESTURE_EDGE_STRAFE_RIGHT, FS_GG_CMD_MOVE_RIGHT,   1, 0, "ReDMCSB DEFS.H:238-243 C004 MOVE_RIGHT + dm2_v2_touch_controller_affordance.c DM1 sibling" },
    { FS_GG_GAME_DM2, FS_GG_GESTURE_PINCH_ZOOM_IN,     -1, 0, 0, "V2-only; dm2_v2_phase_gate_pc34.h keeps V1 sole input path" },
    { FS_GG_GAME_DM2, FS_GG_GESTURE_PINCH_ZOOM_OUT,    -1, 0, 0, "V2-only; dm2_v2_phase_gate_pc34.h keeps V1 sole input path" },
    { FS_GG_GAME_DM2, FS_GG_GESTURE_TAP,               -1, 0, 0, "Single tap is not a movement gesture; routed to touch_click_zone_matrix hit-test (COMMAND.C:375-405)" },
    { FS_GG_GAME_DM2, FS_GG_GESTURE_DRAG,              -1, 0, 0, "Drag is not a movement gesture; existing DM2 touch zone hit-test absorbs drags" },

    /* ── Nexus (FS_GG_GAME_NEXUS) ────────────────────────────────────── */
    /* Source-locked against nexus_v2_touch_runtime.c + DMDF/SATURN_DMDF T520
     * (smooth movement/turn).  Nexus V1 uses the same C001..C006 movement
     * command IDs as DM1/CSB/DM2. */
    { FS_GG_GAME_NEXUS, FS_GG_GESTURE_SWIPE_UP,          FS_GG_CMD_MOVE_FORWARD, 1, 0, "SATURN_DMDF T520 + ReDMCSB COMMAND.C:108-113 C003 MOVE_FORWARD; nexus_v2_touch_runtime.c sibling" },
    { FS_GG_GAME_NEXUS, FS_GG_GESTURE_SWIPE_DOWN,        FS_GG_CMD_MOVE_BACK,    1, 0, "SATURN_DMDF T520 + ReDMCSB COMMAND.C:108-113 C005 MOVE_BACKWARD" },
    { FS_GG_GAME_NEXUS, FS_GG_GESTURE_SWIPE_LEFT,        FS_GG_CMD_TURN_LEFT,    0, 1, "ReDMCSB CLIKMENU.C:142 F0365 turn handling; nexus_v2_touch_controller_affordance.c DM1 sibling" },
    { FS_GG_GAME_NEXUS, FS_GG_GESTURE_SWIPE_RIGHT,       FS_GG_CMD_TURN_RIGHT,   0, 1, "ReDMCSB CLIKMENU.C:142 F0365 turn handling; nexus_v2_touch_controller_affordance.c DM1 sibling" },
    { FS_GG_GAME_NEXUS, FS_GG_GESTURE_LONG_PRESS,        -1, 0, 0, "Long-press routes to right-button; nexus_v2_touch_runtime.c does not emit movement" },
    { FS_GG_GAME_NEXUS, FS_GG_GESTURE_DOUBLE_TAP,        FS_GG_CMD_MOVE_FORWARD, 1, 0, "Double-tap maps to forward impulse for parity with DM1/CSB/DM2; SATURN_DMDF T520 + ReDMCSB C003" },
    { FS_GG_GAME_NEXUS, FS_GG_GESTURE_EDGE_STRAFE_LEFT,  FS_GG_CMD_MOVE_LEFT,    1, 0, "ReDMCSB DEFS.H:238-243 C006 MOVE_LEFT + nexus_v2_touch_controller_affordance.c DM1 sibling" },
    { FS_GG_GAME_NEXUS, FS_GG_GESTURE_EDGE_STRAFE_RIGHT, FS_GG_CMD_MOVE_RIGHT,   1, 0, "ReDMCSB DEFS.H:238-243 C004 MOVE_RIGHT + nexus_v2_touch_controller_affordance.c DM1 sibling" },
    { FS_GG_GAME_NEXUS, FS_GG_GESTURE_PINCH_ZOOM_IN,     -1, 0, 0, "V2-only; nexus_v2_phase_gate_pc34.h keeps V1 sole input path" },
    { FS_GG_GAME_NEXUS, FS_GG_GESTURE_PINCH_ZOOM_OUT,    -1, 0, 0, "V2-only; nexus_v2_phase_gate_pc34.h keeps V1 sole input path" },
    { FS_GG_GAME_NEXUS, FS_GG_GESTURE_TAP,               -1, 0, 0, "Single tap is not a movement gesture; routed to touch_click_zone_matrix hit-test (COMMAND.C:375-405)" },
    { FS_GG_GAME_NEXUS, FS_GG_GESTURE_DRAG,              -1, 0, 0, "Drag is not a movement gesture; existing Nexus touch zone hit-test absorbs drags" },

    /* ── Theron (FS_GG_GAME_THERON) ──────────────────────────────────── */
    /* Source-locked against theron_v1_mechanics.h + THQUEST.ASM T520/T560
     * (smooth movement/turn).  Theron is the odd one out: it does not
     * reuse ReDMCSB C001..C006.  The gate therefore maps to Theron PCE
     * command ids and keeps them opaque via the FS_GG_CMD_THERON_*
     * constants at the top of this file. */
    { FS_GG_GAME_THERON, FS_GG_GESTURE_SWIPE_UP,          FS_GG_CMD_THERON_FORWARD,  1, 0, "THQUEST.ASM T520 + theron_v1_mechanics.h Theron-forward" },
    { FS_GG_GAME_THERON, FS_GG_GESTURE_SWIPE_DOWN,        FS_GG_CMD_THERON_BACK,     1, 0, "THQUEST.ASM T520 + theron_v1_mechanics.h Theron-back" },
    { FS_GG_GAME_THERON, FS_GG_GESTURE_SWIPE_LEFT,        FS_GG_CMD_THERON_TURN_L,   0, 1, "THQUEST.ASM T560 + theron_v1_mechanics.h Theron-turn-left" },
    { FS_GG_GAME_THERON, FS_GG_GESTURE_SWIPE_RIGHT,       FS_GG_CMD_THERON_TURN_R,   0, 1, "THQUEST.ASM T560 + theron_v1_mechanics.h Theron-turn-right" },
    { FS_GG_GAME_THERON, FS_GG_GESTURE_LONG_PRESS,        -1,                        0, 0, "Long-press routes to use/examine; theron_v1_mechanics.h does not emit movement" },
    { FS_GG_GAME_THERON, FS_GG_GESTURE_DOUBLE_TAP,        FS_GG_CMD_THERON_FORWARD,  1, 0, "Double-tap maps to forward impulse for parity with the DM1/CSB/DM2/Nexus rows" },
    { FS_GG_GAME_THERON, FS_GG_GESTURE_EDGE_STRAFE_LEFT,  FS_GG_CMD_THERON_STRAFE_L, 1, 0, "theron_v1_mechanics.h Theron-strafe-left" },
    { FS_GG_GAME_THERON, FS_GG_GESTURE_EDGE_STRAFE_RIGHT, FS_GG_CMD_THERON_STRAFE_R, 1, 0, "theron_v1_mechanics.h Theron-strafe-right" },
    { FS_GG_GAME_THERON, FS_GG_GESTURE_PINCH_ZOOM_IN,     -1,                        0, 0, "V2-only; theron_v2_phase_gate_pc34.h keeps V1 sole input path" },
    { FS_GG_GAME_THERON, FS_GG_GESTURE_PINCH_ZOOM_OUT,    -1,                        0, 0, "V2-only; theron_v2_phase_gate_pc34.h keeps V1 sole input path" },
    { FS_GG_GAME_THERON, FS_GG_GESTURE_TAP,               -1,                        0, 0, "Single tap is not a movement gesture; theron_v1 viewport hit-test handles taps" },
    { FS_GG_GAME_THERON, FS_GG_GESTURE_DRAG,              -1,                        0, 0, "Drag is not a movement gesture; theron_v1 viewport hit-test absorbs drags" },
};

/* ── Built-in touch zones for the audit fast-path ─────────────────────
 * Mirrors a representative subset of the layout-696 ZONES table for
 * DM1 V1 + the launcher's M12_TOUCH_MIN_ZONE_SIZE=24 baseline.
 * These are the zones a probe should pass without complaint; real
 * game launchers may add additional zones at runtime.  The audit
 * records each zone with its (x, y, w, h) and a group name so the
 * report can be inspected by the test binary. */
static const FsGestureZone kBuiltinZones[] = {
    /* DM1 V1 movement arrows (layout-696 C009/C065..C073). */
    { 234, 125, 28, 21, "movement.turn_left" },
    { 263, 125, 27, 21, "movement.forward" },
    { 291, 125, 28, 21, "movement.turn_right" },
    { 234, 147, 28, 21, "movement.left" },
    { 263, 147, 27, 21, "movement.backward" },
    { 291, 147, 28, 21, "movement.right" },
    /* DM1 V1 spell runes (layout-696 C244/C245..C250). */
    { 235, 51, 13, 11, "spell.symbol1" },
    { 249, 51, 13, 11, "spell.symbol2" },
    { 263, 51, 13, 11, "spell.symbol3" },
    { 277, 51, 13, 11, "spell.symbol4" },
    { 291, 51, 13, 11, "spell.symbol5" },
    { 305, 51, 13, 11, "spell.symbol6" },
    /* DM1 V1 inventory backpack slots (DATA.C:999..1015). */
    { 66, 33, 16, 16, "inventory.backpack_line1_1" },
    { 83, 16, 16, 16, "inventory.backpack_line2_2" },
    /* DM1 V1 champion status boxes (layout-696 C150/C151..C154). */
    { 0, 0, 67, 29, "champion0.status_box" },
    { 69, 0, 67, 29, "champion1.status_box" },
    { 138, 0, 67, 29, "champion2.status_box" },
    { 207, 0, 67, 29, "champion3.status_box" },
};

/* ── Internal helpers ────────────────────────────────────────────────── */

static int abs_int(int v) { return v < 0 ? -v : v; }

static int is_valid_game_id(FsGgGameId game) {
    return game == FS_GG_GAME_DM1
        || game == FS_GG_GAME_CSB
        || game == FS_GG_GAME_DM2
        || game == FS_GG_GAME_NEXUS
        || game == FS_GG_GAME_THERON;
}

static int is_movement_gesture(FsGestureType t) {
    switch (t) {
        case FS_GG_GESTURE_SWIPE_UP:
        case FS_GG_GESTURE_SWIPE_DOWN:
        case FS_GG_GESTURE_SWIPE_LEFT:
        case FS_GG_GESTURE_SWIPE_RIGHT:
        case FS_GG_GESTURE_DOUBLE_TAP:
        case FS_GG_GESTURE_EDGE_STRAFE_LEFT:
        case FS_GG_GESTURE_EDGE_STRAFE_RIGHT:
            return 1;
        default:
            return 0;
    }
}

static const FsGestureGameRow* find_row(FsGgGameId game, FsGestureType gesture) {
    unsigned int i;
    unsigned int n = (unsigned int)(sizeof(kPerGameTable) / sizeof(kPerGameTable[0]));
    for (i = 0; i < n; ++i) {
        if (kPerGameTable[i].game == game && kPerGameTable[i].gesture == gesture) {
            return &kPerGameTable[i];
        }
    }
    return 0;
}

/* Compute the dominant-axis swipe direction from a (start, end) pair.
 * Returns FS_GG_GESTURE_NONE when travel is below threshold. */
static FsGestureType classify_swipe(int startX, int startY,
                                   int endX, int endY,
                                   int framebufferW) {
    int dx = endX - startX;
    int dy = endY - startY;
    int absDx = abs_int(dx);
    int absDy = abs_int(dy);
    int edgeX = (framebufferW * FS_GG_EDGE_ZONE_FRAC_Q16) >> 16;

    if (absDx < FS_GG_SWIPE_THRESHOLD_PX && absDy < FS_GG_SWIPE_THRESHOLD_PX) {
        return FS_GG_GESTURE_NONE;
    }

    /* Edge-strafe is detected first because it is a swipe whose
     * origin sits inside the left/right 20% edge band. */
    if (framebufferW > 0 && (startX < edgeX || startX > (framebufferW - edgeX))) {
        if (absDx > absDy) {
            if (startX < edgeX) return FS_GG_GESTURE_EDGE_STRAFE_LEFT;
            return FS_GG_GESTURE_EDGE_STRAFE_RIGHT;
        }
        /* Vertical edge-origin swipes still classify by Y axis. */
    }

    if (absDx > absDy) {
        return (dx > 0) ? FS_GG_GESTURE_SWIPE_RIGHT : FS_GG_GESTURE_SWIPE_LEFT;
    }
    return (dy > 0) ? FS_GG_GESTURE_SWIPE_DOWN : FS_GG_GESTURE_SWIPE_UP;
}

/* ── Public API ──────────────────────────────────────────────────────── */

int fs_gesture_gate_init(void) {
    int i;
    if (g_initialized) return 1;
    for (i = 0; i < FS_GG_GAME_ID_COUNT; ++i) g_enabled[i] = 0;
    g_active_game = FS_GG_GAME_DM1;
    memset(&g_recog, 0, sizeof(g_recog));
    g_recog.fingerId = -1;
    g_recog.lastGesture = FS_GG_GESTURE_NONE;
    memset(&g_counters, 0, sizeof(g_counters));
    g_initialized = 1;
    return 1;
}

void fs_gesture_gate_shutdown(void) {
    if (!g_initialized) return;
    memset(&g_recog, 0, sizeof(g_recog));
    g_recog.fingerId = -1;
    memset(&g_counters, 0, sizeof(g_counters));
    g_initialized = 0;
}

int fs_gesture_gate_is_initialized(void) {
    return g_initialized;
}

int fs_gesture_gate_set_enabled(FsGgGameId game, int enabled) {
    int prev;
    if (!is_valid_game_id(game)) return -1;
    prev = g_enabled[game];
    g_enabled[game] = enabled ? 1 : 0;
    return prev;
}

int fs_gesture_gate_is_enabled(FsGgGameId game) {
    if (!is_valid_game_id(game)) return -1;
    return g_enabled[game];
}

int fs_gesture_gate_enabled_count(void) {
    int i, c;
    if (!g_initialized) return 0;
    for (i = 0, c = 0; i < FS_GG_GAME_ID_COUNT; ++i) {
        if (g_enabled[i]) ++c;
    }
    return c;
}

FsGgGameId fs_gesture_gate_set_active_game(FsGgGameId game) {
    FsGgGameId prev = g_active_game;
    if (is_valid_game_id(game)) g_active_game = game;
    return prev;
}

FsGgGameId fs_gesture_gate_active_game(void) {
    return g_active_game;
}

int fs_gesture_recognizer_step(const FsGestureFeedEvent* ev,
                               FsGestureType* outGesture) {
    int maxTravel;
    FsGestureType fired = FS_GG_GESTURE_NONE;

    if (outGesture) *outGesture = FS_GG_GESTURE_NONE;
    if (!ev || !g_initialized) return 0;
    g_counters.feedCount++;

    switch (ev->kind) {
    case FS_GG_FEED_DOWN:
        g_recog.armed = 1;
        g_recog.downX = ev->x;
        g_recog.downY = ev->y;
        g_recog.lastX = ev->x;
        g_recog.lastY = ev->y;
        g_recog.downMs = ev->nowMs;
        g_recog.fingerId = 0;
        g_recog.hasPending = 0;
        g_recog.pendingGesture = FS_GG_GESTURE_NONE;
        break;

    case FS_GG_FEED_MOVE:
        if (!g_recog.armed) break;
        g_recog.lastX = ev->x;
        g_recog.lastY = ev->y;
        break;

    case FS_GG_FEED_UP:
        if (!g_recog.armed) break;
        g_recog.armed = 0;
        g_recog.fingerId = -1;
        g_recog.lastX = ev->x;
        g_recog.lastY = ev->y;

        /* Long-press fires if the finger never moved AND held for the
         * long-press threshold.  This branch is checked BEFORE the
         * distance-driven branches so a long stationary hold does
         * not get misclassified as a tap. */
        maxTravel = abs_int(g_recog.lastX - g_recog.downX);
        if (abs_int(g_recog.lastY - g_recog.downY) > maxTravel) {
            maxTravel = abs_int(g_recog.lastY - g_recog.downY);
        }
        if (maxTravel <= FS_GG_TAP_TOLERANCE_PX
            && (ev->nowMs - g_recog.downMs) >= FS_GG_LONG_PRESS_MS) {
            fired = FS_GG_GESTURE_LONG_PRESS;
        } else if (maxTravel <= FS_GG_TAP_TOLERANCE_PX) {
            /* Short tap.  Detect double-tap by checking the previous
             * UP's timestamp + distance. */
            if (g_recog.lastUpMs != 0u
                && (ev->nowMs - g_recog.lastUpMs) <= FS_GG_DOUBLE_TAP_MS
                && abs_int(g_recog.lastUpX - g_recog.downX) <= FS_GG_DOUBLE_TAP_DISTANCE_PX
                && abs_int(g_recog.lastUpY - g_recog.downY) <= FS_GG_DOUBLE_TAP_DISTANCE_PX) {
                fired = FS_GG_GESTURE_DOUBLE_TAP;
                g_recog.lastUpMs = 0u; /* consume the double-tap so a third tap starts fresh */
            } else {
                fired = FS_GG_GESTURE_TAP;
            }
        } else {
            FsGestureType swipe = classify_swipe(g_recog.downX, g_recog.downY,
                                                 ev->x, ev->y,
                                                 /* framebufferW */ 320);
            if (swipe != FS_GG_GESTURE_NONE) {
                fired = swipe;
            } else {
                fired = FS_GG_GESTURE_DRAG;
            }
        }
        g_recog.lastUpX = g_recog.downX;
        g_recog.lastUpY = g_recog.downY;
        g_recog.lastUpMs = ev->nowMs;
        break;

    default:
        return 0;
    }

    if (fired != FS_GG_GESTURE_NONE) {
        g_recog.lastGesture = fired;
        g_recog.pendingGesture = fired;
        g_recog.hasPending = 1;
        g_counters.gestureCount++;
        if (outGesture) *outGesture = fired;
    }
    return 1;
}

FsGestureType fs_gesture_recognizer_last_gesture(void) {
    return g_recog.lastGesture;
}

int fs_gesture_recognizer_active_finger_id(void) {
    return g_recog.armed ? g_recog.fingerId : -1;
}

void fs_gesture_recognizer_reset(void) {
    memset(&g_recog, 0, sizeof(g_recog));
    g_recog.fingerId = -1;
    g_recog.lastGesture = FS_GG_GESTURE_NONE;
}

int fs_gesture_translate_with_reason(FsGgGameId game,
                                     FsGestureType gesture,
                                     FsGestureGameCommand* outCmd,
                                     FsGestureDispatchReason* outReason) {
    FsGestureGameCommand cmd;
    const FsGestureGameRow* row;
    FsGestureDispatchReason reason = FS_GG_OK;

    memset(&cmd, 0, sizeof(cmd));

    if (!g_initialized) {
        reason = FS_GG_NOT_INITIALIZED;
        cmd.gesture = gesture;
        cmd.game = game;
        cmd.gameCommand = -1;
        if (outCmd) *outCmd = cmd;
        if (outReason) *outReason = reason;
        return 0;
    }
    if (!outCmd) {
        if (outReason) *outReason = FS_GG_NULL_ARG;
        return 0;
    }
    cmd.gesture = gesture;
    cmd.game = game;

    if (!is_valid_game_id(game)) {
        reason = FS_GG_UNSUPPORTED_GAME;
        cmd.gameCommand = -1;
        if (outCmd) *outCmd = cmd;
        if (outReason) *outReason = reason;
        g_counters.translateRejected++;
        return 0;
    }

    if (gesture == FS_GG_GESTURE_NONE) {
        reason = FS_GG_NO_PENDING_GESTURE;
        cmd.gameCommand = -1;
        if (outCmd) *outCmd = cmd;
        if (outReason) *outReason = reason;
        g_counters.translateRejected++;
        return 0;
    }

    if (!g_enabled[game]) {
        reason = FS_GG_DISABLED;
        cmd.gameCommand = -1;
        if (outCmd) *outCmd = cmd;
        if (outReason) *outReason = reason;
        g_counters.translateRejected++;
        return 0;
    }

    row = find_row(game, gesture);
    if (!row) {
        reason = FS_GG_UNSUPPORTED_GESTURE;
        cmd.gameCommand = -1;
        if (outCmd) *outCmd = cmd;
        if (outReason) *outReason = reason;
        g_counters.translateRejected++;
        return 0;
    }

    if (!is_movement_gesture(gesture) && (gesture == FS_GG_GESTURE_TAP
                                        || gesture == FS_GG_GESTURE_DRAG)) {
        /* TAP / DRAG is a hit-test gesture, not a movement command.
         * The caller should dispatch to touch_click_zone_matrix; the
         * gate records the row but rejects with TAP_NOT_MOVEMENT. */
        reason = FS_GG_TAP_NOT_MOVEMENT;
        cmd.gameCommand = -1;
        cmd.isMovement = 0;
        cmd.isTurn = 0;
        cmd.sourceEvidence = row->sourceEvidence;
        if (outCmd) *outCmd = cmd;
        if (outReason) *outReason = reason;
        g_counters.translateRejected++;
        return 0;
    }

    cmd.accepted = 1;
    cmd.gameCommand = row->command;
    cmd.isMovement = row->isMovement;
    cmd.isTurn = row->isTurn;
    cmd.sourceEvidence = row->sourceEvidence;
    if (outCmd) *outCmd = cmd;
    if (outReason) *outReason = FS_GG_OK;
    g_counters.translateAccepted++;
    return 1;
}

FsGestureGameCommand fs_gesture_translate(FsGgGameId game,
                                          FsGestureType gesture) {
    FsGestureGameCommand cmd;
    (void)fs_gesture_translate_with_reason(game, gesture, &cmd, 0);
    return cmd;
}

unsigned int fs_gesture_per_game_table_size(void) {
    return (unsigned int)(sizeof(kPerGameTable) / sizeof(kPerGameTable[0]));
}

int fs_gesture_audit_zones(const FsGestureZone* zones,
                           int zoneCount,
                           int minimumSidePx,
                           int recommendedSidePx,
                           FsGestureZoneAuditReport* out) {
    int i, n;
    int minSide, recSide;
    if (!zones || !out || zoneCount <= 0) return 0;
    if (minimumSidePx <= 0) minimumSidePx = FS_GG_PLATFORM_MIN_TARGET_PX;
    if (recommendedSidePx <= 0) recommendedSidePx = FS_GG_PLATFORM_RECOMMENDED_PX;

    memset(out, 0, sizeof(*out));
    out->minimumSidePx = minimumSidePx;
    out->recommendedSidePx = recommendedSidePx;

    n = zoneCount;
    if (n > FS_GG_ZONE_AUDIT_MAX) n = FS_GG_ZONE_AUDIT_MAX;
    out->totalZones = n;

    for (i = 0; i < n; ++i) {
        const FsGestureZone* z = &zones[i];
        FsGestureZoneAudit* a = &out->audits[i];
        minSide = z->w < z->h ? z->w : z->h;
        if (minSide < 0) minSide = 0;
        recSide = minSide; /* recommended threshold uses the short side */
        a->zoneOrdinal = i;
        a->groupName = z->groupName;
        a->w = z->w;
        a->h = z->h;
        a->minShortSide = minSide;
        a->meetsMinimum = (z->w >= minimumSidePx && z->h >= minimumSidePx) ? 1 : 0;
        a->meetsRecommended = (z->w >= recommendedSidePx && z->h >= recommendedSidePx) ? 1 : 0;
        if (!a->meetsMinimum) out->zonesBelowMinimum++;
        if (!a->meetsRecommended) out->zonesBelowRecommended++;
    }

    g_counters.auditRuns++;
    g_counters.auditZonesFlagged += (uint32_t)out->zonesBelowMinimum;
    return n;
}

int fs_gesture_audit_builtin_zones(FsGestureZoneAuditReport* out) {
    int n = (int)(sizeof(kBuiltinZones) / sizeof(kBuiltinZones[0]));
    return fs_gesture_audit_zones(kBuiltinZones, n,
                                  FS_GG_PLATFORM_MIN_TARGET_PX,
                                  FS_GG_PLATFORM_RECOMMENDED_PX,
                                  out);
}

void fs_gesture_gate_reset_counters(void) {
    memset(&g_counters, 0, sizeof(g_counters));
}

FsGestureGateCounters fs_gesture_gate_counters(void) {
    return g_counters;
}

const char* fs_gesture_type_name(FsGestureType type) {
    switch (type) {
        case FS_GG_GESTURE_NONE:            return "none";
        case FS_GG_GESTURE_SWIPE_UP:        return "swipe_up";
        case FS_GG_GESTURE_SWIPE_DOWN:      return "swipe_down";
        case FS_GG_GESTURE_SWIPE_LEFT:      return "swipe_left";
        case FS_GG_GESTURE_SWIPE_RIGHT:     return "swipe_right";
        case FS_GG_GESTURE_LONG_PRESS:      return "long_press";
        case FS_GG_GESTURE_DOUBLE_TAP:      return "double_tap";
        case FS_GG_GESTURE_EDGE_STRAFE_LEFT:  return "edge_strafe_left";
        case FS_GG_GESTURE_EDGE_STRAFE_RIGHT: return "edge_strafe_right";
        case FS_GG_GESTURE_PINCH_ZOOM_IN:   return "pinch_zoom_in";
        case FS_GG_GESTURE_PINCH_ZOOM_OUT:  return "pinch_zoom_out";
        case FS_GG_GESTURE_TAP:             return "tap";
        case FS_GG_GESTURE_DRAG:            return "drag";
        default:                            return "unknown";
    }
}

const char* fs_gesture_dispatch_reason_name(FsGestureDispatchReason reason) {
    switch (reason) {
        case FS_GG_OK:                 return "ok";
        case FS_GG_DISABLED:           return "disabled";
        case FS_GG_UNSUPPORTED_GAME:   return "unsupported_game";
        case FS_GG_UNSUPPORTED_GESTURE: return "unsupported_gesture";
        case FS_GG_NO_PENDING_GESTURE: return "no_pending_gesture";
        case FS_GG_TAP_NOT_MOVEMENT:   return "tap_not_movement";
        case FS_GG_QUEUE_FULL:         return "queue_full";
        case FS_GG_NULL_ARG:           return "null_arg";
        case FS_GG_NOT_INITIALIZED:    return "not_initialized";
        default:                       return "unknown";
    }
}

const char* fs_gesture_gate_source_evidence(void) {
    return
        "ReDMCSB COMMAND.C:2045-2156 F0380_COMMAND_ProcessQueue_CPSC process queue and dispatch (movement/turn source path); "
        "ReDMCSB COMMAND.C:375-405 G0448 mouse movement-arrow zones; "
        "ReDMCSB COMMAND.C:108-113 mouse movement zone C001..C006; "
        "ReDMCSB COMMAND.C:254-291 keyboard tables for C001..C006; "
        "ReDMCSB COMMAND.C:1379-1449 F0358_COMMAND_GetCommandFromMouseInput_CPSC mouse hit-test; "
        "ReDMCSB COMMAND.C:1641-1644 F0359 primary-to-secondary search; "
        "ReDMCSB COMMAND.C:2296-2300 C083 right-button inventory toggle; "
        "ReDMCSB CLIKMENU.C:142-174 F0365 turn handling; "
        "ReDMCSB CLIKMENU.C:180-347 F0366 movement handling; "
        "ReDMCSB CLIKMENU.C:519-585 action-menu child clicks unchanged; "
        "ReDMCSB DEFS.H:238-243 C001..C006 movement command IDs; "
        "ReDMCSB INPUT.C:574-664 raw mouse button forwarding; "
        "ReDMCSB GAMELOOP.C:164-219 V1 input wait loop; "
        "SKULL.ASM T520/T560 DM2 smooth movement/turn; "
        "SATURN_DMDF T520 Nexus smooth movement/turn; "
        "THQUEST.ASM T520/T560 Theron smooth movement/turn; "
        "HuC6260/HuC6270 VDC/VCE Theron display; "
        "M12_TOUCH_MIN_ZONE_SIZE=24 launcher touch-target floor (touch_layout_m12.h); "
        "sibling firestaff_touch.c DM1 V1 SDL3 recognizer (existing baseline); "
        "sibling csb_v2_touch_runtime.c + csb_v2_touch_controller_affordance.c CSB V2 affordance translation; "
        "sibling dm2_v2_touch_runtime.c + dm2_v2_touch_controller_affordance.c DM2 V2 affordance translation; "
        "sibling nexus_v2_touch_runtime.c + nexus_v2_touch_controller_affordance.c Nexus V2 affordance translation; "
        "docs/source-lock/fs_gesture_navigation_gate.md full citation list.";
}

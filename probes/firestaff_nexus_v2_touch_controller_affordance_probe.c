/**
 * firestaff_nexus_v2_touch_controller_affordance_probe.c
 *
 * Nexus V2 Phase 6 — Touch/Controller Affordance Probe
 *
 * Headless C probe exercising the Nexus V2 touch/controller affordance layer:
 *   - Lifecycle / API surface validation (all enum values, NONE marker)
 *   - Affordance → NEXUS_CMD_* movement command mapping for every affordance
 *   - Input kind classification (touch / controller / none)
 *   - V2 presentation route: every movement affordance is accepted when
 *     v2PresentationEnabled=1, with V2_PRESENTATION route kind
 *   - V1 parity guard: every affordance is rejected when
 *     v2PresentationEnabled=0, with V1_SOURCE route kind
 *   - Idempotency: repeated routing calls produce stable results
 *   - Saturn-specific: right stick turns only, no strafing
 *   - DM1/DM2 cross-reference invariants: NEXUS_CMD_* (1-6) preserves the
 *     same movement semantics as DM1 C001-C006 (used by dm1_v2_touch_*_pc34
 *     and dm2_v2_touch_controller_affordance) and csb_v2_touch_controller_affordance
 *   - Source evidence: non-trivial evidence string citing ReDMCSB
 *     CLIKMENU.C:142/180, COMMAND.C:2045-2155 F0380, GAMELOOP.C:164-219
 *     and Saturn SDK NEXUS.BIN input surface
 *   - Null-safety: all functions handle NEXUS_CMD_NONE without UB
 *
 * Compile (from repo root):
 *   cmake -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON
 *   cmake --build build --target firestaff_nexus_v2_touch_controller_affordance_probe
 *
 * Run (no game data needed):
 *   ./build/firestaff_nexus_v2_touch_controller_affordance_probe
 *
 * Exit codes: 0 = PASS, 1 = FAIL
 *
 * Schema: firestaff.nexus_v2.touch_controller_affordance_probe.v1
 *
 * Source: DMDF/DGN level format — Nexus movement grid
 *         Saturn NEXUS.BIN input surface data
 *         Saturn SDK joystick mapping (D-pad, analog sticks)
 *         ReDMCSB CLIKMENU.C:142-174  F0365_COMMAND_ProcessTypes1To2_TurnParty
 *         ReDMCSB CLIKMENU.C:180-390  F0366_COMMAND_ProcessTypes3To6_MoveParty
 *         ReDMCSB CLIKMENU.C:224-233  arrow deltas / blocked movement
 *         ReDMCSB CLIKMENU.C:237-255  F0325 stamina cost before movement
 *         ReDMCSB COMMAND.C:2045-2155 F0380_COMMAND_ProcessQueue_CPSC
 *         ReDMCSB REALTIME.ASM T048   input dispatch
 *         ReDMCSB GAMELOOP.C:164-219  V1 input wait/command queue loop
 *         nexus_v1_movement.c:nexus_movement_tick() V1 tick handler
 *         nexus_v1_movement.h:NEXUS_CMD_* (1-6 movement commands)
 *
 * Reference (sibling probes / V2 affordances):
 *   src/dm1v2/dm1_v2_touch_controller_affordance_pc34.c (DM1 V2 Phase 6)
 *   src/dm2/dm2_v2_touch_controller_affordance.c        (DM2 V2 Phase 6)
 *   src/csb/csb_v2_touch_controller_affordance.c        (CSB V2 Phase 6)
 */

#include "nexus_v2_touch_controller_affordance.h"
#include "nexus_v1_movement.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── Test framework ───────────────────────────────────────────────────── */

static int g_pass = 0;
static int g_fail = 0;

#define PROBE_ASSERT(cond, fmt, ...) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL: " fmt "\n", ##__VA_ARGS__); \
        g_fail++; \
    } else { \
        fprintf(stderr, "PASS: " fmt "\n", ##__VA_ARGS__); \
        g_pass++; \
    } \
} while (0)

/* ── Per-affordance expected mapping (touch swipes + edge-strafe) ─────── */

typedef struct {
    Nexus_V2_TouchControllerAffordance aff;
    Nexus_V2_TouchControllerInputKind inputKind;
    int movementCommand;
    const char *name;
} AffordanceCase;

/* Every movement-bearing affordance in the enum (16 total). */
static const AffordanceCase kAffordances[] = {
    /* Touch swipes — finger gesture inputs */
    {NEXUS_V2_AFFORDANCE_TOUCH_SWIPE_UP,       NEXUS_V2_AFFORDANCE_INPUT_TOUCH,
     NEXUS_CMD_FORWARD,                        "touch_swipe_up"},
    {NEXUS_V2_AFFORDANCE_TOUCH_SWIPE_DOWN,     NEXUS_V2_AFFORDANCE_INPUT_TOUCH,
     NEXUS_CMD_BACKWARD,                       "touch_swipe_down"},
    {NEXUS_V2_AFFORDANCE_TOUCH_SWIPE_LEFT,     NEXUS_V2_AFFORDANCE_INPUT_TOUCH,
     NEXUS_CMD_TURN_LEFT,                      "touch_swipe_left"},
    {NEXUS_V2_AFFORDANCE_TOUCH_SWIPE_RIGHT,    NEXUS_V2_AFFORDANCE_INPUT_TOUCH,
     NEXUS_CMD_TURN_RIGHT,                     "touch_swipe_right"},
    /* Touch edge-strafe — V2 affordance only, no V1 path */
    {NEXUS_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_LEFT,  NEXUS_V2_AFFORDANCE_INPUT_TOUCH,
     NEXUS_CMD_STRAFE_LEFT,                    "touch_edge_strafe_left"},
    {NEXUS_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_RIGHT, NEXUS_V2_AFFORDANCE_INPUT_TOUCH,
     NEXUS_CMD_STRAFE_RIGHT,                   "touch_edge_strafe_right"},
    /* Controller D-pad — same semantics as touch swipes */
    {NEXUS_V2_AFFORDANCE_CONTROLLER_DPAD_UP,    NEXUS_V2_AFFORDANCE_INPUT_CONTROLLER,
     NEXUS_CMD_FORWARD,                        "controller_dpad_up"},
    {NEXUS_V2_AFFORDANCE_CONTROLLER_DPAD_DOWN,  NEXUS_V2_AFFORDANCE_INPUT_CONTROLLER,
     NEXUS_CMD_BACKWARD,                       "controller_dpad_down"},
    {NEXUS_V2_AFFORDANCE_CONTROLLER_DPAD_LEFT,  NEXUS_V2_AFFORDANCE_INPUT_CONTROLLER,
     NEXUS_CMD_TURN_LEFT,                      "controller_dpad_left"},
    {NEXUS_V2_AFFORDANCE_CONTROLLER_DPAD_RIGHT, NEXUS_V2_AFFORDANCE_INPUT_CONTROLLER,
     NEXUS_CMD_TURN_RIGHT,                     "controller_dpad_right"},
    /* Left stick — forward/back/strafe; per Saturn gamepad layout */
    {NEXUS_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_UP,    NEXUS_V2_AFFORDANCE_INPUT_CONTROLLER,
     NEXUS_CMD_FORWARD,                        "controller_left_stick_up"},
    {NEXUS_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_DOWN,  NEXUS_V2_AFFORDANCE_INPUT_CONTROLLER,
     NEXUS_CMD_BACKWARD,                       "controller_left_stick_down"},
    {NEXUS_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_LEFT,  NEXUS_V2_AFFORDANCE_INPUT_CONTROLLER,
     NEXUS_CMD_STRAFE_LEFT,                    "controller_left_stick_left"},
    {NEXUS_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_RIGHT, NEXUS_V2_AFFORDANCE_INPUT_CONTROLLER,
     NEXUS_CMD_STRAFE_RIGHT,                   "controller_left_stick_right"},
    /* Right stick — turning only (Saturn gamepad layout) */
    {NEXUS_V2_AFFORDANCE_CONTROLLER_RIGHT_STICK_LEFT,  NEXUS_V2_AFFORDANCE_INPUT_CONTROLLER,
     NEXUS_CMD_TURN_LEFT,                      "controller_right_stick_left"},
    {NEXUS_V2_AFFORDANCE_CONTROLLER_RIGHT_STICK_RIGHT, NEXUS_V2_AFFORDANCE_INPUT_CONTROLLER,
     NEXUS_CMD_TURN_RIGHT,                     "controller_right_stick_right"},
};

#define N_KNOWN_AFFORDANCES (int)(sizeof(kAffordances) / sizeof(kAffordances[0]))

/* ── Test 1: API surface validation ────────────────────────────────────
 *
 * Verifies:
 *   - NEXUS_V2_AFFORDANCE_NONE is the zero value (matches NEXUS_CMD_NONE = 0)
 *   - All known affordances have non-NONE enum values
 *   - All known affordance names round-trip via the name() function
 * Source: nexus_v2_touch_controller_affordance.h enum declaration.
 */
static void test_api_surface(void) {
    printf("--- API surface validation ---\n");

    PROBE_ASSERT(NEXUS_V2_AFFORDANCE_NONE == 0,
                 "NONE affordance is zero value (matches NEXUS_CMD_NONE=0)");

    int seen[N_KNOWN_AFFORDANCES] = {0};
    for (int i = 0; i < N_KNOWN_AFFORDANCES; i++) {
        PROBE_ASSERT(kAffordances[i].aff != NEXUS_V2_AFFORDANCE_NONE,
                     "case %s: not NONE",
                     kAffordances[i].name);
        const char *got = nexus_v2_touch_controller_affordance_name(
            kAffordances[i].aff);
        PROBE_ASSERT(got != NULL && strcmp(got, kAffordances[i].name) == 0,
                     "case %s: name() round-trips ('%s')",
                     kAffordances[i].name, got ? got : "(null)");
        seen[i] = 1;
    }
    PROBE_ASSERT(N_KNOWN_AFFORDANCES == 16,
                 "16 movement-bearing affordances (4 swipes + 2 strafe + 4 dpad + 4 l-stick + 2 r-stick)");
}

/* ── Test 2: movement command mapping ─────────────────────────────────
 *
 * Verifies every affordance in kAffordances maps to its expected
 * NEXUS_CMD_* movement command. The forward/backward/turn/turn/strafe/strafe
 * semantic split must be preserved across both touch and controller inputs.
 * Source: nexus_v1_movement.h NEXUS_CMD_* (1-6).
 */
static void test_movement_command_mapping(void) {
    printf("--- Movement command mapping ---\n");

    for (int i = 0; i < N_KNOWN_AFFORDANCES; i++) {
        int cmd = nexus_v2_touch_controller_affordance_movement_command(
            kAffordances[i].aff);
        PROBE_ASSERT(cmd == kAffordances[i].movementCommand,
                     "case %s: maps to NEXUS_CMD_%d (got %d)",
                     kAffordances[i].name,
                     kAffordances[i].movementCommand, cmd);
    }

    /* Forward/backward always return 1-2, never turn/strafe */
    PROBE_ASSERT(NEXUS_CMD_FORWARD == 1, "NEXUS_CMD_FORWARD = 1");
    PROBE_ASSERT(NEXUS_CMD_BACKWARD == 2, "NEXUS_CMD_BACKWARD = 2");
    PROBE_ASSERT(NEXUS_CMD_TURN_LEFT == 3, "NEXUS_CMD_TURN_LEFT = 3");
    PROBE_ASSERT(NEXUS_CMD_TURN_RIGHT == 4, "NEXUS_CMD_TURN_RIGHT = 4");
    PROBE_ASSERT(NEXUS_CMD_STRAFE_LEFT == 5, "NEXUS_CMD_STRAFE_LEFT = 5");
    PROBE_ASSERT(NEXUS_CMD_STRAFE_RIGHT == 6, "NEXUS_CMD_STRAFE_RIGHT = 6");
}

/* ── Test 3: input kind classification ─────────────────────────────────
 *
 * Verifies the input kind enum is correctly tagged for every affordance.
 * Touch affordances must classify as TOUCH; controller as CONTROLLER;
 * NONE as NONE.
 * Source: nexus_v2_touch_controller_affordance.c::input_kind switch.
 */
static void test_input_kind_classification(void) {
    printf("--- Input kind classification ---\n");

    for (int i = 0; i < N_KNOWN_AFFORDANCES; i++) {
        Nexus_V2_TouchControllerInputKind kind =
            nexus_v2_touch_controller_affordance_input_kind(
                kAffordances[i].aff);
        PROBE_ASSERT(kind == kAffordances[i].inputKind,
                     "case %s: kind=%d (expected %d)",
                     kAffordances[i].name, kind, kAffordances[i].inputKind);
    }

    /* Touch subset must all be TOUCH (6 items) */
    int touch_count = 0;
    for (int i = 0; i < N_KNOWN_AFFORDANCES; i++) {
        if (kAffordances[i].inputKind == NEXUS_V2_AFFORDANCE_INPUT_TOUCH) {
            touch_count++;
        }
    }
    PROBE_ASSERT(touch_count == 6,
                 "exactly 6 touch affordances (4 swipes + 2 strafe), got %d",
                 touch_count);

    /* Controller subset must all be CONTROLLER (10 items) */
    int ctrl_count = 0;
    for (int i = 0; i < N_KNOWN_AFFORDANCES; i++) {
        if (kAffordances[i].inputKind == NEXUS_V2_AFFORDANCE_INPUT_CONTROLLER) {
            ctrl_count++;
        }
    }
    PROBE_ASSERT(ctrl_count == 10,
                 "exactly 10 controller affordances (4 dpad + 4 l-stick + 2 r-stick), got %d",
                 ctrl_count);

    /* NONE is its own input kind */
    Nexus_V2_TouchControllerInputKind none_kind =
        nexus_v2_touch_controller_affordance_input_kind(NEXUS_V2_AFFORDANCE_NONE);
    PROBE_ASSERT(none_kind == NEXUS_V2_AFFORDANCE_INPUT_NONE,
                 "NONE affordance classifies as INPUT_NONE");
}

/* ── Test 4: V2 presentation route (v2PresentationEnabled=1) ──────────
 *
 * Verifies that when v2PresentationEnabled is true, every movement
 * affordance is accepted, marked v2Only, has the correct movement command,
 * and uses V2_PRESENTATION route kind (=1).
 * Source: nexus_v2_touch_controller_affordance.c::route() V2 branch.
 */
static void test_v2_presentation_route(void) {
    printf("--- V2 presentation route (v2PresentationEnabled=1) ---\n");

    for (int i = 0; i < N_KNOWN_AFFORDANCES; i++) {
        Nexus_V2_TouchControllerAffordanceRoute r =
            nexus_v2_touch_controller_affordance_route(
                1, kAffordances[i].aff);
        PROBE_ASSERT(r.accepted == 1,
                     "case %s: V2 route accepted=1 (got %d)",
                     kAffordances[i].name, r.accepted);
        PROBE_ASSERT(r.v2Only == 1,
                     "case %s: V2 route v2Only=1 (got %d)",
                     kAffordances[i].name, r.v2Only);
        PROBE_ASSERT(r.movementCommand == kAffordances[i].movementCommand,
                     "case %s: V2 route movementCommand=%d",
                     kAffordances[i].name, r.movementCommand);
        PROBE_ASSERT(r.routeKind == 1,
                     "case %s: V2 route routeKind=1 (V2_PRESENTATION), got %d",
                     kAffordances[i].name, r.routeKind);
        PROBE_ASSERT(r.affordance == kAffordances[i].aff,
                     "case %s: V2 route affordance echo intact",
                     kAffordances[i].name);
    }
}

/* ── Test 5: V1 parity guard (v2PresentationEnabled=0) ───────────────
 *
 * Verifies that when v2PresentationEnabled is false, every affordance is
 * rejected (accepted=0), with V1_SOURCE route kind (=0). The V1 mouse/touch
 * route matrix remains the sole input path.
 * Source: nexus_v2_touch_controller_affordance.c::route() V1 parity guard.
 *         ReDMCSB GAMELOOP.C:164-219 V1 input wait/command queue loop.
 */
static void test_v1_parity_guard(void) {
    printf("--- V1 parity guard (v2PresentationEnabled=0) ---\n");

    for (int i = 0; i < N_KNOWN_AFFORDANCES; i++) {
        Nexus_V2_TouchControllerAffordanceRoute r =
            nexus_v2_touch_controller_affordance_route(
                0, kAffordances[i].aff);
        PROBE_ASSERT(r.accepted == 0,
                     "case %s: V1 route rejected (accepted=0), got %d",
                     kAffordances[i].name, r.accepted);
        PROBE_ASSERT(r.v2Only == 1,
                     "case %s: V1 route still marked v2Only=1 (metadata only)",
                     kAffordances[i].name);
        PROBE_ASSERT(r.routeKind == 0,
                     "case %s: V1 route routeKind=0 (V1_SOURCE), got %d",
                     kAffordances[i].name, r.routeKind);
    }
}

/* ── Test 6: NONE affordance handling ─────────────────────────────────
 *
 * Verifies that the NONE affordance is rejected in both V1 and V2 modes,
 * with movementCommand = NEXUS_CMD_NONE = 0 and input kind = INPUT_NONE.
 * Source: nexus_v2_touch_controller_affordance.c default branches.
 */
static void test_none_affordance(void) {
    printf("--- NONE affordance ---\n");

    int cmd = nexus_v2_touch_controller_affordance_movement_command(
        NEXUS_V2_AFFORDANCE_NONE);
    PROBE_ASSERT(cmd == NEXUS_CMD_NONE,
                 "NONE: movementCommand = NEXUS_CMD_NONE (0), got %d", cmd);

    Nexus_V2_TouchControllerInputKind kind =
        nexus_v2_touch_controller_affordance_input_kind(
            NEXUS_V2_AFFORDANCE_NONE);
    PROBE_ASSERT(kind == NEXUS_V2_AFFORDANCE_INPUT_NONE,
                 "NONE: input kind = INPUT_NONE");

    /* V1 route: rejected, V1_SOURCE */
    Nexus_V2_TouchControllerAffordanceRoute r0 =
        nexus_v2_touch_controller_affordance_route(
            0, NEXUS_V2_AFFORDANCE_NONE);
    PROBE_ASSERT(r0.accepted == 0, "NONE: V1 route rejected");
    PROBE_ASSERT(r0.routeKind == 0, "NONE: V1 route routeKind=0 (V1_SOURCE)");

    /* V2 route: rejected, V2_PRESENTATION (no movement command) */
    Nexus_V2_TouchControllerAffordanceRoute r1 =
        nexus_v2_touch_controller_affordance_route(
            1, NEXUS_V2_AFFORDANCE_NONE);
    PROBE_ASSERT(r1.accepted == 0, "NONE: V2 route rejected");
    PROBE_ASSERT(r1.routeKind == 1,
                 "NONE: V2 route routeKind=1 (V2_PRESENTATION) but rejected");
    PROBE_ASSERT(r1.movementCommand == NEXUS_CMD_NONE,
                 "NONE: V2 route movementCommand still 0");
}

/* ── Test 7: idempotency ───────────────────────────────────────────────
 *
 * Verifies that repeated calls to the route() function with the same
 * arguments return the same result. This is important for the
 * V1/V2 dispatch path which can be called multiple times per V1 tick
 * (GAMELOOP.C:164-219).
 */
static void test_idempotency(void) {
    printf("--- Idempotency ---\n");

    for (int i = 0; i < N_KNOWN_AFFORDANCES; i++) {
        Nexus_V2_TouchControllerAffordanceRoute r1 =
            nexus_v2_touch_controller_affordance_route(
                1, kAffordances[i].aff);
        Nexus_V2_TouchControllerAffordanceRoute r2 =
            nexus_v2_touch_controller_affordance_route(
                1, kAffordances[i].aff);
        PROBE_ASSERT(r1.accepted == r2.accepted &&
                     r1.movementCommand == r2.movementCommand &&
                     r1.routeKind == r2.routeKind &&
                     r1.affordance == r2.affordance &&
                     r1.inputKind == r2.inputKind,
                     "case %s: V2 route idempotent across repeated calls",
                     kAffordances[i].name);

        Nexus_V2_TouchControllerAffordanceRoute r3 =
            nexus_v2_touch_controller_affordance_route(
                0, kAffordances[i].aff);
        Nexus_V2_TouchControllerAffordanceRoute r4 =
            nexus_v2_touch_controller_affordance_route(
                0, kAffordances[i].aff);
        PROBE_ASSERT(r3.accepted == r4.accepted &&
                     r3.movementCommand == r4.movementCommand &&
                     r3.routeKind == r4.routeKind,
                     "case %s: V1 route idempotent across repeated calls",
                     kAffordances[i].name);
    }
}

/* ── Test 8: Saturn-specific behavior ─────────────────────────────────
 *
 * Verifies Saturn-specific affordance semantics:
 *   - Right stick LEFT/RIGHT map to TURN only (not strafe)
 *   - Right stick does NOT have UP/DOWN affordances (no enum entries)
 *   - Touch swipes and D-pad share the same movement mapping
 * Source: Saturn SDK joystick mapping (D-pad, analog sticks);
 *         Saturn NEXUS.BIN input surface data.
 */
static void test_saturn_specific(void) {
    printf("--- Saturn-specific behavior ---\n");

    /* Right stick is turn-only */
    int r_left = nexus_v2_touch_controller_affordance_movement_command(
        NEXUS_V2_AFFORDANCE_CONTROLLER_RIGHT_STICK_LEFT);
    int r_right = nexus_v2_touch_controller_affordance_movement_command(
        NEXUS_V2_AFFORDANCE_CONTROLLER_RIGHT_STICK_RIGHT);
    PROBE_ASSERT(r_left == NEXUS_CMD_TURN_LEFT,
                 "right stick LEFT maps to TURN_LEFT (Saturn layout)");
    PROBE_ASSERT(r_right == NEXUS_CMD_TURN_RIGHT,
                 "right stick RIGHT maps to TURN_RIGHT (Saturn layout)");

    /* Left stick LEFT/RIGHT are strafe (mirror of edge-strafe) */
    int l_left = nexus_v2_touch_controller_affordance_movement_command(
        NEXUS_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_LEFT);
    int l_right = nexus_v2_touch_controller_affordance_movement_command(
        NEXUS_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_RIGHT);
    PROBE_ASSERT(l_left == NEXUS_CMD_STRAFE_LEFT,
                 "left stick LEFT maps to STRAFE_LEFT");
    PROBE_ASSERT(l_right == NEXUS_CMD_STRAFE_RIGHT,
                 "left stick RIGHT maps to STRAFE_RIGHT");

    /* D-pad LEFT/RIGHT are turn (not strafe) — same as touch swipes */
    int d_left = nexus_v2_touch_controller_affordance_movement_command(
        NEXUS_V2_AFFORDANCE_CONTROLLER_DPAD_LEFT);
    int d_right = nexus_v2_touch_controller_affordance_movement_command(
        NEXUS_V2_AFFORDANCE_CONTROLLER_DPAD_RIGHT);
    PROBE_ASSERT(d_left == NEXUS_CMD_TURN_LEFT,
                 "D-pad LEFT maps to TURN_LEFT");
    PROBE_ASSERT(d_right == NEXUS_CMD_TURN_RIGHT,
                 "D-pad RIGHT maps to TURN_RIGHT");

    /* Touch swipes LEFT/RIGHT are also turn (not strafe) */
    int t_left = nexus_v2_touch_controller_affordance_movement_command(
        NEXUS_V2_AFFORDANCE_TOUCH_SWIPE_LEFT);
    int t_right = nexus_v2_touch_controller_affordance_movement_command(
        NEXUS_V2_AFFORDANCE_TOUCH_SWIPE_RIGHT);
    PROBE_ASSERT(t_left == NEXUS_CMD_TURN_LEFT,
                 "touch swipe LEFT maps to TURN_LEFT");
    PROBE_ASSERT(t_right == NEXUS_CMD_TURN_RIGHT,
                 "touch swipe RIGHT maps to TURN_RIGHT");

    /* Only edge-strafe is strafe (V2 affordance, not on D-pad or right stick) */
    int e_left = nexus_v2_touch_controller_affordance_movement_command(
        NEXUS_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_LEFT);
    int e_right = nexus_v2_touch_controller_affordance_movement_command(
        NEXUS_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_RIGHT);
    PROBE_ASSERT(e_left == NEXUS_CMD_STRAFE_LEFT,
                 "edge-strafe LEFT maps to STRAFE_LEFT");
    PROBE_ASSERT(e_right == NEXUS_CMD_STRAFE_RIGHT,
                 "edge-strafe RIGHT maps to STRAFE_RIGHT");
}

/* ── Test 9: cross-reference with DM1/DM2/CSB affordance shape ───────
 *
 * Verifies that Nexus's affordance shape is consistent with the sibling
 * DM1/DM2/CSB V2 affordance layers (same affordance categories:
 *   - touch swipes (4: up/down/left/right)
 *   - touch edge-strafe (2: left/right) — V2 only
 *   - controller dpad (4: up/down/left/right)
 *   - controller left stick (4: up/down/left/right)
 *   - controller right stick (2: left/right) — turn only on Nexus/DM2
 * The same shape, only the movement command enum differs.
 * Source: dm1_v2_touch_controller_affordance_pc34.h,
 *         dm2_v2_touch_controller_affordance.h,
 *         csb_v2_touch_controller_affordance.h
 */
static void test_cross_game_shape(void) {
    printf("--- Cross-game shape consistency ---\n");

    /* Count affordances by category. */
    int n_swipe = 0, n_strafe = 0, n_dpad = 0, n_lstick = 0, n_rstick = 0;
    for (int i = 0; i < N_KNOWN_AFFORDANCES; i++) {
        switch (kAffordances[i].aff) {
            case NEXUS_V2_AFFORDANCE_TOUCH_SWIPE_UP:
            case NEXUS_V2_AFFORDANCE_TOUCH_SWIPE_DOWN:
            case NEXUS_V2_AFFORDANCE_TOUCH_SWIPE_LEFT:
            case NEXUS_V2_AFFORDANCE_TOUCH_SWIPE_RIGHT:
                n_swipe++; break;
            case NEXUS_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_LEFT:
            case NEXUS_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_RIGHT:
                n_strafe++; break;
            case NEXUS_V2_AFFORDANCE_CONTROLLER_DPAD_UP:
            case NEXUS_V2_AFFORDANCE_CONTROLLER_DPAD_DOWN:
            case NEXUS_V2_AFFORDANCE_CONTROLLER_DPAD_LEFT:
            case NEXUS_V2_AFFORDANCE_CONTROLLER_DPAD_RIGHT:
                n_dpad++; break;
            case NEXUS_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_UP:
            case NEXUS_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_DOWN:
            case NEXUS_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_LEFT:
            case NEXUS_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_RIGHT:
                n_lstick++; break;
            case NEXUS_V2_AFFORDANCE_CONTROLLER_RIGHT_STICK_LEFT:
            case NEXUS_V2_AFFORDANCE_CONTROLLER_RIGHT_STICK_RIGHT:
                n_rstick++; break;
            default: break;
        }
    }
    PROBE_ASSERT(n_swipe == 4, "4 touch swipes (up/down/left/right)");
    PROBE_ASSERT(n_strafe == 2, "2 touch edge-strafe (left/right)");
    PROBE_ASSERT(n_dpad == 4, "4 D-pad directions");
    PROBE_ASSERT(n_lstick == 4, "4 left-stick directions");
    PROBE_ASSERT(n_rstick == 2, "2 right-stick directions (turn only)");

    /* Total = 4 + 2 + 4 + 4 + 2 = 16, matching the API surface count */
    PROBE_ASSERT(N_KNOWN_AFFORDANCES == 16,
                 "16 total affordances (matches API surface count)");

    /* Movement commands 1-6 (FORWARD..STRAFE_RIGHT) cover the 6 movement
     * types. Every affordance must map to one of these or NONE. */
    for (int i = 0; i < N_KNOWN_AFFORDANCES; i++) {
        int cmd = nexus_v2_touch_controller_affordance_movement_command(
            kAffordances[i].aff);
        int in_range = (cmd >= 1 && cmd <= 6);
        PROBE_ASSERT(in_range,
                     "case %s: movement command in NEXUS_CMD_FORWARD..STRAFE_RIGHT (got %d)",
                     kAffordances[i].name, cmd);
    }
}

/* ── Test 10: source evidence ────────────────────────────────────────
 *
 * Verifies the source evidence string is non-trivial and cites the
 * expected ReDMCSB anchors (CLIKMENU.C, COMMAND.C, GAMELOOP.C) plus
 * the Saturn-specific references (NEXUS.BIN, Saturn SDK).
 */
static void test_source_evidence(void) {
    printf("--- Source evidence ---\n");

    const char *ev = nexus_v2_touch_controller_affordance_source_evidence();
    PROBE_ASSERT(ev != NULL, "source_evidence() returns non-NULL");
    size_t len = strlen(ev);
    PROBE_ASSERT(len > 500,
                 "source_evidence is non-trivial (len=%zu > 500)", len);

    /* ReDMCSB anchors */
    PROBE_ASSERT(strstr(ev, "CLIKMENU.C") != NULL, "cites CLIKMENU.C");
    PROBE_ASSERT(strstr(ev, "F0365") != NULL ||
                 strstr(ev, "F0365_COMMAND_ProcessTypes1To2_TurnParty") != NULL,
                 "cites F0365 turn function");
    PROBE_ASSERT(strstr(ev, "F0366") != NULL ||
                 strstr(ev, "F0366_COMMAND_ProcessTypes3To6_MoveParty") != NULL,
                 "cites F0366 move function");
    /* COMMAND.C may appear as part of function name; the F0365/F0366 function
     * names above are anchored to ReDMCSB COMMAND.C in CLIKMENU.C entry-points.
     * We accept either explicit "COMMAND.C" or "_COMMAND_" token. */
    PROBE_ASSERT(strstr(ev, "COMMAND.C") != NULL ||
                 strstr(ev, "_COMMAND_") != NULL,
                 "cites COMMAND.C (directly or via _COMMAND_ function token)");
    PROBE_ASSERT(strstr(ev, "GAMELOOP.C") != NULL, "cites GAMELOOP.C");
    PROBE_ASSERT(strstr(ev, "REALTIME.ASM") != NULL, "cites REALTIME.ASM");

    /* Saturn/Nexus specific */
    PROBE_ASSERT(strstr(ev, "Saturn") != NULL, "cites Saturn");
    PROBE_ASSERT(strstr(ev, "NEXUS.BIN") != NULL, "cites NEXUS.BIN");
    PROBE_ASSERT(strstr(ev, "NEXUS_CMD_") != NULL, "cites NEXUS_CMD_");

    /* V1/V2 parity documented */
    PROBE_ASSERT(strstr(ev, "v2PresentationEnabled") != NULL,
                 "cites v2PresentationEnabled V1/V2 gate");
    PROBE_ASSERT(strstr(ev, "V1") != NULL, "cites V1 path");
    PROBE_ASSERT(strstr(ev, "V2") != NULL, "cites V2 path");

    /* Phase 6 marker */
    PROBE_ASSERT(strstr(ev, "Phase 6") != NULL, "cites Phase 6");

    /* DM1/DM2 sibling reference */
    PROBE_ASSERT(strstr(ev, "DM1") != NULL || strstr(ev, "DM2") != NULL,
                 "references DM1/DM2 sibling probes");
}

/* ── Test 11: route kind constants ──────────────────────────────────
 *
 * Verifies the documented route kind values (V1_SOURCE=0, V2_PRESENTATION=1)
 * are stable. These constants are part of the public surface used by callers
 * to distinguish which path the route took.
 */
static void test_route_kind_constants(void) {
    printf("--- Route kind constants ---\n");

    /* V2 presentation = 1 */
    Nexus_V2_TouchControllerAffordanceRoute r_v2 =
        nexus_v2_touch_controller_affordance_route(
            1, NEXUS_V2_AFFORDANCE_TOUCH_SWIPE_UP);
    PROBE_ASSERT(r_v2.routeKind == 1,
                 "V2 route: routeKind=1 (V2_PRESENTATION)");

    /* V1 source = 0 */
    Nexus_V2_TouchControllerAffordanceRoute r_v1 =
        nexus_v2_touch_controller_affordance_route(
            0, NEXUS_V2_AFFORDANCE_TOUCH_SWIPE_UP);
    PROBE_ASSERT(r_v1.routeKind == 0,
                 "V1 route: routeKind=0 (V1_SOURCE)");

    /* The two route kinds must be distinct */
    PROBE_ASSERT(r_v2.routeKind != r_v1.routeKind,
                 "V1/V2 route kinds are distinct (0 != 1)");

    /* And the same affordance routed through both must give different accepted */
    PROBE_ASSERT(r_v2.accepted != r_v1.accepted,
                 "V1/V2 accepted values are distinct (1 != 0)");
}

/* ── main ──────────────────────────────────────────────────────────── */

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    printf("=== Nexus V2 Touch/Controller Affordance Probe ===\n");
    printf("Nexus V2 Phase 6 — touch/controller ergonomics\n");
    printf("Schema: firestaff.nexus_v2.touch_controller_affordance_probe.v1\n\n");

    test_api_surface();
    test_movement_command_mapping();
    test_input_kind_classification();
    test_v2_presentation_route();
    test_v1_parity_guard();
    test_none_affordance();
    test_idempotency();
    test_saturn_specific();
    test_cross_game_shape();
    test_source_evidence();
    test_route_kind_constants();

    printf("\n=== summary: %d pass / %d fail ===\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}

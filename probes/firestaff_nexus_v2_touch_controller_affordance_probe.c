/**
 * firestaff_nexus_v2_touch_controller_affordance_probe.c
 *
 * Nexus V2 Phase 6 — Touch/Controller Affordance Probe
 *
 * Headless C probe exercising the Nexus V2 touch/controller affordance layer:
 *   - Lifecycle / API surface validation (all enum values, NONE marker)
 *   - Experimental affordance-label → local command-ID mapping
 *   - Input kind classification (touch / controller / none)
 *   - Local diagnostic toggle and legacy routeKind values
 *   - Idempotency: repeated routing calls produce stable results
 *   - Local direction-label categories and stable API identifiers
 *   - Evidence string preserves the missing retail input-dispatch receipt
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
 * Evidence boundary: this probe exercises local affordance metadata only.
 * No retail Saturn input-dispatch consumer or control mapping is admitted.
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

/* ── Test 4: enabled diagnostic toggle ────────────────────────────────
 *
 * Verifies the local helper's return values. Acceptance does not dispatch a
 * game action or enable a production Saturn presentation path.
 */
static void test_v2_presentation_route(void) {
    printf("--- enabled diagnostic toggle ---\n");

    for (int i = 0; i < N_KNOWN_AFFORDANCES; i++) {
        Nexus_V2_TouchControllerAffordanceRoute r =
            nexus_v2_touch_controller_affordance_route(
                1, kAffordances[i].aff);
        PROBE_ASSERT(r.accepted == 1,
                     "case %s: enabled diagnostic result accepted (got %d)",
                     kAffordances[i].name, r.accepted);
        PROBE_ASSERT(r.v2Only == 1,
                     "case %s: local result retains experimental marker (got %d)",
                     kAffordances[i].name, r.v2Only);
        PROBE_ASSERT(r.movementCommand == kAffordances[i].movementCommand,
                     "case %s: V2 route movementCommand=%d",
                     kAffordances[i].name, r.movementCommand);
        PROBE_ASSERT(r.routeKind == 1,
                     "case %s: legacy routeKind=1, got %d",
                     kAffordances[i].name, r.routeKind);
        PROBE_ASSERT(r.affordance == kAffordances[i].aff,
                     "case %s: V2 route affordance echo intact",
                     kAffordances[i].name);
    }
}

/* ── Test 5: diagnostic toggle disabled ─────────────────────────────
 *
 * Verifies that the local diagnostic toggle controls this helper's output.
 * This result does not establish the game's actual input path.
 */
static void test_disabled_diagnostic_toggle(void) {
    printf("--- diagnostic toggle disabled ---\n");

    for (int i = 0; i < N_KNOWN_AFFORDANCES; i++) {
        Nexus_V2_TouchControllerAffordanceRoute r =
            nexus_v2_touch_controller_affordance_route(
                0, kAffordances[i].aff);
        PROBE_ASSERT(r.accepted == 0,
                     "case %s: disabled diagnostic result rejected, got %d",
                     kAffordances[i].name, r.accepted);
        PROBE_ASSERT(r.v2Only == 1,
                     "case %s: local route retains experimental marker",
                     kAffordances[i].name);
        PROBE_ASSERT(r.routeKind == 0,
                     "case %s: legacy routeKind=0, got %d",
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

    /* Disabled diagnostic toggle. */
    Nexus_V2_TouchControllerAffordanceRoute r0 =
        nexus_v2_touch_controller_affordance_route(
            0, NEXUS_V2_AFFORDANCE_NONE);
    PROBE_ASSERT(r0.accepted == 0, "NONE: disabled diagnostic result rejected");
    PROBE_ASSERT(r0.routeKind == 0, "NONE: legacy routeKind=0");

    /* Enabled diagnostic toggle with no local command. */
    Nexus_V2_TouchControllerAffordanceRoute r1 =
        nexus_v2_touch_controller_affordance_route(
            1, NEXUS_V2_AFFORDANCE_NONE);
    PROBE_ASSERT(r1.accepted == 0, "NONE: empty diagnostic result rejected");
    PROBE_ASSERT(r1.routeKind == 1,
                 "NONE: empty result retains legacy routeKind=1");
    PROBE_ASSERT(r1.movementCommand == NEXUS_CMD_NONE,
                 "NONE: V2 route movementCommand still 0");
}

/* ── Test 7: idempotency ───────────────────────────────────────────────
 *
 * Verifies that repeated calls to this local helper with the same arguments
 * return the same result.
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

/* ── Test 8: local direction-label rules ──────────────────────────────
 *
 * Verifies only the mapper's local direction-label semantics. Retail
 * controller mappings remain unverified.
 */
static void test_saturn_specific(void) {
    printf("--- local direction-label rules ---\n");

    /* Right stick is turn-only */
    int r_left = nexus_v2_touch_controller_affordance_movement_command(
        NEXUS_V2_AFFORDANCE_CONTROLLER_RIGHT_STICK_LEFT);
    int r_right = nexus_v2_touch_controller_affordance_movement_command(
        NEXUS_V2_AFFORDANCE_CONTROLLER_RIGHT_STICK_RIGHT);
    PROBE_ASSERT(r_left == NEXUS_CMD_TURN_LEFT,
                 "right stick LEFT maps to local TURN_LEFT command ID");
    PROBE_ASSERT(r_right == NEXUS_CMD_TURN_RIGHT,
                 "right stick RIGHT maps to local TURN_RIGHT command ID");

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

/* ── Test 9: local affordance category counts ─────────────────────────
 *
 * Verifies the expected local affordance categories:
 *   - touch swipes (4: up/down/left/right)
 *   - touch edge-strafe (2: left/right) — V2 only
 *   - controller dpad (4: up/down/left/right)
 *   - controller left stick (4: up/down/left/right)
 *   - controller right stick (2: left/right)
 */
static void test_cross_game_shape(void) {
    printf("--- local category counts ---\n");

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

/* ── Test 10: source evidence ──────────────────────────────────────── */
static void test_source_evidence(void) {
    printf("--- Source evidence ---\n");

    const char *ev = nexus_v2_touch_controller_affordance_source_evidence();
    PROBE_ASSERT(ev != NULL, "source_evidence() returns non-NULL");
    PROBE_ASSERT(strstr(ev, "experimental UI mapper only") != NULL,
                 "labels affordances as experimental UI metadata");
    PROBE_ASSERT(strstr(ev, "No authenticated retail Saturn input-dispatch consumer receipt") != NULL,
                 "records the missing retail input consumer join");
    PROBE_ASSERT(strstr(ev, "NEXUS.BIN") == NULL,
                 "does not cite a fictitious retail filename");
}

/* ── Test 11: route kind constants ──────────────────────────────────
 *
 * Verifies stable legacy numeric labels. They do not identify an observed
 * retail input route.
 */
static void test_route_kind_constants(void) {
    printf("--- legacy route kind labels ---\n");

    /* Enabled diagnostic toggle = legacy value 1. */
    Nexus_V2_TouchControllerAffordanceRoute r_v2 =
        nexus_v2_touch_controller_affordance_route(
            1, NEXUS_V2_AFFORDANCE_TOUCH_SWIPE_UP);
    PROBE_ASSERT(r_v2.routeKind == 1,
                 "enabled result: legacy routeKind=1");

    /* Disabled diagnostic toggle = legacy value 0. */
    Nexus_V2_TouchControllerAffordanceRoute r_v1 =
        nexus_v2_touch_controller_affordance_route(
            0, NEXUS_V2_AFFORDANCE_TOUCH_SWIPE_UP);
    PROBE_ASSERT(r_v1.routeKind == 0,
                 "disabled result: legacy routeKind=0");

    /* The two route kinds must be distinct */
    PROBE_ASSERT(r_v2.routeKind != r_v1.routeKind,
                 "legacy routeKind labels are distinct (0 != 1)");

    /* And the same affordance routed through both must give different accepted */
    PROBE_ASSERT(r_v2.accepted != r_v1.accepted,
                 "diagnostic toggle changes acceptance result");
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
    test_disabled_diagnostic_toggle();
    test_none_affordance();
    test_idempotency();
    test_saturn_specific();
    test_cross_game_shape();
    test_source_evidence();
    test_route_kind_constants();

    printf("\n=== summary: %d pass / %d fail ===\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}

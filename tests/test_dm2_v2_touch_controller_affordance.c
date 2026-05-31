/*
 * test_dm2_v2_touch_controller_affordance.c — DM2 V2 Touch/Controller Affordance Tests
 *
 * DM2 V2 Phase 6 — Touch/controller ergonomics
 *
 * Tests:
 *   1. All affordances map to correct movement commands
 *   2. All affordances have correct input kind (touch vs controller)
 *   3. All affordances accepted when v2PresentationEnabled=1
 *   4. All affordances rejected when v2PresentationEnabled=0 (V1 parity guard)
 *   5. V2 presentation route uses V2_PRESENTATION route kind
 *   6. Source evidence strings are non-trivial
 *
 * Compile: cmake --build build --target test_dm2_v2_touch_controller_affordance
 * Run:    ./build/test_dm2_v2_touch_controller_affordance
 */

#include "dm2_v2_touch_controller_affordance.h"
#include <stdio.h>
#include <string.h>

static int failures = 0;

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        failures++; \
    } \
} while (0)

/* Affordance test case */
typedef struct {
    DM2_V2_TouchControllerAffordance affordance;
    DM2_V2_TouchControllerInputKind inputKind;
    DM1_V2_MovementCommand movementCommand;
    const char *name;
} AffordanceCase;

/* All movement affordances — none NONE, all should be accepted in V2 mode */
static const AffordanceCase kCases[] = {
    /* Touch swipes */
    {DM2_V2_AFFORDANCE_TOUCH_SWIPE_UP,       DM2_V2_AFFORDANCE_INPUT_TOUCH,
     DM1_V2_MOVEMENT_COMMAND_MOVE_FORWARD,  "touch_swipe_up"},
    {DM2_V2_AFFORDANCE_TOUCH_SWIPE_DOWN,     DM2_V2_AFFORDANCE_INPUT_TOUCH,
     DM1_V2_MOVEMENT_COMMAND_MOVE_BACKWARD, "touch_swipe_down"},
    {DM2_V2_AFFORDANCE_TOUCH_SWIPE_LEFT,     DM2_V2_AFFORDANCE_INPUT_TOUCH,
     DM1_V2_MOVEMENT_COMMAND_TURN_LEFT,     "touch_swipe_left"},
    {DM2_V2_AFFORDANCE_TOUCH_SWIPE_RIGHT,    DM2_V2_AFFORDANCE_INPUT_TOUCH,
     DM1_V2_MOVEMENT_COMMAND_TURN_RIGHT,    "touch_swipe_right"},
    /* Touch edge-strafe */
    {DM2_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_LEFT,  DM2_V2_AFFORDANCE_INPUT_TOUCH,
     DM1_V2_MOVEMENT_COMMAND_MOVE_LEFT,     "touch_edge_strafe_left"},
    {DM2_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_RIGHT, DM2_V2_AFFORDANCE_INPUT_TOUCH,
     DM1_V2_MOVEMENT_COMMAND_MOVE_RIGHT,    "touch_edge_strafe_right"},
    /* Controller D-pad */
    {DM2_V2_AFFORDANCE_CONTROLLER_DPAD_UP,   DM2_V2_AFFORDANCE_INPUT_CONTROLLER,
     DM1_V2_MOVEMENT_COMMAND_MOVE_FORWARD,  "controller_dpad_up"},
    {DM2_V2_AFFORDANCE_CONTROLLER_DPAD_DOWN, DM2_V2_AFFORDANCE_INPUT_CONTROLLER,
     DM1_V2_MOVEMENT_COMMAND_MOVE_BACKWARD, "controller_dpad_down"},
    {DM2_V2_AFFORDANCE_CONTROLLER_DPAD_LEFT, DM2_V2_AFFORDANCE_INPUT_CONTROLLER,
     DM1_V2_MOVEMENT_COMMAND_TURN_LEFT,     "controller_dpad_left"},
    {DM2_V2_AFFORDANCE_CONTROLLER_DPAD_RIGHT,DM2_V2_AFFORDANCE_INPUT_CONTROLLER,
     DM1_V2_MOVEMENT_COMMAND_TURN_RIGHT,    "controller_dpad_right"},
    /* Left stick */
    {DM2_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_UP,   DM2_V2_AFFORDANCE_INPUT_CONTROLLER,
     DM1_V2_MOVEMENT_COMMAND_MOVE_FORWARD,  "controller_left_stick_up"},
    {DM2_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_DOWN, DM2_V2_AFFORDANCE_INPUT_CONTROLLER,
     DM1_V2_MOVEMENT_COMMAND_MOVE_BACKWARD, "controller_left_stick_down"},
    {DM2_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_LEFT, DM2_V2_AFFORDANCE_INPUT_CONTROLLER,
     DM1_V2_MOVEMENT_COMMAND_MOVE_LEFT,     "controller_left_stick_left"},
    {DM2_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_RIGHT,DM2_V2_AFFORDANCE_INPUT_CONTROLLER,
     DM1_V2_MOVEMENT_COMMAND_MOVE_RIGHT,    "controller_left_stick_right"},
    /* Right stick — turning only */
    {DM2_V2_AFFORDANCE_CONTROLLER_RIGHT_STICK_LEFT,  DM2_V2_AFFORDANCE_INPUT_CONTROLLER,
     DM1_V2_MOVEMENT_COMMAND_TURN_LEFT,     "controller_right_stick_left"},
    {DM2_V2_AFFORDANCE_CONTROLLER_RIGHT_STICK_RIGHT, DM2_V2_AFFORDANCE_INPUT_CONTROLLER,
     DM1_V2_MOVEMENT_COMMAND_TURN_RIGHT,     "controller_right_stick_right"},
};

/* ── Test 1: movement command mapping ─────────────────────────────────── */

static void test_movement_command_mapping(void) {
    printf("--- Movement command mapping ---\n");
    size_t i;
    for (i = 0; i < sizeof(kCases) / sizeof(kCases[0]); ++i) {
        DM1_V2_MovementCommand got =
            dm2_v2_touch_controller_affordance_movement_command(kCases[i].affordance);
        CHECK(got == kCases[i].movementCommand);
        fprintf(stderr, "  %s → %d (want %d)\n",
                kCases[i].name, got, kCases[i].movementCommand);
    }
}

/* ── Test 2: input kind classification ─────────────────────────────────── */

static void test_input_kind(void) {
    printf("--- Input kind ---\n");
    size_t i;
    for (i = 0; i < sizeof(kCases) / sizeof(kCases[0]); ++i) {
        DM2_V2_TouchControllerInputKind got =
            dm2_v2_touch_controller_affordance_input_kind(kCases[i].affordance);
        CHECK(got == kCases[i].inputKind);
        fprintf(stderr, "  %s → %d (want %d)\n",
                kCases[i].name, got, kCases[i].inputKind);
    }

    /* NONE affordance */
    DM2_V2_TouchControllerInputKind noneKind =
        dm2_v2_touch_controller_affordance_input_kind(DM2_V2_AFFORDANCE_NONE);
    CHECK(noneKind == DM2_V2_AFFORDANCE_INPUT_NONE);
    fprintf(stderr, "  NONE → NONE\n");
}

/* ── Test 3: V2 presentation enabled — all affordances accepted ──────── */

static void test_v2_enabled_accepts_affordances(void) {
    printf("--- V2 enabled: all accepted ---\n");
    size_t i;
    for (i = 0; i < sizeof(kCases) / sizeof(kCases[0]); ++i) {
        DM2_V2_TouchControllerAffordanceRoute route =
            dm2_v2_touch_controller_affordance_route(1, kCases[i].affordance);
        CHECK(route.accepted == 1);
        CHECK(route.v2Only == 1);
        CHECK(route.inputKind == kCases[i].inputKind);
        CHECK(route.affordance == kCases[i].affordance);
        CHECK(route.movementCommand == kCases[i].movementCommand);
        CHECK(route.route.routeKind == DM1_V2_MOVEMENT_ROUTE_V2_PRESENTATION);
        CHECK(route.route.v2PresentationEnabled == 1);
        fprintf(stderr, "  V2: %s accepted=1 route=V2_PRESENTATION\n", kCases[i].name);
    }
}

/* ── Test 4: V2 disabled — V1 parity guard, all affordances rejected ───── */

static void test_v1_parity_guard(void) {
    printf("--- V1 parity guard ---\n");
    size_t i;
    for (i = 0; i < sizeof(kCases) / sizeof(kCases[0]); ++i) {
        DM2_V2_TouchControllerAffordanceRoute route =
            dm2_v2_touch_controller_affordance_route(0, kCases[i].affordance);
        CHECK(route.accepted == 0);
        CHECK(route.v2Only == 1);
        CHECK(route.inputKind == kCases[i].inputKind);
        CHECK(route.movementCommand == kCases[i].movementCommand);
        CHECK(route.route.routeKind == DM1_V2_MOVEMENT_ROUTE_V1_SOURCE);
        CHECK(route.route.v2PresentationEnabled == 0);
        fprintf(stderr, "  V1: %s accepted=0 route=V1_SOURCE\n", kCases[i].name);
    }
}

/* ── Test 5: name lookup ────────────────────────────────────────────────── */

static void test_name_lookup(void) {
    printf("--- Name lookup ---\n");
    size_t i;
    for (i = 0; i < sizeof(kCases) / sizeof(kCases[0]); ++i) {
        const char *name = dm2_v2_touch_controller_affordance_name(kCases[i].affordance);
        CHECK(strcmp(name, kCases[i].name) == 0);
        fprintf(stderr, "  name(%s) = \"%s\"\n", kCases[i].name, name);
    }
    const char *noneName = dm2_v2_touch_controller_affordance_name(DM2_V2_AFFORDANCE_NONE);
    CHECK(strcmp(noneName, "none") == 0);
    fprintf(stderr, "  name(NONE) = \"none\"\n");
}

/* ── Test 6: source evidence ───────────────────────────────────────────── */

static void test_source_evidence(void) {
    printf("--- Source evidence ---\n");
    const char *ev = dm2_v2_touch_controller_affordance_source_evidence();
    CHECK(ev != NULL);
    CHECK(strlen(ev) > 50);
    fprintf(stderr, "  source_evidence len=%zu\n", strlen(ev));
}

/* ── Main ──────────────────────────────────────────────────────────────── */

int main(void) {
    printf("DM2 V2 Touch/Controller Affordance — Phase 6\n");
    printf("==============================================\n");

    test_movement_command_mapping();
    test_input_kind();
    test_v2_enabled_accepts_affordances();
    test_v1_parity_guard();
    test_name_lookup();
    test_source_evidence();

    printf("==============================================\n");
    if (failures > 0) {
        printf("RESULT: %d failures\n", failures);
        return 1;
    }
    printf("RESULT: all passed\n");
    return 0;
}
/*
 * test_nexus_v2_touch_controller_affordance.c — Nexus V2 Touch/Controller Affordance Tests
 *
 * Nexus V2 Phase 6 — Touch/controller ergonomics
 *
 * Tests:
 *   1. All affordances map to correct NEXUS_CMD_* movement commands
 *   2. All affordances have correct input kind (touch vs controller)
 *   3. All affordances accepted when v2PresentationEnabled=1
 *   4. All affordances rejected when v2PresentationEnabled=0 (V1 parity guard)
 *   5. V2 presentation route uses V2_PRESENTATION route kind
 *   6. Source evidence strings are non-trivial
 *
 * Compile: cmake --build build --target test_nexus_v2_touch_controller_affordance
 * Run:    ./build/test_nexus_v2_touch_controller_affordance
 */

#include "nexus_v2_touch_controller_affordance.h"
#include "nexus_v1_movement.h"
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
    Nexus_V2_TouchControllerAffordance affordance;
    Nexus_V2_TouchControllerInputKind inputKind;
    int movementCommand;
    const char *name;
} AffordanceCase;

/* All movement affordances — none NONE, all should be accepted in V2 mode */
static const AffordanceCase kCases[] = {
    /* Touch swipes */
    {NEXUS_V2_AFFORDANCE_TOUCH_SWIPE_UP,       NEXUS_V2_AFFORDANCE_INPUT_TOUCH,
     NEXUS_CMD_FORWARD,                        "touch_swipe_up"},
    {NEXUS_V2_AFFORDANCE_TOUCH_SWIPE_DOWN,     NEXUS_V2_AFFORDANCE_INPUT_TOUCH,
     NEXUS_CMD_BACKWARD,                       "touch_swipe_down"},
    {NEXUS_V2_AFFORDANCE_TOUCH_SWIPE_LEFT,     NEXUS_V2_AFFORDANCE_INPUT_TOUCH,
     NEXUS_CMD_TURN_LEFT,                      "touch_swipe_left"},
    {NEXUS_V2_AFFORDANCE_TOUCH_SWIPE_RIGHT,    NEXUS_V2_AFFORDANCE_INPUT_TOUCH,
     NEXUS_CMD_TURN_RIGHT,                     "touch_swipe_right"},
    /* Touch edge-strafe */
    {NEXUS_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_LEFT,  NEXUS_V2_AFFORDANCE_INPUT_TOUCH,
     NEXUS_CMD_STRAFE_LEFT,                    "touch_edge_strafe_left"},
    {NEXUS_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_RIGHT, NEXUS_V2_AFFORDANCE_INPUT_TOUCH,
     NEXUS_CMD_STRAFE_RIGHT,                   "touch_edge_strafe_right"},
    /* Controller D-pad */
    {NEXUS_V2_AFFORDANCE_CONTROLLER_DPAD_UP,    NEXUS_V2_AFFORDANCE_INPUT_CONTROLLER,
     NEXUS_CMD_FORWARD,                        "controller_dpad_up"},
    {NEXUS_V2_AFFORDANCE_CONTROLLER_DPAD_DOWN,  NEXUS_V2_AFFORDANCE_INPUT_CONTROLLER,
     NEXUS_CMD_BACKWARD,                       "controller_dpad_down"},
    {NEXUS_V2_AFFORDANCE_CONTROLLER_DPAD_LEFT,  NEXUS_V2_AFFORDANCE_INPUT_CONTROLLER,
     NEXUS_CMD_TURN_LEFT,                      "controller_dpad_left"},
    {NEXUS_V2_AFFORDANCE_CONTROLLER_DPAD_RIGHT, NEXUS_V2_AFFORDANCE_INPUT_CONTROLLER,
     NEXUS_CMD_TURN_RIGHT,                     "controller_dpad_right"},
    /* Left stick */
    {NEXUS_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_UP,    NEXUS_V2_AFFORDANCE_INPUT_CONTROLLER,
     NEXUS_CMD_FORWARD,                        "controller_left_stick_up"},
    {NEXUS_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_DOWN,  NEXUS_V2_AFFORDANCE_INPUT_CONTROLLER,
     NEXUS_CMD_BACKWARD,                       "controller_left_stick_down"},
    {NEXUS_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_LEFT,  NEXUS_V2_AFFORDANCE_INPUT_CONTROLLER,
     NEXUS_CMD_STRAFE_LEFT,                    "controller_left_stick_left"},
    {NEXUS_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_RIGHT, NEXUS_V2_AFFORDANCE_INPUT_CONTROLLER,
     NEXUS_CMD_STRAFE_RIGHT,                   "controller_left_stick_right"},
    /* Right stick — turning only */
    {NEXUS_V2_AFFORDANCE_CONTROLLER_RIGHT_STICK_LEFT,  NEXUS_V2_AFFORDANCE_INPUT_CONTROLLER,
     NEXUS_CMD_TURN_LEFT,                      "controller_right_stick_left"},
    {NEXUS_V2_AFFORDANCE_CONTROLLER_RIGHT_STICK_RIGHT, NEXUS_V2_AFFORDANCE_INPUT_CONTROLLER,
     NEXUS_CMD_TURN_RIGHT,                     "controller_right_stick_right"},
};

/* ── Test 1: movement command mapping ─────────────────────────────────── */

static void test_movement_command_mapping(void) {
    printf("Test 1: movement command mapping\n");
    for (unsigned i = 0; i < sizeof(kCases) / sizeof(kCases[0]); i++) {
        int cmd = nexus_v2_touch_controller_affordance_movement_command(kCases[i].affordance);
        CHECK(cmd == kCases[i].movementCommand);
        if (cmd != kCases[i].movementCommand) {
            fprintf(stderr, "  case %s: got %d, want %d\n",
                    kCases[i].name, cmd, kCases[i].movementCommand);
        }
    }
    printf("  PASS (%zu cases)\n", sizeof(kCases) / sizeof(kCases[0]));
}

/* ── Test 2: input kind classification ───────────────────────────────── */

static void test_input_kind(void) {
    printf("Test 2: input kind classification\n");
    for (unsigned i = 0; i < sizeof(kCases) / sizeof(kCases[0]); i++) {
        Nexus_V2_TouchControllerInputKind kind =
            nexus_v2_touch_controller_affordance_input_kind(kCases[i].affordance);
        CHECK(kind == kCases[i].inputKind);
        if (kind != kCases[i].inputKind) {
            fprintf(stderr, "  case %s: got %d, want %d\n",
                    kCases[i].name, kind, kCases[i].inputKind);
        }
    }
    printf("  PASS\n");
}

/* ── Test 3: accepted when v2PresentationEnabled=1 ────────────────────── */

static void test_v2_accepted(void) {
    printf("Test 3: accepted when v2PresentationEnabled=1\n");
    for (unsigned i = 0; i < sizeof(kCases) / sizeof(kCases[0]); i++) {
        Nexus_V2_TouchControllerAffordanceRoute route =
            nexus_v2_touch_controller_affordance_route(1, kCases[i].affordance);
        CHECK(route.accepted == 1);
        CHECK(route.v2Only == 1);
        CHECK(route.movementCommand == kCases[i].movementCommand);
        /* routeKind must be V2_PRESENTATION (1) */
        CHECK(route.routeKind == 1);
        if (route.accepted != 1) {
            fprintf(stderr, "  case %s: accepted=%d, want 1\n",
                    kCases[i].name, route.accepted);
        }
    }
    printf("  PASS (%zu cases)\n", sizeof(kCases) / sizeof(kCases[0]));
}

/* ── Test 4: rejected when v2PresentationEnabled=0 (V1 parity guard) ──── */

static void test_v1_parity_guard(void) {
    printf("Test 4: V1 parity guard (v2PresentationEnabled=0)\n");
    for (unsigned i = 0; i < sizeof(kCases) / sizeof(kCases[0]); i++) {
        Nexus_V2_TouchControllerAffordanceRoute route =
            nexus_v2_touch_controller_affordance_route(0, kCases[i].affordance);
        CHECK(route.accepted == 0);
        CHECK(route.v2Only == 1);
        /* movementCommand is preserved for debug; routeKind must be V1_SOURCE (0) */
        CHECK(route.routeKind == 0);
        if (route.accepted != 0) {
            fprintf(stderr, "  case %s: accepted=%d, want 0 (V1 parity)\n",
                    kCases[i].name, route.accepted);
        }
    }
    printf("  PASS (%zu cases)\n", sizeof(kCases) / sizeof(kCases[0]));
}

/* ── Test 5: name strings are non-trivial ─────────────────────────────── */

static void test_name_strings(void) {
    printf("Test 5: name strings\n");
    static const struct {
        Nexus_V2_TouchControllerAffordance aff;
        const char *expected;
    } kNameCases[] = {
        {NEXUS_V2_AFFORDANCE_TOUCH_SWIPE_UP,              "touch_swipe_up"},
        {NEXUS_V2_AFFORDANCE_TOUCH_SWIPE_DOWN,            "touch_swipe_down"},
        {NEXUS_V2_AFFORDANCE_TOUCH_SWIPE_LEFT,            "touch_swipe_left"},
        {NEXUS_V2_AFFORDANCE_TOUCH_SWIPE_RIGHT,           "touch_swipe_right"},
        {NEXUS_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_LEFT,      "touch_edge_strafe_left"},
        {NEXUS_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_RIGHT,     "touch_edge_strafe_right"},
        {NEXUS_V2_AFFORDANCE_CONTROLLER_DPAD_UP,          "controller_dpad_up"},
        {NEXUS_V2_AFFORDANCE_CONTROLLER_DPAD_DOWN,        "controller_dpad_down"},
        {NEXUS_V2_AFFORDANCE_CONTROLLER_DPAD_LEFT,        "controller_dpad_left"},
        {NEXUS_V2_AFFORDANCE_CONTROLLER_DPAD_RIGHT,       "controller_dpad_right"},
        {NEXUS_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_UP,    "controller_left_stick_up"},
        {NEXUS_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_DOWN,  "controller_left_stick_down"},
        {NEXUS_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_LEFT,  "controller_left_stick_left"},
        {NEXUS_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_RIGHT, "controller_left_stick_right"},
        {NEXUS_V2_AFFORDANCE_CONTROLLER_RIGHT_STICK_LEFT, "controller_right_stick_left"},
        {NEXUS_V2_AFFORDANCE_CONTROLLER_RIGHT_STICK_RIGHT,"controller_right_stick_right"},
        {NEXUS_V2_AFFORDANCE_NONE,                        "none"},
    };
    for (unsigned i = 0; i < sizeof(kNameCases) / sizeof(kNameCases[0]); i++) {
        const char *name = nexus_v2_touch_controller_affordance_name(kNameCases[i].aff);
        CHECK(strcmp(name, kNameCases[i].expected) == 0);
        if (strcmp(name, kNameCases[i].expected) != 0) {
            fprintf(stderr, "  aff %d: got '%s', want '%s'\n",
                    kNameCases[i].aff, name, kNameCases[i].expected);
        }
    }
    printf("  PASS (%zu cases)\n", sizeof(kNameCases) / sizeof(kNameCases[0]));
}

/* ── Test 6: source evidence is non-trivial ────────────────────────────── */

static void test_source_evidence(void) {
    printf("Test 6: source evidence\n");
    const char *ev = nexus_v2_touch_controller_affordance_source_evidence();
    CHECK(ev != NULL);
    CHECK(strlen(ev) > 100);
    /* Check key phrases */
    CHECK(strstr(ev, "CLIKMENU.C") != NULL);
    CHECK(strstr(ev, "NEXUS_CMD_") != NULL);
    CHECK(strstr(ev, "GAMELOOP.C") != NULL);
    CHECK(strstr(ev, "nexus_movement_tick") != NULL);
    CHECK(strstr(ev, "Phase 6") != NULL);
    printf("  PASS (ev len=%zu)\n", strlen(ev));
}

/* ── Test 7: NONE affordance ────────────────────────────────────────────── */

static void test_none_affordance(void) {
    printf("Test 7: NONE affordance\n");
    int cmd = nexus_v2_touch_controller_affordance_movement_command(NEXUS_V2_AFFORDANCE_NONE);
    CHECK(cmd == NEXUS_CMD_NONE);

    Nexus_V2_TouchControllerInputKind kind =
        nexus_v2_touch_controller_affordance_input_kind(NEXUS_V2_AFFORDANCE_NONE);
    CHECK(kind == NEXUS_V2_AFFORDANCE_INPUT_NONE);

    Nexus_V2_TouchControllerAffordanceRoute r0 =
        nexus_v2_touch_controller_affordance_route(0, NEXUS_V2_AFFORDANCE_NONE);
    CHECK(r0.accepted == 0);
    CHECK(r0.routeKind == 0);

    Nexus_V2_TouchControllerAffordanceRoute r1 =
        nexus_v2_touch_controller_affordance_route(1, NEXUS_V2_AFFORDANCE_NONE);
    CHECK(r1.accepted == 0);
    CHECK(r1.routeKind == 1);

    printf("  PASS\n");
}

/* ── Test 8: V1/V2 route kind correctness ──────────────────────────────── */

static void test_route_kind(void) {
    printf("Test 8: route kind\n");
    /* V2 presentation = 1 */
    Nexus_V2_TouchControllerAffordanceRoute r2 =
        nexus_v2_touch_controller_affordance_route(1, NEXUS_V2_AFFORDANCE_TOUCH_SWIPE_UP);
    CHECK(r2.routeKind == 1);  /* V2_PRESENTATION */
    CHECK(r2.accepted == 1);

    /* V1 source = 0 */
    Nexus_V2_TouchControllerAffordanceRoute r1 =
        nexus_v2_touch_controller_affordance_route(0, NEXUS_V2_AFFORDANCE_TOUCH_SWIPE_UP);
    CHECK(r1.routeKind == 0);  /* V1_SOURCE */
    CHECK(r1.accepted == 0);

    printf("  PASS\n");
}

/* ── main ───────────────────────────────────────────────────────────────── */

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;
    printf("=== Nexus V2 Touch/Controller Affordance Tests ===\n");
    printf("Nexus V2 Phase 6 — touch/controller ergonomics\n\n");

    test_movement_command_mapping();
    test_input_kind();
    test_v2_accepted();
    test_v1_parity_guard();
    test_name_strings();
    test_source_evidence();
    test_none_affordance();
    test_route_kind();

    printf("\n%s: %d failure(s)\n", failures ? "FAIL" : "PASS", failures);
    return failures ? 1 : 0;
}
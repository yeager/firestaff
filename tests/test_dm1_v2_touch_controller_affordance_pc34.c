#include "dm1_v2_touch_controller_affordance_pc34.h"

#include <stdio.h>
#include <string.h>

static int failures = 0;

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        failures++; \
    } \
} while (0)

typedef struct {
    DM1_V2_TouchControllerAffordance affordance;
    DM1_V2_TouchControllerInputKind inputKind;
    DM1_V2_MovementCommand movementCommand;
    int sourceCommand;
    int runtimeCommand;
    const char* name;
} AffordanceCase;

static const AffordanceCase kCases[] = {
    {DM1_V2_AFFORDANCE_TOUCH_SWIPE_UP, DM1_V2_AFFORDANCE_INPUT_TOUCH,
     DM1_V2_MOVEMENT_COMMAND_MOVE_FORWARD, 3, 1, "touch_swipe_up"},
    {DM1_V2_AFFORDANCE_TOUCH_SWIPE_DOWN, DM1_V2_AFFORDANCE_INPUT_TOUCH,
     DM1_V2_MOVEMENT_COMMAND_MOVE_BACKWARD, 5, 2, "touch_swipe_down"},
    {DM1_V2_AFFORDANCE_TOUCH_SWIPE_LEFT, DM1_V2_AFFORDANCE_INPUT_TOUCH,
     DM1_V2_MOVEMENT_COMMAND_TURN_LEFT, 1, 3, "touch_swipe_left"},
    {DM1_V2_AFFORDANCE_TOUCH_SWIPE_RIGHT, DM1_V2_AFFORDANCE_INPUT_TOUCH,
     DM1_V2_MOVEMENT_COMMAND_TURN_RIGHT, 2, 4, "touch_swipe_right"},
    {DM1_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_LEFT, DM1_V2_AFFORDANCE_INPUT_TOUCH,
     DM1_V2_MOVEMENT_COMMAND_MOVE_LEFT, 6, 6, "touch_edge_strafe_left"},
    {DM1_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_RIGHT, DM1_V2_AFFORDANCE_INPUT_TOUCH,
     DM1_V2_MOVEMENT_COMMAND_MOVE_RIGHT, 4, 5, "touch_edge_strafe_right"},
    {DM1_V2_AFFORDANCE_CONTROLLER_DPAD_UP, DM1_V2_AFFORDANCE_INPUT_CONTROLLER,
     DM1_V2_MOVEMENT_COMMAND_MOVE_FORWARD, 3, 1, "controller_dpad_up"},
    {DM1_V2_AFFORDANCE_CONTROLLER_DPAD_DOWN, DM1_V2_AFFORDANCE_INPUT_CONTROLLER,
     DM1_V2_MOVEMENT_COMMAND_MOVE_BACKWARD, 5, 2, "controller_dpad_down"},
    {DM1_V2_AFFORDANCE_CONTROLLER_DPAD_LEFT, DM1_V2_AFFORDANCE_INPUT_CONTROLLER,
     DM1_V2_MOVEMENT_COMMAND_TURN_LEFT, 1, 3, "controller_dpad_left"},
    {DM1_V2_AFFORDANCE_CONTROLLER_DPAD_RIGHT, DM1_V2_AFFORDANCE_INPUT_CONTROLLER,
     DM1_V2_MOVEMENT_COMMAND_TURN_RIGHT, 2, 4, "controller_dpad_right"},
    {DM1_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_UP, DM1_V2_AFFORDANCE_INPUT_CONTROLLER,
     DM1_V2_MOVEMENT_COMMAND_MOVE_FORWARD, 3, 1, "controller_left_stick_up"},
    {DM1_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_DOWN, DM1_V2_AFFORDANCE_INPUT_CONTROLLER,
     DM1_V2_MOVEMENT_COMMAND_MOVE_BACKWARD, 5, 2, "controller_left_stick_down"},
    {DM1_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_LEFT, DM1_V2_AFFORDANCE_INPUT_CONTROLLER,
     DM1_V2_MOVEMENT_COMMAND_MOVE_LEFT, 6, 6, "controller_left_stick_left"},
    {DM1_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_RIGHT, DM1_V2_AFFORDANCE_INPUT_CONTROLLER,
     DM1_V2_MOVEMENT_COMMAND_MOVE_RIGHT, 4, 5, "controller_left_stick_right"},
    {DM1_V2_AFFORDANCE_CONTROLLER_RIGHT_STICK_LEFT, DM1_V2_AFFORDANCE_INPUT_CONTROLLER,
     DM1_V2_MOVEMENT_COMMAND_TURN_LEFT, 1, 3, "controller_right_stick_left"},
    {DM1_V2_AFFORDANCE_CONTROLLER_RIGHT_STICK_RIGHT, DM1_V2_AFFORDANCE_INPUT_CONTROLLER,
     DM1_V2_MOVEMENT_COMMAND_TURN_RIGHT, 2, 4, "controller_right_stick_right"},
    {DM1_V2_AFFORDANCE_CONTROLLER_LEFT_BUMPER, DM1_V2_AFFORDANCE_INPUT_CONTROLLER,
     DM1_V2_MOVEMENT_COMMAND_MOVE_LEFT, 6, 6, "controller_left_bumper"},
    {DM1_V2_AFFORDANCE_CONTROLLER_RIGHT_BUMPER, DM1_V2_AFFORDANCE_INPUT_CONTROLLER,
     DM1_V2_MOVEMENT_COMMAND_MOVE_RIGHT, 4, 5, "controller_right_bumper"},
};

static void test_v2_affordances_map_to_source_locked_routes(void) {
    size_t i;

    /* Source-lock: DEFS.H:238-243 owns C001..C006 and COMMAND.C:2045-2155
     * consumes those ids. These V2 affordances add labels only; they reuse
     * dm1_v2_movement_command_route_for_presentation for runtime routing. */
    for (i = 0; i < sizeof(kCases) / sizeof(kCases[0]); ++i) {
        DM1_V2_TouchControllerAffordanceRoute route =
            dm1_v2_touch_controller_affordance_route(1, kCases[i].affordance);
        CHECK(route.accepted == 1);
        CHECK(route.v2Only == 1);
        CHECK(route.inputKind == kCases[i].inputKind);
        CHECK(route.affordance == kCases[i].affordance);
        CHECK(route.movementCommand == kCases[i].movementCommand);
        CHECK(route.route.routeKind == DM1_V2_MOVEMENT_ROUTE_V2_PRESENTATION);
        CHECK(route.route.v2PresentationEnabled == 1);
        CHECK(route.route.sourceCommand == kCases[i].sourceCommand);
        CHECK(route.route.runtimeCommand == kCases[i].runtimeCommand);
        CHECK(dm1_v2_touch_controller_affordance_movement_command(kCases[i].affordance) ==
              kCases[i].movementCommand);
        CHECK(dm1_v2_touch_controller_affordance_input_kind(kCases[i].affordance) ==
              kCases[i].inputKind);
        CHECK(strcmp(dm1_v2_touch_controller_affordance_name(kCases[i].affordance),
                     kCases[i].name) == 0);
    }
}

static void test_v2_affordance_runtime_bridge_uses_existing_command_routes(void) {
    size_t i;

    /* Runtime bridge gate: V2 touch/controller labels resolve to the same
     * source route first, then enter the existing V2 runtime adapter.  The
     * bridge must not invent a parallel command id or bypass the adapter. */
    for (i = 0; i < sizeof(kCases) / sizeof(kCases[0]); ++i) {
        DM1_V2_RuntimeState runtime;
        DM1_V2_CameraController camera;
        DM1_V2_TouchControllerAffordanceRoute route;
        DM1_V2_MovementCommandResult result;

        dm1_v2_runtime_init(&runtime);
        dm1_v2_runtime_start(&runtime, 1000U);
        dm1_v2_camera_init(&camera, &runtime.player);

        route = dm1_v2_touch_controller_affordance_route(1, kCases[i].affordance);
        result = dm1_v2_movement_command_apply(
            &runtime,
            &camera,
            route.movementCommand,
            1000U,
            64);

        CHECK(route.accepted == 1);
        CHECK(route.v2Only == 1);
        CHECK(route.route.sourceCommand == kCases[i].sourceCommand);
        CHECK(route.route.runtimeCommand == kCases[i].runtimeCommand);
        CHECK(result.accepted == 1);
        CHECK(result.sourceCommand == route.route.sourceCommand);
        CHECK(result.runtimeCommand == route.route.runtimeCommand);
        CHECK(runtime.lastCommand == route.route.runtimeCommand);
        CHECK(dm1_v2_camera_is_active(&camera));
    }
}

static void test_v1_off_rejects_v2_only_affordances(void) {
    size_t i;

    /* V1 touch/click parity guard: ReDMCSB DEFS.H:197-211 keeps mouse input
     * as command-bearing zones.  With V2 presentation disabled, gesture and
     * controller affordances must not inject commands into that V1 path. */
    for (i = 0; i < sizeof(kCases) / sizeof(kCases[0]); ++i) {
        DM1_V2_TouchControllerAffordanceRoute route =
            dm1_v2_touch_controller_affordance_route(0, kCases[i].affordance);
        CHECK(route.accepted == 0);
        CHECK(route.v2Only == 1);
        CHECK(route.inputKind == kCases[i].inputKind);
        CHECK(route.movementCommand == kCases[i].movementCommand);
        CHECK(route.route.routeKind == DM1_V2_MOVEMENT_ROUTE_V1_SOURCE);
        CHECK(route.route.v2PresentationEnabled == 0);
        CHECK(route.route.sourceCommand == 0);
        CHECK(route.route.runtimeCommand == 0);
    }
}

static void test_invalid_affordance_is_not_a_command(void) {
    DM1_V2_TouchControllerAffordanceRoute route;

    route = dm1_v2_touch_controller_affordance_route(1, DM1_V2_AFFORDANCE_NONE);
    CHECK(route.accepted == 0);
    CHECK(route.inputKind == DM1_V2_AFFORDANCE_INPUT_NONE);
    CHECK(route.movementCommand == DM1_V2_MOVEMENT_COMMAND_NONE);
    CHECK(route.route.sourceCommand == 0);
    CHECK(route.route.runtimeCommand == 0);
    CHECK(strcmp(dm1_v2_touch_controller_affordance_name(DM1_V2_AFFORDANCE_NONE), "none") == 0);
}

static void test_route_probe_does_not_mutate_runtime_state(void) {
    DM1_V2_RuntimeState runtime;

    dm1_v2_runtime_init(&runtime);
    dm1_v2_runtime_start(&runtime, 1000U);
    (void)dm1_v2_touch_controller_affordance_route(
        1,
        DM1_V2_AFFORDANCE_TOUCH_SWIPE_UP);
    CHECK(runtime.lastCommand == 0);
    CHECK(runtime.player.facingDir == 0);
    CHECK(dm1_v2_get_x(&runtime.player) == 0);
    CHECK(dm1_v2_get_y(&runtime.player) == 0);
}

static void test_source_evidence_string_names_referenced_routes(void) {
    const char* evidence = dm1_v2_touch_controller_affordance_source_lock_evidence();
    CHECK(strstr(evidence, "DEFS.H:197-211,238-243") != NULL);
    CHECK(strstr(evidence, "COMMAND.C:2045-2155") != NULL);
    CHECK(strstr(evidence, "CLIKMENU.C:142-174,180-390") != NULL);
    CHECK(strstr(evidence, "GAMELOOP.C:164-219") != NULL);
}

int main(void) {
    test_v2_affordances_map_to_source_locked_routes();
    test_v2_affordance_runtime_bridge_uses_existing_command_routes();
    test_v1_off_rejects_v2_only_affordances();
    test_invalid_affordance_is_not_a_command();
    test_route_probe_does_not_mutate_runtime_state();
    test_source_evidence_string_names_referenced_routes();

    if (failures) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    puts("dm1_v2_touch_controller_affordance_pc34: ok");
    return 0;
}

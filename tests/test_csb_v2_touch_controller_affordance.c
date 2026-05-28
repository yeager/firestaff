#include "csb_v2_touch_controller_affordance.h"

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
    CSB_V2_TouchControllerAffordance affordance;
    CSB_V2_TouchControllerInputKind inputKind;
    DM1_V2_MovementCommand movementCommand;
    int sourceCommand;
    int runtimeCommand;
    const char *name;
} CSBAffordanceCase;

static const CSBAffordanceCase kMovementCases[] = {
    {CSB_V2_AFFORDANCE_TOUCH_SWIPE_UP, CSB_V2_AFFORDANCE_INPUT_TOUCH,
     DM1_V2_MOVEMENT_COMMAND_MOVE_FORWARD, 3, 1, "touch_swipe_up"},
    {CSB_V2_AFFORDANCE_TOUCH_SWIPE_DOWN, CSB_V2_AFFORDANCE_INPUT_TOUCH,
     DM1_V2_MOVEMENT_COMMAND_MOVE_BACKWARD, 5, 2, "touch_swipe_down"},
    {CSB_V2_AFFORDANCE_TOUCH_SWIPE_LEFT, CSB_V2_AFFORDANCE_INPUT_TOUCH,
     DM1_V2_MOVEMENT_COMMAND_TURN_LEFT, 1, 3, "touch_swipe_left"},
    {CSB_V2_AFFORDANCE_TOUCH_SWIPE_RIGHT, CSB_V2_AFFORDANCE_INPUT_TOUCH,
     DM1_V2_MOVEMENT_COMMAND_TURN_RIGHT, 2, 4, "touch_swipe_right"},
    {CSB_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_LEFT, CSB_V2_AFFORDANCE_INPUT_TOUCH,
     DM1_V2_MOVEMENT_COMMAND_MOVE_LEFT, 6, 6, "touch_edge_strafe_left"},
    {CSB_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_RIGHT, CSB_V2_AFFORDANCE_INPUT_TOUCH,
     DM1_V2_MOVEMENT_COMMAND_MOVE_RIGHT, 4, 5, "touch_edge_strafe_right"},
    {CSB_V2_AFFORDANCE_CONTROLLER_DPAD_UP, CSB_V2_AFFORDANCE_INPUT_CONTROLLER,
     DM1_V2_MOVEMENT_COMMAND_MOVE_FORWARD, 3, 1, "controller_dpad_up"},
    {CSB_V2_AFFORDANCE_CONTROLLER_DPAD_DOWN, CSB_V2_AFFORDANCE_INPUT_CONTROLLER,
     DM1_V2_MOVEMENT_COMMAND_MOVE_BACKWARD, 5, 2, "controller_dpad_down"},
    {CSB_V2_AFFORDANCE_CONTROLLER_DPAD_LEFT, CSB_V2_AFFORDANCE_INPUT_CONTROLLER,
     DM1_V2_MOVEMENT_COMMAND_TURN_LEFT, 1, 3, "controller_dpad_left"},
    {CSB_V2_AFFORDANCE_CONTROLLER_DPAD_RIGHT, CSB_V2_AFFORDANCE_INPUT_CONTROLLER,
     DM1_V2_MOVEMENT_COMMAND_TURN_RIGHT, 2, 4, "controller_dpad_right"},
    {CSB_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_UP, CSB_V2_AFFORDANCE_INPUT_CONTROLLER,
     DM1_V2_MOVEMENT_COMMAND_MOVE_FORWARD, 3, 1, "controller_left_stick_up"},
    {CSB_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_DOWN, CSB_V2_AFFORDANCE_INPUT_CONTROLLER,
     DM1_V2_MOVEMENT_COMMAND_MOVE_BACKWARD, 5, 2, "controller_left_stick_down"},
    {CSB_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_LEFT, CSB_V2_AFFORDANCE_INPUT_CONTROLLER,
     DM1_V2_MOVEMENT_COMMAND_MOVE_LEFT, 6, 6, "controller_left_stick_left"},
    {CSB_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_RIGHT, CSB_V2_AFFORDANCE_INPUT_CONTROLLER,
     DM1_V2_MOVEMENT_COMMAND_MOVE_RIGHT, 4, 5, "controller_left_stick_right"},
    {CSB_V2_AFFORDANCE_CONTROLLER_RIGHT_STICK_LEFT, CSB_V2_AFFORDANCE_INPUT_CONTROLLER,
     DM1_V2_MOVEMENT_COMMAND_TURN_LEFT, 1, 3, "controller_right_stick_left"},
    {CSB_V2_AFFORDANCE_CONTROLLER_RIGHT_STICK_RIGHT, CSB_V2_AFFORDANCE_INPUT_CONTROLLER,
     DM1_V2_MOVEMENT_COMMAND_TURN_RIGHT, 2, 4, "controller_right_stick_right"},
};

static void test_movement_affordances_route_to_source_commands(void) {
    size_t i;

    /* Source-lock: CSB shares the DM1 command ids C001..C006. ReDMCSB
     * COMMAND.C and CLIKMENU.C remain the owners of movement behavior. */
    for (i = 0; i < sizeof(kMovementCases) / sizeof(kMovementCases[0]); ++i) {
        CSB_V2_TouchControllerAffordanceRoute route =
            csb_v2_touch_controller_affordance_route(1, kMovementCases[i].affordance);
        CHECK(route.accepted == 1);
        CHECK(route.v2Only == 1);
        CHECK(route.inputKind == kMovementCases[i].inputKind);
        CHECK(route.affordance == kMovementCases[i].affordance);
        CHECK(route.movementCommand == kMovementCases[i].movementCommand);
        CHECK(route.route.routeKind == DM1_V2_MOVEMENT_ROUTE_V2_PRESENTATION);
        CHECK(route.route.v2PresentationEnabled == 1);
        CHECK(route.route.sourceCommand == kMovementCases[i].sourceCommand);
        CHECK(route.route.runtimeCommand == kMovementCases[i].runtimeCommand);
        CHECK(csb_v2_touch_controller_affordance_movement_command(kMovementCases[i].affordance) ==
              kMovementCases[i].movementCommand);
        CHECK(csb_v2_touch_controller_affordance_input_kind(kMovementCases[i].affordance) ==
              kMovementCases[i].inputKind);
        CHECK(strcmp(csb_v2_touch_controller_affordance_name(kMovementCases[i].affordance),
                     kMovementCases[i].name) == 0);
    }
}

static void test_v1_off_rejects_v2_affordances(void) {
    size_t i;

    for (i = 0; i < sizeof(kMovementCases) / sizeof(kMovementCases[0]); ++i) {
        CSB_V2_TouchControllerAffordanceRoute route =
            csb_v2_touch_controller_affordance_route(0, kMovementCases[i].affordance);
        CHECK(route.accepted == 0);
        CHECK(route.route.routeKind == DM1_V2_MOVEMENT_ROUTE_V1_SOURCE);
        CHECK(route.route.v2PresentationEnabled == 0);
        CHECK(route.route.sourceCommand == 0);
        CHECK(route.route.runtimeCommand == 0);
    }
}

static void test_csb_shoulder_buttons_are_actions_not_movement(void) {
    CSB_V2_TouchControllerAffordanceRoute left =
        csb_v2_touch_controller_affordance_route(
            1,
            CSB_V2_AFFORDANCE_CONTROLLER_LEFT_BUMPER);
    CSB_V2_TouchControllerAffordanceRoute right =
        csb_v2_touch_controller_affordance_route(
            1,
            CSB_V2_AFFORDANCE_CONTROLLER_RIGHT_BUMPER);

    CHECK(csb_v2_touch_controller_affordance_movement_command(
              CSB_V2_AFFORDANCE_CONTROLLER_LEFT_BUMPER) ==
          DM1_V2_MOVEMENT_COMMAND_NONE);
    CHECK(csb_v2_touch_controller_affordance_movement_command(
              CSB_V2_AFFORDANCE_CONTROLLER_RIGHT_BUMPER) ==
          DM1_V2_MOVEMENT_COMMAND_NONE);

    CHECK(left.accepted == 1);
    CHECK(left.inputKind == CSB_V2_AFFORDANCE_INPUT_CONTROLLER);
    CHECK(left.movementCommand == DM1_V2_MOVEMENT_COMMAND_NONE);
    CHECK(left.route.routeKind == DM1_V2_MOVEMENT_ROUTE_V2_PRESENTATION);
    CHECK(left.route.sourceCommand == 0);
    CHECK(left.route.runtimeCommand == 0);

    CHECK(right.accepted == 1);
    CHECK(right.inputKind == CSB_V2_AFFORDANCE_INPUT_CONTROLLER);
    CHECK(right.movementCommand == DM1_V2_MOVEMENT_COMMAND_NONE);
    CHECK(right.route.routeKind == DM1_V2_MOVEMENT_ROUTE_V2_PRESENTATION);
    CHECK(right.route.sourceCommand == 0);
    CHECK(right.route.runtimeCommand == 0);
}

static void test_invalid_affordance_and_evidence(void) {
    CSB_V2_TouchControllerAffordanceRoute route =
        csb_v2_touch_controller_affordance_route(1, CSB_V2_AFFORDANCE_NONE);
    const char *evidence = csb_v2_touch_controller_affordance_source_evidence();

    CHECK(route.accepted == 0);
    CHECK(route.inputKind == CSB_V2_AFFORDANCE_INPUT_NONE);
    CHECK(route.movementCommand == DM1_V2_MOVEMENT_COMMAND_NONE);
    CHECK(strcmp(csb_v2_touch_controller_affordance_name(CSB_V2_AFFORDANCE_NONE), "none") == 0);
    CHECK(strstr(evidence, "COMMAND.C:108-113") != NULL);
    CHECK(strstr(evidence, "CLIKMENU.C:142") != NULL);
    CHECK(strstr(evidence, "GAMELOOP.C:164-219") != NULL);
    CHECK(strstr(evidence, "csb_v1_chaos_trigger") != NULL);
}

int main(void) {
    test_movement_affordances_route_to_source_commands();
    test_v1_off_rejects_v2_affordances();
    test_csb_shoulder_buttons_are_actions_not_movement();
    test_invalid_affordance_and_evidence();

    if (failures) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    puts("csb_v2_touch_controller_affordance: ok");
    return 0;
}

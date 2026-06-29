#include "theron_v2_touch_runtime.h"
#include <stdio.h>
#include <string.h>

static int g_failed = 0;
static int g_total = 0;

#define CHECK(cond) do { \
    g_total++; \
    if (!(cond)) { \
        g_failed++; \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
    } \
} while (0)

typedef struct {
    Theron_V2_TouchControllerAffordance aff;
    Theron_V2_TouchControllerInputKind kind;
    DM1_V2_MovementCommand movement;
    int v1_command;
    const char *name;
} AffordanceCase;

static const AffordanceCase k_cases[] = {
    { THERON_V2_AFFORDANCE_TOUCH_SWIPE_UP, THERON_V2_AFFORDANCE_INPUT_TOUCH,
      DM1_V2_MOVEMENT_COMMAND_MOVE_FORWARD, DM1_V1_COMMAND_MOVE_FORWARD, "touch_swipe_up" },
    { THERON_V2_AFFORDANCE_TOUCH_SWIPE_DOWN, THERON_V2_AFFORDANCE_INPUT_TOUCH,
      DM1_V2_MOVEMENT_COMMAND_MOVE_BACKWARD, DM1_V1_COMMAND_MOVE_BACKWARD, "touch_swipe_down" },
    { THERON_V2_AFFORDANCE_TOUCH_SWIPE_LEFT, THERON_V2_AFFORDANCE_INPUT_TOUCH,
      DM1_V2_MOVEMENT_COMMAND_TURN_LEFT, DM1_V1_COMMAND_TURN_LEFT, "touch_swipe_left" },
    { THERON_V2_AFFORDANCE_TOUCH_SWIPE_RIGHT, THERON_V2_AFFORDANCE_INPUT_TOUCH,
      DM1_V2_MOVEMENT_COMMAND_TURN_RIGHT, DM1_V1_COMMAND_TURN_RIGHT, "touch_swipe_right" },
    { THERON_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_LEFT, THERON_V2_AFFORDANCE_INPUT_TOUCH,
      DM1_V2_MOVEMENT_COMMAND_MOVE_LEFT, DM1_V1_COMMAND_MOVE_LEFT, "touch_edge_strafe_left" },
    { THERON_V2_AFFORDANCE_TOUCH_EDGE_STRAFE_RIGHT, THERON_V2_AFFORDANCE_INPUT_TOUCH,
      DM1_V2_MOVEMENT_COMMAND_MOVE_RIGHT, DM1_V1_COMMAND_MOVE_RIGHT, "touch_edge_strafe_right" },
    { THERON_V2_AFFORDANCE_CONTROLLER_DPAD_UP, THERON_V2_AFFORDANCE_INPUT_CONTROLLER,
      DM1_V2_MOVEMENT_COMMAND_MOVE_FORWARD, DM1_V1_COMMAND_MOVE_FORWARD, "controller_dpad_up" },
    { THERON_V2_AFFORDANCE_CONTROLLER_DPAD_DOWN, THERON_V2_AFFORDANCE_INPUT_CONTROLLER,
      DM1_V2_MOVEMENT_COMMAND_MOVE_BACKWARD, DM1_V1_COMMAND_MOVE_BACKWARD, "controller_dpad_down" },
    { THERON_V2_AFFORDANCE_CONTROLLER_DPAD_LEFT, THERON_V2_AFFORDANCE_INPUT_CONTROLLER,
      DM1_V2_MOVEMENT_COMMAND_TURN_LEFT, DM1_V1_COMMAND_TURN_LEFT, "controller_dpad_left" },
    { THERON_V2_AFFORDANCE_CONTROLLER_DPAD_RIGHT, THERON_V2_AFFORDANCE_INPUT_CONTROLLER,
      DM1_V2_MOVEMENT_COMMAND_TURN_RIGHT, DM1_V1_COMMAND_TURN_RIGHT, "controller_dpad_right" },
    { THERON_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_UP, THERON_V2_AFFORDANCE_INPUT_CONTROLLER,
      DM1_V2_MOVEMENT_COMMAND_MOVE_FORWARD, DM1_V1_COMMAND_MOVE_FORWARD, "controller_left_stick_up" },
    { THERON_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_DOWN, THERON_V2_AFFORDANCE_INPUT_CONTROLLER,
      DM1_V2_MOVEMENT_COMMAND_MOVE_BACKWARD, DM1_V1_COMMAND_MOVE_BACKWARD, "controller_left_stick_down" },
    { THERON_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_LEFT, THERON_V2_AFFORDANCE_INPUT_CONTROLLER,
      DM1_V2_MOVEMENT_COMMAND_MOVE_LEFT, DM1_V1_COMMAND_MOVE_LEFT, "controller_left_stick_left" },
    { THERON_V2_AFFORDANCE_CONTROLLER_LEFT_STICK_RIGHT, THERON_V2_AFFORDANCE_INPUT_CONTROLLER,
      DM1_V2_MOVEMENT_COMMAND_MOVE_RIGHT, DM1_V1_COMMAND_MOVE_RIGHT, "controller_left_stick_right" },
    { THERON_V2_AFFORDANCE_CONTROLLER_RIGHT_STICK_LEFT, THERON_V2_AFFORDANCE_INPUT_CONTROLLER,
      DM1_V2_MOVEMENT_COMMAND_TURN_LEFT, DM1_V1_COMMAND_TURN_LEFT, "controller_right_stick_left" },
    { THERON_V2_AFFORDANCE_CONTROLLER_RIGHT_STICK_RIGHT, THERON_V2_AFFORDANCE_INPUT_CONTROLLER,
      DM1_V2_MOVEMENT_COMMAND_TURN_RIGHT, DM1_V1_COMMAND_TURN_RIGHT, "controller_right_stick_right" }
};

static void test_affordance_mapping(void)
{
    size_t i;
    for (i = 0; i < sizeof(k_cases) / sizeof(k_cases[0]); ++i) {
        Theron_V2_TouchControllerAffordanceRoute v2 =
            theron_v2_touch_controller_affordance_route(1, k_cases[i].aff);
        Theron_V2_TouchControllerAffordanceRoute v1 =
            theron_v2_touch_controller_affordance_route(0, k_cases[i].aff);

        CHECK(theron_v2_touch_controller_affordance_movement_command(k_cases[i].aff) ==
              k_cases[i].movement);
        CHECK(theron_v2_touch_controller_affordance_input_kind(k_cases[i].aff) ==
              k_cases[i].kind);
        CHECK(strcmp(theron_v2_touch_controller_affordance_name(k_cases[i].aff),
                     k_cases[i].name) == 0);
        CHECK(v2.accepted == 1);
        CHECK(v2.v2Only == 1);
        CHECK(v2.route.routeKind == DM1_V2_MOVEMENT_ROUTE_V2_PRESENTATION);
        CHECK(v2.movementCommand == k_cases[i].movement);
        CHECK(v1.accepted == 0);
        CHECK(v1.route.routeKind == DM1_V2_MOVEMENT_ROUTE_V1_SOURCE);
        CHECK(v1.movementCommand == k_cases[i].movement);
    }
    CHECK(theron_v2_touch_controller_affordance_movement_command(THERON_V2_AFFORDANCE_NONE) ==
          DM1_V2_MOVEMENT_COMMAND_NONE);
    CHECK(theron_v2_touch_controller_affordance_input_kind(THERON_V2_AFFORDANCE_NONE) ==
          THERON_V2_AFFORDANCE_INPUT_NONE);
    CHECK(strcmp(theron_v2_touch_controller_affordance_name(THERON_V2_AFFORDANCE_NONE),
                 "none") == 0);
}

static void test_runtime_gate_and_translation(void)
{
    THERON_V2_PhaseGateConfig gate_off = { 0, 0 };
    THERON_V2_PhaseGateConfig gate_half = { 1, 0 };
    THERON_V2_PhaseGateConfig gate_on = { 1, 1 };
    struct Dm1V1QueuedCommandPc34Compat out;
    size_t i;

    theron_v2_touch_runtime_init();
    CHECK(theron_v2_touch_runtime_is_active() == 0);
    theron_v2_touch_runtime_set_gate_config(&gate_off);
    CHECK(theron_v2_touch_runtime_translate_affordance(
              THERON_V2_AFFORDANCE_TOUCH_SWIPE_UP, 64, 64, &out) == 0);
    CHECK(out.command == DM1_V1_COMMAND_NONE);
    theron_v2_touch_runtime_set_gate_config(&gate_half);
    CHECK(theron_v2_touch_runtime_translate_affordance(
              THERON_V2_AFFORDANCE_TOUCH_SWIPE_UP, 64, 64, &out) == 1);
    CHECK(out.command == DM1_V1_COMMAND_MOVE_FORWARD);
    CHECK(theron_v2_touch_runtime_translation_count() == 1);

    theron_v2_touch_runtime_set_gate_config(&gate_on);
    CHECK(theron_v2_touch_runtime_is_active() == 1);
    for (i = 0; i < sizeof(k_cases) / sizeof(k_cases[0]); ++i) {
        CHECK(theron_v2_touch_runtime_translate_affordance(
                  k_cases[i].aff, 64, 64, &out) == 1);
        CHECK(out.command == k_cases[i].v1_command);
        CHECK(out.x == 64);
        CHECK(out.y == 64);
    }
    CHECK(theron_v2_touch_runtime_translation_count() ==
          (int)(sizeof(k_cases) / sizeof(k_cases[0])) + 1);

    CHECK(theron_v2_touch_runtime_translate_affordance(
              THERON_V2_AFFORDANCE_NONE, 64, 64, &out) == 0);
    CHECK(theron_v2_touch_runtime_translate_affordance(
              THERON_V2_AFFORDANCE_TOUCH_SWIPE_UP, 64, 64, NULL) == 0);

    theron_v2_touch_runtime_shutdown();
    CHECK(theron_v2_touch_runtime_translation_count() == 0);
}

static void test_hud_chrome_touch_exclusion(void)
{
    THERON_V2_PhaseGateConfig gate_on = { 1, 1 };
    struct Dm1V1QueuedCommandPc34Compat out;

    CHECK(theron_v2_touch_runtime_point_in_hud_chrome(-1, 10) == 1);
    CHECK(theron_v2_touch_runtime_point_in_hud_chrome(0, -1) == 1);
    CHECK(theron_v2_touch_runtime_point_in_hud_chrome(256, 10) == 1);
    CHECK(theron_v2_touch_runtime_point_in_hud_chrome(10, 224) == 1);
    CHECK(theron_v2_touch_runtime_point_in_hud_chrome(10, 0) == 1);
    CHECK(theron_v2_touch_runtime_point_in_hud_chrome(10, 23) == 1);
    CHECK(theron_v2_touch_runtime_point_in_hud_chrome(10, 24) == 0);
    CHECK(theron_v2_touch_runtime_point_in_hud_chrome(10, 184) == 1);
    CHECK(theron_v2_touch_runtime_point_in_hud_chrome(10, 191) == 1);
    CHECK(theron_v2_touch_runtime_point_in_hud_chrome(10, 192) == 0);
    CHECK(theron_v2_touch_runtime_point_in_hud_chrome(10, 208) == 1);
    CHECK(theron_v2_touch_runtime_point_in_hud_chrome(10, 221) == 1);
    CHECK(theron_v2_touch_runtime_point_in_hud_chrome(10, 222) == 0);

    theron_v2_touch_runtime_init();
    theron_v2_touch_runtime_set_gate_config(&gate_on);
    CHECK(theron_v2_touch_runtime_translate_affordance(
              THERON_V2_AFFORDANCE_TOUCH_SWIPE_UP, 20, 12, &out) == 0);
    CHECK(theron_v2_touch_runtime_translate_affordance(
              THERON_V2_AFFORDANCE_TOUCH_SWIPE_UP, 20, 184, &out) == 0);
    CHECK(theron_v2_touch_runtime_translate_affordance(
              THERON_V2_AFFORDANCE_TOUCH_SWIPE_UP, 20, 208, &out) == 0);
    CHECK(theron_v2_touch_runtime_translate_affordance(
              THERON_V2_AFFORDANCE_CONTROLLER_DPAD_UP, 20, 12, &out) == 1);
    CHECK(out.command == DM1_V1_COMMAND_MOVE_FORWARD);
    CHECK(theron_v2_touch_runtime_translation_count() == 1);
    theron_v2_touch_runtime_shutdown();
}

static void test_force_active_and_evidence(void)
{
    THERON_V2_PhaseGateConfig gate_off = { 0, 0 };
    struct Dm1V1QueuedCommandPc34Compat out;
    const char *aff_ev = theron_v2_touch_controller_affordance_source_evidence();
    const char *runtime_ev = theron_v2_touch_runtime_source_evidence();

    theron_v2_touch_runtime_init();
    theron_v2_touch_runtime_set_gate_config(&gate_off);
    CHECK(theron_v2_touch_runtime_is_active() == 0);
    theron_v2_touch_runtime_force_active_for_test(1);
    CHECK(theron_v2_touch_runtime_is_active() == 1);
    CHECK(theron_v2_touch_runtime_translate_affordance(
              THERON_V2_AFFORDANCE_CONTROLLER_DPAD_RIGHT, 1, 1, &out) == 1);
    CHECK(out.command == DM1_V1_COMMAND_TURN_RIGHT);
    theron_v2_touch_runtime_shutdown();

    CHECK(aff_ev != NULL);
    CHECK(runtime_ev != NULL);
    CHECK(strstr(aff_ev, "THQUEST.ASM") != NULL);
    CHECK(strstr(aff_ev, "DEFS.H:238-243") != NULL);
    CHECK(strstr(runtime_ev, "theron_v2_hud_overlay_pc34.h") != NULL);
    CHECK(strstr(runtime_ev, "V1 invariant") != NULL);
}

int main(void)
{
    test_affordance_mapping();
    test_runtime_gate_and_translation();
    test_hud_chrome_touch_exclusion();
    test_force_active_and_evidence();

    printf("theron_v2_touch_controller_affordance: %d checks, %d failures\n",
           g_total, g_failed);
    return g_failed ? 1 : 0;
}

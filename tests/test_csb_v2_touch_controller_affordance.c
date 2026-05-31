#include "csb_v2_touch_controller_affordance.h"
#include "csb_v2_phase_gate_pc34.h"
#include "dm1_v2_runtime_pc34.h"
#include "dm1_v2_camera_controller_pc34.h"

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

static void test_phase_gate_input_presentation_domain(void) {
    /* INPUT_PRESENTATION is a V2-presentation-eligible domain, not a
     * V1-source-locked gameplay domain.  When V2 is disabled, affordances
     * must be rejected so V1 mouse/touch/click route matrix is sole path.
     * ReDMCSB COMMAND.C:108-113 and csb_v2_phase_gate_pc34.h Phase 0 rules. */
    CSB_V2_PhaseGateConfig config;
    CSB_V2_PhaseGateDecision decision;

    csb_v2_phase_gate_pc34_defaults(&config);

    /* V2 off: INPUT_PRESENTATION not allowed */
    decision = csb_v2_phase_gate_pc34_decide(&config,
        CSB_V2_PHASE_DOMAIN_INPUT_PRESENTATION);
    CHECK(decision.v1SourceLocked == 0);
    CHECK(decision.v2PresentationAllowed == 0);

    /* V2 on: INPUT_PRESENTATION allowed */
    config.v2PresentationEnabled = 1;
    decision = csb_v2_phase_gate_pc34_decide(&config,
        CSB_V2_PHASE_DOMAIN_INPUT_PRESENTATION);
    CHECK(decision.v1SourceLocked == 0);
    CHECK(decision.v2PresentationAllowed == 1);

    /* INPUT_PRESENTATION is not a gameplay domain */
    CHECK(csb_v2_phase_gate_pc34_is_gameplay_domain(
        CSB_V2_PHASE_DOMAIN_INPUT_PRESENTATION) == 0);

    /* COMMAND_SEMANTICS is a gameplay domain (stays V1-locked) */
    CHECK(csb_v2_phase_gate_pc34_is_gameplay_domain(
        CSB_V2_PHASE_DOMAIN_COMMAND_SEMANTICS) == 1);

    /* CHAOS_MAGIC_SCRIPTS is a gameplay domain (stays V1-locked) */
    CHECK(csb_v2_phase_gate_pc34_is_gameplay_domain(
        CSB_V2_PHASE_DOMAIN_CHAOS_MAGIC_SCRIPTS) == 1);
}

static void test_route_v2_off_affordance_rejected_via_phase_gate(void) {
    /* With V2 disabled, affordances must be rejected so V1 mouse click
     * matrix (COMMAND.C:1379 F0358, COMMAND.C:1452 F0359) stays sole input.
     * This mirrors the phase gate INPUT_PRESENTATION decision. */
    size_t i;
    CSB_V2_PhaseGateConfig config;
    csb_v2_phase_gate_pc34_defaults(&config);
    config.v2PresentationEnabled = 0;

    for (i = 0; i < sizeof(kMovementCases) / sizeof(kMovementCases[0]); ++i) {
        CSB_V2_TouchControllerAffordanceRoute route =
            csb_v2_touch_controller_affordance_route(
                config.v2PresentationEnabled,
                kMovementCases[i].affordance);

        /* V2 off → affordance rejected → V1 source route */
        CHECK(route.accepted == 0);
        CHECK(route.route.routeKind == DM1_V2_MOVEMENT_ROUTE_V1_SOURCE);
        CHECK(route.route.v2PresentationEnabled == 0);
        CHECK(route.route.sourceCommand == 0);
        CHECK(route.route.runtimeCommand == 0);

        /* Phase gate confirms V2 presentation not allowed when off */
        CHECK(csb_v2_phase_gate_pc34_v2_active(&config) == 0);
        (void)csb_v2_phase_gate_pc34_decide(&config,
            CSB_V2_PHASE_DOMAIN_INPUT_PRESENTATION);
        /* No assertion on decision here — just confirm it does not crash */
    }
}

static void test_route_v2_on_affordance_accepted_via_phase_gate(void) {
    /* With V2 enabled, movement affordances are accepted and route through
     * V2 presentation pipeline; shoulder buttons also accepted as CSB actions. */
    size_t i;
    CSB_V2_PhaseGateConfig config;
    csb_v2_phase_gate_pc34_defaults(&config);
    config.v2PresentationEnabled = 1;

    for (i = 0; i < sizeof(kMovementCases) / sizeof(kMovementCases[0]); ++i) {
        CSB_V2_TouchControllerAffordanceRoute route =
            csb_v2_touch_controller_affordance_route(
                config.v2PresentationEnabled,
                kMovementCases[i].affordance);
        CHECK(route.accepted == 1);
        CHECK(route.route.routeKind == DM1_V2_MOVEMENT_ROUTE_V2_PRESENTATION);
        CHECK(route.route.v2PresentationEnabled == 1);
        CHECK(route.route.sourceCommand == kMovementCases[i].sourceCommand);
        CHECK(route.route.runtimeCommand == kMovementCases[i].runtimeCommand);
    }

    /* Shoulder buttons also accepted when V2 is on */
    {
        CSB_V2_TouchControllerAffordanceRoute left =
            csb_v2_touch_controller_affordance_route(1,
                CSB_V2_AFFORDANCE_CONTROLLER_LEFT_BUMPER);
        CSB_V2_TouchControllerAffordanceRoute right =
            csb_v2_touch_controller_affordance_route(1,
                CSB_V2_AFFORDANCE_CONTROLLER_RIGHT_BUMPER);
        CHECK(left.accepted == 1);
        CHECK(right.accepted == 1);
    }

    /* Phase gate confirms V2 is active */
    CHECK(csb_v2_phase_gate_pc34_v2_active(&config) == 1);
    (void)csb_v2_phase_gate_pc34_decide(&config,
        CSB_V2_PHASE_DOMAIN_INPUT_PRESENTATION);
}

static void test_route_probe_does_not_mutate_runtime_state(void) {
    /* csb_v2_touch_controller_affordance_route() is a pure query —
     * it reads the affordance and returns a routing decision without
     * touching DM1_V2_RuntimeState.  This mirrors the DM1 V2 test. */
    DM1_V2_RuntimeState runtime;
    dm1_v2_runtime_init(&runtime);
    dm1_v2_runtime_start(&runtime, 1000U);

    /* Call route() — must not mutate runtime state */
    (void)csb_v2_touch_controller_affordance_route(
        1,
        CSB_V2_AFFORDANCE_TOUCH_SWIPE_UP);
    CHECK(runtime.lastCommand == 0);
    CHECK(dm1_v2_runtime_is_running(&runtime) == 1);

    (void)csb_v2_touch_controller_affordance_route(
        1,
        CSB_V2_AFFORDANCE_CONTROLLER_LEFT_BUMPER);
    CHECK(runtime.lastCommand == 0);

    (void)csb_v2_touch_controller_affordance_route(0, CSB_V2_AFFORDANCE_NONE);
    CHECK(runtime.lastCommand == 0);
}

static void test_runtime_apply_flow_from_affordance_route(void) {
    /* End-to-end: affordance → movement command → runtime apply.
     * This tests the full CSB V2 touch/controller → runtime pipeline
     * without game data.  Mirrors test_v2_affordance_runtime_bridge in
     * test_dm1_v2_touch_controller_affordance_pc34.c. */
    DM1_V2_RuntimeState runtime;
    DM1_V2_CameraController camera;
    CSB_V2_TouchControllerAffordanceRoute route;
    DM1_V2_MovementCommandResult result;
    size_t i;

    dm1_v2_runtime_init(&runtime);
    dm1_v2_runtime_start(&runtime, 1000U);
    dm1_v2_camera_init(&camera, &runtime.player);

    /* Test a representative subset of affordances through full apply path */
    const CSBAffordanceCase kSubset[] = {
        kMovementCases[0],   /* touch_swipe_up */
        kMovementCases[2],   /* touch_swipe_left */
        kMovementCases[6],   /* controller_dpad_up */
        kMovementCases[14],  /* controller_right_stick_left */
    };

    for (i = 0; i < sizeof(kSubset) / sizeof(kSubset[0]); ++i) {
        route = csb_v2_touch_controller_affordance_route(1, kSubset[i].affordance);
        CHECK(route.accepted == 1);
        CHECK(route.v2Only == 1);
        CHECK(route.route.sourceCommand == kSubset[i].sourceCommand);
        CHECK(route.route.runtimeCommand == kSubset[i].runtimeCommand);

        result = dm1_v2_movement_command_apply(
            &runtime,
            &camera,
            route.movementCommand,
            1000U,
            64);

        CHECK(result.accepted == 1);
        CHECK(result.sourceCommand == route.route.sourceCommand);
        CHECK(result.runtimeCommand == route.route.runtimeCommand);
        CHECK(runtime.lastCommand == route.route.runtimeCommand);
    }

    CHECK(dm1_v2_camera_is_active(&camera));
}

static void test_phase_gate_domain_name_coverage(void) {
    /* Verify domain name strings are stable and cover INPUT_PRESENTATION. */
    CHECK(strcmp(csb_v2_phase_gate_pc34_domain_name(
                     CSB_V2_PHASE_DOMAIN_INPUT_PRESENTATION),
                 "INPUT_PRESENTATION") == 0);
    CHECK(strcmp(csb_v2_phase_gate_pc34_domain_name(
                     CSB_V2_PHASE_DOMAIN_COMMAND_SEMANTICS),
                 "COMMAND_SEMANTICS") == 0);
    CHECK(strcmp(csb_v2_phase_gate_pc34_domain_name(
                     CSB_V2_PHASE_DOMAIN_CHAOS_MAGIC_SCRIPTS),
                 "CHAOS_MAGIC_SCRIPTS") == 0);
    CHECK(strcmp(csb_v2_phase_gate_pc34_domain_name(
                     CSB_V2_PHASE_DOMAIN_RENDER_PRESENTATION),
                 "RENDER_PRESENTATION") == 0);
}

static void test_evidence_string_contains_csb_specific_references(void) {
    /* Source evidence must cover both ReDMCSB movement zones AND
     * CSB-specific references (DSA/chaos magic, champion cycle, bumpers). */
    const char *evidence = csb_v2_touch_controller_affordance_source_evidence();

    CHECK(strstr(evidence, "COMMAND.C:108-113") != NULL);
    CHECK(strstr(evidence, "COMMAND.C:254-291") != NULL);
    CHECK(strstr(evidence, "F0358") != NULL);
    CHECK(strstr(evidence, "F0359") != NULL);
    CHECK(strstr(evidence, "F0360") != NULL);
    CHECK(strstr(evidence, "CLIKMENU.C:142") != NULL);
    CHECK(strstr(evidence, "CLIKMENU.C:180") != NULL);
    CHECK(strstr(evidence, "GAMELOOP.C:164-219") != NULL);
    CHECK(strstr(evidence, "DSA") != NULL);
    CHECK(strstr(evidence, "csb_v1_chaos_trigger") != NULL);
}

int main(void) {
    test_movement_affordances_route_to_source_commands();
    test_v1_off_rejects_v2_affordances();
    test_csb_shoulder_buttons_are_actions_not_movement();
    test_invalid_affordance_and_evidence();
    test_phase_gate_input_presentation_domain();
    test_route_v2_off_affordance_rejected_via_phase_gate();
    test_route_v2_on_affordance_accepted_via_phase_gate();
    test_route_probe_does_not_mutate_runtime_state();
    test_runtime_apply_flow_from_affordance_route();
    test_phase_gate_domain_name_coverage();
    test_evidence_string_contains_csb_specific_references();

    if (failures) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    puts("csb_v2_touch_controller_affordance: ok");
    return 0;
}
#include "dm1_v2_phase_gate_pc34.h"
#include "dm1_v2_movement_command_adapter_pc34.h"
#include "dm1_v2_settings_pc34.h"

#include <stdio.h>

static int failures = 0;
#define CHECK(expr) do { if (!(expr)) { fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); failures++; } } while (0)

static void check_default_v1_locks(void) {
    DM1_V2_PhaseGateConfig config;
    DM1_V2_PhaseGateDecision d;
    dm1_v2_phase_gate_defaults(&config);

    CHECK(config.v2PresentationEnabled == 0);
    CHECK(config.v2ConfigPersistenceEnabled == 0);
    d = dm1_v2_phase_gate_decide(&config, DM1_V2_PHASE_DOMAIN_COMMAND_SEMANTICS);
    CHECK(d.v1SourceLocked == 1);
    CHECK(d.v2PresentationAllowed == 0);
    d = dm1_v2_phase_gate_decide(&config, DM1_V2_PHASE_DOMAIN_DUNGEON_TIMING);
    CHECK(d.v1SourceLocked == 1);
    CHECK(d.v2PresentationAllowed == 0);
    d = dm1_v2_phase_gate_decide(&config, DM1_V2_PHASE_DOMAIN_COLLISION_RULES);
    CHECK(d.v1SourceLocked == 1);
    CHECK(d.v2PresentationAllowed == 0);
    d = dm1_v2_phase_gate_decide(&config, DM1_V2_PHASE_DOMAIN_SAVE_LOAD_DATA);
    CHECK(d.v1SourceLocked == 1);
    CHECK(d.v2PresentationAllowed == 0);
    d = dm1_v2_phase_gate_decide(&config, DM1_V2_PHASE_DOMAIN_SOURCE_LOCKED_RULES);
    CHECK(d.v1SourceLocked == 1);
    CHECK(d.v2PresentationAllowed == 0);
}

static void check_presentation_toggle_scope(void) {
    DM1_V2_PhaseGateConfig config;
    DM1_V2_PhaseGateDecision d;
    dm1_v2_phase_gate_defaults(&config);
    config.v2PresentationEnabled = 1;

    d = dm1_v2_phase_gate_decide(&config, DM1_V2_PHASE_DOMAIN_RENDER_PRESENTATION);
    CHECK(d.v1SourceLocked == 0);
    CHECK(d.v2PresentationAllowed == 1);
    d = dm1_v2_phase_gate_decide(&config, DM1_V2_PHASE_DOMAIN_INPUT_PRESENTATION);
    CHECK(d.v1SourceLocked == 0);
    CHECK(d.v2PresentationAllowed == 1);
    d = dm1_v2_phase_gate_decide(&config, DM1_V2_PHASE_DOMAIN_CONFIG_PRESENTATION);
    CHECK(d.v1SourceLocked == 0);
    CHECK(d.v2PresentationAllowed == 0);
    config.v2ConfigPersistenceEnabled = 1;
    d = dm1_v2_phase_gate_decide(&config, DM1_V2_PHASE_DOMAIN_CONFIG_PRESENTATION);
    CHECK(d.v1SourceLocked == 0);
    CHECK(d.v2PresentationAllowed == 1);

    d = dm1_v2_phase_gate_decide(&config, DM1_V2_PHASE_DOMAIN_COMMAND_SEMANTICS);
    CHECK(d.v1SourceLocked == 1);
    CHECK(d.v2PresentationAllowed == 0);
    d = dm1_v2_phase_gate_decide(&config, DM1_V2_PHASE_DOMAIN_COLLISION_RULES);
    CHECK(d.v1SourceLocked == 1);
    CHECK(d.v2PresentationAllowed == 0);
}

static void check_existing_scaffolds_obey_gate(void) {
    DM1_V2_Settings settings;
    DM1_V2_MovementCommandRoute v1Route;
    DM1_V2_MovementCommandRoute v2Route;

    dm1_v2_settings_defaults(&settings);
    CHECK(settings.aspectMode == DM1_V2_ASPECT_ORIGINAL_4_3);
    CHECK(settings.viewport_scale == 2);
    CHECK(settings.use_epx == 1);
    CHECK(settings.music_enabled == 0);

    v1Route = dm1_v2_movement_command_route_for_presentation(
        0, DM1_V2_MOVEMENT_COMMAND_MOVE_FORWARD);
    v2Route = dm1_v2_movement_command_route_for_presentation(
        1, DM1_V2_MOVEMENT_COMMAND_MOVE_FORWARD);
    CHECK(v1Route.routeKind == DM1_V2_MOVEMENT_ROUTE_V1_SOURCE);
    CHECK(v1Route.sourceCommand == 3);
    CHECK(v1Route.runtimeCommand == 3);
    CHECK(v2Route.routeKind == DM1_V2_MOVEMENT_ROUTE_V2_PRESENTATION);
    CHECK(v2Route.sourceCommand == 3);
    CHECK(v2Route.runtimeCommand == 1);
}

int main(void) {
    check_default_v1_locks();
    check_presentation_toggle_scope();
    check_existing_scaffolds_obey_gate();
    if (failures) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    puts("dm1_v2_phase_gate_pc34: ok");
    return 0;
}

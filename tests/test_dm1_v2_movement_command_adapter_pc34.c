#include "dm1_v2_movement_command_adapter_pc34.h"

#include <stdio.h>
#include <stdlib.h>

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "CHECK failed: %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        return 1; \
    } \
} while (0)

static int test_source_ids_translate_to_runtime_ids(void) {
    DM1_V2_RuntimeState rt;
    DM1_V2_CameraController camera;
    DM1_V2_MovementCommandResult result;

    dm1_v2_runtime_init(&rt);
    dm1_v2_runtime_start(&rt, 1000);
    dm1_v2_camera_init(&camera, &rt.player);

    /* Source-lock: COMMAND.C dispatches C003 move-forward to movement and V2
     * maps that semantic command to the existing runtime forward command. */
    result = dm1_v2_movement_command_apply(&rt, &camera, DM1_V2_MOVEMENT_COMMAND_MOVE_FORWARD, 1000, 64);
    CHECK(result.accepted == 1);
    CHECK(result.sourceCommand == 3);
    CHECK(result.runtimeCommand == 1);
    CHECK(rt.lastCommand == 1);
    CHECK(dm1_v2_get_x(&rt.player) == 1);
    CHECK(dm1_v2_get_y(&rt.player) == 0);
    CHECK(camera.logicalX == dm1_v2_get_x(&rt.player));
    CHECK(camera.logicalY == dm1_v2_get_y(&rt.player));

    rt.player.moveState = 0;
    result = dm1_v2_movement_command_apply(&rt, &camera, DM1_V2_MOVEMENT_COMMAND_MOVE_BACKWARD, 1000, 64);
    CHECK(result.accepted == 1);
    CHECK(result.sourceCommand == 5);
    CHECK(result.runtimeCommand == 2);
    CHECK(rt.lastCommand == 2);
    CHECK(dm1_v2_get_x(&rt.player) == 0);
    CHECK(dm1_v2_get_y(&rt.player) == 0);
    CHECK(camera.logicalX == dm1_v2_get_x(&rt.player));
    CHECK(camera.logicalY == dm1_v2_get_y(&rt.player));

    return 0;
}

static int test_turns_start_presentation_only_camera_turns(void) {
    DM1_V2_RuntimeState rt;
    DM1_V2_CameraController camera;
    DM1_V2_MovementCommandResult result;

    dm1_v2_runtime_init(&rt);
    dm1_v2_runtime_start(&rt, 1000);
    dm1_v2_camera_init(&camera, &rt.player);

    /* Source-lock: COMMAND.C dispatches C001/C002 as turns; adapter preserves
     * those ids while runtime/camera use their own presentation split. */
    result = dm1_v2_movement_command_apply(&rt, &camera, DM1_V2_MOVEMENT_COMMAND_TURN_LEFT, 1100, 80);
    CHECK(result.accepted == 1);
    CHECK(result.sourceCommand == 1);
    CHECK(result.runtimeCommand == 3);
    CHECK(result.fromFacingDir == 0);
    CHECK(result.targetFacingDir == 7);
    CHECK(rt.player.facingDir == 7);
    CHECK(camera.turning == 1);
    CHECK(dm1_v2_camera_is_active(&camera));
    CHECK(camera.fromFacingDir == 0);
    CHECK(camera.targetFacingDir == 7);

    result = dm1_v2_movement_command_apply(&rt, &camera, DM1_V2_MOVEMENT_COMMAND_TURN_RIGHT, 1200, 80);
    CHECK(result.accepted == 1);
    CHECK(result.sourceCommand == 2);
    CHECK(result.runtimeCommand == 4);
    CHECK(result.fromFacingDir == 7);
    CHECK(result.targetFacingDir == 0);
    CHECK(rt.player.facingDir == 0);
    CHECK(camera.turning == 1);
    CHECK(camera.fromFacingDir == 7);
    CHECK(camera.targetFacingDir == 0);

    return 0;
}


static int test_source_strafes_translate_to_runtime_ids(void) {
    DM1_V2_RuntimeState rt;
    DM1_V2_CameraController camera;
    DM1_V2_MovementCommandResult result;

    dm1_v2_runtime_init(&rt);
    dm1_v2_runtime_start(&rt, 1000);
    dm1_v2_camera_init(&camera, &rt.player);

    /* Source-lock: CLIKMENU.C:224-233 maps C004/C006 to right-step counts;
     * DUNGEON.C:1389-1391 applies right/left movement without changing facing. */
    result = dm1_v2_movement_command_apply(&rt, &camera, DM1_V2_MOVEMENT_COMMAND_MOVE_RIGHT, 1000, 40);
    CHECK(result.accepted == 1);
    CHECK(result.sourceCommand == 4);
    CHECK(result.runtimeCommand == 5);
    CHECK(rt.lastCommand == 5);
    CHECK(rt.player.facingDir == 0);
    CHECK(dm1_v2_get_x(&rt.player) == 0);
    CHECK(dm1_v2_get_y(&rt.player) == 1);
    CHECK(camera.logicalX == dm1_v2_get_x(&rt.player));
    CHECK(camera.logicalY == dm1_v2_get_y(&rt.player));

    rt.player.moveState = 0;
    result = dm1_v2_movement_command_apply(&rt, &camera, DM1_V2_MOVEMENT_COMMAND_MOVE_LEFT, 1000, 40);
    CHECK(result.accepted == 1);
    CHECK(result.sourceCommand == 6);
    CHECK(result.runtimeCommand == 6);
    CHECK(rt.lastCommand == 6);
    CHECK(rt.player.facingDir == 0);
    CHECK(dm1_v2_get_x(&rt.player) == 0);
    CHECK(dm1_v2_get_y(&rt.player) == 0);

    return 0;
}

static int test_v1_presentation_route_keeps_source_command_ids(void) {
    const struct {
        DM1_V2_MovementCommand command;
        int sourceCommand;
        int v2RuntimeCommand;
    } cases[] = {
        {DM1_V2_MOVEMENT_COMMAND_TURN_LEFT, 1, 3},
        {DM1_V2_MOVEMENT_COMMAND_TURN_RIGHT, 2, 4},
        {DM1_V2_MOVEMENT_COMMAND_MOVE_FORWARD, 3, 1},
        {DM1_V2_MOVEMENT_COMMAND_MOVE_RIGHT, 4, 5},
        {DM1_V2_MOVEMENT_COMMAND_MOVE_BACKWARD, 5, 2},
        {DM1_V2_MOVEMENT_COMMAND_MOVE_LEFT, 6, 6},
    };
    size_t i;

    /* Phase 0 gate: DEFS.H:238-243 owns the V1 command ids and
     * COMMAND.C:2045-2155 consumes those ids from the source queue.  V2
     * presentation routing is explicit; when the toggle is off, the route
     * leaves the source id untouched for the V1 command path. */
    for (i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        DM1_V2_MovementCommandRoute v1Route =
            dm1_v2_movement_command_route_for_presentation(0, cases[i].command);
        DM1_V2_MovementCommandRoute v2Route =
            dm1_v2_movement_command_route_for_presentation(1, cases[i].command);

        CHECK(v1Route.routeKind == DM1_V2_MOVEMENT_ROUTE_V1_SOURCE);
        CHECK(v1Route.v2PresentationEnabled == 0);
        CHECK(v1Route.sourceCommand == cases[i].sourceCommand);
        CHECK(v1Route.runtimeCommand == cases[i].sourceCommand);

        CHECK(v2Route.routeKind == DM1_V2_MOVEMENT_ROUTE_V2_PRESENTATION);
        CHECK(v2Route.v2PresentationEnabled == 1);
        CHECK(v2Route.sourceCommand == cases[i].sourceCommand);
        CHECK(v2Route.runtimeCommand == cases[i].v2RuntimeCommand);
    }

    return 0;
}

static int test_rejects_stopped_commands(void) {
    DM1_V2_RuntimeState rt;
    DM1_V2_CameraController camera;
    DM1_V2_MovementCommandResult result;

    dm1_v2_runtime_init(&rt);
    dm1_v2_runtime_start(&rt, 1000);
    dm1_v2_camera_init(&camera, &rt.player);

    dm1_v2_runtime_stop(&rt);
    result = dm1_v2_movement_command_apply(&rt, &camera, DM1_V2_MOVEMENT_COMMAND_MOVE_FORWARD, 1500, 40);
    CHECK(result.accepted == 0);
    CHECK(result.runtimeCommand == 1);

    return 0;
}

int main(void) {
    if (test_source_ids_translate_to_runtime_ids()) return 1;
    if (test_turns_start_presentation_only_camera_turns()) return 1;
    if (test_source_strafes_translate_to_runtime_ids()) return 1;
    if (test_v1_presentation_route_keeps_source_command_ids()) return 1;
    if (test_rejects_stopped_commands()) return 1;
    puts("dm1_v2_movement_command_adapter_pc34: ok");
    return 0;
}

#include "csb_v1_command_input_geometry_pc34_compat.h"
#include "dm1_v1_mouse_routes_pc34_compat.h"
#include "firestaff/dm1/v1/box_movement_arrows_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;

#define CHECK(condition, message) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAIL: %s\n", message); \
        failures++; \
    } \
} while (0)

static void check_route(int arrow_index, int command, M12_MenuInput input)
{
    CSB_V1_CommandInputGeometryResultPc34Compat result;
    DM1_V1_MovementArrowRectPc34 rect;
    memset(&result, 0, sizeof(result));
    CHECK(dm1_v1_movement_arrow_rect_pc34(arrow_index, &rect) == 1,
          "source movement-arrow rectangle is available");
    CHECK(CSB_V1_CommandInputGeometryFromPointerPc34Compat(
              rect.x + rect.w / 2, rect.y + rect.h / 2,
              DM1_V1_MOUSE_MASK_LEFT_PC34, &result) == 1,
          "source movement zone is accepted");
    CHECK(result.matched == 1, "accepted route is marked matched");
    CHECK(result.command == command, "source command id is preserved");
    CHECK(result.input == input, "source command reaches its shared input token");
    CHECK(result.zone_id >= 68 && result.zone_id <= 73,
          "route exposes a real G0448 movement zone");
}

static void init_runtime(CSB_V1_RuntimeProfile* profile)
{
    CSB_V1_PartyState party;
    int i;

    csb_v1_runtime_init(profile, NULL);
    csb_v1_character_init_default(&party);
    party.ChampionCount = 2;
    party.PartyDirection = CSB_V1_DIR_NORTH;
    party.LeaderIndex = 0;
    party.MagicCasterIndex = -1;
    party.PartyMapX = CSB_V1_START_PARTY_X;
    party.PartyMapY = CSB_V1_START_PARTY_Y;
    for (i = 0; i < party.ChampionCount; ++i) {
        party.Champions[i].CurrentHealth = 100;
        party.Champions[i].MaximumHealth = 100;
        party.Champions[i].Cell = (uint8_t)i;
        party.Champions[i].Direction = CSB_V1_DIR_NORTH;
    }
    CHECK(csb_v1_runtime_set_party_state(profile, &party) == 0,
          "CSB source runtime accepts a live party");
}

static void test_pointer_handoff(void)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_CommandInputGeometryResultPc34Compat geometry;
    CSB_V1_InputCommandBridgeResult bridge;
    DM1_V1_MovementArrowRectPc34 rect;

    init_runtime(&profile);
    CHECK(dm1_v1_movement_arrow_rect_pc34(
              DM1_V1_MOVEMENT_ARROW_INDEX_TURN_RIGHT_PC34, &rect) == 1,
          "turn-right source rectangle is available for runtime handoff");
    memset(&geometry, 0, sizeof(geometry));
    memset(&bridge, 0, sizeof(bridge));
    CHECK(CSB_V1_CommandInputGeometryProcessPointerPc34Compat(
              &profile,
              rect.x + rect.w / 2,
              rect.y + rect.h / 2,
              DM1_V1_MOUSE_MASK_LEFT_PC34,
              0, 0, 0,
              &geometry,
              &bridge) == 1,
          "G0448 turn-right click reaches one F0380 dispatch");
    CHECK(geometry.command == 2 && geometry.input == M12_MENU_INPUT_TURN_RIGHT,
          "G0448 click retains its C002 command identity");
    CHECK(bridge.event.keyCode == CSB_V1_BRIDGE_PC34_KEY_TURN_R,
          "C002 pointer route uses the original keyboard queue scancode");
    CHECK(bridge.queue_result.command == DM1_V1_COMMAND_TURN_RIGHT &&
              bridge.runtime_state_changed == 1,
          "C002 pointer route mutates the CSB runtime through F0380");
    CHECK(profile.party_dir == CSB_V1_DIR_EAST,
          "C002 pointer route rotates the live party north to east");

    memset(&geometry, 0xff, sizeof(geometry));
    memset(&bridge, 0xff, sizeof(bridge));
    CHECK(CSB_V1_CommandInputGeometryProcessPointerPc34Compat(
              &profile, 0, 0, DM1_V1_MOUSE_MASK_LEFT_PC34,
              0, 0, 0, &geometry, &bridge) == 0,
          "non-G0448 click does not enter the CSB command queue");
    CHECK(geometry.matched == 0 && bridge.mapped == 0,
          "unclaimed pointer keeps both source routes empty");
}

int main(void)
{
    CSB_V1_CommandInputGeometryResultPc34Compat result;
    const char* evidence = CSB_V1_CommandInputGeometrySourceEvidencePc34Compat();

    CHECK(evidence != NULL && strstr(evidence, "G0448") != NULL,
          "evidence identifies the original movement surface");
    CHECK(evidence != NULL && strstr(evidence, "F0358") != NULL,
          "evidence identifies the original mouse command lookup");

    /* Centers come from the real G0448 C068..C073 source rectangles. */
    check_route(DM1_V1_MOVEMENT_ARROW_INDEX_TURN_LEFT_PC34,
                1, M12_MENU_INPUT_TURN_LEFT);
    check_route(DM1_V1_MOVEMENT_ARROW_INDEX_TURN_RIGHT_PC34,
                2, M12_MENU_INPUT_TURN_RIGHT);
    check_route(DM1_V1_MOVEMENT_ARROW_INDEX_FORWARD_PC34,
                3, M12_MENU_INPUT_UP);
    check_route(DM1_V1_MOVEMENT_ARROW_INDEX_RIGHT_PC34,
                4, M12_MENU_INPUT_STRAFE_RIGHT);
    check_route(DM1_V1_MOVEMENT_ARROW_INDEX_BACKWARD_PC34,
                5, M12_MENU_INPUT_DOWN);
    check_route(DM1_V1_MOVEMENT_ARROW_INDEX_LEFT_PC34,
                6, M12_MENU_INPUT_STRAFE_LEFT);

    memset(&result, 0xff, sizeof(result));
    CHECK(CSB_V1_CommandInputGeometryFromPointerPc34Compat(
              276, 125, DM1_V1_MOUSE_MASK_RIGHT_PC34, &result) == 0,
          "right click does not synthesize a movement command");
    CHECK(result.matched == 0 && result.input == M12_MENU_INPUT_NONE,
          "unclaimed button leaves no CSB command behind");
    CHECK(CSB_V1_CommandInputGeometryFromPointerPc34Compat(
              0, 0, DM1_V1_MOUSE_MASK_LEFT_PC34, &result) == 0,
          "outside source geometry is not a synthetic click zone");
    CHECK(CSB_V1_CommandInputGeometryFromPointerPc34Compat(
              224, 124, DM1_V1_MOUSE_MASK_LEFT_PC34, NULL) == 0,
          "NULL result is rejected safely");

    test_pointer_handoff();

    if (failures != 0) {
        fprintf(stderr, "csb command input geometry: %d failures\n", failures);
        return 1;
    }
    puts("ok: csb command input geometry uses source G0448 surfaces");
    return 0;
}

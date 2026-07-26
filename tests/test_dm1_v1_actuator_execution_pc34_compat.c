/*
 * Tests for DM1 V1 Actuator Execution — pc34 compat layer.
 *
 * Verifies that SensorActuatorDispatch_Compat is correctly consumed to
 * mutate dungeon square bytes for doors, pits, and fakewalls.
 */

#include "dm1_v1_actuator_execution_pc34_compat.h"
#include "dm1_v1_sensor_trigger_pc34_compat.h"
#include "dm1_v1_collision_door_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

/* Build a 4x4 map tile array with a given square at (x,y). */
static void setup_tiles(struct DungeonMapTiles_Compat* tiles,
                        unsigned char* data, int w, int h,
                        int x, int y, unsigned char sq)
{
    memset(data, 0, (size_t)(w * h));
    tiles->squareData = data;
    tiles->squareCount = w * h;
    data[x * h + y] = sq;
}

static struct SensorActuatorDispatch_Compat make_dispatch(
    int targetX, int targetY, int kindMask, int action)
{
    struct SensorActuatorDispatch_Compat d;
    memset(&d, 0, sizeof(d));
    d.valid = 1;
    d.targetMapX = targetX;
    d.targetMapY = targetY;
    d.actuatorKindMask = kindMask;
    d.action = action;
    return d;
}

/* ---- Door tests ---- */

static void test_door_open(void)
{
    unsigned char data[16];
    struct DungeonMapTiles_Compat tiles;
    unsigned char doorSquare = (unsigned char)((DUNGEON_ELEMENT_DOOR << 5) | DM1_DOOR_STATE_CLOSED);
    setup_tiles(&tiles, data, 4, 4, 2, 1, doorSquare);

    struct SensorActuatorDispatch_Compat dispatch =
        make_dispatch(2, 1, DM1_ACTUATOR_KIND_DOOR, DM1_ACTUATOR_ACTION_OPEN);

    struct Dm1V1ActuatorResult result;
    int ok = dm1_v1_actuator_execute_dispatch_pc34(&dispatch, &tiles, 4, 4, &result);

    assert(ok == 1);
    assert(result.applied == 1);
    assert(result.previousState == DM1_DOOR_STATE_CLOSED);
    assert(result.newState == DM1_DOOR_STATE_OPEN);
    assert(result.animationNeeded == 1);
    assert(dm1_v1_actuator_read_door_state(data[2 * 4 + 1]) == DM1_DOOR_STATE_OPEN);
    printf("  PASS test_door_open\n");
}

static void test_door_close(void)
{
    unsigned char data[16];
    struct DungeonMapTiles_Compat tiles;
    unsigned char doorSquare = (unsigned char)((DUNGEON_ELEMENT_DOOR << 5) | DM1_DOOR_STATE_OPEN);
    setup_tiles(&tiles, data, 4, 4, 1, 1, doorSquare);

    struct SensorActuatorDispatch_Compat dispatch =
        make_dispatch(1, 1, DM1_ACTUATOR_KIND_DOOR, DM1_ACTUATOR_ACTION_CLOSE);

    struct Dm1V1ActuatorResult result;
    int ok = dm1_v1_actuator_execute_dispatch_pc34(&dispatch, &tiles, 4, 4, &result);

    assert(ok == 1);
    assert(result.newState == DM1_DOOR_STATE_CLOSED);
    assert(result.animationNeeded == 1);
    printf("  PASS test_door_close\n");
}

static void test_door_toggle_from_closed(void)
{
    unsigned char data[16];
    struct DungeonMapTiles_Compat tiles;
    unsigned char doorSquare = (unsigned char)((DUNGEON_ELEMENT_DOOR << 5) | DM1_DOOR_STATE_CLOSED);
    setup_tiles(&tiles, data, 4, 4, 0, 0, doorSquare);

    struct SensorActuatorDispatch_Compat dispatch =
        make_dispatch(0, 0, DM1_ACTUATOR_KIND_DOOR, DM1_ACTUATOR_ACTION_TOGGLE);

    struct Dm1V1ActuatorResult result;
    dm1_v1_actuator_execute_dispatch_pc34(&dispatch, &tiles, 4, 4, &result);

    assert(result.newState == DM1_DOOR_STATE_OPEN);
    printf("  PASS test_door_toggle_from_closed\n");
}

static void test_door_toggle_from_open(void)
{
    unsigned char data[16];
    struct DungeonMapTiles_Compat tiles;
    unsigned char doorSquare = (unsigned char)((DUNGEON_ELEMENT_DOOR << 5) | DM1_DOOR_STATE_OPEN);
    setup_tiles(&tiles, data, 4, 4, 0, 0, doorSquare);

    struct SensorActuatorDispatch_Compat dispatch =
        make_dispatch(0, 0, DM1_ACTUATOR_KIND_DOOR, DM1_ACTUATOR_ACTION_TOGGLE);

    struct Dm1V1ActuatorResult result;
    dm1_v1_actuator_execute_dispatch_pc34(&dispatch, &tiles, 4, 4, &result);

    assert(result.newState == DM1_DOOR_STATE_CLOSED);
    printf("  PASS test_door_toggle_from_open\n");
}

static void test_door_no_change(void)
{
    unsigned char data[16];
    struct DungeonMapTiles_Compat tiles;
    unsigned char doorSquare = (unsigned char)((DUNGEON_ELEMENT_DOOR << 5) | DM1_DOOR_STATE_OPEN);
    setup_tiles(&tiles, data, 4, 4, 0, 0, doorSquare);

    struct SensorActuatorDispatch_Compat dispatch =
        make_dispatch(0, 0, DM1_ACTUATOR_KIND_DOOR, DM1_ACTUATOR_ACTION_OPEN);

    struct Dm1V1ActuatorResult result;
    dm1_v1_actuator_execute_dispatch_pc34(&dispatch, &tiles, 4, 4, &result);

    assert(result.animationNeeded == 0);
    printf("  PASS test_door_no_change\n");
}

/* ---- Door animation step tests ---- */

static void test_door_animation_step_opening(void)
{
    unsigned char data[16];
    struct DungeonMapTiles_Compat tiles;
    unsigned char doorSquare = (unsigned char)((DUNGEON_ELEMENT_DOOR << 5) | DM1_DOOR_STATE_CLOSED);
    setup_tiles(&tiles, data, 4, 4, 1, 1, doorSquare);

    int more = dm1_v1_actuator_door_animation_step_pc34(&tiles, 4, 4, 1, 1, DM1_DOOR_STATE_OPEN);
    assert(more == 1);
    assert(dm1_v1_actuator_read_door_state(data[1 * 4 + 1]) == DM1_DOOR_STATE_CLOSED_THREE_FOURTH);

    more = dm1_v1_actuator_door_animation_step_pc34(&tiles, 4, 4, 1, 1, DM1_DOOR_STATE_OPEN);
    assert(more == 1);
    assert(dm1_v1_actuator_read_door_state(data[1 * 4 + 1]) == DM1_DOOR_STATE_CLOSED_HALF);

    more = dm1_v1_actuator_door_animation_step_pc34(&tiles, 4, 4, 1, 1, DM1_DOOR_STATE_OPEN);
    assert(more == 1);
    assert(dm1_v1_actuator_read_door_state(data[1 * 4 + 1]) == DM1_DOOR_STATE_CLOSED_ONE_FOURTH);

    more = dm1_v1_actuator_door_animation_step_pc34(&tiles, 4, 4, 1, 1, DM1_DOOR_STATE_OPEN);
    assert(more == 0);
    assert(dm1_v1_actuator_read_door_state(data[1 * 4 + 1]) == DM1_DOOR_STATE_OPEN);

    printf("  PASS test_door_animation_step_opening\n");
}

static void test_door_animation_step_closing(void)
{
    unsigned char data[16];
    struct DungeonMapTiles_Compat tiles;
    unsigned char doorSquare = (unsigned char)((DUNGEON_ELEMENT_DOOR << 5) | DM1_DOOR_STATE_OPEN);
    setup_tiles(&tiles, data, 4, 4, 1, 1, doorSquare);

    int more = dm1_v1_actuator_door_animation_step_pc34(&tiles, 4, 4, 1, 1, DM1_DOOR_STATE_CLOSED);
    assert(more == 1);
    assert(dm1_v1_actuator_read_door_state(data[1 * 4 + 1]) == DM1_DOOR_STATE_CLOSED_ONE_FOURTH);

    more = dm1_v1_actuator_door_animation_step_pc34(&tiles, 4, 4, 1, 1, DM1_DOOR_STATE_CLOSED);
    assert(more == 1);

    more = dm1_v1_actuator_door_animation_step_pc34(&tiles, 4, 4, 1, 1, DM1_DOOR_STATE_CLOSED);
    assert(more == 1);

    more = dm1_v1_actuator_door_animation_step_pc34(&tiles, 4, 4, 1, 1, DM1_DOOR_STATE_CLOSED);
    assert(more == 0);
    assert(dm1_v1_actuator_read_door_state(data[1 * 4 + 1]) == DM1_DOOR_STATE_CLOSED);

    printf("  PASS test_door_animation_step_closing\n");
}

static void test_door_animation_already_at_target(void)
{
    unsigned char data[16];
    struct DungeonMapTiles_Compat tiles;
    unsigned char doorSquare = (unsigned char)((DUNGEON_ELEMENT_DOOR << 5) | DM1_DOOR_STATE_OPEN);
    setup_tiles(&tiles, data, 4, 4, 0, 0, doorSquare);

    int more = dm1_v1_actuator_door_animation_step_pc34(&tiles, 4, 4, 0, 0, DM1_DOOR_STATE_OPEN);
    assert(more == 0);
    printf("  PASS test_door_animation_already_at_target\n");
}

/* ---- Pit tests ---- */

static void test_pit_open(void)
{
    unsigned char data[16];
    struct DungeonMapTiles_Compat tiles;
    unsigned char pitSquare = (unsigned char)(DUNGEON_ELEMENT_PIT << 5);
    setup_tiles(&tiles, data, 4, 4, 1, 0, pitSquare);

    struct SensorActuatorDispatch_Compat dispatch =
        make_dispatch(1, 0, DM1_ACTUATOR_KIND_PIT, DM1_ACTUATOR_ACTION_OPEN);

    struct Dm1V1ActuatorResult result;
    int ok = dm1_v1_actuator_execute_dispatch_pc34(&dispatch, &tiles, 4, 4, &result);

    assert(ok == 1);
    assert(result.previousState == 0);
    assert(result.newState == 1);
    assert(dm1_v1_actuator_read_pit_open(data[1 * 4 + 0]) == 1);
    printf("  PASS test_pit_open\n");
}

static void test_pit_close(void)
{
    unsigned char data[16];
    struct DungeonMapTiles_Compat tiles;
    unsigned char pitSquare = (unsigned char)((DUNGEON_ELEMENT_PIT << 5) | 0x08);
    setup_tiles(&tiles, data, 4, 4, 1, 0, pitSquare);

    struct SensorActuatorDispatch_Compat dispatch =
        make_dispatch(1, 0, DM1_ACTUATOR_KIND_PIT, DM1_ACTUATOR_ACTION_CLOSE);

    struct Dm1V1ActuatorResult result;
    dm1_v1_actuator_execute_dispatch_pc34(&dispatch, &tiles, 4, 4, &result);

    assert(result.previousState == 1);
    assert(result.newState == 0);
    printf("  PASS test_pit_close\n");
}

static void test_pit_toggle(void)
{
    unsigned char data[16];
    struct DungeonMapTiles_Compat tiles;
    unsigned char pitSquare = (unsigned char)(DUNGEON_ELEMENT_PIT << 5);
    setup_tiles(&tiles, data, 4, 4, 0, 0, pitSquare);

    struct SensorActuatorDispatch_Compat dispatch =
        make_dispatch(0, 0, DM1_ACTUATOR_KIND_PIT, DM1_ACTUATOR_ACTION_TOGGLE);

    struct Dm1V1ActuatorResult result;
    dm1_v1_actuator_execute_dispatch_pc34(&dispatch, &tiles, 4, 4, &result);
    assert(result.newState == 1);

    dm1_v1_actuator_execute_dispatch_pc34(&dispatch, &tiles, 4, 4, &result);
    assert(result.newState == 0);
    printf("  PASS test_pit_toggle\n");
}

/* ---- Fakewall tests ---- */

static void test_fakewall_open(void)
{
    unsigned char data[16];
    struct DungeonMapTiles_Compat tiles;
    unsigned char fwSquare = (unsigned char)(DUNGEON_ELEMENT_FAKEWALL << 5);
    setup_tiles(&tiles, data, 4, 4, 2, 2, fwSquare);

    struct SensorActuatorDispatch_Compat dispatch =
        make_dispatch(2, 2, DM1_ACTUATOR_KIND_FAKEWALL, DM1_ACTUATOR_ACTION_OPEN);

    struct Dm1V1ActuatorResult result;
    int ok = dm1_v1_actuator_execute_dispatch_pc34(&dispatch, &tiles, 4, 4, &result);

    assert(ok == 1);
    assert(result.newState == DUNGEON_ELEMENT_CORRIDOR);
    assert(dm1_v1_actuator_read_element(data[2 * 4 + 2]) == DUNGEON_ELEMENT_CORRIDOR);
    printf("  PASS test_fakewall_open\n");
}

static void test_fakewall_toggle(void)
{
    unsigned char data[16];
    struct DungeonMapTiles_Compat tiles;
    unsigned char fwSquare = (unsigned char)(DUNGEON_ELEMENT_FAKEWALL << 5);
    setup_tiles(&tiles, data, 4, 4, 0, 0, fwSquare);

    struct SensorActuatorDispatch_Compat dispatch =
        make_dispatch(0, 0, DM1_ACTUATOR_KIND_FAKEWALL, DM1_ACTUATOR_ACTION_TOGGLE);

    struct Dm1V1ActuatorResult result;
    dm1_v1_actuator_execute_dispatch_pc34(&dispatch, &tiles, 4, 4, &result);
    assert(result.newState == DUNGEON_ELEMENT_CORRIDOR);

    dm1_v1_actuator_execute_dispatch_pc34(&dispatch, &tiles, 4, 4, &result);
    assert(result.newState == DUNGEON_ELEMENT_FAKEWALL);
    printf("  PASS test_fakewall_toggle\n");
}

/* ---- Safety tests ---- */

static void test_null_args(void)
{
    struct Dm1V1ActuatorResult result;
    int ok;

    ok = dm1_v1_actuator_execute_dispatch_pc34(NULL, NULL, 4, 4, &result);
    assert(ok == 0);

    struct SensorActuatorDispatch_Compat dispatch =
        make_dispatch(0, 0, DM1_ACTUATOR_KIND_DOOR, DM1_ACTUATOR_ACTION_OPEN);
    ok = dm1_v1_actuator_execute_dispatch_pc34(&dispatch, NULL, 4, 4, &result);
    assert(ok == 0);

    printf("  PASS test_null_args\n");
}

static void test_invalid_dispatch(void)
{
    unsigned char data[16];
    struct DungeonMapTiles_Compat tiles;
    setup_tiles(&tiles, data, 4, 4, 0, 0, 0);

    struct SensorActuatorDispatch_Compat dispatch =
        make_dispatch(0, 0, DM1_ACTUATOR_KIND_DOOR, DM1_ACTUATOR_ACTION_OPEN);
    dispatch.valid = 0;

    struct Dm1V1ActuatorResult result;
    int ok = dm1_v1_actuator_execute_dispatch_pc34(&dispatch, &tiles, 4, 4, &result);
    assert(ok == 0);
    printf("  PASS test_invalid_dispatch\n");
}

static void test_out_of_bounds(void)
{
    unsigned char data[16];
    struct DungeonMapTiles_Compat tiles;
    setup_tiles(&tiles, data, 4, 4, 0, 0, 0);

    struct SensorActuatorDispatch_Compat dispatch =
        make_dispatch(10, 10, DM1_ACTUATOR_KIND_DOOR, DM1_ACTUATOR_ACTION_OPEN);

    struct Dm1V1ActuatorResult result;
    int ok = dm1_v1_actuator_execute_dispatch_pc34(&dispatch, &tiles, 4, 4, &result);
    assert(ok == 0);
    printf("  PASS test_out_of_bounds\n");
}

static void test_preserves_other_bits(void)
{
    unsigned char data[16];
    struct DungeonMapTiles_Compat tiles;
    unsigned char doorSquare = (unsigned char)((DUNGEON_ELEMENT_DOOR << 5) |
                                               0x10 | DM1_DOOR_STATE_CLOSED);
    setup_tiles(&tiles, data, 4, 4, 0, 0, doorSquare);

    struct SensorActuatorDispatch_Compat dispatch =
        make_dispatch(0, 0, DM1_ACTUATOR_KIND_DOOR, DM1_ACTUATOR_ACTION_OPEN);

    struct Dm1V1ActuatorResult result;
    dm1_v1_actuator_execute_dispatch_pc34(&dispatch, &tiles, 4, 4, &result);

    unsigned char after = data[0];
    assert((after & 0xF8) == ((DUNGEON_ELEMENT_DOOR << 5) | 0x10));
    assert((after & 0x07) == DM1_DOOR_STATE_OPEN);
    printf("  PASS test_preserves_other_bits\n");
}

static void test_source_evidence(void)
{
    const char* evidence = dm1_v1_actuator_execution_source_evidence_pc34();
    assert(evidence != NULL);
    assert(evidence[0] != '\0');
    printf("  PASS test_source_evidence\n");
}

int main(void)
{
    printf("dm1_v1_actuator_execution_pc34_compat tests:\n");

    test_door_open();
    test_door_close();
    test_door_toggle_from_closed();
    test_door_toggle_from_open();
    test_door_no_change();
    test_door_animation_step_opening();
    test_door_animation_step_closing();
    test_door_animation_already_at_target();
    test_pit_open();
    test_pit_close();
    test_pit_toggle();
    test_fakewall_open();
    test_fakewall_toggle();
    test_null_args();
    test_invalid_dispatch();
    test_out_of_bounds();
    test_preserves_other_bits();
    test_source_evidence();

    printf("All 18 actuator execution tests passed.\n");
    return 0;
}

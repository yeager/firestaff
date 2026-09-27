#include "m11_game_view.h"
#include "dm1_v1_sensor_trigger_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures;

static void expect_i(const char* label, int got, int want) {
    if (got != want) {
        printf("FAIL %s: got %d, want %d\n", label, got, want);
        ++failures;
    }
}

static void make_state(M11_GameViewState* state,
                       struct DungeonDatState_Compat* dungeon,
                       struct DungeonMapDesc_Compat* map,
                       struct DungeonMapTiles_Compat* tiles,
                       struct DungeonThings_Compat* things,
                       unsigned char* squares,
                       unsigned short* sft) {
    memset(state, 0, sizeof(*state));
    memset(dungeon, 0, sizeof(*dungeon));
    memset(map, 0, sizeof(*map));
    memset(tiles, 0, sizeof(*tiles));
    memset(things, 0, sizeof(*things));
    memset(squares, 0, 4);
    memset(sft, 0xff, 4 * sizeof(*sft));

    map->width = 2;
    map->height = 2;
    dungeon->header.mapCount = 1;
    dungeon->maps = map;
    dungeon->tiles = tiles;
    dungeon->tilesLoaded = 1;
    dungeon->loaded = 1;
    tiles[0].squareData = squares;
    tiles[0].squareCount = 4;
    things->loaded = 1;
    things->squareFirstThings = sft;
    things->squareFirstThingCount = 4;

    state->active = 1;
    strcpy(state->sourceId, "dm1");
    state->world.dungeon = dungeon;
    state->world.things = things;
    state->world.party.mapIndex = 0;
    state->world.party.mapX = 1;
    state->world.party.mapY = 1;
    state->world.party.direction = DIR_SOUTH;
}

static DM1_V1_StartupFullGraphicsRuntimeHandoffReceipt_PC34 make_receipt(int fresh) {
    DM1_V1_StartupFullGraphicsRuntimeHandoffReceipt_PC34 receipt;
    memset(&receipt, 0, sizeof(receipt));
    receipt.handled = 1;
    receipt.runtime_first_frame_ready = 1;
    receipt.hoc_runtime_ready = fresh;
    receipt.resumed_runtime_ready = !fresh;
    return receipt;
}

int main(void) {
    M11_GameViewState* state = (M11_GameViewState*)calloc(1, sizeof(*state));
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat map;
    struct DungeonMapTiles_Compat tiles;
    struct DungeonThings_Compat things;
    unsigned char squares[4];
    unsigned short sft[4];
    struct DungeonSensor_Compat sensors[1];
    DM1_V1_StartupFullGraphicsRuntimeHandoffReceipt_PC34 receipt;
    if (!state) return 2;

    make_state(state, &dungeon, &map, &tiles, &things, squares, sft);
    receipt = make_receipt(1);
    expect_i("fresh F0267 placement accepted",
             M11_GameView_ApplyDm1StartupF0267PartyPlacement(state, &receipt), 1);
    expect_i("fresh placement recorded", state->dm1StartupPartyPlacementExecuted, 1);
    expect_i("fresh party remains at resolved tile", state->world.party.mapX, 1);
    expect_i("fresh party map index synchronized", state->world.partyMapIndex, 0);
    expect_i("fresh placement has no fabricated sensor effect",
             state->dm1StartupPartyPlacementSensorEffectCount, 0);
    expect_i("fresh placement is idempotent",
             M11_GameView_ApplyDm1StartupF0267PartyPlacement(state, &receipt), 1);
    expect_i("replay does not place twice", state->dm1StartupPartyPlacementExecuted, 1);

    make_state(state, &dungeon, &map, &tiles, &things, squares, sft);
    receipt = make_receipt(0);
    expect_i("resume handoff accepted",
             M11_GameView_ApplyDm1StartupF0267PartyPlacement(state, &receipt), 1);
    expect_i("resume does not execute F0267", state->dm1StartupPartyPlacementExecuted, 0);
    expect_i("resume preserves loaded coordinates", state->world.party.mapY, 1);

    /* Fresh Atari F0462 enters from the off-square PARTY sentinel. F0276
     * suppresses C003 when there are no champions. See MOVESENS.C:1675-1689. */
    make_state(state, &dungeon, &map, &tiles, &things, squares, sft);
    memset(sensors, 0, sizeof(sensors));
    squares[3] = (unsigned char)((DUNGEON_ELEMENT_CORRIDOR << 5) |
                                 DUNGEON_SQUARE_MASK_THING_LIST);
    sft[0] = (unsigned short)(THING_TYPE_SENSOR << 10);
    sensors[0].sensorType = DM1_SENSOR_FLOOR_PARTY;
    sensors[0].targetMapX = 0;
    sensors[0].targetMapY = 0;
    sensors[0].next = THING_ENDOFLIST;
    things.sensors = sensors;
    things.sensorCount = 1;
    receipt = make_receipt(1);
    expect_i("empty-party startup handoff accepted",
             M11_GameView_ApplyDm1StartupF0267PartyPlacement(state, &receipt), 1);
    expect_i("empty-party C003 plate remains inactive",
             state->dm1StartupPartyPlacementSensorEffectCount, 0);

    make_state(state, &dungeon, &map, &tiles, &things, squares, sft);
    squares[3] = (unsigned char)((DUNGEON_ELEMENT_CORRIDOR << 5) |
                                 DUNGEON_SQUARE_MASK_THING_LIST);
    sft[0] = (unsigned short)(THING_TYPE_SENSOR << 10);
    things.sensors = sensors;
    things.sensorCount = 1;
    state->world.party.championCount = 1;
    receipt = make_receipt(1);
    expect_i("occupied-party startup handoff accepted",
             M11_GameView_ApplyDm1StartupF0267PartyPlacement(state, &receipt), 1);
    expect_i("C003 plate still fires for a nonempty party",
             state->dm1StartupPartyPlacementSensorEffectCount, 1);

    receipt = make_receipt(1);
    receipt.return_to_launcher = 1;
    expect_i("launcher return rejected",
             M11_GameView_ApplyDm1StartupF0267PartyPlacement(state, &receipt), 0);

    free(state);
    if (failures) return 1;
    puts("PASS dm1 startup F0267 party placement handoff");
    return 0;
}

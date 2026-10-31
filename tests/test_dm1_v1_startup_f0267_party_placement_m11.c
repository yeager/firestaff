#include "m11_game_view.h"
#include "dm1_v1_sensor_trigger_pc34_compat.h"
#include "dm1_v1_text_message_pc34_compat.h"

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

static unsigned short make_thing(int type, int index) {
    return (unsigned short)(((type & 0x0f) << 10) | (index & 0x03ff));
}

static unsigned short pack_text3(int a, int b, int c) {
    return (unsigned short)(((a & 31) << 10) | ((b & 31) << 5) | (c & 31));
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
    struct DungeonTextString_Compat textStrings[3];
    unsigned char rawTextStrings[12];
    unsigned short textData[6];
    char printedText[DM1_V1_MESSAGE_AREA_ROW_COUNT *
                     DM1_V1_MESSAGE_MAX_LENGTH + 1];
    int printedTextLength;
    int row;
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

    /* ReDMCSB F0276: a party arrival prints each visible TextString in the
     * destination square's Thing-chain order as a C1 message. */
    make_state(state, &dungeon, &map, &tiles, &things, squares, sft);
    squares[3] = (unsigned char)((DUNGEON_ELEMENT_CORRIDOR << 5) |
                                 DUNGEON_SQUARE_MASK_THING_LIST);
    memset(textStrings, 0, sizeof(textStrings));
    memset(rawTextStrings, 0, sizeof(rawTextStrings));
    textData[0] = pack_text3(14, 13, 4); /* ONE */
    textData[1] = pack_text3(31, 31, 31);
    textData[2] = pack_text3(1, 0, 3); /* BAD: hidden TextString */
    textData[3] = pack_text3(31, 31, 31);
    textData[4] = pack_text3(19, 22, 14); /* TWO */
    textData[5] = pack_text3(31, 31, 31);
    textStrings[0].next = make_thing(THING_TYPE_TEXTSTRING, 1);
    textStrings[0].visible = 1;
    textStrings[0].textDataWordOffset = 0;
    textStrings[1].next = make_thing(THING_TYPE_TEXTSTRING, 2);
    textStrings[1].visible = 0;
    textStrings[1].textDataWordOffset = 2;
    textStrings[2].next = THING_ENDOFLIST;
    textStrings[2].visible = 1;
    textStrings[2].textDataWordOffset = 4;
    rawTextStrings[0] = (unsigned char)(textStrings[0].next & 0xffu);
    rawTextStrings[1] = (unsigned char)(textStrings[0].next >> 8);
    rawTextStrings[4] = (unsigned char)(textStrings[1].next & 0xffu);
    rawTextStrings[5] = (unsigned char)(textStrings[1].next >> 8);
    rawTextStrings[8] = (unsigned char)(THING_ENDOFLIST & 0xffu);
    rawTextStrings[9] = (unsigned char)(THING_ENDOFLIST >> 8);
    sft[3] = make_thing(THING_TYPE_TEXTSTRING, 0);
    things.textStrings = textStrings;
    things.textStringCount = 3;
    things.textData = textData;
    things.textDataWordCount = 6;
    things.rawThingData[THING_TYPE_TEXTSTRING] = rawTextStrings;
    things.thingCounts[THING_TYPE_TEXTSTRING] = 3;
    dm1_v1_text_init(&state->dm1V1TextMessage);
    receipt = make_receipt(1);
    expect_i("text-string startup handoff accepted",
             M11_GameView_ApplyDm1StartupF0267PartyPlacement(state, &receipt), 1);
    printedText[0] = '\0';
    printedTextLength = 0;
    for (row = 0; row < DM1_V1_MESSAGE_AREA_ROW_COUNT; ++row) {
        const DM1_V1_MessageRow* messageRow =
            dm1_v1_text_get_row(&state->dm1V1TextMessage, row);
        const size_t remaining = sizeof(printedText) -
                                 (size_t)printedTextLength - 1;
        const size_t rowLength = strlen(messageRow->text);
        if (rowLength > remaining) break;
        memcpy(printedText + printedTextLength, messageRow->text, rowLength);
        printedTextLength += (int)rowLength;
        printedText[printedTextLength] = '\0';
    }
    {
        const char* firstText = strstr(printedText, "ONE");
        const char* secondText = strstr(printedText, "TWO");
        expect_i("first destination TextString prints", firstText != NULL, 1);
        expect_i("second destination TextString prints", secondText != NULL, 1);
        expect_i("destination TextStrings retain chain order",
                 firstText && secondText && firstText < secondText, 1);
        expect_i("invisible destination TextString stays hidden",
                 strstr(printedText, "BAD") == NULL, 1);
    }

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
    sensors[0].sensorData = 1;
    receipt = make_receipt(1);
    expect_i("occupied-party startup handoff accepted",
             M11_GameView_ApplyDm1StartupF0267PartyPlacement(state, &receipt), 1);
    expect_i("C003 rejects a nonmatching party facing",
             state->dm1StartupPartyPlacementSensorEffectCount, 0);

    make_state(state, &dungeon, &map, &tiles, &things, squares, sft);
    squares[3] = (unsigned char)((DUNGEON_ELEMENT_CORRIDOR << 5) |
                                 DUNGEON_SQUARE_MASK_THING_LIST);
    sft[0] = (unsigned short)(THING_TYPE_SENSOR << 10);
    things.sensors = sensors;
    things.sensorCount = 1;
    state->world.party.championCount = 1;
    sensors[0].sensorData = DIR_SOUTH + 1;
    receipt = make_receipt(1);
    expect_i("matching-party startup handoff accepted",
             M11_GameView_ApplyDm1StartupF0267PartyPlacement(state, &receipt), 1);
    expect_i("C003 plate fires for matching nonempty party",
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

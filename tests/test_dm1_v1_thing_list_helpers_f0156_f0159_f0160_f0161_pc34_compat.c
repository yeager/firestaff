#include "dm1_v1_thing_list_helpers_f0156_f0159_f0160_f0161_pc34_compat.h"

#include <assert.h>
#include <string.h>

#define MAP_W 4
#define MAP_H 5

static uint16_t thing_ref(int type, int index)
{
    return (uint16_t)(((type & 15) << 10) | (index & 0x03ff));
}

static void write_next(uint8_t *record, uint16_t next)
{
    record[0] = (uint8_t)(next & 0xffu);
    record[1] = (uint8_t)((next >> 8) & 0xffu);
}

static void set_square(uint8_t map[MAP_W * MAP_H], int x, int y, uint8_t raw)
{
    map[(x * MAP_H) + y] = raw;
}

static void build_context(
    DM1_V1_DungeonThingListContextF0156F0161Pc34 *context,
    uint8_t map[MAP_W * MAP_H],
    uint16_t columnCounts[MAP_W],
    uint16_t squareFirstThings[6],
    uint8_t weaponRecords[3][4],
    uint8_t sensorRecords[2][8])
{
    memset(context, 0, sizeof(*context));
    memset(map, 0, MAP_W * MAP_H);
    memset(columnCounts, 0, sizeof(uint16_t) * MAP_W);
    memset(squareFirstThings, 0, sizeof(uint16_t) * 6);
    memset(weaponRecords, 0, 3 * 4);
    memset(sensorRecords, 0, 2 * 8);

    set_square(map, 0, 1, DM1_V1_F0160_SQUARE_THING_LIST_MASK_PC34);
    set_square(map, 0, 3, DM1_V1_F0160_SQUARE_THING_LIST_MASK_PC34);
    set_square(map, 2, 0, DM1_V1_F0160_SQUARE_THING_LIST_MASK_PC34);
    set_square(map, 2, 2, DM1_V1_F0160_SQUARE_THING_LIST_MASK_PC34);
    set_square(map, 2, 4, DM1_V1_F0160_SQUARE_THING_LIST_MASK_PC34);

    columnCounts[0] = 0;
    columnCounts[1] = 2;
    columnCounts[2] = 2;
    columnCounts[3] = 5;

    squareFirstThings[0] = thing_ref(5, 0);
    squareFirstThings[1] = thing_ref(5, 2);
    squareFirstThings[2] = thing_ref(3, 1);
    squareFirstThings[3] = thing_ref(5, 1);
    squareFirstThings[4] = thing_ref(3, 0);
    squareFirstThings[5] = DM1_V1_F0156_THING_END_OF_LIST_PC34;

    write_next(weaponRecords[0], thing_ref(5, 1));
    weaponRecords[0][2] = 0xa0;
    weaponRecords[0][3] = 0xa1;
    write_next(weaponRecords[1], DM1_V1_F0156_THING_END_OF_LIST_PC34);
    weaponRecords[1][2] = 0xb0;
    weaponRecords[1][3] = 0xb1;
    write_next(weaponRecords[2], thing_ref(3, 0));
    weaponRecords[2][2] = 0xc0;
    weaponRecords[2][3] = 0xc1;

    write_next(sensorRecords[0], DM1_V1_F0156_THING_END_OF_LIST_PC34);
    sensorRecords[0][2] = 0xd0;
    sensorRecords[0][7] = 0xd7;
    write_next(sensorRecords[1], thing_ref(5, 0));
    sensorRecords[1][2] = 0xe0;
    sensorRecords[1][7] = 0xe7;

    context->columnMajorSquares = map;
    context->width = MAP_W;
    context->height = MAP_H;
    context->columnFirstThingCounts = columnCounts;
    context->columnFirstThingCount = MAP_W;
    context->squareFirstThings = squareFirstThings;
    context->squareFirstThingCount = 6;
    context->thingData[5].records = &weaponRecords[0][0];
    context->thingData[5].recordCount = 3;
    context->thingData[5].recordSize = 4;
    context->thingData[3].records = &sensorRecords[0][0];
    context->thingData[3].recordCount = 2;
    context->thingData[3].recordSize = 8;
}

static void test_source_evidence(void)
{
    const char *evidence =
        DM1_V1_F0156_F0159_F0160_F0161_SourceEvidencePc34();
    (void)evidence;

    assert(evidence != 0);
    assert(strstr(evidence, "F0156_DUNGEON_GetThingData") != 0);
    assert(strstr(evidence, "F0159_DUNGEON_GetNextThing") != 0);
    assert(strstr(evidence, "F0160") != 0);
    assert(strstr(evidence, "F0161") != 0);
}

static void test_thing_data_and_next(void)
{
    DM1_V1_DungeonThingListContextF0156F0161Pc34 context;
    DM1_V1_ThingDataResultF0156Pc34 result;
    uint8_t map[MAP_W * MAP_H];
    uint16_t columnCounts[MAP_W];
    uint16_t squareFirstThings[6];
    uint8_t weaponRecords[3][4];
    uint8_t sensorRecords[2][8];
    const uint8_t *record;
    (void)record;

    build_context(&context, map, columnCounts, squareFirstThings,
                  weaponRecords, sensorRecords);

    assert(DM1_V1_ThingTypeFromThingF0156Pc34(thing_ref(5, 2)) == 5);
    assert(DM1_V1_ThingIndexFromThingF0156Pc34(thing_ref(5, 2)) == 2);
    assert(DM1_V1_ThingTypeFromThingF0156Pc34(
               DM1_V1_F0156_THING_END_OF_LIST_PC34) == -1);

    record = F0156_DUNGEON_GetThingData(&context, thing_ref(5, 2));
    assert(record == &weaponRecords[2][0]);
    assert(record[2] == 0xc0);

    memset(&result, 0x7f, sizeof(result));
    assert(DM1_V1_Dungeon_GetThingDataF0156Pc34Compat(
        &context, thing_ref(3, 1), &result) == 1);
    assert(result.valid == 1);
    assert(result.thingType == 3);
    assert(result.thingIndex == 1);
    assert(result.recordSize == 8);
    assert(result.record == &sensorRecords[1][0]);

    assert(F0159_DUNGEON_GetNextThing(&context, thing_ref(5, 0)) ==
           thing_ref(5, 1));
    assert(F0159_DUNGEON_GetNextThing(&context, thing_ref(5, 2)) ==
           thing_ref(3, 0));
    assert(F0159_DUNGEON_GetNextThing(&context, thing_ref(5, 1)) ==
           DM1_V1_F0156_THING_END_OF_LIST_PC34);
}

static void test_square_first_thing_index_and_head(void)
{
    DM1_V1_DungeonThingListContextF0156F0161Pc34 context;
    uint8_t map[MAP_W * MAP_H];
    uint16_t columnCounts[MAP_W];
    uint16_t squareFirstThings[6];
    uint8_t weaponRecords[3][4];
    uint8_t sensorRecords[2][8];

    build_context(&context, map, columnCounts, squareFirstThings,
                  weaponRecords, sensorRecords);

    assert(F0160_DUNGEON_GetSquareFirstThingIndex(&context, 0, 1) == 0);
    assert(F0160_DUNGEON_GetSquareFirstThingIndex(&context, 0, 3) == 1);
    assert(F0160_DUNGEON_GetSquareFirstThingIndex(&context, 2, 0) == 2);
    assert(F0160_DUNGEON_GetSquareFirstThingIndex(&context, 2, 2) == 3);
    assert(F0160_DUNGEON_GetSquareFirstThingIndex(&context, 2, 4) == 4);

    assert(F0161_DUNGEON_GetSquareFirstThing(&context, 2, 0) ==
           thing_ref(3, 1));
    assert(F0161_DUNGEON_GetSquareFirstThing(&context, 2, 2) ==
           thing_ref(5, 1));
    assert(F0161_DUNGEON_GetSquareFirstThing(&context, 0, 2) ==
           DM1_V1_F0156_THING_END_OF_LIST_PC34);
}

static void test_fail_closed_inputs(void)
{
    DM1_V1_DungeonThingListContextF0156F0161Pc34 context;
    DM1_V1_ThingDataResultF0156Pc34 result;
    uint8_t map[MAP_W * MAP_H];
    uint16_t columnCounts[MAP_W];
    uint16_t squareFirstThings[6];
    uint8_t weaponRecords[3][4];
    uint8_t sensorRecords[2][8];

    build_context(&context, map, columnCounts, squareFirstThings,
                  weaponRecords, sensorRecords);

    assert(F0156_DUNGEON_GetThingData(&context, thing_ref(5, 3)) == 0);
    assert(F0156_DUNGEON_GetThingData(
               &context, DM1_V1_F0156_THING_NONE_PC34) == 0);
    assert(F0159_DUNGEON_GetNextThing(&context, thing_ref(7, 0)) ==
           DM1_V1_F0156_THING_END_OF_LIST_PC34);
    assert(DM1_V1_Dungeon_GetThingDataF0156Pc34Compat(
        &context, thing_ref(5, 0), 0) == 0);

    memset(&result, 0x7f, sizeof(result));
    assert(DM1_V1_Dungeon_GetThingDataF0156Pc34Compat(
        &context, thing_ref(12, 0), &result) == 0);
    assert(result.valid == 0);
    assert(result.thingType == 12);
    assert(result.thingIndex == 0);

    assert(F0160_DUNGEON_GetSquareFirstThingIndex(&context, -1, 0) == -1);
    assert(F0160_DUNGEON_GetSquareFirstThingIndex(&context, 0, -1) == -1);
    assert(F0160_DUNGEON_GetSquareFirstThingIndex(&context, 3, 3) == -1);

    context.squareFirstThingCount = 3;
    assert(F0160_DUNGEON_GetSquareFirstThingIndex(&context, 2, 4) == -1);
    assert(F0161_DUNGEON_GetSquareFirstThing(&context, 2, 4) ==
           DM1_V1_F0156_THING_END_OF_LIST_PC34);
}

int main(void)
{
    test_source_evidence();
    test_thing_data_and_next();
    test_square_first_thing_index_and_head();
    test_fail_closed_inputs();
    return 0;
}

#include "dm1_v1_thing_list_mutation_f0162_f0163_f0164_pc34_compat.h"

#include <assert.h>
#include <string.h>

#define MAP_W 4
#define MAP_H 4

static uint16_t thing_ref(int type, int index)
{
    return (uint16_t)(((type & 15) << 10) | (index & 0x03ff));
}

static __attribute__((unused)) uint16_t next_of(const uint8_t *record)
{
    return (uint16_t)((uint16_t)record[0] | ((uint16_t)record[1] << 8));
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
    DM1_V1_MutableThingListContextF0162F0164Pc34 *context,
    uint8_t map[MAP_W * MAP_H],
    uint16_t columnCounts[MAP_W],
    uint16_t squareFirstThings[8],
    uint8_t groupRecords[2][4],
    uint8_t weaponRecords[4][4],
    uint8_t sensorRecords[2][8])
{
    memset(context, 0, sizeof(*context));
    memset(map, 0, MAP_W * MAP_H);
    memset(columnCounts, 0, sizeof(uint16_t) * MAP_W);
    memset(squareFirstThings, 0, sizeof(uint16_t) * 8);
    memset(groupRecords, 0, 2 * 4);
    memset(weaponRecords, 0, 4 * 4);
    memset(sensorRecords, 0, 2 * 8);

    set_square(map, 0, 1, DM1_V1_F0162_SQUARE_THING_LIST_MASK_PC34);
    set_square(map, 2, 0, DM1_V1_F0162_SQUARE_THING_LIST_MASK_PC34);
    columnCounts[0] = 0;
    columnCounts[1] = 1;
    columnCounts[2] = 1;
    columnCounts[3] = 2;
    squareFirstThings[0] = thing_ref(4, 0);
    squareFirstThings[1] = thing_ref(3, 0);

    write_next(groupRecords[0], thing_ref(3, 0));
    write_next(groupRecords[1], DM1_V1_F0162_THING_END_OF_LIST_PC34);
    write_next(sensorRecords[0], thing_ref(5, 0));
    write_next(sensorRecords[1], DM1_V1_F0162_THING_END_OF_LIST_PC34);
    write_next(weaponRecords[0], thing_ref(5, 1));
    write_next(weaponRecords[1], DM1_V1_F0162_THING_END_OF_LIST_PC34);
    write_next(weaponRecords[2], DM1_V1_F0162_THING_END_OF_LIST_PC34);
    write_next(weaponRecords[3], DM1_V1_F0162_THING_END_OF_LIST_PC34);

    context->columnMajorSquares = map;
    context->width = MAP_W;
    context->height = MAP_H;
    context->columnFirstThingCounts = columnCounts;
    context->columnFirstThingCount = MAP_W;
    context->squareFirstThings = squareFirstThings;
    context->squareFirstThingCount = 2;
    context->squareFirstThingCapacity = 8;
    context->thingData[4].records = &groupRecords[0][0];
    context->thingData[4].recordCount = 2;
    context->thingData[4].recordSize = 4;
    context->thingData[5].records = &weaponRecords[0][0];
    context->thingData[5].recordCount = 4;
    context->thingData[5].recordSize = 4;
    context->thingData[3].records = &sensorRecords[0][0];
    context->thingData[3].recordCount = 2;
    context->thingData[3].recordSize = 8;
}

static void test_source_evidence(void)
{
    const char *evidence = DM1_V1_F0162_F0163_F0164_SourceEvidencePc34();
    (void)evidence;

    assert(evidence != 0);
    assert(strstr(evidence, "F0162_DUNGEON_GetSquareFirstObject") != 0);
    assert(strstr(evidence, "F0163_DUNGEON_LinkThingToList") != 0);
    assert(strstr(evidence, "F0164_DUNGEON_UnlinkThingFromList") != 0);
}

static void test_f0162_first_object_filter(void)
{
    DM1_V1_MutableThingListContextF0162F0164Pc34 context;
    uint8_t map[MAP_W * MAP_H];
    uint16_t columnCounts[MAP_W];
    uint16_t squareFirstThings[8];
    uint8_t groupRecords[2][4];
    uint8_t weaponRecords[4][4];
    uint8_t sensorRecords[2][8];

    build_context(&context, map, columnCounts, squareFirstThings,
                  groupRecords, weaponRecords, sensorRecords);
    assert(F0162_DUNGEON_GetSquareFirstObject(&context, 0, 1) ==
           thing_ref(5, 0));
    assert(F0162_DUNGEON_GetSquareFirstObject(&context, 1, 1) ==
           DM1_V1_F0162_THING_END_OF_LIST_PC34);
    write_next(sensorRecords[0], DM1_V1_F0162_THING_END_OF_LIST_PC34);
    assert(F0162_DUNGEON_GetSquareFirstObject(&context, 0, 1) ==
           DM1_V1_F0162_THING_END_OF_LIST_PC34);
}

static void test_f0163_append_and_create_square_head(void)
{
    DM1_V1_MutableThingListContextF0162F0164Pc34 context;
    DM1_V1_ThingListMutationResultF0163F0164Pc34 result;
    (void)result;
    uint8_t map[MAP_W * MAP_H];
    uint16_t columnCounts[MAP_W];
    uint16_t squareFirstThings[8];
    uint8_t groupRecords[2][4];
    uint8_t weaponRecords[4][4];
    uint8_t sensorRecords[2][8];

    build_context(&context, map, columnCounts, squareFirstThings,
                  groupRecords, weaponRecords, sensorRecords);
    assert(F0163_DUNGEON_LinkThingToList(
        &context, thing_ref(5, 2), thing_ref(4, 0), -1, -1, &result) == 1);
    assert(result.valid == 1);
    assert(result.previousThing == thing_ref(5, 1));
    assert(next_of(weaponRecords[1]) == thing_ref(5, 2));
    assert(next_of(weaponRecords[2]) == DM1_V1_F0162_THING_END_OF_LIST_PC34);

    assert(F0163_DUNGEON_LinkThingToList(
        &context, thing_ref(5, 3), DM1_V1_F0162_THING_END_OF_LIST_PC34,
        1, 2, &result) == 1);
    assert(result.valid == 1);
    assert(result.squareFirstThingIndex == 1);
    assert(context.squareFirstThingCount == 3);
    assert((map[(1 * MAP_H) + 2] &
            DM1_V1_F0162_SQUARE_THING_LIST_MASK_PC34) != 0);
    assert(squareFirstThings[1] == thing_ref(5, 3));
    assert(squareFirstThings[2] == thing_ref(3, 0));
    assert(columnCounts[2] == 2);
    assert(columnCounts[3] == 3);
}

static void test_f0164_unlink_middle_and_head(void)
{
    DM1_V1_MutableThingListContextF0162F0164Pc34 context;
    DM1_V1_ThingListMutationResultF0163F0164Pc34 result;
    (void)result;
    uint8_t map[MAP_W * MAP_H];
    uint16_t columnCounts[MAP_W];
    uint16_t squareFirstThings[8];
    uint8_t groupRecords[2][4];
    uint8_t weaponRecords[4][4];
    uint8_t sensorRecords[2][8];

    build_context(&context, map, columnCounts, squareFirstThings,
                  groupRecords, weaponRecords, sensorRecords);
    assert(F0164_DUNGEON_UnlinkThingFromList(
        &context, thing_ref(5, 0), thing_ref(4, 0), -1, -1, &result) == 1);
    assert(result.valid == 1);
    assert(result.previousThing == thing_ref(3, 0));
    assert(result.nextThing == thing_ref(5, 1));
    assert(next_of(sensorRecords[0]) == thing_ref(5, 1));
    assert(next_of(weaponRecords[0]) == DM1_V1_F0162_THING_END_OF_LIST_PC34);

    assert(F0164_DUNGEON_UnlinkThingFromList(
        &context, thing_ref(4, 0), DM1_V1_F0162_THING_END_OF_LIST_PC34,
        0, 1, &result) == 1);
    assert(result.squareFirstThingIndex == 0);
    assert(result.newHeadThing == thing_ref(3, 0));
    assert(squareFirstThings[0] == thing_ref(3, 0));
    assert(next_of(groupRecords[0]) == DM1_V1_F0162_THING_END_OF_LIST_PC34);
}

static void test_fail_closed_inputs(void)
{
    DM1_V1_MutableThingListContextF0162F0164Pc34 context;
    DM1_V1_ThingListMutationResultF0163F0164Pc34 result;
    (void)result;
    uint8_t map[MAP_W * MAP_H];
    uint16_t columnCounts[MAP_W];
    uint16_t squareFirstThings[8];
    uint8_t groupRecords[2][4];
    uint8_t weaponRecords[4][4];
    uint8_t sensorRecords[2][8];

    build_context(&context, map, columnCounts, squareFirstThings,
                  groupRecords, weaponRecords, sensorRecords);
    assert(F0163_DUNGEON_LinkThingToList(
        &context, thing_ref(5, 7), thing_ref(4, 0), -1, -1, &result) == 0);
    assert(result.valid == 0);
    assert(F0163_DUNGEON_LinkThingToList(
        &context, thing_ref(5, 2), DM1_V1_F0162_THING_END_OF_LIST_PC34,
        -1, -1, &result) == 0);
    assert(F0164_DUNGEON_UnlinkThingFromList(
        &context, thing_ref(5, 2), thing_ref(4, 0), -1, -1, &result) == 0);
    assert(F0164_DUNGEON_UnlinkThingFromList(
        &context, DM1_V1_F0162_THING_END_OF_LIST_PC34, thing_ref(4, 0),
        -1, -1, &result) == 0);

    context.squareFirstThingCapacity = context.squareFirstThingCount;
    assert(F0163_DUNGEON_LinkThingToList(
        &context, thing_ref(5, 2), DM1_V1_F0162_THING_END_OF_LIST_PC34,
        1, 1, &result) == 0);
}

int main(void)
{
    test_source_evidence();
    test_f0162_first_object_filter();
    test_f0163_append_and_create_square_head();
    test_f0164_unlink_middle_and_head();
    test_fail_closed_inputs();
    return 0;
}

#ifndef DM1_V1_THING_LIST_HELPERS_F0156_F0159_F0160_F0161_PC34_COMPAT_H
#define DM1_V1_THING_LIST_HELPERS_F0156_F0159_F0160_F0161_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_V1_F0156_THING_TYPE_COUNT_PC34 = 16,
    DM1_V1_F0156_THING_END_OF_LIST_PC34 = 0xfffeu,
    DM1_V1_F0156_THING_NONE_PC34 = 0xffffu,
    DM1_V1_F0160_SQUARE_THING_LIST_MASK_PC34 = 0x10
};

typedef struct {
    const uint8_t *records;
    size_t recordCount;
    size_t recordSize;
} DM1_V1_ThingDataTableF0156Pc34;

typedef struct {
    const uint8_t *columnMajorSquares;
    int width;
    int height;
    const uint16_t *columnFirstThingCounts;
    size_t columnFirstThingCount;
    const uint16_t *squareFirstThings;
    size_t squareFirstThingCount;
    DM1_V1_ThingDataTableF0156Pc34 thingData[
        DM1_V1_F0156_THING_TYPE_COUNT_PC34];
} DM1_V1_DungeonThingListContextF0156F0161Pc34;

typedef struct {
    int valid;
    int thingType;
    int thingIndex;
    size_t recordSize;
    const uint8_t *record;
} DM1_V1_ThingDataResultF0156Pc34;

const char *DM1_V1_F0156_F0159_F0160_F0161_SourceEvidencePc34(void);

int DM1_V1_ThingTypeFromThingF0156Pc34(uint16_t thing);
int DM1_V1_ThingIndexFromThingF0156Pc34(uint16_t thing);

const uint8_t *F0156_DUNGEON_GetThingData(
    const DM1_V1_DungeonThingListContextF0156F0161Pc34 *context,
    uint16_t thing);

int DM1_V1_Dungeon_GetThingDataF0156Pc34Compat(
    const DM1_V1_DungeonThingListContextF0156F0161Pc34 *context,
    uint16_t thing,
    DM1_V1_ThingDataResultF0156Pc34 *out);

uint16_t F0159_DUNGEON_GetNextThing(
    const DM1_V1_DungeonThingListContextF0156F0161Pc34 *context,
    uint16_t thing);

int F0160_DUNGEON_GetSquareFirstThingIndex(
    const DM1_V1_DungeonThingListContextF0156F0161Pc34 *context,
    int mapX,
    int mapY);

uint16_t F0161_DUNGEON_GetSquareFirstThing(
    const DM1_V1_DungeonThingListContextF0156F0161Pc34 *context,
    int mapX,
    int mapY);

#ifdef __cplusplus
}
#endif

#endif

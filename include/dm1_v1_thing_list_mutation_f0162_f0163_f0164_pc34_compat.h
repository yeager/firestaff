#ifndef DM1_V1_THING_LIST_MUTATION_F0162_F0163_F0164_PC34_COMPAT_H
#define DM1_V1_THING_LIST_MUTATION_F0162_F0163_F0164_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_V1_F0162_THING_TYPE_COUNT_PC34 = 16,
    DM1_V1_F0162_THING_TYPE_GROUP_PC34 = 4,
    DM1_V1_F0162_THING_END_OF_LIST_PC34 = 0xfffeu,
    DM1_V1_F0162_THING_NONE_PC34 = 0xffffu,
    DM1_V1_F0162_SQUARE_THING_LIST_MASK_PC34 = 0x10,
    DM1_V1_F0162_MAX_CHAIN_STEPS_PC34 = 1024
};

typedef struct {
    uint8_t *records;
    size_t recordCount;
    size_t recordSize;
} DM1_V1_MutableThingDataTableF0162Pc34;

typedef struct {
    uint8_t *columnMajorSquares;
    int width;
    int height;
    uint16_t *columnFirstThingCounts;
    size_t columnFirstThingCount;
    uint16_t *squareFirstThings;
    size_t squareFirstThingCount;
    size_t squareFirstThingCapacity;
    DM1_V1_MutableThingDataTableF0162Pc34 thingData[
        DM1_V1_F0162_THING_TYPE_COUNT_PC34];
} DM1_V1_MutableThingListContextF0162F0164Pc34;

typedef struct {
    int valid;
    int squareFirstThingIndex;
    uint16_t previousThing;
    uint16_t nextThing;
    uint16_t newHeadThing;
} DM1_V1_ThingListMutationResultF0163F0164Pc34;

const char *DM1_V1_F0162_F0163_F0164_SourceEvidencePc34(void);

uint16_t F0162_DUNGEON_GetSquareFirstObject(
    DM1_V1_MutableThingListContextF0162F0164Pc34 *context,
    int mapX,
    int mapY);

int F0163_DUNGEON_LinkThingToList(
    DM1_V1_MutableThingListContextF0162F0164Pc34 *context,
    uint16_t thingToLink,
    uint16_t thingInList,
    int mapX,
    int mapY,
    DM1_V1_ThingListMutationResultF0163F0164Pc34 *out);

int F0164_DUNGEON_UnlinkThingFromList(
    DM1_V1_MutableThingListContextF0162F0164Pc34 *context,
    uint16_t thingToUnlink,
    uint16_t thingInList,
    int mapX,
    int mapY,
    DM1_V1_ThingListMutationResultF0163F0164Pc34 *out);

#ifdef __cplusplus
}
#endif

#endif

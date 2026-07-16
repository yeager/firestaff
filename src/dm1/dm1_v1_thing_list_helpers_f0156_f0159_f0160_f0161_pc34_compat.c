#include "dm1_v1_thing_list_helpers_f0156_f0159_f0160_f0161_pc34_compat.h"

static uint16_t dm1_v1_read_le16(const uint8_t *bytes)
{
    return (uint16_t)((uint16_t)bytes[0] | ((uint16_t)bytes[1] << 8));
}

const char *DM1_V1_F0156_F0159_F0160_F0161_SourceEvidencePc34(void)
{
    return
        "DUNGEON.C:1586 F0156_DUNGEON_GetThingData indexes "
        "G0284_apuc_ThingData[M012_TYPE(thing)] by M013_INDEX(thing); "
        "DUNGEON.C:1664-1681 F0159_DUNGEON_GetNextThing reads the Generic.Next "
        "word from that record; DUNGEON.C:1699-1728 F0160 starts from "
        "G0270_pui_CurrentMapColumnsCumulativeSquareFirstThingCount[x] and "
        "adds prior MASK0x0010_THING_LIST rows in the same column; "
        "DUNGEON.C:1730-1746 F0161 returns G0283_pT_SquareFirstThings[index].";
}

int DM1_V1_ThingTypeFromThingF0156Pc34(uint16_t thing)
{
    if (thing == DM1_V1_F0156_THING_END_OF_LIST_PC34 ||
        thing == DM1_V1_F0156_THING_NONE_PC34) {
        return -1;
    }
    return (int)((thing & 0x3c00u) >> 10);
}

int DM1_V1_ThingIndexFromThingF0156Pc34(uint16_t thing)
{
    if (thing == DM1_V1_F0156_THING_END_OF_LIST_PC34 ||
        thing == DM1_V1_F0156_THING_NONE_PC34) {
        return -1;
    }
    return (int)(thing & 0x03ffu);
}

int DM1_V1_Dungeon_GetThingDataF0156Pc34Compat(
    const DM1_V1_DungeonThingListContextF0156F0161Pc34 *context,
    uint16_t thing,
    DM1_V1_ThingDataResultF0156Pc34 *out)
{
    int type;
    int index;
    const DM1_V1_ThingDataTableF0156Pc34 *table;

    if (!out) {
        return 0;
    }
    out->valid = 0;
    out->thingType = -1;
    out->thingIndex = -1;
    out->recordSize = 0;
    out->record = 0;

    if (!context) {
        return 0;
    }
    type = DM1_V1_ThingTypeFromThingF0156Pc34(thing);
    index = DM1_V1_ThingIndexFromThingF0156Pc34(thing);
    out->thingType = type;
    out->thingIndex = index;
    if (type < 0 || type >= DM1_V1_F0156_THING_TYPE_COUNT_PC34 ||
        index < 0) {
        return 0;
    }

    table = &context->thingData[type];
    if (!table->records || table->recordSize < 2 ||
        (size_t)index >= table->recordCount) {
        return 0;
    }

    out->valid = 1;
    out->recordSize = table->recordSize;
    out->record = table->records + ((size_t)index * table->recordSize);
    return 1;
}

const uint8_t *F0156_DUNGEON_GetThingData(
    const DM1_V1_DungeonThingListContextF0156F0161Pc34 *context,
    uint16_t thing)
{
    DM1_V1_ThingDataResultF0156Pc34 result;

    if (!DM1_V1_Dungeon_GetThingDataF0156Pc34Compat(
            context, thing, &result)) {
        return 0;
    }
    return result.record;
}

uint16_t F0159_DUNGEON_GetNextThing(
    const DM1_V1_DungeonThingListContextF0156F0161Pc34 *context,
    uint16_t thing)
{
    DM1_V1_ThingDataResultF0156Pc34 result;

    if (!DM1_V1_Dungeon_GetThingDataF0156Pc34Compat(
            context, thing, &result) ||
        result.recordSize < 2) {
        return (uint16_t)DM1_V1_F0156_THING_END_OF_LIST_PC34;
    }
    return dm1_v1_read_le16(result.record);
}

int F0160_DUNGEON_GetSquareFirstThingIndex(
    const DM1_V1_DungeonThingListContextF0156F0161Pc34 *context,
    int mapX,
    int mapY)
{
    int index;
    int y;

    if (!context || !context->columnMajorSquares ||
        !context->columnFirstThingCounts ||
        context->width <= 0 || context->height <= 0 ||
        mapX < 0 || mapX >= context->width ||
        mapY < 0 || mapY >= context->height ||
        (size_t)mapX >= context->columnFirstThingCount) {
        return -1;
    }

    if ((context->columnMajorSquares[(mapX * context->height) + mapY] &
         DM1_V1_F0160_SQUARE_THING_LIST_MASK_PC34) == 0) {
        return -1;
    }

    index = (int)context->columnFirstThingCounts[mapX];
    for (y = 0; y < mapY; ++y) {
        if (context->columnMajorSquares[(mapX * context->height) + y] &
            DM1_V1_F0160_SQUARE_THING_LIST_MASK_PC34) {
            ++index;
        }
    }

    if (index < 0 || (size_t)index >= context->squareFirstThingCount) {
        return -1;
    }
    return index;
}

uint16_t F0161_DUNGEON_GetSquareFirstThing(
    const DM1_V1_DungeonThingListContextF0156F0161Pc34 *context,
    int mapX,
    int mapY)
{
    int index = F0160_DUNGEON_GetSquareFirstThingIndex(
        context, mapX, mapY);

    if (index < 0 || !context || !context->squareFirstThings) {
        return (uint16_t)DM1_V1_F0156_THING_END_OF_LIST_PC34;
    }
    return context->squareFirstThings[index];
}

#include "dm1_v1_thing_list_mutation_f0162_f0163_f0164_pc34_compat.h"

static uint16_t dm1_v1_read_le16(const uint8_t *bytes)
{
    return (uint16_t)((uint16_t)bytes[0] | ((uint16_t)bytes[1] << 8));
}

static void dm1_v1_write_le16(uint8_t *bytes, uint16_t value)
{
    bytes[0] = (uint8_t)(value & 0xffu);
    bytes[1] = (uint8_t)((value >> 8) & 0xffu);
}

static int dm1_v1_thing_type(uint16_t thing)
{
    if (thing == DM1_V1_F0162_THING_END_OF_LIST_PC34 ||
        thing == DM1_V1_F0162_THING_NONE_PC34) {
        return -1;
    }
    return (int)((thing & 0x3c00u) >> 10);
}

static int dm1_v1_thing_index(uint16_t thing)
{
    if (thing == DM1_V1_F0162_THING_END_OF_LIST_PC34 ||
        thing == DM1_V1_F0162_THING_NONE_PC34) {
        return -1;
    }
    return (int)(thing & 0x03ffu);
}

static uint8_t *dm1_v1_thing_record(
    DM1_V1_MutableThingListContextF0162F0164Pc34 *context,
    uint16_t thing)
{
    int type;
    int index;
    DM1_V1_MutableThingDataTableF0162Pc34 *table;

    if (!context) {
        return 0;
    }
    type = dm1_v1_thing_type(thing);
    index = dm1_v1_thing_index(thing);
    if (type < 0 || type >= DM1_V1_F0162_THING_TYPE_COUNT_PC34 ||
        index < 0) {
        return 0;
    }
    table = &context->thingData[type];
    if (!table->records || table->recordSize < 2 ||
        (size_t)index >= table->recordCount) {
        return 0;
    }
    return table->records + ((size_t)index * table->recordSize);
}

static uint16_t dm1_v1_next_thing(
    DM1_V1_MutableThingListContextF0162F0164Pc34 *context,
    uint16_t thing)
{
    uint8_t *record = dm1_v1_thing_record(context, thing);

    if (!record) {
        return (uint16_t)DM1_V1_F0162_THING_END_OF_LIST_PC34;
    }
    return dm1_v1_read_le16(record);
}

static int dm1_v1_set_next_thing(
    DM1_V1_MutableThingListContextF0162F0164Pc34 *context,
    uint16_t thing,
    uint16_t next)
{
    uint8_t *record = dm1_v1_thing_record(context, thing);

    if (!record) {
        return 0;
    }
    dm1_v1_write_le16(record, next);
    return 1;
}

static int dm1_v1_square_valid(
    const DM1_V1_MutableThingListContextF0162F0164Pc34 *context,
    int mapX,
    int mapY)
{
    return context && context->columnMajorSquares &&
           context->columnFirstThingCounts &&
           context->squareFirstThings &&
           context->width > 0 && context->height > 0 &&
           mapX >= 0 && mapX < context->width &&
           mapY >= 0 && mapY < context->height &&
           (size_t)mapX < context->columnFirstThingCount;
}

static int dm1_v1_square_first_index(
    const DM1_V1_MutableThingListContextF0162F0164Pc34 *context,
    int mapX,
    int mapY)
{
    int index;
    int y;

    if (!dm1_v1_square_valid(context, mapX, mapY)) {
        return -1;
    }
    if ((context->columnMajorSquares[(mapX * context->height) + mapY] &
         DM1_V1_F0162_SQUARE_THING_LIST_MASK_PC34) == 0) {
        return -1;
    }
    index = (int)context->columnFirstThingCounts[mapX];
    for (y = 0; y < mapY; ++y) {
        if (context->columnMajorSquares[(mapX * context->height) + y] &
            DM1_V1_F0162_SQUARE_THING_LIST_MASK_PC34) {
            ++index;
        }
    }
    if (index < 0 || (size_t)index >= context->squareFirstThingCount) {
        return -1;
    }
    return index;
}

static int dm1_v1_create_square_first_index(
    DM1_V1_MutableThingListContextF0162F0164Pc34 *context,
    int mapX,
    int mapY)
{
    int insertIndex;
    int column;
    size_t i;

    if (!dm1_v1_square_valid(context, mapX, mapY) ||
        (context->columnMajorSquares[(mapX * context->height) + mapY] &
         DM1_V1_F0162_SQUARE_THING_LIST_MASK_PC34) != 0 ||
        context->squareFirstThingCount >= context->squareFirstThingCapacity) {
        return -1;
    }

    insertIndex = (int)context->columnFirstThingCounts[mapX];
    for (i = 0; i < (size_t)mapY; ++i) {
        if (context->columnMajorSquares[(mapX * context->height) + (int)i] &
            DM1_V1_F0162_SQUARE_THING_LIST_MASK_PC34) {
            ++insertIndex;
        }
    }
    if (insertIndex < 0 ||
        (size_t)insertIndex > context->squareFirstThingCount) {
        return -1;
    }

    for (i = context->squareFirstThingCount; i > (size_t)insertIndex; --i) {
        context->squareFirstThings[i] = context->squareFirstThings[i - 1];
    }
    context->squareFirstThings[insertIndex] =
        (uint16_t)DM1_V1_F0162_THING_END_OF_LIST_PC34;
    ++context->squareFirstThingCount;
    context->columnMajorSquares[(mapX * context->height) + mapY] |=
        DM1_V1_F0162_SQUARE_THING_LIST_MASK_PC34;
    for (column = mapX + 1; column < context->width; ++column) {
        if ((size_t)column < context->columnFirstThingCount) {
            ++context->columnFirstThingCounts[column];
        }
    }
    return insertIndex;
}

static void dm1_v1_clear_result(
    DM1_V1_ThingListMutationResultF0163F0164Pc34 *out)
{
    if (!out) {
        return;
    }
    out->valid = 0;
    out->squareFirstThingIndex = -1;
    out->previousThing = (uint16_t)DM1_V1_F0162_THING_END_OF_LIST_PC34;
    out->nextThing = (uint16_t)DM1_V1_F0162_THING_END_OF_LIST_PC34;
    out->newHeadThing = (uint16_t)DM1_V1_F0162_THING_END_OF_LIST_PC34;
}

const char *DM1_V1_F0162_F0163_F0164_SourceEvidencePc34(void)
{
    return
        "DUNGEON.C:1754 F0162_DUNGEON_GetSquareFirstObject starts from "
        "F0161 and walks F0159 until M012_TYPE(thing) > C04_THING_TYPE_GROUP; "
        "DUNGEON.C:1769-1838 F0163_DUNGEON_LinkThingToList clears "
        "ThingToLink->Next to C0xFFFE_THING_ENDOFLIST, appends after the tail, "
        "or creates/updates the square-first list for MapX/MapY; "
        "DUNGEON.C:1840-1905 F0164_DUNGEON_UnlinkThingFromList relinks the "
        "previous node or square head and clears the removed thing's Next.";
}

uint16_t F0162_DUNGEON_GetSquareFirstObject(
    DM1_V1_MutableThingListContextF0162F0164Pc34 *context,
    int mapX,
    int mapY)
{
    int index;
    uint16_t thing;
    size_t steps;

    index = dm1_v1_square_first_index(context, mapX, mapY);
    if (index < 0) {
        return (uint16_t)DM1_V1_F0162_THING_END_OF_LIST_PC34;
    }
    thing = context->squareFirstThings[index];
    for (steps = 0; steps < DM1_V1_F0162_MAX_CHAIN_STEPS_PC34 &&
                    thing != DM1_V1_F0162_THING_END_OF_LIST_PC34 &&
                    thing != DM1_V1_F0162_THING_NONE_PC34;
         ++steps) {
        if (dm1_v1_thing_type(thing) > DM1_V1_F0162_THING_TYPE_GROUP_PC34) {
            return thing;
        }
        thing = dm1_v1_next_thing(context, thing);
    }
    return (uint16_t)DM1_V1_F0162_THING_END_OF_LIST_PC34;
}

int F0163_DUNGEON_LinkThingToList(
    DM1_V1_MutableThingListContextF0162F0164Pc34 *context,
    uint16_t thingToLink,
    uint16_t thingInList,
    int mapX,
    int mapY,
    DM1_V1_ThingListMutationResultF0163F0164Pc34 *out)
{
    uint16_t tail;
    uint16_t next;
    size_t steps;
    int index;

    dm1_v1_clear_result(out);
    if (!context || !dm1_v1_thing_record(context, thingToLink) ||
        !dm1_v1_set_next_thing(
            context, thingToLink,
            (uint16_t)DM1_V1_F0162_THING_END_OF_LIST_PC34)) {
        return 0;
    }

    if (thingInList != DM1_V1_F0162_THING_END_OF_LIST_PC34 &&
        thingInList != DM1_V1_F0162_THING_NONE_PC34) {
        tail = thingInList;
        for (steps = 0; steps < DM1_V1_F0162_MAX_CHAIN_STEPS_PC34; ++steps) {
            next = dm1_v1_next_thing(context, tail);
            if (next == DM1_V1_F0162_THING_END_OF_LIST_PC34 ||
                next == DM1_V1_F0162_THING_NONE_PC34) {
                if (!dm1_v1_set_next_thing(context, tail, thingToLink)) {
                    return 0;
                }
                if (out) {
                    out->valid = 1;
                    out->previousThing = tail;
                    out->nextThing =
                        (uint16_t)DM1_V1_F0162_THING_END_OF_LIST_PC34;
                }
                return 1;
            }
            tail = next;
        }
        return 0;
    }

    index = dm1_v1_square_first_index(context, mapX, mapY);
    if (index < 0) {
        index = dm1_v1_create_square_first_index(context, mapX, mapY);
    }
    if (index < 0) {
        return 0;
    }
    context->squareFirstThings[index] = thingToLink;
    if (out) {
        out->valid = 1;
        out->squareFirstThingIndex = index;
        out->newHeadThing = thingToLink;
    }
    return 1;
}

int F0164_DUNGEON_UnlinkThingFromList(
    DM1_V1_MutableThingListContextF0162F0164Pc34 *context,
    uint16_t thingToUnlink,
    uint16_t thingInList,
    int mapX,
    int mapY,
    DM1_V1_ThingListMutationResultF0163F0164Pc34 *out)
{
    uint16_t current;
    uint16_t previous;
    uint16_t next;
    size_t steps;
    int index;

    dm1_v1_clear_result(out);
    if (!context ||
        thingToUnlink == DM1_V1_F0162_THING_END_OF_LIST_PC34 ||
        thingToUnlink == DM1_V1_F0162_THING_NONE_PC34 ||
        !dm1_v1_thing_record(context, thingToUnlink)) {
        return 0;
    }

    if (thingInList != DM1_V1_F0162_THING_END_OF_LIST_PC34 &&
        thingInList != DM1_V1_F0162_THING_NONE_PC34) {
        current = thingInList;
        previous = (uint16_t)DM1_V1_F0162_THING_END_OF_LIST_PC34;
        for (steps = 0; steps < DM1_V1_F0162_MAX_CHAIN_STEPS_PC34; ++steps) {
            if (current == thingToUnlink) {
                next = dm1_v1_next_thing(context, current);
                if (previous != DM1_V1_F0162_THING_END_OF_LIST_PC34 &&
                    !dm1_v1_set_next_thing(context, previous, next)) {
                    return 0;
                }
                if (!dm1_v1_set_next_thing(
                        context, current,
                        (uint16_t)DM1_V1_F0162_THING_END_OF_LIST_PC34)) {
                    return 0;
                }
                if (out) {
                    out->valid = 1;
                    out->previousThing = previous;
                    out->nextThing = next;
                }
                return previous != DM1_V1_F0162_THING_END_OF_LIST_PC34;
            }
            next = dm1_v1_next_thing(context, current);
            if (next == DM1_V1_F0162_THING_END_OF_LIST_PC34 ||
                next == DM1_V1_F0162_THING_NONE_PC34) {
                return 0;
            }
            previous = current;
            current = next;
        }
        return 0;
    }

    index = dm1_v1_square_first_index(context, mapX, mapY);
    if (index < 0 || context->squareFirstThings[index] != thingToUnlink) {
        return 0;
    }
    next = dm1_v1_next_thing(context, thingToUnlink);
    context->squareFirstThings[index] = next;
    if (!dm1_v1_set_next_thing(
            context, thingToUnlink,
            (uint16_t)DM1_V1_F0162_THING_END_OF_LIST_PC34)) {
        return 0;
    }
    if (out) {
        out->valid = 1;
        out->squareFirstThingIndex = index;
        out->nextThing = next;
        out->newHeadThing = next;
    }
    return 1;
}

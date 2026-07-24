#include "dm1_v1_group_state_bundle_pc34_compat.h"

#include <string.h>

#include "dm1_v1_creature_ai_behavior_pc34_compat.h"

#define DM1_V1_GROUP_RAW_BYTES_PC34 16
#define DM1_V1_GROUP_TYPE_PC34 4
#define DM1_V1_THING_END_PC34 0xfffeu
#define DM1_V1_THING_NONE_PC34 0xffffu

static const unsigned char dm1_v1_thing_record_bytes_pc34[16] = {
    4u, 6u, 4u, 8u, 16u, 4u, 4u, 4u,
    4u, 8u, 4u, 0u, 0u, 0u, 8u, 4u
};

static uint16_t dm1_v1_read_u16_pc34(const unsigned char *bytes)
{
    return (uint16_t)((uint16_t)bytes[0] | ((uint16_t)bytes[1] << 8));
}

static int dm1_v1_square_first_thing_index_pc34(
    const struct DungeonDatState_Compat *dungeon,
    int mapIndex,
    int mapX,
    int mapY)
{
    const struct DungeonMapDesc_Compat *map;
    int priorMap;
    int index = 0;
    int square;

    if (!dungeon || !dungeon->tilesLoaded || !dungeon->maps ||
        !dungeon->tiles || mapIndex < 0 ||
        mapIndex >= (int)dungeon->header.mapCount) return -1;
    map = &dungeon->maps[mapIndex];
    if (mapX < 0 || mapX >= map->width || mapY < 0 || mapY >= map->height ||
        !dungeon->tiles[mapIndex].squareData) return -1;
    for (priorMap = 0; priorMap < mapIndex; ++priorMap) {
        int count;
        int item;
        if (!dungeon->tiles[priorMap].squareData) return -1;
        count = dungeon->maps[priorMap].width * dungeon->maps[priorMap].height;
        for (item = 0; item < count; ++item) {
            if (dungeon->tiles[priorMap].squareData[item] &
                DUNGEON_SQUARE_MASK_THING_LIST) ++index;
        }
    }
    square = mapX * map->height + mapY;
    if (!(dungeon->tiles[mapIndex].squareData[square] &
          DUNGEON_SQUARE_MASK_THING_LIST)) return -1;
    while (square-- > 0) {
        if (dungeon->tiles[mapIndex].squareData[square] &
            DUNGEON_SQUARE_MASK_THING_LIST) ++index;
    }
    return index;
}

static int dm1_v1_map_coordinates_valid_pc34(
    const struct GameWorld_Compat *world,
    int mapIndex,
    int mapX,
    int mapY)
{
    const struct DungeonMapDesc_Compat *map;
    if (!world || !world->dungeon || !world->dungeon->tilesLoaded ||
        !world->dungeon->maps || mapIndex < 0 ||
        mapIndex >= (int)world->dungeon->header.mapCount) return 0;
    map = &world->dungeon->maps[mapIndex];
    return mapX >= 0 && mapX < map->width && mapY >= 0 && mapY < map->height;
}

static int dm1_v1_validate_active_group_owner_pc34(
    const struct GameWorld_Compat *world,
    int activeGroupIndex,
    int *outGroupIndex)
{
    int groupIndex;
    if (outGroupIndex) *outGroupIndex = -1;
    if (!world || !world->things || activeGroupIndex < 0 ||
        activeGroupIndex >= world->creatureAICount ||
        activeGroupIndex >= DM1_PC34_ACTIVE_GROUP_CAPACITY ||
        !world->things->groups || !world->things->rawThingData[THING_TYPE_GROUP]) {
        return 0;
    }
    groupIndex = world->creatureAI[activeGroupIndex].reserved0;
    if (groupIndex < 0 || groupIndex >= world->things->groupCount ||
        groupIndex >= world->things->thingCounts[THING_TYPE_GROUP] ||
        world->things->groups[groupIndex].next == THING_NONE) return 0;
    if (outGroupIndex) *outGroupIndex = groupIndex;
    return 1;
}

static uint32_t dm1_v1_group_state_fingerprint_pc34(
    const DM1_V1_SourceActiveGroupPc34Compat *rows,
    int count)
{
    uint32_t hash = 2166136261u;
    int index;
    const unsigned char *bytes;
    size_t byteIndex;

    if (!rows || count < 0) return 0u;
    for (index = 0; index < count; ++index) {
        bytes = (const unsigned char *)&rows[index];
        for (byteIndex = 0u; byteIndex < sizeof(rows[index]); ++byteIndex) {
            hash ^= bytes[byteIndex];
            hash *= 16777619u;
        }
    }
    return hash;
}

static int dm1_v1_group_state_find_active_pc34(
    const struct GameWorld_Compat *world,
    int groupIndex)
{
    int index;

    if (!world || groupIndex < 0 || world->creatureAICount < 0 ||
        world->creatureAICount > DM1_PC34_ACTIVE_GROUP_CAPACITY) {
        return -1;
    }
    for (index = 0; index < world->creatureAICount; ++index) {
        if (world->creatureAI[index].reserved0 == groupIndex) return index;
    }
    return -1;
}

static int dm1_v1_group_state_validate_owner_pc34(
    const struct GameWorld_Compat *world,
    int groupThing,
    int *outGroupIndex)
{
    unsigned int thing = (unsigned int)(uint16_t)groupThing;
    int type = (int)((thing >> 10) & 0x0fu);
    int index = (int)(thing & 0x03ffu);

    if (outGroupIndex) *outGroupIndex = -1;
    if (!world || !world->things || !world->things->groups ||
        world->things->groupCount <= 0 || type != DM1_V1_GROUP_TYPE_PC34 ||
        index < 0 || index >= world->things->groupCount ||
        world->things->groups[index].next == THING_NONE) {
        return 0;
    }
    if (outGroupIndex) *outGroupIndex = index;
    return 1;
}

int dm1_v1_group_state_initialize_f0196_pc34(
    struct GameWorld_Compat *world,
    DM1_V1_GroupStateBundleReceiptPc34Compat *outReceipt)
{
    struct CreatureAIState_Compat staged[DM1_PC34_ACTIVE_GROUP_CAPACITY];
    uint8_t stagedDirections[DM1_PC34_ACTIVE_GROUP_CAPACITY];
    uint8_t stagedHomeX[DM1_PC34_ACTIVE_GROUP_CAPACITY];
    uint8_t stagedHomeY[DM1_PC34_ACTIVE_GROUP_CAPACITY];
    int index;

    if (outReceipt) memset(outReceipt, 0, sizeof(*outReceipt));
    if (!world || GAMEWORLD_CREATURE_AI_CAPACITY <
                      DM1_PC34_ACTIVE_GROUP_CAPACITY) {
        return 0;
    }
    memset(staged, 0, sizeof(staged));
    memset(stagedDirections, 0, sizeof(stagedDirections));
    memset(stagedHomeX, 0, sizeof(stagedHomeX));
    memset(stagedHomeY, 0, sizeof(stagedHomeY));
    for (index = 0; index < DM1_PC34_ACTIVE_GROUP_CAPACITY; ++index) {
        staged[index].reserved0 = -1;
    }

    memcpy(world->creatureAI, staged, sizeof(staged));
    memcpy(world->pc34ActiveGroupDirections, stagedDirections,
           sizeof(stagedDirections));
    memcpy(world->pc34ActiveGroupHomeMapX, stagedHomeX, sizeof(stagedHomeX));
    memcpy(world->pc34ActiveGroupHomeMapY, stagedHomeY, sizeof(stagedHomeY));
    world->creatureAICount = 0;
    world->pc34ActiveGroupSourceCount = 0;
    if (outReceipt) {
        outReceipt->valid = 1;
        outReceipt->activeGroupCount = 0;
        outReceipt->sourceCapacity = DM1_PC34_ACTIVE_GROUP_CAPACITY;
        outReceipt->fingerprint = 2166136261u;
        outReceipt->sourceSymbol = "F0196_GROUP_InitializeActiveGroups";
    }
    return 1;
}

int dm1_v1_group_state_write_f0146_f0148_pc34(
    struct GameWorld_Compat *world,
    int mapIndex,
    int groupIndex,
    unsigned int cells,
    unsigned int directions,
    DM1_V1_GroupStateBundleReceiptPc34Compat *outReceipt)
{
    unsigned char *raw;
    int activeIndex;

    if (outReceipt) memset(outReceipt, 0, sizeof(*outReceipt));
    if (!world || !world->things || !world->things->loaded ||
        !world->things->groups || groupIndex < 0 ||
        groupIndex >= world->things->groupCount || mapIndex < 0 ||
        cells > 0xffu) {
        return 0;
    }
    if (mapIndex == world->partyMapIndex && directions > 0xffu) return 0;
    if (!world->things->rawThingData[THING_TYPE_GROUP] ||
        world->things->thingCounts[THING_TYPE_GROUP] <= groupIndex) {
        return 0;
    }
    raw = world->things->rawThingData[THING_TYPE_GROUP] +
        (size_t)groupIndex * DM1_V1_GROUP_RAW_BYTES_PC34;
    if (mapIndex == world->partyMapIndex) {
        activeIndex = dm1_v1_group_state_find_active_pc34(world, groupIndex);
        if (activeIndex < 0) return 0;
        world->creatureAI[activeIndex].groupCells = (int)cells;
        world->creatureAI[activeIndex].groupDirection = (int)directions;
        if (activeIndex < DM1_PC34_ACTIVE_GROUP_CAPACITY) {
            world->pc34ActiveGroupDirections[activeIndex] =
                (uint8_t)directions;
        }
    } else {
        world->things->groups[groupIndex].cells = (unsigned char)cells;
        world->things->groups[groupIndex].direction =
            (unsigned char)(directions & 0x03u);
        raw[5] = (unsigned char)cells;
        raw[15] = (unsigned char)((raw[15] & ~0x03u) | (directions & 0x03u));
    }
    if (outReceipt) {
        outReceipt->valid = 1;
        outReceipt->activeGroupCount = world->creatureAICount;
        outReceipt->groupIndex = groupIndex;
        outReceipt->mapIndex = mapIndex;
        outReceipt->cells = cells;
        outReceipt->directions = directions;
        outReceipt->sourceSymbol = "F0146_DUNGEON_SetGroupCells/F0148_DUNGEON_SetGroupDirections";
    }
    return 1;
}

int dm1_v1_group_state_apply_save_handoff_pc34(
    struct GameWorld_Compat *world,
    const DM1_V1_SourceActiveGroupPc34Compat *rows,
    int currentCount,
    int sourceCapacity,
    DM1_V1_GroupStateBundleReceiptPc34Compat *outReceipt)
{
    struct CreatureAIState_Compat staged[DM1_PC34_ACTIVE_GROUP_CAPACITY];
    uint8_t stagedDirections[DM1_PC34_ACTIVE_GROUP_CAPACITY];
    uint8_t stagedHomeX[DM1_PC34_ACTIVE_GROUP_CAPACITY];
    uint8_t stagedHomeY[DM1_PC34_ACTIVE_GROUP_CAPACITY];
    int index;

    if (outReceipt) memset(outReceipt, 0, sizeof(*outReceipt));
    if (!world || !rows || currentCount < 0 ||
        sourceCapacity < currentCount ||
        sourceCapacity > DM1_PC34_ACTIVE_GROUP_CAPACITY) {
        return 0;
    }
    memset(staged, 0, sizeof(staged));
    memset(stagedDirections, 0, sizeof(stagedDirections));
    memset(stagedHomeX, 0, sizeof(stagedHomeX));
    memset(stagedHomeY, 0, sizeof(stagedHomeY));
    for (index = 0; index < DM1_PC34_ACTIVE_GROUP_CAPACITY; ++index) {
        staged[index].reserved0 = -1;
    }
    for (index = 0; index < currentCount; ++index) {
        const DM1_V1_SourceActiveGroupPc34Compat *src = &rows[index];
        int groupIndex;

        if (!dm1_v1_group_state_validate_owner_pc34(
                world, src->groupThing, &groupIndex) ||
            src->cells < 0 || src->cells > 0xff ||
            src->directions < 0 || src->directions > 0xff ||
            src->homeMapX < 0 || src->homeMapX > 0xff ||
            src->homeMapY < 0 || src->homeMapY > 0xff) {
            return 0;
        }
        staged[index].stateKind = AI_STATE_WANDER;
        staged[index].creatureType = world->things->groups[groupIndex].creatureType;
        staged[index].groupMapIndex = world->partyMapIndex;
        staged[index].groupMapX = src->priorMapX;
        staged[index].groupMapY = src->priorMapY;
        staged[index].groupCells = src->cells;
        staged[index].groupDirection = src->directions & 0x03;
        staged[index].targetChampionIndex = -1;
        staged[index].lastSeenPartyMapX = src->targetMapX;
        staged[index].lastSeenPartyMapY = src->targetMapY;
        staged[index].lastSeenPartyTick = src->lastMoveTime;
        staged[index].fearCounter = src->delayFleeingFromTarget;
        staged[index].reserved0 = groupIndex;
        memcpy(staged[index].aspect, src->aspect, sizeof(src->aspect));
        stagedDirections[index] = (uint8_t)src->directions;
        stagedHomeX[index] = (uint8_t)src->homeMapX;
        stagedHomeY[index] = (uint8_t)src->homeMapY;
    }

    memcpy(world->creatureAI, staged, sizeof(staged));
    memcpy(world->pc34ActiveGroupDirections, stagedDirections,
           sizeof(stagedDirections));
    memcpy(world->pc34ActiveGroupHomeMapX, stagedHomeX, sizeof(stagedHomeX));
    memcpy(world->pc34ActiveGroupHomeMapY, stagedHomeY, sizeof(stagedHomeY));
    world->creatureAICount = currentCount;
    world->pc34ActiveGroupSourceCount = sourceCapacity;
    if (outReceipt) {
        outReceipt->valid = 1;
        outReceipt->activeGroupCount = currentCount;
        outReceipt->sourceCapacity = sourceCapacity;
        outReceipt->fingerprint = dm1_v1_group_state_fingerprint_pc34(rows,
                                                                        currentCount);
        outReceipt->sourceSymbol = "LOADSAVE.C:F0435 -> F0145/F0146/F0147/F0196";
    }
    return 1;
}

int dm1_v1_group_stop_attacking_f0182_pc34(
    struct GameWorld_Compat *world,
    int activeGroupIndex,
    int mapIndex,
    int mapX,
    int mapY,
    DM1_V1_GroupSensorReceiptPc34Compat *outReceipt)
{
    struct TimelineEvent_Compat staged[TIMELINE_QUEUE_CAPACITY];
    int groupIndex;
    int readIndex;
    int writeIndex = 0;
    int removed = 0;

    if (outReceipt) memset(outReceipt, 0, sizeof(*outReceipt));
    if (!world || !dm1_v1_validate_active_group_owner_pc34(
            world, activeGroupIndex, &groupIndex) ||
        !dm1_v1_map_coordinates_valid_pc34(world, mapIndex, mapX, mapY) ||
        world->timeline.count < 0 ||
        world->timeline.count > TIMELINE_QUEUE_CAPACITY) return 0;

    /* F0181's exact C29..C41 square compaction is staged before the four
     * F0182 Aspect bytes publish.  A damaged timeline can never leave a
     * half-stopped C04 group behind. */
    for (readIndex = 0; readIndex < world->timeline.count; ++readIndex) {
        const struct TimelineEvent_Compat *event = &world->timeline.events[readIndex];
        if (event->kind == TIMELINE_EVENT_CREATURE_REACTION &&
            event->mapIndex == mapIndex && event->mapX == mapX &&
            event->mapY == mapY &&
            event->aux2 >= DM1_EVENT_REACTION_DANGER_ON_SQUARE &&
            event->aux2 <= DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_3) {
            ++removed;
            continue;
        }
        staged[writeIndex++] = *event;
    }
    while (writeIndex < TIMELINE_QUEUE_CAPACITY) {
        memset(&staged[writeIndex], 0, sizeof(staged[writeIndex]));
        ++writeIndex;
    }
    memcpy(world->timeline.events, staged, sizeof(staged));
    world->timeline.count -= removed;
    for (readIndex = 0; readIndex < 4; ++readIndex) {
        world->creatureAI[activeGroupIndex].aspect[readIndex] &= 0x7fu;
    }
    if (outReceipt) {
        outReceipt->valid = 1;
        outReceipt->mapIndex = mapIndex;
        outReceipt->mapX = mapX;
        outReceipt->mapY = mapY;
        outReceipt->activeGroupIndex = activeGroupIndex;
        outReceipt->groupIndex = groupIndex;
        outReceipt->removedReactionCount = removed;
        outReceipt->matchedThing = (uint16_t)((THING_TYPE_GROUP << 10) | groupIndex);
        outReceipt->sourceSymbol = "F0182_GROUP_StopAttacking/F0181_GROUP_DeleteEvents";
    }
    return 1;
}

int dm1_v1_group_square_distance_f0226_pc34(
    const struct GameWorld_Compat *world,
    int mapIndex,
    int sourceMapX,
    int sourceMapY,
    int destinationMapX,
    int destinationMapY,
    DM1_V1_GroupSensorReceiptPc34Compat *outReceipt)
{
    int distanceX;
    int distanceY;

    if (outReceipt) memset(outReceipt, 0, sizeof(*outReceipt));
    if (!dm1_v1_map_coordinates_valid_pc34(world, mapIndex, sourceMapX, sourceMapY) ||
        !dm1_v1_map_coordinates_valid_pc34(world, mapIndex,
                                            destinationMapX, destinationMapY)) return 0;
    distanceX = sourceMapX - destinationMapX;
    distanceY = sourceMapY - destinationMapY;
    if (distanceX < 0) distanceX = -distanceX;
    if (distanceY < 0) distanceY = -distanceY;
    if (outReceipt) {
        outReceipt->valid = 1;
        outReceipt->mapIndex = mapIndex;
        outReceipt->mapX = sourceMapX;
        outReceipt->mapY = sourceMapY;
        outReceipt->squareDistance = distanceX + distanceY;
        outReceipt->sourceSymbol = "F0226_GROUP_GetDistanceBetweenSquares";
    }
    return 1;
}

int dm1_v1_sensor_get_object_of_type_f0273_pc34(
    const struct GameWorld_Compat *world,
    int mapIndex,
    int mapX,
    int mapY,
    int cell,
    uint16_t objectType,
    DM1_V1_GroupSensorReceiptPc34Compat *outReceipt)
{
    const struct DungeonThings_Compat *things;
    int sftIndex;
    uint16_t thing;
    int guard;

    if (outReceipt) memset(outReceipt, 0, sizeof(*outReceipt));
    if (!world || !world->things || !world->things->loaded ||
        (cell < -1 || cell > 3) ||
        !dm1_v1_map_coordinates_valid_pc34(world, mapIndex, mapX, mapY)) return 0;
    things = world->things;
    sftIndex = dm1_v1_square_first_thing_index_pc34(world->dungeon, mapIndex, mapX, mapY);
    if (sftIndex < 0) return 0;
    if (!things->squareFirstThings || sftIndex >= things->squareFirstThingCount) return 0;
    thing = things->squareFirstThings[sftIndex];
    for (guard = 0; guard < 1024 && thing != DM1_V1_THING_END_PC34 &&
         thing != DM1_V1_THING_NONE_PC34; ++guard) {
        int type = (int)((thing >> 10) & 0x0fu);
        int index = (int)(thing & 0x03ffu);
        int thingCell = (int)(thing >> 14);
        const unsigned char *raw;
        unsigned char recordBytes;
        uint16_t nextThing;

        if (type < 0 || type >= 16 || index < 0 ||
            index >= things->thingCounts[type] ||
            !(raw = things->rawThingData[type]) ||
            !(recordBytes = dm1_v1_thing_record_bytes_pc34[type])) return 0;
        raw += (size_t)index * recordBytes;
        nextThing = dm1_v1_read_u16_pc34(raw);
        if (type > THING_TYPE_GROUP && type < 14 && recordBytes >= 4 &&
            (dm1_v1_read_u16_pc34(raw + 2) & 0x007fu) == objectType &&
            (cell == -1 || thingCell == cell)) {
            if (outReceipt) {
                outReceipt->valid = 1;
                outReceipt->mapIndex = mapIndex;
                outReceipt->mapX = mapX;
                outReceipt->mapY = mapY;
                outReceipt->requestedThingType = objectType;
                outReceipt->requestedCell = cell;
                outReceipt->matchedThing = thing;
                outReceipt->matchedThingType = type;
                outReceipt->matchedThingIndex = index;
                outReceipt->matchedCell = thingCell;
                outReceipt->sourceSymbol = "F0273_SENSOR_GetObjectOfTypeInCell";
            }
            return 1;
        }
        thing = nextThing;
    }
    if (guard >= 1024) return 0;
    if (outReceipt) {
        outReceipt->valid = 1;
        outReceipt->mapIndex = mapIndex;
        outReceipt->mapX = mapX;
        outReceipt->mapY = mapY;
        outReceipt->requestedThingType = objectType;
        outReceipt->requestedCell = cell;
        outReceipt->matchedThing = DM1_V1_THING_NONE_PC34;
        outReceipt->matchedThingType = -1;
        outReceipt->matchedThingIndex = -1;
        outReceipt->matchedCell = -1;
        outReceipt->sourceSymbol = "F0273_SENSOR_GetObjectOfTypeInCell";
    }
    return 1;
}

const char *dm1_v1_group_state_bundle_source_evidence_pc34(void)
{
    return "ReDMCSB DUNGEON.C F0145-F0148:1264-1333 routes party-map "
           "Cells/Directions through ACTIVE_GROUP and other maps through C04; "
           "GROUP.C F0196:1135-1175 initializes ACTIVE_GROUP owners; "
           "LOADSAVE.C F0435:2180-2183 publishes new-game group runtime.";
}

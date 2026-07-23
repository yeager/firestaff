#include "dm1_v1_group_state_bundle_pc34_compat.h"

#include <string.h>

#define DM1_V1_GROUP_RAW_BYTES_PC34 16
#define DM1_V1_GROUP_TYPE_PC34 4

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

const char *dm1_v1_group_state_bundle_source_evidence_pc34(void)
{
    return "ReDMCSB DUNGEON.C F0145-F0148:1264-1333 routes party-map "
           "Cells/Directions through ACTIVE_GROUP and other maps through C04; "
           "GROUP.C F0196:1135-1175 initializes ACTIVE_GROUP owners; "
           "LOADSAVE.C F0435:2180-2183 publishes new-game group runtime.";
}

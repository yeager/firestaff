#include "dm1_v1_group_active_lifecycle_pc34_compat.h"

static const char s_source_evidence[] =
    "ReDMCSB GROUP.C F0182 lines 374-381 clears MASK0x0080_IS_ATTACKING "
    "from all four ACTIVE_GROUP Aspect bytes and then calls F0181 at the "
    "group map coordinate. F0195 lines 1104-1131 walks current-map squares, "
    "uses MASK0x0010_THING_LIST_PRESENT, scans the square thing chain until "
    "the first C04 group, then deletes square events, adds the active group, "
    "and starts wandering. F0196 lines 1134-1150 sets new-game maximum to "
    "110, allocates at least 110 ACTIVE_GROUP slots, and initializes every "
    "GroupThingIndex to -1.";

static int dm1_v1_f0195_thing_type(uint16_t thing)
{
    return (int)((thing >> 10) & 0x0Fu);
}

void F0182_GROUP_StopAttacking(
    DM1_V1_F0182_ActiveGroupPc34Compat* activeGroup,
    int16_t mapX,
    int16_t mapY,
    DM1_V1_F0182_DeleteEventsPc34Compat deleteEvents,
    void* deleteEventsContext)
{
    int creatureIndex;

    if (!activeGroup) {
        return;
    }
    for (creatureIndex = 0; creatureIndex < 4; ++creatureIndex) {
        activeGroup->aspect[creatureIndex] =
            (uint8_t)(activeGroup->aspect[creatureIndex] &
                      (uint8_t)~DM1_V1_F0182_ATTACKING_ASPECT_MASK_PC34);
    }
    if (deleteEvents) {
        deleteEvents(deleteEventsContext, mapX, mapY);
    }
}

int F0195_GROUP_AddAllActiveGroups(
    const DM1_V1_F0195_AddAllActiveGroupsInputPc34Compat* input)
{
    size_t squareIndex = 0u;
    size_t firstThingIndex = 0u;
    int16_t mapX;
    int16_t mapY;
    int addedCount = 0;

    if (!input || !input->mapSquares || !input->squareFirstThings ||
        !input->getNextThing || input->mapWidth <= 0 ||
        input->mapHeight <= 0) {
        return -1;
    }
    if ((size_t)input->mapWidth * (size_t)input->mapHeight >
        input->mapSquareCount) {
        return -1;
    }

    for (mapX = 0; mapX < input->mapWidth; ++mapX) {
        for (mapY = 0; mapY < input->mapHeight; ++mapY) {
            uint8_t square = input->mapSquares[squareIndex++];
            uint16_t thing;
            if ((square & DM1_V1_F0195_SQUARE_THING_LIST_PRESENT_PC34) == 0u) {
                continue;
            }
            if (firstThingIndex >= input->squareFirstThingCount) {
                return -1;
            }
            thing = input->squareFirstThings[firstThingIndex++];
            while (thing != DM1_V1_F0195_THING_END_OF_LIST_PC34) {
                if (dm1_v1_f0195_thing_type(thing) ==
                    DM1_V1_F0195_GROUP_THING_TYPE_PC34) {
                    if (input->deleteEvents) {
                        input->deleteEvents(input->deleteEventsContext,
                                            thing, mapX, mapY);
                    }
                    if (input->addActiveGroup) {
                        input->addActiveGroup(input->addActiveGroupContext,
                                              thing, mapX, mapY);
                    }
                    if (input->startWandering) {
                        input->startWandering(input->startWanderingContext,
                                              thing, mapX, mapY);
                    }
                    ++addedCount;
                    break;
                }
                thing = input->getNextThing(input->getNextThingContext, thing);
            }
        }
    }

    return addedCount;
}

int F0196_GROUP_InitializeActiveGroups(
    DM1_V1_F0196_InitializeActiveGroupsInputPc34Compat* input)
{
    size_t i;
    size_t initializedCount;

    if (!input || !input->slots) {
        return -1;
    }
    if (input->newGame) {
        input->maximumActiveGroupCount =
            DM1_V1_F0196_MINIMUM_ACTIVE_GROUP_COUNT_PC34;
    }
    initializedCount = input->maximumActiveGroupCount;
    if (initializedCount < DM1_V1_F0196_MINIMUM_ACTIVE_GROUP_COUNT_PC34) {
        initializedCount = DM1_V1_F0196_MINIMUM_ACTIVE_GROUP_COUNT_PC34;
    }
    if (initializedCount > input->slotCapacity) {
        return -1;
    }
    for (i = 0u; i < initializedCount; ++i) {
        input->slots[i].groupThingIndex = -1;
    }
    return (int)initializedCount;
}

const char* DM1_V1_GroupActiveLifecycle_SourceEvidencePc34(void)
{
    return s_source_evidence;
}

#include "dm1_v1_group_timeline_f0179_f0181_pc34_compat.h"

#include <string.h>

const char *DM1_V1_F0179_F0180_F0181_SourceEvidencePc34(void)
{
    return
        "GROUP.C:187-308 F0179_GROUP_GetCreatureAspectUpdateTime masks "
        "ACTIVE_GROUP Aspect to FLIP_BITMAP|IS_ATTACKING, writes bounded "
        "horizontal/vertical offset bits, and schedules the next C32..C36 "
        "aspect update event\n"
        "GROUP.C:311-338 F0180_GROUP_StartWandering creates a C37 "
        "UPDATE_BEHAVIOR_GROUP event at current game time + 1\n"
        "GROUP.C:340-371 F0181_GROUP_DeleteEvents deletes C29..C41 group "
        "timeline events matching the group map coordinates";
}

static uint8_t bounded_offset(uint16_t randomValue, int maximum, int shift)
{
    int value;

    if (maximum <= 0) {
        return 0;
    }
    value = (int)((randomValue >> shift) % (uint16_t)maximum);
    if (randomValue & 1u) {
        value = (-value) & DM1_V1_F0179_ASPECT_HORIZONTAL_MASK_PC34;
    }
    return (uint8_t)(value & DM1_V1_F0179_ASPECT_HORIZONTAL_MASK_PC34);
}

static int event_type_to_creature_index(uint8_t eventType)
{
    if (eventType == DM1_EVENT_UPDATE_ASPECT_GROUP) {
        return -1;
    }
    if (eventType >= DM1_EVENT_UPDATE_ASPECT_CREATURE_0 &&
        eventType <= DM1_EVENT_UPDATE_ASPECT_CREATURE_3) {
        return (int)(eventType - DM1_EVENT_UPDATE_ASPECT_CREATURE_0);
    }
    return -2;
}

int F0179_GROUP_GetCreatureAspectUpdateTime(
    const DM1_V1_GroupAspectUpdateInputF0179Pc34 *input,
    DM1_V1_GroupAspectUpdateResultF0179Pc34 *out)
{
    int creatureIndex;
    int maxHorizontal;
    int maxVertical;
    uint8_t baseAspect;
    uint8_t horizontal;
    uint8_t vertical;
    uint8_t updatedAspect;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    out->updatedCreatureIndex = -1;

    if (!input || input->updateDelay == 0) {
        return 0;
    }
    creatureIndex = event_type_to_creature_index(input->eventType);
    if (creatureIndex == -2) {
        return 0;
    }
    if (creatureIndex < 0) {
        creatureIndex = 0;
    }

    maxHorizontal = (int)((input->graphicInfo >> 12) & 0x0003u);
    maxVertical = (int)((input->graphicInfo >> 14) & 0x0003u);
    baseAspect = (uint8_t)(input->aspect[creatureIndex] &
        (DM1_V1_F0179_ASPECT_FLIP_BITMAP_PC34 |
         DM1_V1_F0179_ASPECT_IS_ATTACKING_PC34));
    horizontal = bounded_offset(input->randomValue, maxHorizontal, 1);
    vertical = bounded_offset(input->randomValue, maxVertical, 5);
    updatedAspect = (uint8_t)(baseAspect |
        horizontal |
        ((vertical << DM1_V1_F0179_ASPECT_VERTICAL_SHIFT_PC34) &
         DM1_V1_F0179_ASPECT_VERTICAL_MASK_PC34));

    out->valid = 1;
    out->updatedCreatureIndex = creatureIndex;
    out->previousAspect = input->aspect[creatureIndex];
    out->updatedAspect = updatedAspect;
    out->updateTime = input->gameTime + input->updateDelay;
    out->nextEvent.map_time =
        DM1_MAP_TIME_MAKE(input->mapIndex, out->updateTime);
    out->nextEvent.type = input->eventType;
    out->nextEvent.priority = 0;
    out->nextEvent.b_mapX = input->mapX;
    out->nextEvent.b_mapY = input->mapY;
    out->nextEvent.c_cell = (uint8_t)creatureIndex;
    out->nextEvent.c_effect = updatedAspect;
    return 1;
}

int F0180_GROUP_StartWandering(
    const DM1_V1_StartWanderingInputF0180Pc34 *input,
    DM1_V1_StartWanderingResultF0180Pc34 *out)
{
    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    if (!input) {
        return 0;
    }

    out->valid = 1;
    out->updateTime = input->gameTime + 1u;
    out->event.map_time = DM1_MAP_TIME_MAKE(input->mapIndex, out->updateTime);
    out->event.type = DM1_EVENT_UPDATE_BEHAVIOR_GROUP;
    out->event.priority = input->priority;
    out->event.b_mapX = input->mapX;
    out->event.b_mapY = input->mapY;
    out->event.c_cell = input->groupCell;
    out->event.c_effect = input->groupThingIndex;
    return 1;
}

int F0181_GROUP_DeleteEvents(
    struct DM1_Event_V1 *events,
    size_t eventCount,
    int mapX,
    int mapY,
    DM1_V1_DeleteEventsResultF0181Pc34 *out)
{
    size_t i;
    size_t deleted = 0;

    if (out) {
        memset(out, 0, sizeof(*out));
    }
    if (!events || mapX < 0 || mapX > 255 || mapY < 0 || mapY > 255) {
        return -1;
    }

    for (i = 0; i < eventCount; ++i) {
        if (out) {
            out->scannedEventCount += 1u;
        }
        if (events[i].type >= DM1_V1_F0181_GROUP_EVENT_FIRST_PC34 &&
            events[i].type <= DM1_V1_F0181_GROUP_EVENT_LAST_PC34 &&
            events[i].b_mapX == (uint8_t)mapX &&
            events[i].b_mapY == (uint8_t)mapY) {
            events[i].type = DM1_EVENT_NONE;
            ++deleted;
        }
    }
    if (out) {
        out->deletedEventCount = deleted;
    }
    return (int)deleted;
}

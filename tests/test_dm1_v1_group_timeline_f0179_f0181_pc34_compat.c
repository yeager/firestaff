#include "dm1_v1_group_timeline_f0179_f0181_pc34_compat.h"

#include <assert.h>
#include <string.h>

static void test_f0179_aspect_update(void)
{
    DM1_V1_GroupAspectUpdateInputF0179Pc34 input;
    DM1_V1_GroupAspectUpdateResultF0179Pc34 result;
    (void)result;

    memset(&input, 0, sizeof(input));
    input.aspect[2] = 0xffu;
    input.graphicInfo = (uint16_t)((3u << 12) | (2u << 14));
    input.gameTime = 1000u;
    input.randomValue = 0x0045u;
    input.updateDelay = 7u;
    input.eventType = DM1_EVENT_UPDATE_ASPECT_CREATURE_2;
    input.mapIndex = 4u;
    input.mapX = 12u;
    input.mapY = 9u;

    assert(F0179_GROUP_GetCreatureAspectUpdateTime(&input, &result) == 1);
    assert(result.valid == 1);
    assert(result.updatedCreatureIndex == 2);
    assert(result.previousAspect == 0xffu);
    assert((result.updatedAspect &
            (DM1_V1_F0179_ASPECT_FLIP_BITMAP_PC34 |
             DM1_V1_F0179_ASPECT_IS_ATTACKING_PC34)) == 0xc0u);
    assert((result.updatedAspect &
            DM1_V1_F0179_ASPECT_HORIZONTAL_MASK_PC34) == 7u);
    assert(((result.updatedAspect &
             DM1_V1_F0179_ASPECT_VERTICAL_MASK_PC34) >>
            DM1_V1_F0179_ASPECT_VERTICAL_SHIFT_PC34) == 0u);
    assert(result.updateTime == 1007u);
    assert(DM1_MAP_TIME_MAP(result.nextEvent.map_time) == 4u);
    assert(DM1_MAP_TIME_TIME(result.nextEvent.map_time) == 1007u);
    assert(result.nextEvent.type == DM1_EVENT_UPDATE_ASPECT_CREATURE_2);
    assert(result.nextEvent.b_mapX == 12u);
    assert(result.nextEvent.b_mapY == 9u);
    assert(result.nextEvent.c_cell == 2u);
    assert(result.nextEvent.c_effect == result.updatedAspect);

    input.eventType = DM1_EVENT_UPDATE_ASPECT_GROUP;
    input.aspect[0] = DM1_V1_F0179_ASPECT_FLIP_BITMAP_PC34 | 0x3fu;
    input.graphicInfo = 0u;
    input.randomValue = 0xffffu;
    assert(F0179_GROUP_GetCreatureAspectUpdateTime(&input, &result) == 1);
    assert(result.updatedCreatureIndex == 0);
    assert(result.updatedAspect == DM1_V1_F0179_ASPECT_FLIP_BITMAP_PC34);
}

static void test_f0180_start_wandering_event(void)
{
    DM1_V1_StartWanderingInputF0180Pc34 input;
    DM1_V1_StartWanderingResultF0180Pc34 result;
    (void)result;

    memset(&input, 0, sizeof(input));
    input.gameTime = 77u;
    input.mapIndex = 3u;
    input.mapX = 6u;
    input.mapY = 11u;
    input.groupCell = 2u;
    input.groupThingIndex = 44u;
    input.priority = 5u;

    assert(F0180_GROUP_StartWandering(&input, &result) == 1);
    assert(result.valid == 1);
    assert(result.updateTime == 78u);
    assert(DM1_MAP_TIME_MAP(result.event.map_time) == 3u);
    assert(DM1_MAP_TIME_TIME(result.event.map_time) == 78u);
    assert(result.event.type == DM1_EVENT_UPDATE_BEHAVIOR_GROUP);
    assert(result.event.priority == 5u);
    assert(result.event.b_mapX == 6u);
    assert(result.event.b_mapY == 11u);
    assert(result.event.c_cell == 2u);
    assert(result.event.c_effect == 44u);
}

static void test_f0181_delete_group_events(void)
{
    struct DM1_Event_V1 events[8];
    DM1_V1_DeleteEventsResultF0181Pc34 result;
    (void)result;

    memset(events, 0, sizeof(events));
    events[0].type = DM1_EVENT_GROUP_REACTION_DANGER_ON_SQUARE;
    events[0].b_mapX = 4u;
    events[0].b_mapY = 5u;
    events[1].type = DM1_EVENT_UPDATE_ASPECT_CREATURE_3;
    events[1].b_mapX = 4u;
    events[1].b_mapY = 5u;
    events[2].type = DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_3;
    events[2].b_mapX = 4u;
    events[2].b_mapY = 5u;
    events[3].type = DM1_EVENT_UPDATE_BEHAVIOR_GROUP;
    events[3].b_mapX = 4u;
    events[3].b_mapY = 6u;
    events[4].type = DM1_EVENT_MOVE_GROUP_AUDIBLE;
    events[4].b_mapX = 4u;
    events[4].b_mapY = 5u;
    events[5].type = DM1_EVENT_DOOR;
    events[5].b_mapX = 4u;
    events[5].b_mapY = 5u;

    assert(F0181_GROUP_DeleteEvents(events, 8, 4, 5, &result) == 3);
    assert(result.scannedEventCount == 8u);
    assert(result.deletedEventCount == 3u);
    assert(events[0].type == DM1_EVENT_NONE);
    assert(events[1].type == DM1_EVENT_NONE);
    assert(events[2].type == DM1_EVENT_NONE);
    assert(events[3].type == DM1_EVENT_UPDATE_BEHAVIOR_GROUP);
    assert(events[4].type == DM1_EVENT_MOVE_GROUP_AUDIBLE);
    assert(events[5].type == DM1_EVENT_DOOR);
}

static void test_fail_closed(void)
{
    DM1_V1_GroupAspectUpdateInputF0179Pc34 aspectInput;
    DM1_V1_GroupAspectUpdateResultF0179Pc34 aspectResult;
    (void)aspectResult;
    DM1_V1_StartWanderingInputF0180Pc34 wanderInput;
    DM1_V1_StartWanderingResultF0180Pc34 wanderResult;
    (void)wanderResult;

    memset(&aspectInput, 0, sizeof(aspectInput));
    aspectInput.eventType = DM1_EVENT_UPDATE_ASPECT_CREATURE_0;
    aspectInput.updateDelay = 0u;
    assert(F0179_GROUP_GetCreatureAspectUpdateTime(
               &aspectInput, &aspectResult) == 0);
    aspectInput.updateDelay = 1u;
    aspectInput.eventType = DM1_EVENT_DOOR;
    assert(F0179_GROUP_GetCreatureAspectUpdateTime(
               &aspectInput, &aspectResult) == 0);
    assert(F0179_GROUP_GetCreatureAspectUpdateTime(0, &aspectResult) == 0);
    assert(F0179_GROUP_GetCreatureAspectUpdateTime(&aspectInput, 0) == 0);

    memset(&wanderInput, 0, sizeof(wanderInput));
    assert(F0180_GROUP_StartWandering(&wanderInput, &wanderResult) == 1);
    assert(F0180_GROUP_StartWandering(0, &wanderResult) == 0);
    assert(F0180_GROUP_StartWandering(&wanderInput, 0) == 0);

    assert(F0181_GROUP_DeleteEvents(0, 0, 1, 1, 0) == -1);
    assert(F0181_GROUP_DeleteEvents(&wanderResult.event, 1, -1, 1, 0) == -1);
}

int main(void)
{
    const char *evidence = DM1_V1_F0179_F0180_F0181_SourceEvidencePc34();
    (void)evidence;

    assert(strstr(evidence, "F0179_GROUP_GetCreatureAspectUpdateTime") != 0);
    assert(strstr(evidence, "F0180_GROUP_StartWandering") != 0);
    assert(strstr(evidence, "F0181_GROUP_DeleteEvents") != 0);

    test_f0179_aspect_update();
    test_f0180_start_wandering_event();
    test_f0181_delete_group_events();
    test_fail_closed();

    return 0;
}

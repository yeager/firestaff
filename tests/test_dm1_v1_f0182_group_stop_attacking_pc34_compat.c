#include "memory_tick_orchestrator_pc34_compat.h"

#include <stdio.h>
#include <string.h>

#define CHECK(condition, label) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAIL: %s\\n", label); \
        return 1; \
    } \
} while (0)

static struct TimelineEvent_Compat reaction(
    int mapIndex, int mapX, int mapY, int eventType)
{
    struct TimelineEvent_Compat event;

    memset(&event, 0, sizeof(event));
    event.kind = TIMELINE_EVENT_CREATURE_REACTION;
    event.mapIndex = mapIndex;
    event.mapX = mapX;
    event.mapY = mapY;
    event.aux2 = eventType;
    return event;
}

int main(void)
{
    struct GameWorld_Compat world;
    struct DM1ActiveGroup_Compat activeGroup;

    memset(&world, 0, sizeof(world));
    memset(&activeGroup, 0, sizeof(activeGroup));
    activeGroup.aspect[0] = 0x80;
    activeGroup.aspect[1] = 0xff;
    activeGroup.aspect[2] = 0x7f;
    activeGroup.aspect[3] = 0x01;

    world.timeline.count = 5;
    world.timeline.events[0] = reaction(3, 10, 11,
        DM1_EVENT_REACTION_DANGER_ON_SQUARE);
    world.timeline.events[1] = reaction(3, 10, 11,
        DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_3);
    world.timeline.events[2] = reaction(3, 10, 11,
        DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_3 + 1);
    world.timeline.events[3] = reaction(3, 10, 12,
        DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0);
    world.timeline.events[4] = reaction(4, 10, 11,
        DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0);

    CHECK(F0182_DM1_GROUP_StopAttacking_Compat(
              &world, &activeGroup, 3, 10, 11),
          "F0182 accepts a valid active group and timeline");
    CHECK(activeGroup.aspect[0] == 0x00,
          "F0182 clears first attacking bit");
    CHECK(activeGroup.aspect[1] == 0x7f,
          "F0182 clears only attacking bit");
    CHECK(activeGroup.aspect[2] == 0x7f && activeGroup.aspect[3] == 0x01,
          "F0182 preserves non-attacking aspect bits");
    CHECK(world.timeline.count == 3,
          "F0182 removes only C29 through C41 on its exact square");
    CHECK(world.timeline.events[0].aux2 ==
              DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_3 + 1 &&
          world.timeline.events[1].mapY == 12 &&
          world.timeline.events[2].mapIndex == 4,
          "F0182 retains out-of-range and other-square reaction events");

    puts("PASS: DM1 F0182 source-locked stop-attacking cleanup");
    return 0;
}

#include "dm1_v1_creature_behavior_bootstrap_pc34_compat.h"
#include "dm1_v1_event_timer_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"
#include "memory_timeline_pc34_compat.h"
#include "memory_tick_orchestrator_pc34_compat.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* Integration test: bootstrap seeds events → F0884 dispatches them.
 * Uses a minimal world with one living group to verify the event flows
 * through the M10 tick orchestrator. */

static struct DungeonGroup_Compat s_groups[2];
static struct DungeonThings_Compat s_things;

typedef struct {
    int map;
    int x;
    int y;
    int valid;
} MockPos;

static MockPos s_pos[2];

static int mock_find(void *ctx, int gi, int *om, int *ox, int *oy)
{
    (void)ctx;
    if (gi < 0 || gi >= 2 || !s_pos[gi].valid) return 0;
    *om = s_pos[gi].map;
    *ox = s_pos[gi].x;
    *oy = s_pos[gi].y;
    return 1;
}

static void test_bootstrap_seeds_dispatchable_event(void)
{
    struct GameWorld_Compat world;
    DM1_V1_CreatureBehaviorBootstrapResultPc34 result;
    struct TimelineEvent_Compat ev;

    memset(&world, 0, sizeof(world));
    memset(&s_things, 0, sizeof(s_things));
    memset(s_groups, 0, sizeof(s_groups));
    memset(s_pos, 0, sizeof(s_pos));

    s_groups[0].creatureType = 10;
    s_groups[0].count = 0;
    s_groups[0].health[0] = 100;
    s_groups[0].next = THING_ENDOFLIST;
    s_things.groups = s_groups;
    s_things.groupCount = 1;

    s_pos[0].valid = 1;
    s_pos[0].map = 3;
    s_pos[0].x = 5;
    s_pos[0].y = 8;

    world.things = &s_things;
    world.gameTick = 50;
    F0720_TIMELINE_Init_Compat(&world.timeline, world.gameTick);

    assert(dm1_v1_creature_behavior_bootstrap_pc34(
        &world, mock_find, NULL, &result) == 1);
    assert(result.events_scheduled == 1);

    assert(F0722_TIMELINE_Peek_Compat(&world.timeline, &ev) == 1);
    assert(ev.kind == TIMELINE_EVENT_CREATURE_REACTION);
    assert(ev.aux2 == DM1_EVENT_UPDATE_BEHAVIOR_GROUP);
    assert(ev.fireAtTick == 51);
    assert(ev.mapIndex == 3);
    assert(ev.aux0 == 0);
    assert(ev.aux1 == 10);

    printf("  PASS: bootstrap_seeds_dispatchable_event\n");
}

static void test_event_fires_at_tick(void)
{
    struct GameWorld_Compat world;
    DM1_V1_CreatureBehaviorBootstrapResultPc34 result;
    struct TimelineEvent_Compat ev;

    memset(&world, 0, sizeof(world));
    memset(&s_things, 0, sizeof(s_things));
    memset(s_groups, 0, sizeof(s_groups));
    memset(s_pos, 0, sizeof(s_pos));

    s_groups[0].creatureType = 2;
    s_groups[0].count = 1;
    s_groups[0].health[0] = 50;
    s_groups[0].health[1] = 40;
    s_groups[0].next = THING_ENDOFLIST;
    s_things.groups = s_groups;
    s_things.groupCount = 1;

    s_pos[0].valid = 1;
    s_pos[0].map = 0;
    s_pos[0].x = 1;
    s_pos[0].y = 1;

    world.things = &s_things;
    world.gameTick = 10;
    F0720_TIMELINE_Init_Compat(&world.timeline, world.gameTick);

    dm1_v1_creature_behavior_bootstrap_pc34(&world, mock_find, NULL, &result);
    assert(result.events_scheduled == 1);

    assert(F0722_TIMELINE_Peek_Compat(&world.timeline, &ev) == 1);
    assert(ev.fireAtTick == 11);
    assert(ev.fireAtTick > world.gameTick);

    printf("  PASS: event_fires_at_tick\n");
}

static void test_multiple_groups_multiple_events(void)
{
    struct GameWorld_Compat world;
    DM1_V1_CreatureBehaviorBootstrapResultPc34 result;

    memset(&world, 0, sizeof(world));
    memset(&s_things, 0, sizeof(s_things));
    memset(s_groups, 0, sizeof(s_groups));
    memset(s_pos, 0, sizeof(s_pos));

    s_groups[0].creatureType = 5;
    s_groups[0].count = 0;
    s_groups[0].health[0] = 30;
    s_groups[0].next = THING_ENDOFLIST;

    s_groups[1].creatureType = 15;
    s_groups[1].count = 0;
    s_groups[1].health[0] = 60;
    s_groups[1].next = THING_ENDOFLIST;

    s_things.groups = s_groups;
    s_things.groupCount = 2;

    s_pos[0].valid = 1;
    s_pos[0].map = 0;
    s_pos[0].x = 2;
    s_pos[0].y = 3;

    s_pos[1].valid = 1;
    s_pos[1].map = 1;
    s_pos[1].x = 4;
    s_pos[1].y = 6;

    world.things = &s_things;
    world.gameTick = 200;
    F0720_TIMELINE_Init_Compat(&world.timeline, world.gameTick);

    dm1_v1_creature_behavior_bootstrap_pc34(&world, mock_find, NULL, &result);
    assert(result.groups_found == 2);
    assert(result.events_scheduled == 2);
    assert(world.timeline.count == 2);

    printf("  PASS: multiple_groups_multiple_events\n");
}

int main(void)
{
    test_bootstrap_seeds_dispatchable_event();
    test_event_fires_at_tick();
    test_multiple_groups_multiple_events();

    printf("PASS: dm1_v1_creature_behavior_bootstrap_integration (3 tests)\n");
    return 0;
}

#include "dm1_v1_creature_behavior_bootstrap_pc34_compat.h"
#include "dm1_v1_event_timer_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"
#include "memory_timeline_pc34_compat.h"
#include "memory_tick_orchestrator_pc34_compat.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

#define MAX_GROUPS 8

static struct DungeonGroup_Compat s_groups[MAX_GROUPS];
static struct DungeonThings_Compat s_things;

typedef struct {
    int map[MAX_GROUPS];
    int x[MAX_GROUPS];
    int y[MAX_GROUPS];
    int valid[MAX_GROUPS];
} MockGroupPositions;

static MockGroupPositions s_positions;

static int mock_find_group(void *ctx, int group_index,
                           int *out_map, int *out_x, int *out_y)
{
    (void)ctx;
    if (group_index < 0 || group_index >= MAX_GROUPS) return 0;
    if (!s_positions.valid[group_index]) return 0;
    *out_map = s_positions.map[group_index];
    *out_x = s_positions.x[group_index];
    *out_y = s_positions.y[group_index];
    return 1;
}

static void init_test_world(struct GameWorld_Compat *world)
{
    memset(world, 0, sizeof(*world));
    memset(&s_things, 0, sizeof(s_things));
    memset(s_groups, 0, sizeof(s_groups));
    memset(&s_positions, 0, sizeof(s_positions));

    s_things.groups = s_groups;
    s_things.groupCount = 0;

    world->things = &s_things;
    world->gameTick = 100;

    F0720_TIMELINE_Init_Compat(&world->timeline, world->gameTick);
}

static void test_null_safety(void)
{
    assert(dm1_v1_creature_behavior_bootstrap_pc34(NULL, mock_find_group, NULL, NULL) == 0);
    printf("  PASS: null_safety\n");
}

static void test_null_callback(void)
{
    struct GameWorld_Compat world;
    init_test_world(&world);
    assert(dm1_v1_creature_behavior_bootstrap_pc34(&world, NULL, NULL, NULL) == 0);
    printf("  PASS: null_callback\n");
}

static void test_empty_dungeon(void)
{
    struct GameWorld_Compat world;
    DM1_V1_CreatureBehaviorBootstrapResultPc34 result;

    init_test_world(&world);
    assert(dm1_v1_creature_behavior_bootstrap_pc34(&world, mock_find_group, NULL, &result) == 1);
    assert(result.groups_found == 0);
    assert(result.events_scheduled == 0);
    printf("  PASS: empty_dungeon\n");
}

static void test_one_living_group(void)
{
    struct GameWorld_Compat world;
    DM1_V1_CreatureBehaviorBootstrapResultPc34 result;
    struct TimelineEvent_Compat ev;

    init_test_world(&world);

    s_groups[0].creatureType = 5;
    s_groups[0].count = 0;
    s_groups[0].health[0] = 50;
    s_groups[0].next = THING_ENDOFLIST;
    s_things.groupCount = 1;

    s_positions.valid[0] = 1;
    s_positions.map[0] = 2;
    s_positions.x[0] = 3;
    s_positions.y[0] = 7;

    assert(dm1_v1_creature_behavior_bootstrap_pc34(&world, mock_find_group, NULL, &result) == 1);
    assert(result.groups_found == 1);
    assert(result.events_scheduled == 1);

    assert(F0722_TIMELINE_Peek_Compat(&world.timeline, &ev) == 1);
    assert(ev.kind == TIMELINE_EVENT_CREATURE_REACTION);
    assert(ev.fireAtTick == 101);
    assert(ev.mapIndex == 2);
    assert(ev.mapX == 3);
    assert(ev.mapY == 7);
    assert(ev.aux0 == 0);
    assert(ev.aux1 == 5);
    assert(ev.aux2 == DM1_EVENT_UPDATE_BEHAVIOR_GROUP);
    printf("  PASS: one_living_group\n");
}

static void test_dead_group_skipped(void)
{
    struct GameWorld_Compat world;
    DM1_V1_CreatureBehaviorBootstrapResultPc34 result;

    init_test_world(&world);

    s_groups[0].creatureType = 3;
    s_groups[0].count = 1;
    s_groups[0].health[0] = 0;
    s_groups[0].health[1] = 0;
    s_groups[0].next = THING_ENDOFLIST;
    s_things.groupCount = 1;

    s_positions.valid[0] = 1;
    s_positions.map[0] = 0;
    s_positions.x[0] = 1;
    s_positions.y[0] = 1;

    assert(dm1_v1_creature_behavior_bootstrap_pc34(&world, mock_find_group, NULL, &result) == 1);
    assert(result.groups_found == 0);
    assert(result.events_scheduled == 0);
    printf("  PASS: dead_group_skipped\n");
}

static void test_multiple_groups(void)
{
    struct GameWorld_Compat world;
    DM1_V1_CreatureBehaviorBootstrapResultPc34 result;

    init_test_world(&world);

    s_groups[0].creatureType = 1;
    s_groups[0].count = 0;
    s_groups[0].health[0] = 30;
    s_groups[0].next = THING_ENDOFLIST;

    s_groups[1].creatureType = 7;
    s_groups[1].count = 2;
    s_groups[1].health[0] = 100;
    s_groups[1].health[1] = 80;
    s_groups[1].health[2] = 60;
    s_groups[1].next = THING_ENDOFLIST;

    s_groups[2].creatureType = 12;
    s_groups[2].count = 0;
    s_groups[2].health[0] = 0;
    s_groups[2].next = THING_ENDOFLIST;

    s_things.groupCount = 3;

    s_positions.valid[0] = 1;
    s_positions.map[0] = 0;
    s_positions.x[0] = 0;
    s_positions.y[0] = 0;

    s_positions.valid[1] = 1;
    s_positions.map[1] = 1;
    s_positions.x[1] = 5;
    s_positions.y[1] = 3;

    assert(dm1_v1_creature_behavior_bootstrap_pc34(&world, mock_find_group, NULL, &result) == 1);
    assert(result.groups_found == 2);
    assert(result.events_scheduled == 2);
    assert(world.timeline.count == 2);
    printf("  PASS: multiple_groups\n");
}

static void test_group_not_on_map(void)
{
    struct GameWorld_Compat world;
    DM1_V1_CreatureBehaviorBootstrapResultPc34 result;

    init_test_world(&world);

    s_groups[0].creatureType = 4;
    s_groups[0].count = 0;
    s_groups[0].health[0] = 25;
    s_groups[0].next = THING_ENDOFLIST;
    s_things.groupCount = 1;

    assert(dm1_v1_creature_behavior_bootstrap_pc34(&world, mock_find_group, NULL, &result) == 1);
    assert(result.groups_found == 1);
    assert(result.events_scheduled == 0);
    printf("  PASS: group_not_on_map\n");
}

static void test_already_scheduled(void)
{
    struct GameWorld_Compat world;
    DM1_V1_CreatureBehaviorBootstrapResultPc34 result;
    struct TimelineEvent_Compat existing;

    init_test_world(&world);

    s_groups[0].creatureType = 9;
    s_groups[0].count = 0;
    s_groups[0].health[0] = 75;
    s_groups[0].next = THING_ENDOFLIST;
    s_things.groupCount = 1;

    s_positions.valid[0] = 1;
    s_positions.map[0] = 0;
    s_positions.x[0] = 2;
    s_positions.y[0] = 4;

    memset(&existing, 0, sizeof(existing));
    existing.kind = TIMELINE_EVENT_CREATURE_REACTION;
    existing.fireAtTick = 105;
    existing.mapIndex = 0;
    existing.mapX = 2;
    existing.mapY = 4;
    existing.aux0 = 0;
    existing.aux1 = 9;
    existing.aux2 = DM1_EVENT_UPDATE_BEHAVIOR_GROUP;
    F0721_TIMELINE_Schedule_Compat(&world.timeline, &existing);

    assert(dm1_v1_creature_behavior_bootstrap_pc34(&world, mock_find_group, NULL, &result) == 1);
    assert(result.groups_found == 1);
    assert(result.already_scheduled == 1);
    assert(result.events_scheduled == 0);
    assert(world.timeline.count == 1);
    printf("  PASS: already_scheduled\n");
}

static void test_idempotent(void)
{
    struct GameWorld_Compat world;
    DM1_V1_CreatureBehaviorBootstrapResultPc34 result;

    init_test_world(&world);

    s_groups[0].creatureType = 2;
    s_groups[0].count = 0;
    s_groups[0].health[0] = 40;
    s_groups[0].next = THING_ENDOFLIST;
    s_things.groupCount = 1;

    s_positions.valid[0] = 1;
    s_positions.map[0] = 0;
    s_positions.x[0] = 1;
    s_positions.y[0] = 1;

    assert(dm1_v1_creature_behavior_bootstrap_pc34(&world, mock_find_group, NULL, &result) == 1);
    assert(result.events_scheduled == 1);

    assert(dm1_v1_creature_behavior_bootstrap_pc34(&world, mock_find_group, NULL, &result) == 1);
    assert(result.already_scheduled == 1);
    assert(result.events_scheduled == 0);
    assert(world.timeline.count == 1);
    printf("  PASS: idempotent\n");
}

int main(void)
{
    test_null_safety();
    test_null_callback();
    test_empty_dungeon();
    test_one_living_group();
    test_dead_group_skipped();
    test_multiple_groups();
    test_group_not_on_map();
    test_already_scheduled();
    test_idempotent();

    printf("PASS: dm1_v1_creature_behavior_bootstrap (9 tests)\n");
    return 0;
}

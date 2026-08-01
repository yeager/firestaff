#include "theron_v1_track02_creature_names.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

static void test_creature_count(void)
{
    assert(THERON_TRACK02_CREATURE_TYPE_COUNT == 7);
    printf("  PASS: creature_count\n");
}

static void test_creature_names(void)
{
    assert(strcmp(theron_v1_track02_us_creature_name(0), "AKUTUBA") == 0);
    assert(strcmp(theron_v1_track02_us_creature_name(1), "DRATOR") == 0);
    assert(strcmp(theron_v1_track02_us_creature_name(2), "FORMIC") == 0);
    assert(strcmp(theron_v1_track02_us_creature_name(3), "SARMON") == 0);
    assert(strcmp(theron_v1_track02_us_creature_name(4), "SHADO") == 0);
    assert(strcmp(theron_v1_track02_us_creature_name(5), "THIEF") == 0);
    assert(strcmp(theron_v1_track02_us_creature_name(6), "DEMON") == 0);
    printf("  PASS: creature_names\n");
}

static void test_bounds(void)
{
    assert(theron_v1_track02_us_creature_name(7) == NULL);
    assert(theron_v1_track02_us_creature_name(255) == NULL);
    printf("  PASS: bounds\n");
}

static void test_game_speed(void)
{
    assert(strcmp(theron_v1_track02_us_game_speed_label(), "GAME SPEED") == 0);
    printf("  PASS: game_speed\n");
}

int main(void)
{
    printf("test_theron_v1_track02_creature_names:\n");
    test_creature_count();
    test_creature_names();
    test_bounds();
    test_game_speed();
    printf("All tests passed.\n");
    return 0;
}

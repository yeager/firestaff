#include "dm1_v1_group_destination_visibility_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_direction_constants(void)
{
    assert(DM1_V1_GROUP_DIRECTION_NORTH_PC34 == 0);
    assert(DM1_V1_GROUP_DIRECTION_EAST_PC34 == 1);
    assert(DM1_V1_GROUP_DIRECTION_SOUTH_PC34 == 2);
    assert(DM1_V1_GROUP_DIRECTION_WEST_PC34 == 3);
}

static void test_north_visible(void)
{
    int r = F0227_GROUP_IsDestinationVisibleFromSource(0, 5, 5, 5, 3);
    (void)r;
    assert(r != 0);
}

static void test_north_not_visible_behind(void)
{
    int r = F0227_GROUP_IsDestinationVisibleFromSource(0, 5, 5, 5, 7);
    (void)r;
    assert(r == 0);
}

static void test_north_not_visible_side(void)
{
    int r = F0227_GROUP_IsDestinationVisibleFromSource(0, 5, 5, 3, 3);
    (void)r;
    assert(r == 0);
}

static void test_east_visible(void)
{
    int r = F0227_GROUP_IsDestinationVisibleFromSource(1, 5, 5, 8, 5);
    (void)r;
    assert(r != 0);
}

static void test_south_visible(void)
{
    int r = F0227_GROUP_IsDestinationVisibleFromSource(2, 5, 5, 5, 8);
    (void)r;
    assert(r != 0);
}

static void test_west_visible(void)
{
    int r = F0227_GROUP_IsDestinationVisibleFromSource(3, 5, 5, 2, 5);
    (void)r;
    assert(r != 0);
}

static void test_same_square_not_visible(void)
{
    int r = F0227_GROUP_IsDestinationVisibleFromSource(0, 5, 5, 5, 5);
    (void)r;
    assert(r == 0);
}

static void test_compat_matches(void)
{
    int a = F0227_GROUP_IsDestinationVisibleFromSource(1, 3, 3, 7, 3);
    int b = F0227_GROUP_IsDestinationVisibleFromSource_Compat(1, 3, 3, 7, 3);
    (void)a; (void)b;
    assert(a == b);
}

static void test_source_evidence(void)
{
    const char* e = F0227_GROUP_IsDestinationVisibleFromSource_SourceEvidencePc34();
    assert(e != NULL);
    assert(strlen(e) > 0);
}

int main(void)
{
    test_direction_constants();
    test_north_visible();
    test_north_not_visible_behind();
    test_north_not_visible_side();
    test_east_visible();
    test_south_visible();
    test_west_visible();
    test_same_square_not_visible();
    test_compat_matches();
    test_source_evidence();

    puts("ok: DM1 group destination visibility (Q-DM1-04) 10 tests passed");
    return 0;
}

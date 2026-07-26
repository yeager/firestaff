#include "dm1_v1_group_destination_visibility_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_cardinal_front_cones_are_visible(void)
{
    assert(F0227_GROUP_IsDestinationVisibleFromSource(
               DM1_V1_GROUP_DIRECTION_NORTH_PC34, 5, 5, 5, 2) == 1);
    assert(F0227_GROUP_IsDestinationVisibleFromSource(
               DM1_V1_GROUP_DIRECTION_EAST_PC34, 5, 5, 8, 5) == 1);
    assert(F0227_GROUP_IsDestinationVisibleFromSource(
               DM1_V1_GROUP_DIRECTION_SOUTH_PC34, 5, 5, 5, 9) == 1);
    assert(F0227_GROUP_IsDestinationVisibleFromSource(
               DM1_V1_GROUP_DIRECTION_WEST_PC34, 5, 5, 1, 5) == 1);
}

static void test_side_behind_and_same_square_are_not_visible(void)
{
    assert(F0227_GROUP_IsDestinationVisibleFromSource(
               DM1_V1_GROUP_DIRECTION_NORTH_PC34, 5, 5, 6, 4) == 0);
    assert(F0227_GROUP_IsDestinationVisibleFromSource(
               DM1_V1_GROUP_DIRECTION_NORTH_PC34, 5, 5, 5, 6) == 0);
    assert(F0227_GROUP_IsDestinationVisibleFromSource(
               DM1_V1_GROUP_DIRECTION_NORTH_PC34, 5, 5, 5, 5) == 0);
    assert(F0227_GROUP_IsDestinationVisibleFromSource(
               DM1_V1_GROUP_DIRECTION_EAST_PC34, 5, 5, 4, 5) == 0);
}

static void test_direction_is_normalized_like_redmcsb_direction_macros(void)
{
    assert(F0227_GROUP_IsDestinationVisibleFromSource(
               4u, 5, 5, 5, 2) == 1);
    assert(F0227_GROUP_IsDestinationVisibleFromSource(
               5u, 5, 5, 8, 5) == 1);
}

static void test_compat_boundary_delegates_to_source_named_boundary(void)
{
    assert(F0227_GROUP_IsDestinationVisibleFromSource_Compat(
               DM1_V1_GROUP_DIRECTION_WEST_PC34, 5, 5, 1, 5) == 1);
    assert(F0227_GROUP_IsDestinationVisibleFromSource_Compat(
               DM1_V1_GROUP_DIRECTION_WEST_PC34, 5, 5, 8, 5) == 0);
}

static void test_source_evidence_names_redmcsb_symbol(void)
{
    const char *evidence =
        F0227_GROUP_IsDestinationVisibleFromSource_SourceEvidencePc34();
    (void)evidence;

    assert(evidence != NULL);
    assert(strstr(evidence, "F0227_GROUP_IsDestinationVisibleFromSource") !=
           NULL);
    assert(strstr(evidence, "F0199/F0200") != NULL);
}

int main(void)
{
    test_cardinal_front_cones_are_visible();
    test_side_behind_and_same_square_are_not_visible();
    test_direction_is_normalized_like_redmcsb_direction_macros();
    test_compat_boundary_delegates_to_source_named_boundary();
    test_source_evidence_names_redmcsb_symbol();

    puts("ok: DM1 F0227 destination visible source cone");
    return 0;
}

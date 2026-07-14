#include "csb_v1_f0227_group_is_destination_visible_from_source_pc34_compat.h"

#include <stdio.h>

static int check(int condition, const char *label)
{
    if (condition) {
        return 1;
    }
    fprintf(stderr, "FAIL: %s\n", label);
    return 0;
}

int main(void)
{
    int ok = 1;

    ok &= check(csb_v1_f0227_group_is_destination_visible_from_source_pc34(
                    CSB_V1_F0227_DIRECTION_WEST_PC34, 10, 10, 7, 10),
                "west admits a destination directly ahead");
    ok &= check(csb_v1_f0227_group_is_destination_visible_from_source_pc34(
                    CSB_V1_F0227_DIRECTION_NORTH_PC34, 10, 10, 10, 7),
                "north applies the source coordinate swap");
    ok &= check(csb_v1_f0227_group_is_destination_visible_from_source_pc34(
                    CSB_V1_F0227_DIRECTION_EAST_PC34, 10, 10, 13, 10),
                "east applies both source and destination swaps");
    ok &= check(csb_v1_f0227_group_is_destination_visible_from_source_pc34(
                    CSB_V1_F0227_DIRECTION_SOUTH_PC34, 10, 10, 10, 13),
                "south applies the cross-coordinate swaps");

    ok &= check(csb_v1_f0227_group_is_destination_visible_from_source_pc34(
                    CSB_V1_F0227_DIRECTION_WEST_PC34, 10, 10, 7, 6),
                "west admits the source formula's wedge boundary");
    ok &= check(!csb_v1_f0227_group_is_destination_visible_from_source_pc34(
                     CSB_V1_F0227_DIRECTION_WEST_PC34, 10, 10, 7, 5),
                 "west rejects a destination beyond the wedge boundary");
    ok &= check(!csb_v1_f0227_group_is_destination_visible_from_source_pc34(
                     CSB_V1_F0227_DIRECTION_WEST_PC34, 10, 10, 11, 10),
                 "west rejects a destination behind the source");
    ok &= check(csb_v1_f0227_group_is_destination_visible_from_source_pc34(
                    CSB_V1_F0227_DIRECTION_WEST_PC34, 10, 10, 10, 10),
                "F0227 considers the source square visible");
    ok &= check(csb_v1_f0227_group_is_destination_visible_from_source_pc34(
                    99, 10, 10, 7, 10),
                "unknown direction retains F0227's west-facing fallthrough");

    if (!ok) {
        return 1;
    }
    puts("PASS csb_v1_f0227_group_is_destination_visible_from_source_pc34_compat");
    return 0;
}

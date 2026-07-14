#include "csb_v1_f0148_dungeon_set_group_directions_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

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
    uint8_t group[16];
    CSB_V1_F0148_ActiveGroupPc34Compat active_groups[2] = {{0x11u}, {0x22u}};
    int ok = 1;

    memset(group, 0, sizeof(group));
    group[5] = 1u;
    group[14] = 0xd7u;
    group[15] = 0xa5u;
    ok &= check(csb_v1_f0148_dungeon_set_group_directions_pc34_compat(
                    group, (int)sizeof(group), 6u, 3u, 2u, NULL, 0) == 1,
                "off-party C04 group accepts directions");
    ok &= check(group[14] == 0xd7u && group[15] == 0xa6u,
                "off-party branch replaces only C04 Direction bits");

    group[15] = 0x5cu;
    ok &= check(csb_v1_f0148_dungeon_set_group_directions_pc34_compat(
                    group, (int)sizeof(group), 0xffffu, 2u, 2u,
                    active_groups, 2) == 1,
                "party-map group accepts its active-group slot");
    ok &= check(active_groups[0].directions == 0x11u &&
                active_groups[1].directions == 0xffu && group[15] == 0x5cu,
                "party-map branch writes unnormalized active directions only");

    group[5] = 0u;
    ok &= check(csb_v1_f0148_dungeon_set_group_directions_pc34_compat(
                    group, (int)sizeof(group), 0x0055u, 2u, 2u,
                    active_groups, 2) == 1,
                "party-map branch accepts packed creature directions");
    ok &= check(active_groups[0].directions == 0x55u && group[15] == 0x5cu,
                "party-map branch preserves C04 Direction");

    group[5] = 2u;
    ok &= check(csb_v1_f0148_dungeon_set_group_directions_pc34_compat(
                    group, (int)sizeof(group), 1u, 2u, 2u,
                    active_groups, 2) == 0 && group[15] == 0x5cu,
                "out-of-range active group rejects without C04 mutation");
    ok &= check(csb_v1_f0148_dungeon_set_group_directions_pc34_compat(
                    group, 15, 1u, 3u, 2u, NULL, 0) == 0 && group[15] == 0x5cu,
                "short C04 record rejects without mutation");
    ok &= check(csb_v1_f0148_dungeon_set_group_directions_pc34_compat(
                    NULL, (int)sizeof(group), 1u, 3u, 2u, NULL, 0) == 0,
                "null C04 record rejects");

    if (!ok) {
        return 1;
    }
    puts("PASS csb_v1_f0148_dungeon_set_group_directions_pc34_compat");
    return 0;
}

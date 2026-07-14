#include "csb_v1_dungeon_world_pc34_compat.h"

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
    uint8_t directions = 0u;
    int ok = 1;

    memset(group, 0, sizeof(group));
    group[4] = 0x2bu;
    group[14] = 0xd7u;
    group[15] = 0xa5u;
    ok &= check(csb_v1_dungeon_get_group_directions_pc34(
                    group, (int)sizeof(group), &directions) == 1,
                "complete C04 group record is admitted");
    ok &= check(directions == 0x01u,
                "F0147 returns only the packed C04 direction bits");
    ok &= check(group[14] == 0xd7u && group[15] == 0xa5u,
                "F0147 leaves packed source bytes unchanged");

    group[14] = 0xffu;
    group[15] = 0xffu;
    ok &= check(csb_v1_dungeon_get_group_directions_pc34(
                    group, (int)sizeof(group), &directions) == 1 &&
                directions == 0x03u,
                "F0147 retains the all-ones packed direction value");

    directions = 0x5au;
    ok &= check(csb_v1_dungeon_get_group_directions_pc34(
                    group, 15, &directions) == 0 && directions == 0x5au,
                "short C04 record rejects without changing output");
    ok &= check(csb_v1_dungeon_get_group_directions_pc34(
                    NULL, (int)sizeof(group), &directions) == 0,
                "null record rejects");
    ok &= check(csb_v1_dungeon_get_group_directions_pc34(
                    group, (int)sizeof(group), NULL) == 0,
                "null output rejects");

    if (!ok) {
        return 1;
    }
    puts("PASS csb_v1_dungeon_get_group_directions_pc34");
    return 0;
}

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
    uint8_t cells = 0u;
    int ok = 1;

    memset(group, 0, sizeof(group));
    group[4] = CSB_THING_TYPE_GROUP;
    group[5] = 0xe4u;
    ok &= check(csb_v1_dungeon_get_group_cells_pc34(
                    group, (int)sizeof(group), &cells) == 1,
                "complete C04 group record is admitted");
    ok &= check(cells == 0xe4u,
                "F0145 returns packed cells byte without normalization");

    group[5] = 0xffu;
    ok &= check(csb_v1_dungeon_get_group_cells_pc34(
                    group, (int)sizeof(group), &cells) == 1 &&
                cells == 0xffu,
                "F0145 preserves centered-group sentinel");

    cells = 0x5au;
    ok &= check(csb_v1_dungeon_get_group_cells_pc34(group, 15, &cells) == 0 &&
                cells == 0x5au,
                "short C04 record rejects without changing output");
    group[4] = CSB_THING_TYPE_WEAPON;
    ok &= check(csb_v1_dungeon_get_group_cells_pc34(
                    group, (int)sizeof(group), &cells) == 0 &&
                cells == 0x5au,
                "non-C04 record rejects without changing output");
    group[4] = CSB_THING_TYPE_GROUP;
    ok &= check(csb_v1_dungeon_get_group_cells_pc34(
                    NULL, (int)sizeof(group), &cells) == 0,
                "null record rejects");
    ok &= check(csb_v1_dungeon_get_group_cells_pc34(
                    group, (int)sizeof(group), NULL) == 0,
                "null output rejects");

    if (!ok) {
        return 1;
    }
    puts("PASS csb_v1_dungeon_get_group_cells_pc34");
    return 0;
}

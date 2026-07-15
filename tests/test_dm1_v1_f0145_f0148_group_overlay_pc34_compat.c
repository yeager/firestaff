#include "dm1_v1_dungeon_thing_data_pc34_compat.h"

#include <stdio.h>
#include <string.h>

#define CHECK(condition, label) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAIL: %s\\n", label); \
        return 1; \
    } \
} while (0)

int main(void)
{
    struct DungeonThings_Compat things;
    struct DungeonGroup_Compat group;
    struct CreatureAIState_Compat active;
    unsigned char raw[16];
    unsigned int value;

    memset(&things, 0, sizeof(things));
    memset(&group, 0, sizeof(group));
    memset(&active, 0, sizeof(active));
    memset(raw, 0, sizeof(raw));
    raw[5] = 0xe4;
    raw[15] = 0x02;
    group.cells = raw[5];
    group.direction = raw[15] & 3u;
    things.loaded = 1;
    things.groups = &group;
    things.groupCount = 1;
    things.rawThingData[THING_TYPE_GROUP] = raw;
    things.thingCounts[THING_TYPE_GROUP] = 1;

    CHECK(dm1_v1_dungeon_get_group_cells_f0145_pc34(
              &things, &active, 1, 3, 2, 0, &value) && value == 0xe4u,
          "F0145 reads raw C04 Cells away from party map");
    CHECK(dm1_v1_dungeon_get_group_directions_f0147_pc34(
              &things, &active, 1, 3, 2, 0, &value) && value == 0xaau,
          "F0147 expands raw C04 Direction through G0258 away from party map");
    CHECK(dm1_v1_dungeon_set_group_cells_f0146_pc34(
              &things, &active, 1, 3, 2, 0, 0x55u) &&
              raw[5] == 0x55u && group.cells == 0x55u,
          "F0146 writes the raw C04 Cells byte away from party map");
    CHECK(dm1_v1_dungeon_set_group_directions_f0148_pc34(
              &things, &active, 1, 3, 2, 0, 0xffu) &&
              (raw[15] & 3u) == 3u && group.direction == 3u,
          "F0148 normalizes raw C04 Direction away from party map");

    active.reserved0 = 0;
    active.groupCells = 0x1b;
    active.groupDirection = 0x39;
    raw[5] = 0x55;
    raw[15] = 0x03;
    CHECK(dm1_v1_dungeon_get_group_cells_f0145_pc34(
              &things, &active, 1, 3, 3, 0, &value) && value == 0x1bu,
          "F0145 reads matching ACTIVE_GROUP Cells on party map");
    CHECK(dm1_v1_dungeon_get_group_directions_f0147_pc34(
              &things, &active, 1, 3, 3, 0, &value) && value == 0x39u,
          "F0147 reads matching ACTIVE_GROUP Directions on party map");
    CHECK(dm1_v1_dungeon_set_group_cells_f0146_pc34(
              &things, &active, 1, 3, 3, 0, 0xc6u) &&
              active.groupCells == 0xc6 && raw[5] == 0x55u,
          "F0146 keeps party-map Cells in ACTIVE_GROUP only");
    CHECK(dm1_v1_dungeon_set_group_directions_f0148_pc34(
              &things, &active, 1, 3, 3, 0, 0x96u) &&
              active.groupDirection == 0x96 && raw[15] == 0x03u,
          "F0148 keeps party-map Directions in ACTIVE_GROUP only");
    CHECK(!dm1_v1_dungeon_set_group_directions_f0148_pc34(
               &things, &active, 1, 3, 3, 0, 0x100u) &&
              active.groupDirection == 0x96,
          "F0148 rejects an unrepresentable active-direction word");
    active.reserved0 = 1;
    CHECK(!dm1_v1_dungeon_get_group_cells_f0145_pc34(
               &things, &active, 1, 3, 3, 0, &value) &&
              !dm1_v1_dungeon_set_group_cells_f0146_pc34(
               &things, &active, 1, 3, 3, 0, 0x00u) && raw[5] == 0x55u,
          "F0145/F0146 reject a party-map C04 without ACTIVE_GROUP data");

    puts("PASS: DM1 F0145-F0148 source-locked C04 active-group overlay");
    return 0;
}

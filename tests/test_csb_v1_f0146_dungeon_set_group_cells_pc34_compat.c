#include "csb_v1_f0146_dungeon_set_group_cells_pc34_compat.h"

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

static int check_write(uint8_t cells, const char *label)
{
    uint8_t before[CSB_V1_F0146_DUNGEON_GROUP_RECORD_SIZE_PC34];
    uint8_t group[CSB_V1_F0146_DUNGEON_GROUP_RECORD_SIZE_PC34];
    size_t index;

    for (index = 0; index < sizeof(group); ++index) {
        group[index] = (uint8_t)(0x80u + index);
    }
    memcpy(before, group, sizeof(group));

    csb_v1_f0146_dungeon_set_group_cells_pc34(group, cells);

    if (!check(group[CSB_V1_F0146_DUNGEON_GROUP_CELLS_OFFSET_PC34] == cells,
               label)) {
        return 0;
    }
    for (index = 0; index < sizeof(group); ++index) {
        if (index != CSB_V1_F0146_DUNGEON_GROUP_CELLS_OFFSET_PC34 &&
            !check(group[index] == before[index], "write changes only GROUP.Cells")) {
            return 0;
        }
    }
    return 1;
}

int main(void)
{
    int ok = 1;

    ok &= check_write(0x00u, "F0146 writes a zero packed-cell value");
    ok &= check_write(0xe4u, "F0146 writes all four packed creature cells");
    ok &= check_write(0xffu, "F0146 preserves the centered-group sentinel");

    if (!ok) {
        return 1;
    }
    puts("PASS csb_v1_f0146_dungeon_set_group_cells_pc34_compat");
    return 0;
}

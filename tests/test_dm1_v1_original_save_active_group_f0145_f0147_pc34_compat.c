#include "dm1_v1_original_save_pc34_handoff.h"

#include <stdio.h>

static int failures;

#define CHECK_EQ(actual, expected, label) do { \
    int a_ = (int)(actual); \
    int e_ = (int)(expected); \
    if (a_ != e_) { \
        fprintf(stderr, "FAIL %s: expected %d got %d\n", label, e_, a_); \
        ++failures; \
    } \
} while (0)

#define CHECK_TRUE(cond, label) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL %s\n", label); \
        ++failures; \
    } \
} while (0)

static void test_packed_cells(void)
{
    int cells[DM1_ORIGINAL_SAVE_PC34_ACTIVE_GROUP_PACKED_LANE_COUNT];
    int dirs[DM1_ORIGINAL_SAVE_PC34_ACTIVE_GROUP_PACKED_LANE_COUNT];

    /* 0xE4 = lanes {0,1,2,3}; 0x1B = lanes {3,2,1,0}. */
    CHECK_EQ(F0145_DUNGEON_GetGroupCells(0xe4, 0), 0, "F0145 lane0");
    CHECK_EQ(F0145_DUNGEON_GetGroupCells(0xe4, 1), 1, "F0145 lane1");
    CHECK_EQ(F0145_DUNGEON_GetGroupCells(0xe4, 2), 2, "F0145 lane2");
    CHECK_EQ(F0145_DUNGEON_GetGroupCells(0xe4, 3), 3, "F0145 lane3");
    CHECK_EQ(F0145_DUNGEON_GetGroupCells(0x1e4, 3), 3,
             "F0145 ignores non-byte audit noise");
    CHECK_EQ(F0145_DUNGEON_GetGroupCells(0xe4, -1), -1,
             "F0145 rejects negative lane");
    CHECK_EQ(F0145_DUNGEON_GetGroupCells(0xe4, 4), -1,
             "F0145 rejects high lane");

    CHECK_EQ(F0147_DUNGEON_GetGroupDirections(0x1b, 0), 3, "F0147 lane0");
    CHECK_EQ(F0147_DUNGEON_GetGroupDirections(0x1b, 1), 2, "F0147 lane1");
    CHECK_EQ(F0147_DUNGEON_GetGroupDirections(0x1b, 2), 1, "F0147 lane2");
    CHECK_EQ(F0147_DUNGEON_GetGroupDirections(0x1b, 3), 0, "F0147 lane3");
    CHECK_EQ(F0147_DUNGEON_GetGroupDirections(0x11b, 0), 3,
             "F0147 ignores non-byte audit noise");
    CHECK_EQ(F0147_DUNGEON_GetGroupDirections(0x1b, -1), -1,
             "F0147 rejects negative lane");
    CHECK_EQ(F0147_DUNGEON_GetGroupDirections(0x1b, 4), -1,
             "F0147 rejects high lane");

    CHECK_TRUE(dm1_v1_original_save_pc34_unpack_active_group_lanes(
                   0xe4, 0x1b, cells, dirs),
               "packed active group lanes unpack");
    CHECK_EQ(cells[0], 0, "unpack cells lane0");
    CHECK_EQ(cells[1], 1, "unpack cells lane1");
    CHECK_EQ(cells[2], 2, "unpack cells lane2");
    CHECK_EQ(cells[3], 3, "unpack cells lane3");
    CHECK_EQ(dirs[0], 3, "unpack dirs lane0");
    CHECK_EQ(dirs[1], 2, "unpack dirs lane1");
    CHECK_EQ(dirs[2], 1, "unpack dirs lane2");
    CHECK_EQ(dirs[3], 0, "unpack dirs lane3");
    CHECK_EQ(dm1_v1_original_save_pc34_unpack_active_group_lanes(
                 0xe4, 0x1b, 0, dirs), 0,
             "unpack rejects missing cells");
    CHECK_EQ(dm1_v1_original_save_pc34_unpack_active_group_lanes(
                 0xe4, 0x1b, cells, 0), 0,
             "unpack rejects missing directions");
}

int main(void)
{
    test_packed_cells();
    if (failures) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    puts("ok: DM1 F0145/F0147 active-group packed lanes");
    return 0;
}

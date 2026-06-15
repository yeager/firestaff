#include "csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static const char *A_F0111 =
    "ReDMCSB DUNVIEW.C F0111:4218-4339";
static const char *A_F0104 =
    "ReDMCSB DUNVIEW.C F0104:3113-3156";
static const char *A_F0105 =
    "ReDMCSB DUNVIEW.C F0105:3185-3247";
static const char *A_F0107 =
    "ReDMCSB DUNVIEW.C F0107:3502-3938";
static const char *A_F0108 =
    "ReDMCSB DUNVIEW.C F0108:3940-4011";
static const char *A_F0115 =
    "ReDMCSB DUNVIEW.C F0115:4547-4581,4923,5180-5188,5211-5214,5668-5671";
static const char *A_F0128 =
    "ReDMCSB DUNVIEW.C F0128:8318-8486";
static const char *A_DUNGEON =
    "ReDMCSB DUNGEON.C F0163:1769-1838 F0164:1840-1905 F0172:2466-2523";
static const char *A_DEFS =
    "ReDMCSB DEFS.H:2088,2596-2611,2662,2668-2677,4045-4046,4139-4153";
static const char *A_LINEAGE =
    "CSB-lineage Viewport.cpp:1192-1209,1865-1879,1903-1915,1930-1944";
static const char *A_CUSTOM =
    "CSB-lineage Viewport.cpp:6507-6548";

static int g_assertions;
static int g_failures;

#define CHECK_EQ(ID, GOT, WANT, ANCHOR)                                      \
    do {                                                                     \
        const int got_value__ = (int)(GOT);                                  \
        const int want_value__ = (int)(WANT);                                \
        ++g_assertions;                                                      \
        if (got_value__ != want_value__) {                                   \
            printf("FAIL %s got=%d want=%d anchor=%s\n",                    \
                   (ID), got_value__, want_value__, (ANCHOR));              \
            ++g_failures;                                                    \
        } else {                                                             \
            printf("ok %s=%d anchor=%s\n", (ID), want_value__, (ANCHOR));   \
        }                                                                    \
    } while (0)

#define CHECK_TRUE(ID, GOT, ANCHOR) CHECK_EQ((ID), (GOT) ? 1 : 0, 1, (ANCHOR))

static void check_contains(const char *id, const char *haystack,
                           const char *needle, const char *anchor)
{
    ++g_assertions;
    if (!haystack || !needle || strstr(haystack, needle) == NULL) {
        printf("FAIL %s missing=%s anchor=%s\n", id, needle ? needle : "(null)",
               anchor);
        ++g_failures;
    } else {
        printf("ok %s contains=%s anchor=%s\n", id, needle, anchor);
    }
}

static void check_spec_one(
    const CSB_V1_D0L2D0R2F0111PartlyOpenDoorSpecPc34 *s,
    int side,
    int square,
    int function_number,
    int dispatch_order,
    int lane,
    int flip,
    int wall_zone,
    int door_zone,
    int rear_order,
    int front_order)
{
    CHECK_TRUE("spec.present", s != 0, "two-lane D0L2/D0R2 fixture");
    if (!s) return;

    CHECK_EQ("spec.side", s->side, side, A_F0128);
    CHECK_EQ("spec.contract_only", s->source_locked_contract_only, 1, A_F0111);
    CHECK_EQ("spec.no_asset_bitmap", s->no_real_asset_bitmap_parity, 1,
             "contract-only no real-asset bitmap parity");
    CHECK_EQ("spec.no_game_data", s->no_game_data_load, 1,
             "contract-only no CSB data load");
    CHECK_EQ("spec.view_square", s->view_square, square, A_DEFS);
    CHECK_EQ("spec.function", s->f0125_f0126_function_number, function_number,
             A_F0128);
    CHECK_EQ("spec.dispatch", s->f0128_dispatch_order, dispatch_order,
             A_F0128);
    CHECK_EQ("spec.depth", s->f0128_relative_depth, 0, A_F0128);
    CHECK_EQ("spec.lane", s->f0128_relative_lateral, lane, A_F0128);
    CHECK_EQ("spec.flip", s->d0r_uses_horizontal_flip, flip, A_F0105);
    CHECK_EQ("spec.wall_zone", s->wall_zone, wall_zone, A_F0107);
    CHECK_EQ("spec.door_zone", s->door_zone_base, door_zone, A_F0111);
    CHECK_EQ("spec.rear_order", s->rear_cell_order, rear_order, A_F0115);
    CHECK_EQ("spec.front_order", s->front_cell_order, front_order, A_F0115);
    CHECK_EQ("spec.open", s->open_state, 0, A_F0111);
    CHECK_EQ("spec.partly1", s->partly_open_state_one, 1, A_F0111);
    CHECK_EQ("spec.partly2", s->partly_open_state_two, 2, A_F0111);
    CHECK_EQ("spec.partly3", s->partly_open_state_three, 3, A_F0111);
    CHECK_EQ("spec.closed", s->closed_state, 4, A_F0111);
    CHECK_EQ("spec.destroyed", s->destroyed_state, 5, A_F0111);
    CHECK_EQ("spec.fraction_denominator", s->open_fraction_denominator, 4,
             "1/4 1/2 3/4 full gate");
    CHECK_EQ("spec.first_offset", s->first_half_zone_offset, 6, A_F0111);
    CHECK_EQ("spec.second_offset", s->second_half_zone_offset, 3, A_F0111);
    CHECK_EQ("spec.second_mask", s->second_half_zone_mask, 0x4000, A_F0111);
    CHECK_EQ("spec.c10", s->c10_transparent_color, 10, A_DEFS);
    CHECK_EQ("spec.mask8000_keepout", s->mask0x8000_footprint_recursion_keepout,
             1, A_F0108);
    CHECK_EQ("spec.wall_keepout", s->wall_ornament_keepout, 1, A_F0107);
    CHECK_EQ("spec.floor_keepout", s->floor_ornament_keepout, 1, A_F0108);
    CHECK_EQ("spec.custom_before_door", s->custom_backgrounds_before_door, 1,
             A_CUSTOM);
    CHECK_EQ("spec.f0163_not_called", s->f0163_not_called_by_draw, 1,
             A_DUNGEON);
    CHECK_EQ("spec.f0164_not_called", s->f0164_not_called_by_draw, 1,
             A_DUNGEON);
    check_contains("spec.left_bitmap", s->left_horizontal_frame_bitmap,
                   "LeftHorizontal", A_F0111);
    check_contains("spec.right_bitmap", s->right_horizontal_frame_bitmap,
                   "RightHorizontal", A_F0111);
    check_contains("spec.f0111_anchor", s->f0111_anchor, "4218-4339", A_F0111);
    check_contains("spec.f0104_anchor", s->f0104_anchor, "3113-3156", A_F0104);
    check_contains("spec.f0105_anchor", s->f0105_anchor, "3185-3247", A_F0105);
    check_contains("spec.f0107_anchor", s->f0107_anchor, "3502-3938", A_F0107);
    check_contains("spec.f0108_anchor", s->f0108_anchor, "3940-4011", A_F0108);
    check_contains("spec.f0115_anchor", s->f0115_anchor, "4547-4581", A_F0115);
    check_contains("spec.f0128_anchor", s->f0128_anchor, "8318-8486", A_F0128);
    check_contains("spec.dungeon_anchor", s->dungeon_anchor, "F0172", A_DUNGEON);
    check_contains("spec.defs_anchor", s->defs_anchor, "2596-2611", A_DEFS);
    check_contains("spec.lineage_anchor", s->lineage_anchor, "1903-1915",
                   A_LINEAGE);
    check_contains("spec.custom_anchor", s->custom_backgrounds_anchor,
                   "6507-6548", A_CUSTOM);
}

static void test_spec_identity(void)
{
    const CSB_V1_D0L2D0R2F0111PartlyOpenDoorSpecPc34 *left =
        csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_spec_for_side_pc34(
            CSB_V1_D0L2_D0R2_F0111_PARTLY_OPEN_DOOR_SIDE_D0L2_PC34);
    const CSB_V1_D0L2D0R2F0111PartlyOpenDoorSpecPc34 *right =
        csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_spec_for_side_pc34(
            CSB_V1_D0L2_D0R2_F0111_PARTLY_OPEN_DOOR_SIDE_D0R2_PC34);

    CHECK_EQ("spec.count",
             csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_spec_count_pc34(),
             2, A_F0128);
    CHECK_TRUE("spec.at0.left",
               csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_spec_at_pc34(0) == left,
               A_F0128);
    CHECK_TRUE("spec.at1.right",
               csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_spec_at_pc34(1) == right,
               A_F0128);
    CHECK_TRUE("spec.at2.null",
               csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_spec_at_pc34(2) == 0,
               "fixture bounds");
    CHECK_TRUE("spec.bad_side.null",
               csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_spec_for_side_pc34(9) == 0,
               "fixture side guard");
    CHECK_TRUE("spec.square8.left",
               csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_spec_for_square_pc34(8) == left,
               A_DEFS);
    CHECK_TRUE("spec.square10.right",
               csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_spec_for_square_pc34(10) == right,
               A_DEFS);
    CHECK_TRUE("spec.square12.absent",
               csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_spec_for_square_pc34(12) == 0,
               "non-duplicative with pass703 D0L2/D0R2 door-front gate");

    check_spec_one(left, 1, 8, 125, 16, -2, 0, 716, 3720, 0x0028, 0x0039);
    check_spec_one(right, 2, 10, 126, 17, 2, 1, 717, 3740, 0x0018, 0x0049);
}

static void test_partly_open_gate_and_zone_math(void)
{
    const CSB_V1_D0L2D0R2F0111PartlyOpenDoorSpecPc34 *left =
        csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_spec_for_side_pc34(1);
    const CSB_V1_D0L2D0R2F0111PartlyOpenDoorSpecPc34 *right =
        csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_spec_for_side_pc34(2);
    const CSB_V1_D0L2D0R2F0111PartlyOpenDoorSpecPc34 *specs[2] = { left, right };
    const int bases[2] = { 3720, 3740 };
    const char *names[2] = { "left", "right" };

    for (int i = 0; i < 2; ++i) {
        const CSB_V1_D0L2D0R2F0111PartlyOpenDoorSpecPc34 *s = specs[i];
        char id[64];
        snprintf(id, sizeof(id), "%s.branch.open", names[i]);
        CHECK_EQ(id,
                 csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_branch_pc34(s, 0),
                 CSB_V1_D0L2_D0R2_F0111_DOOR_BRANCH_OPEN_PC34, A_F0111);
        for (int state = 1; state <= 3; ++state) {
            snprintf(id, sizeof(id), "%s.branch.partly%d", names[i], state);
            CHECK_EQ(id,
                     csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_branch_pc34(s, state),
                     CSB_V1_D0L2_D0R2_F0111_DOOR_BRANCH_PARTLY_OPEN_PC34,
                     A_F0111);
            snprintf(id, sizeof(id), "%s.fraction.%d", names[i], state);
            CHECK_EQ(id,
                     csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_open_fraction_pc34(s, state),
                     state, "1/4 1/2 3/4 partly-open ordinal gate");
            snprintf(id, sizeof(id), "%s.first.%d", names[i], state);
            CHECK_EQ(id,
                     csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_first_half_zone_pc34(s, state, 1),
                     bases[i] + state + 6, A_F0111);
            snprintf(id, sizeof(id), "%s.second.%d", names[i], state);
            CHECK_EQ(id,
                     csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_second_half_zone_pc34(s, state, 1),
                     bases[i] + state + (3 | 0x4000), A_F0111);
            snprintf(id, sizeof(id), "%s.vertical.%d", names[i], state);
            CHECK_EQ(id,
                     csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_second_half_zone_pc34(s, state, 0),
                     bases[i] + state, A_F0111);
        }
        snprintf(id, sizeof(id), "%s.branch.closed", names[i]);
        CHECK_EQ(id,
                 csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_branch_pc34(s, 4),
                 CSB_V1_D0L2_D0R2_F0111_DOOR_BRANCH_CLOSED_PC34, A_F0111);
        snprintf(id, sizeof(id), "%s.fraction.full", names[i]);
        CHECK_EQ(id,
                 csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_open_fraction_pc34(s, 4),
                 4, "full closed ordinal gate");
        snprintf(id, sizeof(id), "%s.branch.destroyed", names[i]);
        CHECK_EQ(id,
                 csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_branch_pc34(s, 5),
                 CSB_V1_D0L2_D0R2_F0111_DOOR_BRANCH_DESTROYED_PC34, A_F0111);
        snprintf(id, sizeof(id), "%s.branch.invalid", names[i]);
        CHECK_EQ(id,
                 csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_branch_pc34(s, 6),
                 CSB_V1_D0L2_D0R2_F0111_DOOR_BRANCH_INVALID_PC34, A_F0111);
        snprintf(id, sizeof(id), "%s.first.open.reject", names[i]);
        CHECK_EQ(id,
                 csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_first_half_zone_pc34(s, 0, 1),
                 -1, A_F0111);
        snprintf(id, sizeof(id), "%s.second.closed.base", names[i]);
        CHECK_EQ(id,
                 csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_second_half_zone_pc34(s, 4, 1),
                 bases[i], A_F0111);
    }

    CHECK_EQ("branch.null",
             csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_branch_pc34(0, 2),
             CSB_V1_D0L2_D0R2_F0111_DOOR_BRANCH_INVALID_PC34, A_F0111);
    CHECK_EQ("fraction.null",
             csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_open_fraction_pc34(0, 2),
             -1, A_F0111);
}

static void test_frames_flip_cells_and_keepouts(void)
{
    const CSB_V1_D0L2D0R2F0111PartlyOpenDoorSpecPc34 *left =
        csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_spec_for_side_pc34(1);
    const CSB_V1_D0L2D0R2F0111PartlyOpenDoorSpecPc34 *right =
        csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_spec_for_side_pc34(2);
    int recursions = 0;

    check_contains("frame.left.left_half",
                   csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_frame_bitmap_pc34(left, 2, 0),
                   "D0L2.LeftHorizontal", A_F0111);
    check_contains("frame.left.right_half",
                   csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_frame_bitmap_pc34(left, 2, 1),
                   "D0L2.RightHorizontal", A_F0111);
    check_contains("frame.right.left_half",
                   csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_frame_bitmap_pc34(right, 2, 0),
                   "D0R2.LeftHorizontal", A_F0111);
    check_contains("frame.right.right_half",
                   csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_frame_bitmap_pc34(right, 2, 1),
                   "D0R2.RightHorizontal", A_F0111);
    CHECK_TRUE("frame.open.null",
               csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_frame_bitmap_pc34(left, 0, 0) == 0,
               A_F0111);

    CHECK_EQ("flip.left.x0",
             csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_source_x_pc34(left, 8, 0),
             0, A_F0104);
    CHECK_EQ("flip.left.x7",
             csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_source_x_pc34(left, 8, 7),
             7, A_F0104);
    CHECK_EQ("flip.right.x0",
             csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_source_x_pc34(right, 8, 0),
             7, A_F0105);
    CHECK_EQ("flip.right.x7",
             csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_source_x_pc34(right, 8, 7),
             0, A_F0105);
    CHECK_EQ("flip.bad",
             csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_source_x_pc34(right, 8, 8),
             -1, "flip bounds guard");

    CHECK_EQ("decode.0021.0",
             csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_decode_cell_pc34(0x0021u, 0),
             0, A_DEFS);
    CHECK_EQ("decode.0021.1",
             csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_decode_cell_pc34(0x0021u, 1),
             1, A_DEFS);
    CHECK_EQ("decode.0028.0",
             csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_decode_cell_pc34(0x0028u, 0),
             -1, A_DEFS);
    CHECK_EQ("decode.0028.1",
             csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_decode_cell_pc34(0x0028u, 1),
             1, A_DEFS);
    CHECK_EQ("decode.0018.1",
             csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_decode_cell_pc34(0x0018u, 1),
             0, A_DEFS);
    CHECK_EQ("decode.0039.1",
             csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_decode_cell_pc34(0x0039u, 1),
             2, A_DEFS);
    CHECK_EQ("decode.0049.1",
             csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_decode_cell_pc34(0x0049u, 1),
             3, A_DEFS);
    CHECK_EQ("decode.0128.2",
             csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_decode_cell_pc34(0x0128u, 2),
             0, A_DEFS);
    CHECK_EQ("decode.0439.2",
             csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_decode_cell_pc34(0x0439u, 2),
             3, A_DEFS);
    CHECK_EQ("decode.bad.ordinal",
             csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_decode_cell_pc34(0x0439u, 4),
             -1, "cell-order guard");

    CHECK_EQ("wall.keepout.door",
             csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_wall_keepout_pc34(left, 1, 1),
             0, A_F0107);
    CHECK_EQ("wall.allow.no_door",
             csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_wall_keepout_pc34(left, 1, 0),
             1, A_F0107);
    CHECK_EQ("wall.none",
             csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_wall_keepout_pc34(left, 0, 1),
             0, A_F0107);
    CHECK_EQ("floor.keepout.door",
             csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_floor_keepout_pc34(
                 left, 1, 1, &recursions),
             0, A_F0108);
    CHECK_EQ("floor.recursion.none", recursions, 0, A_F0108);
    CHECK_EQ("floor.footprint.keepout",
             csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_floor_keepout_pc34(
                 left, 0x8001, 1, &recursions),
             0, A_F0108);
    CHECK_EQ("floor.footprint.recursion", recursions, 1,
             "MASK 0x8000 footprint recursion");
    CHECK_EQ("floor.allow.no_door",
             csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_floor_keepout_pc34(
                 left, 1, 0, &recursions),
             1, A_F0108);
}

static void test_custom_depth_and_blit_guards(void)
{
    const CSB_V1_D0L2D0R2F0111PartlyOpenDoorSpecPc34 *left =
        csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_spec_for_side_pc34(1);
    const CSB_V1_D0L2D0R2F0111PartlyOpenDoorSpecPc34 *right =
        csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_spec_for_side_pc34(2);
    CSB_V1_D0L2D0R2F0111PartlyOpenDoorStepPc34 order[6];
    uint8_t source[8] = { 1, 10, 2, 3, 4, 5, 10, 6 };
    uint8_t dest[8] = { 99, 99, 99, 99, 99, 99, 99, 99 };
    uint8_t before[8];
    CSB_V1_D0L2D0R2F0111PartlyOpenDoorBlitResultPc34 result;

    CHECK_EQ("order.count",
             csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_order_pc34(order, 6),
             6, A_CUSTOM);
    CHECK_EQ("order.custom_mask",
             order[0],
             CSB_V1_D0L2_D0R2_F0111_STEP_CUSTOM_MASK_AFTER_FLOOR_CEILING_PC34,
             A_CUSTOM);
    CHECK_EQ("order.custom_bitmap",
             order[1],
             CSB_V1_D0L2_D0R2_F0111_STEP_CUSTOM_ROOM_BITMAP_PC34,
             A_CUSTOM);
    CHECK_EQ("order.rear_objects",
             order[2],
             CSB_V1_D0L2_D0R2_F0111_STEP_F0115_REAR_OBJECTS_PC34,
             A_F0115);
    CHECK_EQ("order.door_first_after_custom",
             order[3],
             CSB_V1_D0L2_D0R2_F0111_STEP_F0111_DOOR_FIRST_HALF_PC34,
             A_F0111);
    CHECK_EQ("order.door_second",
             order[4],
             CSB_V1_D0L2_D0R2_F0111_STEP_F0111_DOOR_SECOND_HALF_PC34,
             A_F0111);
    CHECK_EQ("order.front_objects",
             order[5],
             CSB_V1_D0L2_D0R2_F0111_STEP_F0115_FRONT_OBJECTS_PC34,
             A_F0115);
    CHECK_TRUE("order.depth.custom_before_door",
               order[0] < order[3] && order[1] < order[3], A_CUSTOM);

    CHECK_EQ("macro.c10",
             CSB_V1_D0L2_D0R2_F0111_PARTLY_OPEN_DOOR_C10_COLOR_FLESH_PC34,
             10, A_DEFS);
    CHECK_EQ("macro.mask4000",
             CSB_V1_D0L2_D0R2_F0111_PARTLY_OPEN_DOOR_MASK0X4000_PC34,
             0x4000, A_DEFS);
    CHECK_EQ("macro.mask8000",
             CSB_V1_D0L2_D0R2_F0111_PARTLY_OPEN_DOOR_MASK0X8000_PC34,
             0x8000, A_F0108);

    CHECK_EQ("blit.left.copied",
             csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_synthetic_blit_pc34(
                 left, 2, source, 4, 2, 4, dest, 4, 2, 4, &result),
             6, A_F0111);
    CHECK_EQ("blit.left.ok", result.ok, 1, A_F0111);
    CHECK_EQ("blit.left.skipped", result.c10_skipped_pixels, 2, A_DEFS);
    CHECK_EQ("blit.left.pixel0", dest[0], 1, A_F0104);
    CHECK_EQ("blit.left.transparent", dest[1], 99, A_DEFS);
    CHECK_EQ("blit.left.pixel2", dest[2], 2, A_F0104);
    CHECK_EQ("blit.left.edge_left", result.left_edge_writes, 2, A_F0111);
    CHECK_EQ("blit.left.edge_right", result.right_edge_writes, 2, A_F0111);
    CHECK_TRUE("blit.left.hash_present", result.deterministic_hash != 0,
               "deterministic_hash present");

    for (int i = 0; i < 8; ++i) dest[i] = 99;
    CHECK_EQ("blit.right.copied",
             csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_synthetic_blit_pc34(
                 right, 2, source, 4, 2, 4, dest, 4, 2, 4, &result),
             6, A_F0105);
    CHECK_EQ("blit.right.flip.pixel0", dest[0], 3, A_F0105);
    CHECK_EQ("blit.right.flip.pixel1", dest[1], 2, A_F0105);
    CHECK_EQ("blit.right.flip.transparent", dest[2], 99, A_DEFS);
    CHECK_EQ("blit.right.flip.pixel3", dest[3], 1, A_F0105);

    memcpy(before, dest, sizeof(dest));
    CHECK_EQ("blit.open.skip",
             csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_synthetic_blit_pc34(
                 right, 0, source, 4, 2, 4, dest, 4, 2, 4, &result),
             0, A_F0111);
    CHECK_EQ("blit.open.no_mutation", memcmp(dest, before, sizeof(dest)), 0,
             "caller-owned surface unchanged on open skip");

    memcpy(before, dest, sizeof(dest));
    CHECK_EQ("blit.reject.row_guard",
             csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_synthetic_blit_pc34(
                 right, 2, source, 4, 2, 3, dest, 4, 2, 4, &result),
             -1, "row-guard rejection");
    CHECK_EQ("blit.reject.row_count", result.row_guard_rejections, 1,
             "row-guard rejections >=1");
    CHECK_EQ("blit.reject.mutation_count", result.mutation_rejections, 1,
             "mutation rejections >=1");
    CHECK_EQ("blit.reject.unchanged", memcmp(dest, before, sizeof(dest)), 0,
             "caller-owned surface bytes unchanged after guarded rejection");
}

static void test_probe_hash_and_evidence(void)
{
    CSB_V1_D0L2D0R2F0111PartlyOpenDoorProbePc34 probe;
    const char *e =
        csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_source_evidence_pc34();
    uint32_t hash1 =
        csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_hash_pc34();
    uint32_t hash2 =
        csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_hash_pc34();

    CHECK_EQ("probe.run",
             csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_probe_pc34_compat(&probe),
             0, "pass719 probe");
    CHECK_EQ("probe.route_count", probe.route_count, 2, A_F0128);
    CHECK_EQ("probe.flip", probe.d0r_flip_ok, 1, A_F0105);
    CHECK_EQ("probe.partly_gate", probe.partly_open_gate_ok, 1, A_F0111);
    CHECK_EQ("probe.custom_depth", probe.custom_backgrounds_depth_ok, 1, A_CUSTOM);
    CHECK_EQ("probe.wall_keepout", probe.wall_keepout_ok, 1, A_F0107);
    CHECK_EQ("probe.floor_keepout", probe.floor_keepout_ok, 1, A_F0108);
    CHECK_EQ("probe.first_half", probe.first_half_zone, 3728, A_F0111);
    CHECK_EQ("probe.second_half", probe.second_half_zone, 20109, A_F0111);
    CHECK_EQ("probe.copied", probe.copied_pixels, 6, A_F0111);
    CHECK_EQ("probe.skipped", probe.c10_skipped_pixels, 2, A_DEFS);
    CHECK_TRUE("probe.hash_present", probe.deterministic_hash != 0,
               "deterministic_hash present");
    CHECK_EQ("hash.stable", hash1 == hash2, 1, "deterministic_hash stable");
    CHECK_EQ("hash.probe_matches", probe.deterministic_hash == hash1, 1,
             "deterministic_hash stable");

    check_contains("evidence.contract", e, "source_locked_contract_only=1",
                   "required marker");
    check_contains("evidence.no_bitmap", e, "no_real_asset_bitmap_parity=1",
                   "required marker");
    check_contains("evidence.no_data", e, "no_game_data_load=1",
                   "required marker");
    check_contains("evidence.f0111", e, "F0111:4218-4339", A_F0111);
    check_contains("evidence.f0104", e, "F0104:3113-3156", A_F0104);
    check_contains("evidence.f0105", e, "F0105:3185-3247", A_F0105);
    check_contains("evidence.f0107", e, "F0107:3502-3938", A_F0107);
    check_contains("evidence.f0108", e, "F0108:3940-4011", A_F0108);
    check_contains("evidence.f0115", e, "F0115:4547-4581", A_F0115);
    check_contains("evidence.f0128", e, "F0128:8318-8486", A_F0128);
    check_contains("evidence.f0163", e, "F0163:1769-1838", A_DUNGEON);
    check_contains("evidence.f0164", e, "F0164:1840-1905", A_DUNGEON);
    check_contains("evidence.f0172", e, "F0172:2466-2523", A_DUNGEON);
    check_contains("evidence.defs2088", e, "DEFS.H:2088", A_DEFS);
    check_contains("evidence.defs_views", e, "2596-2611", A_DEFS);
    check_contains("evidence.defs_cell_order", e, "2668-2677", A_DEFS);
    check_contains("evidence.defs_zones", e, "4045-4046", A_DEFS);
    check_contains("evidence.lineage_open", e, "1192-1209", A_LINEAGE);
    check_contains("evidence.lineage_f2", e, "1865-1879", A_LINEAGE);
    check_contains("evidence.lineage_f1", e, "1903-1915", A_LINEAGE);
    check_contains("evidence.lineage_f0", e, "1930-1944", A_LINEAGE);
    check_contains("evidence.custom", e, "6507-6548", A_CUSTOM);
}

int main(void)
{
    const uint32_t hash =
        csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_hash_pc34();

    printf("probe=csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_source_evidence_pc34());

    test_spec_identity();
    test_partly_open_gate_and_zone_math();
    test_frames_flip_cells_and_keepouts();
    test_custom_depth_and_blit_guards();
    test_probe_hash_and_evidence();

    CHECK_TRUE("assertion_count_at_least_130", g_assertions >= 130,
               "assigned pass719 assertion floor");
    printf("assertions=%d failures=%d deterministic_hash=0x%08x\n",
           g_assertions, g_failures, hash);
    if (g_failures == 0) {
        printf("PASS csb_v1_viewport_d0l2_d0r2_f0111_partly_open_door_pc34_compat assertions=%d failures=%d deterministic_hash=0x%08x\n",
               g_assertions, g_failures, hash);
    }
    return g_failures == 0 ? 0 : 1;
}

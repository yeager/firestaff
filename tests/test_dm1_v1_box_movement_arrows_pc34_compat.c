#include "firestaff/dm1/v1/box_movement_arrows_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_failures = 0;
static int g_assertions = 0;

static void check(int cond, const char *expr, const char *file, int line)
{
    ++g_assertions;
    if (!cond) {
        ++g_failures;
        fprintf(stderr, "FAIL: %s:%d %s\n", file, line, expr);
    }
}

#define CHECK(c) check((c), #c, __FILE__, __LINE__)

static void test_table_values(void)
{
    /* DATA.C:123 G0002 init: { 224, 319, 124, 168 }. */
    const int *t = dm1_v1_box_movement_arrows_table_pc34();
    int n = dm1_v1_box_movement_arrows_size_pc34();
    CHECK(t != 0);
    CHECK(n == 4);
    CHECK(t[0] == 224);
    CHECK(t[1] == 319);
    CHECK(t[2] == 124);
    CHECK(t[3] == 168);
}

static void test_accessor_functions(void)
{
    CHECK(dm1_v1_box_movement_arrows_x_pc34() == 224);
    CHECK(dm1_v1_box_movement_arrows_y_pc34() == 319);
    CHECK(dm1_v1_box_movement_arrows_w_pc34() == 124);
    CHECK(dm1_v1_box_movement_arrows_h_pc34() == 168);
}

static void test_get_function(void)
{
    int v;
    int rc;
    rc = dm1_v1_box_movement_arrows_get_pc34(0, &v);
    CHECK(rc == 1);
    CHECK(v == 224);
    rc = dm1_v1_box_movement_arrows_get_pc34(1, &v);
    CHECK(rc == 1);
    CHECK(v == 319);
    rc = dm1_v1_box_movement_arrows_get_pc34(2, &v);
    CHECK(rc == 1);
    CHECK(v == 124);
    rc = dm1_v1_box_movement_arrows_get_pc34(3, &v);
    CHECK(rc == 1);
    CHECK(v == 168);
    CHECK(dm1_v1_box_movement_arrows_get_pc34(-1, &v) == 0);
    CHECK(v == -1);
    CHECK(dm1_v1_box_movement_arrows_get_pc34(4, &v) == 0);
    CHECK(v == -1);
    CHECK(dm1_v1_box_movement_arrows_get_pc34(0, 0) == 0);
}

static void test_components_non_negative(void)
{
    CHECK(dm1_v1_box_movement_arrows_x_pc34() >= 0);
    CHECK(dm1_v1_box_movement_arrows_y_pc34() >= 0);
    CHECK(dm1_v1_box_movement_arrows_w_pc34() > 0);
    CHECK(dm1_v1_box_movement_arrows_h_pc34() > 0);
}

static void test_screen_rects(void)
{
    DM1_V1_MovementArrowRectPc34 r;
    CHECK(dm1_v1_movement_arrows_zone_id_pc34() == 9);
    CHECK(dm1_v1_movement_arrows_graphic_id_pc34() == 13);
    CHECK(dm1_v1_movement_arrows_outer_rect_pc34(&r) == 1);
    CHECK(r.x == 224);
    CHECK(r.y == 124);
    CHECK(r.w == 96);
    CHECK(r.h == 45);
    CHECK(dm1_v1_movement_arrows_graphic_rect_pc34(&r) == 1);
    CHECK(r.x == 233);
    CHECK(r.y == 124);
    CHECK(r.w == 87);
    CHECK(r.h == 45);
    CHECK(dm1_v1_movement_arrows_outer_rect_pc34(0) == 0);
    CHECK(dm1_v1_movement_arrows_graphic_rect_pc34(0) == 0);
}

static void test_arrow_zones(void)
{
    static const int kExpected[DM1_V1_MOVEMENT_ARROW_COUNT_PC34][5] = {
        { 68, 234, 125, 19, 21 },
        { 69, 291, 125, 19, 21 },
        { 70, 263, 125, 27, 21 },
        { 71, 291, 147, 28, 21 },
        { 72, 263, 147, 27, 21 },
        { 73, 234, 147, 28, 21 }
    };
    int i;
    for (i = 0; i < DM1_V1_MOVEMENT_ARROW_COUNT_PC34; ++i) {
        DM1_V1_MovementArrowRectPc34 r;
        CHECK(dm1_v1_movement_arrow_zone_id_pc34(i) == kExpected[i][0]);
        CHECK(dm1_v1_movement_arrow_rect_pc34(i, &r) == 1);
        CHECK(r.x == kExpected[i][1]);
        CHECK(r.y == kExpected[i][2]);
        CHECK(r.w == kExpected[i][3]);
        CHECK(r.h == kExpected[i][4]);
    }
    CHECK(dm1_v1_movement_arrow_zone_id_pc34(-1) == 0);
    CHECK(dm1_v1_movement_arrow_zone_id_pc34(DM1_V1_MOVEMENT_ARROW_COUNT_PC34) == 0);
    CHECK(dm1_v1_movement_arrow_rect_pc34(-1, 0) == 0);
}

static void test_visual_masks(void)
{
    CHECK(dm1_v1_movement_arrow_visual_mask_for_command_pc34(
              DM1_V1_MOVEMENT_ARROW_COMMAND_TURN_LEFT_PC34) ==
          DM1_V1_MOVEMENT_ARROW_VIS_TURN_LEFT_PC34);
    CHECK(dm1_v1_movement_arrow_visual_mask_for_command_pc34(
              DM1_V1_MOVEMENT_ARROW_COMMAND_TURN_RIGHT_PC34) ==
          DM1_V1_MOVEMENT_ARROW_VIS_TURN_RIGHT_PC34);
    CHECK(dm1_v1_movement_arrow_visual_mask_for_command_pc34(
              DM1_V1_MOVEMENT_ARROW_COMMAND_FORWARD_PC34) ==
          DM1_V1_MOVEMENT_ARROW_VIS_FORWARD_PC34);
    CHECK(dm1_v1_movement_arrow_visual_mask_for_command_pc34(
              DM1_V1_MOVEMENT_ARROW_COMMAND_RIGHT_PC34) ==
          DM1_V1_MOVEMENT_ARROW_VIS_RIGHT_PC34);
    CHECK(dm1_v1_movement_arrow_visual_mask_for_command_pc34(
              DM1_V1_MOVEMENT_ARROW_COMMAND_BACKWARD_PC34) ==
          DM1_V1_MOVEMENT_ARROW_VIS_BACKWARD_PC34);
    CHECK(dm1_v1_movement_arrow_visual_mask_for_command_pc34(
              DM1_V1_MOVEMENT_ARROW_COMMAND_LEFT_PC34) ==
          DM1_V1_MOVEMENT_ARROW_VIS_LEFT_PC34);
    CHECK(dm1_v1_movement_arrow_visual_mask_for_command_pc34(0) == 0);
}

static void test_visual_receipts(void)
{
    DM1_V1_MovementArrowVisualReceiptPc34 r;
    CHECK(dm1_v1_movement_arrow_visual_receipt_pc34(
              DM1_V1_MOVEMENT_ARROW_VIS_TURN_LEFT_PC34,
              DM1_V1_MOVEMENT_ARROW_INDEX_TURN_LEFT_PC34, &r) == 1);
    CHECK(r.accepted == 1);
    CHECK(r.arrowIndex == DM1_V1_MOVEMENT_ARROW_INDEX_TURN_LEFT_PC34);
    CHECK(r.rect.x == 234);
    CHECK(r.rect.y == 125);
    CHECK(r.cueColorKind == 1);

    CHECK(dm1_v1_movement_arrow_visual_receipt_pc34(
              DM1_V1_MOVEMENT_ARROW_VIS_FORWARD_PC34,
              DM1_V1_MOVEMENT_ARROW_INDEX_FORWARD_PC34, &r) == 1);
    CHECK(r.accepted == 1);
    CHECK(r.rect.x == 263);
    CHECK(r.rect.y == 125);
    CHECK(r.cueColorKind == 2);

    CHECK(dm1_v1_movement_arrow_visual_receipt_pc34(
              DM1_V1_MOVEMENT_ARROW_VIS_FORWARD_PC34,
              DM1_V1_MOVEMENT_ARROW_INDEX_BACKWARD_PC34, &r) == 0);
    CHECK(dm1_v1_movement_arrow_visual_receipt_pc34(
              DM1_V1_MOVEMENT_ARROW_VIS_FORWARD_PC34,
              DM1_V1_MOVEMENT_ARROW_COUNT_PC34, &r) == 0);
    CHECK(dm1_v1_movement_arrow_visual_receipt_pc34(
              DM1_V1_MOVEMENT_ARROW_VIS_FORWARD_PC34,
              DM1_V1_MOVEMENT_ARROW_INDEX_FORWARD_PC34, 0) == 0);
}

static void test_run_accepted(void)
{
    DM1_V1_BoxMovementArrowsResultPc34 r;
    int ok = dm1_v1_box_movement_arrows_run_pc34(&r);
    int i;
    CHECK(ok == 1);
    CHECK(r.accepted == 1);
    CHECK(r.assertionCount == 11);
    CHECK(r.tableSize == 4);
    CHECK(r.tableEntries[0] == 224);
    CHECK(r.tableEntries[1] == 319);
    CHECK(r.tableEntries[2] == 124);
    CHECK(r.tableEntries[3] == 168);
    CHECK(r.tableMatchesDeclaration == 1);
    CHECK(r.xIs224 == 1);
    CHECK(r.yIs319 == 1);
    CHECK(r.wIs124 == 1);
    CHECK(r.hIs168 == 1);
    CHECK(r.allComponentsNonNegative == 1);
    CHECK(r.widthPositive == 1);
    CHECK(r.heightPositive == 1);
    CHECK(r.withinRowRange == 1);
    CHECK(r.withinBoxBounds == 1);
    for (i = 0; i < 4; ++i) {
        int v;
        int rc = dm1_v1_box_movement_arrows_get_pc34(i, &v);
        CHECK(rc == 1);
        CHECK(r.tableEntries[i] == v);
    }
}

int main(void)
{
    test_table_values();
    test_accessor_functions();
    test_get_function();
    test_components_non_negative();
    test_screen_rects();
    test_arrow_zones();
    test_visual_masks();
    test_visual_receipts();
    test_run_accepted();
    printf("dm1_v1_box_movement_arrows: "
           "%d/%d assertions passed\n",
           g_assertions - g_failures, g_assertions);
    return g_failures == 0 ? 0 : 1;
}

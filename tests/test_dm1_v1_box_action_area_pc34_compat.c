#include "firestaff/dm1/v1/box_action_area_pc34_compat.h"

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
    /* DATA.C:121 G0001 init: { 224, 319, 77, 121 }. */
    const int *t = dm1_v1_box_action_area_table_pc34();
    int n = dm1_v1_box_action_area_size_pc34();
    CHECK(t != 0);
    CHECK(n == 4);
    CHECK(t[0] == 224);
    CHECK(t[1] == 319);
    CHECK(t[2] == 77);
    CHECK(t[3] == 121);
}

static void test_accessor_functions(void)
{
    CHECK(dm1_v1_box_action_area_x_pc34() == 224);
    CHECK(dm1_v1_box_action_area_y_pc34() == 319);
    CHECK(dm1_v1_box_action_area_w_pc34() == 77);
    CHECK(dm1_v1_box_action_area_h_pc34() == 121);
}

static void test_get_function(void)
{
    int v;
    int rc;
    rc = dm1_v1_box_action_area_get_pc34(0, &v);
    CHECK(rc == 1);
    CHECK(v == 224);
    rc = dm1_v1_box_action_area_get_pc34(1, &v);
    CHECK(rc == 1);
    CHECK(v == 319);
    rc = dm1_v1_box_action_area_get_pc34(2, &v);
    CHECK(rc == 1);
    CHECK(v == 77);
    rc = dm1_v1_box_action_area_get_pc34(3, &v);
    CHECK(rc == 1);
    CHECK(v == 121);
    CHECK(dm1_v1_box_action_area_get_pc34(-1, &v) == 0);
    CHECK(v == -1);
    CHECK(dm1_v1_box_action_area_get_pc34(4, &v) == 0);
    CHECK(v == -1);
    CHECK(dm1_v1_box_action_area_get_pc34(0, 0) == 0);
}

static void test_components_non_negative(void)
{
    /* All 4 components non-negative. */
    CHECK(dm1_v1_box_action_area_x_pc34() >= 0);
    CHECK(dm1_v1_box_action_area_y_pc34() >= 0);
    CHECK(dm1_v1_box_action_area_w_pc34() > 0);
    CHECK(dm1_v1_box_action_area_h_pc34() > 0);
}

static void test_run_accepted(void)
{
    DM1_V1_BoxActionAreaResultPc34 r;
    int ok = dm1_v1_box_action_area_run_pc34(&r);
    int i;
    CHECK(ok == 1);
    CHECK(r.accepted == 1);
    CHECK(r.assertionCount == 12);
    CHECK(r.tableSize == 4);
    CHECK(r.tableEntries[0] == 224);
    CHECK(r.tableEntries[1] == 319);
    CHECK(r.tableEntries[2] == 77);
    CHECK(r.tableEntries[3] == 121);
    CHECK(r.tableMatchesDeclaration == 1);
    CHECK(r.xIs224 == 1);
    CHECK(r.yIs319 == 1);
    CHECK(r.wIs77 == 1);
    CHECK(r.hIs121 == 1);
    CHECK(r.allComponentsNonNegative == 1);
    CHECK(r.widthPositive == 1);
    CHECK(r.heightPositive == 1);
    CHECK(r.byteAligned == 1);
    CHECK(r.withinRowRange == 1);
    CHECK(r.withinBoxBounds == 1);
    for (i = 0; i < 4; ++i) {
        int v;
        int rc = dm1_v1_box_action_area_get_pc34(i, &v);
        CHECK(rc == 1);
        CHECK(r.tableEntries[i] == v);
    }
}

static void test_action_area_render_contract(void)
{
    DM1_V1_ActionAreaRectPc34 rect = dm1_v1_action_area_rect_pc34();
    DM1_V1_ActionMenuRenderPlanPc34 plan;
    DM1_V1_ActionAreaRectPc34 icon;
    DM1_V1_ActionAreaRectPc34 inner;

    CHECK(rect.x == 224);
    CHECK(rect.y == 77);
    CHECK(rect.w == 96);
    CHECK(rect.h == 45);

    CHECK(dm1_v1_action_menu_build_render_plan_pc34(3, &plan) == 1);
    CHECK(plan.graphic_id == DM1_V1_ACTION_AREA_GRAPHIC_ID_PC34);
    CHECK(plan.graphic_zone_id == 11);
    CHECK(plan.clear_rect.x == 224);
    CHECK(plan.clear_rect.w == 96);
    CHECK(plan.graphic_rect.h == 45);
    CHECK(plan.header_rect.x == 224);
    CHECK(plan.header_text.x == 235);
    CHECK(plan.header_text.y == 83);
    CHECK(plan.row_rects[0].x == 234);
    CHECK(plan.row_rects[0].y == 86);
    CHECK(plan.row_text[2].x == 241);
    CHECK(plan.row_text[2].y == 117);
    CHECK(plan.header_text_color == 0);
    CHECK(plan.header_fill_color == 4);
    CHECK(plan.row_text_color == 4);
    CHECK(plan.row_fill_color == 0);

    CHECK(dm1_v1_action_menu_build_render_plan_pc34(2, &plan) == 1);
    CHECK(plan.graphic_zone_id == 77);
    CHECK(plan.graphic_rect.h == 33);
    CHECK(dm1_v1_action_menu_build_render_plan_pc34(1, &plan) == 1);
    CHECK(plan.graphic_zone_id == 79);
    CHECK(plan.graphic_rect.h == 21);
    CHECK(dm1_v1_action_menu_build_render_plan_pc34(0, &plan) == 0);

    CHECK(dm1_v1_action_icon_cell_zone_id_pc34(0) == 89);
    CHECK(dm1_v1_action_icon_cell_zone_id_pc34(3) == 92);
    CHECK(dm1_v1_action_icon_inner_zone_id_pc34(0) == 93);
    CHECK(dm1_v1_action_icon_inner_zone_id_pc34(3) == 96);
    icon = dm1_v1_action_icon_cell_rect_pc34(3);
    CHECK(icon.x == 299);
    CHECK(icon.y == 86);
    CHECK(icon.w == 20);
    CHECK(icon.h == 35);
    inner = dm1_v1_action_icon_inner_rect_pc34(3);
    CHECK(inner.x == 301);
    CHECK(inner.y == 95);
    CHECK(inner.w == 16);
    CHECK(inner.h == 16);
    CHECK(dm1_v1_action_icon_cell_zone_id_pc34(4) == 0);
}

int main(void)
{
    test_table_values();
    test_accessor_functions();
    test_get_function();
    test_components_non_negative();
    test_run_accepted();
    test_action_area_render_contract();
    printf("dm1_v1_box_action_area: "
           "%d/%d assertions passed\n",
           g_assertions - g_failures, g_assertions);
    return g_failures == 0 ? 0 : 1;
}

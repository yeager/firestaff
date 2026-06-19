#include "firestaff/dm1/v1/champion_panel_box_water_pc34_compat.h"

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
    /* DATA.C:318 G0036 init: { 112, 159, 83, 91 }. */
    const int *t = dm1_v1_champion_panel_box_water_table_pc34();
    CHECK(t != 0);
    CHECK(t[0] == 112);
    CHECK(t[1] == 159);
    CHECK(t[2] == 83);
    CHECK(t[3] == 91);
}

static void test_accessor_functions(void)
{
    CHECK(dm1_v1_champion_panel_box_water_x_pc34() == 112);
    CHECK(dm1_v1_champion_panel_box_water_y_pc34() == 159);
    CHECK(dm1_v1_champion_panel_box_water_w_pc34() == 83);
    CHECK(dm1_v1_champion_panel_box_water_h_pc34() == 91);
}

static void test_get_function(void)
{
    int v;
    int rc;
    rc = dm1_v1_champion_panel_box_water_get_pc34(0, &v);
    CHECK(rc == 1);
    CHECK(v == 112);
    rc = dm1_v1_champion_panel_box_water_get_pc34(1, &v);
    CHECK(rc == 1);
    CHECK(v == 159);
    rc = dm1_v1_champion_panel_box_water_get_pc34(2, &v);
    CHECK(rc == 1);
    CHECK(v == 83);
    rc = dm1_v1_champion_panel_box_water_get_pc34(3, &v);
    CHECK(rc == 1);
    CHECK(v == 91);
    CHECK(dm1_v1_champion_panel_box_water_get_pc34(-1, &v) == 0);
    CHECK(v == -1);
    CHECK(dm1_v1_champion_panel_box_water_get_pc34(4, &v) == 0);
    CHECK(v == -1);
    CHECK(dm1_v1_champion_panel_box_water_get_pc34(0, 0) == 0);
}

static void test_water_x_aligns_with_food(void)
{
    /* G0035 (food) X = 112 per pass805. G0036 (water) X must equal. */
    CHECK(dm1_v1_champion_panel_box_water_x_pc34() == 112);
}

static void test_run_accepted(void)
{
    DM1_V1_ChampionPanelBoxWaterResultPc34 r;
    int ok = dm1_v1_champion_panel_box_water_run_pc34(&r);
    int i;
    CHECK(ok == 1);
    CHECK(r.accepted == 1);
    CHECK(r.assertionCount == 11);
    CHECK(r.tableEntries[0] == 112);
    CHECK(r.tableEntries[1] == 159);
    CHECK(r.tableEntries[2] == 83);
    CHECK(r.tableEntries[3] == 91);
    CHECK(r.tableMatchesDeclaration == 1);
    CHECK(r.xIs112 == 1);
    CHECK(r.yIs159 == 1);
    CHECK(r.wIs83 == 1);
    CHECK(r.hIs91 == 1);
    CHECK(r.allComponentsNonNegative == 1);
    CHECK(r.widthPositive  == 1);
    CHECK(r.heightPositive == 1);
    CHECK(r.withinReasonableBounds == 1);
    CHECK(r.xAlignedWithFoodLabel == 1);
    for (i = 0; i < 4; ++i) {
        int v;
        int rc = dm1_v1_champion_panel_box_water_get_pc34(i, &v);
        CHECK(rc == 1);
        CHECK(r.tableEntries[i] == v);
    }
}

int main(void)
{
    test_table_values();
    test_accessor_functions();
    test_get_function();
    test_water_x_aligns_with_food();
    test_run_accepted();
    printf("dm1_v1_champion_panel_box_water: "
           "%d/%d assertions passed\n",
           g_assertions - g_failures, g_assertions);
    return g_failures == 0 ? 0 : 1;
}
#include "firestaff/dm1/v1/champion_panel_box_object_description_circle_pc34_compat.h"

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
    /* DATA.C:316 G0034 init: { 105, 136, 53, 79 }. */
    const int *t = dm1_v1_champion_panel_box_object_description_circle_table_pc34();
    CHECK(t != 0);
    CHECK(t[0] == 105);
    CHECK(t[1] == 136);
    CHECK(t[2] == 53);
    CHECK(t[3] == 79);
}

static void test_accessor_functions(void)
{
    CHECK(dm1_v1_champion_panel_box_object_description_circle_x_pc34() == 105);
    CHECK(dm1_v1_champion_panel_box_object_description_circle_y_pc34() == 136);
    CHECK(dm1_v1_champion_panel_box_object_description_circle_w_pc34() == 53);
    CHECK(dm1_v1_champion_panel_box_object_description_circle_h_pc34() == 79);
}

static void test_get_function(void)
{
    int v;
    int rc;
    rc = dm1_v1_champion_panel_box_object_description_circle_get_pc34(0, &v);
    CHECK(rc == 1);
    CHECK(v == 105);
    rc = dm1_v1_champion_panel_box_object_description_circle_get_pc34(1, &v);
    CHECK(rc == 1);
    CHECK(v == 136);
    rc = dm1_v1_champion_panel_box_object_description_circle_get_pc34(2, &v);
    CHECK(rc == 1);
    CHECK(v == 53);
    rc = dm1_v1_champion_panel_box_object_description_circle_get_pc34(3, &v);
    CHECK(rc == 1);
    CHECK(v == 79);
    CHECK(dm1_v1_champion_panel_box_object_description_circle_get_pc34(-1, &v) == 0);
    CHECK(v == -1);
    CHECK(dm1_v1_champion_panel_box_object_description_circle_get_pc34(4, &v) == 0);
    CHECK(v == -1);
    CHECK(dm1_v1_champion_panel_box_object_description_circle_get_pc34(0, 0) == 0);
}

static void test_components_non_negative(void)
{
    CHECK(dm1_v1_champion_panel_box_object_description_circle_x_pc34() >= 0);
    CHECK(dm1_v1_champion_panel_box_object_description_circle_y_pc34() >= 0);
    CHECK(dm1_v1_champion_panel_box_object_description_circle_w_pc34() > 0);
    CHECK(dm1_v1_champion_panel_box_object_description_circle_h_pc34() > 0);
}

static void test_circle_taller_than_wide(void)
{
    /* Circular aspect: H=79 > W=53. */
    CHECK(dm1_v1_champion_panel_box_object_description_circle_h_pc34() >
          dm1_v1_champion_panel_box_object_description_circle_w_pc34());
}

static void test_run_accepted(void)
{
    DM1_V1_ChampionPanelBoxObjectDescriptionCircleResultPc34 r;
    int ok = dm1_v1_champion_panel_box_object_description_circle_run_pc34(&r);
    int i;
    CHECK(ok == 1);
    CHECK(r.accepted == 1);
    CHECK(r.assertionCount == 11);
    CHECK(r.tableEntries[0] == 105);
    CHECK(r.tableEntries[1] == 136);
    CHECK(r.tableEntries[2] == 53);
    CHECK(r.tableEntries[3] == 79);
    CHECK(r.tableMatchesDeclaration == 1);
    CHECK(r.xIs105 == 1);
    CHECK(r.yIs136 == 1);
    CHECK(r.wIs53 == 1);
    CHECK(r.hIs79 == 1);
    CHECK(r.allComponentsNonNegative == 1);
    CHECK(r.widthPositive == 1);
    CHECK(r.heightPositive == 1);
    CHECK(r.withinReasonableBounds == 1);
    CHECK(r.heightLargerThanWidth == 1);
    for (i = 0; i < 4; ++i) {
        int v;
        int rc = dm1_v1_champion_panel_box_object_description_circle_get_pc34(i, &v);
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
    test_circle_taller_than_wide();
    test_run_accepted();
    printf("dm1_v1_champion_panel_box_object_description_circle: "
           "%d/%d assertions passed\n",
           g_assertions - g_failures, g_assertions);
    return g_failures == 0 ? 0 : 1;
}
#include "firestaff/dm1/v1/champion_panel_box_food_pc34_compat.h"

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
    /* DATA.C:317 G0035 init: { 112, 159, 60, 68 }. */
    const int *t = dm1_v1_champion_panel_box_food_table_pc34();
    CHECK(t != 0);
    CHECK(t[0] == 112);
    CHECK(t[1] == 159);
    CHECK(t[2] == 60);
    CHECK(t[3] == 68);
}

static void test_accessor_functions(void)
{
    /* All 4 single-component accessors return the expected value. */
    CHECK(dm1_v1_champion_panel_box_food_x_pc34() == 112);
    CHECK(dm1_v1_champion_panel_box_food_y_pc34() == 159);
    CHECK(dm1_v1_champion_panel_box_food_w_pc34() == 60);
    CHECK(dm1_v1_champion_panel_box_food_h_pc34() == 68);
}

static void test_get_function(void)
{
    /* The component-indexed accessor returns 1 + writes the value
     * for valid indices, 0 for OOB.
     */
    int v;
    int rc;
    rc = dm1_v1_champion_panel_box_food_get_pc34(0, &v);
    CHECK(rc == 1);
    CHECK(v == 112);
    rc = dm1_v1_champion_panel_box_food_get_pc34(1, &v);
    CHECK(rc == 1);
    CHECK(v == 159);
    rc = dm1_v1_champion_panel_box_food_get_pc34(2, &v);
    CHECK(rc == 1);
    CHECK(v == 60);
    rc = dm1_v1_champion_panel_box_food_get_pc34(3, &v);
    CHECK(rc == 1);
    CHECK(v == 68);
    /* OOB component. */
    CHECK(dm1_v1_champion_panel_box_food_get_pc34(-1, &v) == 0);
    CHECK(v == -1);
    CHECK(dm1_v1_champion_panel_box_food_get_pc34(4, &v) == 0);
    CHECK(v == -1);
    /* NULL out_value rejected. */
    CHECK(dm1_v1_champion_panel_box_food_get_pc34(0, 0) == 0);
}

static void test_components_non_negative(void)
{
    /* All 4 components are non-negative. */
    CHECK(dm1_v1_champion_panel_box_food_x_pc34() >= 0);
    CHECK(dm1_v1_champion_panel_box_food_y_pc34() >= 0);
    CHECK(dm1_v1_champion_panel_box_food_w_pc34() > 0);
    CHECK(dm1_v1_champion_panel_box_food_h_pc34() > 0);
}

static void test_within_panel(void)
{
    /* The food label is drawn in the panel region. X=112 aligns
     * with the champion-panel left margin.
     */
    CHECK(dm1_v1_champion_panel_box_food_x_pc34() == 112);
}

static void test_run_accepted(void)
{
    DM1_V1_ChampionPanelBoxFoodResultPc34 r;
    int ok = dm1_v1_champion_panel_box_food_run_pc34(&r);
    int i;
    CHECK(ok == 1);
    CHECK(r.accepted == 1);
    CHECK(r.assertionCount == 11);
    CHECK(r.tableEntries[0] == 112);
    CHECK(r.tableEntries[1] == 159);
    CHECK(r.tableEntries[2] == 60);
    CHECK(r.tableEntries[3] == 68);
    CHECK(r.tableMatchesDeclaration == 1);
    CHECK(r.xIs112 == 1);
    CHECK(r.yIs159 == 1);
    CHECK(r.wIs60  == 1);
    CHECK(r.hIs68  == 1);
    CHECK(r.allComponentsNonNegative == 1);
    CHECK(r.widthPositive  == 1);
    CHECK(r.heightPositive == 1);
    CHECK(r.withinReasonableBounds == 1);
    CHECK(r.withinPanelLeftMargin == 1);
    /* Cross-check the result struct's tableEntries match the
     * source-of-truth accessor functions.
     */
    for (i = 0; i < 4; ++i) {
        int v;
        int rc = dm1_v1_champion_panel_box_food_get_pc34(i, &v);
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
    test_within_panel();
    test_run_accepted();
    printf("dm1_v1_champion_panel_box_food: "
           "%d/%d assertions passed\n",
           g_assertions - g_failures, g_assertions);
    return g_failures == 0 ? 0 : 1;
}
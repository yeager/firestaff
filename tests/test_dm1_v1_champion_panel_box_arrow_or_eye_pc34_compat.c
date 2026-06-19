#include "firestaff/dm1/v1/champion_panel_box_arrow_or_eye_pc34_compat.h"

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
    /* DATA.C:315 G0033 init: { 83, 98, 57, 65 }. */
    const int *t = dm1_v1_champion_panel_box_arrow_or_eye_table_pc34();
    CHECK(t != 0);
    CHECK(t[0] == 83);
    CHECK(t[1] == 98);
    CHECK(t[2] == 57);
    CHECK(t[3] == 65);
}

static void test_accessor_functions(void)
{
    /* All 4 single-component accessors return the expected value. */
    CHECK(dm1_v1_champion_panel_box_arrow_or_eye_x_pc34() == 83);
    CHECK(dm1_v1_champion_panel_box_arrow_or_eye_y_pc34() == 98);
    CHECK(dm1_v1_champion_panel_box_arrow_or_eye_w_pc34() == 57);
    CHECK(dm1_v1_champion_panel_box_arrow_or_eye_h_pc34() == 65);
}

static void test_get_function(void)
{
    int v;
    int rc;
    rc = dm1_v1_champion_panel_box_arrow_or_eye_get_pc34(0, &v);
    CHECK(rc == 1);
    CHECK(v == 83);
    rc = dm1_v1_champion_panel_box_arrow_or_eye_get_pc34(1, &v);
    CHECK(rc == 1);
    CHECK(v == 98);
    rc = dm1_v1_champion_panel_box_arrow_or_eye_get_pc34(2, &v);
    CHECK(rc == 1);
    CHECK(v == 57);
    rc = dm1_v1_champion_panel_box_arrow_or_eye_get_pc34(3, &v);
    CHECK(rc == 1);
    CHECK(v == 65);
    /* OOB component. */
    CHECK(dm1_v1_champion_panel_box_arrow_or_eye_get_pc34(-1, &v) == 0);
    CHECK(v == -1);
    CHECK(dm1_v1_champion_panel_box_arrow_or_eye_get_pc34(4, &v) == 0);
    CHECK(v == -1);
    /* NULL out_value rejected. */
    CHECK(dm1_v1_champion_panel_box_arrow_or_eye_get_pc34(0, 0) == 0);
}

static void test_components_non_negative(void)
{
    CHECK(dm1_v1_champion_panel_box_arrow_or_eye_x_pc34() >= 0);
    CHECK(dm1_v1_champion_panel_box_arrow_or_eye_y_pc34() >= 0);
    CHECK(dm1_v1_champion_panel_box_arrow_or_eye_w_pc34() > 0);
    CHECK(dm1_v1_champion_panel_box_arrow_or_eye_h_pc34() > 0);
}

static void test_run_accepted(void)
{
    DM1_V1_ChampionPanelBoxArrowOrEyeResultPc34 r;
    int ok = dm1_v1_champion_panel_box_arrow_or_eye_run_pc34(&r);
    int i;
    CHECK(ok == 1);
    CHECK(r.accepted == 1);
    CHECK(r.assertionCount == 11);
    CHECK(r.tableEntries[0] == 83);
    CHECK(r.tableEntries[1] == 98);
    CHECK(r.tableEntries[2] == 57);
    CHECK(r.tableEntries[3] == 65);
    CHECK(r.tableMatchesDeclaration == 1);
    CHECK(r.xIs83 == 1);
    CHECK(r.yIs98 == 1);
    CHECK(r.wIs57 == 1);
    CHECK(r.hIs65 == 1);
    CHECK(r.allComponentsNonNegative == 1);
    CHECK(r.widthPositive  == 1);
    CHECK(r.heightPositive == 1);
    CHECK(r.withinReasonableBounds == 1);
    CHECK(r.withinPanelRegion == 1);
    for (i = 0; i < 4; ++i) {
        int v;
        int rc = dm1_v1_champion_panel_box_arrow_or_eye_get_pc34(i, &v);
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
    test_run_accepted();
    printf("dm1_v1_champion_panel_box_arrow_or_eye: "
           "%d/%d assertions passed\n",
           g_assertions - g_failures, g_assertions);
    return g_failures == 0 ? 0 : 1;
}
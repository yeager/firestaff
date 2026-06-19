#include "firestaff/dm1/v1/champion_panel_box_panel_pc34_compat.h"

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
    /* DATA.C:314 G0032 init: { 80, 223, 52, 124 }. */
    const int *t = dm1_v1_champion_panel_box_panel_table_pc34();
    CHECK(t != 0);
    CHECK(t[0] == 80);
    CHECK(t[1] == 223);
    CHECK(t[2] == 52);
    CHECK(t[3] == 124);
}

static void test_accessor_functions(void)
{
    CHECK(dm1_v1_champion_panel_box_panel_x_pc34() == 80);
    CHECK(dm1_v1_champion_panel_box_panel_y_pc34() == 223);
    CHECK(dm1_v1_champion_panel_box_panel_w_pc34() == 52);
    CHECK(dm1_v1_champion_panel_box_panel_h_pc34() == 124);
}

static void test_get_function(void)
{
    int v;
    int rc;
    rc = dm1_v1_champion_panel_box_panel_get_pc34(0, &v);
    CHECK(rc == 1);
    CHECK(v == 80);
    rc = dm1_v1_champion_panel_box_panel_get_pc34(1, &v);
    CHECK(rc == 1);
    CHECK(v == 223);
    rc = dm1_v1_champion_panel_box_panel_get_pc34(2, &v);
    CHECK(rc == 1);
    CHECK(v == 52);
    rc = dm1_v1_champion_panel_box_panel_get_pc34(3, &v);
    CHECK(rc == 1);
    CHECK(v == 124);
    CHECK(dm1_v1_champion_panel_box_panel_get_pc34(-1, &v) == 0);
    CHECK(v == -1);
    CHECK(dm1_v1_champion_panel_box_panel_get_pc34(4, &v) == 0);
    CHECK(v == -1);
    CHECK(dm1_v1_champion_panel_box_panel_get_pc34(0, 0) == 0);
}

static void test_components_non_negative(void)
{
    CHECK(dm1_v1_champion_panel_box_panel_x_pc34() >= 0);
    CHECK(dm1_v1_champion_panel_box_panel_y_pc34() >= 0);
    CHECK(dm1_v1_champion_panel_box_panel_w_pc34() > 0);
    CHECK(dm1_v1_champion_panel_box_panel_h_pc34() > 0);
}

static void test_panel_taller_than_wide(void)
{
    /* Panel is vertical: H (124) > W (52). */
    CHECK(dm1_v1_champion_panel_box_panel_h_pc34() >
          dm1_v1_champion_panel_box_panel_w_pc34());
}

static void test_run_accepted(void)
{
    DM1_V1_ChampionPanelBoxPanelResultPc34 r;
    int ok = dm1_v1_champion_panel_box_panel_run_pc34(&r);
    int i;
    CHECK(ok == 1);
    CHECK(r.accepted == 1);
    CHECK(r.assertionCount == 11);
    CHECK(r.tableEntries[0] == 80);
    CHECK(r.tableEntries[1] == 223);
    CHECK(r.tableEntries[2] == 52);
    CHECK(r.tableEntries[3] == 124);
    CHECK(r.tableMatchesDeclaration == 1);
    CHECK(r.xIs80 == 1);
    CHECK(r.yIs223 == 1);
    CHECK(r.wIs52 == 1);
    CHECK(r.hIs124 == 1);
    CHECK(r.allComponentsNonNegative == 1);
    CHECK(r.widthPositive == 1);
    CHECK(r.heightPositive == 1);
    CHECK(r.withinReasonableBounds == 1);
    CHECK(r.heightLargerThanWidth == 1);
    for (i = 0; i < 4; ++i) {
        int v;
        int rc = dm1_v1_champion_panel_box_panel_get_pc34(i, &v);
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
    test_panel_taller_than_wide();
    test_run_accepted();
    printf("dm1_v1_champion_panel_box_panel: "
           "%d/%d assertions passed\n",
           g_assertions - g_failures, g_assertions);
    return g_failures == 0 ? 0 : 1;
}
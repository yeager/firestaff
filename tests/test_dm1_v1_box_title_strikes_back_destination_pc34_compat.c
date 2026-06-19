#include "firestaff/dm1/v1/box_title_strikes_back_destination_pc34_compat.h"

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
    /* DATA.C:125 G0003 init: { 0, 319, 118, 174 }. */
    const int *t = dm1_v1_box_title_strikes_back_destination_table_pc34();
    int n = dm1_v1_box_title_strikes_back_destination_size_pc34();
    CHECK(t != 0);
    CHECK(n == 4);
    CHECK(t[0] == 0);
    CHECK(t[1] == 319);
    CHECK(t[2] == 118);
    CHECK(t[3] == 174);
}

static void test_accessor_functions(void)
{
    CHECK(dm1_v1_box_title_strikes_back_destination_x_pc34() == 0);
    CHECK(dm1_v1_box_title_strikes_back_destination_y_pc34() == 319);
    CHECK(dm1_v1_box_title_strikes_back_destination_w_pc34() == 118);
    CHECK(dm1_v1_box_title_strikes_back_destination_h_pc34() == 174);
}

static void test_get_function(void)
{
    int v;
    int rc;
    rc = dm1_v1_box_title_strikes_back_destination_get_pc34(0, &v);
    CHECK(rc == 1);
    CHECK(v == 0);
    rc = dm1_v1_box_title_strikes_back_destination_get_pc34(1, &v);
    CHECK(rc == 1);
    CHECK(v == 319);
    rc = dm1_v1_box_title_strikes_back_destination_get_pc34(2, &v);
    CHECK(rc == 1);
    CHECK(v == 118);
    rc = dm1_v1_box_title_strikes_back_destination_get_pc34(3, &v);
    CHECK(rc == 1);
    CHECK(v == 174);
    CHECK(dm1_v1_box_title_strikes_back_destination_get_pc34(-1, &v) == 0);
    CHECK(v == -1);
    CHECK(dm1_v1_box_title_strikes_back_destination_get_pc34(4, &v) == 0);
    CHECK(v == -1);
    CHECK(dm1_v1_box_title_strikes_back_destination_get_pc34(0, 0) == 0);
}

static void test_components_non_negative(void)
{
    CHECK(dm1_v1_box_title_strikes_back_destination_x_pc34() >= 0);
    CHECK(dm1_v1_box_title_strikes_back_destination_y_pc34() >= 0);
    CHECK(dm1_v1_box_title_strikes_back_destination_w_pc34() > 0);
    CHECK(dm1_v1_box_title_strikes_back_destination_h_pc34() > 0);
}

static void test_run_accepted(void)
{
    DM1_V1_BoxTitleStrikesBackDestinationResultPc34 r;
    int ok = dm1_v1_box_title_strikes_back_destination_run_pc34(&r);
    int i;
    CHECK(ok == 1);
    CHECK(r.accepted == 1);
    CHECK(r.assertionCount == 11);
    CHECK(r.tableSize == 4);
    CHECK(r.tableEntries[0] == 0);
    CHECK(r.tableEntries[1] == 319);
    CHECK(r.tableEntries[2] == 118);
    CHECK(r.tableEntries[3] == 174);
    CHECK(r.tableMatchesDeclaration == 1);
    CHECK(r.xIs0 == 1);
    CHECK(r.yIs319 == 1);
    CHECK(r.wIs118 == 1);
    CHECK(r.hIs174 == 1);
    CHECK(r.allComponentsNonNegative == 1);
    CHECK(r.widthPositive == 1);
    CHECK(r.heightPositive == 1);
    CHECK(r.withinRowRange == 1);
    CHECK(r.withinBoxBounds == 1);
    for (i = 0; i < 4; ++i) {
        int v;
        int rc = dm1_v1_box_title_strikes_back_destination_get_pc34(i, &v);
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
    printf("dm1_v1_box_title_strikes_back_destination: "
           "%d/%d assertions passed\n",
           g_assertions - g_failures, g_assertions);
    return g_failures == 0 ? 0 : 1;
}
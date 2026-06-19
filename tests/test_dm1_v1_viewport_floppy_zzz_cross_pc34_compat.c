#include "firestaff/dm1/v1/viewport_floppy_zzz_cross_pc34_compat.h"

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
    const int *t = dm1_v1_viewport_floppy_zzz_cross_table_pc34();
    int n = dm1_v1_viewport_floppy_zzz_cross_size_pc34();
    CHECK(t != 0);
    CHECK(n == 4);
    CHECK(t[0] == 174);
    CHECK(t[1] == 218);
    CHECK(t[2] == 2);
    CHECK(t[3] == 12);
}

static void test_accessor_functions(void)
{
    CHECK(dm1_v1_viewport_floppy_zzz_cross_x_pc34() == 174);
    CHECK(dm1_v1_viewport_floppy_zzz_cross_y_pc34() == 218);
    CHECK(dm1_v1_viewport_floppy_zzz_cross_w_pc34() == 2);
    CHECK(dm1_v1_viewport_floppy_zzz_cross_h_pc34() == 12);
}

static void test_get_function(void)
{
    int v;
    int rc;
    rc = dm1_v1_viewport_floppy_zzz_cross_get_pc34(0, &v);
    CHECK(rc == 1);
    CHECK(v == 174);
    rc = dm1_v1_viewport_floppy_zzz_cross_get_pc34(1, &v);
    CHECK(rc == 1);
    CHECK(v == 218);
    rc = dm1_v1_viewport_floppy_zzz_cross_get_pc34(2, &v);
    CHECK(rc == 1);
    CHECK(v == 2);
    rc = dm1_v1_viewport_floppy_zzz_cross_get_pc34(3, &v);
    CHECK(rc == 1);
    CHECK(v == 12);
    CHECK(dm1_v1_viewport_floppy_zzz_cross_get_pc34(-1, &v) == 0);
    CHECK(v == -1);
    CHECK(dm1_v1_viewport_floppy_zzz_cross_get_pc34(4, &v) == 0);
    CHECK(v == -1);
    CHECK(dm1_v1_viewport_floppy_zzz_cross_get_pc34(0, 0) == 0);
}

static void test_components_non_negative(void)
{
    CHECK(dm1_v1_viewport_floppy_zzz_cross_x_pc34() >= 0);
    CHECK(dm1_v1_viewport_floppy_zzz_cross_y_pc34() >= 0);
    CHECK(dm1_v1_viewport_floppy_zzz_cross_w_pc34() > 0);
    CHECK(dm1_v1_viewport_floppy_zzz_cross_h_pc34() > 0);
}

static void test_run_accepted(void)
{
    DM1_V1_BoxViewportFloppyZzzCrossResultPc34 r;
    int ok = dm1_v1_viewport_floppy_zzz_cross_run_pc34(&r);
    int i;
    CHECK(ok == 1);
    CHECK(r.accepted == 1);
    CHECK(r.assertionCount == 11);
    CHECK(r.tableSize == 4);
    CHECK(r.tableEntries[0] == 174);
    CHECK(r.tableEntries[1] == 218);
    CHECK(r.tableEntries[2] == 2);
    CHECK(r.tableEntries[3] == 12);
    CHECK(r.tableMatchesDeclaration == 1);
    CHECK(r.xIs174 == 1);
    CHECK(r.yIs218 == 1);
    CHECK(r.wIs2 == 1);
    CHECK(r.hIs12 == 1);
    CHECK(r.allComponentsNonNegative == 1);
    CHECK(r.widthPositive == 1);
    CHECK(r.heightPositive == 1);
    CHECK(r.withinRowRange == 1);
    CHECK(r.withinBoxBounds == 1);
    for (i = 0; i < 4; ++i) {
        int v;
        int rc = dm1_v1_viewport_floppy_zzz_cross_get_pc34(i, &v);
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
    printf("dm1_v1_viewport_floppy_zzz_cross: %d/%d assertions passed\n",
           g_assertions - g_failures, g_assertions);
    return g_failures == 0 ? 0 : 1;
}

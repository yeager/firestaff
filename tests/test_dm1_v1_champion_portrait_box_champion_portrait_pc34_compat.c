#include "firestaff/dm1/v1/champion_portrait_box_champion_portrait_pc34_compat.h"

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
    /* DATA.C:424 G0047 init: { 0, 31, 0, 28 }. */
    const unsigned char *t = dm1_v1_box_champion_portrait_table_pc34();
    int n = dm1_v1_box_champion_portrait_size_pc34();
    CHECK(t != 0);
    CHECK(n == 4);
    CHECK(t[0] == 0);
    CHECK(t[1] == 31);
    CHECK(t[2] == 0);
    CHECK(t[3] == 28);
}

static void test_accessor_functions(void)
{
    CHECK(dm1_v1_box_champion_portrait_x_pc34() == 0);
    CHECK(dm1_v1_box_champion_portrait_y_pc34() == 31);
    CHECK(dm1_v1_box_champion_portrait_w_pc34() == 0);
    CHECK(dm1_v1_box_champion_portrait_h_pc34() == 28);
}

static void test_get_function(void)
{
    int v;
    int rc;
    rc = dm1_v1_box_champion_portrait_get_pc34(0, &v);
    CHECK(rc == 1);
    CHECK(v == 0);
    rc = dm1_v1_box_champion_portrait_get_pc34(1, &v);
    CHECK(rc == 1);
    CHECK(v == 31);
    rc = dm1_v1_box_champion_portrait_get_pc34(2, &v);
    CHECK(rc == 1);
    CHECK(v == 0);
    rc = dm1_v1_box_champion_portrait_get_pc34(3, &v);
    CHECK(rc == 1);
    CHECK(v == 28);
    CHECK(dm1_v1_box_champion_portrait_get_pc34(-1, &v) == 0);
    CHECK(v == -1);
    CHECK(dm1_v1_box_champion_portrait_get_pc34(4, &v) == 0);
    CHECK(v == -1);
    CHECK(dm1_v1_box_champion_portrait_get_pc34(0, 0) == 0);
}

static void test_components_in_byte_range(void)
{
    /* All 4 components in [0, 255] (unsigned char range). */
    CHECK(dm1_v1_box_champion_portrait_x_pc34() >= 0);
    CHECK(dm1_v1_box_champion_portrait_x_pc34() <= 255);
    CHECK(dm1_v1_box_champion_portrait_y_pc34() >= 0);
    CHECK(dm1_v1_box_champion_portrait_y_pc34() <= 255);
    CHECK(dm1_v1_box_champion_portrait_w_pc34() >= 0);
    CHECK(dm1_v1_box_champion_portrait_w_pc34() <= 255);
    CHECK(dm1_v1_box_champion_portrait_h_pc34() >= 0);
    CHECK(dm1_v1_box_champion_portrait_h_pc34() <= 255);
}

static void test_y31_h28(void)
{
    /* G0047 specific: Y=31 (byte width of portrait extraction),
     * H=28 (byte height of portrait extraction).
     */
    CHECK(dm1_v1_box_champion_portrait_y_pc34() == 31);
    CHECK(dm1_v1_box_champion_portrait_h_pc34() == 28);
}

static void test_run_accepted(void)
{
    DM1_V1_BoxChampionPortraitResultPc34 r;
    int ok = dm1_v1_box_champion_portrait_run_pc34(&r);
    int i;
    CHECK(ok == 1);
    CHECK(r.accepted == 1);
    CHECK(r.assertionCount == 11);
    CHECK(r.tableSize == 4);
    CHECK(r.tableEntries[0] == 0);
    CHECK(r.tableEntries[1] == 31);
    CHECK(r.tableEntries[2] == 0);
    CHECK(r.tableEntries[3] == 28);
    CHECK(r.tableMatchesDeclaration == 1);
    CHECK(r.xIs0 == 1);
    CHECK(r.yIs31 == 1);
    CHECK(r.wIs0 == 1);
    CHECK(r.hIs28 == 1);
    CHECK(r.allComponentsNonNegative == 1);
    CHECK(r.widthPositive == 1);
    CHECK(r.heightPositive == 1);
    CHECK(r.byteAligned == 1);
    CHECK(r.withinByteRange == 1);
    for (i = 0; i < 4; ++i) {
        int v;
        int rc = dm1_v1_box_champion_portrait_get_pc34(i, &v);
        CHECK(rc == 1);
        CHECK(r.tableEntries[i] == (unsigned char)v);
    }
}

int main(void)
{
    test_table_values();
    test_accessor_functions();
    test_get_function();
    test_components_in_byte_range();
    test_y31_h28();
    test_run_accepted();
    printf("dm1_v1_champion_portrait_box_champion_portrait: "
           "%d/%d assertions passed\n",
           g_assertions - g_failures, g_assertions);
    return g_failures == 0 ? 0 : 1;
}
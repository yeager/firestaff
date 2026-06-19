#include "firestaff/dm1/v1/champion_portrait_box_eye_pc34_compat.h"

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
    /* DATA.C:423 G0048 init: { 11, 28, 12, 29 }. */
    const int *t = dm1_v1_champion_portrait_box_eye_table_pc34();
    CHECK(t != 0);
    CHECK(t[0] == 11);
    CHECK(t[1] == 28);
    CHECK(t[2] == 12);
    CHECK(t[3] == 29);
}

static void test_accessor_functions(void)
{
    CHECK(dm1_v1_champion_portrait_box_eye_x_pc34() == 11);
    CHECK(dm1_v1_champion_portrait_box_eye_y_pc34() == 28);
    CHECK(dm1_v1_champion_portrait_box_eye_w_pc34() == 12);
    CHECK(dm1_v1_champion_portrait_box_eye_h_pc34() == 29);
}

static void test_get_function(void)
{
    int v;
    int rc;
    rc = dm1_v1_champion_portrait_box_eye_get_pc34(0, &v);
    CHECK(rc == 1);
    CHECK(v == 11);
    rc = dm1_v1_champion_portrait_box_eye_get_pc34(1, &v);
    CHECK(rc == 1);
    CHECK(v == 28);
    rc = dm1_v1_champion_portrait_box_eye_get_pc34(2, &v);
    CHECK(rc == 1);
    CHECK(v == 12);
    rc = dm1_v1_champion_portrait_box_eye_get_pc34(3, &v);
    CHECK(rc == 1);
    CHECK(v == 29);
    CHECK(dm1_v1_champion_portrait_box_eye_get_pc34(-1, &v) == 0);
    CHECK(v == -1);
    CHECK(dm1_v1_champion_portrait_box_eye_get_pc34(4, &v) == 0);
    CHECK(v == -1);
    CHECK(dm1_v1_champion_portrait_box_eye_get_pc34(0, 0) == 0);
}

static void test_components_non_negative(void)
{
    CHECK(dm1_v1_champion_portrait_box_eye_x_pc34() >= 0);
    CHECK(dm1_v1_champion_portrait_box_eye_y_pc34() >= 0);
    CHECK(dm1_v1_champion_portrait_box_eye_w_pc34() > 0);
    CHECK(dm1_v1_champion_portrait_box_eye_h_pc34() > 0);
}

static void test_eye_taller_than_wide(void)
{
    /* Small vertical eye box aspect: H=29 > W=12. */
    CHECK(dm1_v1_champion_portrait_box_eye_h_pc34() >
          dm1_v1_champion_portrait_box_eye_w_pc34());
}

static void test_run_accepted(void)
{
    DM1_V1_ChampionPortraitBoxEyeResultPc34 r;
    int ok = dm1_v1_champion_portrait_box_eye_run_pc34(&r);
    int i;
    CHECK(ok == 1);
    CHECK(r.accepted == 1);
    CHECK(r.assertionCount == 11);
    CHECK(r.tableEntries[0] == 11);
    CHECK(r.tableEntries[1] == 28);
    CHECK(r.tableEntries[2] == 12);
    CHECK(r.tableEntries[3] == 29);
    CHECK(r.tableMatchesDeclaration == 1);
    CHECK(r.xIs11 == 1);
    CHECK(r.yIs28 == 1);
    CHECK(r.wIs12 == 1);
    CHECK(r.hIs29 == 1);
    CHECK(r.allComponentsNonNegative == 1);
    CHECK(r.widthPositive == 1);
    CHECK(r.heightPositive == 1);
    CHECK(r.withinReasonableBounds == 1);
    CHECK(r.heightLargerThanWidth == 1);
    for (i = 0; i < 4; ++i) {
        int v;
        int rc = dm1_v1_champion_portrait_box_eye_get_pc34(i, &v);
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
    test_eye_taller_than_wide();
    test_run_accepted();
    printf("dm1_v1_champion_portrait_box_eye: "
           "%d/%d assertions passed\n",
           g_assertions - g_failures, g_assertions);
    return g_failures == 0 ? 0 : 1;
}
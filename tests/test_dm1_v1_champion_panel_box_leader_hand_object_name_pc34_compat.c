#include "firestaff/dm1/v1/champion_panel_box_leader_hand_object_name_pc34_compat.h"

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
    /* DATA.C:262 G0028 init: { 233, 319, 33, 38 }. */
    const int *t = dm1_v1_champion_panel_box_leader_hand_object_name_table_pc34();
    CHECK(t != 0);
    CHECK(t[0] == 233);
    CHECK(t[1] == 319);
    CHECK(t[2] == 33);
    CHECK(t[3] == 38);
}

static void test_accessor_functions(void)
{
    CHECK(dm1_v1_champion_panel_box_leader_hand_object_name_x_pc34() == 233);
    CHECK(dm1_v1_champion_panel_box_leader_hand_object_name_y_pc34() == 319);
    CHECK(dm1_v1_champion_panel_box_leader_hand_object_name_w_pc34() == 33);
    CHECK(dm1_v1_champion_panel_box_leader_hand_object_name_h_pc34() == 38);
}

static void test_get_function(void)
{
    int v;
    int rc;
    rc = dm1_v1_champion_panel_box_leader_hand_object_name_get_pc34(0, &v);
    CHECK(rc == 1);
    CHECK(v == 233);
    rc = dm1_v1_champion_panel_box_leader_hand_object_name_get_pc34(1, &v);
    CHECK(rc == 1);
    CHECK(v == 319);
    rc = dm1_v1_champion_panel_box_leader_hand_object_name_get_pc34(2, &v);
    CHECK(rc == 1);
    CHECK(v == 33);
    rc = dm1_v1_champion_panel_box_leader_hand_object_name_get_pc34(3, &v);
    CHECK(rc == 1);
    CHECK(v == 38);
    CHECK(dm1_v1_champion_panel_box_leader_hand_object_name_get_pc34(-1, &v) == 0);
    CHECK(v == -1);
    CHECK(dm1_v1_champion_panel_box_leader_hand_object_name_get_pc34(4, &v) == 0);
    CHECK(v == -1);
    CHECK(dm1_v1_champion_panel_box_leader_hand_object_name_get_pc34(0, 0) == 0);
}

static void test_components_non_negative(void)
{
    CHECK(dm1_v1_champion_panel_box_leader_hand_object_name_x_pc34() >= 0);
    CHECK(dm1_v1_champion_panel_box_leader_hand_object_name_y_pc34() >= 0);
    CHECK(dm1_v1_champion_panel_box_leader_hand_object_name_w_pc34() > 0);
    CHECK(dm1_v1_champion_panel_box_leader_hand_object_name_h_pc34() > 0);
}

static void test_run_accepted(void)
{
    DM1_V1_ChampionPanelBoxLeaderHandObjectNameResultPc34 r;
    int ok = dm1_v1_champion_panel_box_leader_hand_object_name_run_pc34(&r);
    int i;
    CHECK(ok == 1);
    CHECK(r.accepted == 1);
    CHECK(r.assertionCount == 10);
    CHECK(r.tableEntries[0] == 233);
    CHECK(r.tableEntries[1] == 319);
    CHECK(r.tableEntries[2] == 33);
    CHECK(r.tableEntries[3] == 38);
    CHECK(r.tableMatchesDeclaration == 1);
    CHECK(r.xIs233 == 1);
    CHECK(r.yIs319 == 1);
    CHECK(r.wIs33  == 1);
    CHECK(r.hIs38  == 1);
    CHECK(r.allComponentsNonNegative == 1);
    CHECK(r.widthPositive  == 1);
    CHECK(r.heightPositive == 1);
    CHECK(r.withinReasonableBounds == 1);
    for (i = 0; i < 4; ++i) {
        int v;
        int rc = dm1_v1_champion_panel_box_leader_hand_object_name_get_pc34(i, &v);
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
    printf("dm1_v1_champion_panel_box_leader_hand_object_name: "
           "%d/%d assertions passed\n",
           g_assertions - g_failures, g_assertions);
    return g_failures == 0 ? 0 : 1;
}
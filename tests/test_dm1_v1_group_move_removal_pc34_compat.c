#include "dm1_v1_teleporter_pit_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_failed = 0;

static void expect_int(const char* name, int actual, int expected) {
    if (actual != expected) {
        fprintf(stderr, "FAIL %s: got %d expected %d\n", name, actual, expected);
        g_failed = 1;
    }
}

static void expect_contains(const char* name, const char* haystack, const char* needle) {
    if (!haystack || !strstr(haystack, needle)) {
        fprintf(stderr, "FAIL %s: missing %s\n", name, needle);
        g_failed = 1;
    }
}

static void test_allowed_group_keeps_move(void) {
    M11_GroupMoveRemovalPlan plan;

    expect_int("allowed_plan_ok",
        m11_plan_group_move_removal_after_pit_teleporter(
            0, 1, 4, 5, 7, 8, &plan), 1);
    expect_int("allowed_move_not_prevented", plan.movePrevented, 0);
    expect_int("allowed_reason_none", plan.reason, M11_GROUP_MOVE_REMOVAL_REASON_NONE);
    expect_int("allowed_no_fixed_drop", plan.dropMovingCreatureFixedPossessions, 0);
    expect_int("allowed_no_group_drop", plan.dropGroupPossessions, 0);
    expect_int("allowed_no_source_delete", plan.deleteSourceGroup, 0);
}

static void test_fall_killed_group_drops_and_deletes_source(void) {
    M11_GroupMoveRemovalPlan plan;

    expect_int("fall_killed_plan_ok",
        m11_plan_group_move_removal_after_pit_teleporter(
            1, 1, 4, 5, 7, 8, &plan), 1);
    expect_int("fall_killed_move_prevented", plan.movePrevented, 1);
    expect_int("fall_killed_reason", plan.reason, M11_GROUP_MOVE_REMOVAL_REASON_FALL_KILLED);
    expect_int("fall_killed_fixed_drop", plan.dropMovingCreatureFixedPossessions, 1);
    expect_int("fall_killed_group_drop", plan.dropGroupPossessions, 1);
    expect_int("fall_killed_drop_mode", plan.dropGroupPossessionsSoundMode,
        M11_GROUP_MOVE_REMOVAL_SOUND_ONE_TICK_LATER);
    expect_int("fall_killed_drop_x", plan.dropMapX, 7);
    expect_int("fall_killed_drop_y", plan.dropMapY, 8);
    expect_int("fall_killed_delete_source", plan.deleteSourceGroup, 1);
    expect_int("fall_killed_delete_x", plan.deleteMapX, 4);
    expect_int("fall_killed_delete_y", plan.deleteMapY, 5);
}

static void test_not_allowed_group_drops_without_placement_delete(void) {
    M11_GroupMoveRemovalPlan plan;

    expect_int("not_allowed_plan_ok",
        m11_plan_group_move_removal_after_pit_teleporter(
            0, 0, -1, 0, 3, 2, &plan), 1);
    expect_int("not_allowed_move_prevented", plan.movePrevented, 1);
    expect_int("not_allowed_reason", plan.reason, M11_GROUP_MOVE_REMOVAL_REASON_NOT_ALLOWED);
    expect_int("not_allowed_fixed_drop", plan.dropMovingCreatureFixedPossessions, 1);
    expect_int("not_allowed_group_drop", plan.dropGroupPossessions, 1);
    expect_int("not_allowed_drop_mode", plan.dropGroupPossessionsSoundMode,
        M11_GROUP_MOVE_REMOVAL_SOUND_ONE_TICK_LATER);
    expect_int("not_allowed_drop_x", plan.dropMapX, 3);
    expect_int("not_allowed_drop_y", plan.dropMapY, 2);
    expect_int("not_allowed_no_source_delete", plan.deleteSourceGroup, 0);
}

static void test_source_evidence(void) {
    const char* evidence = m11_group_move_removal_source_evidence();

    expect_contains("evidence_f0267_damage", evidence, "MOVESENS.C F0267 lines 608-624");
    expect_contains("evidence_f0267_removal", evidence, "MOVESENS.C F0267 lines 656-663");
    expect_contains("evidence_f0187", evidence, "GROUP.C F0187 lines 648-674");
    expect_contains("evidence_f0188", evidence, "GROUP.C F0188 lines 676-737");
    expect_contains("evidence_f0189", evidence, "GROUP.C F0189 lines 739-762");
}

int main(void) {
    test_allowed_group_keeps_move();
    test_fall_killed_group_drops_and_deletes_source();
    test_not_allowed_group_drops_without_placement_delete();
    test_source_evidence();

    if (g_failed) return 1;
    puts("DM1_V1_GROUP_MOVE_REMOVAL_PC34_COMPAT_OK");
    return 0;
}

#include "dm1_v1_teleporter_pit_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_teleporter_rotation_null(void)
{
    int outDir = -1, outCell = -1;
    int rc = DM1_V1_ApplyTeleporterRotationF0267Pc34Compat(
        DM1_V1_TELEPORTER_ROTATE_THING_PARTY_PC34,
        0, NULL, 0, 0, &outDir, &outCell);
    (void)rc;
    assert(rc == 0);
}

static void test_teleporter_rotation_party(void)
{
    DM1_V1_TeleporterDefPc34 tp;
    memset(&tp, 0, sizeof(tp));
    tp.absoluteRotation = 1;
    tp.destFacing = 2;
    int outDir = -1, outCell = -1;
    int rc = DM1_V1_ApplyTeleporterRotationF0267Pc34Compat(
        DM1_V1_TELEPORTER_ROTATE_THING_PARTY_PC34,
        5, &tp, 0, 0, &outDir, &outCell);
    (void)rc;
    assert(rc == 1);
    assert(outDir >= 0 && outDir <= 3);
}

static void test_group_teleporter_rotation_null(void)
{
    unsigned int outDirs = 0, outCells = 0;
    int rc = DM1_V1_ApplyGroupTeleporterRotationF0262Pc34Compat(
        NULL, 0, 0, 0, 0, &outDirs, &outCells);
    (void)rc;
    assert(rc == 0);
}

static void test_group_value_updated(void)
{
    unsigned int val = DM1_V1_GroupValueUpdatedWithCreatureValueF0178Pc34Compat(
        0x00, 0, 2);
    (void)val;
    assert((val & 0x03) == 2);
}

static void test_group_value_updated_index1(void)
{
    unsigned int val = DM1_V1_GroupValueUpdatedWithCreatureValueF0178Pc34Compat(
        0x00, 1, 3);
    (void)val;
    assert(((val >> 2) & 0x03) == 3);
}

static void test_plan_group_move_removal_null(void)
{
    int rc = DM1_V1_PlanGroupMoveRemovalAfterPitTeleporterF0267Pc34Compat(
        0, 1, 0, 0, 5, 5, NULL);
    (void)rc;
    assert(rc == 0);
}

static void test_plan_group_move_removal_fall_killed(void)
{
    DM1_V1_GroupMoveRemovalPlanPc34 plan;
    memset(&plan, 0, sizeof(plan));
    int rc = DM1_V1_PlanGroupMoveRemovalAfterPitTeleporterF0267Pc34Compat(
        1, 1, 3, 3, 5, 5, &plan);
    (void)rc;
    assert(rc == 1);
    assert(plan.movePrevented == 1);
    assert(plan.reason == DM1_V1_GROUP_MOVE_REMOVAL_REASON_FALL_KILLED_PC34);
}

static void test_plan_group_move_removal_not_allowed(void)
{
    DM1_V1_GroupMoveRemovalPlanPc34 plan;
    memset(&plan, 0, sizeof(plan));
    int rc = DM1_V1_PlanGroupMoveRemovalAfterPitTeleporterF0267Pc34Compat(
        0, 0, 3, 3, 5, 5, &plan);
    (void)rc;
    assert(rc == 1);
    assert(plan.movePrevented == 1);
    assert(plan.reason == DM1_V1_GROUP_MOVE_REMOVAL_REASON_NOT_ALLOWED_PC34);
}

static void test_plan_deferred_group_move_route_null(void)
{
    int rc = DM1_V1_PlanDeferredGroupMoveRouteF0267Pc34Compat(
        0, 1, 0, 0, 0, 100, 5, 5, 0, 0, NULL);
    (void)rc;
    assert(rc == 0);
}

static void test_plan_ordinary_group_move_null(void)
{
    int rc = DM1_V1_PlanOrdinaryGroupMoveF0267Pc34Compat(
        5, 5, 0, 1, 0, 0, 100, NULL);
    (void)rc;
    assert(rc == 0);
}

static void test_plan_ordinary_group_move_apply_null(void)
{
    int rc = DM1_V1_PlanOrdinaryGroupMoveApplyF0267Pc34Compat(
        NULL, 0, 5, 5, 0, 0xFF, 100, NULL);
    (void)rc;
    assert(rc == 0);
}

static void test_plan_pit_fall_square_null(void)
{
    int rc = DM1_V1_PlanGroupPitFallSquareF0267Pc34Compat(
        2, 2, 1, 0, NULL);
    (void)rc;
    assert(rc == 0);
}

static void test_plan_pit_fall_square_open(void)
{
    DM1_V1_GroupPitFallSquarePlanPc34 plan;
    memset(&plan, 0, sizeof(plan));
    int rc = DM1_V1_PlanGroupPitFallSquareF0267Pc34Compat(
        2, 2, 1, 0, &plan);
    (void)rc;
    assert(rc == 1);
    assert(plan.valid == 1);
    assert(plan.shouldFall == 1);
}

static void test_plan_teleporter_destination_null(void)
{
    int rc = DM1_V1_PlanGroupTeleporterDestinationF0267Pc34Compat(
        3, 3, 1, 1, 0, 0, 0, 5, 5, 0, 3, 3, 1, NULL);
    (void)rc;
    assert(rc == 0);
}

static void test_plan_generated_group_placement_null(void)
{
    int rc = DM1_V1_PlanGeneratedGroupPlacementF0183F0180Pc34Compat(
        0, 0, 5, 5, 0, 1, 0xFF, 0, 0, 10, 100, NULL);
    (void)rc;
    assert(rc == 0);
}

static void test_plan_lord_chaos_adjacent_retry_null(void)
{
    int rc = DM1_V1_PlanLordChaosAdjacentRetryF0252Pc34Compat(
        0, 1, 0, 5, 5, 1, 0, NULL);
    (void)rc;
    assert(rc == 0);
}

static void test_teleporter_source_evidence(void)
{
    const char *ev = DM1_V1_TeleporterRotation_SourceEvidencePc34Compat();
    (void)ev;
    assert(ev != NULL);
    assert(strlen(ev) > 0);
}

static void test_group_move_removal_source_evidence(void)
{
    const char *ev = DM1_V1_GroupMoveRemoval_SourceEvidencePc34Compat();
    (void)ev;
    assert(ev != NULL);
    assert(strlen(ev) > 0);
}

static void test_direction_constants(void)
{
    assert(DM1_V1_DIRECTION_NORTH_PC34 == 0);
    assert(DM1_V1_DIRECTION_EAST_PC34 == 1);
    assert(DM1_V1_DIRECTION_SOUTH_PC34 == 2);
    assert(DM1_V1_DIRECTION_WEST_PC34 == 3);
}

int main(void)
{
    test_teleporter_rotation_null();
    test_teleporter_rotation_party();
    test_group_teleporter_rotation_null();
    test_group_value_updated();
    test_group_value_updated_index1();
    test_plan_group_move_removal_null();
    test_plan_group_move_removal_fall_killed();
    test_plan_group_move_removal_not_allowed();
    test_plan_deferred_group_move_route_null();
    test_plan_ordinary_group_move_null();
    test_plan_ordinary_group_move_apply_null();
    test_plan_pit_fall_square_null();
    test_plan_pit_fall_square_open();
    test_plan_teleporter_destination_null();
    test_plan_generated_group_placement_null();
    test_plan_lord_chaos_adjacent_retry_null();
    test_teleporter_source_evidence();
    test_group_move_removal_source_evidence();
    test_direction_constants();

    puts("ok: DM1 teleporter/pit (Q-DM1-04) 19 tests passed");
    return 0;
}

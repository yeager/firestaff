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
    expect_contains("evidence_f0252", evidence, "TIMELINE.C F0252 lines 1527-1567");
}

static void test_deferred_route_plan(void) {
    M11_GroupMoveRoutePlan plan;

    expect_int("route_insert_ok",
        m11_plan_deferred_group_move_route_f0267(
            0, 1, 0, 1, 0, 100u, 7, 8, 0, 0, &plan), 1);
    expect_int("route_insert", plan.route, M11_GROUP_MOVE_ROUTE_INSERT);
    expect_int("route_insert_buzz", plan.shouldEmitAudibleBuzz, 1);
    expect_int("route_insert_x", plan.mapX, 7);
    expect_int("route_insert_y", plan.mapY, 8);

    expect_int("route_remove_ok",
        m11_plan_deferred_group_move_route_f0267(
            0, 0, 0, 1, 0, 100u, 3, 4, 0, 0, &plan), 1);
    expect_int("route_remove", plan.route, M11_GROUP_MOVE_ROUTE_REMOVE);
    expect_int("route_remove_reason", plan.removalReason,
        M11_GROUP_MOVE_REMOVAL_REASON_NOT_ALLOWED);
    expect_int("route_remove_buzz", plan.shouldEmitAudibleBuzz, 1);

    expect_int("route_retry_ok",
        m11_plan_deferred_group_move_route_f0267(
            0, 1, 1, 0, 0, 100u, 5, 6, 0, 0, &plan), 1);
    expect_int("route_retry", plan.route, M11_GROUP_MOVE_ROUTE_RETRY);
    expect_int("route_retry_scheduled", plan.shouldScheduleRetry, 1);
    expect_int("route_retry_tick", (int)plan.retryFireAtTick, 105);
    expect_int("route_retry_x", plan.mapX, 5);
    expect_int("route_retry_y", plan.mapY, 6);

    expect_int("route_chaos_ok",
        m11_plan_deferred_group_move_route_f0267(
            0, 1, 1, 1, 1, 100u, 5, 6, 9, 10, &plan), 1);
    expect_int("route_chaos", plan.route,
        M11_GROUP_MOVE_ROUTE_CHAOS_ADJACENT_INSERT);
    expect_int("route_chaos_buzz", plan.shouldEmitAudibleBuzz, 1);
    expect_int("route_chaos_x", plan.mapX, 9);
    expect_int("route_chaos_y", plan.mapY, 10);
}

static void test_ordinary_group_move_plan(void) {
    M11_OrdinaryGroupMovePlan plan;
    DM1_V1_OrdinaryGroupMoveApplyPlanPc34 apply;

    expect_int("ordinary_east_ok",
        m11_plan_ordinary_group_move_f0267(
            4, 5, M11_DIRECTION_EAST, 1, 0, 0, 100u, &plan), 1);
    expect_int("ordinary_east_route", plan.route, M11_GROUP_MOVE_ROUTE_INSERT);
    expect_int("ordinary_east_x", plan.destinationMapX, 5);
    expect_int("ordinary_east_y", plan.destinationMapY, 5);
    expect_int("ordinary_east_next_tick", (int)plan.retryFireAtTick, 101);

    expect_int("ordinary_blocked_ok",
        m11_plan_ordinary_group_move_f0267(
            4, 5, M11_DIRECTION_NORTH, 1, 1, 0, 200u, &plan), 1);
    expect_int("ordinary_blocked_route", plan.route, M11_GROUP_MOVE_ROUTE_RETRY);
    expect_int("ordinary_blocked_x", plan.destinationMapX, 4);
    expect_int("ordinary_blocked_y", plan.destinationMapY, 4);
    expect_int("ordinary_blocked_tick", (int)plan.retryFireAtTick, 201);

    expect_int("ordinary_wall_ok",
        m11_plan_ordinary_group_move_f0267(
            4, 5, M11_DIRECTION_WEST, 0, 0, 0, 300u, &plan), 1);
    expect_int("ordinary_wall_route", plan.route, M11_GROUP_MOVE_ROUTE_RETRY);
    expect_int("ordinary_wall_x", plan.destinationMapX, 3);
    expect_int("ordinary_wall_y", plan.destinationMapY, 5);

    expect_int("ordinary_projectile_kill_ok",
        m11_plan_ordinary_group_move_f0267(
            4, 5, M11_DIRECTION_SOUTH, 1, 0, 1, 400u, &plan), 1);
    expect_int("ordinary_projectile_kill_route", plan.route,
        M11_GROUP_MOVE_ROUTE_KILLED_BY_PROJECTILE);
    expect_int("ordinary_projectile_kill_x", plan.destinationMapX, 4);
    expect_int("ordinary_projectile_kill_y", plan.destinationMapY, 6);

    expect_int("ordinary_apply_insert_ok",
        DM1_V1_PlanOrdinaryGroupMoveApplyF0267Pc34Compat(
            &plan, 2, M11_DIRECTION_SOUTH, 0x44, 400u, &apply), 1);
    expect_int("ordinary_apply_kill_unlink", apply.shouldUnlinkSource, 1);
    expect_int("ordinary_apply_kill_remove", apply.shouldRemoveActiveGroup, 1);
    expect_int("ordinary_apply_kill_no_link", apply.shouldLinkDestination, 0);

    expect_int("ordinary_apply_move_plan_ok",
        m11_plan_ordinary_group_move_f0267(
            4, 5, M11_DIRECTION_EAST, 1, 0, 0, 500u, &plan), 1);
    expect_int("ordinary_apply_insert_plan_ok",
        DM1_V1_PlanOrdinaryGroupMoveApplyF0267Pc34Compat(
            &plan, 3, M11_DIRECTION_EAST, 0x12, 500u, &apply), 1);
    expect_int("ordinary_apply_insert_unlink", apply.shouldUnlinkSource, 1);
    expect_int("ordinary_apply_insert_link", apply.shouldLinkDestination, 1);
    expect_int("ordinary_apply_insert_requeue", apply.shouldRequeue, 1);
    expect_int("ordinary_apply_insert_dir", apply.groupDirection, M11_DIRECTION_EAST);
    expect_int("ordinary_apply_insert_map", apply.activeMapIndex, 3);
    expect_int("ordinary_apply_insert_x", apply.activeMapX, 5);
    expect_int("ordinary_apply_insert_y", apply.activeMapY, 5);
    expect_int("ordinary_apply_insert_cells", apply.activeCells, 0x12);
    expect_int("ordinary_apply_insert_tick", (int)apply.nextFireAtTick, 501);
    expect_int("ordinary_apply_insert_event_x", apply.nextEventMapX, 5);
    expect_int("ordinary_apply_insert_event_y", apply.nextEventMapY, 5);
}

static void test_pit_and_chaos_subplans(void) {
    M11_GroupPitFallSquarePlan pit;
    M11_LordChaosAdjacentRetryPlan chaos;

    expect_int("pit_open_ok",
        m11_plan_group_pit_fall_square_f0267(
            2, 2, 1, 0, &pit), 1);
    expect_int("pit_open_falls", pit.shouldFall, 1);

    expect_int("pit_closed_ok",
        m11_plan_group_pit_fall_square_f0267(
            2, 2, 0, 0, &pit), 1);
    expect_int("pit_closed_no_fall", pit.shouldFall, 0);

    expect_int("pit_imaginary_ok",
        m11_plan_group_pit_fall_square_f0267(
            2, 2, 1, 1, &pit), 1);
    expect_int("pit_imaginary_no_fall", pit.shouldFall, 0);

    expect_int("chaos_gate_miss_ok",
        m11_plan_lord_chaos_adjacent_retry_f0252(
            23, 1, 0, 10, 20, -1, -1, &chaos), 1);
    expect_int("chaos_gate_miss_no_attempt", chaos.shouldAttempt, 0);

    expect_int("chaos_candidate_ok",
        m11_plan_lord_chaos_adjacent_retry_f0252(
            23, 0, 1, 10, 20, -1, -1, &chaos), 1);
    expect_int("chaos_candidate_attempt", chaos.shouldAttempt, 1);
    expect_int("chaos_candidate_x", chaos.candidateMapX, 11);
    expect_int("chaos_candidate_y", chaos.candidateMapY, 20);
    expect_int("chaos_candidate_no_insert_without_accept",
        chaos.shouldInsertAdjacent, 0);

    expect_int("chaos_accept_ok",
        m11_plan_lord_chaos_adjacent_retry_f0252(
            23, 0, 1, 10, 20, 1, 0, &chaos), 1);
    expect_int("chaos_accept_insert", chaos.shouldInsertAdjacent, 1);

    expect_int("chaos_blocked_ok",
        m11_plan_lord_chaos_adjacent_retry_f0252(
            23, 0, 1, 10, 20, 1, 1, &chaos), 1);
    expect_int("chaos_blocked_no_insert", chaos.shouldInsertAdjacent, 0);
}

static void test_group_teleporter_destination_plan(void) {
    DM1_V1_GroupTeleporterDestinationPlanPc34 plan;

    expect_int("teleporter_open_scope_ok",
        DM1_V1_PlanGroupTeleporterDestinationF0267Pc34Compat(
            3, 3, 1, 1, M11_TELEPORTER_SCOPE_CREATURES, 1,
            2, 7, 8, 1, 4, 5, 5, &plan), 1);
    expect_int("teleporter_open_scope_teleports", plan.shouldTeleport, 1);
    expect_int("teleporter_open_scope_buzz", plan.shouldEmitAudibleBuzz, 1);
    expect_int("teleporter_open_scope_map", plan.targetMapIndex, 2);
    expect_int("teleporter_open_scope_x", plan.targetMapX, 7);
    expect_int("teleporter_open_scope_y", plan.targetMapY, 8);
    expect_int("teleporter_open_scope_chain", plan.shouldStopChain, 0);

    expect_int("teleporter_self_ok",
        DM1_V1_PlanGroupTeleporterDestinationF0267Pc34Compat(
            3, 3, 1, 1, M11_TELEPORTER_SCOPE_CREATURES, 0,
            1, 4, 5, 1, 4, 5, 5, &plan), 1);
    expect_int("teleporter_self_teleports", plan.shouldTeleport, 1);
    expect_int("teleporter_self_stop", plan.shouldStopChain, 1);
    expect_int("teleporter_self_no_buzz", plan.shouldEmitAudibleBuzz, 0);

    expect_int("teleporter_closed_ok",
        DM1_V1_PlanGroupTeleporterDestinationF0267Pc34Compat(
            3, 3, 0, 1, M11_TELEPORTER_SCOPE_CREATURES, 1,
            2, 7, 8, 1, 4, 5, 5, &plan), 1);
    expect_int("teleporter_closed_no_move", plan.shouldTeleport, 0);

    expect_int("teleporter_wrong_scope_ok",
        DM1_V1_PlanGroupTeleporterDestinationF0267Pc34Compat(
            3, 3, 1, 1, 0, 1, 2, 7, 8, 1, 4, 5, 5, &plan), 1);
    expect_int("teleporter_wrong_scope_no_move", plan.shouldTeleport, 0);

    expect_int("teleporter_invalid_target_ok",
        DM1_V1_PlanGroupTeleporterDestinationF0267Pc34Compat(
            3, 3, 1, 1, M11_TELEPORTER_SCOPE_CREATURES, 1,
            5, 7, 8, 1, 4, 5, 5, &plan), 1);
    expect_int("teleporter_invalid_target_no_move", plan.shouldTeleport, 0);
}

static void test_generated_group_placement_plan(void) {
    DM1_V1_GeneratedGroupPlacementPlanPc34 plan;

    expect_int("generated_party_map_ok",
        DM1_V1_PlanGeneratedGroupPlacementF0183F0180Pc34Compat(
            2, 2, 7, 8, 4, 11, 0x55, M11_DIRECTION_WEST,
            3, 16, 100u, &plan), 1);
    expect_int("generated_party_active", plan.shouldCreateActiveState, 1);
    expect_int("generated_party_state", plan.activeStateKind, M11_AI_STATE_WANDER);
    expect_int("generated_party_creature", plan.activeCreatureType, 11);
    expect_int("generated_party_map", plan.activeMapIndex, 2);
    expect_int("generated_party_x", plan.activeMapX, 7);
    expect_int("generated_party_y", plan.activeMapY, 8);
    expect_int("generated_party_cells", plan.activeCells, 0x55);
    expect_int("generated_party_dir", plan.activeDirection, M11_DIRECTION_WEST);
    expect_int("generated_party_target", plan.activeTargetChampionIndex, -1);
    expect_int("generated_party_seen_x", plan.activeLastSeenPartyMapX, -1);
    expect_int("generated_party_seen_y", plan.activeLastSeenPartyMapY, -1);
    expect_int("generated_party_seen_tick", plan.activeLastSeenPartyTick, -1);
    expect_int("generated_party_reserved", plan.activeReservedGroupIndex, 4);
    expect_int("generated_party_wander", plan.shouldScheduleWanderEvent, 1);
    expect_int("generated_party_wander_tick", (int)plan.wanderFireAtTick, 101);
    expect_int("generated_party_wander_group", plan.wanderGroupIndex, 4);
    expect_int("generated_party_wander_type", plan.wanderEventType, M11_AI_STATE_WANDER);

    expect_int("generated_other_map_ok",
        DM1_V1_PlanGeneratedGroupPlacementF0183F0180Pc34Compat(
            2, 1, 7, 8, 4, 11, 0x55, M11_DIRECTION_EAST,
            3, 16, 100u, &plan), 1);
    expect_int("generated_other_no_active", plan.shouldCreateActiveState, 0);
    expect_int("generated_other_wander", plan.shouldScheduleWanderEvent, 1);

    expect_int("generated_capacity_blocks",
        DM1_V1_PlanGeneratedGroupPlacementF0183F0180Pc34Compat(
            2, 2, 7, 8, 4, 11, 0x55, M11_DIRECTION_EAST,
            16, 16, 100u, &plan), 0);
}

int main(void) {
    test_allowed_group_keeps_move();
    test_fall_killed_group_drops_and_deletes_source();
    test_not_allowed_group_drops_without_placement_delete();
    test_deferred_route_plan();
    test_ordinary_group_move_plan();
    test_pit_and_chaos_subplans();
    test_group_teleporter_destination_plan();
    test_generated_group_placement_plan();
    test_source_evidence();

    if (g_failed) return 1;
    puts("DM1_V1_GROUP_MOVE_REMOVAL_PC34_COMPAT_OK");
    return 0;
}

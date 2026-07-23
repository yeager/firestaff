#include "csb_v1_f0806_entrance_loop_runtime_handoff_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(condition)                                                        \
    do {                                                                        \
        if (!(condition)) {                                                     \
            fprintf(stderr, "CHECK failed at %s:%d: %s\n", __FILE__, __LINE__, \
                    #condition);                                                \
            exit(1);                                                            \
        }                                                                       \
    } while (0)

static void check_contains(const char *text, const char *needle)
{
    CHECK(text != NULL);
    CHECK(strstr(text, needle) != NULL);
}

static CSB_V1_StartEndEntranceBoundaryReceipt_PC34 make_boundary(uint32_t stage)
{
    CSB_V1_StartEndEntranceBoundaryReceipt_PC34 receipt;
    memset(&receipt, 0, sizeof(receipt));
    receipt.valid = 1;
    receipt.accepted_stage_mask = stage;
    receipt.real_asset_matched = 1;
    receipt.host_view_consumed = 1;
    receipt.host_draw_consumed = 1;
    receipt.draw_consumes_receipt_only = 1;
    receipt.no_synthetic_payloads = 1;
    receipt.no_fallback_graphics = 1;
    receipt.route_wrappers_retired = 1;
    return receipt;
}

static CSB_V1_F0797_EntranceMicroDungeonReceipt_PC34 make_micro_dungeon(void)
{
    CSB_V1_F0797_EntranceMicroDungeonReceipt_PC34 receipt;
    memset(&receipt, 0, sizeof(receipt));
    receipt.valid = 1;
    receipt.entrance_map_index = CSB_V1_F0797_ENTRANCE_MAP_INDEX_PC34;
    receipt.map_width = CSB_V1_F0797_MICRO_DUNGEON_WIDTH_PC34;
    receipt.map_height = CSB_V1_F0797_MICRO_DUNGEON_HEIGHT_PC34;
    receipt.square_count = CSB_V1_F0797_MICRO_DUNGEON_SQUARE_COUNT_PC34;
    receipt.wall_square_count = CSB_V1_F0797_MICRO_DUNGEON_WALL_COUNT_PC34;
    receipt.corridor_square_count =
        CSB_V1_F0797_MICRO_DUNGEON_CORRIDOR_COUNT_PC34;
    receipt.corridor_square_mask =
        csb_v1_f0797_entrance_micro_dungeon_corridor_mask_pc34();
    receipt.draw_floor_and_ceiling_requested = 1;
    receipt.view_direction = CSB_V1_F0797_VIEW_DIRECTION_SOUTH_PC34;
    receipt.view_x = CSB_V1_F0797_VIEW_X_PC34;
    receipt.view_y = CSB_V1_F0797_VIEW_Y_PC34;
    receipt.entrance_boundary_consumed = 1;
    receipt.no_loaded_dungeon_substitute = 1;
    receipt.no_synthetic_viewport_pixels = 1;
    receipt.no_legacy_micro_dungeon_wrapper = 1;
    return receipt;
}

static CSB_V1_F0807_EntranceAnimationStepReceipt_PC34 make_door_animation(void)
{
    CSB_V1_F0807_EntranceAnimationStepReceipt_PC34 receipt;
    memset(&receipt, 0, sizeof(receipt));
    receipt.valid = 1;
    receipt.animation_step_bound = 1;
    receipt.target_screen_bound = 1;
    receipt.source_step_range_locked = 1;
    receipt.bitplanes_consumed = 1;
    receipt.runtime_coupling_consumed = 1;
    receipt.draw_consumes_receipt_only = 1;
    receipt.input_consumes_receipt_only = 1;
    receipt.no_legacy_animation_wrapper = 1;
    receipt.no_synthetic_visuals = 1;
    receipt.accepted_animation_step_index =
        CSB_V1_F0807_ENTRANCE_DOOR_STEP_LAST_PC34;
    return receipt;
}

static CSB_V1_F0806_EntranceLoopFacts_PC34 make_base_facts(void)
{
    CSB_V1_F0806_EntranceLoopFacts_PC34 facts;
    memset(&facts, 0, sizeof(facts));
    facts.valid = 1;
    facts.entrance_assets_bound = 1;
    facts.entrance_input_tables_bound = 1;
    facts.entrance_music_started = 1;
    facts.draw_entrance_each_outer_loop = 1;
    facts.pointer_shown_before_wait = 1;
    facts.input_discarded_before_wait = 1;
    facts.waiting_mode_set_before_queue = 1;
    facts.command_queue_processed_until_exit = 1;
    facts.final_new_game_mode = CSB_V1_F0806_MODE_LOAD_DUNGEON_PC34;
    facts.final_command = CSB_V1_F0806_COMMAND_ENTRANCE_ENTER_DUNGEON_PC34;
    facts.post_selection_switch_sound_played = 1;
    facts.post_selection_delay_ticks = 20;
    facts.temporary_entrance_memory_released = 1;
    facts.no_synthetic_input = 1;
    facts.no_synthetic_graphics_bytes = 1;
    facts.no_fallback_visuals = 1;
    facts.no_legacy_entrance_wrapper = 1;
    facts.draw_entrance =
        make_boundary(CSB_V1_STARTEND_F0439_DRAW_ENTRANCE_PC34);
    facts.micro_dungeon = make_micro_dungeon();
    facts.door_animation_step = make_door_animation();
    return facts;
}

static void make_last_opening_session(
    CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    CSB_V1_StartupRealPackageConsumptionReceipt_PC34 *package_receipt,
    CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 *opening_host)
{
    static unsigned char entrance_pixels[320 * 200];
    static unsigned char left_pixels[105 * 161];
    static unsigned char right_pixels[128 * 161];

    memset(session, 0, sizeof(*session));
    memset(package_receipt, 0, sizeof(*package_receipt));
    memset(opening_host, 0, sizeof(*opening_host));
    session->valid = session->real_asset_matched = 1;
    session->generation = 41u;
    session->source_tick = 913u;
    session->playback.stage = CSB_V1_STARTUP_PLAYBACK_STAGE_ENTRANCE_PC34;
    session->playback.no_fallback_routes = 1;
    session->playback.title_phase_mask = 0x0f;
    session->surfaces.valid = session->surfaces.real_asset_matched = 1;
    session->surfaces.opening_frame_ready = 1;
    session->surfaces.entrance_screen_ready = 1;
    session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_ENTRANCE_SCREEN_PC34].valid = 1;
    session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_ENTRANCE_SCREEN_PC34].pixels = entrance_pixels;
    session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_ENTRANCE_SCREEN_PC34].source_asset_id = 4;
    session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_ENTRANCE_SCREEN_PC34].width = 320;
    session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_ENTRANCE_SCREEN_PC34].height = 200;
    session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_ENTRANCE_SCREEN_PC34].transparent_color = -1;
    session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_LEFT_PC34].valid = 1;
    session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_LEFT_PC34].pixels = left_pixels;
    session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_LEFT_PC34].source_asset_id = 2;
    session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_LEFT_PC34].width = 105;
    session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_LEFT_PC34].height = 161;
    session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_LEFT_PC34].transparent_color = -1;
    session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_RIGHT_PC34].valid = 1;
    session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_RIGHT_PC34].pixels = right_pixels;
    session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_RIGHT_PC34].source_asset_id = 3;
    session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_RIGHT_PC34].width = 128;
    session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_RIGHT_PC34].height = 161;
    session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_RIGHT_PC34].transparent_color = -1;

    package_receipt->valid = package_receipt->real_package_matched = 1;
    package_receipt->c002_left_door_consumed = 1;
    package_receipt->c003_right_door_consumed = 1;
    package_receipt->c004_entrance_consumed = 1;
    package_receipt->title_to_entrance_same_session = 1;
    package_receipt->title_to_hud_same_session = 1;
    package_receipt->no_legacy_wrappers = package_receipt->no_fallback_routes = 1;
    package_receipt->source_tick = session->source_tick;
    package_receipt->session_generation = session->generation;
    package_receipt->real_asset_receipt_hash = 0x4f0806u;
    package_receipt->consumed_surface_hash = 0xc0040203u;

    opening_host->valid = opening_host->real_asset_matched = 1;
    opening_host->no_legacy_wrappers = opening_host->no_synthetic_surface = 1;
    opening_host->host_surface = CSB_V1_STARTUP_RUNTIME_HOST_SURFACE_DOOR_OPENING_PC34;
    opening_host->door_opening_decision = 1;
    opening_host->host_surface_hash = 0xc004f080u;
    opening_host->frame.session_generation = session->generation;
    opening_host->frame.source_tick = session->source_tick;
    opening_host->frame.frame_route_hash = 0xc004f083u;
    opening_host->frame.opening_step = CSB_V1_F0807_ENTRANCE_DOOR_STEP_LAST_PC34;
    opening_host->frame.entrance_surface = &session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_ENTRANCE_SCREEN_PC34];
    opening_host->frame.left_door_surface = &session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_LEFT_PC34];
    opening_host->frame.right_door_surface = &session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_RIGHT_PC34];
    opening_host->raster.valid = opening_host->raster.real_asset_matched = 1;
    opening_host->raster.entrance_composited = 1;
    opening_host->raster.door_composited = 1;
    opening_host->raster.source_surface_count = 2;
    opening_host->raster.pixel_hash = 0xc004f081u;
    opening_host->raster.route_hash = 0xc004f082u;
}

static void test_binds_f0806_handoff_to_last_real_opening_tick(void)
{
    CSB_V1_StartupRuntimeAssetSession_PC34 session;
    CSB_V1_StartupRealPackageConsumptionReceipt_PC34 package_receipt;
    CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 opening_host;
    CSB_V1_F0806_EntranceLoopFacts_PC34 facts = make_base_facts();
    CSB_V1_F0806_EntranceLoopReceipt_PC34 receipt;

    make_last_opening_session(&session, &package_receipt, &opening_host);
    CHECK(csb_v1_f0806_entrance_loop_runtime_handoff_from_session_pc34(
              &session, &package_receipt, &opening_host, &facts, &receipt) == 1);
    CHECK(receipt.opening_material_consumed == 1);
    CHECK(receipt.source_tick == session.source_tick);
    CHECK(receipt.session_generation == session.generation);
    CHECK(receipt.opening_host_surface_hash == opening_host.host_surface_hash);
    CHECK(receipt.real_asset_receipt_hash == package_receipt.real_asset_receipt_hash);

    opening_host.frame.source_tick++;
    CHECK(csb_v1_f0806_entrance_loop_runtime_handoff_from_session_pc34(
              &session, &package_receipt, &opening_host, &facts, &receipt) == 0);
    opening_host.frame.source_tick--;
    opening_host.frame.opening_step--;
    CHECK(csb_v1_f0806_entrance_loop_runtime_handoff_from_session_pc34(
              &session, &package_receipt, &opening_host, &facts, &receipt) == 0);
}

static void test_accepts_load_dungeon_after_real_door_route(void)
{
    CSB_V1_F0806_EntranceLoopFacts_PC34 facts = make_base_facts();
    CSB_V1_F0806_EntranceLoopReceipt_PC34 receipt;

    CHECK(F0806_F0806_ENTRANCE_int(&facts, &receipt) == 1);
    CHECK(receipt.valid == 1);
    CHECK(receipt.decision ==
          CSB_V1_F0806_ENTRANCE_LOOP_DECISION_LOAD_DUNGEON_PC34);
    CHECK(receipt.final_new_game_mode == CSB_V1_F0806_MODE_LOAD_DUNGEON_PC34);
    CHECK(receipt.entrance_draw_consumed == 1);
    CHECK(receipt.micro_dungeon_consumed == 1);
    CHECK(receipt.door_animation_consumed == 1);
    CHECK(receipt.credits_route_consumed == 0);
    CHECK(receipt.temporary_entrance_memory_released == 1);
    CHECK(receipt.no_synthetic_input == 1);
    CHECK(receipt.no_synthetic_graphics_bytes == 1);
    CHECK(receipt.no_fallback_visuals == 1);
}

static void test_accepts_credits_loop_before_final_dungeon_entry(void)
{
    CSB_V1_F0806_EntranceLoopFacts_PC34 facts = make_base_facts();
    CSB_V1_F0806_EntranceLoopReceipt_PC34 receipt;

    facts.credits_command_loops_before_exit = 1;
    facts.draw_credits = make_boundary(CSB_V1_STARTEND_F0442_DRAW_CREDITS_PC34);

    CHECK(F0806_F0806_ENTRANCE_int(&facts, &receipt) == 1);
    CHECK(receipt.credits_route_consumed == 1);
    CHECK(receipt.decision ==
          CSB_V1_F0806_ENTRANCE_LOOP_DECISION_LOAD_DUNGEON_PC34);
}

static void test_accepts_load_saved_without_opening_doors(void)
{
    CSB_V1_F0806_EntranceLoopFacts_PC34 facts = make_base_facts();
    CSB_V1_F0806_EntranceLoopReceipt_PC34 receipt;

    memset(&facts.door_animation_step, 0, sizeof(facts.door_animation_step));
    facts.final_new_game_mode = CSB_V1_F0806_MODE_LOAD_SAVED_GAME_PC34;
    facts.final_command = CSB_V1_F0806_MODE_LOAD_SAVED_GAME_PC34;

    CHECK(F0806_F0806_ENTRANCE_int(&facts, &receipt) == 1);
    CHECK(receipt.decision ==
          CSB_V1_F0806_ENTRANCE_LOOP_DECISION_LOAD_SAVED_GAME_PC34);
    CHECK(receipt.door_animation_consumed == 0);
}

static void test_accepts_bonus_dungeon_command_with_door_route(void)
{
    CSB_V1_F0806_EntranceLoopFacts_PC34 facts = make_base_facts();
    CSB_V1_F0806_EntranceLoopReceipt_PC34 receipt;

    facts.bonus_dungeon_selected = 1;
    facts.final_command = CSB_V1_F0806_COMMAND_ENTRANCE_ENTER_BONUS_DUNGEON_PC34;

    CHECK(F0806_F0806_ENTRANCE_int(&facts, &receipt) == 1);
    CHECK(receipt.decision ==
          CSB_V1_F0806_ENTRANCE_LOOP_DECISION_LOAD_BONUS_DUNGEON_PC34);
    CHECK(receipt.bonus_dungeon_selected == 1);
    CHECK(receipt.door_animation_consumed == 1);
}

static void test_rejects_unowned_or_synthetic_routes(void)
{
    CSB_V1_F0806_EntranceLoopFacts_PC34 facts = make_base_facts();
    CSB_V1_F0806_EntranceLoopReceipt_PC34 receipt;

    facts.final_new_game_mode = CSB_V1_F0806_COMMAND_ENTRANCE_DRAW_CREDITS_PC34;
    CHECK(F0806_F0806_ENTRANCE_int(&facts, &receipt) == 0);
    CHECK(receipt.no_synthetic_input == 1);
    CHECK(receipt.no_synthetic_graphics_bytes == 1);

    facts = make_base_facts();
    facts.draw_entrance.no_fallback_graphics = 0;
    CHECK(F0806_F0806_ENTRANCE_int(&facts, &receipt) == 0);

    facts = make_base_facts();
    facts.micro_dungeon.no_synthetic_viewport_pixels = 0;
    CHECK(F0806_F0806_ENTRANCE_int(&facts, &receipt) == 0);

    facts = make_base_facts();
    facts.door_animation_step.valid = 0;
    CHECK(F0806_F0806_ENTRANCE_int(&facts, &receipt) == 0);

    facts = make_base_facts();
    facts.credits_command_loops_before_exit = 1;
    CHECK(F0806_F0806_ENTRANCE_int(&facts, &receipt) == 0);

    facts = make_base_facts();
    facts.no_legacy_entrance_wrapper = 0;
    CHECK(F0806_F0806_ENTRANCE_int(&facts, &receipt) == 0);
}

static void test_evidence_string(void)
{
    const char *evidence = csb_v1_f0806_entrance_loop_source_evidence_pc34();

    check_contains(evidence, "ENTRANCE.C:721-826/850-903");
    check_contains(evidence, "C099_MODE_WAITING_ON_ENTRANCE");
    check_contains(evidence, "M567 credits");
    check_contains(evidence, "releases the temporary entrance allocation");
}

int main(void)
{
    test_accepts_load_dungeon_after_real_door_route();
    test_accepts_credits_loop_before_final_dungeon_entry();
    test_accepts_load_saved_without_opening_doors();
    test_accepts_bonus_dungeon_command_with_door_route();
    test_binds_f0806_handoff_to_last_real_opening_tick();
    test_rejects_unowned_or_synthetic_routes();
    test_evidence_string();
    return 0;
}

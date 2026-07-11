#ifndef FIRESTAFF_DM1_V1_STARTUP_SEQUENCE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_STARTUP_SEQUENCE_PC34_COMPAT_H

#include "dm1_v1_entrance_champion_select_pc34_compat.h"
#include "dm1_v1_champion_mirror_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum DM1_V1_StartupStage_PC34 {
    DM1_V1_STARTUP_STAGE_SWSH_LOGO_PC34 = 1,
    DM1_V1_STARTUP_STAGE_SWSH_RUN_START_PC34 = 2,
    DM1_V1_STARTUP_STAGE_TITLE_BEGIN_PC34 = 3,
    DM1_V1_STARTUP_STAGE_TITLE_LAST_FRAME_PC34 = 4,
    DM1_V1_STARTUP_STAGE_MENU_ELIGIBLE_PC34 = 5,
    DM1_V1_STARTUP_STAGE_ENTRANCE_WAIT_PC34 = 6
} DM1_V1_StartupStage_PC34;

typedef enum DM1_V1_StartupLaunchPath_PC34 {
    DM1_V1_STARTUP_LAUNCH_PATH_LAUNCHER_PC34 = 1,
    DM1_V1_STARTUP_LAUNCH_PATH_DIRECT_CLI_PC34 = 2,
    DM1_V1_STARTUP_LAUNCH_PATH_DIRECT_GAME_VIEW_PC34 = 3
} DM1_V1_StartupLaunchPath_PC34;

typedef enum DM1_V1_StartupHandoffAction_PC34 {
    DM1_V1_STARTUP_HANDOFF_ACTION_NONE_PC34 = 0,
    DM1_V1_STARTUP_HANDOFF_ACTION_ENTER_GAME_PC34 = 1,
    DM1_V1_STARTUP_HANDOFF_ACTION_RESUME_GAME_PC34 = 2,
    DM1_V1_STARTUP_HANDOFF_ACTION_QUIT_PC34 = 3,
    DM1_V1_STARTUP_HANDOFF_ACTION_SKIPPED_NONFATAL_PC34 = 4
} DM1_V1_StartupHandoffAction_PC34;

enum {
    DM1_V1_ENTRANCE_DOOR_OPEN_FRAME_INDEX_PC34 = 9
};

enum {
    DM1_V1_HOC_CAPTURE_CONSUMER_HOST_RENDER_PC34 = 0x1u,
    DM1_V1_HOC_CAPTURE_CONSUMER_M11_BOOT_PROBE_PC34 = 0x2u,
    DM1_V1_HOC_CAPTURE_CONSUMER_M12_STARTUP_PC34 = 0x4u,
    DM1_V1_HOC_CAPTURE_CONSUMER_ALL_PC34 =
        DM1_V1_HOC_CAPTURE_CONSUMER_HOST_RENDER_PC34 |
        DM1_V1_HOC_CAPTURE_CONSUMER_M11_BOOT_PROBE_PC34 |
        DM1_V1_HOC_CAPTURE_CONSUMER_M12_STARTUP_PC34
};

typedef struct DM1_V1_StartupFullGraphicsMediaReceipt_PC34 {
    int handled;
    int play_swsh;
    int play_title;
    int play_entrance;
    unsigned int swsh_vblank_ms;
    unsigned int swsh_initial_logo_hold_ms;
    unsigned int swsh_palette_wait_ms;
    unsigned int swsh_sound_wait_ms;
    unsigned int swsh_final_hold_ms;
    unsigned int title_presents_hold_ms;
    unsigned int title_zoom_frame_delay_ms;
    unsigned int title_zoom_step_count;
    unsigned int title_post_zoom_guard_ms;
    unsigned int title_c001_cadence_pad_ms;
    unsigned int title_source_animation_steps;
    unsigned int title_frame_bank_equivalent_steps;
    unsigned int title_menu_boundary_frame;
    int title_presents_palette;
    int title_zoom_palette;
    int title_menu_eligible;
    int title_consume_pending_input;
    int entrance_auto_enter_ms;
    unsigned int entrance_source_animation_steps;
    unsigned int entrance_door_step_count;
    unsigned int entrance_vblank_ms;
    unsigned int entrance_pre_open_delay_ms;
    const char* source_evidence;
} DM1_V1_StartupFullGraphicsMediaReceipt_PC34;

typedef struct DM1_V1_StartupTitleRuntimeSourceReceipt_PC34 {
    int handled;
    int graphics_c001_usable;
    int title_dat_fallback_usable;
    int selected_runtime_source;
    int require_graphics_c001_for_release_start;
    int fallback_is_visible_last_resort;
    const char* source_evidence;
} DM1_V1_StartupTitleRuntimeSourceReceipt_PC34;

typedef enum DM1_V1_StartupEntranceRenderKind_PC34 {
    DM1_V1_STARTUP_ENTRANCE_RENDER_NONE_PC34 = 0,
    DM1_V1_STARTUP_ENTRANCE_RENDER_DUNGEON_FRAME_PC34 = 1,
    DM1_V1_STARTUP_ENTRANCE_RENDER_FADE_BLACK_PC34 = 2,
    DM1_V1_STARTUP_ENTRANCE_RENDER_CLOSED_DOORS_PC34 = 3,
    DM1_V1_STARTUP_ENTRANCE_RENDER_OPENING_DOOR_PC34 = 4
} DM1_V1_StartupEntranceRenderKind_PC34;

typedef struct DM1_V1_StartupEntranceRenderAudioCommand_PC34 {
    int handled;
    int consume_media_receipt_only;
    int lower_level_renderer_helper_owned;
    int lower_level_audio_helper_owned;
    int source_timing_receipt_consumed;
    DM1_V1_StartupEntranceRenderKind_PC34 render_kind;
    int present_entrance_palette;
    int play_door_rattle_sound;
    unsigned int source_step;
    unsigned int door_animation_step;
    int door_geometry_ready;
    unsigned int door_left_box_x;
    unsigned int door_left_box_y;
    unsigned int door_left_box_w;
    unsigned int door_left_box_h;
    unsigned int door_right_box_x;
    unsigned int door_right_box_y;
    unsigned int door_right_box_w;
    unsigned int door_right_box_h;
    unsigned int door_left_source_x;
    unsigned int door_right_source_x;
    unsigned int delay_ms;
} DM1_V1_StartupEntranceRenderAudioCommand_PC34;

typedef struct DM1_V1_StartupHandoffPreludePlan_PC34 {
    int required;
    int source_order_valid;
    int play_swsh;
    int discard_presentation_after_swsh;
    const char* game_id;
    const char* failure_evidence;
    DM1_V1_StartupFullGraphicsMediaReceipt_PC34 media_receipt;
} DM1_V1_StartupHandoffPreludePlan_PC34;

typedef struct DM1_V1_StartupHandoffPostLaunchPlan_PC34 {
    int required;
    int play_title;
    int play_entrance;
    int entrance_auto_enter_ms;
    int title_menu_boundary_frame;
    int title_menu_eligible;
    int title_keep_surface;
    int title_consume_pending_input;
    DM1_V1_StartupStage_PC34 title_next_stage;
    const char* title_menu_reason;
    DM1_V1_StartupStage_PC34 entrance_wait_stage;
    DM1_V1_EntranceFullStartRenderReceiptPc34 entrance_full_start_receipt;
    DM1_V1_StartupFullGraphicsMediaReceipt_PC34 media_receipt;
    const char* source_id;
} DM1_V1_StartupHandoffPostLaunchPlan_PC34;

typedef struct DM1_V1_StartupHandoffOutcome_PC34 {
    int title_played;
    int entrance_command;
    DM1_V1_StartupHandoffAction_PC34 action;
    const char* status;
} DM1_V1_StartupHandoffOutcome_PC34;

typedef struct DM1_V1_StartupHostApplyResult_PC34 {
    int handled;
    int quit_requested;
    int resume_requested;
    int resume_loaded;
    int resume_used_backup;
    char resume_path[512];
} DM1_V1_StartupHostApplyResult_PC34;

typedef struct DM1_V1_StartupFullGraphicsRuntimeHandoffReceipt_PC34 {
    int handled;
    int full_graphics_required;
    int swsh_consumed;
    int title_consumed;
    int entrance_consumed;
    int full_graphics_consumed;
    int hoc_runtime_ready;
    int resumed_runtime_ready;
    int champion_mirror_startup_handoff_ready;
    int champion_mirror_startup_input_ready;
    int champion_mirror_startup_panel_clear;
    int champion_mirror_startup_blocks_enter;
    int champion_mirror_startup_overlay_commands_ready;
    int champion_mirror_startup_overlay_command_count;
    DM1_V1_EntranceRenderOverlayCommandPc34
        champion_mirror_startup_overlay_commands[
            DM1_V1_ENTRANCE_OVERLAY_COMMAND_MAX_PC34];
    int hoc_first_frame_ready;
    int runtime_first_frame_ready;
    int draw_opened_runtime;
    int suppress_draw_opened;
    int return_to_launcher;
    int entrance_command;
    DM1_V1_StartupHandoffAction_PC34 action;
    DM1_V1_EntranceMenuRouteReceiptPc34 champion_mirror_startup_route;
    const char* status;
} DM1_V1_StartupFullGraphicsRuntimeHandoffReceipt_PC34;

enum {
    DM1_V1_STARTUP_SAVE_CORPUS_PART_COUNT_PC34 = 5,
    DM1_V1_STARTUP_SAVE_CORPUS_PORTRAIT_COUNT_PC34 = 4
};

typedef struct DM1_V1_StartupSaveResumeCaptureFacts_PC34 {
    const char* source_id;
    const DM1_V1_StartupHandoffOutcome_PC34* outcome;
    const DM1_V1_StartupHostApplyResult_PC34* host_apply;
    const DM1_V1_StartupFullGraphicsRuntimeHandoffReceipt_PC34* runtime_handoff;
    int observed_save_header;
    int observed_save_part_count;
    int observed_champion_portrait_count;
    int observed_dungeon_payload;
    int observed_required_graphics_hash_match;
    int observed_required_dungeon_hash_match;
    int observed_user_save_corpus_scan;
    int observed_user_save_corpus_files;
    int observed_user_save_corpus_classified;
    int observed_user_save_corpus_pc34;
    int observed_user_save_corpus_part_envelope;
    int observed_user_save_corpus_roundtrip_verified;
    int observed_user_save_corpus_roundtrip_failed;
    unsigned int observed_user_save_corpus_roundtrip_hash;
    int observed_user_save_corpus_rejected;
    int observed_user_save_corpus_truncated;
    const char* observed_user_save_corpus_first_pc34_path;
} DM1_V1_StartupSaveResumeCaptureFacts_PC34;

typedef struct DM1_V1_StartupSaveResumeCaptureReceipt_PC34 {
    int handled;
    int ready;
    int consumed_resume_outcome;
    int consumed_host_apply_result;
    int consumed_runtime_handoff_receipt;
    int resume_command_maps_load_saved_game;
    int resume_path_resolved;
    int resume_load_consumed;
    int resume_runtime_ready;
    int suppress_hoc_first_frame;
    int save_header_present;
    int save_part_corpus_present;
    int champion_portrait_corpus_present;
    int dungeon_payload_present;
    int required_asset_hashes_present;
    int save_corpus_capture_ready;
    int original_save_roundtrip_route_ready;
    int expected_save_part_count;
    int observed_save_part_count;
    int expected_champion_portrait_count;
    int observed_champion_portrait_count;
    int user_save_corpus_scan_consumed;
    int user_save_corpus_files;
    int user_save_corpus_classified;
    int user_save_corpus_pc34;
    int user_save_corpus_part_envelope;
    int user_save_corpus_roundtrip_verified;
    int user_save_corpus_roundtrip_failed;
    unsigned int user_save_corpus_roundtrip_hash;
    int user_save_corpus_rejected;
    int user_save_corpus_truncated;
    char user_save_corpus_first_pc34_path[512];
    char resume_path[512];
    const char* source_evidence;
} DM1_V1_StartupSaveResumeCaptureReceipt_PC34;

typedef enum DM1_V1_StartupHoCRenderCommandKind_PC34 {
    DM1_V1_STARTUP_HOC_RENDER_COMMAND_NONE_PC34 = 0,
    DM1_V1_STARTUP_HOC_RENDER_COMMAND_ENTRANCE_OPEN_FRAME_PC34 = 1,
    DM1_V1_STARTUP_HOC_RENDER_COMMAND_HALL_MIRRORS_PC34 = 2,
    DM1_V1_STARTUP_HOC_RENDER_COMMAND_CLEAR_CHAMPION_PANEL_PC34 = 3
} DM1_V1_StartupHoCRenderCommandKind_PC34;

typedef struct DM1_V1_StartupHoCRenderCommand_PC34 {
    int valid;
    DM1_V1_StartupHoCRenderCommandKind_PC34 kind;
    int map_index;
    int door_frame_index;
    int overlay_kind;
    int overlay_command_index;
    int clear_stale_panel_first;
    int suppress_host_fallback_visuals;
    int block_enter_until_champion_selected;
    const char* source_evidence;
} DM1_V1_StartupHoCRenderCommand_PC34;

typedef struct DM1_V1_StartupHoCFirstFrameReceipt_PC34 {
    int handled;
    int full_graphics_required;
    int title_surface_released;
    int entrance_wait_consumed;
    int full_start_render_ready;
    int entrance_map_ready;
    int entrance_music_requested;
    int entrance_door_open_frame_ready;
    int hoc_menu_route_ready;
    int champion_select_ui_ready;
    int render_hall_mirrors;
    int render_overlay_commands_ready;
    int render_overlay_command_count;
    DM1_V1_EntranceRenderOverlayCommandPc34
        render_overlay_commands[DM1_V1_ENTRANCE_OVERLAY_COMMAND_MAX_PC34];
    int clear_stale_champion_panel;
    int block_enter_until_champion_selected;
    int suppress_host_fallback_visuals;
    int runtime_first_frame_ready;
    int hoc_render_command_count;
    DM1_V1_StartupHoCRenderCommand_PC34 hoc_render_commands[4];
    DM1_V1_EntranceFullStartRenderReceiptPc34 entrance_full_start_receipt;
    DM1_V1_EntranceMenuRouteReceiptPc34 champion_select_route;
    const char* source_evidence;
} DM1_V1_StartupHoCFirstFrameReceipt_PC34;

typedef struct DM1_V1_StartupHoCHostRenderPlan_PC34 {
    int handled;
    int ready;
    int consume_dm1_receipt_only;
    int draw_opened_entrance_frame;
    int entrance_map_index;
    int entrance_door_frame_index;
    int clear_champion_panel;
    int render_hall_mirror_overlay;
    int hall_mirror_overlay_kind;
    int suppress_host_fallback_visuals;
    int block_enter_until_champion_selected;
    int command_count;
    const char* source_evidence;
} DM1_V1_StartupHoCHostRenderPlan_PC34;

typedef struct DM1_V1_StartupHoCPackagedFullGraphicsProof_PC34 {
    int handled;
    int ready;
    int consume_host_render_plan_only;
    int capture_required;
    int packaged_full_graphics_proof_ready;
    int expected_map_index;
    int expected_map_width;
    int expected_map_height;
    int expected_entrance_door_frame_index;
    int expected_hall_overlay_kind;
    int require_opened_entrance_frame;
    int require_clear_champion_panel;
    int require_hall_mirror_overlay;
    int require_no_title_surface;
    int require_no_closed_door_frame;
    int require_no_host_fallback_visuals;
    int require_lower_level_renderer_helper;
    int require_lower_level_audio_helper;
    int block_enter_until_champion_selected;
    int command_count;
    const char* capture_phase;
    const char* source_evidence;
} DM1_V1_StartupHoCPackagedFullGraphicsProof_PC34;

typedef struct DM1_V1_StartupHoCProductionFullStartHook_PC34 {
    int handled;
    int ready;
    int consume_dm1_startup_receipts_only;
    int run_before_hoc_input;
    int draw_opened_entrance_frame;
    int clear_champion_panel;
    int render_hall_mirror_overlay;
    int suppress_host_fallback_visuals;
    int lower_level_renderer_helper_owned;
    int lower_level_audio_helper_owned;
    int capture_after_first_frame_render;
    int publish_packaged_full_graphics_proof;
    int expected_map_index;
    int expected_map_width;
    int expected_map_height;
    int expected_entrance_door_frame_index;
    int expected_hall_overlay_kind;
    int block_enter_until_champion_selected;
    const char* capture_phase;
    const char* source_evidence;
} DM1_V1_StartupHoCProductionFullStartHook_PC34;

typedef struct DM1_V1_StartupHoCFullStartProductionReceipt_PC34 {
    int handled;
    int ready;
    int consumed_post_launch_plan;
    int consumed_handoff_outcome;
    int consumed_title_menu_plan;
    int consumed_entrance_full_start_plan;
    int first_frame_ready;
    int host_render_plan_ready;
    int packaged_full_graphics_proof_ready;
    int production_hook_ready;
    int title_surface_released;
    int entrance_wait_consumed;
    DM1_V1_StartupHoCFirstFrameReceipt_PC34 first_frame;
    DM1_V1_StartupHoCHostRenderPlan_PC34 host_render_plan;
    DM1_V1_StartupHoCPackagedFullGraphicsProof_PC34 packaged_proof;
    DM1_V1_StartupHoCProductionFullStartHook_PC34 production_hook;
    const char* source_evidence;
} DM1_V1_StartupHoCFullStartProductionReceipt_PC34;

typedef struct DM1_V1_StartupHoCFullGraphicsCaptureArtifact_PC34 {
    int handled;
    int ready;
    int consume_full_start_production_receipt_only;
    int capture_manifest_ready;
    int capture_after_first_frame_render;
    int publish_packaged_full_graphics_proof;
    int title_surface_forbidden;
    int closed_door_frame_forbidden;
    int host_fallback_visuals_forbidden;
    int lower_level_renderer_helper_owned;
    int lower_level_audio_helper_owned;
    int opened_entrance_frame_required;
    int hall_mirror_overlay_required;
    int clear_champion_panel_required;
    int block_enter_until_champion_selected;
    int expected_map_index;
    int expected_map_width;
    int expected_map_height;
    int expected_entrance_door_frame_index;
    int expected_hall_overlay_kind;
    int expected_hoc_render_command_count;
    const char* capture_phase;
    const char* source_evidence;
} DM1_V1_StartupHoCFullGraphicsCaptureArtifact_PC34;

typedef struct DM1_V1_StartupHoCFullGraphicsCaptureFacts_PC34 {
    int captured_after_first_frame_render;
    int captured_from_real_assets;
    int captured_from_mac_window;
    int captured_from_release_app;
    int observed_c026_portrait_asset;
    int observed_c346_mirror_backing_asset;
    int observed_required_graphics_hash_match;
    int observed_required_dungeon_hash_match;
    int observed_host_window_present;
    int observed_presented_rgba_capture;
    int presented_capture_width;
    int presented_capture_height;
    int presented_capture_byte_count;
    unsigned int presented_capture_hash;
    unsigned int presented_capture_consumer_mask;
    unsigned int presented_capture_chain_hash;
    int captured_map_index;
    int captured_map_width;
    int captured_map_height;
    int captured_entrance_door_frame_index;
    int captured_hall_overlay_kind;
    int captured_hoc_render_command_count;
    int saw_title_surface;
    int saw_closed_door_frame;
    int saw_host_fallback_visuals;
    int saw_opened_entrance_frame;
    int saw_hall_mirror_overlay;
    int cleared_champion_panel;
    int blocked_enter_until_champion_selected;
} DM1_V1_StartupHoCFullGraphicsCaptureFacts_PC34;

typedef struct DM1_V1_StartupHoCHostCaptureObservation_PC34 {
    int host_window_present;
    int presented_capture_ready;
    int started_from_launcher;
    int intro_not_bypassed;
    int captured_from_real_assets;
    int observed_c026_portrait_asset;
    int observed_c346_mirror_backing_asset;
    int observed_required_graphics_hash_match;
    int observed_required_dungeon_hash_match;
} DM1_V1_StartupHoCHostCaptureObservation_PC34;

typedef struct DM1_V1_StartupHoCFullGraphicsCaptureProofReceipt_PC34 {
    int handled;
    int ready;
    int proof_passed;
    int consumed_capture_artifact;
    int consumed_capture_facts;
    int real_asset_capture;
    int mac_window_capture;
    int release_app_capture;
    int release_app_identity_ready;
    unsigned int release_app_identity_hash;
    int host_capture_route_matches;
    int hoc_asset_capture;
    int required_asset_capture;
    int host_window_capture;
    int presented_capture;
    int presented_capture_geometry_matches;
    int presented_capture_pixels_present;
    int presented_capture_byte_count;
    unsigned int presented_capture_hash;
    int presented_capture_chain_ready;
    unsigned int presented_capture_consumer_mask;
    unsigned int presented_capture_chain_hash;
    int redmcsb_c026_asset_present;
    int redmcsb_c346_asset_present;
    int geometry_matches;
    int entrance_frame_matches;
    int hall_overlay_matches;
    int command_count_matches;
    int stale_title_absent;
    int stale_door_absent;
    int host_fallback_absent;
    int required_layers_present;
    int input_block_matches;
    const char* capture_phase;
    const char* source_evidence;
} DM1_V1_StartupHoCFullGraphicsCaptureProofReceipt_PC34;

typedef struct DM1_V1_StartupHoCFullGraphicsRuntimeApplyReceipt_PC34 {
    int handled;
    int ready;
    int consumed_capture_artifact;
    int consumed_capture_proof;
    int require_proof_passed;
    int real_asset_capture;
    int mac_window_capture;
    int release_app_capture;
    int release_app_identity_ready;
    unsigned int release_app_identity_hash;
    int host_capture_route_matches;
    int hoc_asset_capture;
    int host_window_capture;
    int redmcsb_c026_asset_present;
    int redmcsb_c346_asset_present;
    int apply_before_hoc_input;
    int apply_opened_entrance_frame;
    int apply_clear_champion_panel;
    int apply_hall_mirror_overlay;
    int suppress_title_surface;
    int suppress_closed_door_frame;
    int suppress_host_fallback_visuals;
    int lower_level_renderer_helper_owned;
    int lower_level_audio_helper_owned;
    int publish_packaged_full_graphics_proof;
    int block_enter_until_champion_selected;
    int map_index;
    int map_width;
    int map_height;
    int entrance_door_frame_index;
    int hall_overlay_kind;
    int render_command_count;
    const char* capture_phase;
    const char* source_evidence;
} DM1_V1_StartupHoCFullGraphicsRuntimeApplyReceipt_PC34;

typedef struct DM1_V1_StartupHoCFullGraphicsThingSuppressionFacts_PC34 {
    int observed_hall_mirror_overlay;
    int observed_false_floor_item_payload_count;
    int observed_projectile_payload_count;
    int observed_spell_effect_payload_count;
    int observed_mirror_payload_as_thing_count;
    int observed_host_fallback_visuals;
    int observed_title_surface;
    int observed_closed_door_frame;
    int observed_enter_blocked_until_champion_selected;
    int observed_hoc_render_command_count;
} DM1_V1_StartupHoCFullGraphicsThingSuppressionFacts_PC34;

typedef struct DM1_V1_StartupHoCFullGraphicsThingSuppressionReceipt_PC34 {
    int handled;
    int ready;
    int proof_passed;
    int consumed_runtime_apply_receipt;
    int consumed_suppression_facts;
    int champion_mirror_overlay_present;
    int false_item_payloads_absent;
    int projectile_payloads_absent;
    int spell_effect_payloads_absent;
    int mirror_payload_thing_absent;
    int fallback_visuals_absent;
    int stale_title_absent;
    int stale_door_absent;
    int command_count_matches;
    int enter_block_matches;
    int walk_capture_safe;
    const char* capture_phase;
    const char* source_evidence;
} DM1_V1_StartupHoCFullGraphicsThingSuppressionReceipt_PC34;

typedef struct DM1_V1_StartupHoCFullGraphicsProductionConsumerReceipt_PC34 {
    int handled;
    int ready;
    int consumed_runtime_apply_receipt;
    int consumed_thing_suppression_receipt;
    int consume_dm1_receipts_only;
    int real_asset_capture;
    int mac_window_capture;
    int release_app_capture;
    int release_app_identity_ready;
    unsigned int release_app_identity_hash;
    int host_capture_route_matches;
    int hoc_asset_capture;
    int host_window_capture;
    int execute_before_hoc_input;
    int draw_opened_entrance_frame;
    int clear_champion_panel;
    int render_hall_mirror_overlay;
    int suppress_title_surface;
    int suppress_closed_door_frame;
    int suppress_host_fallback_visuals;
    int lower_level_renderer_helper_owned;
    int lower_level_audio_helper_owned;
    int suppress_false_item_payloads;
    int suppress_projectile_payloads;
    int suppress_spell_effect_payloads;
    int suppress_mirror_payload_things;
    int publish_packaged_full_graphics_proof;
    int redmcsb_c026_portrait_overlay_ready;
    int redmcsb_c346_mirror_backing_ready;
    int redmcsb_f0115_thing_layer_suppression_ready;
    int block_enter_until_champion_selected;
    int map_index;
    int map_width;
    int map_height;
    int entrance_door_frame_index;
    int hall_overlay_kind;
    int render_command_count;
    int walk_capture_safe;
    const char* capture_phase;
    const char* source_evidence;
} DM1_V1_StartupHoCFullGraphicsProductionConsumerReceipt_PC34;

typedef struct DM1_V1_StartupHoCFullGraphicsHostProbeFacts_PC34 {
    const char* source_id;
    int dungeon_loaded;
    int map_count;
    int entrance_command;
    int title_played;
    int consumed_launch_path_receipt;
    int launch_path_started_from_launcher;
    int launch_path_intro_not_bypassed;
    int captured_after_first_frame_render;
    int captured_from_real_assets;
    int captured_from_mac_window;
    int captured_from_release_app;
    int observed_c026_portrait_asset;
    int observed_c346_mirror_backing_asset;
    int observed_required_graphics_hash_match;
    int observed_required_dungeon_hash_match;
    int observed_host_window_present;
    int observed_presented_rgba_capture;
    int presented_capture_width;
    int presented_capture_height;
    int presented_capture_byte_count;
    unsigned int presented_capture_hash;
    unsigned int presented_capture_consumer_mask;
    unsigned int presented_capture_chain_hash;
    int consumed_hoc_host_render_receipt;
    int consumed_m11_boot_probe_consumer;
    int consumed_m12_startup_capture_consumer;
    int observed_false_floor_item_payload_count;
    int observed_projectile_payload_count;
    int observed_spell_effect_payload_count;
    int observed_mirror_payload_as_thing_count;
    int observed_host_fallback_visuals;
    int observed_title_surface;
    int observed_closed_door_frame;
} DM1_V1_StartupHoCFullGraphicsHostProbeFacts_PC34;

typedef struct DM1_V1_StartupHoCReleaseAppCaptureOwnershipReceipt_PC34 {
    int handled;
    int ready;
    int consumed_host_probe_facts;
    int consumed_hoc_host_render_receipt;
    int consumed_m11_boot_probe_consumer;
    int consumed_m12_startup_capture_consumer;
    int consumed_all_named_host_consumers;
    unsigned int named_consumer_mask;
    unsigned int named_consumer_hash;
    int host_capture_route_packaged;
    unsigned int host_capture_route_mask;
    unsigned int host_capture_route_hash;
    int consumed_launch_path_receipt;
    int consumed_runtime_apply_receipt;
    int consumed_production_consumer_receipt;
    int consume_dm1_receipts_only;
    int publish_packaged_full_graphics_proof;
    int launch_path_started_from_launcher;
    int launch_path_intro_not_bypassed;
    int real_asset_capture;
    int mac_window_capture;
    int release_app_capture;
    int release_app_identity_ready;
    unsigned int release_app_identity_hash;
    int host_capture_route_matches;
    int hoc_asset_capture;
    int required_asset_capture;
    int host_window_capture;
    int presented_capture;
    int presented_capture_width;
    int presented_capture_height;
    int presented_capture_geometry_matches;
    int presented_capture_pixels_present;
    int presented_capture_byte_count;
    unsigned int presented_capture_hash;
    int presented_capture_chain_ready;
    unsigned int presented_capture_consumer_mask;
    unsigned int presented_capture_chain_hash;
    int presented_capture_route_packaged;
    int draw_opened_entrance_frame;
    int render_hall_mirror_overlay;
    int suppress_host_fallback_visuals;
    int consumed_required_graphics_asset;
    int consumed_required_dungeon_asset;
    int consumed_owned_host_draw_receipt;
    int host_draw_uses_owned_receipt;
    int host_draw_consumes_backing_asset;
    int host_draw_rejects_backing_fallback;
    int lower_level_renderer_helper_owned;
    int lower_level_audio_helper_owned;
    int block_enter_until_champion_selected;
    int map_index;
    int map_width;
    int map_height;
    int entrance_door_frame_index;
    int hall_overlay_kind;
    int render_command_count;
    const char* capture_phase;
    const char* source_evidence;
} DM1_V1_StartupHoCReleaseAppCaptureOwnershipReceipt_PC34;

typedef struct DM1_V1_StartupHoCPresentedCapturePublishFacts_PC34 {
    const char* source_id;
    int presented_capture_ready;
    int host_window_present;
    int captured_from_mac_window;
    int captured_from_release_app;
    int width;
    int height;
    int byte_count;
    unsigned int framebuffer_hash;
    unsigned int required_consumer_mask;
} DM1_V1_StartupHoCPresentedCapturePublishFacts_PC34;

typedef struct DM1_V1_StartupHoCPresentedCapturePublishReceipt_PC34 {
    int handled;
    int ready;
    int presented_capture_ready;
    int host_window_present;
    int captured_from_mac_window;
    int captured_from_release_app;
    int geometry_matches;
    int pixels_present;
    int width;
    int height;
    int byte_count;
    unsigned int framebuffer_hash;
    unsigned int consumer_mask;
    unsigned int chain_hash;
    const char* source_evidence;
} DM1_V1_StartupHoCPresentedCapturePublishReceipt_PC34;

typedef struct DM1_V1_StartupHoCPresentedCaptureHostExportReceipt_PC34 {
    int handled;
    int ready;
    int consumed_publish_receipt;
    int presented_capture_ready;
    int host_window_present;
    int captured_from_mac_window;
    int captured_from_release_app;
    int width;
    int height;
    int byte_count;
    unsigned int framebuffer_hash;
    unsigned int consumer_mask;
    unsigned int chain_hash;
    const char* source_evidence;
} DM1_V1_StartupHoCPresentedCaptureHostExportReceipt_PC34;

typedef struct DM1_V1_StartupHoCPresentedCaptureM12ImportFacts_PC34 {
    const char* source_id;
    int presented_capture_ready;
    int host_window_present;
    int captured_from_mac_window;
    int captured_from_release_app;
    int width;
    int height;
    int byte_count;
    unsigned int framebuffer_hash;
    unsigned int consumer_mask;
    unsigned int chain_hash;
} DM1_V1_StartupHoCPresentedCaptureM12ImportFacts_PC34;

typedef struct DM1_V1_StartupHoCPresentedCaptureM12ImportReceipt_PC34 {
    int handled;
    int ready;
    int consumed_publish_receipt;
    int consumed_host_export_receipt;
    int consumed_m12_presented_capture;
    int presented_capture_ready;
    int host_window_present;
    int captured_from_mac_window;
    int captured_from_release_app;
    int geometry_matches;
    int pixels_present;
    int width;
    int height;
    int byte_count;
    unsigned int framebuffer_hash;
    unsigned int consumer_mask;
    unsigned int expected_chain_hash;
    unsigned int observed_chain_hash;
    int chain_hash_matches;
    const char* source_evidence;
} DM1_V1_StartupHoCPresentedCaptureM12ImportReceipt_PC34;

typedef struct DM1_V1_StartupHoCSaveCaptureHostReadinessReceipt_PC34 {
    int handled;
    int ready;
    int consumed_runtime_handoff_receipt;
    int consumed_release_app_capture_ownership;
    int consume_dm1_receipts_only;
    int enter_route_ready;
    int resume_route_ready;
    int save_capture_ready;
    int original_save_capture_ready;
    int real_asset_capture;
    int mac_window_capture;
    int release_app_capture;
    int release_app_identity_ready;
    unsigned int release_app_identity_hash;
    int host_capture_route_matches;
    int hoc_asset_capture;
    int host_window_capture;
    int draw_opened_entrance_frame;
    int render_hall_mirror_overlay;
    int suppress_host_fallback_visuals;
    int host_draw_uses_owned_receipt;
    int block_enter_until_champion_selected;
    int resumed_runtime_ready;
    int resume_loaded;
    int resume_used_backup;
    int resume_path_present;
    int map_index;
    int map_width;
    int map_height;
    int entrance_door_frame_index;
    int hall_overlay_kind;
    int render_command_count;
    char resume_path[512];
    const char* capture_phase;
    const char* source_evidence;
} DM1_V1_StartupHoCSaveCaptureHostReadinessReceipt_PC34;

typedef struct DM1_V1_CompleteSupportReceipt_PC34 {
    int handled;
    int ready;
    int consumed_hoc_runtime_handoff_receipt;
    int consumed_release_app_capture_ownership;
    int consumed_hoc_save_capture_host_readiness;
    int consumed_original_save_capture_receipt;
    int complete_source_visible_startup;
    int complete_entrance_to_hoc_transition;
    int complete_hoc_render_route;
    int complete_host_app_capture_route;
    int complete_save_corpus_route;
    int complete_original_save_roundtrip_route;
    int consume_dm1_receipts_only;
    int no_host_fallback_visuals;
    int redmcsb_entrance_micro_dungeon_ready;
    int redmcsb_hoc_mirror_overlay_ready;
    int redmcsb_hoc_thing_layer_suppression_ready;
    int redmcsb_save_part_corpus_ready;
    int user_save_corpus_scan_consumed;
    int user_save_corpus_pc34_ready;
    int user_save_corpus_part_envelope_ready;
    int user_save_corpus_roundtrip_ready;
    int user_save_corpus_roundtrip_verified;
    int user_save_corpus_roundtrip_failed;
    unsigned int user_save_corpus_roundtrip_hash;
    int user_save_corpus_rejected;
    int user_save_corpus_truncated;
    char user_save_corpus_first_pc34_path[512];
    int host_capture_route_packaged;
    int presented_capture_chain_ready;
    int host_draw_uses_owned_receipt;
    int block_enter_until_champion_selected;
    int map_index;
    int map_width;
    int map_height;
    int render_command_count;
    const char* source_evidence;
} DM1_V1_CompleteSupportReceipt_PC34;

typedef struct DM1_V1_StartupHoCBootFullGraphicsReceipt_PC34 {
    int handled;
    int ready;
    int consumed_host_probe_facts;
    int consumed_hoc_runtime_handoff_receipt;
    int consumed_hoc_save_capture_host_readiness;
    int consumed_original_save_capture_receipt;
    DM1_V1_StartupHoCFullGraphicsRuntimeApplyReceipt_PC34 runtime_apply;
    DM1_V1_StartupHoCFullGraphicsProductionConsumerReceipt_PC34 production_consumer;
    DM1_V1_StartupHoCReleaseAppCaptureOwnershipReceipt_PC34 ownership;
    DM1_V1_CompleteSupportReceipt_PC34 complete_support;
    const char* source_evidence;
} DM1_V1_StartupHoCBootFullGraphicsReceipt_PC34;

typedef struct DM1_V1_StartupHoCBootCompleteSupportFacts_PC34 {
    const char* source_id;
    const char* resume_path;
    int dungeon_loaded;
    int assets_available;
} DM1_V1_StartupHoCBootCompleteSupportFacts_PC34;

typedef struct DM1_V1_StartupHoCBootProbeSummary_PC34 {
    int handled;
    int full_graphics_ready;
    int host_render_plan_ready;
    int capture_proof_passed;
    int runtime_apply_ready;
    int production_consumer_ready;
    int no_host_fallback_visuals;
    int real_asset_capture;
    int mac_window_capture;
    int release_app_capture;
    int release_app_identity_ready;
    unsigned int release_app_identity_hash;
    int host_capture_route_matches;
    int release_capture_ownership_ready;
    int host_render_consumer_ready;
    int m11_boot_probe_consumer_ready;
    int launch_path_ready;
    int required_asset_capture;
    int receipt_only_consumer_ready;
    int lower_level_helpers_ready;
    int host_draw_uses_owned_receipt;
    int host_draw_consumes_backing_asset;
    int host_draw_rejects_backing_fallback;
    int hoc_asset_capture;
    int host_window_capture;
    int presented_capture;
    int presented_capture_width;
    int presented_capture_height;
    int presented_capture_geometry;
    int presented_capture_pixels;
    int presented_capture_bytes;
    unsigned int presented_capture_hash;
    int presented_capture_chain_ready;
    unsigned int presented_capture_consumer_mask;
    unsigned int presented_capture_chain_hash;
    int host_capture_route_packaged;
    unsigned int host_capture_route_mask;
    unsigned int host_capture_route_hash;
    int presented_capture_route_packaged;
    int opened_entrance_frame;
    int hall_mirror_overlay;
    int blocked_enter_until_champion;
    int map_width;
    int map_height;
    int render_command_count;
    int consumed_hoc_save_capture_host_readiness;
    int hoc_save_capture_ready;
    int hoc_original_save_capture_ready;
    int complete_support_ready;
    int complete_source_visible_startup;
    int complete_entrance_to_hoc;
    int complete_hoc_render_route;
    int complete_host_app_capture_route;
    int complete_save_corpus_route;
    int complete_original_save_roundtrip_route;
    int user_save_corpus_pc34_ready;
    int user_save_corpus_part_envelope_ready;
    int user_save_corpus_roundtrip_ready;
    int user_save_corpus_roundtrip_verified;
    int user_save_corpus_roundtrip_failed;
    unsigned int user_save_corpus_roundtrip_hash;
    int user_save_corpus_rejected;
    int user_save_corpus_truncated;
    char user_save_corpus_first_pc34_path[512];
    const char* source_evidence;
} DM1_V1_StartupHoCBootProbeSummary_PC34;

typedef enum DM1_V1_StartupHoCBootProbeExpectation_PC34 {
    DM1_V1_STARTUP_HOC_BOOT_PROBE_EXPECT_COMPLETE_SUPPORT_PC34 = 1,
    DM1_V1_STARTUP_HOC_BOOT_PROBE_EXPECT_RELEASE_APP_CAPTURE_PC34 = 2,
    DM1_V1_STARTUP_HOC_BOOT_PROBE_EXPECT_ORIGINAL_SAVE_CORPUS_PC34 = 3
} DM1_V1_StartupHoCBootProbeExpectation_PC34;

typedef struct DM1_V1_StartupHoCBootProbeExpectationReceipt_PC34 {
    int handled;
    int expectation;
    int ready;
    int complete_support_ready;
    int release_app_capture_ready;
    int original_save_corpus_ready;
    char diagnostic[1536];
    const char* source_evidence;
} DM1_V1_StartupHoCBootProbeExpectationReceipt_PC34;

typedef struct DM1_V1_StartupHoCBootProbeLogReceipt_PC34 {
    int handled;
    int ready;
    char fields[3072];
    const char* source_evidence;
} DM1_V1_StartupHoCBootProbeLogReceipt_PC34;

typedef struct DM1_V1_StartupHoCBootProbeHostFields_PC34 {
    int handled;
    int full_graphics_ready;
    int host_render_plan_ready;
    int capture_proof_passed;
    int runtime_apply_ready;
    int production_consumer_ready;
    int no_host_fallback_visuals;
    int real_asset_capture;
    int mac_window_capture;
    int release_app_capture;
    int release_app_identity_ready;
    unsigned int release_app_identity_hash;
    int host_capture_route_matches;
    int release_capture_ownership_ready;
    int host_render_consumer_ready;
    int m11_boot_probe_consumer_ready;
    int launch_path_ready;
    int required_asset_capture;
    int receipt_only_consumer_ready;
    int lower_level_helpers_ready;
    int host_draw_uses_owned_receipt;
    int host_draw_consumes_backing_asset;
    int host_draw_rejects_backing_fallback;
    int hoc_asset_capture;
    int host_window_capture;
    int presented_capture;
    int presented_capture_width;
    int presented_capture_height;
    int presented_capture_geometry;
    int presented_capture_pixels;
    int presented_capture_bytes;
    unsigned int presented_capture_hash;
    int presented_capture_chain_ready;
    unsigned int presented_capture_consumer_mask;
    unsigned int presented_capture_chain_hash;
    int host_capture_route_packaged;
    unsigned int host_capture_route_mask;
    unsigned int host_capture_route_hash;
    int presented_capture_route_packaged;
    int opened_entrance_frame;
    int hall_mirror_overlay;
    int blocked_enter_until_champion;
    int map_width;
    int map_height;
    int render_command_count;
    int consumed_hoc_save_capture_host_readiness;
    int hoc_save_capture_ready;
    int hoc_original_save_capture_ready;
    int complete_support_ready;
    int complete_source_visible_startup;
    int complete_entrance_to_hoc;
    int complete_hoc_render_route;
    int complete_host_app_capture_route;
    int complete_save_corpus_route;
    int complete_original_save_roundtrip_route;
    const char* source_evidence;
} DM1_V1_StartupHoCBootProbeHostFields_PC34;

typedef struct DM1_V1_StartupHoCM12CapturePackageFacts_PC34 {
    const char* source_id;
    int data_ready;
    int version_ready;
    int startup_contract_ready;
    int required_graphics_asset_ready;
    int required_dungeon_asset_ready;
    const DM1_V1_StartupHoCPresentedCaptureHostExportReceipt_PC34*
        presented_capture;
} DM1_V1_StartupHoCM12CapturePackageFacts_PC34;

typedef struct DM1_V1_StartupHoCM12CaptureFields_PC34 {
    int handled;
    int ready;
    int real_asset_capture_ready;
    int mac_window_capture_ready;
    int release_app_capture_ready;
    int release_app_identity_ready;
    unsigned int release_app_identity_hash;
    int host_capture_route_ready;
    int release_capture_ownership_ready;
    int host_render_consumer_ready;
    int m12_capture_consumer_ready;
    int launch_path_ready;
    int required_asset_capture_ready;
    int receipt_only_consumer_ready;
    int no_host_fallback_visuals_ready;
    int lower_level_helpers_ready;
    int host_draw_uses_owned_receipt_ready;
    int host_draw_consumes_backing_asset_ready;
    int host_draw_rejects_backing_fallback_ready;
    int hoc_asset_capture_ready;
    int host_window_capture_ready;
    int presented_capture_ready;
    int presented_capture_width;
    int presented_capture_height;
    int presented_capture_geometry_ready;
    int presented_capture_pixels_ready;
    int presented_capture_bytes;
    unsigned int presented_capture_hash;
    int presented_capture_chain_ready;
    unsigned int presented_capture_consumer_mask;
    unsigned int presented_capture_chain_hash;
    int host_capture_route_packaged_ready;
    unsigned int host_capture_route_mask;
    unsigned int host_capture_route_hash;
    int presented_capture_route_packaged_ready;
    int opened_entrance_frame_ready;
    int hall_mirror_overlay_ready;
    int blocked_enter_until_champion_ready;
    int render_command_count;
    const char* source_evidence;
} DM1_V1_StartupHoCM12CaptureFields_PC34;

typedef struct DM1_V1_StartupHoCRenderConsumerReceipt_PC34 {
    int handled;
    int ready;
    int consumed_hoc_first_frame_receipt;
    int consumed_mirror_thing_layer_consumer;
    int consume_dm1_receipts_only;
    int no_m11_fallback_scan;
    int execute_before_hoc_input;
    int draw_opened_entrance_frame;
    int clear_champion_panel;
    int render_hall_mirror_overlay;
    int draw_champion_mirror_wall_overlay;
    int draw_real_floor_object;
    int draw_real_projectile;
    int require_runtime_spell_effect_receipt;
    int suppress_mirror_floor_item_payload;
    int suppress_mirror_projectile_payload;
    int suppress_mirror_spell_effect_payload;
    int suppress_materialized_item_payload;
    int suppress_host_fallback_visuals;
    int block_enter_until_champion_selected;
    int map_index;
    int entrance_door_frame_index;
    int hall_overlay_kind;
    int render_command_count;
    int zone;
    int row;
    int view_cell;
    const char* source_evidence;
} DM1_V1_StartupHoCRenderConsumerReceipt_PC34;

typedef struct DM1_V1_StartupHoCFallbackDrawOwnershipReceipt_PC34 {
    int handled;
    int ready;
    int consumed_production_consumer_receipt;
    int consumed_render_consumer_receipt;
    int consume_dm1_receipts_only;
    int no_m11_fallback_scan;
    int execute_before_hoc_input;
    int draw_opened_entrance_frame;
    int clear_champion_panel;
    int render_hall_mirror_overlay;
    int draw_champion_mirror_wall_overlay;
    int draw_real_floor_object;
    int draw_real_projectile;
    int require_runtime_spell_effect_receipt;
    int suppress_title_surface;
    int suppress_closed_door_frame;
    int suppress_host_fallback_visuals;
    int suppress_false_item_payloads;
    int suppress_projectile_payloads;
    int suppress_spell_effect_payloads;
    int suppress_mirror_payload_things;
    int suppress_materialized_item_payload;
    int redmcsb_c026_portrait_overlay_ready;
    int redmcsb_c346_mirror_backing_ready;
    int redmcsb_f0115_thing_layer_suppression_ready;
    int block_enter_until_champion_selected;
    int map_index;
    int map_width;
    int map_height;
    int entrance_door_frame_index;
    int hall_overlay_kind;
    int render_command_count;
    int zone;
    int row;
    int view_cell;
    int walk_capture_safe;
    const char* capture_phase;
    const char* source_evidence;
} DM1_V1_StartupHoCFallbackDrawOwnershipReceipt_PC34;

typedef struct DM1_V1_StartupHandoffCallbacks_PC34 {
    void* user;
    int (*begin_prelude_plan)(void* user,
                              const DM1_V1_StartupHandoffPreludePlan_PC34* plan);
    int (*end_prelude_plan)(void* user);
    int (*begin_post_launch_plan)(
        void* user,
        const DM1_V1_StartupHandoffPostLaunchPlan_PC34* plan);
    int (*end_post_launch_plan)(void* user);
    int (*report_source_order_failure)(void* user, const char* evidence);
    int (*raise_window)(void* user);
    int (*play_swsh)(void* user, const char* game_id, int preserve_audio);
    int (*discard_presentation_texture)(void* user);
    int (*play_title)(void* user, const char* source_id, int* out_played_any_frame);
    int (*play_entrance)(void* user,
                         const char* source_id,
                         int auto_enter_after_ms,
                         int* out_entrance_command);
} DM1_V1_StartupHandoffCallbacks_PC34;

typedef struct DM1_V1_StartupHostCallbacks_PC34 {
    void* user;
    int (*set_game_active)(void* user, int active);
    int (*resolve_resume_save_path)(void* user,
                                    const char* source_id,
                                    char* out_path,
                                    int out_path_size);
    int (*load_resume_save_path)(void* user,
                                 const char* save_path,
                                 int* out_used_backup);
    int (*log_resume_loaded)(void* user,
                             const char* save_path,
                             int used_backup);
    int (*log_resume_missing)(void* user, const char* save_path);
    int (*log_entrance_skipped)(void* user);
} DM1_V1_StartupHostCallbacks_PC34;

typedef struct DM1_V1_StartupSelectedLaunchCallbacks_PC34 {
    void* user;
    const DM1_V1_StartupHandoffCallbacks_PC34* handoff_callbacks;
    const DM1_V1_StartupHostCallbacks_PC34* host_callbacks;
    int (*open_selected_entry)(void* user,
                               char* out_source_id,
                               int out_source_id_size);
    int (*after_open)(void* user);
    int (*draw_opened)(void* user);
    int (*mark_launch_failed)(void* user);
} DM1_V1_StartupSelectedLaunchCallbacks_PC34;

typedef struct DM1_V1_StartupSelectedLaunchResult_PC34 {
    int handled;
    int opened;
    int launch_failed;
    DM1_V1_StartupHandoffOutcome_PC34 handoff_outcome;
    DM1_V1_StartupHostApplyResult_PC34 host_apply_result;
    DM1_V1_StartupFullGraphicsRuntimeHandoffReceipt_PC34 runtime_handoff_receipt;
} DM1_V1_StartupSelectedLaunchResult_PC34;

typedef struct DM1_V1_StartupSelectedLaunchRouteFacts_PC34 {
    const char* selected_game_id;
} DM1_V1_StartupSelectedLaunchRouteFacts_PC34;

typedef struct DM1_V1_StartupSelectedLaunchRouteReceipt_PC34 {
    int handled;
    int use_dm1_transaction;
    int use_generic_launch;
    int requires_source_visible_intro;
} DM1_V1_StartupSelectedLaunchRouteReceipt_PC34;

typedef struct DM1_V1_StartupLaunchPathFacts_PC34 {
    const char* source_id;
    DM1_V1_StartupLaunchPath_PC34 launch_path;
} DM1_V1_StartupLaunchPathFacts_PC34;

typedef struct DM1_V1_StartupLaunchPathReceipt_PC34 {
    int handled;
    int intro_bypassed;
    int started_from_launcher;
    int selected_entry_receipt_valid;
} DM1_V1_StartupLaunchPathReceipt_PC34;

enum {
    DM1_V1_STARTUP_RUNTIME_TITLE_CAPACITY_PC34 = 64,
    DM1_V1_STARTUP_RUNTIME_SOURCE_ID_CAPACITY_PC34 = 32,
    DM1_V1_STARTUP_RUNTIME_MD5_CAPACITY_PC34 = 33,
    DM1_V1_STARTUP_RUNTIME_PATH_CAPACITY_PC34 = 512,
    DM1_V1_STARTUP_RUNTIME_STATUS_CAPACITY_PC34 = 32,
    DM1_V1_STARTUP_RUNTIME_INSPECT_CAPACITY_PC34 = 128
};

typedef struct DM1_V1_StartupRuntimeStartFacts_PC34 {
    const char* game_id;
    const char* source_id;
    const char* title;
    const char* verified_asset_md5;
    const char* dungeon_path;
    int source_kind;
    int presentation_mode;
    int presentation_width;
    int presentation_height;
    int font_scale;
    DM1_V1_StartupLaunchPath_PC34 launch_path;
} DM1_V1_StartupRuntimeStartFacts_PC34;

typedef struct DM1_V1_StartupRuntimeStartReceipt_PC34 {
    int handled;
    int active;
    int started_from_launcher;
    int source_kind;
    char boot_asset_md5[DM1_V1_STARTUP_RUNTIME_MD5_CAPACITY_PC34];
    int presentation_mode;
    int presentation_width;
    int presentation_height;
    int font_scale;
    char title[DM1_V1_STARTUP_RUNTIME_TITLE_CAPACITY_PC34];
    char source_id[DM1_V1_STARTUP_RUNTIME_SOURCE_ID_CAPACITY_PC34];
    char dungeon_path[DM1_V1_STARTUP_RUNTIME_PATH_CAPACITY_PC34];
    DM1_V1_StartupLaunchPathReceipt_PC34 launch_path_receipt;
    char status_title[DM1_V1_STARTUP_RUNTIME_STATUS_CAPACITY_PC34];
    char status_detail[DM1_V1_STARTUP_RUNTIME_STATUS_CAPACITY_PC34];
    char inspect_title[DM1_V1_STARTUP_RUNTIME_STATUS_CAPACITY_PC34];
    char inspect_detail[DM1_V1_STARTUP_RUNTIME_INSPECT_CAPACITY_PC34];
} DM1_V1_StartupRuntimeStartReceipt_PC34;

enum {
    DM1_V1_STARTUP_SOURCE_KIND_BUILTIN_CATALOG_PC34 = 0,
    DM1_V1_STARTUP_SOURCE_KIND_CUSTOM_DUNGEON_PC34 = 1,
    DM1_V1_STARTUP_SOURCE_KIND_DIRECT_DUNGEON_PC34 = 2
};

typedef struct DM1_V1_StartupDungeonPathFacts_PC34 {
    const char* game_id;
    const char* data_dir;
    const char* explicit_dungeon_path;
    int source_kind;
} DM1_V1_StartupDungeonPathFacts_PC34;

typedef struct DM1_V1_StartupDungeonPathReceipt_PC34 {
    int handled;
    int explicit_path_required;
    int use_explicit_path;
    int resolve_builtin_path;
    char explicit_dungeon_path[DM1_V1_STARTUP_RUNTIME_PATH_CAPACITY_PC34];
} DM1_V1_StartupDungeonPathReceipt_PC34;

typedef struct DM1_V1_StartupGraphicsBindFacts_PC34 {
    const char* game_id;
    const char* dungeon_path;
} DM1_V1_StartupGraphicsBindFacts_PC34;

typedef struct DM1_V1_StartupGraphicsBindReceipt_PC34 {
    int handled;
    int bind_graphics_dat;
    char graphics_dat_path[DM1_V1_STARTUP_RUNTIME_PATH_CAPACITY_PC34];
} DM1_V1_StartupGraphicsBindReceipt_PC34;

typedef struct DM1_V1_StartupDungeonLoadFacts_PC34 {
    const char* game_id;
    int load_succeeded;
} DM1_V1_StartupDungeonLoadFacts_PC34;

typedef struct DM1_V1_StartupDungeonLoadReceipt_PC34 {
    int handled;
    int load_succeeded;
    char status_title[DM1_V1_STARTUP_RUNTIME_STATUS_CAPACITY_PC34];
    char status_detail[DM1_V1_STARTUP_RUNTIME_STATUS_CAPACITY_PC34];
} DM1_V1_StartupDungeonLoadReceipt_PC34;

typedef struct DM1_V1_StartupRuntimeReadyFacts_PC34 {
    DM1_V1_StartupRuntimeStartFacts_PC34 runtime_start;
    int load_succeeded;
} DM1_V1_StartupRuntimeReadyFacts_PC34;

typedef struct DM1_V1_StartupRuntimeReadyReceipt_PC34 {
    int handled;
    DM1_V1_StartupDungeonLoadReceipt_PC34 load_receipt;
    DM1_V1_StartupRuntimeStartReceipt_PC34 runtime_start_receipt;
    DM1_V1_StartupGraphicsBindReceipt_PC34 graphics_bind_receipt;
} DM1_V1_StartupRuntimeReadyReceipt_PC34;

enum {
    DM1_V1_STARTUP_BOOT_PROBE_SOURCE_ID_CAPACITY_PC34 = 32,
    DM1_V1_STARTUP_BOOT_PROBE_PHASE_CAPACITY_PC34 = 48,
    DM1_V1_STARTUP_BOOT_PROBE_ANIMATION_CAPACITY_PC34 = 48
};

typedef struct DM1_V1_StartupBootProbeFacts_PC34 {
    const char* source_id;
    int level_loaded;
    int intro_bypassed;
    int map_index;
    int party_x;
    int party_y;
    int party_dir;
    int champion_count;
    int runtime_tick;
    unsigned int world_tick;
} DM1_V1_StartupBootProbeFacts_PC34;

typedef struct DM1_V1_StartupBootProbeReceipt_PC34 {
    int handled;
    char source_id[DM1_V1_STARTUP_BOOT_PROBE_SOURCE_ID_CAPACITY_PC34];
    int dm1_startup_intro_bypassed;
    char startup_phase[DM1_V1_STARTUP_BOOT_PROBE_PHASE_CAPACITY_PC34];
    int startup_active;
    int startup_frame;
    char startup_animation[DM1_V1_STARTUP_BOOT_PROBE_ANIMATION_CAPACITY_PC34];
    int startup_animation_active;
    int startup_title_frame;
    int startup_title_frame_max;
    int startup_title_ready;
    int level_loaded;
    int map_index;
    int party_x;
    int party_y;
    int party_dir;
    int champion_count;
    int runtime_tick;
    unsigned int world_tick;
} DM1_V1_StartupBootProbeReceipt_PC34;

typedef struct DM1_V1_StartupTitleMenuEligibilityFacts_PC34 {
    unsigned int title_frame;
    unsigned int title_frame_max;
    int advance_requested;
    int title_handoff_ready;
} DM1_V1_StartupTitleMenuEligibilityFacts_PC34;

typedef struct DM1_V1_StartupTitleMenuEligibilityReceipt_PC34 {
    int handled;
    int menu_eligible;
    int keep_title_surface;
    int consume_pending_input;
    DM1_V1_StartupStage_PC34 next_stage;
    const char* reason;
} DM1_V1_StartupTitleMenuEligibilityReceipt_PC34;

typedef struct DM1_V1_StartupSelectedBootProbeFacts_PC34 {
    const char* expected_game_id;
    const char* actual_source_id;
    int active;
    int started_from_launcher;
    int intro_bypassed;
} DM1_V1_StartupSelectedBootProbeFacts_PC34;

typedef struct DM1_V1_StartupSelectedBootProbeReceipt_PC34 {
    int handled;
    int valid;
    int selected_entry_receipt_valid;
    int source_matches;
    int active;
    int started_from_launcher;
} DM1_V1_StartupSelectedBootProbeReceipt_PC34;

typedef struct DM1_V1_StartupSelectedBootProbeSourceKindFacts_PC34 {
    const char* expected_game_id;
    int actual_source_kind;
    int dm1_builtin_source_kind;
} DM1_V1_StartupSelectedBootProbeSourceKindFacts_PC34;

typedef struct DM1_V1_StartupSelectedBootProbeSourceKindReceipt_PC34 {
    int handled;
    int valid;
    int expected_source_kind;
    int actual_source_kind;
} DM1_V1_StartupSelectedBootProbeSourceKindReceipt_PC34;

const char* dm1_v1_startup_stage_name_pc34(DM1_V1_StartupStage_PC34 stage);
int dm1_v1_startup_stage_after_pc34(DM1_V1_StartupStage_PC34 later,
                                    DM1_V1_StartupStage_PC34 earlier);
unsigned int dm1_v1_startup_hoc_presented_capture_chain_hash_pc34(
    int width,
    int height,
    int byte_count,
    unsigned int presented_hash,
    unsigned int consumer_mask);
unsigned int dm1_v1_startup_hoc_presented_rgba_hash_pc34(
    const unsigned char* rgba,
    int width,
    int height,
    int* out_byte_count);
int dm1_v1_startup_hoc_capture_facts_set_presented_rgba_pc34(
    DM1_V1_StartupHoCFullGraphicsCaptureFacts_PC34* facts,
    const unsigned char* rgba,
    int width,
    int height,
    unsigned int consumer_mask);
int dm1_v1_startup_hoc_capture_facts_set_presented_hash_pc34(
    DM1_V1_StartupHoCFullGraphicsCaptureFacts_PC34* facts,
    int width,
    int height,
    int byte_count,
    unsigned int presented_hash,
    unsigned int consumer_mask);
int dm1_v1_startup_hoc_host_probe_facts_set_presented_rgba_pc34(
    DM1_V1_StartupHoCFullGraphicsHostProbeFacts_PC34* facts,
    const unsigned char* rgba,
    int width,
    int height,
    unsigned int consumer_mask);
int dm1_v1_startup_hoc_host_probe_facts_set_presented_hash_pc34(
    DM1_V1_StartupHoCFullGraphicsHostProbeFacts_PC34* facts,
    int width,
    int height,
    int byte_count,
    unsigned int presented_hash,
    unsigned int consumer_mask);
int dm1_v1_startup_hoc_capture_facts_apply_host_observation_pc34(
    DM1_V1_StartupHoCFullGraphicsCaptureFacts_PC34* facts,
    const DM1_V1_StartupHoCHostCaptureObservation_PC34* observation);
int dm1_v1_startup_hoc_host_probe_facts_apply_host_observation_pc34(
    DM1_V1_StartupHoCFullGraphicsHostProbeFacts_PC34* facts,
    const DM1_V1_StartupHoCHostCaptureObservation_PC34* observation);
int dm1_v1_startup_launch_path_bypasses_intro_pc34(
    DM1_V1_StartupLaunchPath_PC34 path);
int dm1_v1_startup_source_visible_handoff_required_pc34(const char* game_id);
int dm1_v1_startup_intro_bypass_applies_to_source_pc34(const char* sourceId,
                                                       int bypassed);
int dm1_v1_startup_selected_entry_receipt_valid_pc34(const char* game_id,
                                                     int intro_bypassed);
int dm1_v1_startup_launch_path_receipt_pc34(
    const DM1_V1_StartupLaunchPathFacts_PC34* facts,
    DM1_V1_StartupLaunchPathReceipt_PC34* out_receipt);
int dm1_v1_startup_runtime_start_receipt_pc34(
    const DM1_V1_StartupRuntimeStartFacts_PC34* facts,
    DM1_V1_StartupRuntimeStartReceipt_PC34* out_receipt);
int dm1_v1_startup_dungeon_path_receipt_pc34(
    const DM1_V1_StartupDungeonPathFacts_PC34* facts,
    DM1_V1_StartupDungeonPathReceipt_PC34* out_receipt);
int dm1_v1_startup_graphics_bind_receipt_pc34(
    const DM1_V1_StartupGraphicsBindFacts_PC34* facts,
    DM1_V1_StartupGraphicsBindReceipt_PC34* out_receipt);
int dm1_v1_startup_dungeon_load_receipt_pc34(
    const DM1_V1_StartupDungeonLoadFacts_PC34* facts,
    DM1_V1_StartupDungeonLoadReceipt_PC34* out_receipt);
int dm1_v1_startup_runtime_ready_receipt_pc34(
    const DM1_V1_StartupRuntimeReadyFacts_PC34* facts,
    DM1_V1_StartupRuntimeReadyReceipt_PC34* out_receipt);
int dm1_v1_startup_handoff_prelude_plan_pc34(
    const char* game_id,
    DM1_V1_StartupHandoffPreludePlan_PC34* out_plan);
int dm1_v1_startup_handoff_post_launch_plan_pc34(
    const char* source_id,
    DM1_V1_StartupHandoffPostLaunchPlan_PC34* out_plan);
int dm1_v1_startup_execute_handoff_prelude_pc34(
    const char* game_id,
    const DM1_V1_StartupHandoffCallbacks_PC34* callbacks);
int dm1_v1_startup_execute_handoff_post_launch_pc34(
    const char* source_id,
    const DM1_V1_StartupHandoffCallbacks_PC34* callbacks,
    int* out_title_played,
    int* out_entrance_command);
int dm1_v1_startup_handoff_outcome_from_entrance_command_pc34(
    int entrance_command,
    DM1_V1_StartupHandoffOutcome_PC34* out_outcome);
int dm1_v1_startup_execute_handoff_post_launch_outcome_pc34(
    const char* source_id,
    const DM1_V1_StartupHandoffCallbacks_PC34* callbacks,
    DM1_V1_StartupHandoffOutcome_PC34* out_outcome);
int dm1_v1_startup_apply_handoff_outcome_pc34(
    const DM1_V1_StartupHandoffOutcome_PC34* outcome,
    const char* source_id,
    const DM1_V1_StartupHostCallbacks_PC34* callbacks,
    DM1_V1_StartupHostApplyResult_PC34* out_result);
int dm1_v1_startup_full_graphics_runtime_handoff_receipt_pc34(
    const char* selected_game_id,
    const char* opened_source_id,
    const DM1_V1_StartupHandoffOutcome_PC34* outcome,
    const DM1_V1_StartupHostApplyResult_PC34* host_result,
    DM1_V1_StartupFullGraphicsRuntimeHandoffReceipt_PC34* out_receipt);
int dm1_v1_startup_save_resume_capture_receipt_pc34(
    const DM1_V1_StartupSaveResumeCaptureFacts_PC34* facts,
    DM1_V1_StartupSaveResumeCaptureReceipt_PC34* out_receipt);
int dm1_v1_startup_hoc_first_frame_receipt_pc34(
    const char* source_id,
    const DM1_V1_StartupHandoffPostLaunchPlan_PC34* post_plan,
    const DM1_V1_StartupHandoffOutcome_PC34* outcome,
    DM1_V1_StartupHoCFirstFrameReceipt_PC34* out_receipt);
int dm1_v1_startup_hoc_host_render_plan_from_first_frame_pc34(
    const DM1_V1_StartupHoCFirstFrameReceipt_PC34* receipt,
    DM1_V1_StartupHoCHostRenderPlan_PC34* out_plan);
int dm1_v1_startup_hoc_packaged_full_graphics_proof_from_host_plan_pc34(
    const DM1_V1_StartupHoCHostRenderPlan_PC34* plan,
    DM1_V1_StartupHoCPackagedFullGraphicsProof_PC34* out_proof);
int dm1_v1_startup_hoc_production_full_start_hook_from_proof_pc34(
    const DM1_V1_StartupHoCPackagedFullGraphicsProof_PC34* proof,
    DM1_V1_StartupHoCProductionFullStartHook_PC34* out_hook);
int dm1_v1_startup_hoc_full_start_production_receipt_pc34(
    const char* source_id,
    const DM1_V1_StartupHandoffPostLaunchPlan_PC34* post_plan,
    const DM1_V1_StartupHandoffOutcome_PC34* outcome,
    DM1_V1_StartupHoCFullStartProductionReceipt_PC34* out_receipt);
int dm1_v1_startup_hoc_full_graphics_capture_artifact_from_production_pc34(
    const DM1_V1_StartupHoCFullStartProductionReceipt_PC34* receipt,
    DM1_V1_StartupHoCFullGraphicsCaptureArtifact_PC34* out_artifact);
int dm1_v1_startup_hoc_full_graphics_capture_proof_receipt_pc34(
    const DM1_V1_StartupHoCFullGraphicsCaptureArtifact_PC34* artifact,
    const DM1_V1_StartupHoCFullGraphicsCaptureFacts_PC34* facts,
    DM1_V1_StartupHoCFullGraphicsCaptureProofReceipt_PC34* out_receipt);
int dm1_v1_startup_hoc_full_graphics_runtime_apply_receipt_pc34(
    const DM1_V1_StartupHoCFullGraphicsCaptureArtifact_PC34* artifact,
    const DM1_V1_StartupHoCFullGraphicsCaptureProofReceipt_PC34* proof,
    DM1_V1_StartupHoCFullGraphicsRuntimeApplyReceipt_PC34* out_receipt);
int dm1_v1_startup_hoc_full_graphics_thing_suppression_receipt_pc34(
    const DM1_V1_StartupHoCFullGraphicsRuntimeApplyReceipt_PC34* apply,
    const DM1_V1_StartupHoCFullGraphicsThingSuppressionFacts_PC34* facts,
    DM1_V1_StartupHoCFullGraphicsThingSuppressionReceipt_PC34* out_receipt);
int dm1_v1_startup_hoc_full_graphics_production_consumer_receipt_pc34(
    const DM1_V1_StartupHoCFullGraphicsRuntimeApplyReceipt_PC34* apply,
    const DM1_V1_StartupHoCFullGraphicsThingSuppressionReceipt_PC34* suppression,
    DM1_V1_StartupHoCFullGraphicsProductionConsumerReceipt_PC34* out_receipt);
int dm1_v1_startup_hoc_full_graphics_host_probe_receipt_pc34(
    const DM1_V1_StartupHoCFullGraphicsHostProbeFacts_PC34* facts,
    DM1_V1_StartupHoCFullGraphicsRuntimeApplyReceipt_PC34* out_apply,
    DM1_V1_StartupHoCFullGraphicsProductionConsumerReceipt_PC34* out_consumer);
int dm1_v1_startup_hoc_release_app_capture_ownership_receipt_pc34(
    const DM1_V1_StartupHoCFullGraphicsHostProbeFacts_PC34* facts,
    DM1_V1_StartupHoCReleaseAppCaptureOwnershipReceipt_PC34* out_receipt);
int dm1_v1_startup_hoc_presented_capture_publish_receipt_pc34(
    const DM1_V1_StartupHoCPresentedCapturePublishFacts_PC34* facts,
    DM1_V1_StartupHoCPresentedCapturePublishReceipt_PC34* out_receipt);
int dm1_v1_startup_hoc_presented_capture_publish_from_boot_summary_pc34(
    const DM1_V1_StartupHoCBootProbeSummary_PC34* summary,
    DM1_V1_StartupHoCPresentedCapturePublishReceipt_PC34* out_receipt);
int dm1_v1_startup_hoc_presented_capture_host_export_receipt_pc34(
    const DM1_V1_StartupHoCPresentedCapturePublishReceipt_PC34* publish,
    DM1_V1_StartupHoCPresentedCaptureHostExportReceipt_PC34* out_receipt);
int dm1_v1_startup_hoc_presented_capture_host_export_from_boot_summary_pc34(
    const DM1_V1_StartupHoCBootProbeSummary_PC34* summary,
    DM1_V1_StartupHoCPresentedCaptureHostExportReceipt_PC34* out_receipt);
int dm1_v1_startup_hoc_presented_capture_m12_import_receipt_pc34(
    const DM1_V1_StartupHoCPresentedCaptureM12ImportFacts_PC34* facts,
    DM1_V1_StartupHoCPresentedCaptureM12ImportReceipt_PC34* out_receipt);
int dm1_v1_startup_hoc_save_capture_host_readiness_receipt_pc34(
    const DM1_V1_StartupFullGraphicsRuntimeHandoffReceipt_PC34* handoff,
    const DM1_V1_StartupHostApplyResult_PC34* host_apply,
    const DM1_V1_StartupHoCReleaseAppCaptureOwnershipReceipt_PC34* ownership,
    DM1_V1_StartupHoCSaveCaptureHostReadinessReceipt_PC34* out_receipt);
int dm1_v1_complete_support_receipt_pc34(
    const DM1_V1_StartupFullGraphicsRuntimeHandoffReceipt_PC34* hoc_handoff,
    const DM1_V1_StartupHoCReleaseAppCaptureOwnershipReceipt_PC34* ownership,
    const DM1_V1_StartupHoCSaveCaptureHostReadinessReceipt_PC34* hoc_save,
    const DM1_V1_StartupSaveResumeCaptureReceipt_PC34* original_save,
    DM1_V1_CompleteSupportReceipt_PC34* out_receipt);
int dm1_v1_startup_hoc_boot_full_graphics_receipt_pc34(
    const DM1_V1_StartupHoCFullGraphicsHostProbeFacts_PC34* facts,
    const DM1_V1_StartupFullGraphicsRuntimeHandoffReceipt_PC34* hoc_handoff,
    const DM1_V1_StartupHoCSaveCaptureHostReadinessReceipt_PC34* hoc_save,
    const DM1_V1_StartupSaveResumeCaptureReceipt_PC34* original_save,
    DM1_V1_StartupHoCBootFullGraphicsReceipt_PC34* out_receipt);
int dm1_v1_startup_hoc_boot_complete_support_from_host_facts_pc34(
    const DM1_V1_StartupHoCBootCompleteSupportFacts_PC34* complete_facts,
    const DM1_V1_StartupHoCFullGraphicsHostProbeFacts_PC34* hoc_facts,
    const DM1_V1_StartupHoCReleaseAppCaptureOwnershipReceipt_PC34* ownership,
    DM1_V1_StartupHoCBootFullGraphicsReceipt_PC34* out_receipt);
int dm1_v1_startup_hoc_boot_probe_summary_from_host_facts_pc34(
    const DM1_V1_StartupHoCBootCompleteSupportFacts_PC34* complete_facts,
    const DM1_V1_StartupHoCFullGraphicsHostProbeFacts_PC34* hoc_facts,
    DM1_V1_StartupHoCBootFullGraphicsReceipt_PC34* out_receipt,
    DM1_V1_StartupHoCBootProbeSummary_PC34* out_summary);
int dm1_v1_startup_hoc_boot_probe_summary_pc34(
    const DM1_V1_StartupHoCBootFullGraphicsReceipt_PC34* receipt,
    DM1_V1_StartupHoCBootProbeSummary_PC34* out_summary);
int dm1_v1_startup_hoc_boot_probe_complete_support_ready_pc34(
    const DM1_V1_StartupHoCBootProbeSummary_PC34* summary);
int dm1_v1_startup_hoc_boot_probe_release_app_capture_ready_pc34(
    const DM1_V1_StartupHoCBootProbeSummary_PC34* summary);
int dm1_v1_startup_hoc_boot_probe_expectation_receipt_pc34(
    const DM1_V1_StartupHoCBootProbeSummary_PC34* summary,
    DM1_V1_StartupHoCBootProbeExpectation_PC34 expectation,
    DM1_V1_StartupHoCBootProbeExpectationReceipt_PC34* out_receipt);
int dm1_v1_startup_hoc_boot_probe_log_receipt_pc34(
    const DM1_V1_StartupHoCBootProbeSummary_PC34* summary,
    DM1_V1_StartupHoCBootProbeLogReceipt_PC34* out_receipt);
int dm1_v1_startup_hoc_boot_probe_host_fields_pc34(
    const DM1_V1_StartupHoCBootProbeSummary_PC34* summary,
    DM1_V1_StartupHoCBootProbeHostFields_PC34* out_fields);
int dm1_v1_startup_hoc_m12_capture_fields_pc34(
    const DM1_V1_StartupHoCFullGraphicsHostProbeFacts_PC34* facts,
    DM1_V1_StartupHoCM12CaptureFields_PC34* out_fields);
int dm1_v1_startup_hoc_m12_capture_fields_from_package_pc34(
    const DM1_V1_StartupHoCM12CapturePackageFacts_PC34* facts,
    DM1_V1_StartupHoCM12CaptureFields_PC34* out_fields);
int dm1_v1_startup_hoc_render_consumer_from_first_frame_and_thing_pc34(
    const DM1_V1_StartupHoCFirstFrameReceipt_PC34* first_frame,
    const DM1_V1_ChampionMirrorThingLayerConsumerReceiptPc34* thing_consumer,
    DM1_V1_StartupHoCRenderConsumerReceipt_PC34* out_receipt);
int dm1_v1_startup_hoc_fallback_draw_ownership_receipt_pc34(
    const DM1_V1_StartupHoCFullGraphicsProductionConsumerReceipt_PC34* production,
    const DM1_V1_StartupHoCRenderConsumerReceipt_PC34* render,
    DM1_V1_StartupHoCFallbackDrawOwnershipReceipt_PC34* out_receipt);
int dm1_v1_startup_hoc_owned_host_draw_receipt_pc34(
    const DM1_V1_StartupHoCFallbackDrawOwnershipReceipt_PC34* ownership,
    const DM1_V1_ChampionMirrorRenderReceiptPc34* render,
    int candidate_panel_active,
    int backing_asset_available,
    DM1_V1_ChampionMirrorHostDrawReceiptPc34* out_receipt);
int dm1_v1_startup_execute_handoff_post_launch_and_apply_pc34(
    const char* source_id,
    const DM1_V1_StartupHandoffCallbacks_PC34* handoff_callbacks,
    const DM1_V1_StartupHostCallbacks_PC34* host_callbacks,
    DM1_V1_StartupHandoffOutcome_PC34* out_outcome,
    DM1_V1_StartupHostApplyResult_PC34* out_result);
int dm1_v1_startup_execute_selected_launch_transaction_pc34(
    const char* selected_game_id,
    const DM1_V1_StartupSelectedLaunchCallbacks_PC34* callbacks,
    DM1_V1_StartupSelectedLaunchResult_PC34* out_result);
int dm1_v1_startup_selected_launch_route_receipt_pc34(
    const DM1_V1_StartupSelectedLaunchRouteFacts_PC34* facts,
    DM1_V1_StartupSelectedLaunchRouteReceipt_PC34* out_receipt);
int dm1_v1_startup_receipt_phase_pc34(int level_loaded,
                                      int intro_bypassed,
                                      char* out_phase,
                                      int out_phase_size);
int dm1_v1_startup_boot_probe_receipt_pc34(int level_loaded,
                                           int intro_bypassed,
                                           char* out_phase,
                                           int out_phase_size,
                                           int* out_startup_active,
                                           char* out_animation,
                                           int out_animation_size,
                                           int* out_animation_active,
                                           int* out_title_frame,
                                           int* out_title_frame_max,
                                           int* out_title_ready);
int dm1_v1_startup_boot_probe_receipt_from_facts_pc34(
    const DM1_V1_StartupBootProbeFacts_PC34* facts,
    DM1_V1_StartupBootProbeReceipt_PC34* out_receipt);
int dm1_v1_startup_selected_boot_probe_receipt_pc34(
    const DM1_V1_StartupSelectedBootProbeFacts_PC34* facts,
    DM1_V1_StartupSelectedBootProbeReceipt_PC34* out_receipt);
int dm1_v1_startup_selected_boot_probe_source_kind_receipt_pc34(
    const DM1_V1_StartupSelectedBootProbeSourceKindFacts_PC34* facts,
    DM1_V1_StartupSelectedBootProbeSourceKindReceipt_PC34* out_receipt);
int dm1_v1_startup_title_menu_eligibility_receipt_pc34(
    const DM1_V1_StartupTitleMenuEligibilityFacts_PC34* facts,
    DM1_V1_StartupTitleMenuEligibilityReceipt_PC34* out_receipt);
int dm1_v1_startup_full_graphics_media_receipt_pc34(
    const char* source_id,
    DM1_V1_StartupFullGraphicsMediaReceipt_PC34* out_receipt);
int dm1_v1_startup_full_graphics_media_receipt_for_source_pc34(
    const char* source_id,
    DM1_V1_StartupFullGraphicsMediaReceipt_PC34* out_receipt);
int dm1_v1_startup_title_runtime_source_receipt_pc34(
    const char* source_id,
    int graphics_c001_candidate_available,
    unsigned int graphics_c001_width,
    unsigned int graphics_c001_height,
    int title_dat_fallback_available,
    DM1_V1_StartupTitleRuntimeSourceReceipt_PC34* out_receipt);
unsigned int dm1_v1_startup_entrance_step_delay_ms_pc34(
    const DM1_V1_StartupFullGraphicsMediaReceipt_PC34* media_receipt,
    int entrance_event_kind,
    unsigned int delay_ticks,
    unsigned int vblank_loop_count);
int dm1_v1_startup_entrance_timing_receipt_valid_pc34(
    const DM1_V1_StartupFullGraphicsMediaReceipt_PC34* media_receipt);
int dm1_v1_startup_entrance_render_audio_command_pc34(
    const DM1_V1_StartupFullGraphicsMediaReceipt_PC34* media_receipt,
    unsigned int source_step,
    int entrance_event_kind,
    unsigned int delay_ticks,
    unsigned int vblank_loop_count,
    DM1_V1_StartupEntranceRenderAudioCommand_PC34* out_command);
int dm1_v1_startup_sequence_source_order_valid_pc34(void);
const char* dm1_v1_startup_sequence_source_evidence_pc34(void);
unsigned int dm1_v1_startup_title_zoom_steps_pc34(void);
unsigned int dm1_v1_startup_title_source_animation_steps_pc34(void);
unsigned int dm1_v1_startup_title_frame_bank_equivalent_steps_pc34(void);
unsigned int dm1_v1_startup_title_presents_hold_vblanks_pc34(void);
unsigned int dm1_v1_startup_title_vblank_tick_ms_pc34(void);
unsigned int dm1_v1_startup_title_presents_hold_ms_pc34(void);
unsigned int dm1_v1_startup_title_post_zoom_vblanks_pc34(void);
unsigned int dm1_v1_startup_title_final_guard_vblanks_pc34(void);

#ifdef __cplusplus
}
#endif

#endif

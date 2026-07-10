#ifndef FIRESTAFF_CSB_V1_BOOT_H
#define FIRESTAFF_CSB_V1_BOOT_H

#include <stddef.h>
#include <stdint.h>

#include "csb_v1_csbgraphics_m11_runtime_plan.h"
#include "csb_v1_runtime_pc34_compat.h"
#include "csb_v1_startup_real_asset_receipt.h"
#include "csb_v1_viewport_pc34_compat.h"
#include "firestaff/csb/v1/startup_sequence_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CSB_V1_BOOT_GAME_ID "csb"
#define CSB_V1_BOOT_SAVE_SUBDIR "saves/csb"

typedef enum {
    CSB_V1_BOOT_STATE_EMPTY = 0,
    CSB_V1_BOOT_STATE_PROFILE_READY,
    CSB_V1_BOOT_STATE_ASSETS_READY,
    CSB_V1_BOOT_STATE_RUNTIME_READY
} CSB_V1_BootState;

typedef struct CSB_V1_BootProfile {
    char game_id[8];
    CSB_V1_BootState state;
    CSB_V1_VariantId variant_id;
    char version_id[32];
    char variant_label[64];
    char media_ref[64];

    char asset_root[512];
    char graphics_path[512];
    char dungeon_path[512];
    char save_root[512];
    char graphics_md5[33];
    char dungeon_md5[33];

    int assets_verified;
    int graphics_verified;
    int dungeon_verified;
    CSB_V1_AssetGfxArchiveType graphics_kind;

    uint32_t tick_ms;
    uint32_t entrance_map_index;
    uint32_t start_map_index;
    uint32_t default_party_x;
    uint32_t default_party_y;
    uint32_t default_party_dir;
    int imported_party_ready;
    int cmp_import_attempted;
    int cmp_import_succeeded;
    int cmp_imported_slot;
    int cmp_imported_champion_count;
    int engine_version_displayed;
    CSB_V1_PartyState imported_party;

    int csbgraphics_scan_attempted;
    int csbgraphics_scan_result;
    int csbgraphics_plan_result;
    int csbgraphics_skin_def_loaded;
    uint16_t csbgraphics_skin_def_words
        [CSB_V1_CSBGRAPHICS_M11_SKIN_DEF_MAX_WORDS];
    size_t csbgraphics_skin_def_word_count;
    CSB_V1_CSBGraphicsDatRealCache csbgraphics_cache;
    CSB_V1_CSBGraphicsM11RuntimePlan csbgraphics_m11_plan;

    CSB_V1_RuntimeProfile runtime;
} CSB_V1_BootProfile;

typedef struct CSB_V1_BootStartupLaunchReceipts_PC34 {
    CSB_V1_RuntimeStartupHandoffReceipt_PC34 handoff;
    CSB_V1_StartupInitStateReceipt_PC34 init_state;
    CSB_V1_RuntimeStartupSessionStateReceipt_PC34 session_state;
    CSB_V1_StartupHostReceipt_PC34 launch_host_receipt;
} CSB_V1_BootStartupLaunchReceipts_PC34;

typedef struct CSB_V1_BootStartupLaunch_PC34 {
    CSB_V1_BootProfile *profile;
    CSB_V1_BootStartupLaunchReceipts_PC34 receipts;
    CSB_V1_StartupHostReceipt_PC34 failure_host_receipt;
} CSB_V1_BootStartupLaunch_PC34;

typedef struct CSB_V1_BootStartupRuntimeReceipt_PC34 {
    CSB_V1_BootProfile *profile;
    CSB_V1_BootStartupLaunchReceipts_PC34 receipts;
    char boot_asset_md5[33];
    char title[64];
    char source_id[32];
    int bind_graphics_to_m11_asset_loader;
    int load_original_font_from_graphics;
    int real_asset_receipt_valid;
    CSB_V1_StartupRealReceipt real_asset_receipt;
    char graphics_path[512];
    char dungeon_path[512];
} CSB_V1_BootStartupRuntimeReceipt_PC34;

typedef struct CSB_V1_BootRuntimeStartupSnapshot_PC34 {
    int title_active;
    int title_frame;
    int title_source_step;
    int entrance_active;
    int entrance_source_step;
    int entrance_dismissed;
    int credits_active;
    int credits_remaining_ticks;
    int opening_active;
    int opening_delay_ticks;
    int opening_step;
    int pending_command;
    int entrance_frame;
    int utility_overlay_active;
    int utility_selected_action_index;
    int utility_imported_champion_count;
    int utility_preview_active;
    const char *utility_prompt;
    int resume_available;
    const char *resume_path;
    const CSB_V1_BootProfile *boot_profile;
    int runtime_level_loaded;
    int runtime_map_index;
    int runtime_party_x;
    int runtime_party_y;
    int runtime_party_dir;
    int runtime_champion_count;
    int runtime_tick_count;
} CSB_V1_BootRuntimeStartupSnapshot_PC34;

typedef enum CSB_V1_BootStartupRenderRouteKind_PC34 {
    CSB_V1_BOOT_STARTUP_RENDER_ROUTE_NONE_PC34 = 0,
    CSB_V1_BOOT_STARTUP_RENDER_ROUTE_TITLE_PC34 = 1,
    CSB_V1_BOOT_STARTUP_RENDER_ROUTE_ENTRANCE_BLACK_PC34 = 2,
    CSB_V1_BOOT_STARTUP_RENDER_ROUTE_ENTRANCE_CLOSED_PC34 = 3,
    CSB_V1_BOOT_STARTUP_RENDER_ROUTE_ENTRANCE_CREDITS_PC34 = 4,
    CSB_V1_BOOT_STARTUP_RENDER_ROUTE_ENTRANCE_OPENING_DELAY_PC34 = 5,
    CSB_V1_BOOT_STARTUP_RENDER_ROUTE_ENTRANCE_OPENING_FRAME_PC34 = 6
} CSB_V1_BootStartupRenderRouteKind_PC34;

#define CSB_V1_BOOT_STARTUP_HUD_PROMPT_CAP_PC34 192
#define CSB_V1_BOOT_STARTUP_RESUME_PATH_CAP_PC34 512

typedef enum CSB_V1_BootStartupHudMenuKind_PC34 {
    CSB_V1_BOOT_STARTUP_HUD_MENU_NONE_PC34 = 0,
    CSB_V1_BOOT_STARTUP_HUD_MENU_ENTRANCE_PC34 = 1,
    CSB_V1_BOOT_STARTUP_HUD_MENU_UTILITY_PC34 = 2
} CSB_V1_BootStartupHudMenuKind_PC34;

typedef struct CSB_V1_BootStartupHudMenuStateReceipt_PC34 {
    int valid;
    CSB_V1_BootStartupHudMenuKind_PC34 kind;
    int option_count;
    int selected_command_id;
    int resume_enabled;
    int resume_available;
    int resume_option_visible;
    int resume_option_selected;
    char resume_path[CSB_V1_BOOT_STARTUP_RESUME_PATH_CAP_PC34];
    int utility_selected_action_index;
    int utility_preview_active;
    int utility_menu_row_count;
    char prompt[CSB_V1_BOOT_STARTUP_HUD_PROMPT_CAP_PC34];
} CSB_V1_BootStartupHudMenuStateReceipt_PC34;

typedef struct CSB_V1_BootStartupPresentationRouteReceipt_PC34 {
    int valid;
    CSB_V1_BootStartupRenderRouteKind_PC34 route;
    int special_palette;
    int draw_title;
    int draw_surface;
    int draw_closed_doors;
    int draw_opening_frame;
    int draw_fallback_text;
    int draw_utility_panel;
    int hud_menu_visible;
    int menu_option_count;
    int utility_plan_valid;
    CSB_V1_UtilRenderPlan utility_plan;
    CSB_V1_BootStartupHudMenuStateReceipt_PC34 hud_menu_state;
    int accepts_input;
    int waiting_for_input;
    CSB_V1_StartupPresentationReceipt_PC34 presentation;
} CSB_V1_BootStartupPresentationRouteReceipt_PC34;

typedef struct CSB_V1_BootStartupRenderViewReceipt_PC34 {
    int valid;
    int render_plan_valid;
    int boot_executor_route;
    int title_after_swoosh_route;
    int title_stage;
    int title_source_step;
    int title_frame;
    int title_frame_max;
    int title_presents_visible;
    int title_chaos_visible;
    int title_chaos_zoom_visible;
    int title_chaos_hold_visible;
    int title_strikes_back_visible;
    int title_phase_tick;
    int title_phase_tick_count;
    int title_render_command_count;
    int title_blit_kind;
    int title_transparent_color;
    int title_special_palette;
    int title_source_x;
    int title_source_y;
    int title_source_w;
    int title_source_h;
    int title_dest_x;
    int title_dest_y;
    int title_dest_w;
    int title_dest_h;
    int closed_door_menu_route;
    int closed_door_render_command_count;
    int closed_door_asset_command_count;
    int closed_door_menu_option_count;
    int closed_door_selected_command_id;
    int closed_door_resume_enabled;
    int closed_door_resume_available;
    int closed_door_resume_option_visible;
    int closed_door_resume_option_selected;
    char closed_door_prompt[CSB_V1_BOOT_STARTUP_HUD_PROMPT_CAP_PC34];
    int utility_menu_route;
    int utility_menu_row_count;
    int utility_selected_action_index;
    int utility_preview_active;
    char utility_prompt[CSB_V1_BOOT_STARTUP_HUD_PROMPT_CAP_PC34];
    int opening_door_route;
    int hud_menu_receipt_ready;
    int suppress_legacy_utility_fallback;
    CSB_V1_StartupRenderPlan_PC34 render_plan;
    CSB_V1_BootStartupPresentationRouteReceipt_PC34 route_receipt;
} CSB_V1_BootStartupRenderViewReceipt_PC34;

typedef struct CSB_V1_BootStartupReadinessReceipt_PC34 {
    int valid;
    int startup_active;
    CSB_V1_BootStartupRenderRouteKind_PC34 route;
    int post_ftl_title_active;
    int title_ready;
    int title_frame;
    int title_frame_max;
    int title_stage;
    int title_presents_visible;
    int title_chaos_visible;
    int title_chaos_zoom_visible;
    int title_chaos_hold_visible;
    int title_strikes_back_visible;
    int title_phase_tick;
    int title_phase_tick_count;
    int input_ready;
    int hud_menu_ready;
    int host_input_blocked;
    int host_startup_input_ready;
    int host_runtime_input_ready;
    int host_hud_blocked;
    int host_startup_hud_ready;
    int host_runtime_hud_ready;
    CSB_V1_BootStartupHudMenuKind_PC34 hud_menu_kind;
    int hud_menu_option_count;
    int utility_menu_row_count;
    int selected_command_id;
    int selected_utility_action_index;
    int resume_available;
    int suppress_legacy_utility_fallback;
    int runtime_handoff_ready;
    int runtime_viewport_ready;
    int runtime_hud_ready;
    int runtime_level_loaded;
    int runtime_map_index;
    int runtime_party_x;
    int runtime_party_y;
    int runtime_party_dir;
    int runtime_champion_count;
    int runtime_tick_count;
    char animation[CSB_V1_STARTUP_ANIMATION_CAP_PC34];
} CSB_V1_BootStartupReadinessReceipt_PC34;

typedef enum CSB_V1_BootStartupActionKind_PC34 {
    CSB_V1_BOOT_STARTUP_ACTION_NONE_PC34 = 0,
    CSB_V1_BOOT_STARTUP_ACTION_UTILITY_PC34 = 1,
    CSB_V1_BOOT_STARTUP_ACTION_ENTRANCE_PC34 = 2
} CSB_V1_BootStartupActionKind_PC34;

typedef struct CSB_V1_BootStartupActionReceipt_PC34 {
    CSB_V1_BootStartupActionKind_PC34 kind;
    int handled;
    int menu_input;
    int input_is_pointer;
    int pointer_x;
    int pointer_y;
    unsigned int pointer_button_mask;
    int pointer_left_button;
    CSB_V1_StartupInput_PC34 startup_input;
    int entrance_command_id;
    int input_blocked_by_title;
    int input_routed_to_utility;
    int input_routed_to_entrance;
    int host_receipt_valid;
    CSB_V1_StartupEntranceInputResult_PC34 host_input_result;
    const char *host_status_scope;
    const char *host_status;
    int host_clear_import_preview;
    int host_bonus_requested_changed;
    int host_bonus_requested;
    CSB_V1_BootStartupPresentationRouteReceipt_PC34 pre_input_route;
    int pre_input_render_view_valid;
    CSB_V1_BootStartupRenderViewReceipt_PC34 pre_input_render_view;
    int post_input_render_view_valid;
    CSB_V1_BootStartupRenderViewReceipt_PC34 post_input_render_view;
    int input_stays_on_startup;
    int input_requests_launcher_return;
    CSB_V1_RuntimeUtilStartupHostActionReceipt_PC34 utility_receipt;
    CSB_V1_StartupEntranceHostActionReceipt_PC34 entrance_receipt;
} CSB_V1_BootStartupActionReceipt_PC34;

typedef struct CSB_V1_BootStartupHostDecisionReceipt_PC34 {
    int valid;
    int menu_input;
    int input_is_pointer;
    int pointer_left_button;
    int consumed_input;
    int blocked_by_title;
    int routed_to_utility;
    int routed_to_entrance;
    int redraw_startup;
    int stays_on_startup;
    int return_to_launcher;
    int clear_import_preview;
    int bonus_requested_changed;
    int bonus_requested;
    int utility_selected_action_index;
    int entrance_command_id;
    CSB_V1_StartupEntranceInputResult_PC34 host_input_result;
    CSB_V1_BootStartupRenderRouteKind_PC34 pre_render_route;
    CSB_V1_BootStartupRenderRouteKind_PC34 post_render_route;
    const char *status_scope;
    const char *status;
} CSB_V1_BootStartupHostDecisionReceipt_PC34;

typedef struct CSB_V1_BootStartupHudMenuDrawReceipt_PC34 {
    int valid;
    CSB_V1_BootStartupHudMenuKind_PC34 kind;
    CSB_V1_BootStartupRenderRouteKind_PC34 route;
    int from_post_input_render_view;
    int startup_render_plan_valid;
    int utility_render_plan_valid;
    int draw_closed_doors;
    int draw_utility_panel;
    int draw_fallback_text;
    int suppress_legacy_utility_fallback;
    int option_count;
    int selected_command_id;
    int selected_utility_action_index;
    int resume_enabled;
    int resume_available;
    int resume_option_visible;
    int resume_option_selected;
    char prompt[CSB_V1_BOOT_STARTUP_HUD_PROMPT_CAP_PC34];
    CSB_V1_StartupRenderPlan_PC34 startup_render_plan;
    CSB_V1_UtilRenderPlan utility_render_plan;
    int host_decision_valid;
    CSB_V1_BootStartupHostDecisionReceipt_PC34 host_decision;
} CSB_V1_BootStartupHudMenuDrawReceipt_PC34;

typedef struct CSB_V1_BootStartupInputRenderReceipt_PC34 {
    int valid;
    int action_valid;
    CSB_V1_BootStartupActionReceipt_PC34 action;
    int host_decision_valid;
    CSB_V1_BootStartupHostDecisionReceipt_PC34 host_decision;
    int pre_input_readiness_valid;
    CSB_V1_BootStartupReadinessReceipt_PC34 pre_input_readiness;
    int post_input_readiness_valid;
    CSB_V1_BootStartupReadinessReceipt_PC34 post_input_readiness;
    int hud_menu_draw_valid;
    CSB_V1_BootStartupHudMenuDrawReceipt_PC34 hud_menu_draw;
    int draw_from_post_input;
    int input_consumed;
    int startup_redraw;
    int startup_hud_draw_ready;
    int runtime_handoff_ready;
    int return_to_launcher;
} CSB_V1_BootStartupInputRenderReceipt_PC34;

typedef struct CSB_V1_BootStartupInputGateReceipt_PC34 {
    int valid;
    int input_is_pointer;
    int pointer_left_button;
    int pointer_button_relevant;
    int startup_active;
    int startup_input_ready;
    int host_input_blocked;
    int should_dispatch_input;
    int should_ignore_input;
    int input_render_valid;
    CSB_V1_BootStartupReadinessReceipt_PC34 readiness;
    CSB_V1_BootStartupInputRenderReceipt_PC34 input_render;
} CSB_V1_BootStartupInputGateReceipt_PC34;

typedef struct CSB_V1_BootStartupCaptureReceipt_PC34 {
    int valid;
    int route_valid;
    CSB_V1_BootStartupPresentationRouteReceipt_PC34 route;
    int render_view_valid;
    CSB_V1_BootStartupRenderViewReceipt_PC34 render_view;
    int readiness_valid;
    CSB_V1_BootStartupReadinessReceipt_PC34 readiness;
    int hud_menu_draw_valid;
    CSB_V1_BootStartupHudMenuDrawReceipt_PC34 hud_menu_draw;
    int real_asset_receipt_valid;
    CSB_V1_StartupRealReceipt real_asset_receipt;
    int title_capture_ready;
    int hud_menu_capture_ready;
    int runtime_capture_ready;
    int host_input_blocked;
    int host_hud_blocked;
    int startup_input_ready;
    int startup_hud_ready;
    int render_route;
    int hud_menu_kind;
    int title_stage;
    int title_frame;
    int title_source_step;
    int selected_command_id;
    int selected_utility_action_index;
    int suppress_legacy_utility_fallback;
} CSB_V1_BootStartupCaptureReceipt_PC34;

typedef struct CSB_V1_BootStartupPackagedCaptureProof_PC34 {
    int valid;
    int capture_valid;
    int real_asset_matched;
    uint64_t real_asset_receipt_hash;
    uint32_t packaged_capture_hash;
    int route;
    int hud_menu_kind;
    int title_capture_ready;
    int hud_menu_capture_ready;
    int runtime_capture_ready;
    int render_plan_available;
    int hud_menu_draw_available;
    int boot_executor_route;
    int title_route;
    int closed_door_menu_route;
    int utility_menu_route;
    int credits_route;
    int opening_door_route;
    int draw_closed_doors;
    int draw_utility_panel;
    int draw_fallback_text;
    int draw_opening_frame;
    int host_input_blocked;
    int host_hud_blocked;
    int startup_input_ready;
    int startup_hud_ready;
    int hud_menu_option_count;
    int utility_menu_row_count;
    int resume_available;
    int resume_option_visible;
    int suppress_legacy_utility_fallback;
    int title_stage;
    int title_frame;
    int selected_command_id;
    int selected_utility_action_index;
    const char *source_evidence;
} CSB_V1_BootStartupPackagedCaptureProof_PC34;

#define CSB_V1_BOOT_STARTUP_TITLE_SAMPLE_COUNT_PC34 4

typedef struct CSB_V1_BootStartupVisualSequenceCaptureReceipt_PC34 {
    int valid;
    int real_asset_matched;
    uint32_t sequence_capture_hash;
    int title_sample_count;
    int title_all_stages_captured;
    int title_presents_capture_ready;
    int title_chaos_zoom_capture_ready;
    int title_chaos_hold_capture_ready;
    int title_strikes_back_capture_ready;
    int closed_door_hud_capture_ready;
    int utility_hud_capture_ready;
    int door_opening_delay_capture_ready;
    int door_opening_frame_capture_ready;
    int credits_capture_ready;
    int no_fallback_text_routes;
    int no_legacy_door_fallback_routes;
    int hud_menu_draw_available;
    int opening_frame_draw_available;
    int source_title_presents_ticks;
    int source_title_chaos_zoom_ticks;
    int source_title_chaos_hold_ticks;
    int source_title_strikes_back_ticks;
    int source_door_pre_open_delay_ticks;
    int source_door_step_count;
    uint32_t title_sample_hashes[CSB_V1_BOOT_STARTUP_TITLE_SAMPLE_COUNT_PC34];
    uint32_t closed_door_hud_hash;
    uint32_t utility_hud_hash;
    uint32_t door_opening_delay_hash;
    uint32_t door_opening_frame_hash;
    uint32_t credits_hash;
    const char *source_evidence;
} CSB_V1_BootStartupVisualSequenceCaptureReceipt_PC34;

typedef struct CSB_V1_BootStartupRuntimeVisualCaptureReceipt_PC34 {
    int valid;
    int visual_sequence_valid;
    int real_asset_matched;
    int title_runtime_consumed;
    int title_runtime_sample_count;
    int title_runtime_all_stages_consumed;
    int closed_door_hud_runtime_consumed;
    int utility_hud_runtime_consumed;
    int door_opening_delay_runtime_consumed;
    int door_opening_frame_runtime_consumed;
    int credits_runtime_consumed;
    int title_draw_consumed;
    int closed_door_hud_draw_consumed;
    int utility_hud_draw_consumed;
    int door_opening_frame_draw_consumed;
    int credits_surface_draw_consumed;
    int no_fallback_callbacks;
    int no_wrapper_fallback_routes;
    int draw_consumes_receipt_only;
    int input_consumes_receipt_only;
    uint32_t sequence_capture_hash;
    uint32_t runtime_capture_hash;
    uint32_t title_runtime_sample_hashes[CSB_V1_BOOT_STARTUP_TITLE_SAMPLE_COUNT_PC34];
    CSB_V1_BootStartupVisualSequenceCaptureReceipt_PC34 visual_sequence;
    const char *source_evidence;
} CSB_V1_BootStartupRuntimeVisualCaptureReceipt_PC34;

typedef struct CSB_V1_BootStartupRuntimeRouteHardeningReceipt_PC34 {
    int valid;
    int visual_sequence_valid;
    int host_ownership_valid;
    int real_asset_matched;
    int route_covered_by_full_capture;
    int title_route_covered;
    int closed_door_hud_route_covered;
    int utility_hud_route_covered;
    int door_opening_route_covered;
    int credits_route_covered;
    int no_fallback_text_route;
    int no_legacy_door_fallback_route;
    int host_draw_consumes_receipt_only;
    int input_consumes_receipt_only;
    uint32_t route_hardening_hash;
    const char *source_evidence;
} CSB_V1_BootStartupRuntimeRouteHardeningReceipt_PC34;

typedef struct CSB_V1_BootStartupRuntimeHostCaptureGateReceipt_PC34 {
    int valid;
    int runtime_visual_valid;
    int visual_sequence_valid;
    int route_hardening_valid;
    int all_runtime_routes_consumed;
    int title_runtime_captured;
    int closed_door_hud_runtime_captured;
    int utility_hud_runtime_captured;
    int door_opening_runtime_captured;
    int credits_runtime_captured;
    int draw_consumes_receipt_only;
    int input_consumes_receipt_only;
    int no_fallback_callbacks;
    int no_wrapper_fallback_routes;
    int title_host_ownership_valid;
    int closed_door_host_ownership_valid;
    int utility_host_ownership_valid;
    int door_opening_host_ownership_valid;
    int title_host_draw_consumes_receipt_only;
    int closed_door_host_draw_consumes_receipt_only;
    int utility_host_draw_consumes_receipt_only;
    int door_opening_host_draw_consumes_receipt_only;
    int title_host_input_consumes_receipt_only;
    int closed_door_host_input_consumes_receipt_only;
    int utility_host_input_consumes_receipt_only;
    int door_opening_host_input_consumes_receipt_only;
    uint32_t title_packaged_capture_hash;
    uint32_t closed_door_packaged_capture_hash;
    uint32_t utility_packaged_capture_hash;
    uint32_t door_opening_packaged_capture_hash;
    uint32_t sequence_capture_hash;
    uint32_t runtime_capture_hash;
    uint32_t route_hardening_hash;
    uint32_t runtime_host_gate_hash;
    CSB_V1_BootStartupRuntimeVisualCaptureReceipt_PC34 runtime_visual;
    CSB_V1_BootStartupRuntimeRouteHardeningReceipt_PC34 title_route_hardening;
    CSB_V1_BootStartupRuntimeRouteHardeningReceipt_PC34 closed_door_route_hardening;
    CSB_V1_BootStartupRuntimeRouteHardeningReceipt_PC34 utility_route_hardening;
    CSB_V1_BootStartupRuntimeRouteHardeningReceipt_PC34 door_opening_route_hardening;
    const char *source_evidence;
} CSB_V1_BootStartupRuntimeHostCaptureGateReceipt_PC34;

typedef struct CSB_V1_BootStartupRenderDrawReceipt_PC34 {
    int valid;
    int route;
    int surface;
    int render_plan_valid;
    int title_draw_ready;
    int hud_menu_draw_ready;
    int opening_draw_ready;
    int real_asset_matched;
    CSB_V1_StartupRenderPlan_PC34 render_plan;
} CSB_V1_BootStartupRenderDrawReceipt_PC34;

typedef struct CSB_V1_BootStartupHostViewReceipt_PC34 {
    int valid;
    int startup_active;
    char phase[64];
    char animation[CSB_V1_STARTUP_ANIMATION_CAP_PC34];
    int animation_active;
    int startup_frame;
    int title_frame;
    int title_frame_max;
    int title_ready;
    int route;
    int special_palette;
    int startup_input_ready;
    int startup_hud_menu_ready;
    int startup_hud_runtime_ready;
    CSB_V1_BootStartupHudMenuKind_PC34 hud_menu_kind;
    int hud_menu_option_count;
    int selected_command_id;
    int selected_utility_action_index;
    int runtime_handoff_ready;
    int runtime_level_loaded;
    int runtime_map_index;
    int runtime_party_x;
    int runtime_party_y;
    int runtime_party_dir;
    int runtime_champion_count;
    int runtime_tick_count;
    int readiness_valid;
    CSB_V1_BootStartupReadinessReceipt_PC34 readiness;
    int render_plan_valid;
    CSB_V1_StartupRenderPlan_PC34 render_plan;
    int render_draw_valid;
    CSB_V1_BootStartupRenderDrawReceipt_PC34 render_draw;
    int hud_menu_draw_valid;
    CSB_V1_BootStartupHudMenuDrawReceipt_PC34 hud_menu_draw;
    int capture_proof_valid;
    CSB_V1_BootStartupPackagedCaptureProof_PC34 capture_proof;
} CSB_V1_BootStartupHostViewReceipt_PC34;

typedef struct CSB_V1_BootStartupHostViewDrawReceipt_PC34 {
    int valid;
    int host_view_valid;
    int render_draw_valid;
    int hud_menu_draw_valid;
    int render_executed;
    int hud_menu_executed;
    int route;
    int surface;
    CSB_V1_BootStartupHudMenuKind_PC34 hud_menu_kind;
    int real_asset_matched;
    int suppress_legacy_utility_fallback;
    int title_asset_draw_ready;
    int closed_door_asset_draw_ready;
    int opening_frame_draw_ready;
    int fallback_text_suppressed;
    int fallback_callbacks_stripped;
    int consumed_host_view_only;
} CSB_V1_BootStartupHostViewDrawReceipt_PC34;

typedef struct CSB_V1_BootStartupHostInputDispatchReceipt_PC34 {
    int valid;
    int input_is_pointer;
    int pointer_button_relevant;
    int startup_active;
    int startup_input_ready;
    int host_input_blocked;
    int should_dispatch_input;
    int should_ignore_input;
    int input_render_valid;
    CSB_V1_BootStartupInputRenderReceipt_PC34 input_render;
} CSB_V1_BootStartupHostInputDispatchReceipt_PC34;

typedef struct CSB_V1_BootStartupHostOwnershipReceipt_PC34 {
    int valid;
    int snapshot_capture_valid;
    int host_view_valid;
    int host_draw_valid;
    int host_input_dispatch_valid;
    int capture_proof_valid;
    int packaged_visual_capture_ready;
    int real_asset_matched;
    uint32_t packaged_capture_hash;
    int route;
    int hud_menu_kind;
    int title_capture_ready;
    int hud_menu_capture_ready;
    int runtime_capture_ready;
    int title_draw_ready;
    int closed_door_menu_draw_ready;
    int utility_menu_draw_ready;
    int opening_draw_ready;
    int render_executed;
    int hud_menu_executed;
    int draw_consumes_receipt_only;
    int input_consumes_receipt_only;
    int host_input_blocked;
    int startup_input_ready;
    int should_dispatch_input;
    int should_ignore_input;
    int input_redraws_hud_menu;
    int suppress_legacy_utility_fallback;
    CSB_V1_BootStartupHostViewReceipt_PC34 host_view;
    CSB_V1_BootStartupHostViewDrawReceipt_PC34 host_draw;
    CSB_V1_BootStartupHostInputDispatchReceipt_PC34 host_input;
} CSB_V1_BootStartupHostOwnershipReceipt_PC34;

void csb_v1_boot_startup_action_receipt_init_pc34(
    CSB_V1_BootStartupActionReceipt_PC34 *receipt);
void csb_v1_boot_startup_host_decision_receipt_init_pc34(
    CSB_V1_BootStartupHostDecisionReceipt_PC34 *receipt);
void csb_v1_boot_startup_hud_menu_draw_receipt_init_pc34(
    CSB_V1_BootStartupHudMenuDrawReceipt_PC34 *receipt);
void csb_v1_boot_startup_input_render_receipt_init_pc34(
    CSB_V1_BootStartupInputRenderReceipt_PC34 *receipt);
void csb_v1_boot_startup_input_gate_receipt_init_pc34(
    CSB_V1_BootStartupInputGateReceipt_PC34 *receipt);
void csb_v1_boot_startup_capture_receipt_init_pc34(
    CSB_V1_BootStartupCaptureReceipt_PC34 *receipt);
void csb_v1_boot_startup_packaged_capture_proof_init_pc34(
    CSB_V1_BootStartupPackagedCaptureProof_PC34 *proof);
void csb_v1_boot_startup_visual_sequence_capture_receipt_init_pc34(
    CSB_V1_BootStartupVisualSequenceCaptureReceipt_PC34 *receipt);
void csb_v1_boot_startup_runtime_visual_capture_receipt_init_pc34(
    CSB_V1_BootStartupRuntimeVisualCaptureReceipt_PC34 *receipt);
void csb_v1_boot_startup_runtime_route_hardening_receipt_init_pc34(
    CSB_V1_BootStartupRuntimeRouteHardeningReceipt_PC34 *receipt);
void csb_v1_boot_startup_runtime_host_capture_gate_receipt_init_pc34(
    CSB_V1_BootStartupRuntimeHostCaptureGateReceipt_PC34 *receipt);
void csb_v1_boot_startup_render_draw_receipt_init_pc34(
    CSB_V1_BootStartupRenderDrawReceipt_PC34 *receipt);
void csb_v1_boot_startup_host_view_receipt_init_pc34(
    CSB_V1_BootStartupHostViewReceipt_PC34 *receipt);
void csb_v1_boot_startup_host_view_draw_receipt_init_pc34(
    CSB_V1_BootStartupHostViewDrawReceipt_PC34 *receipt);
void csb_v1_boot_startup_host_input_dispatch_receipt_init_pc34(
    CSB_V1_BootStartupHostInputDispatchReceipt_PC34 *receipt);
void csb_v1_boot_startup_host_ownership_receipt_init_pc34(
    CSB_V1_BootStartupHostOwnershipReceipt_PC34 *receipt);
void csb_v1_boot_startup_presentation_route_receipt_init_pc34(
    CSB_V1_BootStartupPresentationRouteReceipt_PC34 *receipt);
void csb_v1_boot_startup_render_view_receipt_init_pc34(
    CSB_V1_BootStartupRenderViewReceipt_PC34 *receipt);
void csb_v1_boot_startup_readiness_receipt_init_pc34(
    CSB_V1_BootStartupReadinessReceipt_PC34 *receipt);
void csb_v1_boot_profile_init(CSB_V1_BootProfile *profile);
int csb_v1_boot_scan_assets(CSB_V1_BootProfile *profile, const char *data_dir);
int csb_v1_boot_probe_available(const char *data_dir);
void csb_v1_boot_set_save_root(CSB_V1_BootProfile *profile, const char *save_dir);
int csb_v1_boot_set_imported_party(CSB_V1_BootProfile *profile,
                                   const CSB_V1_PartyState *party);
int csb_v1_boot_set_imported_party_from_cmp(CSB_V1_BootProfile *profile,
                                            const uint8_t *cmp_buf,
                                            size_t cmp_size);
int csb_v1_boot_mark_imported_party_ready(CSB_V1_BootProfile *profile);
void csb_v1_boot_reset_engine_version_to_dm1(void);
int csb_v1_boot_scan_csbgraphics(CSB_V1_BootProfile *profile,
                                 const char *cache_dir);
const CSB_V1_CSBGraphicsM11RuntimePlan *
csb_v1_boot_csbgraphics_m11_plan(const CSB_V1_BootProfile *profile);
const CSB_V1_CSBGraphicsDatRealCache *
csb_v1_boot_csbgraphics_cache(const CSB_V1_BootProfile *profile);
const uint16_t *
csb_v1_boot_csbgraphics_skin_def_words(const CSB_V1_BootProfile *profile,
                                       size_t *out_word_count);
int csb_v1_boot_render_viewport_frame_pc34(
    void *boot_profile,
    unsigned char *framebuffer,
    int framebuffer_width,
    int framebuffer_height,
    const CSB_V1_ViewportRuntimeDrawerBinding *drawer_binding,
    CSB_V1_ViewportRuntimeDrawCounts *out_counts);
int csb_v1_boot_apply_startup_handoff_pc34(
    CSB_V1_BootProfile *profile,
    const char *save_path,
    const char *import_dm1_save_path,
    CSB_V1_RuntimeStartupHandoffReceipt_PC34 *out_receipt);
int csb_v1_boot_build_startup_session_state_receipt_pc34(
    const CSB_V1_BootProfile *profile,
    const CSB_V1_RuntimeStartupHandoffReceipt_PC34 *handoff,
    const char *import_dm1_save_path,
    const char *resume_save_path,
    CSB_V1_RuntimeStartupSessionStateReceipt_PC34 *out_receipt);
int csb_v1_boot_build_startup_launch_receipts_pc34(
    CSB_V1_BootProfile *profile,
    const char *save_path,
    const char *import_dm1_save_path,
    const char *resume_save_path,
    CSB_V1_BootStartupLaunchReceipts_PC34 *out_receipts);
int csb_v1_boot_startup_launch_alloc_pc34(
    const char *data_dir,
    const char *save_path,
    const char *import_dm1_save_path,
    const char *resume_save_path,
    CSB_V1_BootStartupLaunch_PC34 *out_launch);
int csb_v1_boot_startup_launch_detach_runtime_pc34(
    CSB_V1_BootStartupLaunch_PC34 *launch,
    CSB_V1_BootStartupRuntimeReceipt_PC34 *out_receipt);
void csb_v1_boot_startup_launch_cleanup_pc34(
    CSB_V1_BootStartupLaunch_PC34 *launch);
int csb_v1_boot_startup_advance_idle_from_snapshot_pc34(
    const CSB_V1_BootRuntimeStartupSnapshot_PC34 *snapshot,
    CSB_V1_StartupIdleReceipt_PC34 *out_receipt);
int csb_v1_boot_startup_entrance_accepts_input_from_snapshot_pc34(
    const CSB_V1_BootRuntimeStartupSnapshot_PC34 *snapshot);
int csb_v1_boot_startup_presentation_receipt_from_snapshot_pc34(
    const CSB_V1_BootRuntimeStartupSnapshot_PC34 *snapshot,
    char *out_phase,
    int out_phase_size,
    int *out_startup_active,
    int *out_startup_frame,
    char *out_animation,
    int out_animation_size,
    int *out_animation_active,
    int *out_title_frame,
    int *out_title_frame_max,
    int *out_title_ready);
int csb_v1_boot_startup_presentation_state_receipt_from_snapshot_pc34(
    const CSB_V1_BootRuntimeStartupSnapshot_PC34 *snapshot,
    CSB_V1_StartupPresentationReceipt_PC34 *out_receipt);
int csb_v1_boot_startup_presentation_route_receipt_from_snapshot_pc34(
    const CSB_V1_BootRuntimeStartupSnapshot_PC34 *snapshot,
    CSB_V1_BootStartupPresentationRouteReceipt_PC34 *out_receipt);
int csb_v1_boot_startup_render_view_receipt_from_snapshot_pc34(
    const CSB_V1_BootRuntimeStartupSnapshot_PC34 *snapshot,
    CSB_V1_BootStartupRenderViewReceipt_PC34 *out_receipt);
int csb_v1_boot_startup_readiness_receipt_from_view_pc34(
    const CSB_V1_BootStartupRenderViewReceipt_PC34 *view,
    CSB_V1_BootStartupReadinessReceipt_PC34 *out_receipt);
int csb_v1_boot_startup_readiness_receipt_from_snapshot_pc34(
    const CSB_V1_BootRuntimeStartupSnapshot_PC34 *snapshot,
    CSB_V1_BootStartupReadinessReceipt_PC34 *out_receipt);
int csb_v1_boot_startup_capture_receipt_from_snapshot_pc34(
    const CSB_V1_BootRuntimeStartupSnapshot_PC34 *snapshot,
    CSB_V1_BootStartupCaptureReceipt_PC34 *out_receipt);
int csb_v1_boot_startup_hud_menu_draw_receipt_from_view_pc34(
    const CSB_V1_BootStartupRenderViewReceipt_PC34 *view,
    CSB_V1_BootStartupHudMenuDrawReceipt_PC34 *out_receipt);
int csb_v1_boot_startup_hud_menu_draw_receipt_from_action_pc34(
    const CSB_V1_BootStartupActionReceipt_PC34 *action,
    int prefer_post_input_render_view,
    CSB_V1_BootStartupHudMenuDrawReceipt_PC34 *out_receipt);
int csb_v1_boot_startup_execute_hud_menu_draw_receipt_pc34(
    const CSB_V1_BootStartupHudMenuDrawReceipt_PC34 *draw_receipt,
    const CSB_V1_BootStartupReadinessReceipt_PC34 *readiness_receipt,
    const CSB_V1_StartupRenderExecutor_PC34 *executor);
int csb_v1_boot_startup_packaged_capture_proof_from_capture_pc34(
    const CSB_V1_BootStartupCaptureReceipt_PC34 *capture_receipt,
    CSB_V1_BootStartupPackagedCaptureProof_PC34 *out_proof);
int csb_v1_boot_startup_packaged_capture_proof_from_snapshot_pc34(
    const CSB_V1_BootRuntimeStartupSnapshot_PC34 *snapshot,
    CSB_V1_BootStartupPackagedCaptureProof_PC34 *out_proof);
int csb_v1_boot_startup_visual_sequence_capture_receipt_from_profile_pc34(
    const CSB_V1_BootProfile *boot_profile,
    CSB_V1_BootStartupVisualSequenceCaptureReceipt_PC34 *out_receipt);
int csb_v1_boot_startup_runtime_visual_capture_receipt_from_profile_pc34(
    const CSB_V1_BootProfile *boot_profile,
    const CSB_V1_StartupRenderExecutor_PC34 *executor,
    CSB_V1_BootStartupRuntimeVisualCaptureReceipt_PC34 *out_receipt);
int csb_v1_boot_startup_runtime_route_hardening_receipt_from_ownership_pc34(
    const CSB_V1_BootStartupVisualSequenceCaptureReceipt_PC34 *visual_sequence,
    const CSB_V1_BootStartupHostOwnershipReceipt_PC34 *ownership,
    CSB_V1_BootStartupRuntimeRouteHardeningReceipt_PC34 *out_receipt);
int csb_v1_boot_startup_runtime_host_capture_gate_receipt_from_profile_pc34(
    const CSB_V1_BootProfile *boot_profile,
    const CSB_V1_StartupRenderExecutor_PC34 *executor,
    CSB_V1_BootStartupRuntimeHostCaptureGateReceipt_PC34 *out_receipt);
int csb_v1_boot_startup_host_view_receipt_from_capture_pc34(
    const CSB_V1_BootStartupCaptureReceipt_PC34 *capture_receipt,
    CSB_V1_BootStartupHostViewReceipt_PC34 *out_receipt);
int csb_v1_boot_startup_host_view_receipt_from_snapshot_pc34(
    const CSB_V1_BootRuntimeStartupSnapshot_PC34 *snapshot,
    CSB_V1_BootStartupHostViewReceipt_PC34 *out_receipt);
int csb_v1_boot_startup_host_input_dispatch_firestaff_from_snapshot_pc34(
    const CSB_V1_BootRuntimeStartupSnapshot_PC34 *snapshot,
    int menu_input,
    CSB_V1_BootStartupHostInputDispatchReceipt_PC34 *out_receipt);
int csb_v1_boot_startup_host_input_dispatch_pointer_from_snapshot_pc34(
    const CSB_V1_BootRuntimeStartupSnapshot_PC34 *snapshot,
    int x,
    int y,
    unsigned int button_mask,
    CSB_V1_BootStartupHostInputDispatchReceipt_PC34 *out_receipt);
int csb_v1_boot_startup_execute_host_view_receipt_pc34(
    const CSB_V1_BootStartupHostViewReceipt_PC34 *host_view,
    const CSB_V1_StartupRenderExecutor_PC34 *executor,
    CSB_V1_BootStartupHostViewDrawReceipt_PC34 *out_receipt);
int csb_v1_boot_startup_execute_host_ownership_receipt_from_snapshot_pc34(
    const CSB_V1_BootRuntimeStartupSnapshot_PC34 *snapshot,
    int include_menu_input,
    int menu_input,
    const CSB_V1_StartupRenderExecutor_PC34 *executor,
    CSB_V1_BootStartupHostOwnershipReceipt_PC34 *out_receipt);
int csb_v1_boot_runtime_util_apply_pointer_from_snapshot_pc34(
    const CSB_V1_BootRuntimeStartupSnapshot_PC34 *snapshot,
    int x,
    int y,
    CSB_V1_RuntimeUtilStartupHostActionReceipt_PC34 *out_receipt);
int csb_v1_boot_runtime_util_apply_firestaff_input_from_snapshot_pc34(
    const CSB_V1_BootRuntimeStartupSnapshot_PC34 *snapshot,
    int menu_input,
    CSB_V1_RuntimeUtilStartupHostActionReceipt_PC34 *out_receipt);
int csb_v1_boot_runtime_execute_startup_entrance_firestaff_input_from_snapshot_pc34(
    const CSB_V1_BootRuntimeStartupSnapshot_PC34 *snapshot,
    int menu_input,
    CSB_V1_StartupEntranceHostActionReceipt_PC34 *out_receipt);
int csb_v1_boot_runtime_execute_startup_entrance_pointer_from_snapshot_pc34(
    const CSB_V1_BootRuntimeStartupSnapshot_PC34 *snapshot,
    int x,
    int y,
    unsigned int button_mask,
    CSB_V1_StartupEntranceHostActionReceipt_PC34 *out_receipt);
int csb_v1_boot_runtime_execute_startup_firestaff_input_from_snapshot_pc34(
    const CSB_V1_BootRuntimeStartupSnapshot_PC34 *snapshot,
    int menu_input,
    CSB_V1_BootStartupActionReceipt_PC34 *out_receipt);
int csb_v1_boot_runtime_execute_startup_firestaff_input_render_from_snapshot_pc34(
    const CSB_V1_BootRuntimeStartupSnapshot_PC34 *snapshot,
    int menu_input,
    CSB_V1_BootStartupInputRenderReceipt_PC34 *out_receipt);
int csb_v1_boot_runtime_execute_startup_firestaff_input_gate_from_snapshot_pc34(
    const CSB_V1_BootRuntimeStartupSnapshot_PC34 *snapshot,
    int menu_input,
    CSB_V1_BootStartupInputGateReceipt_PC34 *out_receipt);
int csb_v1_boot_startup_host_decision_from_action_receipt_pc34(
    const CSB_V1_BootStartupActionReceipt_PC34 *receipt,
    CSB_V1_BootStartupHostDecisionReceipt_PC34 *out_decision);
int csb_v1_boot_runtime_execute_startup_pointer_from_snapshot_pc34(
    const CSB_V1_BootRuntimeStartupSnapshot_PC34 *snapshot,
    int x,
    int y,
    unsigned int button_mask,
    CSB_V1_BootStartupActionReceipt_PC34 *out_receipt);
int csb_v1_boot_runtime_execute_startup_pointer_render_from_snapshot_pc34(
    const CSB_V1_BootRuntimeStartupSnapshot_PC34 *snapshot,
    int x,
    int y,
    unsigned int button_mask,
    CSB_V1_BootStartupInputRenderReceipt_PC34 *out_receipt);
int csb_v1_boot_runtime_execute_startup_pointer_gate_from_snapshot_pc34(
    const CSB_V1_BootRuntimeStartupSnapshot_PC34 *snapshot,
    int x,
    int y,
    unsigned int button_mask,
    CSB_V1_BootStartupInputGateReceipt_PC34 *out_receipt);
int csb_v1_boot_runtime_save_game_to_path_pc34(
    const CSB_V1_BootProfile *profile,
    const char *path,
    uint32_t *out_game_time);
int csb_v1_boot_runtime_load_game_from_path_pc34(
    CSB_V1_BootProfile *profile,
    const char *path,
    uint32_t *out_game_time);
int csb_v1_boot_runtime_tick_pc34(
    CSB_V1_BootProfile *profile,
    uint32_t *out_game_time);
int csb_v1_boot_runtime_object_icon_index_pc34(
    const CSB_V1_BootProfile *profile,
    unsigned short thing);
int csb_v1_boot_runtime_object_action_set_index_pc34(
    const CSB_V1_BootProfile *profile,
    unsigned short thing);
uint16_t csb_v1_boot_runtime_object_allowed_slots_pc34(
    const CSB_V1_BootProfile *profile,
    unsigned short thing);
int csb_v1_boot_runtime_object_name_pc34(
    const CSB_V1_BootProfile *profile,
    unsigned short thing,
    char *out,
    size_t out_size);
int csb_v1_boot_runtime_read_container_slots_pc34(
    const CSB_V1_BootProfile *profile,
    unsigned short container_thing,
    unsigned short out_slots[8]);
int csb_v1_boot_runtime_write_container_slots_pc34(
    CSB_V1_BootProfile *profile,
    unsigned short container_thing,
    const unsigned short slots[8]);
int csb_v1_boot_runtime_set_thing_next_pc34(
    CSB_V1_BootProfile *profile,
    unsigned short thing,
    unsigned short next_thing);
int csb_v1_boot_runtime_write_inventory_slot_pc34(
    CSB_V1_BootProfile *profile,
    int champion_index,
    int csb_slot,
    unsigned short thing);
int csb_v1_boot_runtime_write_leader_hand_pc34(
    CSB_V1_BootProfile *profile,
    unsigned short thing);
int csb_v1_boot_runtime_write_champion_vitals_pc34(
    CSB_V1_BootProfile *profile,
    int champion_index,
    int current_health,
    int current_stamina,
    int current_mana);
int csb_v1_boot_runtime_m11_mirror_receipt_pc34(
    const CSB_V1_BootProfile *profile,
    CSB_V1_RuntimeM11MirrorReceipt_PC34 *out_receipt);
int csb_v1_boot_runtime_trigger_front_wall_ornament_click_pc34(
    CSB_V1_BootProfile *profile,
    unsigned short leader_hand_thing,
    unsigned short *out_leader_hand_thing);

/* ── Launch→runtime assumption gate ─────────────────────────────────────
 *
 * csb_v1_boot_assume_no_dm1_runtime() rejects boot profiles that look
 * like they were constructed from DM1 defaults.  It is the explicit
 * launch-to-runtime boundary gate: every CSB handoff must clear these
 * assertions before csb_v1_boot_enter_game() rebuilds the runtime.
 *
 * What it rejects (return -1):
 *   - profile->game_id != "csb"                  (defensive: defends against
 *                                                 misrouted DM1/DM2 profiles)
 *   - profile->variant_id outside CSB_V1_VARIANT_* range
 *                                               (catches raw enum leakage)
 *   - profile->default_party_x/y == DM1 HoC     (catches (11,29) leakage)
 *   - profile->tick_ms != CSB_V1_TICK_MS_NOMINAL (catches non-CSB tick quantum)
 *   - profile->entrance_map_index != 255U       (C255_MAP_INDEX_ENTRANCE only)
 *   - profile->start_map_index != 0U            (LOADSAVE.C F0435 new-game map)
 *
 * What it does NOT reject (returns 0):
 *   - graphics_path/dungeon_path fields       (verified in csb_v1_boot_enter_game)
 *   - assets_verified bit                    (verified in csb_v1_boot_enter_game)
 *   - DM1 dungeon hash on the wire           (verified in csb_v1_boot_scan_assets)
 *
 * The scan + enter_game path calls this gate automatically so callers
 * don't have to thread it manually.  The probe + tests call it directly
 * to surface which assertion a DM1-shape profile trips.
 *
 * Returns 0 if the profile is a clean CSB shape, -1 otherwise.  The
 * failure reason is reported through csb_v1_boot_last_assumption_reason()
 * so a regression can pinpoint which CSB-only invariant leaked.
 *
 * Source: ReDMCSB ENTRANCE.C F0806 lines 409-441 (CSB entrance/C28_ENTRANCE_CSB)
 * Source: ReDMCSB LOADSAVE.C F0435 lines 1940-1944 (new-game map 0)
 * Source: ReDMCSB BASE.C line 36-39 (G0298_B_NewGame mode storage)
 * Source: csb_v1_runtime_pc34_compat.h CSB_V1_TICK_MS_NOMINAL (55ms) */
int csb_v1_boot_assume_no_dm1_runtime(const CSB_V1_BootProfile *profile);

/* Returns a short human-readable reason string for the last failed
 * csb_v1_boot_assume_no_dm1_runtime() call.  The pointer is owned by
 * the boot module and remains valid for the lifetime of the process
 * (it is overwritten by every call).  Useful for diagnostics and CI
 * logs that need to attribute the failure to a specific CSB invariant. */
const char *csb_v1_boot_last_assumption_reason(void);

int csb_v1_boot_enter_game(CSB_V1_BootProfile *profile);
void csb_v1_boot_cleanup(CSB_V1_BootProfile *profile);
size_t csb_v1_boot_diagnostic_report(const CSB_V1_BootProfile *profile,
                                     char *buf,
                                     size_t buf_size);
void csb_v1_boot_print_summary(const CSB_V1_BootProfile *profile);
const char *csb_v1_boot_source_evidence(void);

/* ----------------------------------------------------------------
 * CSB V1 boot profile -> M11 entry guard.
 *
 * Deterministic go/no-go check used by both the M12 launch flow
 * and the M11_GameView_Start CSB branch before the launcher hands
 * a verified CSB boot profile off to the FS_GAME_CSB runtime.
 *
 * Two entry shapes are supported:
 *
 *   1) `csb_v1_boot_profile_m11_entry_gate(profile, reason, reason_size)`
 *      validates an already-scanned CSB_V1_BootProfile.
 *      Pass = (state >= CSB_V1_BOOT_STATE_ASSETS_READY) AND
 *             assets_verified AND graphics_verified AND dungeon_verified
 *             AND the matched MD5s come from the canonical CSB V1
 *             graphics + dungeon hash registry (PC 3.4 EN, Atari ST
 *             2.x, Amiga 3.x EN, Amiga 3.x ML).
 *
 *   2) `csb_v1_boot_graphics_dungeon_m11_entry_gate(graphics_md5,
 *      dungeon_md5, reason, reason_size)` validates a raw pair of
 *      matched MD5 hex strings (the same pair the launcher pushes
 *      into the M11_GameLaunchSpec via verifiedAssetPath/Md5 plus
 *      the dungeon MD5 it records after asset scan). Useful when
 *      the caller does not have a boot profile in scope.
 *
 * Both forms return 1 on pass and 0 on fail.  When 0 is returned,
 * `reason` (when non-NULL and `reason_size > 0`) is filled with a
 * short human-readable explanation suitable for an M12 launch
 * dialog or stderr log; it is always NUL-terminated.
 *
 * Source-lock boundary:
 *   ReDMCSB ENTRANCE.C F0806 lines 409-441 (CSB entrance setup;
 *     the canonical CSB V1 entry path is gated by media-class
 *     detection, not by filesystem layout, so the gate hashes
 *     mirror the documented CSB V1 media hash registry).
 *   ReDMCSB LOADSAVE.C F0435 lines 1936-1944 (new-game dungeon
 *     load; the gate blocks launches without a verified dungeon
 *     because F0435 will not reach C001_MODE_LOAD_DUNGEON without
 *     a header it can read).
 *
 * The gate is intentionally narrow: it does NOT touch the live
 * dungeon handle, does NOT mutate the boot profile, and does NOT
 * load or decode asset bytes. It only validates the matched-MD5
 * evidence the launcher already collected via asset_find_by_md5().
 */
int csb_v1_boot_profile_m11_entry_gate(const CSB_V1_BootProfile *profile,
                                       char *reason,
                                       size_t reason_size);
int csb_v1_boot_graphics_dungeon_m11_entry_gate(const char *graphics_md5,
                                                const char *dungeon_md5,
                                                char *reason,
                                                size_t reason_size);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V1_BOOT_H */

#ifndef FIRESTAFF_CSB_V1_BOOT_H
#define FIRESTAFF_CSB_V1_BOOT_H

#include <stddef.h>
#include <stdint.h>

#include "csb_v1_csbgraphics_m11_runtime_plan.h"
#include "csb_v1_runtime_pc34_compat.h"
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

typedef struct {
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
} CSB_V1_BootRuntimeStartupSnapshot_PC34;

typedef enum CSB_V1_BootStartupActionKind_PC34 {
    CSB_V1_BOOT_STARTUP_ACTION_NONE_PC34 = 0,
    CSB_V1_BOOT_STARTUP_ACTION_UTILITY_PC34 = 1,
    CSB_V1_BOOT_STARTUP_ACTION_ENTRANCE_PC34 = 2
} CSB_V1_BootStartupActionKind_PC34;

typedef struct CSB_V1_BootStartupActionReceipt_PC34 {
    CSB_V1_BootStartupActionKind_PC34 kind;
    int handled;
    CSB_V1_RuntimeUtilStartupHostActionReceipt_PC34 utility_receipt;
    CSB_V1_StartupEntranceHostActionReceipt_PC34 entrance_receipt;
} CSB_V1_BootStartupActionReceipt_PC34;

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

void csb_v1_boot_startup_action_receipt_init_pc34(
    CSB_V1_BootStartupActionReceipt_PC34 *receipt);
void csb_v1_boot_startup_presentation_route_receipt_init_pc34(
    CSB_V1_BootStartupPresentationRouteReceipt_PC34 *receipt);
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
int csb_v1_boot_startup_host_facts_from_runtime_state_pc34(
    CSB_V1_StartupHostFacts_PC34 *facts,
    int title_active,
    int title_frame,
    int title_source_step,
    int entrance_active,
    int entrance_source_step,
    int entrance_dismissed,
    int credits_active,
    int credits_remaining_ticks,
    int opening_active,
    int opening_delay_ticks,
    int opening_step,
    int pending_command,
    int entrance_frame,
    int utility_overlay_active,
    int utility_selected_action_index,
    int utility_imported_champion_count,
    int utility_preview_active,
    const char *utility_prompt,
    int resume_available,
    const char *resume_path,
    const CSB_V1_BootProfile *boot_profile);
int csb_v1_boot_startup_build_render_plan_from_runtime_state_pc34(
    CSB_V1_StartupRenderPlan_PC34 *out_plan,
    int title_active,
    int title_frame,
    int title_source_step,
    int entrance_active,
    int entrance_source_step,
    int entrance_dismissed,
    int credits_active,
    int credits_remaining_ticks,
    int opening_active,
    int opening_delay_ticks,
    int opening_step,
    int pending_command,
    int entrance_frame,
    int utility_overlay_active,
    int utility_selected_action_index,
    int utility_imported_champion_count,
    int utility_preview_active,
    const char *utility_prompt,
    int resume_available,
    const char *resume_path,
    const CSB_V1_BootProfile *boot_profile);
int csb_v1_boot_startup_build_render_plan_from_snapshot_pc34(
    const CSB_V1_BootRuntimeStartupSnapshot_PC34 *snapshot,
    CSB_V1_StartupRenderPlan_PC34 *out_plan);
int csb_v1_boot_startup_build_default_render_plan_pc34(
    CSB_V1_StartupRenderPlan_PC34 *out_plan);
int csb_v1_boot_startup_advance_idle_from_runtime_state_pc34(
    CSB_V1_StartupIdleReceipt_PC34 *out_receipt,
    int title_active,
    int title_frame,
    int title_source_step,
    int entrance_active,
    int entrance_source_step,
    int entrance_dismissed,
    int credits_active,
    int credits_remaining_ticks,
    int opening_active,
    int opening_delay_ticks,
    int opening_step,
    int pending_command,
    int entrance_frame,
    int utility_overlay_active,
    int utility_selected_action_index,
    int utility_imported_champion_count,
    int utility_preview_active,
    const char *utility_prompt,
    int resume_available,
    const char *resume_path,
    const CSB_V1_BootProfile *boot_profile);
int csb_v1_boot_startup_advance_idle_from_snapshot_pc34(
    const CSB_V1_BootRuntimeStartupSnapshot_PC34 *snapshot,
    CSB_V1_StartupIdleReceipt_PC34 *out_receipt);
int csb_v1_boot_startup_entrance_accepts_input_from_runtime_state_pc34(
    int title_active,
    int title_frame,
    int title_source_step,
    int entrance_active,
    int entrance_source_step,
    int entrance_dismissed,
    int credits_active,
    int credits_remaining_ticks,
    int opening_active,
    int opening_delay_ticks,
    int opening_step,
    int pending_command,
    int entrance_frame,
    int utility_overlay_active,
    int utility_selected_action_index,
    int utility_imported_champion_count,
    int utility_preview_active,
    const char *utility_prompt,
    int resume_available,
    const char *resume_path,
    const CSB_V1_BootProfile *boot_profile);
int csb_v1_boot_startup_entrance_accepts_input_from_snapshot_pc34(
    const CSB_V1_BootRuntimeStartupSnapshot_PC34 *snapshot);
int csb_v1_boot_startup_presentation_receipt_from_runtime_state_pc34(
    char *out_phase,
    int out_phase_size,
    int *out_startup_active,
    int *out_startup_frame,
    char *out_animation,
    int out_animation_size,
    int *out_animation_active,
    int *out_title_frame,
    int *out_title_frame_max,
    int *out_title_ready,
    int title_active,
    int title_frame,
    int title_source_step,
    int entrance_active,
    int entrance_source_step,
    int entrance_dismissed,
    int credits_active,
    int credits_remaining_ticks,
    int opening_active,
    int opening_delay_ticks,
    int opening_step,
    int pending_command,
    int entrance_frame,
    int utility_overlay_active,
    int utility_selected_action_index,
    int utility_imported_champion_count,
    int utility_preview_active,
    const char *utility_prompt,
    int resume_available,
    const char *resume_path,
    const CSB_V1_BootProfile *boot_profile);
int csb_v1_boot_startup_presentation_state_receipt_from_runtime_state_pc34(
    CSB_V1_StartupPresentationReceipt_PC34 *out_receipt,
    int title_active,
    int title_frame,
    int title_source_step,
    int entrance_active,
    int entrance_source_step,
    int entrance_dismissed,
    int credits_active,
    int credits_remaining_ticks,
    int opening_active,
    int opening_delay_ticks,
    int opening_step,
    int pending_command,
    int entrance_frame,
    int utility_overlay_active,
    int utility_selected_action_index,
    int utility_imported_champion_count,
    int utility_preview_active,
    const char *utility_prompt,
    int resume_available,
    const char *resume_path,
    const CSB_V1_BootProfile *boot_profile);
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
int csb_v1_boot_runtime_util_render_plan_from_runtime_state_pc34(
    CSB_V1_UtilRenderPlan *out_plan,
    int title_active,
    int title_frame,
    int title_source_step,
    int entrance_active,
    int entrance_source_step,
    int entrance_dismissed,
    int credits_active,
    int credits_remaining_ticks,
    int opening_active,
    int opening_delay_ticks,
    int opening_step,
    int pending_command,
    int entrance_frame,
    int utility_overlay_active,
    int utility_selected_action_index,
    int utility_imported_champion_count,
    int utility_preview_active,
    const char *utility_prompt,
    int resume_available,
    const char *resume_path,
    const CSB_V1_BootProfile *boot_profile);
int csb_v1_boot_runtime_util_render_plan_from_snapshot_pc34(
    const CSB_V1_BootRuntimeStartupSnapshot_PC34 *snapshot,
    CSB_V1_UtilRenderPlan *out_plan);
int csb_v1_boot_startup_execute_render_plan_pc34(
    const CSB_V1_StartupRenderPlan_PC34 *plan,
    const CSB_V1_StartupRenderExecutor_PC34 *executor);
void csb_v1_boot_startup_execute_primitive_commands_pc34(
    const CSB_V1_StartupRenderPlan_PC34 *plan,
    unsigned char *framebuffer,
    int framebuffer_width,
    int framebuffer_height);
int csb_v1_boot_startup_execute_asset_commands_kind_pc34(
    const CSB_V1_StartupRenderPlan_PC34 *plan,
    CSB_V1_StartupAssetCommandKind_PC34 kind,
    CSB_V1_StartupAssetExecutor_PC34 executor,
    void *user);
int csb_v1_boot_startup_execute_closed_door_asset_commands_pc34(
    const CSB_V1_StartupRenderPlan_PC34 *plan,
    CSB_V1_StartupAssetExecutor_PC34 executor,
    void *user);
int csb_v1_boot_startup_title_empty_fallback_needed_pc34(
    const CSB_V1_StartupRenderPlan_PC34 *plan,
    const unsigned char *framebuffer,
    int framebuffer_width,
    int framebuffer_height);
int csb_v1_boot_startup_execute_opening_composite_pc34(
    const CSB_V1_StartupRenderPlan_PC34 *plan,
    CSB_V1_StartupOpeningCompositeExecutor_PC34 executor,
    void *user);
int csb_v1_boot_runtime_util_apply_pointer_from_runtime_state_pc34(
    CSB_V1_RuntimeUtilStartupHostActionReceipt_PC34 *out_receipt,
    int title_active,
    int title_frame,
    int title_source_step,
    int entrance_active,
    int entrance_source_step,
    int entrance_dismissed,
    int credits_active,
    int credits_remaining_ticks,
    int opening_active,
    int opening_delay_ticks,
    int opening_step,
    int pending_command,
    int entrance_frame,
    int utility_overlay_active,
    int utility_selected_action_index,
    int utility_imported_champion_count,
    int utility_preview_active,
    const char *utility_prompt,
    int resume_available,
    const char *resume_path,
    const CSB_V1_BootProfile *boot_profile,
    int x,
    int y);
int csb_v1_boot_runtime_util_apply_pointer_from_snapshot_pc34(
    const CSB_V1_BootRuntimeStartupSnapshot_PC34 *snapshot,
    int x,
    int y,
    CSB_V1_RuntimeUtilStartupHostActionReceipt_PC34 *out_receipt);
int csb_v1_boot_runtime_util_apply_firestaff_input_from_runtime_state_pc34(
    CSB_V1_RuntimeUtilStartupHostActionReceipt_PC34 *out_receipt,
    int title_active,
    int title_frame,
    int title_source_step,
    int entrance_active,
    int entrance_source_step,
    int entrance_dismissed,
    int credits_active,
    int credits_remaining_ticks,
    int opening_active,
    int opening_delay_ticks,
    int opening_step,
    int pending_command,
    int entrance_frame,
    int utility_overlay_active,
    int utility_selected_action_index,
    int utility_imported_champion_count,
    int utility_preview_active,
    const char *utility_prompt,
    int resume_available,
    const char *resume_path,
    const CSB_V1_BootProfile *boot_profile,
    int menu_input);
int csb_v1_boot_runtime_util_apply_firestaff_input_from_snapshot_pc34(
    const CSB_V1_BootRuntimeStartupSnapshot_PC34 *snapshot,
    int menu_input,
    CSB_V1_RuntimeUtilStartupHostActionReceipt_PC34 *out_receipt);
int csb_v1_boot_runtime_execute_startup_entrance_firestaff_input_from_runtime_state_pc34(
    CSB_V1_StartupEntranceHostActionReceipt_PC34 *out_receipt,
    int title_active,
    int title_frame,
    int title_source_step,
    int entrance_active,
    int entrance_source_step,
    int entrance_dismissed,
    int credits_active,
    int credits_remaining_ticks,
    int opening_active,
    int opening_delay_ticks,
    int opening_step,
    int pending_command,
    int entrance_frame,
    int utility_overlay_active,
    int utility_selected_action_index,
    int utility_imported_champion_count,
    int utility_preview_active,
    const char *utility_prompt,
    int resume_available,
    const char *resume_path,
    const CSB_V1_BootProfile *boot_profile,
    int menu_input);
int csb_v1_boot_runtime_execute_startup_entrance_firestaff_input_from_snapshot_pc34(
    const CSB_V1_BootRuntimeStartupSnapshot_PC34 *snapshot,
    int menu_input,
    CSB_V1_StartupEntranceHostActionReceipt_PC34 *out_receipt);
int csb_v1_boot_runtime_execute_startup_entrance_pointer_from_runtime_state_pc34(
    CSB_V1_StartupEntranceHostActionReceipt_PC34 *out_receipt,
    int title_active,
    int title_frame,
    int title_source_step,
    int entrance_active,
    int entrance_source_step,
    int entrance_dismissed,
    int credits_active,
    int credits_remaining_ticks,
    int opening_active,
    int opening_delay_ticks,
    int opening_step,
    int pending_command,
    int entrance_frame,
    int utility_overlay_active,
    int utility_selected_action_index,
    int utility_imported_champion_count,
    int utility_preview_active,
    const char *utility_prompt,
    int resume_available,
    const char *resume_path,
    const CSB_V1_BootProfile *boot_profile,
    int x,
    int y,
    unsigned int button_mask);
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
int csb_v1_boot_runtime_execute_startup_pointer_from_snapshot_pc34(
    const CSB_V1_BootRuntimeStartupSnapshot_PC34 *snapshot,
    int x,
    int y,
    unsigned int button_mask,
    CSB_V1_BootStartupActionReceipt_PC34 *out_receipt);
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

#ifndef FIRESTAFF_CSB_V1_BOOT_H
#define FIRESTAFF_CSB_V1_BOOT_H

#include <stddef.h>
#include <stdint.h>

#include "csb_v1_csbgraphics_dat_inventory_pc34_compat.h"
#include "csb_v1_csbgraphics_runtime_plan.h"
#include "csb_v1_csbwin_save_loader_boundary_pc34_compat.h"
#include "csb_v1_runtime_pc34_compat.h"
#include "csb_v1_startup_img3_decode_pc34_compat.h"
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

/* ReDMCSB TITLE.C F0437 loads C001 and uses three zones from that one
 * bitmap; ENTRANCE.C F0806 loads C002-C005 for the entrance.  Keep those
 * original-data identities separate from Firestaff's transient asset slots. */
typedef enum CSB_V1_StartupAssetRole_PC34 {
    CSB_V1_STARTUP_ASSET_ROLE_NONE_PC34 = 0,
    CSB_V1_STARTUP_ASSET_ROLE_TITLE_PRESENTS_PC34,
    CSB_V1_STARTUP_ASSET_ROLE_TITLE_CHAOS_PC34,
    CSB_V1_STARTUP_ASSET_ROLE_TITLE_STRIKES_BACK_PC34,
    CSB_V1_STARTUP_ASSET_ROLE_ENTRANCE_LEFT_DOOR_PC34,
    CSB_V1_STARTUP_ASSET_ROLE_ENTRANCE_RIGHT_DOOR_PC34,
    CSB_V1_STARTUP_ASSET_ROLE_ENTRANCE_SCREEN_PC34,
    CSB_V1_STARTUP_ASSET_ROLE_ENTRANCE_CREDITS_PC34,
    CSB_V1_STARTUP_ASSET_ROLE_HUD_INVENTORY_PC34,
    CSB_V1_STARTUP_ASSET_ROLE_HUD_RESURRECT_PC34,
    CSB_V1_STARTUP_ASSET_ROLE_COUNT_PC34
} CSB_V1_StartupAssetRole_PC34;

typedef enum CSB_V1_StartupAssetSource_PC34 {
    CSB_V1_STARTUP_ASSET_SOURCE_NONE_PC34 = 0,
    CSB_V1_STARTUP_ASSET_SOURCE_GRAPHICS_DAT_PC34,
    CSB_V1_STARTUP_ASSET_SOURCE_CSBGRAPHICS_DAT_PC34
} CSB_V1_StartupAssetSource_PC34;

typedef struct CSB_V1_StartupAssetBinding_PC34 {
    CSB_V1_StartupAssetRole_PC34 role;
    CSB_V1_StartupAssetSource_PC34 source;
    uint32_t graphic_index;
    int verified;
    int rejects_generic_or_test_asset;
    char path[CSB_V1_CSBGRAPHICS_DAT_REAL_PATH_CAP];
} CSB_V1_StartupAssetBinding_PC34;

typedef struct CSB_V1_StartupAssetSelection_PC34 {
    int real_graphics_available;
    int csbgraphics_available;
    int reject_generic_or_test_assets;
    CSB_V1_StartupAssetBinding_PC34
        bindings[CSB_V1_STARTUP_ASSET_ROLE_COUNT_PC34];
} CSB_V1_StartupAssetSelection_PC34;

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

    /* Optional raw PC34 SWSHSND.C G0746 payload.  It is accepted only from
     * the selected CSB asset root, at the original byte count; callers must
     * not substitute a DM1 sound bank or host-generated cue. */
    char swoosh_source_path[512];
    uint8_t swoosh_source_bytes[9078];
    uint32_t swoosh_source_fnv1a;
    int swoosh_source_bound;

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
    int csbgraphics_inventory_ready;
    int csbgraphics_inventory_result;
    int csbgraphics_plan_result;
    int csbgraphics_palette_admission_attempted;
    int csbgraphics_palette_admission_result;
    int csbgraphics_skin_def_loaded;
    uint16_t csbgraphics_skin_def_words
        [CSB_V1_CSBGRAPHICS_RUNTIME_SKIN_DEF_MAX_WORDS];
    size_t csbgraphics_skin_def_word_count;
    CSB_V1_CSBGraphicsDatRealCache csbgraphics_cache;
    CSB_V1_CSBGraphicsInventory csbgraphics_inventory;
    CSB_V1_CSBGraphicsDatPaletteSourceReceipt csbgraphics_palette_receipt;
    CSB_V1_CSBGraphicsRuntimePlan csbgraphics_runtime_plan;
    CSB_V1_StartupAssetSelection_PC34 startup_assets;

    /* F0219 may leave an owned C14 at a resolved C05 destination.  Retain
     * only the admitted source receipt for the next F0128 frame; the boot
     * route never invents a marker or a substitute projectile bitmap. */
#define CSB_V1_BOOT_POST_TELEPORT_PROJECTILE_MAX_PC34 8
    CSB_V1_F0219ProjectileImpactMaterialHandoffPc34
        post_teleport_projectile_handoffs[
            CSB_V1_BOOT_POST_TELEPORT_PROJECTILE_MAX_PC34];
    size_t post_teleport_projectile_handoff_count;
    int post_teleport_projectile_runtime_frame_active;

    CSB_V1_RuntimeProfile runtime;
} CSB_V1_BootProfile;

typedef struct CSB_V1_BootStartupCSBGraphicsPaletteReadiness_PC34 {
    int cache_identity_ready;
    int palette_receipt_ready;
    int title_palette_ready;
    int door_palette_ready;
    int hud_palette_ready;
    int m11_no_draw_without_palette;
    uint32_t palette_entry_index;
    uint32_t palette_decoded_fnv1a;
    char source_path[CSB_V1_CSBGRAPHICS_DAT_REAL_PATH_CAP];
    char source_md5[CSB_V1_CSBGRAPHICS_DAT_REAL_MD5_CAP];
} CSB_V1_BootStartupCSBGraphicsPaletteReadiness_PC34;

/* ReDMCSB ENTRANCE.C F0806 releases the opening page before DUNVIEW.C F0128
 * builds the first runtime viewport. Bind that real viewport output to the
 * same terminal C001-C005/C017/C040 session; M11 must not retain a host page
 * or manufacture a replacement dungeon surface at this boundary. */
typedef struct CSB_V1_FirstLiveDungeonFrameReceipt_PC34 {
    int valid;
    int real_asset_matched;
    int terminal_session_owned;
    int viewport_frame_consumed;
    int no_synthetic_surface;
    uint32_t session_generation;
    uint32_t source_tick;
    uint32_t viewport_pixel_hash;
    uint32_t draw_counts_hash;
    const char *source_evidence;
} CSB_V1_FirstLiveDungeonFrameReceipt_PC34;

typedef struct CSB_V1_BootStartupLaunchReceipts_PC34 {
    CSB_V1_RuntimeStartupHandoffReceipt_PC34 handoff;
    CSB_V1_StartupInitStateReceipt_PC34 init_state;
    CSB_V1_RuntimeStartupSessionStateReceipt_PC34 session_state;
    CSB_V1_StartupHostReceipt_PC34 launch_host_receipt;
} CSB_V1_BootStartupLaunchReceipts_PC34;

/* A verified CSB launch has one owner for startup media and session state.
 * TITLE.C F0437 consumes C001, while ENTRANCE.C F0438/F0441 consumes the
 * entrance artwork and menu state in the same loop.  Do not let a runtime
 * consumer reintroduce an unowned text, door, or utility fallback. */
typedef struct CSB_V1_BootStartupRuntimeAssetGateReceipt_PC34 {
    int valid;
    int real_asset_matched;
    int asset_ownership_valid;
    int session_state_valid;
    int title_assets_owned;
    int entrance_assets_owned;
    int hud_assets_owned;
    int rejects_fallback_sources;
    uint64_t real_asset_receipt_hash;
    CSB_V1_RuntimeStartupSessionStateReceipt_PC34 session_state;
    CSB_V1_StartupRealReceipt real_asset_receipt;
    const char *source_evidence;
} CSB_V1_BootStartupRuntimeAssetGateReceipt_PC34;

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
    int bind_graphics_to_runtime_asset_loader;
    int load_original_font_from_graphics;
    int real_asset_receipt_valid;
    CSB_V1_StartupRealReceipt real_asset_receipt;
    int startup_asset_gate_valid;
    CSB_V1_BootStartupRuntimeAssetGateReceipt_PC34 startup_asset_gate;
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
    int title_unique_sample_hash_count;
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
    int title_runtime_unique_sample_hash_count;
    int title_runtime_all_stages_consumed;
    int title_presents_runtime_consumed;
    int title_chaos_zoom_runtime_consumed;
    int title_chaos_hold_runtime_consumed;
    int title_strikes_back_runtime_consumed;
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
    int title_runtime_unique_sample_hash_count;
    int title_presents_runtime_captured;
    int title_chaos_zoom_runtime_captured;
    int title_chaos_hold_runtime_captured;
    int title_strikes_back_runtime_captured;
    int title_runtime_phase_mask;
    int title_runtime_expected_phase_mask;
    int title_runtime_phase_route_complete;
    int title_runtime_phase_hash_count;
    uint32_t title_runtime_phase_hashes[CSB_V1_BOOT_STARTUP_TITLE_SAMPLE_COUNT_PC34];
    uint32_t title_runtime_phase_hash;
    int closed_door_hud_runtime_captured;
    int utility_hud_runtime_captured;
    int door_opening_runtime_captured;
    int credits_runtime_captured;
    int draw_consumes_receipt_only;
    int input_consumes_receipt_only;
    int no_fallback_callbacks;
    int no_wrapper_fallback_routes;
    int real_startup_assets_bound;
    int real_startup_asset_role_count;
    uint32_t real_startup_asset_binding_hash;
    int title_host_ownership_valid;
    int closed_door_host_ownership_valid;
    int utility_host_ownership_valid;
    int door_opening_host_ownership_valid;
    int credits_host_ownership_valid;
    int title_host_draw_consumes_receipt_only;
    int closed_door_host_draw_consumes_receipt_only;
    int utility_host_draw_consumes_receipt_only;
    int door_opening_host_draw_consumes_receipt_only;
    int credits_host_draw_consumes_receipt_only;
    int title_host_input_consumes_receipt_only;
    int closed_door_host_input_consumes_receipt_only;
    int utility_host_input_consumes_receipt_only;
    int door_opening_host_input_consumes_receipt_only;
    int credits_host_input_consumes_receipt_only;
    int host_route_wrappers_retired;
    int no_loose_render_plan_exports;
    uint32_t title_packaged_capture_hash;
    uint32_t closed_door_packaged_capture_hash;
    uint32_t utility_packaged_capture_hash;
    uint32_t door_opening_packaged_capture_hash;
    uint32_t credits_packaged_capture_hash;
    uint32_t sequence_capture_hash;
    uint32_t runtime_capture_hash;
    uint32_t route_hardening_hash;
    uint32_t runtime_host_gate_hash;
    CSB_V1_BootStartupRuntimeVisualCaptureReceipt_PC34 runtime_visual;
    CSB_V1_BootStartupRuntimeRouteHardeningReceipt_PC34 title_route_hardening;
    CSB_V1_BootStartupRuntimeRouteHardeningReceipt_PC34 closed_door_route_hardening;
    CSB_V1_BootStartupRuntimeRouteHardeningReceipt_PC34 utility_route_hardening;
    CSB_V1_BootStartupRuntimeRouteHardeningReceipt_PC34 door_opening_route_hardening;
    CSB_V1_BootStartupRuntimeRouteHardeningReceipt_PC34 credits_route_hardening;
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
    int primitive_commands_ready;
    int title_asset_commands_ready;
    int closed_door_asset_commands_ready;
    int opening_frame_command_ready;
    int real_asset_matched;
    CSB_V1_StartupRenderPlan_PC34 render_plan;
    const char *source_evidence;
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
    int host_draw_package_ready;
    int host_draw_uses_receipt_package;
    int no_legacy_render_wrapper_ready;
} CSB_V1_BootStartupHostViewReceipt_PC34;

/* Complete CSB-owned startup presentation transaction for an M11 consumer.
 * Verification may still inspect capture receipts, but frame consumers get
 * the resolved render plan, utility/HUD plan, readiness and asset proof in
 * one snapshot-bound value. ReDMCSB TITLE.C F0437 lines 424-463 and
 * ENTRANCE.C F0441/F0806 lines 850-883 keep those decisions in one loop;
 * CSBWin Viewport.cpp keeps the menu panel as presentation state. */
typedef struct CSB_V1_BootStartupM11PresentationReceipt_PC34 {
    int valid;
    int route;
    int startup_render_plan_valid;
    CSB_V1_StartupRenderPlan_PC34 startup_render_plan;
    int utility_render_plan_valid;
    CSB_V1_UtilRenderPlan utility_render_plan;
    int hud_menu_draw_valid;
    CSB_V1_BootStartupHudMenuDrawReceipt_PC34 hud_menu_draw;
    int readiness_valid;
    CSB_V1_BootStartupReadinessReceipt_PC34 readiness;
    int capture_proof_valid;
    CSB_V1_BootStartupPackagedCaptureProof_PC34 capture_proof;
    int host_view_valid;
    int host_draw_package_ready;
    int host_draw_uses_receipt_package;
    int no_legacy_render_wrapper_ready;
    int legacy_plan_exports_inspection_only;
    int input_ready;
    int hud_ready;
    int runtime_ready;
    int selected_command_id;
    int selected_utility_action_index;
    const char *source_evidence;
} CSB_V1_BootStartupM11PresentationReceipt_PC34;

/* CSB-owned, indexed source pixels ready for a startup renderer.  The
 * title source is C001; the three title regions are independent cropped
 * surfaces.  The door surfaces are the C002/C003 strips for the current
 * opening step.  Call release before discarding a populated set. */
typedef enum CSB_V1_StartupRuntimeSurfaceRole_PC34 {
    CSB_V1_STARTUP_RUNTIME_SURFACE_TITLE_PC34 = 0,
    CSB_V1_STARTUP_RUNTIME_SURFACE_PRESENTS_PC34,
    CSB_V1_STARTUP_RUNTIME_SURFACE_CHAOS_PC34,
    CSB_V1_STARTUP_RUNTIME_SURFACE_STRIKES_BACK_PC34,
    CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_LEFT_PC34,
    CSB_V1_STARTUP_RUNTIME_SURFACE_OPENING_RIGHT_PC34,
    CSB_V1_STARTUP_RUNTIME_SURFACE_ENTRANCE_SCREEN_PC34,
    CSB_V1_STARTUP_RUNTIME_SURFACE_ENTRANCE_CREDITS_PC34,
    CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_INVENTORY_PC34,
    CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_RESURRECT_PC34,
    CSB_V1_STARTUP_RUNTIME_SURFACE_COUNT_PC34
} CSB_V1_StartupRuntimeSurfaceRole_PC34;

/* C001--C005 use the PC34 four-plane GRAPHICS.DAT stream while optional
 * CSBgraphics.dat overrides are already decoded indexed entries.  Keep that
 * distinction with the surface: treating both as IMG3 made an authentic
 * CSBgraphics C017/C040 page fail the runtime HUD admission gate. */
typedef enum CSB_V1_StartupRuntimeSurfaceSource_PC34 {
    CSB_V1_STARTUP_RUNTIME_SURFACE_SOURCE_NONE_PC34 = 0,
    CSB_V1_STARTUP_RUNTIME_SURFACE_SOURCE_GRAPHICS_DAT_PC34,
    CSB_V1_STARTUP_RUNTIME_SURFACE_SOURCE_CSBGRAPHICS_DAT_PC34
} CSB_V1_StartupRuntimeSurfaceSource_PC34;

typedef struct CSB_V1_StartupRuntimeSurface_PC34 {
    unsigned char *pixels;
    int width;
    int height;
    int source_asset_id;
    int source_x;
    int source_y;
    int transparent_color;
    int valid;
    CSB_V1_StartupRuntimeSurfaceSource_PC34 source_kind;
    uint32_t decoded_pixel_fnv1a;
    CSB_V1_StartupGraphicDecodeReceipt_PC34 decode_receipt;
} CSB_V1_StartupRuntimeSurface_PC34;

typedef struct CSB_V1_StartupRuntimeSurfaceSet_PC34 {
    int valid;
    int real_asset_matched;
    int title_regions_ready;
    int opening_frame_ready;
    int entrance_screen_ready;
    int hud_surfaces_ready;
    CSB_V1_StartupRuntimeSurface_PC34
        surfaces[CSB_V1_STARTUP_RUNTIME_SURFACE_COUNT_PC34];
} CSB_V1_StartupRuntimeSurfaceSet_PC34;

/* Startup playback remains with the verified asset owner.  The host consumes
 * the emitted audio action, but cannot substitute a synthetic title, entrance
 * or HUD route once a real CSB session has started. */
typedef enum CSB_V1_StartupPlaybackStage_PC34 {
    CSB_V1_STARTUP_PLAYBACK_STAGE_NONE_PC34 = 0,
    CSB_V1_STARTUP_PLAYBACK_STAGE_FTL_SWOOSH_PC34,
    CSB_V1_STARTUP_PLAYBACK_STAGE_TITLE_PC34,
    CSB_V1_STARTUP_PLAYBACK_STAGE_ENTRANCE_PC34,
    CSB_V1_STARTUP_PLAYBACK_STAGE_HUD_PC34
} CSB_V1_StartupPlaybackStage_PC34;

typedef enum CSB_V1_StartupAudioAction_PC34 {
    CSB_V1_STARTUP_AUDIO_ACTION_NONE_PC34 = 0,
    CSB_V1_STARTUP_AUDIO_ACTION_PLAY_FTL_SWOOSH_PC34,
    CSB_V1_STARTUP_AUDIO_ACTION_RELEASE_FTL_SWOOSH_PC34,
    CSB_V1_STARTUP_AUDIO_ACTION_PLAY_ENTRANCE_MUSIC_PC34
} CSB_V1_StartupAudioAction_PC34;

enum {
    CSB_V1_STARTUP_HUD_INVENTORY_WIDTH_PC34 = 224,
    CSB_V1_STARTUP_HUD_INVENTORY_HEIGHT_PC34 = 136,
    CSB_V1_STARTUP_HUD_RESURRECT_WIDTH_PC34 = 144,
    CSB_V1_STARTUP_HUD_RESURRECT_HEIGHT_PC34 = 73,
    CSB_V1_STARTUP_HUD_RESURRECT_TRANSPARENT_COLOR_PC34 = 6
};

typedef struct CSB_V1_StartupPlaybackState_PC34 {
    CSB_V1_StartupPlaybackStage_PC34 stage;
    CSB_V1_StartupStage_PC34 title_stage;
    int title_frame;
    int title_phase_mask;
    int swoosh_active;
    int entrance_music_active;
    int entrance_complete;
    int entrance_scene_presented;
    /* ENTRANCE.C F0442's C005 credits page is a real GRAPHICS.DAT frame in
     * the same temporary entrance session.  Retain its presentation receipt
     * and require a return to the real C004/C002/C003 page before F0807
     * hands off to the dungeon. C005 itself remains an optional command. */
    int credits_scene_presented;
    int credits_return_presented;
    int door_frame_presented;
    int last_door_opening_step;
    int next_door_opening_step;
    int entrance_special_palette;
    uint32_t credits_source_tick;
    uint32_t credits_frame_route_hash;
    uint32_t credits_raster_hash;
    uint32_t credits_return_source_tick;
    uint32_t credits_return_frame_route_hash;
    uint32_t credits_return_raster_hash;
    int no_fallback_routes;
} CSB_V1_StartupPlaybackState_PC34;

/* One verified startup session owns every source image for the complete
 * FTL -> title -> entrance -> HUD handoff.  ReDMCSB keeps C001 resident
 * while TITLE.C changes zones, then keeps C002-C005 resident through the
 * entrance loop; CSBWin's Graphics.cpp ReadGraphic keeps the archive read
 * boundary separate from its consumers. */
struct CSB_V1_StartupRuntimeAssetSession_PC34 {
    int valid;
    int real_asset_matched;
    int title_assets_ready;
    int title_presents_ready;
    int title_chaos_ready;
    int title_strikes_back_ready;
    int entrance_assets_ready;
    int door_assets_ready;
    int hud_assets_bound;
    int full_startup_ready;
    int rejects_legacy_wrappers;
    uint32_t source_tick;
    uint32_t generation;
    uint32_t csbSaveCandidateIdentity;
    uint32_t csbStartupPackageIdentity;
    /* LOADSAVE.C F0435 enters live dungeon state without replaying the
     * TITLE.C/ENTRANCE.C terminal surface transaction. */
    int direct_resume_loaded;
    CSB_V1_StartupAssetBinding_PC34 hud_inventory_binding;
    CSB_V1_StartupAssetBinding_PC34 hud_resurrect_binding;
    int csbgraphics_palette_receipt_ready;
    uint32_t csbgraphics_palette_receipt_hash;
    uint32_t hud_source_receipt_hash;
    CSB_V1_StartupRuntimeSurfaceSet_PC34 surfaces;
    CSB_V1_StartupPlaybackState_PC34 playback;
};

typedef enum CSB_V1_BootStartupDoorRuntimeRoute_PC34 {
    CSB_V1_BOOT_STARTUP_DOOR_RUNTIME_ROUTE_NONE_PC34 = 0,
    CSB_V1_BOOT_STARTUP_DOOR_RUNTIME_ROUTE_OPENING_PC34,
    CSB_V1_BOOT_STARTUP_DOOR_RUNTIME_ROUTE_RUNTIME_BLOCKED_PC34,
    CSB_V1_BOOT_STARTUP_DOOR_RUNTIME_ROUTE_HUD_READY_PC34
} CSB_V1_BootStartupDoorRuntimeRoute_PC34;

/* The prison-door boundary is a runtime handoff, not a new render callback.
 * ReDMCSB ENTRANCE.C F0806 lines 857-889 leaves the entrance loop only after
 * the door sequence, and CSBWin CSBCode.cpp lines 9515-9535 follows the same
 * order: complete OpenPrisonDoors, release temporary entrance memory, then
 * return to the game runtime. */
typedef struct CSB_V1_BootStartupDoorRuntimeReceipt_PC34 {
    int valid;
    CSB_V1_BootStartupDoorRuntimeRoute_PC34 route;
    int door_opening_finished;
    int runtime_view_ready;
    int hud_session_ready;
    CSB_V1_StartupIdleReceipt_PC34 idle;
    CSB_V1_RuntimeM11MirrorReceipt_PC34 runtime_mirror;
    const char *status_scope;
    const char *status;
} CSB_V1_BootStartupDoorRuntimeReceipt_PC34;

/* One strict runtime-data proof for the complete CSB start.  Consumers may
 * still request a single frame, but the session is only accepted after C001
 * title regions, C002/C003 opening doors, entrance art and HUD bindings all
 * come from the verified CSB data owner. */
typedef struct CSB_V1_StartupFullRuntimeReceipt_PC34 {
    int valid;
    int real_asset_matched;
    int title_sequence_ready;
    int title_presents_ready;
    int title_chaos_ready;
    int title_strikes_back_ready;
    int entrance_ready;
    int hud_ready;
    int door_ready;
    int playback_route_ready;
    int playback_reaches_title;
    int playback_reaches_entrance;
    int playback_reaches_hud;
    int title_to_hud_same_session;
    int no_legacy_wrappers;
    uint32_t session_generation;
    uint32_t playback_route_hash;
    const char *source_evidence;
} CSB_V1_StartupFullRuntimeReceipt_PC34;

typedef struct CSB_V1_StartupCompleteSupportReceipt_PC34 {
    int valid;
    int full_runtime_valid;
    int host_capture_gate_valid;
    int real_asset_matched;
    int title_sequence_ready;
    int title_phase_route_complete;
    int title_presents_ready;
    int title_chaos_ready;
    int title_strikes_back_ready;
    int entrance_ready;
    int hud_ready;
    int door_ready;
    int playback_route_ready;
    int title_to_hud_same_session;
    int runtime_host_routes_ready;
    int draw_consumes_receipt_only;
    int input_consumes_receipt_only;
    int no_legacy_wrappers;
    int no_fallback_callbacks;
    int no_wrapper_fallback_routes;
    int host_route_wrappers_retired;
    int no_loose_render_plan_exports;
    int real_startup_assets_bound;
    int title_runtime_phase_mask;
    int title_runtime_expected_phase_mask;
    int title_runtime_phase_hash_count;
    uint32_t title_runtime_phase_hashes[CSB_V1_BOOT_STARTUP_TITLE_SAMPLE_COUNT_PC34];
    uint32_t title_runtime_phase_hash;
    uint32_t credits_packaged_capture_hash;
    uint32_t real_startup_asset_binding_hash;
    uint32_t session_generation;
    uint32_t playback_route_hash;
    uint32_t runtime_host_gate_hash;
    uint32_t complete_support_hash;
    CSB_V1_StartupFullRuntimeReceipt_PC34 full_runtime;
    CSB_V1_BootStartupRuntimeHostCaptureGateReceipt_PC34 host_capture_gate;
    const char *source_evidence;
} CSB_V1_StartupCompleteSupportReceipt_PC34;

typedef struct CSB_V1_StartupReleaseAppCaptureReceipt_PC34 {
    int valid;
    int complete_support_valid;
    int host_capture_gate_valid;
    int release_app_capture_ready;
    int title_release_app_capture_ready;
    int closed_door_release_app_capture_ready;
    int utility_release_app_capture_ready;
    int door_opening_release_app_capture_ready;
    int credits_release_app_capture_ready;
    int title_sequence_capture_ready;
    int title_sequence_host_consumer_ready;
    int title_sequence_same_capture_route;
    int title_host_consumer_ready;
    int closed_door_host_consumer_ready;
    int utility_host_consumer_ready;
    int door_opening_host_consumer_ready;
    int credits_host_consumer_ready;
    int route_specific_host_consumers_ready;
    int hud_door_capture_ready;
    int hud_door_host_consumers_ready;
    int hud_door_same_capture_route;
    int title_phase_route_complete;
    int runtime_host_routes_ready;
    int draw_consumes_receipt_only;
    int input_consumes_receipt_only;
    int no_fallback_callbacks;
    int no_wrapper_fallback_routes;
    int host_route_wrappers_retired;
    int no_loose_render_plan_exports;
    int release_app_real_asset_capture_ready;
    int full_runtime_real_asset_matched;
    int host_runtime_visual_real_asset_matched;
    int real_startup_assets_bound;
    int title_runtime_phase_mask;
    int title_runtime_expected_phase_mask;
    int title_runtime_phase_hash_count;
    uint32_t title_runtime_phase_hashes[CSB_V1_BOOT_STARTUP_TITLE_SAMPLE_COUNT_PC34];
    uint32_t title_runtime_phase_hash;
    uint32_t title_packaged_capture_hash;
    uint32_t title_sequence_capture_hash;
    uint32_t closed_door_packaged_capture_hash;
    uint32_t utility_packaged_capture_hash;
    uint32_t door_opening_packaged_capture_hash;
    uint32_t credits_packaged_capture_hash;
    uint32_t hud_door_capture_hash;
    uint32_t release_app_real_asset_capture_hash;
    uint32_t runtime_host_gate_hash;
    uint32_t complete_support_hash;
    uint32_t release_app_capture_hash;
    CSB_V1_StartupCompleteSupportReceipt_PC34 complete_support;
    const char *source_evidence;
} CSB_V1_StartupReleaseAppCaptureReceipt_PC34;

typedef struct CSB_V1_StartupPresentedAppCaptureFacts_PC34 {
    int running_from_macos_app_bundle;
    int mac_window_capture_ready;
    int presented_frame_captured;
    int presented_frame_width;
    int presented_frame_height;
    int presented_frame_indexed_pixels;
    int presented_frame_uses_real_csb_assets;
    uint32_t presented_frame_hash;
    uint32_t presented_frame_route_hash;
} CSB_V1_StartupPresentedAppCaptureFacts_PC34;

typedef struct CSB_V1_StartupPresentedAppCaptureReceipt_PC34 {
    int valid;
    int release_app_capture_valid;
    int running_from_macos_app_bundle;
    int mac_window_capture_ready;
    int presented_frame_captured;
    int presented_frame_geometry_ready;
    int presented_frame_pixels_ready;
    int presented_frame_real_asset_ready;
    int presented_frame_route_hash_ready;
    int presented_runtime_capture_boundary_ready;
    int presented_title_sequence_ready;
    int presented_title_phase_mask_ready;
    int presented_hud_door_ready;
    int presented_hud_door_route_hash_ready;
    int presented_credits_ready;
    int presented_credits_route_hash_ready;
    int presented_route_aggregates_ready;
    int presented_wrapper_cleanup_ready;
    uint32_t release_app_capture_hash;
    uint32_t title_sequence_capture_hash;
    uint32_t hud_door_capture_hash;
    uint32_t credits_capture_hash;
    uint32_t presented_wrapper_cleanup_hash;
    uint32_t presented_frame_hash;
    uint32_t presented_frame_route_hash;
    uint32_t presented_app_capture_hash;
    CSB_V1_StartupReleaseAppCaptureReceipt_PC34 release_app_capture;
    const char *source_evidence;
} CSB_V1_StartupPresentedAppCaptureReceipt_PC34;

/* A non-owning frame view into a startup asset session.  The source pointers
 * remain valid until csb_v1_boot_startup_runtime_asset_session_release_pc34.
 * No fallback text or synthetic door pixels are represented here. */
typedef struct CSB_V1_StartupRuntimeAssetFrame_PC34 {
    int valid;
    int real_asset_matched;
    int title_sequence_ready;
    int entrance_ready;
    int door_ready;
    int no_legacy_wrappers;
    int title_phase_mask;
    /* TITLE.C F0437 selects a distinct indexed palette for each C001 phase.
     * Retain the source plan's selection with the resident bitmap so host
     * presentation cannot pair a valid raster with a different palette. */
    int special_palette;
    int title_special_palette;
    uint32_t frame_route_hash;
    uint32_t hud_inventory_pixel_hash;
    uint32_t hud_resurrect_pixel_hash;
    uint32_t hud_source_receipt_hash;
    uint32_t hud_binding_hash;
    uint32_t source_tick;
    uint32_t session_generation;
    CSB_V1_StartupStage_PC34 stage;
    int title_phase_tick;
    int title_phase_tick_count;
    const CSB_V1_StartupRuntimeSurface_PC34 *title_surface;
    const CSB_V1_StartupRuntimeSurface_PC34 *entrance_surface;
    const CSB_V1_StartupRuntimeSurface_PC34 *left_door_surface;
    const CSB_V1_StartupRuntimeSurface_PC34 *right_door_surface;
    const CSB_V1_StartupRuntimeSurface_PC34 *hud_inventory_surface;
    const CSB_V1_StartupRuntimeSurface_PC34 *hud_resurrect_surface;
    int opening_step;
    int uses_verified_hud_bindings;
} CSB_V1_StartupRuntimeAssetFrame_PC34;

typedef struct {
    int valid;
    int c001_complete;
    int terminal_f0807_complete;
    int hud_session_ready;
    int c017_ready;
    int c040_ready;
    int no_legacy_wrapper;
    int no_fallback_route;
    uint32_t source_tick;
    uint32_t session_generation;
} CSB_V1_StartupCompleteTimelineReceipt_PC34;

typedef struct {
    int valid;
    int c040_cleared_once;
    int post_c101_presented;
    int live_c017_only_panel_base;
    int palette_neutral;
    int no_legacy_wrapper;
    int no_cast_or_combat;
    int c017_source_asset_id;
    int c017_width;
    int c017_height;
    int c017_special_palette;
    uint32_t source_tick;
    uint32_t session_generation;
} CSB_V1_TerminalUiStateReceipt_PC34;

/* An owned indexed startup frame composed from verified GRAPHICS.DAT
 * surfaces. ReDMCSB TITLE.C F0437 and ENTRANCE.C F0438/F0807 perform this
 * composition before presentation, keeping it out of legacy callbacks. */
#define CSB_V1_STARTUP_RUNTIME_RASTER_WIDTH_PC34 320
#define CSB_V1_STARTUP_RUNTIME_RASTER_HEIGHT_PC34 200
#define CSB_V1_STARTUP_HUD_INVENTORY_WIDTH_PC34 224
#define CSB_V1_STARTUP_HUD_INVENTORY_HEIGHT_PC34 136
#define CSB_V1_STARTUP_HUD_RESURRECT_WIDTH_PC34 144
#define CSB_V1_STARTUP_HUD_RESURRECT_HEIGHT_PC34 73
#define CSB_V1_STARTUP_HUD_RESURRECT_TRANSPARENT_COLOR_PC34 6

typedef struct CSB_V1_StartupRuntimeRaster_PC34 {
    unsigned char *pixels;
    int width;
    int height;
    int valid;
    int real_asset_matched;
    int title_composited;
    int entrance_composited;
    int door_composited;
    int source_surface_count;
    uint32_t pixel_hash;
    uint32_t route_hash;
} CSB_V1_StartupRuntimeRaster_PC34;

/* A concrete CSB host-surface decision, backed only by the owning PC34
 * startup session.  TITLE.C F0437 presents C001 from its resident source;
 * ENTRANCE.C F0806 retains C002-C005 until the entrance loop exits; and
 * DUNVIEW.C F0111 selects real door-frame pixels for an opening state. */
typedef enum CSB_V1_StartupRuntimeHostSurface_PC34 {
    CSB_V1_STARTUP_RUNTIME_HOST_SURFACE_NONE_PC34 = 0,
    CSB_V1_STARTUP_RUNTIME_HOST_SURFACE_TITLE_PC34,
    CSB_V1_STARTUP_RUNTIME_HOST_SURFACE_ENTRANCE_PC34,
    CSB_V1_STARTUP_RUNTIME_HOST_SURFACE_CREDITS_PC34,
    CSB_V1_STARTUP_RUNTIME_HOST_SURFACE_DOOR_OPENING_PC34,
    CSB_V1_STARTUP_RUNTIME_HOST_SURFACE_HUD_PC34
} CSB_V1_StartupRuntimeHostSurface_PC34;

typedef struct CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 {
    int valid;
    int real_asset_matched;
    int no_legacy_wrappers;
    int no_synthetic_surface;
    CSB_V1_StartupRuntimeHostSurface_PC34 host_surface;
    int door_opening_decision;
    int runtime_hud_decision;
    int uses_c017_inventory;
    int uses_c040_resurrect;
    int special_palette;
    int title_special_palette;
    uint32_t host_surface_hash;
    CSB_V1_StartupRuntimeAssetFrame_PC34 frame;
    CSB_V1_StartupRuntimeRaster_PC34 raster;
    const char *source_evidence;
} CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34;

/* ENTRANCE.C F0438/F0807 presents all 31 C002/C003 door positions over C004.
 * Keep the direct session capture as a concrete raster consumer so a host
 * cannot replace the sequence with a single sampled page or a wrapper route. */
#define CSB_V1_STARTUP_DOOR_OPENING_CAPTURE_FRAME_COUNT_PC34 31

typedef struct CSB_V1_StartupDoorOpeningCaptureReceipt_PC34 {
    int valid;
    int real_asset_matched;
    int no_legacy_wrappers;
    int no_synthetic_surface;
    int source_step_count;
    int captured_frame_count;
    int first_step;
    int last_step;
    uint32_t session_generation;
    uint32_t frame_route_hashes[
        CSB_V1_STARTUP_DOOR_OPENING_CAPTURE_FRAME_COUNT_PC34];
    uint32_t raster_pixel_hashes[
        CSB_V1_STARTUP_DOOR_OPENING_CAPTURE_FRAME_COUNT_PC34];
    uint32_t capture_sequence_hash;
    const char *source_evidence;
} CSB_V1_StartupDoorOpeningCaptureReceipt_PC34;

/* PANEL.C F0347 first restores C017 at (0,33), then F0346 optionally
 * composites C040 at panel-relative (80,52) with transparency key 6. Keep
 * the resulting destination update in CSB so M11 never reads startup
 * surfaces or recreates the composition itself. */
typedef struct CSB_V1_StartupRuntimeHudPanelReceipt_PC34 {
    int valid;
    int real_asset_matched;
    int c017_presented;
    int c040_presented;
    int no_legacy_wrappers;
    int no_synthetic_surface;
    uint32_t source_tick;
    uint32_t session_generation;
    uint32_t c017_pixel_hash;
    uint32_t c040_pixel_hash;
    uint32_t panel_hash;
    const char *source_evidence;
} CSB_V1_StartupRuntimeHudPanelReceipt_PC34;

/* Runtime-only startup presentation.  This is the CSB boundary for title,
 * entrance/HUD, utility, and opening-door plans when verified game data is
 * present.  The older snapshot helpers remain inspection adapters. */
typedef struct CSB_V1_BootStartupRuntimePresentationReceipt_PC34 {
    int valid;
    int asset_gate_valid;
    int render_plan_uses_owned_assets;
    int utility_plan_uses_owned_session;
    int door_plan_has_no_fallback;
    CSB_V1_BootStartupRuntimeAssetGateReceipt_PC34 asset_gate;
    CSB_V1_BootStartupM11PresentationReceipt_PC34 presentation;
    CSB_V1_StartupRuntimeSurfaceSet_PC34 surfaces;
} CSB_V1_BootStartupRuntimePresentationReceipt_PC34;

/* CSB-owned save/import handoff for runtime consumers.  ReDMCSB keeps
 * save/load under LOADSAVE.C F0433/F0435 while the CSB utility/import path
 * enters from ENTRANCE.C F0806; CSBWin adds the CSBGAME.DAT/.BAK filename
 * surface.  Keep those facts behind boot so M11 does not infer them from
 * paths or menu labels. */
typedef struct CSB_V1_BootRuntimeSaveImportReceipt_PC34 {
    int valid;
    int boot_profile_ready;
    int runtime_ready;
    int save_root_bound;
    char save_root[CSB_V1_BOOT_STARTUP_RESUME_PATH_CAP_PC34];
    int save_adapter_available;
    int load_adapter_available;
    int tick_adapter_available;
    int resume_path_present;
    char resume_path[CSB_V1_BOOT_STARTUP_RESUME_PATH_CAP_PC34];
    int dm1_import_path_present;
    char dm1_import_path[CSB_V1_BOOT_STARTUP_RESUME_PATH_CAP_PC34];
    int imported_party_ready;
    int cmp_import_attempted;
    int cmp_import_succeeded;
    int cmp_imported_slot;
    int cmp_imported_champion_count;
    int csbwin_path_present;
    char csbwin_path[CSB_V1_BOOT_STARTUP_RESUME_PATH_CAP_PC34];
    int csbwin_filename_candidate;
    int csbwin_should_attempt_import;
    int csbwin_loader_code;
    int csbwin_contract_match;
    CSB_V1_CSBWinSaveShape csbwin_shape;
    CSB_V1_CSBWinSaveFileKind csbwin_file_kind;
    const char *csbwin_decision_label;
    int csbwin_dsa_corpus_positive;
    int csbwin_dsa_runtime_handoff_ready;
    int csbwin_dsa_extended_tail_valid;
    int csbwin_dsa_section_valid;
    int csbwin_dsa_has_runtime_actions;
    int csbwin_dsa_gameblock1_valid;
    int csbwin_dsa_gameblock1_body_valid;
    uint32_t csbwin_dsa_save_bytes_fnv1a;
    uint32_t csbwin_dsa_gameblock1_body_fnv1a;
    const char *csbwin_dsa_decision_label;
    int csbwin_runtime_load_attempted;
    int csbwin_runtime_load_succeeded;
    int csbwin_runtime_load_code;
    int runtime_party_loaded_after;
    int runtime_import_source_after;
    int runtime_champion_count_after;
    int runtime_leader_index_after;
    int runtime_current_level_after;
    uint32_t runtime_game_time_after;
    int runtime_party_x_after;
    int runtime_party_y_after;
    int runtime_party_dir_after;
    const char *source_evidence;
} CSB_V1_BootRuntimeSaveImportReceipt_PC34;

/* CSBWin DSA runtime handoff is narrower than the CSBGAME loader path:
 * a staged file may be loader-ready while still not proving Extended Features
 * DSA state.  Runtime consumers must ask for this receipt before treating a
 * save import as DSA-bearing runtime evidence. */
typedef struct CSB_V1_BootRuntimeDSASaveHandoffReceipt_PC34 {
    int valid;
    int save_import_receipt_consumed;
    int runtime_load_consumed;
    int dsa_corpus_positive;
    int dsa_runtime_handoff_ready;
    int extended_tail_valid;
    int dsa_section_valid;
    int dsa_has_runtime_actions;
    int gameblock1_valid;
    int gameblock1_body_valid;
    uint32_t save_bytes_fnv1a;
    uint32_t gameblock1_body_fnv1a;
    int runtime_party_loaded;
    int runtime_import_source_after;
    int runtime_champion_count_after;
    int runtime_current_level_after;
    uint32_t runtime_game_time_after;
    const char *decision_label;
    const char *source_evidence;
} CSB_V1_BootRuntimeDSASaveHandoffReceipt_PC34;

/* ReDMCSB LOADSAVE.C F0435 owns the native CSB save load.  Keep this
 * separate from CSBWin/roster fallbacks so a package capture can only claim
 * an original-save runtime handoff after the native header and live runtime
 * both agree. */
typedef struct CSB_V1_BootOriginalSaveRuntimeReceipt_PC34 {
    int valid;
    int boot_profile_ready;
    int native_csb_header_valid;
    int runtime_load_succeeded;
    int runtime_dungeon_ready;
    int runtime_party_ready;
    char save_path[CSB_V1_BOOT_STARTUP_RESUME_PATH_CAP_PC34];
    int runtime_current_level_after;
    int runtime_champion_count_after;
    uint32_t runtime_game_time_after;
    uint32_t native_header_fnv1a;
    char dungeon_md5[33];
    uint32_t source_identity_hash;
    const char *source_evidence;
} CSB_V1_BootOriginalSaveRuntimeReceipt_PC34;

int csb_v1_boot_runtime_load_original_save_receipt_pc34(
    CSB_V1_BootProfile *profile, const char *path,
    CSB_V1_BootOriginalSaveRuntimeReceipt_PC34 *out_receipt);

/* Re-read the native F0435 header and bind it to the still-current verified
 * Dungeon.dat identity. This is observational: it never changes runtime
 * state or resumes the save. */
int csb_v1_boot_original_save_runtime_receipt_current_pc34(
    const CSB_V1_BootProfile *profile,
    const CSB_V1_BootOriginalSaveRuntimeReceipt_PC34 *receipt);

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
    int primitive_commands_consumed;
    int title_asset_commands_consumed;
    int closed_door_asset_commands_consumed;
    int opening_frame_command_consumed;
    int fallback_text_suppressed;
    int fallback_callbacks_stripped;
    int consumed_host_view_only;
    int render_draw_receipt_consumed;
    int capture_proof_consumed;
    int route_capture_proof_consumed;
    int readiness_receipt_consumed;
    int no_legacy_plan_fallback;
    const char *source_evidence;
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
    int host_route_wrappers_retired;
    int no_loose_render_plan_exports;
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
/* Restored non-static contract (was exported at 4b7aed6da, lost to the
 * worktree merge drift); the M11 startup host-view probe consumes it. */
int csb_v1_boot_startup_runtime_route_hardening_receipt_from_ownership_pc34(
    const CSB_V1_BootStartupVisualSequenceCaptureReceipt_PC34 *visual_sequence,
    const CSB_V1_BootStartupHostOwnershipReceipt_PC34 *ownership,
    CSB_V1_BootStartupRuntimeRouteHardeningReceipt_PC34 *out_receipt);
void csb_v1_boot_startup_runtime_host_capture_gate_receipt_init_pc34(
    CSB_V1_BootStartupRuntimeHostCaptureGateReceipt_PC34 *receipt);
void csb_v1_boot_startup_render_draw_receipt_init_pc34(
    CSB_V1_BootStartupRenderDrawReceipt_PC34 *receipt);
void csb_v1_boot_startup_host_view_receipt_init_pc34(
    CSB_V1_BootStartupHostViewReceipt_PC34 *receipt);
void csb_v1_boot_startup_m11_presentation_receipt_init_pc34(
    CSB_V1_BootStartupM11PresentationReceipt_PC34 *receipt);
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
int csb_v1_boot_load_swoosh_source_pc34(CSB_V1_BootProfile *profile);
int csb_v1_boot_profile_from_startup_real_receipt_pc34(
    const CSB_V1_StartupRealReceipt *receipt, CSB_V1_BootProfile *out_profile);

/* Resolve startup artwork from a verified CSB boot profile. Original
 * GRAPHICS.DAT supplies C001-C005; a validated CSBgraphics.dat may replace
 * only the known HUD entries (17/40). CSB render-receipt consumers use this
 * CSB-owned selection to reject generic/test artwork for a real profile. */
void csb_v1_boot_startup_assets_resolve_pc34(CSB_V1_BootProfile *profile);
const CSB_V1_StartupAssetBinding_PC34 *
csb_v1_boot_startup_asset_binding_pc34(
    const CSB_V1_BootProfile *profile,
    CSB_V1_StartupAssetRole_PC34 role);
int csb_v1_boot_startup_render_plan_uses_real_assets_pc34(
    const CSB_V1_BootProfile *profile,
    const CSB_V1_StartupRenderPlan_PC34 *plan);
void csb_v1_boot_startup_runtime_asset_gate_receipt_init_pc34(
    CSB_V1_BootStartupRuntimeAssetGateReceipt_PC34 *receipt);
int csb_v1_boot_startup_runtime_asset_gate_from_launch_receipts_pc34(
    const CSB_V1_BootProfile *profile,
    const CSB_V1_BootStartupLaunchReceipts_PC34 *launch_receipts,
    CSB_V1_BootStartupRuntimeAssetGateReceipt_PC34 *out_receipt);
void csb_v1_boot_startup_runtime_surface_set_release_pc34(
    CSB_V1_StartupRuntimeSurfaceSet_PC34 *surfaces);
void csb_v1_boot_startup_runtime_asset_session_init_pc34(
    CSB_V1_StartupRuntimeAssetSession_PC34 *session);
/* Decode one original PC3.4 GRAPHICS.DAT entry through CSB's IMG3/LZW
 * stream, rather than the DM1 packed-nibble asset path. The returned pixels
 * are caller-owned and must be released with free(). */
int csb_v1_boot_decode_graphics_dat_asset_pc34(
    const char *path, unsigned int graphic_index,
    unsigned char **out_pixels, int *out_width, int *out_height,
    CSB_V1_StartupGraphicDecodeReceipt_PC34 *out_decode_receipt);
/* Decode the raw PC3.4 GRAPHICS.DAT member selected by F0490.  This is for
 * non-raster records such as ReDMCSB OBJECT.C F0031's M564 name stream. */
int csb_v1_graphics_decode_raw_entry_pc34(
    const uint8_t *file_bytes, size_t file_size, unsigned int entry_index,
    uint8_t *out, size_t out_capacity, size_t *out_size);
/* Decode an original Atari ST DMCSB1 GRAPHICS.DAT item through the same
 * CSBWin ExpandGraphic-compatible source decoder used for PC startup
 * records.  This is for standard CSBWin packages which retain the original
 * ST graphics/dungeon pair but do not ship ANIMATE.SCR/ANIMATE.DAT. */
int csb_v1_boot_decode_atari_st_graphics_dat_asset_pc34(
    const char *path, unsigned int graphic_index,
    unsigned char **out_pixels, int *out_width, int *out_height,
    CSB_V1_StartupGraphicDecodeReceipt_PC34 *out_decode_receipt);
int csb_v1_boot_startup_runtime_asset_session_open_pc34(
    const CSB_V1_BootProfile *profile,
    CSB_V1_StartupRuntimeAssetSession_PC34 *out_session);
void csb_v1_boot_startup_runtime_asset_session_release_pc34(
    CSB_V1_StartupRuntimeAssetSession_PC34 *session);
int csb_v1_boot_startup_runtime_asset_session_frame_pc34(
    CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    const CSB_V1_StartupRenderPlan_PC34 *plan,
    uint32_t source_tick,
    CSB_V1_StartupRuntimeAssetFrame_PC34 *out_frame);
void csb_v1_boot_startup_runtime_raster_release_pc34(
    CSB_V1_StartupRuntimeRaster_PC34 *raster);
int csb_v1_boot_startup_runtime_frame_rasterize_pc34(
    const CSB_V1_StartupRuntimeAssetFrame_PC34 *frame,
    const CSB_V1_StartupRenderPlan_PC34 *plan,
    CSB_V1_StartupRuntimeRaster_PC34 *out_raster);
/* ReDMCSB ENTRANCE.C F0439/F0438 first composes the real C004 interface,
 * then copies the F0128 224x136 viewport at (0,33), and finally overlays
 * C002/C003. The viewport receipt is required so callers cannot supply a
 * host/generated surface in place of a decoder-bound F0128 raster. */
int csb_v1_boot_startup_entrance_f0128_raster_compose_pc34(
    const CSB_V1_StartupRuntimeAssetFrame_PC34 *frame,
    const CSB_V1_StartupRenderPlan_PC34 *plan,
    const CSB_V1_ViewportFirstFrameRasterReceiptPc34 *viewport_receipt,
    const uint8_t *viewport_pixels,
    size_t viewport_pixel_count,
    CSB_V1_StartupRuntimeRaster_PC34 *out_raster);
int csb_v1_boot_startup_runtime_hud_frame_rasterize_pc34(
    const CSB_V1_StartupRuntimeAssetFrame_PC34 *frame,
    int include_resurrection_panel,
    CSB_V1_StartupRuntimeRaster_PC34 *out_raster);
int csb_v1_boot_terminal_ui_state_host_raster_pc34(
    const CSB_V1_TerminalUiStateReceipt_PC34 *terminal_ui,
    const CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    const CSB_V1_StartupRuntimeAssetFrame_PC34 *frame,
    int include_resurrection_panel,
    CSB_V1_StartupRuntimeRaster_PC34 *out_raster);
int csb_v1_boot_startup_runtime_hud_panel_blit_from_session_pc34(
    CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    int draw_resurrect_panel,
    unsigned char *destination,
    int destination_width,
    int destination_height,
    CSB_V1_StartupRuntimeHudPanelReceipt_PC34 *out_receipt);
void csb_v1_boot_startup_runtime_host_surface_receipt_release_pc34(
    CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 *receipt);
int csb_v1_boot_startup_runtime_host_surface_receipt_from_session_pc34(
    CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    const CSB_V1_StartupRenderPlan_PC34 *plan,
    uint32_t source_tick,
    CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 *out_receipt);
/* Production Entrance consumer for the F0439/F0438 C004 path. The admitted
 * F0128 viewport replaces only the source viewport rectangle before the
 * decoded C002/C003 strips are restored; title and credits never enter it. */
int csb_v1_boot_startup_runtime_entrance_f0128_receipt_from_session_pc34(
    CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    const CSB_V1_StartupRenderPlan_PC34 *plan,
    uint32_t source_tick,
    const CSB_V1_ViewportFirstFrameRasterReceiptPc34 *viewport_receipt,
    const uint8_t *viewport_pixels,
    size_t viewport_pixel_count,
    CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 *out_receipt);
int csb_v1_boot_startup_door_opening_capture_from_session_pc34(
    CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    uint32_t first_source_tick,
    CSB_V1_StartupDoorOpeningCaptureReceipt_PC34 *out_receipt);
/* Verify that the framebuffer handed to M11 is the exact indexed raster
 * composed by the currently owned PC3.4 startup plan.  This closes the
 * title/entrance capture boundary: a valid session alone cannot promote a
 * stale title phase, a door strip from another step, or a host wrapper page. */
int csb_v1_boot_startup_runtime_host_surface_matches_indexed_frame_pc34(
    CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    const CSB_V1_StartupRenderPlan_PC34 *plan,
    uint32_t source_tick,
    const unsigned char *indexed_pixels,
    int width,
    int height,
    int special_palette);
int csb_v1_boot_startup_full_runtime_receipt_from_session_pc34(
    const CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    CSB_V1_StartupFullRuntimeReceipt_PC34 *out_receipt);
void csb_v1_boot_startup_complete_support_receipt_init_pc34(
    CSB_V1_StartupCompleteSupportReceipt_PC34 *receipt);
int csb_v1_boot_startup_complete_support_receipt_from_runtime_and_host_pc34(
    const CSB_V1_StartupFullRuntimeReceipt_PC34 *full_runtime,
    const CSB_V1_BootStartupRuntimeHostCaptureGateReceipt_PC34 *host_capture_gate,
    CSB_V1_StartupCompleteSupportReceipt_PC34 *out_receipt);
void csb_v1_boot_startup_release_app_capture_receipt_init_pc34(
    CSB_V1_StartupReleaseAppCaptureReceipt_PC34 *receipt);
int csb_v1_boot_startup_release_app_capture_receipt_from_complete_support_pc34(
    const CSB_V1_StartupCompleteSupportReceipt_PC34 *complete_support,
    CSB_V1_StartupReleaseAppCaptureReceipt_PC34 *out_receipt);
void csb_v1_boot_startup_presented_app_capture_receipt_init_pc34(
    CSB_V1_StartupPresentedAppCaptureReceipt_PC34 *receipt);
int csb_v1_boot_startup_presented_app_capture_facts_from_indexed_frame_pc34(
    const CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    const unsigned char *indexed_pixels,
    int width,
    int height,
    int running_from_macos_app_bundle,
    int mac_window_capture_ready,
    CSB_V1_StartupPresentedAppCaptureFacts_PC34 *out_facts);
int csb_v1_boot_startup_presented_app_capture_receipt_from_release_pc34(
    const CSB_V1_StartupReleaseAppCaptureReceipt_PC34 *release_app_capture,
    const CSB_V1_StartupPresentedAppCaptureFacts_PC34 *presented_facts,
    CSB_V1_StartupPresentedAppCaptureReceipt_PC34 *out_receipt);
/* ReDMCSB SWSH.C F0909/F0910 owns the pre-title sound, TITLE.C F0437 owns
 * the title timing, and ENTRANCE.C F0806 starts C0_MUSIC_ENTRANCE. */
int csb_v1_boot_startup_playback_begin_pc34(
    CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    CSB_V1_StartupAudioAction_PC34 *out_audio_action);
int csb_v1_boot_startup_playback_complete_swoosh_pc34(
    CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    CSB_V1_StartupAudioAction_PC34 *out_audio_action);
/* TITLE.C F0437 may advance only from the currently presented C001 plan.
 * A stale Entrance or transfer plan must leave the title transaction intact. */
int csb_v1_boot_startup_playback_accepts_title_plan_pc34(
    const CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    const CSB_V1_StartupRenderPlan_PC34 *plan,
    int title_frame);
int csb_v1_boot_startup_playback_title_frame_pc34(
    CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    int title_frame,
    CSB_V1_StartupRenderPlan_PC34 *out_plan,
    CSB_V1_StartupAudioAction_PC34 *out_audio_action);
int csb_v1_boot_startup_title_capture_plan_admit_pc34(
    const CSB_V1_StartupRenderPlan_PC34 *plan,
    int title_frame);
int csb_v1_boot_startup_playback_complete_entrance_pc34(
    CSB_V1_StartupRuntimeAssetSession_PC34 *session);
int csb_v1_boot_startup_playback_enter_hud_pc34(
    CSB_V1_StartupRuntimeAssetSession_PC34 *session);
int csb_v1_boot_startup_complete_timeline_receipt_from_session_pc34(
    const CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    CSB_V1_StartupCompleteTimelineReceipt_PC34 *out_receipt);
int csb_v1_boot_startup_authorize_live_hud_pc34(
    CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    CSB_V1_StartupCompleteTimelineReceipt_PC34 *out_receipt);
int csb_v1_boot_startup_live_hud_terminal_receipt_current_pc34(
    const CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    uint32_t terminal_source_tick,
    uint32_t terminal_generation);
const char *csb_v1_boot_startup_asset_source_name_pc34(
    CSB_V1_StartupAssetSource_PC34 source);
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
int csb_v1_boot_csbgraphics_inventory_receipt_ready(
    const CSB_V1_BootProfile *profile);
const CSB_V1_CSBGraphicsInventory *
csb_v1_boot_csbgraphics_inventory(const CSB_V1_BootProfile *profile);
int csb_v1_boot_admit_csbgraphics_palette_candidate(
    CSB_V1_BootProfile *profile,
    const CSB_V1_CSBGraphicsDatPaletteAdmissionSpec *spec);
int csb_v1_boot_csbgraphics_palette_receipt_ready(
    const CSB_V1_BootProfile *profile);
int csb_v1_boot_startup_csbgraphics_palette_readiness_pc34(
    const CSB_V1_BootProfile *profile,
    CSB_V1_BootStartupCSBGraphicsPaletteReadiness_PC34 *out_receipt);
const CSB_V1_CSBGraphicsRuntimePlan *
csb_v1_boot_csbgraphics_runtime_plan(const CSB_V1_BootProfile *profile);
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
/* Source-bound C49/F0219 admission for the live boot profile.  The request
 * succeeds only while the exact C14 remains in the resolved F0161 list. */
int csb_v1_boot_admit_post_teleport_projectile_impact_pc34(
    CSB_V1_BootProfile *profile,
    int map_index,
    int map_x,
    int map_y,
    uint16_t projectile_thing,
    int projectile_aspect_ordinal,
    int side,
    int coordinate_set,
    CSB_V1_F0219ProjectileImpactMaterialHandoffPc34 *out_handoff);

/* Consume transient, source-owned F0219 C05 receipts from the embedded live
 * runtime before F0128 renders.  The function derives only viewer geometry;
 * it never manufactures C14 ownership, a bitmap, or a marker fallback. */
int csb_v1_boot_sync_post_teleport_projectile_runtime_pc34(
    CSB_V1_BootProfile *profile);
int csb_v1_boot_first_live_dungeon_frame_receipt_from_session_pc34(
    const CSB_V1_BootProfile *profile,
    const CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    const CSB_V1_ViewportRuntimeDrawCounts *draw_counts,
    const unsigned char *framebuffer,
    int framebuffer_width,
    int framebuffer_height,
    CSB_V1_FirstLiveDungeonFrameReceipt_PC34 *out_receipt);
int csb_v1_boot_project_verified_dungeon_ingress_pc34(
    const CSB_V1_FirstLiveDungeonFrameReceipt_PC34 *receipt,
    CSB_V1_ViewportVerifiedDungeonIngressPc34 *out_ingress);
/* The boot-owner live material route accepts only a parser-admitted manifest;
 * raw declaration arrays and corpus structs remain implementation details. */
int csb_v1_boot_render_manifest_live_material_pc34(
    CSB_V1_BootProfile *profile,
    const CSB_V1_FirstLiveDungeonFrameReceipt_PC34 *first_receipt,
    const CSB_V1_ViewportOperatorDeclarationManifestPc34 *manifest,
    const CSB_V1_ViewportLiveDungeonStatePc34 *explicit_identity,
    int square_x, int square_y,
    CSB_V1_ViewportLiveFrameProgressionPc34 *progression,
    const CSB_V1_ViewportLiveDungeonSelectionPc34 *previous_selection,
    uint8_t *framebuffer, int framebuffer_width, int framebuffer_height,
    CSB_V1_ViewportLiveDungeonSelectionPc34 *out_selection);
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
void csb_v1_boot_startup_door_runtime_receipt_init_pc34(
    CSB_V1_BootStartupDoorRuntimeReceipt_PC34 *receipt);
int csb_v1_boot_startup_door_runtime_handoff_from_snapshot_pc34(
    const CSB_V1_BootRuntimeStartupSnapshot_PC34 *snapshot,
    CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    CSB_V1_BootStartupDoorRuntimeReceipt_PC34 *out_receipt);
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
int csb_v1_boot_startup_m11_presentation_receipt_from_snapshot_pc34(
    const CSB_V1_BootRuntimeStartupSnapshot_PC34 *snapshot,
    CSB_V1_BootStartupM11PresentationReceipt_PC34 *out_receipt);
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
int csb_v1_boot_runtime_save_import_receipt_pc34(
    const CSB_V1_BootProfile *profile,
    const char *dm1_import_path,
    const char *resume_save_path,
    const char *csbwin_save_path,
    CSB_V1_BootRuntimeSaveImportReceipt_PC34 *out_receipt);
int csb_v1_boot_runtime_import_csbwin_save_from_path_pc34(
    CSB_V1_BootProfile *profile,
    const char *csbwin_save_path,
    CSB_V1_BootRuntimeSaveImportReceipt_PC34 *out_receipt);
int csb_v1_boot_runtime_dsa_save_handoff_receipt_pc34(
    const CSB_V1_BootRuntimeSaveImportReceipt_PC34 *save_import_receipt,
    CSB_V1_BootRuntimeDSASaveHandoffReceipt_PC34 *out_receipt);
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

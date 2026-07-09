#ifndef DM2_V1_BOOT_H
#define DM2_V1_BOOT_H

#include <stdint.h>
#include <stddef.h>

typedef struct DM2_V1_StartupHostFacts DM2_V1_StartupHostFacts;
typedef struct DM2_V1_StartupLaunchReceipt DM2_V1_StartupLaunchReceipt;
typedef struct DM2_V1_StartupDrawCommand DM2_V1_StartupDrawCommand;
typedef struct DM2_V1_StartupViewReceipt DM2_V1_StartupViewReceipt;
struct DM2_V1_StartupHostReceipt;
struct DM2_V1_SessionState;
struct DM2_V1_StartupExecution;
struct DM2_V1_StartupHostActionReceipt;
struct DM2_V1_StartupIdleReceipt;

/* ══════════════════════════════════════════════════════════════════════
 * DM2 V1 Boot Profile — Phase 1: Runtime Profile Split
 *
 * Separates DM2/Skullkeep boot/runtime from DM1/CSB:
 *   - Asset discovery: DM2GRAPHICS.DAT, DM2DUNGEON.DAT, GRAPHICS.DAT in dm2/
 *   - Save namespace:  saves/dm2/slotN.fssv
 *   - Platform/version diagnostics
 *   - Deterministic config defaults
 *
 * Source references:
 *   SKULL.ASM T560  — DM2 dungeon loading (DUNGEON_Load)
 *   SKULL.ASM T000  — DM2 startup / title screen
 *   SKULL.ASM T800  — DM2 outdoor / shop / NPC entry points
 *   SKULL.ASM T520  — DM2 party placement and start position
 *   SKULL.ASM T048  — DM2 platform detection / version
 * ══════════════════════════════════════════════════════════════════════ */

/* DM2 supported platform IDs.
 * Used in diagnostics, save headers, and platform-specific logic. */
typedef enum {
    DM2_PLATFORM_PC_EN,      /* PC English — primary reference */
    DM2_PLATFORM_PC_FR,      /* PC French */
    DM2_PLATFORM_PC_JEWEL,   /* PC German/English JewelCase */
    DM2_PLATFORM_COUNT
} DM2_Platform;

/* DM2 deterministic config.
 * Fixed gameplay constants that ensure reproducible runs.
 * These mirror the original DM2 fixed-point arithmetic timing. */
typedef struct {
    /* Tick rate: DM2 runs at the same 18.2 Hz VBlank as DM1/CSB.
     * Each tick = ~55ms. No interpolation in V1. */
    uint32_t tick_rate_hz;          /* default: 18 */
    uint32_t tick_rate_hz_frac;     /* default: 2 (18.2 Hz = 18 + 2/10) */
    uint32_t tick_ms;              /* ~55ms per tick */

    /* Movement: DM2 uses slightly different speed values.
     * outdoor movement speed is higher than dungeon movement.
     * These are fixed-point Q8 values. */
    uint32_t dungeon_move_speed;   /* Q8: default 0x0080 (=0.5 squares/tick) */
    uint32_t outdoor_move_speed;   /* Q8: default 0x0100 (=1.0 squares/tick) */

    /* Party: DM2 supports up to 4 champions + minion slots.
     * Companion AI runs on same tick as party movement. */
    uint32_t max_champions;        /* 4 */
    uint32_t max_party_members;   /* 5 incl. leader */

    /* Time-of-day cycle: DM2 outdoor areas have day/night.
     * Full cycle in 1440 minutes (24 hours). */
    uint32_t day_cycle_minutes;    /* 1440 */
    uint32_t day_cycle_ticks;      /* derived from tick_rate */

    /* Dungeon: DM2 has 28 levels in PC English version.
     * Level 0 = Entrance / Hall of Champions.
     * Levels 1-27 = various indoor/outdoor areas. */
    uint32_t max_levels;          /* 28 for PC EN */

    /* Deterministic RNG seed: DM2 dungeon seed from DUNGEON.DAT header.
     * Used to initialize the event RNG for reproducible runs. */
    uint32_t dungeon_seed;

    /* Reserved for future deterministic options */
    uint32_t reserved[4];
} DM2_V1_DeterministicConfig;

/* DM2 boot profile — collected at startup before game loop begins.
 * All fields are set once and read-only during gameplay. */
typedef struct {
    /* ── Identity ────────────────────────────────────────── */
    char             game_id[8];        /* "dm2" */
    DM2_Platform     platform;          /* detected platform */
    char             platform_label[32]; /* e.g. "PC English" */
    char             version_id[16];    /* e.g. "pc-en", "pc-fr" */

    /* ── Asset paths ─────────────────────────────────────── */
    char    asset_root[512];   /* parent dir of resolved dungeon/graphics data */
    char    graphics_path[512]; /* resolved by known MD5 hash, filename fallback */
    char    dungeon_path[512]; /* resolved by known MD5 hash, filename fallback */
    int     use_dm2_filenames;  /* 1 if legacy DM2* filenames were used */
    int     assets_verified;    /* 1 if MD5 hash matched a known version */

    /* ── Save namespace ───────────────────────────────────── */
    char    save_root[512];    /* saves/dm2/ */

    /* ── Detected file sizes (diagnostic) ─────────────────── */
    size_t  graphics_size;
    size_t  dungeon_size;
    char    graphics_md5[33];
    char    dungeon_md5[33];

    /* ── Deterministic config ──────────────────────────────── */
    DM2_V1_DeterministicConfig deterministic;

    /* ── Runtime references (set after boot) ──────────────── */
    void   *dm2_state;         /* DM2_V1_GameState* — set by dm2_v1_boot_enter_game() */
    void   *dungeon_data;      /* DM2_V1_DungeonData* — parsed dungeon */
    void   *graphics_dat;      /* graphics data handle */
} DM2_V1_BootProfile;

typedef enum {
    DM2_V1_BOOT_STARTUP_PREPARE_OK = 0,
    DM2_V1_BOOT_STARTUP_PREPARE_BAD_INPUT,
    DM2_V1_BOOT_STARTUP_PREPARE_OOM,
    DM2_V1_BOOT_STARTUP_PREPARE_SCAN_FAILED,
    DM2_V1_BOOT_STARTUP_PREPARE_UNVERIFIED_ASSETS,
    DM2_V1_BOOT_STARTUP_PREPARE_ENTER_GAME_FAILED,
    DM2_V1_BOOT_STARTUP_PREPARE_RUNTIME_BIND_FAILED
} DM2_V1_BootStartupPrepareResult;

typedef struct {
    DM2_V1_BootProfile *profile;
    DM2_V1_BootStartupPrepareResult prepare_result;
    const char *failure_status_scope;
    const char *failure_status;
    int runtime_bound;
} DM2_V1_BootStartupLaunch;

typedef struct {
    const DM2_V1_BootProfile *profile;
    int startup_menu_active;
    const char *startup_save_root;
    int resume_available;
    unsigned int slot_mask;
    int selected_row;
} DM2_V1_BootRuntimeStartupSnapshot;

typedef struct {
    DM2_V1_BootProfile *profile;
    void *dm2_state;
    char boot_asset_md5[33];
    char dungeon_path[512];
    char title[64];
    char source_id[16];
    int initialize_v2_runtime;
    int initialize_hud_runtime;
    int initialize_touch_runtime;
} DM2_V1_BootStartupRuntimeReceipt;

typedef struct {
    int runtime_ready;
    int current_level;
    int party_x;
    int party_y;
    int party_dir;
    int tick_count;
    uint32_t leader_hand_object;
    int operation_result;
} DM2_V1_BootRuntimeReceipt;

typedef enum {
    DM2_V1_BOOT_ACTION_NO_TARGET = 0,
    DM2_V1_BOOT_ACTION_SHOP,
    DM2_V1_BOOT_ACTION_DOOR,
    DM2_V1_BOOT_ACTION_NPC,
    DM2_V1_BOOT_ACTION_ACTUATOR,
    DM2_V1_BOOT_ACTION_NO_ACTION
} DM2_V1_BootRuntimeActionKind;

typedef struct {
    DM2_V1_BootRuntimeReceipt runtime;
    DM2_V1_BootRuntimeActionKind action_kind;
    int target_level;
    int target_x;
    int target_y;
    int target_square;
    const char *status_scope;
    const char *status;
    const char *inspect_title;
    const char *inspect_text;
    int reset_shop_selection;
} DM2_V1_BootRuntimeActionReceipt;

typedef struct {
    DM2_V1_BootRuntimeReceipt runtime;
    int champion_index;
    int champion_slot;
    uint32_t slot_object_before;
    uint32_t leader_hand_before;
    uint32_t slot_object_after;
    uint32_t leader_hand_after;
    const char *status_scope;
    const char *status;
} DM2_V1_BootRuntimeInventoryReceipt;

typedef int (*DM2_V1_BootRuntimeRenderCallback)(
    int party_dir,
    int party_x,
    int party_y,
    uint8_t *framebuffer,
    int fb_stride,
    int view_w,
    int view_h,
    void *userdata);

typedef struct {
    DM2_V1_BootRuntimeReceipt runtime;
    int render_result;
    int v2_attempted;
    int v2_succeeded;
    int v1_attempted;
    int v1_succeeded;
    int startup_title_ready;
    int startup_profile_verified;
    int startup_hud_runtime_ready;
    int startup_render_ready;
    int runtime_hud_capture_ready;
    int runtime_hud_real_asset_ready;
    int runtime_hud_asset_portrait_count;
    int runtime_hud_fallback_portrait_count;
    int runtime_hud_no_fallback_portraits;
    uint32_t runtime_hud_frame_hash;
    uint32_t runtime_hud_frame_pixel_count;
    int runtime_render_real_asset_ready;
    int runtime_render_asset_floor_ceiling_count;
    int runtime_render_fallback_floor_ceiling_count;
    int runtime_render_asset_wall_count;
    int runtime_render_fallback_wall_count;
    int runtime_render_no_core_fallbacks;
} DM2_V1_BootRuntimeRenderReceipt;

typedef struct {
    int valid;
    int profile_ready;
    int graphics_dat_ready;
    int runtime_ready;
    int render_sample_count;
    int render_success_count;
    int sampled_direction_mask;
    int runtime_direction_mask;
    int runtime_turn_count;
    int unique_frame_hash_count;
    int total_asset_portrait_count;
    int total_fallback_portrait_count;
    int min_asset_portrait_count;
    int max_asset_portrait_count;
    int no_fallback_portraits;
    int total_asset_floor_ceiling_count;
    int total_fallback_floor_ceiling_count;
    int total_asset_wall_count;
    int total_fallback_wall_count;
    int min_asset_floor_ceiling_count;
    int min_asset_wall_count;
    int no_core_render_fallbacks;
    int first_runtime_hud_ready;
    int real_gdat_portrait_ready;
    int real_gdat_core_render_ready;
    int real_gdat_runtime_hud_breadth_ready;
    uint32_t combined_frame_hash;
    uint32_t combined_pixel_count;
    DM2_V1_BootRuntimeRenderReceipt first_frame;
} DM2_V1_BootRuntimeHudCaptureReceipt;

typedef struct {
    int valid;
    int startup_menu_active;
    int title_animation_tick;
    int title_frame;
    int title_frame_max;
    int title_frame_duration_ticks;
    int title_ready;
    int title_gdat_category;
    int title_gdat_index;
    int title_gdat_field;
    int title_backdrop_ready;
    int title_gdat_asset_ready;
    int title_gdat_asset_w;
    int title_gdat_asset_h;
    int title_gdat_asset_stride;
    int title_cycle_ticks;
    int title_cycle_position_tick;
    int title_frame_start_tick;
    int title_next_frame_tick;
    int title_frame_elapsed_ticks;
    int title_frame_remaining_ticks;
    int title_cycle_remaining_ticks;
    int exact_title_timing_ready;
    int menu_row_count;
    int menu_text_count;
    int selectable_text_count;
    int selected_highlight_count;
    int menu_panel_ready;
    int startup_menu_assets_ready;
    int hud_overlay_suppressed;
    int hud_runtime_ready;
    int runtime_menu_ready;
    int runtime_action_ready;
    int first_hud_frame_ready;
    int full_start_graphics_ready;
    int full_start_real_asset_ready;
} DM2_V1_BootStartupFullStartReceipt;

typedef struct {
    int valid;
    int host_view_valid;
    int full_start_valid;
    uint32_t packaged_capture_hash;
    int title_capture_ready;
    int menu_capture_ready;
    int hud_handoff_capture_ready;
    int runtime_handoff_capture_ready;
    int m11_consumer_ready;
    int draw_startup_menu;
    int command_count;
    int selected_row;
    int title_animation_tick;
    int title_frame;
    int title_frame_max;
    int title_frame_duration_ticks;
    int title_ready;
    int title_cycle_ticks;
    int title_cycle_position_tick;
    int title_frame_start_tick;
    int title_next_frame_tick;
    int title_frame_elapsed_ticks;
    int title_frame_remaining_ticks;
    int title_cycle_remaining_ticks;
    int exact_title_timing_ready;
    int title_gdat_category;
    int title_gdat_index;
    int title_gdat_field;
    int title_gdat_asset_ready;
    int title_gdat_asset_w;
    int title_gdat_asset_h;
    int title_gdat_asset_stride;
    int menu_row_count;
    int menu_text_count;
    int selectable_text_count;
    int selected_highlight_count;
    int menu_panel_ready;
    int startup_menu_assets_ready;
    int hud_overlay_suppressed;
    int hud_runtime_ready;
    int first_hud_frame_ready;
    const char *status_scope;
    const char *status;
} DM2_V1_BootStartupPackagedCaptureProof;

typedef struct {
    int valid;
    int full_start_valid;
    int capture_proof_valid;
    uint32_t packaged_full_start_hash;
    int full_start_graphics_ready;
    int full_start_real_asset_ready;
    int exact_title_timing_ready;
    int title_capture_ready;
    int menu_capture_ready;
    int hud_handoff_capture_ready;
    int runtime_handoff_capture_ready;
    int m11_consumer_ready;
    int title_ready;
    int runtime_menu_ready;
    int runtime_action_ready;
    int first_hud_frame_ready;
    int startup_menu_active;
    int draw_startup_menu;
    int command_count;
    int selected_row;
    int title_animation_tick;
    int title_frame;
    int title_frame_max;
    int title_frame_duration_ticks;
    int title_cycle_ticks;
    int title_cycle_position_tick;
    int title_frame_start_tick;
    int title_next_frame_tick;
    int title_frame_elapsed_ticks;
    int title_frame_remaining_ticks;
    int title_cycle_remaining_ticks;
    int title_gdat_category;
    int title_gdat_index;
    int title_gdat_field;
    int title_gdat_asset_ready;
    int title_gdat_asset_w;
    int title_gdat_asset_h;
    int title_gdat_asset_stride;
    int menu_row_count;
    int menu_text_count;
    int selectable_text_count;
    int selected_highlight_count;
    int menu_panel_ready;
    int startup_menu_assets_ready;
    int hud_overlay_suppressed;
    int hud_runtime_ready;
    const char *status_scope;
    const char *status;
    DM2_V1_BootStartupFullStartReceipt full_start;
    DM2_V1_BootStartupPackagedCaptureProof capture_proof;
} DM2_V1_BootStartupPackagedFullStartReceipt;

typedef struct {
    int valid;
    int packaged_full_start_valid;
    uint32_t packaged_full_start_hash;
    int startup_active;
    int startup_animation_active;
    int startup_title_frame;
    int startup_title_frame_max;
    int startup_title_ready;
    int startup_hud_runtime_ready;
    int startup_draw_ready;
    int startup_draw_command_count;
    int startup_draw_menu_capture_ready;
    int startup_draw_hud_handoff_ready;
    int title_capture_ready;
    int menu_capture_ready;
    int hud_handoff_capture_ready;
    int runtime_handoff_capture_ready;
    int exact_title_timing_ready;
    int packaged_title_timing_consumed;
    int packaged_first_hud_receipt_consumed;
    int m11_startup_receipt_ready;
    int full_start_real_asset_ready;
    int runtime_menu_ready;
    int runtime_action_ready;
    int first_hud_frame_ready;
    int title_gdat_asset_ready;
    int title_gdat_asset_w;
    int title_gdat_asset_h;
    int title_frame_duration_ticks;
    int title_cycle_ticks;
    int title_cycle_position_tick;
    int title_frame_start_tick;
    int title_next_frame_tick;
    int title_frame_elapsed_ticks;
    int title_frame_remaining_ticks;
    const char *phase;
    const char *animation;
    const char *status_scope;
    const char *status;
} DM2_V1_BootStartupPackagedConsumerReceipt;

typedef struct {
    int valid;
    int consume_startup_package;
    int render_startup_title;
    int render_startup_menu;
    int suppress_game_hud;
    int enable_runtime_input;
    int present_first_hud_frame;
    int schedule_next_title_tick;
    int next_title_tick_delta;
    int title_animation_tick;
    int title_frame;
    int title_frame_max;
    int title_frame_duration_ticks;
    int title_frame_elapsed_ticks;
    int title_frame_remaining_ticks;
    int title_next_frame_tick;
    int startup_draw_command_count;
    int startup_draw_ready;
    int startup_hud_runtime_ready;
    int runtime_menu_ready;
    int runtime_action_ready;
    int first_hud_frame_ready;
    uint32_t packaged_full_start_hash;
    const char *phase;
    const char *animation;
    const char *status_scope;
    const char *status;
} DM2_V1_BootStartupHostFrameReceipt;

typedef struct {
    int valid;
    int packaged_full_start_valid;
    int host_frame_valid;
    int consume_host_frame_receipt;
    int execute_startup_draw_commands;
    int draw_command_count;
    int executed_command_count;
    int executed_gdat_image_count;
    int executed_rect_count;
    int executed_text_count;
    int title_gdat_command_count;
    int title_gdat_asset_required;
    int title_gdat_asset_consumed;
    int title_gdat_asset_ready;
    int title_gdat_asset_w;
    int title_gdat_asset_h;
    int title_gdat_asset_stride;
    int fallback_title_blit_used;
    int final_m11_draw_caller_ready;
    int final_m11_draw_caller_consumes_ownership;
    int packaged_draw_commands_consumed;
    int title_timing_receipt_consumed;
    int real_gdat_title_asset_receipt_breadth;
    int menu_hud_startup_receipt_breadth;
    int suppress_game_hud;
    int present_first_hud_frame;
    int schedule_next_title_tick;
    int next_title_tick_delta;
    int title_animation_tick;
    int title_frame;
    int title_frame_max;
    int title_frame_duration_ticks;
    int title_frame_elapsed_ticks;
    int title_frame_remaining_ticks;
    int title_next_frame_tick;
    int runtime_menu_ready;
    int runtime_action_ready;
    int first_hud_frame_ready;
    uint32_t packaged_full_start_hash;
    const char *phase;
    const char *animation;
    const char *status_scope;
    const char *status;
} DM2_V1_BootStartupRenderOwnershipReceipt;

typedef struct {
    int valid;
    int profile_ready;
    int graphics_dat_ready;
    int packaged_full_start_valid;
    int packaged_consumer_valid;
    int host_frame_valid;
    int render_ownership_valid;
    int real_visual_capture_consumes_package;
    int real_visual_capture_consumes_host_frame;
    int real_visual_status_consumer_ready;
    int packaged_status_consumed;
    int packaged_startup_phase_consumed;
    int packaged_hud_suppression_consumed;
    int sampled_title_timing_capture_count;
    int sampled_title_frame_mask;
    int sampled_title_pixel_capture_count;
    int sampled_menu_selection_capture_count;
    int sampled_menu_selection_mask;
    int sampled_menu_composite_capture_count;
    int sampled_menu_unique_composite_hash_count;
    uint32_t sampled_menu_composite_hash;
    int sampled_runtime_hud_handoff_capture_ready;
    int real_gdat_capture_breadth_ready;
    int real_gdat_title_asset_required;
    int real_gdat_title_asset_consumed;
    int real_gdat_menu_asset_required;
    int real_gdat_menu_asset_consumed;
    int title_capture_ready;
    int menu_gdat_capture_ready;
    int full_title_frame_capture_ready;
    int title_gdat_category;
    int title_gdat_index;
    int title_gdat_field;
    int menu_gdat_category;
    int menu_gdat_index;
    int menu_gdat_field;
    int skproject_title_query_ready;
    int skproject_menu_query_ready;
    int skproject_title_category;
    int skproject_title_index;
    int skproject_credit_screen_field;
    int skproject_menu_screen_field;
    int title_gdat_asset_w;
    int title_gdat_asset_h;
    int title_gdat_asset_stride;
    uint32_t title_pixel_hash;
    uint32_t title_pixel_count;
    int menu_gdat_asset_w;
    int menu_gdat_asset_h;
    int menu_gdat_asset_stride;
    uint32_t menu_pixel_hash;
    uint32_t menu_pixel_count;
    int full_visual_composite_capture_ready;
    int composite_gdat_blit_count;
    int composite_rect_count;
    int composite_text_zone_count;
    uint32_t composite_pixel_hash;
    uint32_t composite_pixel_count;
    int m11_draw_consumption_ready;
    int m11_draw_executed_command_count;
    int m11_draw_gdat_blit_count;
    int m11_draw_rect_count;
    int m11_draw_text_count;
    int m11_draw_matches_real_visual_receipt;
    uint32_t m11_draw_frame_hash;
    uint32_t m11_draw_frame_pixel_count;
    int hud_suppressed_capture_ready;
    int menu_capture_ready;
    int menu_title_composite_capture_ready;
    int menu_command_count;
    int menu_gdat_command_count;
    int menu_rect_command_count;
    int menu_text_command_count;
    int menu_row_count;
    int selected_highlight_count;
    int resume_menu_ready;
    int save_slot_menu_ready;
    int new_game_menu_ready;
    int exact_selected_highlight_ready;
    int startup_title_menu_hud_breadth_ready;
    int hud_handoff_capture_ready;
    int title_menu_hud_visual_proof_ready;
    int suppress_game_hud;
    int present_first_hud_frame;
    int exact_title_timing_ready;
    int title_animation_tick;
    int title_frame;
    int title_frame_remaining_ticks;
    int no_fallback_title_blit;
    uint32_t packaged_visual_capture_hash;
    uint32_t packaged_full_start_hash;
    uint32_t packaged_consumer_hash;
    const char *phase;
    const char *animation;
    const char *status_scope;
    const char *status;
} DM2_V1_BootStartupRealVisualCaptureReceipt;

typedef struct {
    int valid;
    int draw_startup_menu;
    int command_count;
    int selected_row;
    int render_commands_ready;
    int menu_state_ready;
    int row_selection_ready;
    int resume_menu_ready;
    int save_slot_menu_ready;
    int new_game_menu_ready;
    int title_timing_ready;
    int title_asset_ready;
    int title_menu_ready;
    int title_animation_tick;
    int title_frame;
    int title_frame_max;
    int title_frame_duration_ticks;
    int title_ready;
    int title_gdat_asset_ready;
    int title_gdat_asset_w;
    int title_gdat_asset_h;
    int title_gdat_asset_stride;
    int full_start_real_asset_ready;
    int title_cycle_ticks;
    int title_cycle_position_tick;
    int title_frame_start_tick;
    int title_next_frame_tick;
    int title_frame_elapsed_ticks;
    int title_frame_remaining_ticks;
    int title_cycle_remaining_ticks;
    int exact_title_timing_ready;
    int menu_row_count;
    int menu_text_count;
    int selectable_text_count;
    int selected_highlight_count;
    int menu_panel_ready;
    int startup_menu_assets_ready;
    int hud_overlay_suppressed;
    int hud_runtime_ready;
    int runtime_menu_ready;
    int runtime_action_ready;
    int first_hud_frame_ready;
    int startup_hud_handoff_ready;
    int runtime_handoff_ready;
    int m11_host_view_ready;
    const char *status_scope;
    const char *status;
    const char *log_line;
    DM2_V1_BootStartupFullStartReceipt full_start;
    int capture_proof_valid;
    DM2_V1_BootStartupPackagedCaptureProof capture_proof;
} DM2_V1_BootStartupHostViewReceipt;

enum {
    DM2_V1_BOOT_STARTUP_VIEW_MODEL_COMMAND_CAP = 32,
    DM2_V1_BOOT_STARTUP_VIEW_MODEL_TEXT_CAP = 32,
    DM2_V1_BOOT_STARTUP_VIEW_MODEL_ANIMATION_CAP = 32
};

typedef struct DM2_V1_BootStartupViewModel DM2_V1_BootStartupViewModel;

/* ── Boot API ──────────────────────────────────────────────────────── */

/* Initialize a boot profile with defaults.
 * Does not touch the filesystem — only sets struct fields. */
void dm2_v1_boot_profile_init(DM2_V1_BootProfile *profile);

/* Scan and verify DM2 assets in data_dir by known hashes first.
 * Sets asset_root, graphics_path, dungeon_path, assets_verified.
 * Returns 0 on success, -1 if no valid DM2 assets found. */
int dm2_v1_boot_scan_assets(DM2_V1_BootProfile *profile,
                            const char *data_dir);

/* Probe a data_dir for DM2 assets without full verification.
 * Used by the launcher menu to determine DM2 availability.
 * Returns: 1 if assets found, 0 if not. */
int dm2_v1_boot_probe_available(const char *data_dir);

/* Set the save root directory.
 * If save_dir is NULL, uses default: <data_dir>/../saves/dm2/ */
void dm2_v1_boot_set_save_root(DM2_V1_BootProfile *profile,
                                const char *save_dir);

/* Build deterministic config from detected dungeon header.
 * Reads dungeon_seed from the DUNGEON.DAT header word at offset 8.
 * Source: SKULL.ASM T560 — DUNGEON_Load header parsing */
void dm2_v1_boot_build_deterministic_config(DM2_V1_BootProfile *profile,
                                            const uint8_t *dungeon_header,
                                            int dungeon_size);

/* Enter the game: initialize the game state from the boot profile.
 * Sets profile->dm2_state and profile->dungeon_data.
 * Returns 0 on success. */
int dm2_v1_boot_enter_game(DM2_V1_BootProfile *profile);

/* Allocate and prepare a DM2 boot profile through the verified game-entry
 * boundary. The caller still owns M11 startup menu/session receipts, but DM2
 * owns profile allocation, asset scanning, save-root setup, and enter_game. */
int dm2_v1_boot_startup_launch_alloc(
    const char *data_dir,
    DM2_V1_BootStartupLaunch *out_launch);

int dm2_v1_boot_startup_launch_detach_runtime(
    DM2_V1_BootStartupLaunch *launch,
    DM2_V1_BootStartupRuntimeReceipt *out_receipt);

void dm2_v1_boot_startup_launch_cleanup(
    DM2_V1_BootStartupLaunch *launch);

const char *dm2_v1_boot_startup_prepare_result_name(
    DM2_V1_BootStartupPrepareResult result);

int dm2_v1_boot_startup_prepare_failure_host_receipt(
    const DM2_V1_BootStartupLaunch *launch,
    struct DM2_V1_StartupHostReceipt *out_receipt);

/* Build M11-facing startup facts from boot-owned profile state plus the
 * host's current startup menu snapshot. Keeps save-root fallback/scan roots
 * owned by the DM2 boot layer instead of M11 unpacking the profile. */
int dm2_v1_boot_startup_host_facts_from_runtime_state(
    const DM2_V1_BootProfile *profile,
    int startup_menu_active,
    const char *startup_save_root,
    int resume_available,
    unsigned int slot_mask,
    int selected_row,
    DM2_V1_StartupHostFacts *out_facts);

int dm2_v1_boot_startup_launch_from_runtime_state(
    const DM2_V1_BootProfile *profile,
    int startup_menu_active,
    const char *startup_save_root,
    int resume_available,
    unsigned int slot_mask,
    int selected_row,
    DM2_V1_StartupLaunchReceipt *out_receipt);
int dm2_v1_boot_startup_launch_from_snapshot(
    const DM2_V1_BootRuntimeStartupSnapshot *snapshot,
    DM2_V1_StartupLaunchReceipt *out_receipt);
int dm2_v1_boot_startup_launch_from_launch_snapshot(
    const DM2_V1_BootStartupLaunch *launch,
    const DM2_V1_BootRuntimeStartupSnapshot *snapshot,
    DM2_V1_StartupLaunchReceipt *out_receipt);
int dm2_v1_boot_startup_launch_from_launch(
    const DM2_V1_BootStartupLaunch *launch,
    DM2_V1_StartupLaunchReceipt *out_receipt);

int dm2_v1_boot_startup_advance_idle_from_runtime_state(
    const DM2_V1_BootProfile *profile,
    int startup_menu_active,
    const char *startup_save_root,
    int resume_available,
    unsigned int slot_mask,
    int selected_row,
    int mouth_redraw,
    struct DM2_V1_StartupIdleReceipt *out_receipt);
int dm2_v1_boot_startup_advance_idle_from_snapshot(
    const DM2_V1_BootRuntimeStartupSnapshot *snapshot,
    int mouth_redraw,
    struct DM2_V1_StartupIdleReceipt *out_receipt);

int dm2_v1_boot_startup_execute_firestaff_input_from_runtime_state(
    const DM2_V1_BootProfile *profile,
    int startup_menu_active,
    const char *startup_save_root,
    int resume_available,
    unsigned int slot_mask,
    int selected_row,
    int menu_input,
    int (*apply_session)(void *userdata,
                         const struct DM2_V1_SessionState *session),
    void *apply_userdata,
    struct DM2_V1_StartupExecution *out_execution,
    struct DM2_V1_StartupHostActionReceipt *out_receipt);
int dm2_v1_boot_startup_execute_firestaff_input_from_snapshot(
    const DM2_V1_BootRuntimeStartupSnapshot *snapshot,
    int menu_input,
    int (*apply_session)(void *userdata,
                         const struct DM2_V1_SessionState *session),
    void *apply_userdata,
    struct DM2_V1_StartupExecution *out_execution,
    struct DM2_V1_StartupHostActionReceipt *out_receipt);

int dm2_v1_boot_startup_execute_pointer_from_runtime_state(
    const DM2_V1_BootProfile *profile,
    int startup_menu_active,
    const char *startup_save_root,
    int resume_available,
    unsigned int slot_mask,
    int selected_row,
    int x,
    int y,
    int (*apply_session)(void *userdata,
                         const struct DM2_V1_SessionState *session),
    void *apply_userdata,
    struct DM2_V1_StartupExecution *out_execution,
    struct DM2_V1_StartupHostActionReceipt *out_receipt);
int dm2_v1_boot_startup_execute_pointer_from_snapshot(
    const DM2_V1_BootRuntimeStartupSnapshot *snapshot,
    int x,
    int y,
    int (*apply_session)(void *userdata,
                         const struct DM2_V1_SessionState *session),
    void *apply_userdata,
    struct DM2_V1_StartupExecution *out_execution,
    struct DM2_V1_StartupHostActionReceipt *out_receipt);

int dm2_v1_boot_startup_presentation_build_from_runtime_state(
    const DM2_V1_BootProfile *profile,
    int startup_menu_active,
    const char *startup_save_root,
    int resume_available,
    unsigned int slot_mask,
    int selected_row,
    DM2_V1_StartupDrawCommand *out_commands,
    int max_commands);
int dm2_v1_boot_startup_presentation_build_from_snapshot(
    const DM2_V1_BootRuntimeStartupSnapshot *snapshot,
    DM2_V1_StartupDrawCommand *out_commands,
    int max_commands);
int dm2_v1_boot_startup_view_model_from_snapshot(
    const DM2_V1_BootRuntimeStartupSnapshot *snapshot,
    void *out_commands,
    int max_commands,
    int *out_command_count,
    DM2_V1_StartupViewReceipt *out_view_receipt,
    char *out_phase,
    int out_phase_size,
    int *out_startup_active,
    char *out_animation,
    int out_animation_size,
    int *out_animation_active,
    int *out_title_frame,
    int *out_title_frame_max,
    int *out_title_ready);
void dm2_v1_boot_startup_view_model_clear(
    DM2_V1_BootStartupViewModel *out_view_model);
int dm2_v1_boot_startup_view_model_receipt_from_snapshot(
    const DM2_V1_BootRuntimeStartupSnapshot *snapshot,
    DM2_V1_BootStartupViewModel *out_view_model);
int dm2_v1_boot_startup_host_view_receipt_from_snapshot(
    const DM2_V1_BootRuntimeStartupSnapshot *snapshot,
    DM2_V1_BootStartupHostViewReceipt *out_receipt);
int dm2_v1_boot_startup_host_view_receipt_from_runtime_state(
    const DM2_V1_BootProfile *profile,
    int startup_menu_active,
    const char *startup_save_root,
    int resume_available,
    unsigned int slot_mask,
    int selected_row,
    int title_animation_tick,
    DM2_V1_BootStartupHostViewReceipt *out_receipt);
void dm2_v1_boot_startup_packaged_capture_proof_init(
    DM2_V1_BootStartupPackagedCaptureProof *proof);
int dm2_v1_boot_startup_packaged_capture_proof_from_host_view(
    const DM2_V1_BootStartupHostViewReceipt *host_view,
    DM2_V1_BootStartupPackagedCaptureProof *out_proof);
void dm2_v1_boot_startup_packaged_full_start_receipt_init(
    DM2_V1_BootStartupPackagedFullStartReceipt *receipt);
int dm2_v1_boot_startup_packaged_full_start_receipt_from_host_view(
    const DM2_V1_BootStartupHostViewReceipt *host_view,
    DM2_V1_BootStartupPackagedFullStartReceipt *out_receipt);
int dm2_v1_boot_startup_packaged_full_start_receipt_from_snapshot(
    const DM2_V1_BootRuntimeStartupSnapshot *snapshot,
    DM2_V1_BootStartupPackagedFullStartReceipt *out_receipt);
int dm2_v1_boot_startup_packaged_full_start_receipt_from_runtime_state(
    const DM2_V1_BootProfile *profile,
    int startup_menu_active,
    const char *startup_save_root,
    int resume_available,
    unsigned int slot_mask,
    int selected_row,
    int title_animation_tick,
    DM2_V1_BootStartupPackagedFullStartReceipt *out_receipt);
void dm2_v1_boot_startup_packaged_consumer_receipt_init(
    DM2_V1_BootStartupPackagedConsumerReceipt *receipt);
int dm2_v1_boot_startup_packaged_consumer_receipt_from_full_start(
    const DM2_V1_BootStartupPackagedFullStartReceipt *package,
    DM2_V1_BootStartupPackagedConsumerReceipt *out_receipt);
int dm2_v1_boot_startup_packaged_consumer_receipt_from_snapshot(
    const DM2_V1_BootRuntimeStartupSnapshot *snapshot,
    DM2_V1_BootStartupPackagedConsumerReceipt *out_receipt);
int dm2_v1_boot_startup_packaged_consumer_receipt_from_runtime_state(
    const DM2_V1_BootProfile *profile,
    int startup_menu_active,
    const char *startup_save_root,
    int resume_available,
    unsigned int slot_mask,
    int selected_row,
    int title_animation_tick,
    DM2_V1_BootStartupPackagedConsumerReceipt *out_receipt);
void dm2_v1_boot_startup_host_frame_receipt_init(
    DM2_V1_BootStartupHostFrameReceipt *receipt);
int dm2_v1_boot_startup_host_frame_receipt_from_consumer(
    const DM2_V1_BootStartupPackagedConsumerReceipt *consumer,
    DM2_V1_BootStartupHostFrameReceipt *out_receipt);
int dm2_v1_boot_startup_host_frame_receipt_from_snapshot(
    const DM2_V1_BootRuntimeStartupSnapshot *snapshot,
    DM2_V1_BootStartupHostFrameReceipt *out_receipt);
int dm2_v1_boot_startup_host_frame_receipt_from_runtime_state(
    const DM2_V1_BootProfile *profile,
    int startup_menu_active,
    const char *startup_save_root,
    int resume_available,
    unsigned int slot_mask,
    int selected_row,
    int title_animation_tick,
    DM2_V1_BootStartupHostFrameReceipt *out_receipt);
void dm2_v1_boot_startup_render_ownership_receipt_init(
    DM2_V1_BootStartupRenderOwnershipReceipt *receipt);
int dm2_v1_boot_startup_render_ownership_receipt_from_snapshot(
    const DM2_V1_BootRuntimeStartupSnapshot *snapshot,
    DM2_V1_BootStartupRenderOwnershipReceipt *out_receipt);
int dm2_v1_boot_startup_render_ownership_receipt_from_runtime_state(
    const DM2_V1_BootProfile *profile,
    int startup_menu_active,
    const char *startup_save_root,
    int resume_available,
    unsigned int slot_mask,
    int selected_row,
    int title_animation_tick,
    DM2_V1_BootStartupRenderOwnershipReceipt *out_receipt);
void dm2_v1_boot_startup_real_visual_capture_receipt_init(
    DM2_V1_BootStartupRealVisualCaptureReceipt *receipt);
int dm2_v1_boot_startup_real_visual_capture_receipt_from_runtime_state(
    DM2_V1_BootProfile *profile,
    int startup_menu_active,
    const char *startup_save_root,
    int resume_available,
    unsigned int slot_mask,
    int selected_row,
    int title_animation_tick,
    DM2_V1_BootStartupRealVisualCaptureReceipt *out_receipt);
int dm2_v1_boot_startup_presentation_receipt_from_runtime_state(
    int startup_menu_active,
    char *out_phase,
    int out_phase_size,
    int *out_startup_active,
    char *out_animation,
    int out_animation_size,
    int *out_animation_active,
    int *out_title_frame,
    int *out_title_frame_max,
    int *out_title_ready);
int dm2_v1_boot_startup_presentation_receipt_from_snapshot(
    const DM2_V1_BootRuntimeStartupSnapshot *snapshot,
    char *out_phase,
    int out_phase_size,
    int *out_startup_active,
    char *out_animation,
    int out_animation_size,
    int *out_animation_active,
    int *out_title_frame,
    int *out_title_frame_max,
    int *out_title_ready);
int dm2_v1_boot_startup_execute_draw_commands(
    const DM2_V1_StartupDrawCommand *commands,
    int command_count,
    const void *executor);
int dm2_v1_boot_startup_execute_save_path_with_host_receipt(
    const char *save_path,
    int (*apply_session)(void *userdata,
                         const struct DM2_V1_SessionState *session),
    void *apply_userdata,
    struct DM2_V1_StartupExecution *out_execution,
    void *out_direct_resume_receipt);
int dm2_v1_boot_startup_execute_launch_save_path_with_host_receipt(
    DM2_V1_BootStartupLaunch *launch,
    const char *save_path,
    int (*apply_session)(void *userdata,
                         const struct DM2_V1_SessionState *session),
    void *apply_userdata,
    struct DM2_V1_StartupExecution *out_execution,
    void *out_direct_resume_receipt);

int dm2_v1_boot_runtime_capture(DM2_V1_BootProfile *profile,
                                DM2_V1_BootRuntimeReceipt *out_receipt);
int dm2_v1_boot_runtime_tick(DM2_V1_BootProfile *profile,
                             DM2_V1_BootRuntimeReceipt *out_receipt);
int dm2_v1_boot_runtime_turn(DM2_V1_BootProfile *profile,
                             int delta,
                             DM2_V1_BootRuntimeReceipt *out_receipt);
int dm2_v1_boot_runtime_move(DM2_V1_BootProfile *profile,
                             int direction,
                             DM2_V1_BootRuntimeReceipt *out_receipt);
int dm2_v1_boot_runtime_action_front_cell(
    DM2_V1_BootProfile *profile,
    int direction,
    DM2_V1_BootRuntimeActionReceipt *out_receipt);
int dm2_v1_boot_runtime_swap_inventory_slot(
    DM2_V1_BootProfile *profile,
    int champion_index,
    int champion_slot,
    DM2_V1_BootRuntimeInventoryReceipt *out_receipt);
int dm2_v1_boot_runtime_render_frame(
    DM2_V1_BootProfile *profile,
    uint8_t *framebuffer,
    int fb_stride,
    int view_w,
    int view_h,
    DM2_V1_BootRuntimeRenderCallback v2_render,
    void *v2_userdata,
    DM2_V1_BootRuntimeRenderReceipt *out_receipt);
void dm2_v1_boot_runtime_hud_capture_receipt_init(
    DM2_V1_BootRuntimeHudCaptureReceipt *receipt);
int dm2_v1_boot_runtime_hud_capture_receipt(
    DM2_V1_BootProfile *profile,
    DM2_V1_BootRuntimeHudCaptureReceipt *out_receipt);

/* Viewport asset provider backed by profile->graphics_dat.
 * Pass the DM2_V1_BootProfile as the user pointer. */
int dm2_v1_boot_viewport_asset_fetch(void *user,
                                     int gdat_index,
                                     const uint8_t **out_pixels,
                                     int *out_w,
                                     int *out_h,
                                     int *out_stride);

/* Fetch a DM2 object icon image from the boot-owned GRAPHICS.DAT handle.
 * The returned pixel buffer is owned by the caller and must be freed with
 * dm2_v1_boot_object_icon_asset_free(). Returns 0 on success. */
int dm2_v1_boot_object_icon_asset_fetch(
    DM2_V1_BootProfile *profile,
    uint32_t object_id,
    uint8_t **out_pixels,
    int *out_w,
    int *out_h,
    int *out_stride);

void dm2_v1_boot_object_icon_asset_free(uint8_t *pixels);

/* Fetch a GDAT image directly from the boot-owned GRAPHICS.DAT handle.
 * Used by startup/title/credits presentation code where the DM2 module
 * owns the GDAT address and M11 only executes the resulting blit. */
int dm2_v1_boot_gdat_image_asset_fetch(
    DM2_V1_BootProfile *profile,
    int category,
    int index,
    int field,
    uint8_t **out_pixels,
    int *out_w,
    int *out_h,
    int *out_stride);

void dm2_v1_boot_gdat_image_asset_free(uint8_t *pixels);

/* Free resources allocated during boot (but not the profile itself). */
void dm2_v1_boot_cleanup(DM2_V1_BootProfile *profile);

/* ── Diagnostics API ──────────────────────────────────────────────── */

/* Fill a diagnostic report buffer with human-readable text.
 * Returns bytes written (capped at buf_size). */
size_t dm2_v1_diagnostic_report(const DM2_V1_BootProfile *profile,
                                char *buf, size_t buf_size);

/* Print a one-line platform/version summary to stdout. */
void dm2_v1_boot_print_summary(const DM2_V1_BootProfile *profile);

/* Source evidence citation string.
 * Used in assert comments and debug output. */
const char *dm2_v1_boot_source_evidence(void);

#endif /* DM2_V1_BOOT_H */

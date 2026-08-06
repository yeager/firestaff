#ifndef DM2_V1_BOOT_H
#define DM2_V1_BOOT_H

#include <stdint.h>
#include "dm2_v1_asset_loader.h"
#include "dm2_v1_dialogue_gdat.h"
#include "dm2_v1_dungeon_loader.h"
#include "dm2_v1_gdat_scene_m11_command.h"
#include "dm2_v1_weather_gdat.h"
#include "dm2_v1_fmtowns_disc.h"
#include "dm2_v1_fmtowns_cdda_music.h"
#include "dm2_v1_fmtowns_anim_stream.h"
#include <stddef.h>

#define DM2_V1_GRAPHICSSET_SCENE_COLORKEY_PRESENT_MASK (1u << 0)
#define DM2_V1_GRAPHICSSET_SCENE_FLAGS_PRESENT_MASK    (1u << 1)
#define DM2_V1_GRAPHICSSET_SCENE_ADMISSION_PRESENT_MASK \
    (DM2_V1_GRAPHICSSET_SCENE_COLORKEY_PRESENT_MASK | \
     DM2_V1_GRAPHICSSET_SCENE_FLAGS_PRESENT_MASK)

typedef struct DM2_V1_StartupHostFacts DM2_V1_StartupHostFacts;
typedef struct DM2_V1_StartupLaunchReceipt DM2_V1_StartupLaunchReceipt;
typedef struct DM2_V1_StartupDrawCommand DM2_V1_StartupDrawCommand;
typedef struct DM2_V1_StartupViewReceipt DM2_V1_StartupViewReceipt;
typedef struct DM2_V1_GdatHudM11CommandPlan DM2_V1_GdatHudM11CommandPlan;
typedef struct DM2_V1_GdatWallM11CommandPlan DM2_V1_GdatWallM11CommandPlan;
typedef struct DM2_V1_GdatDoorOverlayM11CommandPlan DM2_V1_GdatDoorOverlayM11CommandPlan;
typedef struct DM2_V1_HudPartyState DM2_V1_HudPartyState;
typedef struct DM2_V1_DoorRenderPlan DM2_V1_DoorRenderPlan;
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
    DM2_PLATFORM_PC_EN,        /* PC English — primary reference */
    DM2_PLATFORM_PC_FR,        /* PC French */
    DM2_PLATFORM_PC_JEWEL,     /* PC German/English JewelCase */
    DM2_PLATFORM_FMTOWNS_JA,  /* FM Towns Japanese (Victor HME-242, 1994) */
    DM2_PLATFORM_MAC_EN,       /* Macintosh English */
    DM2_PLATFORM_MAC_FR,       /* Macintosh French */
    DM2_PLATFORM_AMIGA_EN,     /* Amiga AGA English */
    DM2_PLATFORM_MEGACD_JA,    /* Mega CD / Sega CD Japanese */
    DM2_PLATFORM_PC9821_JA,    /* PC-9821 Japanese */
    DM2_PLATFORM_COUNT
} DM2_Platform;

/* Return nonzero only for a documented, same-platform DM2 graphics/dungeon
 * hash pair.  Hash membership alone is insufficient: PC, FM Towns and Amiga
 * dungeon payloads have different formats and must never be cross-launched. */
int dm2_v1_boot_asset_hash_pair_supported(const char *graphics_md5,
                                          const char *dungeon_md5);

/* Music system classification by platform.
 * PC:           HMP via SONGLIST.DAT (63B, per-map index)
 * Mac/Amiga:    HMP/MOD via CD.DAT/md.dat (176B, per-map mapping)
 * FM Towns/Mega CD/PC-9821: CDDA via CD.DAT (40B, coordinate triggers) */
typedef enum {
    DM2_MUSIC_SYSTEM_HMP_SONGLIST,   /* PC: SONGLIST.DAT -> GDAT HMP */
    DM2_MUSIC_SYSTEM_HMP_MAP176,     /* Mac: md.dat -> external HMP files */
    DM2_MUSIC_SYSTEM_MOD_MAP176,     /* Amiga: CD.DAT -> MOD files */
    DM2_MUSIC_SYSTEM_CDDA_COORD,     /* FM Towns/Mega CD/PC-9821: CD.DAT -> disc tracks */
} DM2_MusicSystem;

static inline DM2_MusicSystem dm2_v1_platform_music_system(DM2_Platform p) {
    switch (p) {
    case DM2_PLATFORM_MAC_EN:
    case DM2_PLATFORM_MAC_FR:       return DM2_MUSIC_SYSTEM_HMP_MAP176;
    case DM2_PLATFORM_AMIGA_EN:     return DM2_MUSIC_SYSTEM_MOD_MAP176;
    case DM2_PLATFORM_FMTOWNS_JA:
    case DM2_PLATFORM_MEGACD_JA:
    case DM2_PLATFORM_PC9821_JA:    return DM2_MUSIC_SYSTEM_CDDA_COORD;
    default:                        return DM2_MUSIC_SYSTEM_HMP_SONGLIST;
    }
}

/* Bounded Phar Lap P3 header facts for the native FM Towns SKULL.EXP. */
typedef struct {
    int      valid;
    uint16_t level;
    uint32_t header_size;
    uint32_t file_size;
    uint32_t runtime_offset;
    uint32_t runtime_size;
    uint32_t relocation_offset;
    uint32_t relocation_size;
    uint32_t load_image_offset;
    uint32_t load_image_size;
    uint32_t symbol_table_offset;
    uint32_t symbol_table_size;
    uint32_t initial_eip;
    uint32_t memory_requirements;
} DM2_V1_FmtownsP3Receipt;

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

    /* Source environment-clock owner is not yet imported. These fields stay
     * zero rather than claiming a fabricated 24-hour cycle. */
    uint32_t day_cycle_minutes;    /* 0 = unavailable */
    uint32_t day_cycle_ticks;      /* 0 = unavailable */

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
    /* Resolved only by a recognised content hash.  The original install name
     * is retained as display metadata after admission; it is never a lookup
     * fallback or a launch credential. */
    char    graphics_path[512];
    char    dungeon_path[512];
    char    songlist_path[512]; /* optional PC SONGLIST.DAT, resolved by hash */
    int     use_dm2_filenames;  /* 1 if legacy DM2* filenames were used */
    int     assets_verified;    /* 1 if MD5 hash matched a known version */

    /* ── In-memory asset buffers (from archive extraction) ── */
    uint8_t *graphics_mem;   /* malloc'd GRAPHICS.DAT from disc image */
    size_t   graphics_mem_size;
    uint8_t *dungeon_mem;    /* malloc'd DUNGEON.DAT from disc image */
    size_t   dungeon_mem_size;

    /* ── Save namespace ───────────────────────────────────── */
    char    save_root[1024];   /* saves/dm2/ */

    /* ── Detected file sizes (diagnostic) ─────────────────── */
    size_t  graphics_size;
    size_t  dungeon_size;
    char    graphics_md5[33];
    char    dungeon_md5[33];
    char    songlist_md5[33];
    size_t  songlist_size;
    uint8_t songlist_map[44]; /* original PC map 0..43 music selectors */
    int     songlist_verified;

    /* ── Music map (Mac/Amiga 176-byte CD.DAT/md.dat) ──────── */
    char    music_map_path[512];
    char    music_map_md5[33];
    size_t  music_map_size;
    uint8_t music_map_data[176];
    int     music_map_verified;

    /* ── CDDA coordinate table (FM Towns/Mega CD/PC-9821) ──── */
    char    cdda_cd_dat_path[512];
    char    cdda_cd_dat_md5[33];
    size_t  cdda_cd_dat_size;
    uint8_t cdda_cd_dat_data[40];
    int     cdda_cd_dat_verified;

    /* ── FM Towns disc image (loaded from ZIP, kept in memory) ── */
    char    fmtowns_zip_path[512];
    uint8_t *fmtowns_disc_image;
    size_t   fmtowns_disc_image_size;
    uint32_t fmtowns_cdda_track_starts[9]; /* track_starts[2..8] */
    int      fmtowns_cdda_track_count;
    /* Phar Lap P3 executable receipts for the selected native programs.
     * They record only bounded header facts from in-memory disc members; no
     * program image is unpacked or executed by either receipt. */
    DM2_V1_FmtownsP3Receipt fmtowns_twanim_p3;
    DM2_V1_FmtownsP3Receipt fmtowns_skull_p3;
    /* HMP-to-CDDA selection read from the native SKULL.EXP member in the
     * selected disc buffer.  The receipt owns only copied table bytes. */
    DM2_V1_FmtownsCddaMusicReceipt fmtowns_cdda_music;
    /* Original HME-242 AUTOEXEC route, read directly from the selected
     * in-memory IMG.  A Towns session is not admitted without this media. */
    DM2_V1_FmtownsStartupPlan fmtowns_startup_plan;
    int      fmtowns_startup_media_verified;
    /* Retail animation-stream identities, calculated from the selected IMG
     * in RAM.  These prevent a name-compatible replacement TITLE/SWOOSH/END
     * stream from being presented as the HME-242 startup. */
    char     fmtowns_swoosh_md5[33];
    char     fmtowns_title_md5[33];
    char     fmtowns_end_md5[33];
    int      fmtowns_animation_media_verified;
    /* Bounded record receipts for the selected retail streams.  They are
     * source data facts, not a claim that the host has rendered a frame. */
    DM2_V1_FmtownsAnimStreamReceipt fmtowns_swoosh_stream;
    DM2_V1_FmtownsAnimStreamReceipt fmtowns_title_stream;
    DM2_V1_FmtownsAnimStreamReceipt fmtowns_end_stream;
    int      fmtowns_animation_streams_verified;

    /* ── Deterministic config ──────────────────────────────── */
    DM2_V1_DeterministicConfig deterministic;

    /* ── Runtime references (set after boot) ──────────────── */
    void   *dm2_state;         /* DM2_V1_GameState* — set by dm2_v1_boot_enter_game() */
    void   *dungeon_data;      /* DM2_V1_DungeonData* — parsed dungeon */
    void   *graphics_dat;      /* graphics data handle */
} DM2_V1_BootProfile;

/* Returns only the verified source material and source draw semantics for
 * skproject's save/load dialogue. The host must expand RECT_453 before it
 * can render the plan. */
int dm2_v1_boot_dialogue_box_draw_plan(
    const DM2_V1_BootProfile *profile,
    DM2_V1_DialogueBoxDrawPlan *out);

/* Resolve the exact dtImage selected by SKProject _2405_014a for a
 * leader-hand object. Missing source state returns zero; callers must not
 * substitute a generic map-chip image. */
int dm2_v1_boot_leader_hand_image_field(
    const DM2_V1_BootProfile *profile,
    int gdat_category,
    int gdat_index,
    uint32_t object_index,
    uint32_t game_tick,
    int party_direction,
    uint8_t *out_image_field);

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

/* skproject c_savegame.cpp::DM2_LOAD_NEW_DUNGEON opens the selected original
 * dungeon, clears the prior party/leader ownership, then calls
 * DM2_READ_DUNGEON_STRUCTURE(1). Firestaff mirrors that clear but does not
 * manufacture a replacement party before source mirror selection. */
typedef struct {
    int valid;
    int reloaded;
    int source_party_reset_required;
    int source_leader_hand_reset_required;
    /* Set only after Firestaff's retained source-save cache is observed
     * empty.  These are deliberately distinct from the source requirements
     * above: an M11 handoff may not treat a requested clear as a completed
     * GAME_LOAD boundary. */
    int source_party_reset_applied;
    int source_leader_hand_reset_applied;
    int synthetic_party_created;
    int map_count;
    int dungeon_seed;
    uint32_t raw_byte_count;
    uint32_t raw_hash;
} DM2_V1_BootNewDungeonReceipt;

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
    int runtime_hud_raw_gdat_capture_ready;
    int runtime_hud_raw_portrait_count;
    uint32_t runtime_hud_raw_portrait_hash;
    uint32_t runtime_hud_raw_portrait_byte_count;
    uint32_t runtime_hud_raw_core_hash;
    uint32_t runtime_hud_raw_core_byte_count;
    int runtime_hud_raw_interface_count;
    int runtime_hud_decoded_gdat_capture_ready;
    int runtime_hud_decoded_portrait_count;
    uint32_t runtime_hud_decoded_portrait_hash;
    uint32_t runtime_hud_decoded_portrait_pixel_count;
    uint32_t runtime_hud_decoded_core_hash;
    uint32_t runtime_hud_decoded_core_pixel_count;
    int runtime_hud_decoded_interface_count;
    uint32_t runtime_hud_frame_hash;
    uint32_t runtime_hud_frame_pixel_count;
    int runtime_render_real_asset_ready;
    int runtime_render_asset_floor_ceiling_count;
    int runtime_render_fallback_floor_ceiling_count;
    int runtime_render_asset_wall_count;
    int runtime_render_fallback_wall_count;
    int runtime_render_asset_door_panel_count;
    int runtime_render_asset_door_overlay_count;
    int runtime_render_asset_door_frame_count;
    int runtime_render_asset_door_button_count;
    int runtime_render_fallback_door_count;
    int runtime_render_asset_creature_count;
    int runtime_render_fallback_creature_count;
    int runtime_render_asset_item_count;
    int runtime_render_fallback_item_count;
    int runtime_render_asset_creature_possession_item_count;
    int runtime_render_fallback_creature_possession_item_count;
    int runtime_render_asset_carried_item_count;
    int runtime_render_fallback_carried_item_count;
    int runtime_render_asset_projectile_count;
    int runtime_render_fallback_projectile_count;
    int runtime_render_no_core_fallbacks;
    int runtime_render_blocked_material_draw_count;
    /* Atomic source-required frame identity consumed by M11. */
    int runtime_m11_frame_receipt_consumed;
    /* Raw SKSave layout identity consumed by this runtime frame, if that
     * frame follows a validated original GAME_LOAD handoff. */
    int runtime_raw_sksave_handoff_consumed;
    uint32_t runtime_raw_sksave_prefix_hash;
    uint32_t runtime_raw_sksave_map_data_hash;
    uint32_t runtime_raw_sksave_dungeon_byte_count;
    uint32_t runtime_raw_sksave_db_record_count;
    uint32_t runtime_m11_frame_map_load_token;
    uint32_t runtime_m11_frame_scene_control_hash;
    uint32_t runtime_m11_frame_scene_light_hash;
    uint32_t runtime_m11_frame_presentation_state_hash;
    uint32_t runtime_m11_frame_scene_ambient_light;
    int runtime_m11_frame_weather_graphicsset_bound;
    uint32_t runtime_m11_frame_weather_graphicsset;
    uint32_t runtime_m11_frame_weather_source_receipt_hash;
    uint32_t runtime_m11_frame_weather_destination_receipt_hash;
    uint32_t runtime_m11_frame_floor_material_hash;
    uint32_t runtime_m11_frame_ceiling_material_hash;
    uint32_t runtime_m11_frame_wall_material_plan_hash;
    int runtime_m11_frame_wall_material_plan_command_count;
    int runtime_m11_frame_door_material_plan_required;
    uint32_t runtime_m11_frame_door_material_plan_hash;
    int runtime_m11_frame_door_material_plan_command_count;
    int runtime_m11_frame_door_material_plan_consumed;
    int runtime_m11_frame_hud_material_plan_required;
    uint32_t runtime_m11_frame_hud_material_plan_hash;
    uint32_t runtime_m11_frame_hud_scene_control_hash;
    int runtime_m11_frame_hud_material_plan_command_count;
    int runtime_m11_frame_hud_material_plan_consumed;
    int runtime_m11_frame_creature_material_plan_required;
    uint32_t runtime_m11_frame_creature_material_plan_hash;
    int runtime_m11_frame_creature_material_plan_command_count;
    int runtime_m11_frame_creature_material_plan_consumed;
    int runtime_m11_frame_projectile_material_plan_required;
    uint32_t runtime_m11_frame_projectile_material_plan_hash;
    int runtime_m11_frame_projectile_material_plan_command_count;
    int runtime_m11_frame_projectile_material_plan_consumed;
    int runtime_m11_frame_item_material_plan_required;
    uint32_t runtime_m11_frame_item_material_plan_hash;
    uint32_t runtime_m11_frame_item_scene_control_hash;
    int runtime_m11_frame_item_material_plan_command_count;
    int runtime_m11_frame_item_material_plan_consumed;
    int runtime_m11_frame_weather_material_plan_required;
    uint32_t runtime_m11_frame_weather_material_plan_hash;
    int runtime_m11_frame_weather_material_plan_command_count;
    int runtime_m11_frame_weather_material_plan_consumed;
    int runtime_m11_frame_teleporter_material_plan_required;
    uint32_t runtime_m11_frame_teleporter_material_plan_hash;
    int runtime_m11_frame_teleporter_material_plan_consumed;
    int runtime_m11_frame_floor_gfx_map_chip_material_plan_required;
    uint32_t runtime_m11_frame_floor_gfx_map_chip_material_plan_hash;
    int runtime_m11_frame_floor_gfx_map_chip_material_plan_consumed;
    int runtime_m11_frame_wall_gfx_map_chip_material_plan_required;
    uint32_t runtime_m11_frame_wall_gfx_map_chip_material_plan_hash;
    int runtime_m11_frame_wall_gfx_map_chip_material_plan_consumed;
    int runtime_m11_frame_door_map_chip_material_plan_required;
    uint32_t runtime_m11_frame_door_map_chip_material_plan_hash;
    int runtime_m11_frame_door_map_chip_material_plan_consumed;
    uint32_t runtime_m11_frame_palette_hash;
    uint32_t runtime_m11_frame_interface_action_palette_hash;
    int runtime_m11_frame_interface_action_palette_consumed;
    int runtime_m11_frame_interface_rect14_required;
    int runtime_m11_frame_interface_rect14_consumed;
    uint32_t runtime_m11_frame_interface_rect14_table_hash;
    uint32_t runtime_m11_frame_interface_rect14_placement_hash;
    uint32_t runtime_m11_frame_interface_rect14_row_count;
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
    int total_asset_door_panel_count;
    int total_asset_door_overlay_count;
    int total_asset_door_frame_count;
    int total_asset_door_button_count;
    int total_fallback_door_count;
    int total_asset_creature_count;
    int total_fallback_creature_count;
    int total_asset_item_count;
    int total_fallback_item_count;
    int total_asset_creature_possession_item_count;
    int total_fallback_creature_possession_item_count;
    int total_asset_carried_item_count;
    int total_fallback_carried_item_count;
    int total_asset_projectile_count;
    int total_fallback_projectile_count;
    int min_asset_floor_ceiling_count;
    int min_asset_wall_count;
    int no_core_render_fallbacks;
    int first_runtime_hud_ready;
    int real_gdat_portrait_ready;
    int real_gdat_core_render_ready;
    int real_gdat_runtime_hud_breadth_ready;
    int raw_gdat_runtime_hud_capture_ready;
    int raw_gdat_runtime_portrait_count;
    uint32_t raw_gdat_runtime_portrait_hash;
    uint32_t raw_gdat_runtime_portrait_byte_count;
    uint32_t raw_gdat_runtime_core_hash;
    uint32_t raw_gdat_runtime_core_byte_count;
    int raw_gdat_runtime_interface_count;
    int decoded_gdat_runtime_hud_capture_ready;
    int decoded_gdat_runtime_portrait_count;
    uint32_t decoded_gdat_runtime_portrait_hash;
    uint32_t decoded_gdat_runtime_portrait_pixel_count;
    uint32_t decoded_gdat_runtime_core_hash;
    uint32_t decoded_gdat_runtime_core_pixel_count;
    int decoded_gdat_runtime_interface_count;
    int teleporter_map_chip_ready;
    uint32_t teleporter_map_chip_raw_hash;
    uint32_t teleporter_map_chip_raw_byte_count;
    uint32_t teleporter_map_chip_decoded_hash;
    uint32_t teleporter_map_chip_decoded_pixel_count;
    int dungeon_map_chip_ready;
    int dungeon_map_chip_graphicsset_count;
    int dungeon_map_chip_wall_count;
    int dungeon_map_chip_floor_count;
    int dungeon_map_chip_graphicsset_ready;
    int dungeon_map_chip_wall_ready;
    int dungeon_map_chip_floor_ready;
    uint32_t dungeon_map_chip_raw_hash;
    uint32_t dungeon_map_chip_raw_byte_count;
    uint32_t dungeon_map_chip_decoded_hash;
    uint32_t dungeon_map_chip_decoded_pixel_count;
    uint32_t dungeon_map_chip_graphicsset_raw_hash;
    uint32_t dungeon_map_chip_graphicsset_raw_byte_count;
    uint32_t dungeon_map_chip_graphicsset_decoded_hash;
    uint32_t dungeon_map_chip_graphicsset_decoded_pixel_count;
    uint32_t dungeon_map_chip_wall_raw_hash;
    uint32_t dungeon_map_chip_wall_raw_byte_count;
    uint32_t dungeon_map_chip_wall_decoded_hash;
    uint32_t dungeon_map_chip_wall_decoded_pixel_count;
    uint32_t dungeon_map_chip_floor_raw_hash;
    uint32_t dungeon_map_chip_floor_raw_byte_count;
    uint32_t dungeon_map_chip_floor_decoded_hash;
    uint32_t dungeon_map_chip_floor_decoded_pixel_count;
    int graphicsset_word_values_ready;
    uint32_t graphicsset_word_values_hash;
    uint32_t graphicsset_word_values_present_mask;
    uint32_t graphicsset_word_values_query_count;
    uint32_t graphicsset_scene_flags;
    uint32_t graphicsset_scene_colorkey;
    uint32_t graphicsset_ambient_light;
    uint32_t graphicsset_highest_light_level;
    uint32_t graphicsset_void_random_fall;
    uint32_t graphicsset_animated_floor;
    uint32_t graphicsset_scene_rain;
    uint32_t graphicsset_misty_map;
    uint32_t graphicsset_thunder_position;
    uint32_t graphicsset_ambient_darkness;
    int wall_gfx_image_offsets_ready;
    uint32_t wall_gfx_image_offsets_hash;
    uint32_t wall_gfx_image_offsets_query_count;
    uint32_t wall_gfx_image_offsets_nonzero_count;
    uint32_t wall_gfx_image_offsets_present_mask;
    int interface_rect14_ready;
    uint32_t interface_rect14_hash;
    uint32_t interface_rect14_byte_count;
    uint32_t interface_rect14_row_count;
    uint32_t interface_rect14_stride;
    uint32_t interface_rect14_nonzero_5x5_count;
    uint32_t interface_rect14_image_field_count;
    uint32_t interface_rect14_stretch_field_count;
    uint32_t interface_rect14_flag_field_count;
    int interface_rect14_placement_plan_ready;
    uint32_t interface_rect14_placement_hash;
    uint32_t interface_rect14_placement_count;
    uint32_t interface_rect14_rotated_cell_mask;
    uint32_t interface_rect14_max_stretched_size;
    int interface_action_table_ready;
    uint32_t interface_action_table_hash;
    uint32_t interface_action_table_byte_count;
    uint32_t interface_action_group_count;
    uint32_t interface_action_entry_count;
    uint32_t interface_action_tail_byte_count;
    int interface_font_table_ready;
    uint32_t interface_font_table_hash;
    uint32_t interface_font_table_byte_count;
    uint32_t interface_font_table_row_count;
    uint32_t interface_font_table_char_count;
    uint32_t interface_font_table_nonzero_byte_count;
    uint32_t interface_font_table_printable_char_count;
    int interface_palette_ready;
    uint32_t interface_palette_hash;
    uint32_t interface_palette_irgb_byte_count;
    uint32_t interface_palette_pal16_byte_count;
    uint32_t interface_palette_irgb_color_count;
    uint32_t interface_palette_pal16_color_count;
    uint32_t combined_frame_hash;
    uint32_t combined_pixel_count;
    DM2_V1_BootRuntimeRenderReceipt first_frame;
} DM2_V1_BootRuntimeHudCaptureReceipt;

typedef struct {
    int valid;
    int profile_ready;
    int graphics_dat_ready;
    int sampled_creature_index_count;
    int materialized_creature_index_count;
    int frame_parity_matrix_count;
    int min_frame_count;
    int max_frame_count;
    uint32_t sampled_creature_mask_low;
    uint32_t sampled_creature_mask_high;
    uint32_t raw_gdat_hash;
    uint32_t raw_gdat_byte_count;
    uint32_t decoded_gdat_hash;
    uint32_t decoded_gdat_pixel_count;
    int animation_attribution_count;
    int animation_info_sequence_count;
    int animation_frame_sequence_count;
    /* A creature animation route is source-owned only when all three
     * SKProject table payloads resolve for one CREATURES index. */
    int animation_complete_creature_index_count;
    uint32_t animation_complete_creature_mask_low;
    uint32_t animation_complete_creature_mask_high;
    uint32_t animation_complete_creature_hash;
    uint32_t animation_table_hash;
    uint32_t animation_table_byte_count;
    int animation_table_ready;
    uint32_t frame_parity_hash;
    uint32_t atlas_material_hash;
} DM2_V1_BootCreatureAtlasCaptureReceipt;

/* skproject SkWinCore::EXTENDED_LOAD_SPELLS_DEFINITION reads the custom
 * spell family from SPELL_DEF/index/dtWordValue fields 1..7. This is a
 * boot-owned receipt, not a parallel spell table: its hash proves the exact
 * original records carried through host and M11 boundaries. */
typedef struct {
    int loaded;
    uint32_t spell_count;
    uint32_t gdat_hash;
} DM2_V1_ExtendedSpellsDefinitionReceipt;

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
    int menu_gdat_category;
    int menu_gdat_index;
    int menu_gdat_field;
    int title_menu_raw_gdat_capture_ready;
    uint32_t title_raw_gdat_hash;
    uint32_t title_raw_gdat_byte_count;
    uint32_t menu_raw_gdat_hash;
    uint32_t menu_raw_gdat_byte_count;
    int title_menu_decoded_gdat_capture_ready;
    uint32_t title_decoded_gdat_hash;
    uint32_t title_decoded_gdat_pixel_count;
    uint32_t menu_decoded_gdat_hash;
    uint32_t menu_decoded_gdat_pixel_count;
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
    int hud_raw_gdat_capture_ready;
    int hud_raw_gdat_portrait_count;
    uint32_t hud_raw_gdat_portrait_hash;
    uint32_t hud_raw_gdat_portrait_byte_count;
    uint32_t hud_raw_gdat_core_hash;
    uint32_t hud_raw_gdat_core_byte_count;
    int runtime_menu_ready;
    int runtime_action_ready;
    int first_hud_frame_ready;
    int full_start_graphics_ready;
    int full_start_real_asset_ready;
    DM2_V1_ExtendedSpellsDefinitionReceipt extended_spells;
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
    int menu_gdat_category;
    int menu_gdat_index;
    int menu_gdat_field;
    int title_menu_raw_gdat_capture_ready;
    uint32_t title_raw_gdat_hash;
    uint32_t title_raw_gdat_byte_count;
    uint32_t menu_raw_gdat_hash;
    uint32_t menu_raw_gdat_byte_count;
    int title_menu_decoded_gdat_capture_ready;
    uint32_t title_decoded_gdat_hash;
    uint32_t title_decoded_gdat_pixel_count;
    uint32_t menu_decoded_gdat_hash;
    uint32_t menu_decoded_gdat_pixel_count;
    int menu_row_count;
    int menu_text_count;
    int selectable_text_count;
    int selected_highlight_count;
    int menu_panel_ready;
    int startup_menu_assets_ready;
    int hud_overlay_suppressed;
    int hud_runtime_ready;
    int hud_raw_gdat_capture_ready;
    int hud_raw_gdat_portrait_count;
    uint32_t hud_raw_gdat_portrait_hash;
    uint32_t hud_raw_gdat_portrait_byte_count;
    uint32_t hud_raw_gdat_core_hash;
    uint32_t hud_raw_gdat_core_byte_count;
    int first_hud_frame_ready;
    DM2_V1_ExtendedSpellsDefinitionReceipt extended_spells;
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
    int menu_gdat_category;
    int menu_gdat_index;
    int menu_gdat_field;
    int title_menu_raw_gdat_capture_ready;
    uint32_t title_raw_gdat_hash;
    uint32_t title_raw_gdat_byte_count;
    uint32_t menu_raw_gdat_hash;
    uint32_t menu_raw_gdat_byte_count;
    int title_menu_decoded_gdat_capture_ready;
    uint32_t title_decoded_gdat_hash;
    uint32_t title_decoded_gdat_pixel_count;
    uint32_t menu_decoded_gdat_hash;
    uint32_t menu_decoded_gdat_pixel_count;
    int menu_row_count;
    int menu_text_count;
    int selectable_text_count;
    int selected_highlight_count;
    int menu_panel_ready;
    int startup_menu_assets_ready;
    int hud_overlay_suppressed;
    int hud_runtime_ready;
    int hud_raw_gdat_capture_ready;
    int hud_raw_gdat_portrait_count;
    uint32_t hud_raw_gdat_portrait_hash;
    uint32_t hud_raw_gdat_portrait_byte_count;
    uint32_t hud_raw_gdat_core_hash;
    uint32_t hud_raw_gdat_core_byte_count;
    DM2_V1_ExtendedSpellsDefinitionReceipt extended_spells;
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
    int startup_hud_raw_gdat_capture_ready;
    int startup_hud_raw_gdat_portrait_count;
    uint32_t startup_hud_raw_gdat_portrait_hash;
    uint32_t startup_hud_raw_gdat_portrait_byte_count;
    uint32_t startup_hud_raw_gdat_core_hash;
    uint32_t startup_hud_raw_gdat_core_byte_count;
    int startup_title_menu_raw_gdat_capture_ready;
    uint32_t startup_title_raw_gdat_hash;
    uint32_t startup_title_raw_gdat_byte_count;
    uint32_t startup_menu_raw_gdat_hash;
    uint32_t startup_menu_raw_gdat_byte_count;
    int startup_title_menu_decoded_gdat_capture_ready;
    uint32_t startup_title_decoded_gdat_hash;
    uint32_t startup_title_decoded_gdat_pixel_count;
    uint32_t startup_menu_decoded_gdat_hash;
    uint32_t startup_menu_decoded_gdat_pixel_count;
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
    DM2_V1_ExtendedSpellsDefinitionReceipt extended_spells;
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
    int startup_hud_raw_gdat_capture_ready;
    int startup_hud_raw_gdat_portrait_count;
    uint32_t startup_hud_raw_gdat_portrait_hash;
    uint32_t startup_hud_raw_gdat_portrait_byte_count;
    uint32_t startup_hud_raw_gdat_core_hash;
    uint32_t startup_hud_raw_gdat_core_byte_count;
    int startup_title_menu_raw_gdat_capture_ready;
    uint32_t startup_title_raw_gdat_hash;
    uint32_t startup_title_raw_gdat_byte_count;
    uint32_t startup_menu_raw_gdat_hash;
    uint32_t startup_menu_raw_gdat_byte_count;
    int startup_title_menu_decoded_gdat_capture_ready;
    uint32_t startup_title_decoded_gdat_hash;
    uint32_t startup_title_decoded_gdat_pixel_count;
    uint32_t startup_menu_decoded_gdat_hash;
    uint32_t startup_menu_decoded_gdat_pixel_count;
    DM2_V1_ExtendedSpellsDefinitionReceipt extended_spells;
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
    int menu_gdat_command_count;
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
    int startup_hud_raw_gdat_receipt_consumed;
    int startup_title_menu_raw_gdat_receipt_consumed;
    uint32_t startup_title_raw_gdat_hash;
    uint32_t startup_title_raw_gdat_byte_count;
    uint32_t startup_menu_raw_gdat_hash;
    uint32_t startup_menu_raw_gdat_byte_count;
    int startup_title_menu_decoded_gdat_receipt_consumed;
    uint32_t startup_title_decoded_gdat_hash;
    uint32_t startup_title_decoded_gdat_pixel_count;
    uint32_t startup_menu_decoded_gdat_hash;
    uint32_t startup_menu_decoded_gdat_pixel_count;
    DM2_V1_ExtendedSpellsDefinitionReceipt extended_spells;
    int extended_spells_definition_consumed;
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
    int sampled_title_unique_pixel_hash_count;
    uint32_t sampled_title_pixel_hash;
    int sampled_menu_selection_capture_count;
    int sampled_menu_selection_mask;
    int sampled_menu_composite_capture_count;
    int sampled_menu_unique_composite_hash_count;
    uint32_t sampled_menu_composite_hash;
    int sampled_runtime_hud_handoff_capture_ready;
    /* Runtime-HUD fields are intentionally zero during SHOW_MENU_SCREEN,
     * which occurs before GAME_LOAD creates a source party. They are retained
     * for the separate post-GAME_LOAD runtime-HUD proof. */
    int runtime_hud_capture_consumed;
    int runtime_hud_real_gdat_ready;
    int runtime_hud_direction_mask;
    int runtime_hud_sample_count;
    int runtime_hud_unique_frame_hash_count;
    int runtime_hud_min_asset_portrait_count;
    int runtime_hud_total_fallback_portrait_count;
    int runtime_hud_min_asset_floor_ceiling_count;
    int runtime_hud_total_fallback_floor_ceiling_count;
    int runtime_hud_min_asset_wall_count;
    int runtime_hud_total_fallback_wall_count;
    int runtime_hud_raw_gdat_capture_ready;
    int runtime_hud_raw_portrait_count;
    uint32_t runtime_hud_raw_portrait_hash;
    uint32_t runtime_hud_raw_portrait_byte_count;
    uint32_t runtime_hud_raw_core_hash;
    uint32_t runtime_hud_raw_core_byte_count;
    int runtime_hud_raw_interface_count;
    int runtime_hud_decoded_gdat_capture_ready;
    int runtime_hud_decoded_portrait_count;
    uint32_t runtime_hud_decoded_portrait_hash;
    uint32_t runtime_hud_decoded_portrait_pixel_count;
    uint32_t runtime_hud_decoded_core_hash;
    uint32_t runtime_hud_decoded_core_pixel_count;
    int runtime_hud_decoded_interface_count;
    uint32_t runtime_hud_frame_hash;
    uint32_t runtime_hud_pixel_count;
    int real_gdat_capture_breadth_ready;
    int real_gdat_title_asset_required;
    int real_gdat_title_asset_consumed;
    int real_gdat_menu_asset_required;
    int real_gdat_menu_asset_consumed;
    int raw_gdat_capture_ready;
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
    uint32_t title_raw_byte_hash;
    uint32_t title_raw_byte_count;
    uint32_t title_pixel_hash;
    uint32_t title_pixel_count;
    int menu_gdat_asset_w;
    int menu_gdat_asset_h;
    int menu_gdat_asset_stride;
    uint32_t menu_raw_byte_hash;
    uint32_t menu_raw_byte_count;
    int menu_raw_screen_route_ready;
    int menu_raw_screen_consumed;
    /* The verified PC-DOS TITLE/0/4 decoded image route. This is original
     * GDAT material, selected only when SHOW_MENU_SCREEN has no raw-screen
     * record; it is not a generated fallback. */
    int menu_decoded_image_route_used;
    uint32_t menu_raw_screen_hash;
    uint32_t menu_raw_screen_byte_count;
    uint32_t menu_pixel_hash;
    uint32_t menu_pixel_count;
    int full_visual_composite_capture_ready;
    int composite_gdat_blit_count;
    int composite_rect_count;
    int composite_text_zone_count;
    int synthetic_menu_overlay_suppressed;
    int synthetic_menu_overlay_command_count;
    int real_menu_screen_no_synthetic_overlay_ready;
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
    DM2_V1_BootStartupRealVisualCaptureReceipt startup_visual;
    DM2_V1_BootRuntimeHudCaptureReceipt runtime_hud;
    DM2_V1_BootCreatureAtlasCaptureReceipt creature_atlas;
    int skproject_gdat_queries_ready;
    int startup_title_menu_complete;
    int startup_hud_handoff_complete;
    int runtime_gdat_hud_complete;
    int runtime_gdat_dungeon_complete;
    int runtime_gdat_map_chip_categories_complete;
    int runtime_gdat_interface_placement_complete;
    int runtime_creature_atlas_complete;
    int runtime_gdat_direction_breadth_complete;
    int no_fallback_title_or_runtime_visuals;
    int raw_gdat_capture_complete;
    int decoded_gdat_capture_complete;
    int save_corpus_scan_complete;
    int save_corpus_valid_candidate_count;
    int save_corpus_importable_candidate_count;
    int save_corpus_rejected_candidate_count;
    int save_corpus_original_candidate_count;
    unsigned int save_corpus_valid_slot_mask;
    uint32_t save_corpus_hash;
    /* Source-owned GAME_LOAD census over header- and file-hash-verified
     * original candidates. This remains observational until the remaining
     * dungeon DB and timer record layouts have exact source contracts. */
    int save_corpus_original_state_scan_complete;
    int save_corpus_original_state_list_complete;
    int save_corpus_original_state_candidate_count;
    int save_corpus_original_state_parsed_candidate_count;
    int save_corpus_original_state_rejected_candidate_count;
    uint32_t save_corpus_original_state_hash;
    int complete_support_ready;
    uint32_t complete_support_hash;
    const char *status_scope;
    const char *status;
} DM2_V1_CompleteSupportReceipt;

typedef struct {
    int valid;
    uint32_t table_hash;
    uint32_t row_count;
    uint32_t placement_hash;
    uint32_t placement_count;
    uint32_t rotated_cell_mask;
    uint32_t max_stretched_size;
} DM2_V1_InterfaceRect14HostReceipt;

typedef struct {
    int valid;
    int host_receipt_consumed;
    uint32_t table_hash;
    uint32_t row_count;
    uint32_t stride;
    uint32_t byte_count;
    uint32_t placement_hash;
    uint32_t placement_count;
    uint32_t rotated_cell_mask;
    uint32_t max_stretched_size;
    uint32_t receipt_hash;
} DM2_V1_LoadGdatInterface000AReceipt;

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
    int interface_rect14_host_ready;
    DM2_V1_InterfaceRect14HostReceipt interface_rect14;
    int extended_spell_gdat_ready;
    uint32_t extended_spell_gdat_defined_count;
    uint32_t extended_spell_gdat_word_hash;
    int m11_host_view_ready;
    DM2_V1_ExtendedSpellsDefinitionReceipt extended_spells;
    const char *status_scope;
    const char *status;
    const char *log_line;
    DM2_V1_BootStartupFullStartReceipt full_start;
    int capture_proof_valid;
    DM2_V1_BootStartupPackagedCaptureProof capture_proof;
} DM2_V1_BootStartupHostViewReceipt;

typedef struct {
    int valid;
    uint32_t defined_count;
    uint32_t word_hash;
} DM2_V1_ExtendedSpellGdatReceipt;

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

/* Source: skproject/SKULLWIN/dm2data.cpp tblMusicsMap and
 * SKULLWIN/c_sound.cpp DM2_GET_MUSIC_INDEX_FROM_MODLIST. Returns 1 only
 * for a hash-verified original PC SONGLIST.DAT selector. */
int dm2_v1_boot_songlist_track_for_map(const DM2_V1_BootProfile *profile,
                                       int map_index, int *out_track);

/* Unified music track lookup: dispatches to the correct music system
 * (songlist, music_map, or cdda) based on the detected platform.
 * For CDDA platforms, x/y are the party's tile coordinates.
 * For songlist/music_map platforms, x/y are ignored.
 * Returns 1 if a track was resolved, 0 otherwise. */
int dm2_v1_boot_music_track_for_level(const DM2_V1_BootProfile *profile,
                                       int level_index,
                                       int x, int y,
                                       int *out_track);

/* Load a CDDA track directly from the selected, hash-verified FM Towns
 * original disc image. No unpacked track directory is accepted. disc_track
 * is the 1-based disc track number (2-8 for DM2 FM Towns). Caller must
 * free(*out_data) when done. `out_media_verified` is set only when the
 * returned bytes are bound to the verified game-data and CD.DAT receipts. */
size_t dm2_v1_boot_load_cdda_track(const DM2_V1_BootProfile *profile,
                                    int disc_track,
                                    uint8_t **out_data,
                                    int *out_media_verified);

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

/* Performs only the source-owned DUNGEON.DAT reload portion of GAME_LOAD.
 * It rechecks the selected asset hash when one was verified at boot and swaps
 * the parsed G1 data only after a complete candidate parse succeeds.
 * It also clears Firestaff's retained source-save party/leader cache and
 * verifies that cache is empty before publishing the reload receipt. This
 * function never creates a Firestaff starter party; original champion,
 * inventory, actuator and timer ownership remains with later GAME_LOAD
 * work. */
int dm2_v1_boot_load_new_dungeon(
    DM2_V1_BootProfile *profile,
    DM2_V1_BootNewDungeonReceipt *out_receipt);

/* Allocate and prepare a DM2 boot profile through the verified game-entry
 * boundary. The caller still owns M11 startup menu/session receipts, but DM2
 * owns profile allocation, asset scanning, save-root setup, and enter_game. */
int dm2_v1_boot_startup_launch_alloc(
    const char *data_dir,
    DM2_V1_BootStartupLaunch *out_launch);

/* FM Towns is a Japanese retail edition.  An English request therefore needs
 * an explicit, user-selected PC-English GRAPHICS.DAT companion with the
 * canonical MD5; no host-path discovery or generated translation is allowed.
 * `language_index` follows M12's stable language ordering (0 == English).
 * Other locales retain the selected retail corpus unchanged. */
int dm2_v1_boot_startup_launch_alloc_with_language(
    const char *data_dir,
    const char *english_companion_graphics_path,
    int language_index,
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
int dm2_v1_boot_startup_execute_original_pointer_from_runtime_state(
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
void dm2_v1_boot_creature_atlas_capture_receipt_init(
    DM2_V1_BootCreatureAtlasCaptureReceipt *receipt);
int dm2_v1_boot_creature_atlas_capture_receipt(
    DM2_V1_BootProfile *profile,
    DM2_V1_BootCreatureAtlasCaptureReceipt *out_receipt);
void dm2_v1_boot_complete_support_receipt_init(
    DM2_V1_CompleteSupportReceipt *receipt);
int dm2_v1_boot_complete_support_receipt_from_runtime_state(
    DM2_V1_BootProfile *profile,
    int startup_menu_active,
    const char *startup_save_root,
    int resume_available,
    unsigned int slot_mask,
    int selected_row,
    int title_animation_tick,
    DM2_V1_CompleteSupportReceipt *out_receipt);
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

/* Source-locked interface palette used by the runtime HUD/viewport handoff.
 * SkWinCore::INIT loads dtPalIRGB and dtPalette16 before entering the game. */
int dm2_v1_boot_interface_palette(DM2_V1_BootProfile *profile,
                                  DM2_V1_InterfacePalette *out_palette);

/* skproject LOAD_GDAT_INTERFACE_00_02 materialises dt07/2 as a group-count
 * byte, one length byte per group, one primary and one secondary variable
 * span per group, then a command tail. Storage is owned by graphics_dat. */
#define DM2_V1_INTERFACE_ACTION_GROUP_MAX 255u
typedef struct {
    uint8_t length;
    uint32_t primary_offset;
    uint32_t secondary_offset;
} DM2_V1_InterfaceActionGroup;

typedef struct {
    int valid;
    const uint8_t *raw;
    uint32_t raw_size;
    uint32_t hash;
    uint32_t group_count;
    uint32_t entry_count;
    uint32_t tail_offset;
    uint32_t tail_size;
    DM2_V1_InterfaceActionGroup groups[DM2_V1_INTERFACE_ACTION_GROUP_MAX];
} DM2_V1_InterfaceActionTable;

int dm2_v1_boot_interface_action_table(
    DM2_V1_BootProfile *profile,
    DM2_V1_InterfaceActionTable *out_table);
int dm2_v1_boot_extended_spell_gdat_receipt(
    DM2_V1_BootProfile *profile,
    DM2_V1_ExtendedSpellGdatReceipt *out_receipt);

/* ReDMCSB/skproject SkWinCore.cpp _0b36_037e uses the dt07/2 tail as 256
 * (group, threshold) pairs, then selects the nearest source threshold in
 * that group's pv1 span and returns the matching pv5 palette entry. */
int dm2_v1_interface_action_table_remap_palette(
    const DM2_V1_InterfaceActionTable *table,
    uint8_t *palette,
    uint32_t palette_count,
    uint8_t darkness_0_to_64,
    int colorkey1,
    int colorkey2);

/* skproject QUERY_FONT reads this six-row, 128-glyph dt07/0 table directly.
 * Storage remains boot-owned and is valid while profile->graphics_dat lives. */
int dm2_v1_boot_interface_font_table(
    DM2_V1_BootProfile *profile,
    const uint8_t **out_rows,
    uint32_t *out_hash);

/* Bridges a source-proven G1 DB2 text receipt to boot-owned GDAT only. */
int dm2_v1_boot_g1_text_wall_gfx_materials(
    DM2_V1_BootProfile *profile,
    const DM2_V1_G1Map5TextRuntimeReceipt *texts,
    DM2_V1_G1TextWallGfxRuntimeReceipt *out);
int dm2_v1_boot_g1_gdat_text_materials(
    DM2_V1_BootProfile *profile,
    const DM2_V1_G1Map5TextRuntimeReceipt *texts,
    DM2_V1_G1GdatTextMessageRuntimeReceipt *out);
int dm2_v1_boot_g1_actuator_wall_gfx_materials(
    DM2_V1_BootProfile *profile,
    int map,
    DM2_V1_G1ActuatorWallGfxRuntimeReceipt *out);
int dm2_v1_boot_g1_creature_map_chip_materials(
    DM2_V1_BootProfile *profile,
    int map,
    DM2_V1_G1CreatureMapChipRuntimeReceipt *out);
int dm2_v1_boot_g1_weapon_map_chip_materials(
    DM2_V1_BootProfile *profile,
    int map,
    DM2_V1_G1WeaponMapChipRuntimeReceipt *out);
int dm2_v1_boot_g1_container_map_chip_materials(
    DM2_V1_BootProfile *profile,
    int map,
    DM2_V1_G1ContainerMapChipRuntimeReceipt *out);

#define DM2_V1_INTERFACE_HUD_CHAMPION_COUNT 4u
typedef struct {
    int x;
    int y;
    int w;
    int h;
} DM2_V1_InterfaceRect;

typedef struct {
    int valid;
    uint16_t rect_id;
    const uint8_t *raw4_bytes;
    size_t raw4_byte_count;
    uint32_t raw4_hash;
    DM2_V1_InterfaceRect rect;
    uint32_t receipt_hash;
} DM2_V1_BootExpandedRectReceipt;

/* Exact c_xrect.cpp QUERY_EXPANDED_RECT owner: the compressed RAW4 table is
 * retained with the expanded result, so M11 consumers cannot substitute a
 * host rectangle or a different interface table. */
int dm2_v1_boot_query_expanded_rect_receipt(
    const DM2_V1_BootProfile *profile, uint16_t rect_id,
    DM2_V1_BootExpandedRectReceipt *out_receipt);

int dm2_v1_boot_g1_static_object_material_receipt(
    const DM2_V1_BootProfile *profile,
    const DM2_V1_G1StaticObjectMaterialSelector *selector,
    uint16_t clip_rect_id, DM2_V1_G1StaticObjectMaterialReceipt *out_receipt);
int dm2_v1_boot_g1_flying_item_material_receipt(
    const DM2_V1_BootProfile *profile,
    const DM2_V1_G1FlyingItemSourceReceipt *source,
    DM2_V1_G1FlyingItemMaterialReceipt *out_receipt);
int dm2_v1_boot_g1_static_weapon_selector(
    const DM2_V1_BootProfile *profile, const DM2_V1_G1DirectWeaponRoot *root,
    DM2_V1_G1StaticObjectMaterialSelector *out_selector);
int dm2_v1_boot_g1_static_container_selector(
    const DM2_V1_BootProfile *profile, const DM2_V1_G1DirectContainerRoot *root,
    DM2_V1_G1StaticObjectMaterialSelector *out_selector);

/* Source-owned host command for c_dialog.cpp::DM2_dialog_2066_3820.  The
 * panel image/palette come from DIALOG_BOXES/0x81/0 and its destination comes
 * from the original INTERFACE_GENERAL raw4 rectangle table. */
typedef struct {
    int valid;
    DM2_V1_DialogueBoxDrawPlan draw;
    DM2_V1_InterfaceRect rect;
    uint32_t command_hash;
} DM2_V1_DialogueBoxHostCommand;

int dm2_v1_boot_dialogue_box_host_command(
    DM2_V1_BootProfile *profile,
    DM2_V1_DialogueBoxHostCommand *out_command);

/* Source-owned opening orchestration for DM2_dialog_OPEN_DIALOG_PANEL.
 * All placement comes from the original INTERFACE_GENERAL raw4 table. */
typedef struct {
    int valid;
    DM2_V1_DialogueOpenPanelReceipt draw;
    DM2_V1_InterfaceRect panel_rect;
    DM2_V1_InterfaceRect version_rect;
    DM2_V1_InterfaceRect primary_button_rect;
    DM2_V1_InterfaceRect secondary_button_rect;
    DM2_V1_InterfaceRect save_list_rect;
    DM2_V1_InterfaceRect version_text_rect;
    DM2_V1_InterfaceRect primary_text_rect;
    DM2_V1_InterfaceRect secondary_text_rect;
    uint32_t command_hash;
} DM2_V1_DialogueOpenPanelHostCommand;

int dm2_v1_boot_dialogue_open_panel_host_command(
    DM2_V1_BootProfile *profile,
    DM2_V1_DialogueOpenPanelHostCommand *out_command);

/* skproject c_dialog.cpp/c_savegame.cpp receive the two rectangle IDs from
 * the original event queue.  This host receipt resolves only those supplied
 * IDs and the source row formula; it never substitutes a click layout. */
typedef struct {
    int valid;
    uint16_t event_rect_index;
    uint16_t event_top_left_index;
    DM2_V1_InterfaceRect event_rect;
    int top_left_x;
    int top_left_y;
    int row_stride;
    int selected_slot;
    uint32_t command_hash;
} DM2_V1_DialogueSavePointerReceipt;

int dm2_v1_boot_dialogue_save_pointer_receipt(
    DM2_V1_BootProfile *profile,
    uint16_t event_rect_index,
    uint16_t event_top_left_index,
    int pointer_y,
    DM2_V1_DialogueSavePointerReceipt *out_receipt);
typedef struct {
    int valid;
    uint32_t table_hash;
    DM2_V1_InterfaceRect portrait[DM2_V1_INTERFACE_HUD_CHAMPION_COUNT];
    DM2_V1_InterfaceRect name[DM2_V1_INTERFACE_HUD_CHAMPION_COUNT];
    DM2_V1_InterfaceRect status[DM2_V1_INTERFACE_HUD_CHAMPION_COUNT][3];
} DM2_V1_InterfaceHudLayout;

/* skproject _098d_1208 loads INTERFACE_GENERAL/0/dt04/0 and expands these
 * champion rect IDs: names 165..168, portraits 173..176, status 185..204. */
int dm2_v1_boot_interface_hud_layout(
    DM2_V1_BootProfile *profile,
    DM2_V1_InterfaceHudLayout *out_layout);

/* Decodes only the four DRAW_CHAMPION_PICTURE destination rectangles. This
 * smaller receipt remains usable when unrelated HUD status rows are not yet
 * admitted by the broader layout decoder. */
int dm2_v1_boot_interface_hud_portrait_destinations(
    DM2_V1_BootProfile *profile,
    DM2_V1_InterfaceRect out_portraits[DM2_V1_INTERFACE_HUD_CHAMPION_COUNT],
    uint32_t *out_table_hash);

typedef struct {
    int valid;
    uint32_t table_hash;
    DM2_V1_InterfaceRect new_game;
    DM2_V1_InterfaceRect resume_game;
} DM2_V1_StartupMenuPointerLayout;

typedef struct {
    int valid;
    uint32_t table_hash;
    DM2_V1_InterfaceRect show_credits;
    DM2_V1_InterfaceRect quit_game;
    DM2_V1_InterfaceRect dismiss_credits;
} DM2_V1_StartupMenuAuxPointerLayout;

/* skproject SHOW_MENU_SCREEN installs the title-menu rectangle table before
 * HANDLE_UI_EVENT dispatches event 0xD7 (NEW, rect 0x0197) or 0xD9 (RESUME,
 * rect 0x0199). Both routes retain their source-owned GDAT hit rectangles.
 * RESUME stays fail-closed
 * until the boot-owned save scan admits a real SKSave.dat session. */
typedef enum {
    DM2_V1_STARTUP_POINTER_TARGET_NONE = 0,
    DM2_V1_STARTUP_POINTER_TARGET_NEW_GAME,
    DM2_V1_STARTUP_POINTER_TARGET_RESUME_GAME
} DM2_V1_StartupMenuPointerTarget;

typedef struct {
    int valid;
    uint32_t table_hash;
    DM2_V1_StartupMenuPointerTarget target;
    DM2_V1_InterfaceRect rect;
} DM2_V1_StartupMenuPointerHitReceipt;

typedef struct {
    int valid;
    int graphics_dat_ready;
    int title_image_ready;
    int menu_image_ready;
    int pointer_layout_ready;
    int new_game_click_ready;
    int resume_click_surface_ready;
    int interface_palette_ready;
    int hud_static_plan_ready;
    int hud_palette_ready;
    int title_width;
    int title_height;
    int menu_width;
    int menu_height;
    DM2_ImageFormat title_format;
    DM2_ImageFormat menu_format;
    uint32_t title_raw_hash;
    uint32_t title_pixel_hash;
    uint32_t menu_raw_hash;
    uint32_t menu_pixel_hash;
    uint32_t pointer_table_hash;
    uint32_t interface_palette_hash;
    uint32_t hud_static_plan_hash;
    int hud_static_command_count;
    uint32_t receipt_hash;
} DM2_V1_BootStartupMenuHudGdatReceipt;

/* skproject SHOW_MENU_SCREEN installs the raw4 rect table before handling
 * event 0xD7 (NEW) and 0xD9 (RESUME). */
int dm2_v1_boot_startup_menu_pointer_layout(
    DM2_V1_BootProfile *profile,
    DM2_V1_StartupMenuPointerLayout *out_layout);

/* Source rectangle ids 0x019b (credits), 0x01b2 (quit) and 0x0002
 * (credits dismissal) from SHOW_MENU_SCREEN's installed RAW4 table. */
int dm2_v1_boot_startup_menu_aux_pointer_layout(
    DM2_V1_BootProfile *profile,
    DM2_V1_StartupMenuAuxPointerLayout *out_layout);

/* Returns a source-owned 0xD7/0xD9 hit receipt only. The caller resolves
 * 0xD9 through the boot-owned SKSave admission path before it mutates a
 * session. */
int dm2_v1_boot_startup_menu_pointer_hit(
    DM2_V1_BootProfile *profile,
    int x,
    int y,
    DM2_V1_StartupMenuPointerHitReceipt *out_receipt);

/* Pure consumer for an already decoded source layout. This lets callers
 * preserve the 0xD7/0xD9 event boundary without re-reading GDAT. */
int dm2_v1_boot_startup_menu_pointer_hit_from_layout(
    const DM2_V1_StartupMenuPointerLayout *layout,
    int x,
    int y,
    DM2_V1_StartupMenuPointerHitReceipt *out_receipt);

/* Source-owned startup/HUD material join for M11. This proves the visible
 * title/menu GDAT surfaces, 0xD7/0xD9 click rectangles, interface palette,
 * and static HUD chrome command plan are all present in the same verified
 * GRAPHICS.DAT before the dungeon viewport renderer consumes anything. */
int dm2_v1_boot_startup_menu_hud_gdat_receipt(
    DM2_V1_BootProfile *profile,
    DM2_V1_BootStartupMenuHudGdatReceipt *out_receipt);

/* skproject LOAD_GDAT_INTERFACE_00_0A table. Storage remains owned by the
 * boot graphics handle and is valid while profile->graphics_dat is alive. */
int dm2_v1_boot_interface_rect14_table(
    DM2_V1_BootProfile *profile,
    const uint8_t **out_rows,
    uint32_t *out_row_count,
    uint32_t *out_hash);

/* Host-facing dt07/0x0A receipt. It carries only proven placement metadata;
 * pixel decode and drawing remain owned by the original GDAT material path. */
int dm2_v1_boot_interface_rect14_host_receipt(
    DM2_V1_BootProfile *profile,
    DM2_V1_InterfaceRect14HostReceipt *out_receipt);

/* skproject DM2_LOAD_GDAT_INTERFACE_00_0A receipt. */
int dm2_v1_boot_load_gdat_interface_00_0a_receipt(
    DM2_V1_BootProfile *profile,
    DM2_V1_LoadGdatInterface000AReceipt *out_receipt);

/* Viewport asset provider backed by profile->graphics_dat.
 * Pass the DM2_V1_BootProfile as the user pointer. */
int dm2_v1_boot_viewport_asset_fetch(void *user,
                                     int gdat_index,
                                     const uint8_t **out_pixels,
                                     int *out_w,
                                     int *out_h,
                                     int *out_stride);
int dm2_v1_boot_viewport_asset_palette_fetch(void *user,
                                             int gdat_index,
                                             uint8_t out_palette16[16],
                                             uint32_t *out_hash);

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
 * owns the GDAT address and M11 only executes the resulting blit. For the
 * TITLE/0 field-4 menu surface this preserves SHOW_MENU_SCREEN's dt07/4
 * 320x200 raw-screen priority over the decoded image fallback. */
int dm2_v1_boot_gdat_image_asset_fetch(
    DM2_V1_BootProfile *profile,
    int category,
    int index,
    int field,
    uint8_t **out_pixels,
    int *out_w,
    int *out_h,
    int *out_stride);

/* Prove the exact bounded raw GDAT payload selected for an M11 blit. */
int dm2_v1_boot_gdat_raw_asset_proof(
    DM2_V1_BootProfile *profile,
    int category,
    int index,
    int field,
    uint32_t seed,
    uint32_t *out_hash,
    uint32_t *out_byte_count);

/* Resolves SKProject QUERY_ORNATE_ANIM_FRAME from boot-owned GDAT and turns
 * its frame into the WALL_GFX dtImage field used by the live viewport. */
int dm2_v1_boot_wall_gfx_ornate_animation_field(
    DM2_V1_BootProfile *profile, uint8_t wall_gfx_index, uint32_t tick,
    uint32_t delta, uint8_t *out_field, uint32_t *out_receipt_hash);

int dm2_v1_boot_gdat_typed_raw_asset_proof(
    DM2_V1_BootProfile *profile,
    int category,
    int index,
    int type,
    int field,
    uint32_t seed,
    uint32_t *out_hash,
    uint32_t *out_byte_count);

/* Expose the boot-owned GDAT loader for source-locked viewport placement
 * tables. The loader lifetime is tied to the boot profile; callers must not
 * retain it past profile destruction. */
const DM2_V1_AssetLoader *dm2_v1_boot_asset_loader(
    const DM2_V1_BootProfile *profile);

int dm2_v1_boot_graphicsset_scene_control(
    DM2_V1_BootProfile *profile,
    int graphicsset_index,
    uint32_t *out_hash,
    uint32_t *out_present_mask,
    uint32_t *out_query_count,
    uint32_t *out_scene_flags,
    uint32_t *out_scene_colorkey,
    uint32_t *out_ambient_light,
    uint32_t *out_highest_light_level,
    uint32_t *out_void_random_fall,
    uint32_t *out_animated_floor,
    uint32_t *out_scene_rain,
    uint32_t *out_misty_map,
    uint32_t *out_thunder_position,
    uint32_t *out_ambient_darkness);

/* Loads the complete live GRAPHICSSET scene/light command family directly
 * from boot-owned canonical GDAT. Missing source pixels, palettes, or control
 * words fail closed before a dungeon frame is admitted. */
int dm2_v1_boot_gdat_scene_m11_command_plan(
    DM2_V1_BootProfile *profile,
    int graphicsset_index,
    DM2_V1_GdatSceneM11CommandPlan *out_plan);
int dm2_v1_boot_gdat_wall_m11_command_plan(
    DM2_V1_BootProfile *profile, int graphicsset_index,
    DM2_V1_GdatWallM11CommandPlan *out_plan);
int dm2_v1_boot_gdat_door_overlay_m11_command_plan(
    DM2_V1_BootProfile *profile, const DM2_V1_DoorRenderPlan *door_plan,
    DM2_V1_GdatDoorOverlayM11CommandPlan *out_plan);
/* Applies SKProject QUERY_TEMP_PICST/_32cb_0804's original dt07/2 palette
 * remap to nonzero DRAW_DOOR light-palette retries. A missing c_light receipt
 * or source table rejects the complete transaction rather than using base
 * IMG3 colours. */
int dm2_v1_boot_gdat_door_overlay_apply_light_palette(
    DM2_V1_BootProfile *profile,
    uint8_t c_light_parameter,
    uint32_t c_light_receipt_hash,
    DM2_V1_GdatDoorOverlayM11CommandPlan *plan);
/* Applies SKProject QUERY_TEMP_PICST/_32cb_0804 to GRAPHICSSET floor/ceiling
 * IMG3s: its optional exact dt07/0 or dt07/1 stationary lookup (dt07/9 or
 * dt07/10 while moving), followed by the original _0b36_037e light remap. */
int dm2_v1_boot_gdat_scene_m11_apply_light_palette(
    DM2_V1_BootProfile *profile,
    int movement_active,
    uint8_t c_light_parameter,
    uint32_t c_light_receipt_hash,
    DM2_V1_GdatSceneM11CommandPlan *plan);

int dm2_v1_boot_gdat_hud_m11_command_plan(
    DM2_V1_BootProfile *profile,
    const DM2_V1_HudPartyState *party,
    DM2_V1_GdatHudM11CommandPlan *out_plan);
/* Source-owned static HUD chrome for a boot that has not yet received a
 * GAME_LOAD/new-game Champion::HeroType handoff. When is_outdoor is non-zero
 * the right-side portrait panel is omitted because DM2 outdoor viewports do
 * not draw it. */
int dm2_v1_boot_gdat_hud_static_m11_command_plan(
    DM2_V1_BootProfile *profile,
    int is_outdoor,
    DM2_V1_GdatHudM11CommandPlan *out_plan);

int dm2_v1_boot_hud_core_asset_address(int field,
                                       int *out_category,
                                       int *out_index,
                                       int *out_field);

/* c_weather.cpp resolves ENVIRONMENT command text and the matching IMG3 by
 * the live MapGraphicsStyle. The returned receipt remains boot-owned evidence;
 * callers must not infer a viewport destination from it. */
int dm2_v1_boot_weather_gdat_receipt(
    DM2_V1_BootProfile *profile,
    int graphicsset_index,
    DM2_V1_WeatherGdatReceipt *out_receipt);

/* Source-only weather destination evidence. c_bkgrnd.cpp passes CD into
 * QUERY_TEMP_PICST, which resolves the original INTERFACE_GENERAL dt04
 * rectangle route before DRAW_TEMP_PICST. This receipt does not draw. */
typedef struct {
    int valid;
    uint8_t graphicsset;
    uint32_t destination_mask;
    uint32_t rect_table_hash;
    uint32_t receipt_hash;
    DM2_V1_WeatherDestinationClip clips[6];
} DM2_V1_BootWeatherDestinationReceipt;

int dm2_v1_boot_weather_gdat_destination_receipt(
    DM2_V1_BootProfile *profile,
    int graphicsset_index,
    DM2_V1_BootWeatherDestinationReceipt *out_receipt);

/* Joins source-owned live DistantEnvironment slots to the verified GDAT
 * material and original dt04 destination route. It cannot select commands
 * from generic weather state and returns no drawable receipt without slots. */
int dm2_v1_boot_weather_renderer_receipt(
    DM2_V1_BootProfile *profile,
    int graphicsset_index,
    const DM2_V1_WeatherRestoredStateReceipt *restored_state,
    const DM2_V1_DistantEnvironmentReceipt *slots,
    unsigned int slot_count,
    const DM2_V1_WeatherDrawContext *context,
    DM2_V1_WeatherRendererReceipt *out_receipt);

/* c_gui_vp.cpp dialogue shell/glyph evidence only. The receipt does not
 * authorize text layout or a DRAW_PICST call. */
int dm2_v1_boot_dialogue_gdat_receipt(
    DM2_V1_BootProfile *profile,
    int graphicsset_index,
    uint8_t shell_field,
    DM2_V1_DialogueGdatReceipt *out_receipt);

/* Raw-byte and decoded-pixel evidence for one virtual viewport resource.
 * The virtual index is the one used by DM2_V1_ViewportAssetFetch. */
typedef struct {
    int gdat_index;
    int category;
    int entry_index;
    int field;
    uint32_t raw_hash;
    uint32_t raw_byte_count;
    int decoded_w;
    int decoded_h;
    int decoded_stride;
    uint32_t decoded_hash;
    uint32_t decoded_pixel_count;
} DM2_V1_BootViewportAssetEvidence;

int dm2_v1_boot_viewport_asset_evidence(
    DM2_V1_BootProfile *profile,
    int gdat_index,
    DM2_V1_BootViewportAssetEvidence *out_evidence);

/* skproject QUERY_CREATURE_PICST resolves a live non-static creature through
 * GET_CREATURE_ANIMATION_FRAME's FB/FC/FD table chain, then draws the exact
 * CREATURES/type/dtImage field selected by FD.  This receipt deliberately
 * does not fall back to the DB4 map-chip F9 route. */
typedef struct {
    int valid;
    int creature_type;
    uint16_t command;
    uint16_t previous_frame;
    uint16_t selected_frame;
    uint16_t sequence_offset;
    uint8_t direction;
    uint8_t image_field;
    uint32_t animation_table_hash;
    uint32_t material_hash;
    uint32_t palette_hash;
    uint16_t raw_material_index;
    const uint8_t *raw_material_bytes;
    uint32_t raw_material_byte_count;
    uint32_t raw_material_hash;
    uint32_t raw_material_receipt_hash;
    DM2_V1_BootViewportAssetEvidence image;
} DM2_V1_BootDynamicCreatureMaterialReceipt;

int dm2_v1_boot_dynamic_creature_material_receipt(
    DM2_V1_BootProfile *profile,
    int creature_type,
    uint16_t command,
    uint16_t previous_frame,
    int direction,
    DM2_V1_BootDynamicCreatureMaterialReceipt *out_receipt);

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

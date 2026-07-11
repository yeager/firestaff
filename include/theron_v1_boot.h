#ifndef THERON_V1_BOOT_H
#define THERON_V1_BOOT_H

#include <stdint.h>
#include <stddef.h>
#include "asset_status_m12.h"
#include "theron_v1_startup_media.h"
#include "theron_v1_startup_save_resume.h"
#include "theron_v1_startup_flow.h"
#include "theron/theron_v1_asset_loader.h"
#include "theron_v1_viewport.h"
#include "theron_v1_world.h"

/* ══════════════════════════════════════════════════════════════════════
 * Theron's Quest V1 Boot Profile — Phase 1: Runtime Profile Split
 *
 * Separates Theron's Quest boot/runtime from DM1/CSB/DM2/Nexus:
 *   - Asset discovery (PC Engine HuCard data layout)
 *   - Menu launch routing (M12 game-card → boot profile)
 *   - Save namespace: saves/theron/ (between-dungeon only)
 *   - Platform diagnostics (PC Engine HuCard JP/US)
 *   - Deterministic config (PC Engine fixed-tick, no chivalry)
 *
 * Platform reference:
 *   PC Engine / TurboGrafx-CD — Hudson Soft / NEC, 1992 (JP) / 1993 (US)
 *   CPU: 7.16 MHz HuC6280 (65C02 derivative)
 *   Resolution: 256x224 (NTSC) / 320x224 (x1/x2 pixel modes)
 *   Palette: 512 colors, 16-color per-sprite tiles
 *   Data storage: CD-ROM Track 02 data BIN — game code, graphics, dungeon data
 *
 * Provenance (Phase 0 — PASSED, docs/source-lock/tqr_v1_phase0_provenance_gate_H2339.md):
 *   JP MD5: b7afb338ad31be1025b53f9aff12d73a (Track 02 BIN, cdromance.org)
 *   US MD5: f23601102138f87c33025877767ebf76 (Track 02 BIN, cdromance.org)
 *   JP Rev 1 ISO MD5: 397039af02d50d15c70b74088eb8a1cb (Track 02 ISO)
 *   US ISO MD5:       3d8b78571dcd0e6eb8eb4b01eeb7fbba (Track 02 ISO)
 *   g_theronVersions[] version slots (pce-jp, pce-en) are wired in
 *   asset_status_m12.c with Track 02 MD5s.
 *
 * Save namespace design (distinct from DM1/CSB/DM2):
 *   Theron's Quest was a "light" version — smaller dungeon set,
 *   4-character party (Theron + 3 champions), 7 mini-dungeons.
 *   No in-dungeon save. Between-dungeon saves only (saves/theron/).
 *   This separates TQ save timing semantics from full DM games.
 *
 * Source references:
 *   THQUEST.ASM T000  — Theron's Quest title/startup entry
 *   THQUEST.ASM T200  — PC Engine platform detection / version label
 *   THQUEST.ASM T400  — Dungeon bank loading (HuCard ROM mapping)
 *   THQUEST.ASM T520  — Party placement and start position
 *   THQUEST.ASM T560  — Dungeon loading (header parsing, dungeon_seed)
 *   THQUEST.ASM T800  — Champion persistence between dungeons
 *   THQUEST.ASM T080  — Save/load (between-dungeon only, no in-dungeon saves)
 * ══════════════════════════════════════════════════════════════════════ */

/* Theron's Quest supported platform/region IDs.
 * Two confirmed releases:
 *   JP —released 1992-09-18 (Hudson Soft, PC Engine DUO compatible)
 *   US — released 1993     (Hudson Soft USA, TurboGrafx-16) */
typedef enum {
    THERON_PLATFORM_PCE_JP,  /* PC Engine HuCard — Japanese 1st edition */
    THERON_PLATFORM_PCE_US,  /* TurboGrafx-16 — North American edition */
    THERON_PLATFORM_COUNT
} Theron_Platform;

/* Theron's Quest deterministic config.
 * Mirrors the PC Engine fixed-tick game loop (18.2 Hz VBlank,
 * matching DM1/CSB since the PC Engine was designed to replicate
 * that timing where possible).
 *
 * Key differences from DM1/CSB:
 *   - No chivalry system (simplified champion Need management)
 *   - Smaller party (4 vs 4 champions + party members)
 *   - 7 mini-dungeons rather than 15+ level dungeon
 *   - Quest structure: 7 dungeon goal sequence
 *   - No day/night outdoor cycle (all indoor on rails)
 */
typedef struct Theron_V1_BootProfile {
    /* Tick rate: PC Engine HuC6280 VBlank drives same ~18.2 Hz ticks.
     * Each tick ≈ 55ms. No interpolation in V1. */
    uint32_t tick_rate_hz;          /* default: 18 */
    uint32_t tick_rate_hz_frac;     /* default: 2 (18.2 Hz = 18 + 2/10) */
    uint32_t tick_ms;              /* ~55ms per tick */

    /* Movement: PC Engine version uses DM2-style fixed speeds.
     * Note: TQ has simpler movement — no diagonal strafing. */
    uint32_t dungeon_move_speed;    /* Q8: default 0x0080 (=0.5 sq/tick) */
    uint32_t outdoor_move_speed;    /* Q8: same as dungeon (no outdoor map) */

    /* Party: Theron + 3 champion slots (4 total). No minion slots.
     * Champions persist stats/skills between dungeons but lose
     * current inventory on dungeon restart (per design docs). */
    uint32_t max_champions;         /* 4 (1 Theron + 3 champions) */
    uint32_t max_party_members;     /* 4 */

    /* Quest: 7 mini-dungeons. No outdoor / day-night cycle.
     * Start is dungeon 1 in the Hall of Records. */
    uint32_t dungeon_count;         /* 7 mini-dungeons */
    uint32_t max_levels;           /* ~3 levels per mini-dungeon max */

    /* No time-of-day cycle in TQ (no outdoor exploration).
     * Reserved slot preserved for parity in config struct. */
    uint32_t reserved_day_cycle_minutes;
    uint32_t reserved_day_cycle_ticks;

    /* Deterministic RNG seed: from TQ dungeon header word at offset 8.
     * Mirrors DM2 structure so boot config builder reuses logic. */
    uint32_t dungeon_seed;          /* default fallback: 313 */

    /* TQ-specific: quest goal item (7 dungeon → 7 quest items).
     * quest_items_collected ranges 0..7 in saves/theron/namespace. */
    uint32_t quest_items_collected; /* persists across between-dungeon saves */

    uint32_t reserved[2];
} Theron_V1_DeterministicConfig;

/* Theron boot profile — collected at startup before game loop begins.
 * All fields are set once and read-only during gameplay.
 *
 * Save namespace: saves/theron/slotN.tqsv
 *   Format: between-dungeon only. In-dungeon saves mocked.
 *   (TQ design: you can save only at dungeon entrance, not mid-dungeon)
 */
typedef struct {
    /* ── Identity ────────────────────────────────────────── */
    char             game_id[8];       /* "theron" */
    Theron_Platform  platform;         /* detected platform */
    char             platform_label[32]; /* e.g. "PC Engine JP" */
    char             version_id[16];   /* e.g. "pce-jp", "pce-us" */

    /* ── Asset paths ─────────────────────────────────────── */
    char    asset_root[512];    /* ~/.firestaff/data/theron/ */
    char    graphics_path[512]; /* resolved: THQUEST.GFX or GRAPHICS.DAT */
    char    dungeon_path[512];  /* resolved: THQUEST.DUN or DUNGEON.DAT */
    int     assets_verified;   /* 1 when Track 02 matches a known JP/US MD5 */

    /* ── Save namespace (between-dungeon only) ─────────── */
    char    save_root[512];     /* saves/theron/ — NOT saves/dm1/ */
    int     in_dungeon_save_allowed; /* 0 — design restriction */

    /* ── Detected file sizes (diagnostic) ───────────────── */
    size_t  graphics_size;
    size_t  dungeon_size;
    char    graphics_md5[33];  /* empty until Phase 0 hash lock */
    char    dungeon_md5[33];  /* empty until Phase 0 hash lock */

    /* ── Deterministic config ────────────────────────────── */
    Theron_V1_DeterministicConfig deterministic;

    /* ── Runtime references (set after boot) ──────────────── */
    void   *theron_state;     /* Theron_V1_GameState* — set post boot */
    void   *dungeon_data;     /* Theron_V1_DungeonData* — parsed TQ data */
    void   *graphics_dat;    /* graphics data handle */
} Theron_V1_BootProfile;

/* ── Boot API ──────────────────────────────────────────────────────── */

/* Initialize a boot profile with Theron V1 defaults.
 * Does not touch filesystem — only sets struct fields. */
void theron_v1_boot_profile_init(Theron_V1_BootProfile *profile);

/* Scan for Theron's Quest assets in data_dir.
 * Phase 0 gate: assets_verified is set only for known Track 02 hashes.
 * Probes for Track 02 BIN names in theron/, theron/jp/, theron/us/.
 *
 * Returns 0 on success (assets found, even if unverified),
 * -1 if no Theron assets detected. */
int theron_v1_boot_scan_assets(Theron_V1_BootProfile *profile,
                               const char *data_dir);

/* Probe a data_dir for Theron assets without full verification.
 * Used by menu to check availability without triggering hash lock.
 * Returns: 1 if assets found, 0 if not. */
int theron_v1_boot_probe_available(const char *data_dir);

/* theron_v1_boot_load_verified_path — direct launch via a known
 * hash-verified Track 02 path.  Avoids the full data_dir fallback
 * search when an upstream catalog (M12 asset status scanner,
 * quick-resume, or explicit --data-dir <file>) has already confirmed
 * the path against a known MD5.
 *
 * Inputs:
 *   profile       - output, populated like theron_v1_boot_scan_assets
 *                   but skipping fallback search
 *   track02_path  - exact path to a Track 02 BIN/ISO file
 *   expected_md5  - 32-char hex MD5 the caller already matched
 *                   (one of the four locked-in TQ Track 02 MD5s)
 *
 * Sets profile->assets_verified = 1 without re-hashing the file, and
 * copies expected_md5 into profile->graphics_md5 / dungeon_md5.
 *
 * Filename heuristic (same as scan) sets platform / version_id.
 *
 * Stale-path guard: refuses (-1) if track02_path is empty, NULL, or
 * no longer exists as a regular file on disk.  This makes "reuse
 * safely" mean "the verified Track 02 is still where you said it
 * was" — a deleted or moved file is treated as bad input, not a
 * successful profile fill.  The check is one stat() per call (no MD5
 * re-hash), counted in theron_v1_boot_rescan_call_count() so tests
 * can prove direct launch stays out of the data-dir fallback walk.
 *
 * Returns 0 on success, -1 on any input or stat failure.
 *
 * Source: THQUEST.ASM T400 (Track 02 data-track loading) — when
 * the runtime already knows the verified blob, it must not re-walk
 * the data root and stat every candidate.
 */
int theron_v1_boot_load_verified_path(Theron_V1_BootProfile *profile,
                                       const char *track02_path,
                                       const char *expected_md5);

typedef enum {
    THERON_V1_BOOT_STARTUP_PREPARE_OK = 0,
    THERON_V1_BOOT_STARTUP_PREPARE_BAD_INPUT = -1,
    THERON_V1_BOOT_STARTUP_PREPARE_VERIFY_FAILED = -2,
    THERON_V1_BOOT_STARTUP_PREPARE_MISSING_TRACK02 = -3,
    THERON_V1_BOOT_STARTUP_PREPARE_ASSET_LOAD_FAILED = -4,
    THERON_V1_BOOT_STARTUP_PREPARE_STATE_FAILED = -5
} Theron_V1BootStartupPrepareResult;

typedef struct Theron_V1_BootStartupLaunch {
    Theron_V1_BootProfile *profile;
    Theron_V1_World *world;
    Theron_V1_Viewport *viewport;
    TrAssetBundle *assets;
    Theron_V1StartupSaveResume save_resume;
    int save_resume_ready;
    Theron_StartupFlow startup_flow;
    Theron_StartupStateReceipt initial_state_receipt;
    Theron_StartupStateReceipt save_resume_state_receipt;
    Theron_StartupMediaStateReceipt startup_media_state_receipt;
    Theron_StartupHostReceipt launch_host_receipt;
    Theron_V1BootStartupPrepareResult prepare_result;
} Theron_V1_BootStartupLaunch;

typedef struct Theron_V1_BootStartupRuntimeReceipt {
    Theron_V1_BootProfile *profile;
    Theron_V1_World *world;
    Theron_V1_Viewport *viewport;
    TrAssetBundle *assets;
    Theron_StartupStateReceipt initial_state_receipt;
    Theron_StartupStateReceipt save_resume_state_receipt;
    Theron_StartupMediaStateReceipt startup_media_state_receipt;
    Theron_StartupHostReceipt launch_host_receipt;
    char boot_asset_md5[33];
    char title[64];
    char source_id[32];
    char dungeon_path[512];
} Theron_V1_BootStartupRuntimeReceipt;

typedef struct Theron_V1_BootRuntimeStartupSnapshot {
    int startup_phase;
    int selected_dungeon;
    const void *boot_profile;
    const Theron_V1_World *world;
    const void *assets;
    int startup_cursor;
    int continue_focus;
    int resume_claim;
    int tqsv_slot;
    int srm_slot;
    int srm_import_status;
    const char *srm_root;
    int runtime_level_source;
    int runtime_track02_semantic_handoff;
    int runtime_fallback_visuals_blocked;
    int runtime_structured_route;
    int runtime_receipt_text_route;
    int all_dungeon_real_data_capture_ready;
    int all_dungeon_capture_count;
    unsigned int all_dungeon_capture_mask;
    int exact_level_semantics_ready;
    int exact_object_semantics_ready;
    unsigned int no_fallback_semantic_role_mask;
    int track02_state_predicates_consumed;
    int track02_bitmap_routes_complete;
    int track02_no_fallback_runtime_route_ready;
    int object_table_no_fallback_ready;
    unsigned int object_table_blocked_anchor_mask;
    int object_table_blocked_anchor_count;
    int nonstartup_level_no_fallback_ready;
    unsigned int nonstartup_level_blocked_anchor_mask;
    int nonstartup_level_blocked_anchor_count;
    unsigned int startup_level_blocked_anchor_mask;
    int startup_level_blocked_anchor_count;
    uint32_t object_table_route_hash;
    uint32_t level_route_hash;
    const char *startup_text_prompt;
    const char (*startup_roster_names)
        [THERON_TRACK02_STARTUP_ROSTER_NAME_CAPACITY];
    const char (*startup_roster_titles)
        [THERON_TRACK02_STARTUP_ROSTER_TITLE_CAPACITY];
    int startup_roster_name_count;
    int selected_mirrors_mask;
    int companion_count;
    const int *selected_mirror_order;
    int selected_mirror_order_count;
} Theron_V1_BootRuntimeStartupSnapshot;

enum {
    THERON_V1_BOOT_STARTUP_VIEW_MODEL_LAYOUT_CAP = 16,
    THERON_V1_BOOT_STARTUP_VIEW_MODEL_ROW_CAP = 16,
    THERON_V1_BOOT_STARTUP_VIEW_MODEL_TEXT_CAP = 32,
    THERON_V1_BOOT_STARTUP_VIEW_MODEL_ANIMATION_CAP = 32
};

typedef struct Theron_V1_BootStartupViewModel {
    Theron_StartupLayoutElement
        layout[THERON_V1_BOOT_STARTUP_VIEW_MODEL_LAYOUT_CAP];
    int layout_count;
    char rows[THERON_V1_BOOT_STARTUP_VIEW_MODEL_ROW_CAP]
        [THERON_STARTUP_RENDER_ROW_CAPACITY];
    int row_count;
    Theron_StartupRenderPlan render_plan;
    int render_plan_valid;
    char phase[THERON_V1_BOOT_STARTUP_VIEW_MODEL_TEXT_CAP];
    int startup_active;
    char animation[THERON_V1_BOOT_STARTUP_VIEW_MODEL_ANIMATION_CAP];
    int animation_active;
    int title_frame;
    int title_frame_max;
    int title_ready;
    int startup_phase;
    int selected_dungeon;
    const void *boot_profile;
    const Theron_V1_World *world;
    const void *assets;
    int startup_cursor;
    int selected_mirrors_mask;
    int companion_count;
    int selected_mirror_order[THERON_STARTUP_MAX_COMPANIONS];
    int selected_mirror_order_count;
    const char *srm_root;
    int runtime_level;
    int runtime_champion_count;
    int runtime_level_source;
    int runtime_track02_semantic_handoff;
    int runtime_fallback_visuals_blocked;
    int runtime_structured_route;
    int runtime_receipt_text_route;
    int all_dungeon_real_data_capture_ready;
    int all_dungeon_capture_count;
    unsigned int all_dungeon_capture_mask;
    int exact_level_semantics_ready;
    int exact_object_semantics_ready;
    unsigned int no_fallback_semantic_role_mask;
    int track02_state_predicates_consumed;
    int track02_bitmap_routes_complete;
    int track02_no_fallback_runtime_route_ready;
    int object_table_no_fallback_ready;
    unsigned int object_table_blocked_anchor_mask;
    int object_table_blocked_anchor_count;
    int nonstartup_level_no_fallback_ready;
    unsigned int nonstartup_level_blocked_anchor_mask;
    int nonstartup_level_blocked_anchor_count;
    unsigned int startup_level_blocked_anchor_mask;
    int startup_level_blocked_anchor_count;
    uint32_t object_table_route_hash;
    uint32_t level_route_hash;
    int continue_focus;
    int resume_claim;
    int tqsv_slot;
    int srm_slot;
    int srm_import_status;
    int startup_media_state_valid;
    Theron_StartupMediaStateReceipt startup_media_state_receipt;
} Theron_V1_BootStartupViewModel;

typedef struct Theron_V1_BootStartupRenderRouteReceipt {
    int startup_menu_render_allowed;
    int track02_title_menu_ready;
    int title_menu_runtime_handoff_ready;
    int save_resume_start_ready;
    int save_resume_runtime_handoff_ready;
    int save_resume_track02_no_fallback_ready;
    int save_resume_claim;
    int save_resume_tqsv_slot;
    int save_resume_srm_slot;
    int save_resume_srm_import_status;
    int runtime_readiness_ready;
    int runtime_level_render_allowed;
    int first_level_render_ready;
    int hud_ready;
    int hud_seed_gate;
    int no_fallback_visuals_enforced;
    int fallback_visuals_allowed;
    int render_plan_valid;
    Theron_StartupRenderPlan render_plan;
    int state_receipt_valid;
    Theron_StartupStateReceipt state_receipt;
    int runtime_level_source;
    int runtime_track02_semantic_handoff;
    int runtime_fallback_visuals_blocked;
    int runtime_structured_route;
    int runtime_receipt_text_route;
    int all_dungeon_real_data_capture_ready;
    int all_dungeon_capture_count;
    unsigned int all_dungeon_capture_mask;
    int exact_level_semantics_ready;
    int exact_object_semantics_ready;
    unsigned int no_fallback_semantic_role_mask;
    int track02_state_predicates_consumed;
    int track02_bitmap_routes_complete;
    int track02_no_fallback_runtime_route_ready;
    int object_table_no_fallback_ready;
    unsigned int object_table_blocked_anchor_mask;
    int object_table_blocked_anchor_count;
    int nonstartup_level_no_fallback_ready;
    unsigned int nonstartup_level_blocked_anchor_mask;
    int nonstartup_level_blocked_anchor_count;
    unsigned int startup_level_blocked_anchor_mask;
    int startup_level_blocked_anchor_count;
    uint32_t object_table_route_hash;
    uint32_t level_route_hash;
    const char *status_scope;
    const char *status;
} Theron_V1_BootStartupRenderRouteReceipt;

typedef struct Theron_V1_BootStartupHostViewReceipt {
    int host_consumes_view_model;
    int view_model_valid;
    Theron_V1_BootStartupViewModel view_model;
    int layout_count;
    int row_count;
    int render_plan_valid;
    int presentation_ready;
    char phase[THERON_V1_BOOT_STARTUP_VIEW_MODEL_TEXT_CAP];
    int startup_active;
    char animation[THERON_V1_BOOT_STARTUP_VIEW_MODEL_ANIMATION_CAP];
    int animation_active;
    int title_frame;
    int title_frame_max;
    int title_ready;
    int render_route_valid;
    Theron_V1_BootStartupRenderRouteReceipt render_route;
    int runtime_readiness_ready;
    int runtime_level_render_allowed;
    int title_menu_runtime_handoff_ready;
    int save_resume_runtime_handoff_ready;
    int save_resume_track02_no_fallback_ready;
    int no_fallback_visuals_enforced;
    int fallback_visuals_allowed;
    int runtime_graphics_handoff;
    int track02_runtime_graphics_handoff;
    int save_resume_runtime_graphics_handoff;
    int runtime_level_source;
    int runtime_track02_semantic_handoff;
    int runtime_fallback_visuals_blocked;
    int runtime_structured_route;
    int runtime_receipt_text_route;
    int all_dungeon_real_data_capture_ready;
    int all_dungeon_capture_count;
    unsigned int all_dungeon_capture_mask;
    int exact_level_semantics_ready;
    int exact_object_semantics_ready;
    unsigned int no_fallback_semantic_role_mask;
    int track02_state_predicates_consumed;
    int track02_bitmap_routes_complete;
    int track02_no_fallback_runtime_route_ready;
    int object_table_no_fallback_ready;
    unsigned int object_table_blocked_anchor_mask;
    int object_table_blocked_anchor_count;
    int nonstartup_level_no_fallback_ready;
    unsigned int nonstartup_level_blocked_anchor_mask;
    int nonstartup_level_blocked_anchor_count;
    unsigned int startup_level_blocked_anchor_mask;
    int startup_level_blocked_anchor_count;
    uint32_t object_table_route_hash;
    uint32_t level_route_hash;
    int hud_ready;
    int state_receipt_valid;
    Theron_StartupStateReceipt state_receipt;
    int track02_media_consumed;
    int raw_prompt_roster_required;
    int raw_session_rebuild_required;
    const char *status_scope;
    const char *status;
} Theron_V1_BootStartupHostViewReceipt;

typedef struct Theron_V1_BootStartupGraphicsRouteReceipt {
    int host_consumes_view_model;
    int render_route_valid;
    Theron_V1_BootStartupRenderRouteReceipt render_route;
    int graphics_plan_valid;
    int graphics_executed;
    int graphics_blocked;
    int track02_real_media_ready;
    int real_bitmap_startup_graphics_ready;
    int track02_atlas_startup_graphics_ready;
    int track02_atlas_startup_graphics_executed;
    unsigned int track02_atlas_graphics_route_mask;
    int track02_atlas_graphics_route_count;
    size_t track02_atlas_graphics_pixel_count;
    uint32_t track02_atlas_graphics_checksum;
    int track02_startup_graphics_executed;
    int track02_startup_graphic_receipt_valid;
    Theron_StartupRenderGraphicCommand track02_startup_graphic_receipt;
    unsigned int required_bitmap_route_mask;
    int required_bitmap_route_count;
    int required_bitmap_routes_ready;
    unsigned int bitmap_route_mask;
    int bitmap_route_count;
    unsigned int raw_bitmap_route_mask;
    int raw_bitmap_route_count;
    size_t raw_bitmap_atlas_tile_count;
    unsigned int iso_bitmap_route_mask;
    int iso_bitmap_route_count;
    size_t iso_bitmap_atlas_tile_count;
    int bitmap_package_route_ready;
    int title_bitmap_route_ready;
    int stage_bitmap_route_ready;
    int soul_room_bitmap_route_ready;
    int forcefield_bitmap_route_ready;
    int raw_graphics_plan_consumer_required;
    int no_fallback_startup_graphics_proof;
    int fallback_startup_graphics_executed;
    int startup_menu_render_allowed;
    int runtime_readiness_ready;
    int no_fallback_visuals_enforced;
    int fallback_visuals_allowed;
    int runtime_graphics_handoff;
    int track02_runtime_graphics_handoff;
    int save_resume_runtime_graphics_handoff;
    int runtime_level_source;
    int runtime_track02_semantic_handoff;
    int runtime_fallback_visuals_blocked;
    int runtime_structured_route;
    int runtime_receipt_text_route;
    int all_dungeon_real_data_capture_ready;
    int all_dungeon_capture_count;
    unsigned int all_dungeon_capture_mask;
    int exact_level_semantics_ready;
    int exact_object_semantics_ready;
    unsigned int no_fallback_semantic_role_mask;
    int track02_state_predicates_consumed;
    int track02_bitmap_routes_complete;
    int track02_no_fallback_runtime_route_ready;
    int object_table_no_fallback_ready;
    unsigned int object_table_blocked_anchor_mask;
    int object_table_blocked_anchor_count;
    int nonstartup_level_no_fallback_ready;
    unsigned int nonstartup_level_blocked_anchor_mask;
    int nonstartup_level_blocked_anchor_count;
    unsigned int startup_level_blocked_anchor_mask;
    int startup_level_blocked_anchor_count;
    uint32_t object_table_route_hash;
    uint32_t level_route_hash;
    const char *status_scope;
    const char *status;
} Theron_V1_BootStartupGraphicsRouteReceipt;

typedef struct Theron_V1_BootStartupFullStartReceipt {
    int host_consumes_view_model;
    int view_model_valid;
    Theron_V1_BootStartupViewModel view_model;
    int host_view_valid;
    Theron_V1_BootStartupHostViewReceipt host_view;
    int graphics_route_valid;
    Theron_V1_BootStartupGraphicsRouteReceipt graphics_route;
    int title_menu_ready;
    int stage_menu_ready;
    int soul_room_menu_ready;
    int save_resume_start_ready;
    int save_resume_runtime_handoff_ready;
    int forcefield_menu_ready;
    int forcefield_runtime_handoff_ready;
    int full_start_graphics_ready;
    int full_start_graphics_executed;
    int full_start_graphics_blocked;
    int track02_real_media_ready;
    int real_bitmap_startup_graphics_ready;
    int track02_atlas_startup_graphics_ready;
    int track02_atlas_startup_graphics_executed;
    unsigned int track02_atlas_graphics_route_mask;
    int track02_atlas_graphics_route_count;
    size_t track02_atlas_graphics_pixel_count;
    uint32_t track02_atlas_graphics_checksum;
    int track02_startup_graphics_executed;
    int track02_startup_graphic_receipt_valid;
    Theron_StartupRenderGraphicCommand track02_startup_graphic_receipt;
    unsigned int required_bitmap_route_mask;
    int required_bitmap_route_count;
    int required_bitmap_routes_ready;
    unsigned int bitmap_route_mask;
    int bitmap_route_count;
    unsigned int raw_bitmap_route_mask;
    int raw_bitmap_route_count;
    size_t raw_bitmap_atlas_tile_count;
    unsigned int iso_bitmap_route_mask;
    int iso_bitmap_route_count;
    size_t iso_bitmap_atlas_tile_count;
    int bitmap_package_route_ready;
    int title_bitmap_route_ready;
    int stage_bitmap_route_ready;
    int soul_room_bitmap_route_ready;
    int forcefield_bitmap_route_ready;
    int raw_graphics_plan_consumer_required;
    int no_fallback_startup_graphics_proof;
    int fallback_startup_graphics_executed;
    int no_fallback_visuals_enforced;
    int fallback_visuals_allowed;
    int runtime_readiness_ready;
    int runtime_level_render_allowed;
    int runtime_level;
    int runtime_champion_count;
    int runtime_level_source;
    int runtime_track02_semantic_handoff;
    int runtime_fallback_visuals_blocked;
    int runtime_structured_route;
    int runtime_receipt_text_route;
    int all_dungeon_real_data_capture_ready;
    int all_dungeon_capture_count;
    unsigned int all_dungeon_capture_mask;
    int exact_level_semantics_ready;
    int exact_object_semantics_ready;
    unsigned int no_fallback_semantic_role_mask;
    int track02_state_predicates_consumed;
    int track02_bitmap_routes_complete;
    int track02_no_fallback_runtime_route_ready;
    int object_table_no_fallback_ready;
    unsigned int object_table_blocked_anchor_mask;
    int object_table_blocked_anchor_count;
    int nonstartup_level_no_fallback_ready;
    unsigned int nonstartup_level_blocked_anchor_mask;
    int nonstartup_level_blocked_anchor_count;
    unsigned int startup_level_blocked_anchor_mask;
    int startup_level_blocked_anchor_count;
    uint32_t object_table_route_hash;
    uint32_t level_route_hash;
    int hud_ready;
    int runtime_graphics_handoff;
    int track02_runtime_graphics_handoff;
    int save_resume_runtime_graphics_handoff;
    int raw_prompt_roster_required;
    int raw_session_rebuild_required;
    const char *status_scope;
    const char *status;
} Theron_V1_BootStartupFullStartReceipt;

typedef struct Theron_V1_BootStartupHostRenderReceipt {
    int host_consumes_full_start_receipt;
    int full_start_valid;
    int layout_count;
    Theron_StartupLayoutElement
        layout[THERON_V1_BOOT_STARTUP_VIEW_MODEL_LAYOUT_CAP];
    int row_count;
    char rows[THERON_V1_BOOT_STARTUP_VIEW_MODEL_ROW_CAP]
        [THERON_STARTUP_RENDER_ROW_CAPACITY];
    int render_plan_valid;
    Theron_StartupRenderPlan render_plan;
    int render_route_valid;
    Theron_V1_BootStartupRenderRouteReceipt render_route;
    int graphics_route_valid;
    int graphics_executor_consumed;
    int full_start_graphics_ready;
    int full_start_graphics_executed;
    int full_start_graphics_blocked;
    unsigned int bitmap_route_mask;
    int bitmap_route_count;
    unsigned int raw_bitmap_route_mask;
    int raw_bitmap_route_count;
    size_t raw_bitmap_atlas_tile_count;
    unsigned int iso_bitmap_route_mask;
    int iso_bitmap_route_count;
    size_t iso_bitmap_atlas_tile_count;
    int bitmap_package_route_ready;
    int title_bitmap_route_ready;
    int stage_bitmap_route_ready;
    int soul_room_bitmap_route_ready;
    int forcefield_bitmap_route_ready;
    int track02_real_media_ready;
    int real_bitmap_startup_graphics_ready;
    int track02_atlas_startup_graphics_ready;
    int track02_atlas_startup_graphics_executed;
    unsigned int track02_atlas_graphics_route_mask;
    int track02_atlas_graphics_route_count;
    size_t track02_atlas_graphics_pixel_count;
    uint32_t track02_atlas_graphics_checksum;
    int track02_startup_graphics_executed;
    int track02_startup_graphic_receipt_valid;
    Theron_StartupRenderGraphicCommand track02_startup_graphic_receipt;
    unsigned int required_bitmap_route_mask;
    int required_bitmap_route_count;
    int required_bitmap_routes_ready;
    int no_fallback_startup_graphics_proof;
    int no_fallback_visuals_enforced;
    int fallback_visuals_allowed;
    int raw_prompt_roster_required;
    int raw_session_rebuild_required;
    int raw_graphics_plan_consumer_required;
    const char *status_scope;
    const char *status;
} Theron_V1_BootStartupHostRenderReceipt;

typedef struct Theron_V1_BootStartupMenuRuntimeHandoffReceipt {
    int host_consumes_full_start_receipt;
    int full_start_valid;
    Theron_V1_BootStartupFullStartReceipt full_start;
    int host_render_valid;
    Theron_V1_BootStartupHostRenderReceipt host_render;
    int input_route_requested;
    int input_route_valid;
    Theron_StartupActionHostReceipt input_route;
    int pointer_route_requested;
    int pointer_route_valid;
    Theron_StartupActionHostReceipt pointer_route;
    int track02_media_consumed;
    unsigned int track02_media_route_mask;
    uint32_t track02_media_checksum;
    uint64_t track02_media_title_first_raw_offset;
    uint64_t track02_media_title_last_raw_offset;
    uint64_t track02_media_title_first_user_data_offset;
    uint64_t track02_media_title_last_user_data_offset;
    uint64_t track02_media_stage_first_raw_offset;
    uint64_t track02_media_stage_last_raw_offset;
    uint64_t track02_media_stage_first_user_data_offset;
    uint64_t track02_media_stage_last_user_data_offset;
    uint64_t track02_media_soul_room_first_raw_offset;
    uint64_t track02_media_soul_room_last_raw_offset;
    uint64_t track02_media_soul_room_first_user_data_offset;
    uint64_t track02_media_soul_room_last_user_data_offset;
    uint64_t track02_media_forcefield_first_raw_offset;
    uint64_t track02_media_forcefield_last_raw_offset;
    uint64_t track02_media_forcefield_first_user_data_offset;
    uint64_t track02_media_forcefield_last_user_data_offset;
    int startup_menu_render_allowed;
    int title_menu_ready;
    int stage_menu_ready;
    int soul_room_menu_ready;
    int runtime_handoff_ready;
    int track02_runtime_handoff_ready;
    int save_resume_runtime_handoff_ready;
    int real_graphics_handoff_ready;
    int bitmap_package_route_ready;
    unsigned int raw_bitmap_route_mask;
    unsigned int iso_bitmap_route_mask;
    int startup_graphics_executed;
    int startup_graphics_blocked;
    int no_fallback_visuals_enforced;
    int fallback_visuals_allowed;
    int fallback_startup_graphics_executed;
    int host_may_draw_fallback_visuals;
    int host_must_not_draw_fallback_visuals;
    int raw_prompt_roster_required;
    int raw_session_rebuild_required;
    int raw_graphics_plan_consumer_required;
    const char *status_scope;
    const char *status;
} Theron_V1_BootStartupMenuRuntimeHandoffReceipt;

typedef struct Theron_V1_BootStartupUiCallerReceipt {
    int ui_callers_ready;
    int host_render_valid;
    Theron_V1_BootStartupHostRenderReceipt host_render;
    int menu_runtime_handoff_valid;
    Theron_V1_BootStartupMenuRuntimeHandoffReceipt menu_runtime_handoff;
    int track02_media_consumed;
    unsigned int track02_media_route_mask;
    uint32_t track02_media_checksum;
    uint64_t track02_media_title_first_raw_offset;
    uint64_t track02_media_title_last_raw_offset;
    uint64_t track02_media_title_first_user_data_offset;
    uint64_t track02_media_title_last_user_data_offset;
    uint64_t track02_media_stage_first_raw_offset;
    uint64_t track02_media_stage_last_raw_offset;
    uint64_t track02_media_stage_first_user_data_offset;
    uint64_t track02_media_stage_last_user_data_offset;
    uint64_t track02_media_soul_room_first_raw_offset;
    uint64_t track02_media_soul_room_last_raw_offset;
    uint64_t track02_media_soul_room_first_user_data_offset;
    uint64_t track02_media_soul_room_last_user_data_offset;
    uint64_t track02_media_forcefield_first_raw_offset;
    uint64_t track02_media_forcefield_last_raw_offset;
    uint64_t track02_media_forcefield_first_user_data_offset;
    uint64_t track02_media_forcefield_last_user_data_offset;
    int title_prompt_ready;
    int roster_ready;
    int title_menu_ready;
    int stage_menu_ready;
    int soul_room_menu_ready;
    int forcefield_menu_ready;
    int runtime_handoff_ready;
    int track02_runtime_handoff_ready;
    int save_resume_runtime_handoff_ready;
    int real_graphics_handoff_ready;
    int real_bitmap_decode_ready;
    int required_bitmap_routes_ready;
    int required_bitmap_route_mask;
    int required_bitmap_route_count;
    int bitmap_route_mask;
    int bitmap_route_count;
    int bitmap_package_route_ready;
    int raw_bitmap_route_mask;
    int raw_bitmap_route_count;
    int iso_bitmap_route_mask;
    int iso_bitmap_route_count;
    int title_bitmap_route_ready;
    int stage_bitmap_route_ready;
    int soul_room_bitmap_route_ready;
    int forcefield_bitmap_route_ready;
    int runtime_level;
    int runtime_level_source;
    int runtime_track02_semantic_handoff;
    int semantic_first_level_ready;
    int semantic_nonzero_level_ready;
    int semantic_level_coverage_mask;
    int all_dungeon_real_data_capture_ready;
    int all_dungeon_capture_count;
    unsigned int all_dungeon_capture_mask;
    int exact_level_semantics_ready;
    int exact_object_semantics_ready;
    int complete_runtime_support_ready;
    unsigned int no_fallback_semantic_role_mask;
    int track02_state_predicates_consumed;
    int track02_bitmap_routes_complete;
    int track02_no_fallback_runtime_route_ready;
    int object_table_no_fallback_ready;
    unsigned int object_table_blocked_anchor_mask;
    int object_table_blocked_anchor_count;
    int nonstartup_level_no_fallback_ready;
    unsigned int nonstartup_level_blocked_anchor_mask;
    int nonstartup_level_blocked_anchor_count;
    unsigned int startup_level_blocked_anchor_mask;
    int startup_level_blocked_anchor_count;
    uint32_t object_table_route_hash;
    uint32_t level_route_hash;
    int no_fallback_visuals_enforced;
    int fallback_visuals_allowed;
    int fallback_startup_graphics_executed;
    int host_must_not_draw_fallback_visuals;
    int raw_prompt_roster_required;
    int raw_session_rebuild_required;
    int raw_graphics_plan_consumer_required;
    const char *status_scope;
    const char *status;
} Theron_V1_BootStartupUiCallerReceipt;

int theron_v1_boot_prepare_startup_profile(
    Theron_V1_BootProfile *profile,
    const char *data_dir,
    const char *verified_path,
    const char *verified_md5,
    const char *save_path,
    TrAssetBundle *assets,
    Theron_V1StartupSaveResume *out_save_resume,
    int *out_save_resume_ready,
    Theron_V1BootStartupPrepareResult *out_result);
const char *theron_v1_boot_startup_prepare_result_name(
    Theron_V1BootStartupPrepareResult result);
int theron_v1_boot_startup_launch_alloc(
    const char *data_dir,
    const char *verified_path,
    const char *verified_md5,
    const char *save_path,
    Theron_V1_BootStartupLaunch *out_launch);
int theron_v1_boot_startup_launch_detach_runtime(
    Theron_V1_BootStartupLaunch *launch,
    Theron_V1_BootStartupRuntimeReceipt *out_receipt);
void theron_v1_boot_startup_launch_cleanup(
    Theron_V1_BootStartupLaunch *launch);
void theron_v1_boot_runtime_release(
    Theron_V1_BootProfile **profile,
    Theron_V1_World **world,
    Theron_V1_Viewport **viewport,
    TrAssetBundle **assets);
int theron_v1_boot_startup_session_facts_from_runtime_state(
    Theron_StartupSessionFacts *session,
    int startup_phase,
    int selected_dungeon,
    const void *boot_profile,
    const Theron_V1_World *world,
    const void *assets,
    int startup_cursor,
    int continue_focus,
    int resume_claim,
    int tqsv_slot,
    int srm_slot,
    int srm_import_status,
    const char *srm_root,
    const char *startup_text_prompt,
    const char startup_roster_names[][THERON_TRACK02_STARTUP_ROSTER_NAME_CAPACITY],
    const char startup_roster_titles[][THERON_TRACK02_STARTUP_ROSTER_TITLE_CAPACITY],
    int startup_roster_name_count,
    int selected_mirrors_mask,
    int companion_count,
    const int *selected_mirror_order,
    int selected_mirror_order_count);
int theron_v1_boot_startup_execute_input_from_runtime_state(
    Theron_StartupActionHostReceipt *out_receipt,
    int startup_phase,
    int selected_dungeon,
    const void *boot_profile,
    const Theron_V1_World *world,
    const void *assets,
    int startup_cursor,
    int continue_focus,
    int resume_claim,
    int tqsv_slot,
    int srm_slot,
    int srm_import_status,
    const char *srm_root,
    const char *startup_text_prompt,
    const char startup_roster_names[][THERON_TRACK02_STARTUP_ROSTER_NAME_CAPACITY],
    const char startup_roster_titles[][THERON_TRACK02_STARTUP_ROSTER_TITLE_CAPACITY],
    int startup_roster_name_count,
    int selected_mirrors_mask,
    int companion_count,
    const int *selected_mirror_order,
    int selected_mirror_order_count,
    int input);
int theron_v1_boot_startup_execute_input_from_runtime_state_with_media_receipt(
    Theron_StartupActionHostReceipt *out_receipt,
    const Theron_StartupMediaStateReceipt *startup_media_receipt,
    int startup_phase,
    int selected_dungeon,
    const void *boot_profile,
    const Theron_V1_World *world,
    const void *assets,
    int startup_cursor,
    int continue_focus,
    int resume_claim,
    int tqsv_slot,
    int srm_slot,
    int srm_import_status,
    const char *srm_root,
    int selected_mirrors_mask,
    int companion_count,
    const int *selected_mirror_order,
    int selected_mirror_order_count,
    int input);
int theron_v1_boot_startup_execute_input_from_snapshot(
    const Theron_V1_BootRuntimeStartupSnapshot *snapshot,
    int input,
    Theron_StartupActionHostReceipt *out_receipt);
int theron_v1_boot_startup_execute_input_from_snapshot_with_media_receipt(
    const Theron_V1_BootRuntimeStartupSnapshot *snapshot,
    const Theron_StartupMediaStateReceipt *startup_media_receipt,
    int input,
    Theron_StartupActionHostReceipt *out_receipt);
int theron_v1_boot_startup_execute_pointer_from_runtime_state(
    Theron_StartupActionHostReceipt *out_receipt,
    int startup_phase,
    int selected_dungeon,
    const void *boot_profile,
    const Theron_V1_World *world,
    const void *assets,
    int startup_cursor,
    int continue_focus,
    int resume_claim,
    int tqsv_slot,
    int srm_slot,
    int srm_import_status,
    const char *srm_root,
    const char *startup_text_prompt,
    const char startup_roster_names[][THERON_TRACK02_STARTUP_ROSTER_NAME_CAPACITY],
    const char startup_roster_titles[][THERON_TRACK02_STARTUP_ROSTER_TITLE_CAPACITY],
    int startup_roster_name_count,
    int selected_mirrors_mask,
    int companion_count,
    const int *selected_mirror_order,
    int selected_mirror_order_count,
    int x,
    int y);
int theron_v1_boot_startup_execute_pointer_from_runtime_state_with_media_receipt(
    Theron_StartupActionHostReceipt *out_receipt,
    const Theron_StartupMediaStateReceipt *startup_media_receipt,
    int startup_phase,
    int selected_dungeon,
    const void *boot_profile,
    const Theron_V1_World *world,
    const void *assets,
    int startup_cursor,
    int continue_focus,
    int resume_claim,
    int tqsv_slot,
    int srm_slot,
    int srm_import_status,
    const char *srm_root,
    int selected_mirrors_mask,
    int companion_count,
    const int *selected_mirror_order,
    int selected_mirror_order_count,
    int x,
    int y);
int theron_v1_boot_startup_execute_pointer_from_snapshot(
    const Theron_V1_BootRuntimeStartupSnapshot *snapshot,
    int x,
    int y,
    Theron_StartupActionHostReceipt *out_receipt);
int theron_v1_boot_startup_execute_pointer_from_snapshot_with_media_receipt(
    const Theron_V1_BootRuntimeStartupSnapshot *snapshot,
    const Theron_StartupMediaStateReceipt *startup_media_receipt,
    int x,
    int y,
    Theron_StartupActionHostReceipt *out_receipt);
void theron_v1_boot_startup_view_model_clear(
    Theron_V1_BootStartupViewModel *view_model);
int theron_v1_boot_startup_view_model_from_snapshot(
    const Theron_V1_BootRuntimeStartupSnapshot *snapshot,
    Theron_V1_BootStartupViewModel *out_view_model);
int theron_v1_boot_startup_view_model_from_snapshot_with_media_receipt(
    const Theron_V1_BootRuntimeStartupSnapshot *snapshot,
    const Theron_StartupMediaStateReceipt *startup_media_receipt,
    Theron_V1_BootStartupViewModel *out_view_model);
int theron_v1_boot_startup_view_model_from_runtime_state(
    Theron_V1_BootStartupViewModel *out_view_model,
    int startup_phase,
    int selected_dungeon,
    const void *boot_profile,
    const Theron_V1_World *world,
    const void *assets,
    int startup_cursor,
    int continue_focus,
    int resume_claim,
    int tqsv_slot,
    int srm_slot,
    int srm_import_status,
    const char *srm_root,
    const char *startup_text_prompt,
    const char startup_roster_names[][THERON_TRACK02_STARTUP_ROSTER_NAME_CAPACITY],
    const char startup_roster_titles[][THERON_TRACK02_STARTUP_ROSTER_TITLE_CAPACITY],
    int startup_roster_name_count,
    int selected_mirrors_mask,
    int companion_count,
    const int *selected_mirror_order,
    int selected_mirror_order_count);
int theron_v1_boot_startup_view_model_from_runtime_state_with_media_receipt(
    Theron_V1_BootStartupViewModel *out_view_model,
    const Theron_StartupMediaStateReceipt *startup_media_receipt,
    int startup_phase,
    int selected_dungeon,
    const void *boot_profile,
    const Theron_V1_World *world,
    const void *assets,
    int startup_cursor,
    int continue_focus,
    int resume_claim,
    int tqsv_slot,
    int srm_slot,
    int srm_import_status,
    const char *srm_root,
    const char *startup_text_prompt,
    const char startup_roster_names[][THERON_TRACK02_STARTUP_ROSTER_NAME_CAPACITY],
    const char startup_roster_titles[][THERON_TRACK02_STARTUP_ROSTER_TITLE_CAPACITY],
    int startup_roster_name_count,
    int selected_mirrors_mask,
    int companion_count,
    const int *selected_mirror_order,
    int selected_mirror_order_count);
int theron_v1_boot_startup_layout_build_from_view_model(
    const Theron_V1_BootStartupViewModel *view_model,
    Theron_StartupLayoutElement *elements,
    int max_elements);
int theron_v1_boot_startup_render_rows_from_view_model(
    const Theron_V1_BootStartupViewModel *view_model,
    char rows[][THERON_STARTUP_RENDER_ROW_CAPACITY],
    int max_rows);
int theron_v1_boot_startup_render_plan_from_view_model(
    const Theron_V1_BootStartupViewModel *view_model,
    Theron_StartupRenderPlan *out_plan);
void theron_v1_boot_startup_render_route_receipt_init(
    Theron_V1_BootStartupRenderRouteReceipt *receipt);
int theron_v1_boot_startup_render_route_receipt_from_view_model(
    const Theron_V1_BootStartupViewModel *view_model,
    Theron_V1_BootStartupRenderRouteReceipt *out_receipt);
int theron_v1_boot_startup_render_route_receipt_from_snapshot_with_media_receipt(
    const Theron_V1_BootRuntimeStartupSnapshot *snapshot,
    const Theron_StartupMediaStateReceipt *startup_media_receipt,
    Theron_V1_BootStartupRenderRouteReceipt *out_receipt);
void theron_v1_boot_startup_host_view_receipt_init(
    Theron_V1_BootStartupHostViewReceipt *receipt);
int theron_v1_boot_startup_host_view_receipt_from_view_model(
    const Theron_V1_BootStartupViewModel *view_model,
    Theron_V1_BootStartupHostViewReceipt *out_receipt);
int theron_v1_boot_startup_host_view_receipt_from_snapshot_with_media_receipt(
    const Theron_V1_BootRuntimeStartupSnapshot *snapshot,
    const Theron_StartupMediaStateReceipt *startup_media_receipt,
    Theron_V1_BootStartupHostViewReceipt *out_receipt);
int theron_v1_boot_startup_state_receipt_from_view_model(
    const Theron_V1_BootStartupViewModel *view_model,
    Theron_StartupStateReceipt *out_receipt);
int theron_v1_boot_startup_state_receipt_from_snapshot_with_media_receipt(
    const Theron_V1_BootRuntimeStartupSnapshot *snapshot,
    const Theron_StartupMediaStateReceipt *startup_media_receipt,
    Theron_StartupStateReceipt *out_receipt);
int theron_v1_boot_startup_execute_pointer_from_view_model(
    const Theron_V1_BootStartupViewModel *view_model,
    int x,
    int y,
    Theron_StartupAction *out_action,
    Theron_StartupInputReceipt *out_receipt);
int theron_v1_boot_startup_execute_input_from_view_model(
    const Theron_V1_BootStartupViewModel *view_model,
    Theron_StartupInput input,
    Theron_StartupAction *out_action,
    Theron_StartupInputReceipt *out_receipt);
int theron_v1_boot_startup_execute_action_from_view_model_with_host_receipt(
    const Theron_V1_BootStartupViewModel *view_model,
    const Theron_StartupAction *action,
    Theron_StartupActionHostReceipt *out_receipt);
int theron_v1_boot_startup_execute_input_from_view_model_with_host_receipt(
    const Theron_V1_BootStartupViewModel *view_model,
    Theron_StartupInput input,
    Theron_StartupActionHostReceipt *out_receipt);
int theron_v1_boot_startup_execute_pointer_from_view_model_with_host_receipt(
    const Theron_V1_BootStartupViewModel *view_model,
    int x,
    int y,
    Theron_StartupActionHostReceipt *out_receipt);
int theron_v1_boot_startup_execute_graphics_plan_from_view_model(
    const Theron_V1_BootStartupViewModel *view_model,
    const Theron_StartupGraphicExecutor *executor);
void theron_v1_boot_startup_graphics_route_receipt_init(
    Theron_V1_BootStartupGraphicsRouteReceipt *receipt);
int theron_v1_boot_startup_execute_graphics_plan_from_view_model_with_route_receipt(
    const Theron_V1_BootStartupViewModel *view_model,
    const Theron_StartupGraphicExecutor *executor,
    Theron_V1_BootStartupGraphicsRouteReceipt *out_receipt);
int theron_v1_boot_startup_execute_graphics_plan_from_snapshot_with_media_receipt(
    const Theron_V1_BootRuntimeStartupSnapshot *snapshot,
    const Theron_StartupMediaStateReceipt *startup_media_receipt,
    const Theron_StartupGraphicExecutor *executor,
    Theron_V1_BootStartupGraphicsRouteReceipt *out_receipt);
void theron_v1_boot_startup_full_start_receipt_init(
    Theron_V1_BootStartupFullStartReceipt *receipt);
int theron_v1_boot_startup_full_start_receipt_from_view_model(
    const Theron_V1_BootStartupViewModel *view_model,
    const Theron_StartupGraphicExecutor *executor,
    Theron_V1_BootStartupFullStartReceipt *out_receipt);
int theron_v1_boot_startup_full_start_receipt_from_snapshot_with_media_receipt(
    const Theron_V1_BootRuntimeStartupSnapshot *snapshot,
    const Theron_StartupMediaStateReceipt *startup_media_receipt,
    const Theron_StartupGraphicExecutor *executor,
    Theron_V1_BootStartupFullStartReceipt *out_receipt);
int theron_v1_boot_startup_full_start_receipt_from_runtime_state_with_media_receipt(
    Theron_V1_BootStartupFullStartReceipt *out_receipt,
    const Theron_StartupMediaStateReceipt *startup_media_receipt,
    const Theron_StartupGraphicExecutor *executor,
    int startup_phase,
    int selected_dungeon,
    const void *boot_profile,
    const Theron_V1_World *world,
    const void *assets,
    int startup_cursor,
    int continue_focus,
    int resume_claim,
    int tqsv_slot,
    int srm_slot,
    int srm_import_status,
    const char *srm_root,
    int selected_mirrors_mask,
    int companion_count,
    const int *selected_mirror_order,
    int selected_mirror_order_count);
int theron_v1_boot_startup_full_start_receipt_from_runtime_route_with_media_receipt(
    Theron_V1_BootStartupFullStartReceipt *out_receipt,
    const Theron_StartupMediaStateReceipt *startup_media_receipt,
    const Theron_StartupGraphicExecutor *executor,
    int startup_phase,
    int selected_dungeon,
    const void *boot_profile,
    const Theron_V1_World *world,
    const void *assets,
    int startup_cursor,
    int continue_focus,
    int resume_claim,
    int tqsv_slot,
    int srm_slot,
    int srm_import_status,
    const char *srm_root,
    int runtime_level_source,
    int runtime_track02_semantic_handoff,
    int runtime_fallback_visuals_blocked,
    int runtime_structured_route,
    int runtime_receipt_text_route,
    int selected_mirrors_mask,
    int companion_count,
    const int *selected_mirror_order,
    int selected_mirror_order_count);
int theron_v1_boot_startup_host_view_from_runtime_state_with_media_receipt(
    Theron_V1_BootStartupHostViewReceipt *out_receipt,
    const Theron_StartupMediaStateReceipt *startup_media_receipt,
    int startup_phase,
    int selected_dungeon,
    const void *boot_profile,
    const Theron_V1_World *world,
    const void *assets,
    int startup_cursor,
    int continue_focus,
    int resume_claim,
    int tqsv_slot,
    int srm_slot,
    int srm_import_status,
    const char *srm_root,
    int selected_mirrors_mask,
    int companion_count,
    const int *selected_mirror_order,
    int selected_mirror_order_count);
int theron_v1_boot_startup_execute_input_from_full_start_receipt(
    const Theron_V1_BootStartupFullStartReceipt *receipt,
    Theron_StartupInput input,
    Theron_StartupActionHostReceipt *out_receipt);
int theron_v1_boot_startup_execute_pointer_from_full_start_receipt(
    const Theron_V1_BootStartupFullStartReceipt *receipt,
    int x,
    int y,
    Theron_StartupActionHostReceipt *out_receipt);
int theron_v1_boot_startup_layout_build_from_full_start_receipt(
    const Theron_V1_BootStartupFullStartReceipt *receipt,
    Theron_StartupLayoutElement *elements,
    int max_elements);
int theron_v1_boot_startup_render_rows_from_full_start_receipt(
    const Theron_V1_BootStartupFullStartReceipt *receipt,
    char rows[][THERON_STARTUP_RENDER_ROW_CAPACITY],
    int max_rows);
int theron_v1_boot_startup_host_view_from_full_start_receipt(
    const Theron_V1_BootStartupFullStartReceipt *receipt,
    Theron_V1_BootStartupHostViewReceipt *out_receipt);
void theron_v1_boot_startup_host_render_receipt_init(
    Theron_V1_BootStartupHostRenderReceipt *receipt);
int theron_v1_boot_startup_host_render_receipt_from_full_start_receipt(
    const Theron_V1_BootStartupFullStartReceipt *receipt,
    Theron_V1_BootStartupHostRenderReceipt *out_receipt);
int theron_v1_boot_startup_host_render_receipt_from_snapshot_with_media_receipt_and_executor(
    Theron_V1_BootStartupHostRenderReceipt *out_receipt,
    const Theron_V1_BootRuntimeStartupSnapshot *snapshot,
    const Theron_StartupMediaStateReceipt *startup_media_receipt,
    const Theron_StartupGraphicExecutor *executor);
int theron_v1_boot_startup_host_render_receipt_from_runtime_state_with_media_receipt(
    Theron_V1_BootStartupHostRenderReceipt *out_receipt,
    const Theron_StartupMediaStateReceipt *startup_media_receipt,
    int startup_phase,
    int selected_dungeon,
    const void *boot_profile,
    const Theron_V1_World *world,
    const void *assets,
    int startup_cursor,
    int continue_focus,
    int resume_claim,
    int tqsv_slot,
    int srm_slot,
    int srm_import_status,
    const char *srm_root,
    int selected_mirrors_mask,
    int companion_count,
    const int *selected_mirror_order,
    int selected_mirror_order_count);
int theron_v1_boot_startup_host_render_receipt_from_runtime_state_with_media_receipt_and_executor(
    Theron_V1_BootStartupHostRenderReceipt *out_receipt,
    const Theron_StartupMediaStateReceipt *startup_media_receipt,
    const Theron_StartupGraphicExecutor *executor,
    int startup_phase,
    int selected_dungeon,
    const void *boot_profile,
    const Theron_V1_World *world,
    const void *assets,
    int startup_cursor,
    int continue_focus,
    int resume_claim,
    int tqsv_slot,
    int srm_slot,
    int srm_import_status,
    const char *srm_root,
    int selected_mirrors_mask,
    int companion_count,
    const int *selected_mirror_order,
    int selected_mirror_order_count);
void theron_v1_boot_startup_menu_runtime_handoff_receipt_init(
    Theron_V1_BootStartupMenuRuntimeHandoffReceipt *receipt);
int theron_v1_boot_startup_menu_runtime_handoff_from_full_start_receipt(
    const Theron_V1_BootStartupFullStartReceipt *receipt,
    int input_code,
    int pointer_x,
    int pointer_y,
    Theron_V1_BootStartupMenuRuntimeHandoffReceipt *out_receipt);
int theron_v1_boot_startup_menu_runtime_handoff_from_runtime_state_with_media_receipt(
    Theron_V1_BootStartupMenuRuntimeHandoffReceipt *out_receipt,
    const Theron_StartupMediaStateReceipt *startup_media_receipt,
    const Theron_StartupGraphicExecutor *executor,
    int startup_phase,
    int selected_dungeon,
    const void *boot_profile,
    const Theron_V1_World *world,
    const void *assets,
    int startup_cursor,
    int continue_focus,
    int resume_claim,
    int tqsv_slot,
    int srm_slot,
    int srm_import_status,
    const char *srm_root,
    int selected_mirrors_mask,
    int companion_count,
    const int *selected_mirror_order,
    int selected_mirror_order_count,
    int input_code,
    int pointer_x,
    int pointer_y);
void theron_v1_boot_startup_ui_caller_receipt_init(
    Theron_V1_BootStartupUiCallerReceipt *receipt);
int theron_v1_boot_startup_ui_caller_from_full_start_receipt(
    const Theron_V1_BootStartupFullStartReceipt *receipt,
    int input_code,
    int pointer_x,
    int pointer_y,
    Theron_V1_BootStartupUiCallerReceipt *out_receipt);
int theron_v1_boot_startup_ui_caller_from_snapshot_with_media_receipt(
    Theron_V1_BootStartupUiCallerReceipt *out_receipt,
    const Theron_V1_BootRuntimeStartupSnapshot *snapshot,
    const Theron_StartupMediaStateReceipt *startup_media_receipt,
    const Theron_StartupGraphicExecutor *executor,
    int input_code,
    int pointer_x,
    int pointer_y);
int theron_v1_boot_startup_ui_caller_from_runtime_route_with_media_receipt(
    Theron_V1_BootStartupUiCallerReceipt *out_receipt,
    const Theron_StartupMediaStateReceipt *startup_media_receipt,
    const Theron_StartupGraphicExecutor *executor,
    int startup_phase,
    int selected_dungeon,
    const void *boot_profile,
    const Theron_V1_World *world,
    const void *assets,
    int startup_cursor,
    int continue_focus,
    int resume_claim,
    int tqsv_slot,
    int srm_slot,
    int srm_import_status,
    const char *srm_root,
    int runtime_level_source,
    int runtime_track02_semantic_handoff,
    int runtime_fallback_visuals_blocked,
    int runtime_structured_route,
    int runtime_receipt_text_route,
    int selected_mirrors_mask,
    int companion_count,
    const int *selected_mirror_order,
    int selected_mirror_order_count,
    int input_code,
    int pointer_x,
    int pointer_y);
int theron_v1_boot_startup_presentation_receipt_from_runtime_state(
    char *out_phase,
    int out_phase_size,
    int *out_startup_active,
    char *out_animation,
    int out_animation_size,
    int *out_animation_active,
    int *out_title_frame,
    int *out_title_frame_max,
    int *out_title_ready,
    int startup_phase);
int theron_v1_boot_startup_presentation_receipt_from_snapshot(
    const Theron_V1_BootRuntimeStartupSnapshot *snapshot,
    char *out_phase,
    int out_phase_size,
    int *out_startup_active,
    char *out_animation,
    int out_animation_size,
    int *out_animation_active,
    int *out_title_frame,
    int *out_title_frame_max,
    int *out_title_ready);
int theron_v1_boot_startup_execute_graphics_plan(
    const Theron_StartupRenderPlan *plan,
    const Theron_StartupGraphicExecutor *executor);
int theron_v1_boot_startup_return_to_stage_select_after_exit_host_receipt(
    Theron_StartupActionHostReceipt *out_receipt,
    Theron_V1_World *world);
int theron_v1_boot_startup_return_to_stage_select_after_exit_profile_host_receipt(
    Theron_StartupActionHostReceipt *out_receipt,
    const void *boot_profile,
    Theron_V1_World *world);
int theron_v1_boot_runtime_tick_world(Theron_V1_World *world,
                                      int *out_x,
                                      int *out_y,
                                      int *out_dir,
                                      int *out_tick);
int theron_v1_boot_runtime_turn_party(Theron_V1_World *world,
                                      int turn,
                                      int *out_x,
                                      int *out_y,
                                      int *out_dir,
                                      int *out_tick);
int theron_v1_boot_runtime_move_party(Theron_V1_World *world,
                                      int direction,
                                      int restore_direction,
                                      int *out_x,
                                      int *out_y,
                                      int *out_dir,
                                      int *out_tick);
int theron_v1_boot_runtime_render_frame(Theron_V1_World *world,
                                        Theron_V1_Viewport *viewport,
                                        const TrAssetBundle *assets,
                                        int presentation_is_v2,
                                        int hud_launch_mode,
                                        unsigned char *framebuffer,
                                        int framebuffer_width,
                                        int framebuffer_height);

/* theron_v1_boot_verified_path_is_stale — decide whether a previously
 * verified Track 02 path/MD5 pair still matches the bytes on disk.
 * Used by the launcher to reject stale reuse entries (deleted /
 * moved / replaced file) before the runtime tries to load them.
 *
 * Returns:
 *   1  -- track02_path is missing, empty, no longer a regular file,
 *         or its current MD5 no longer equals expected_md5 (stale;
 *         do not trust the cached verified entry).
 *   0  -- path exists as a regular file and its on-disk MD5 matches.
 *
 * This helper exists separately from the direct-launch path so
 * tests can assert the contract in isolation, and so the M12 reuse
 * gate can mirror the same check before deciding to bypass a
 * re-scan.
 *
 * Note: re-hashing the file is the source of truth for "still the
 * same bytes".  A same-size-but-different-content swap is the
 * dangerous case the helper catches; mtime/size alone are not
 * enough. */
int theron_v1_boot_verified_path_is_stale(const char *track02_path,
                                           const char *expected_md5);

/* theron_v1_boot_rescan_call_count — number of stat() / fopen() probes
 * performed across the most recent scan-style call (probe / scan /
 * load_verified_path).  Useful for asserting in tests that a direct
 * launch path did not re-walk the data root.
 *
 * Returns the accumulated stat probe count for the boot module.
 * Thread-unsafe: simple process-local counter. */
unsigned long theron_v1_boot_rescan_call_count(void);

/* Reset the rescan call counter.  Tests call this before exercising a
 * path, then read the counter back to assert no extra root walk
 * happened. */
void theron_v1_boot_rescan_call_count_reset(void);

/* Set the save root directory.
 * If save_dir is NULL, uses default: <data_dir>/../saves/theron/ */
void theron_v1_boot_set_save_root(Theron_V1_BootProfile *profile,
                                   const char *save_dir);

/* Evaluate the bounded startup save/resume gate from a boot profile.
 * Reports .tqsv/.srm availability without auto-resuming or mutating
 * runtime state. */
int theron_v1_boot_startup_save_resume(
    const Theron_V1_BootProfile *profile,
    Theron_V1StartupSaveResume *out_snapshot);

/* Build deterministic config from detected dungeon header.
 * Reads dungeon_seed from header word at offset 8 (same layout as DM2).
 * Source: THQUEST.ASM T560 — dungeon header parsing */
void theron_v1_boot_build_deterministic_config(Theron_V1_BootProfile *profile,
                                               const uint8_t *dungeon_header,
                                               int dungeon_size);

/* Enter the game: initialize game state from boot profile.
 * Sets profile->theron_state and profile->dungeon_data.
 * Returns 0 on success. */
int theron_v1_boot_enter_game(Theron_V1_BootProfile *profile);

/* Free resources allocated during boot (not the profile itself). */
void theron_v1_boot_cleanup(Theron_V1_BootProfile *profile);

/* ── Diagnostics API ──────────────────────────────────────────────── */

/* Fill a diagnostic report with human-readable text.
 * Returns bytes written (capped at buf_size). */
size_t theron_v1_diagnostic_report(const Theron_V1_BootProfile *profile,
                                    char *buf, size_t buf_size);

/* Print one-line platform/version summary to stdout. */
void theron_v1_boot_print_summary(const Theron_V1_BootProfile *profile);

/* Source evidence citation string. */
const char *theron_v1_boot_source_evidence(void);

#endif /* THERON_V1_BOOT_H */

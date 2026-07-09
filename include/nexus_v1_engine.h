#ifndef NEXUS_V1_ENGINE_H
#define NEXUS_V1_ENGINE_H

/* ── Forward declaration for mechanics.h ─────────────────────────────
 * Define both struct tag and typedef alias so all translation units
 * that include this header (directly or transitively through
 * mechanics.h) can use Nexus_V1_Engine by name as either a typedef
 * or struct tag. Set a guard so mechanics headers skip their own
 * re-declaration. */
#ifndef NEXUS_ENGINE_FWD_FROM_HEADERS
#define NEXUS_ENGINE_FWD_FROM_HEADERS
struct Nexus_V1_Engine;
typedef struct Nexus_V1_Engine Nexus_V1_Engine;
#endif

/* ── Data source headers (order matters: low-level first) ──────────── */
#include "nexus_v1_iso_reader.h"
#include "nexus_v1_dungeon.h"
#include "nexus_v1_game.h"
#include "nexus_v1_dmdf_model.h"
#include "nexus_v1_saturn_font.h"
#include "nexus_v1_text.h"
#include "nexus_v1_champions.h"
#include "nexus_v1_creatures.h"
#include "nexus_v1_ui_surfaces.h"
#include "nexus_v1_bpk_archive.h"
#include "nexus_v1_script_vm.h"
#include "nexus_v1_sound.h"
#include <stdint.h>

/* ── Constants ─────────────────────────────────────────────────────── */
#define NEXUS_MAX_MODELS 64

/* ── Enumerations ──────────────────────────────────────────────────── */
typedef enum {
    NEXUS_SRC_NONE = 0,
    NEXUS_SRC_ISO,         /* Reading from .cue/.bin disc image */
    NEXUS_SRC_EXTRACTED    /* Reading from extracted files on disk */
} Nexus_DataSource;

typedef enum {
    NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_MISSING = 0,
    NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_READY_STORED = 1,
    NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_BLOCKED_PRS3 = 2,
    NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_BLOCKED_TRUNCATED = 3,
    NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_NO_SURFACES = 4,
    NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_INVALID = 5
} Nexus_V1_MenuBpkRendererHandoffStatus;

typedef struct {
    Nexus_V1_MenuBpkRendererHandoffStatus status;
    Nexus_V1_BpkRuntimeDecodeRoute decode_route;
    int attempted;
    int receipt_valid;
    int can_render_stored_surfaces;
    int blocks_real_menu_surface_render;
    int fallback_visuals_permitted;
    uint32_t archive_entries;
    uint32_t surface_entries;
    uint32_t ready_stored_surfaces;
    uint32_t blocked_prs3_surfaces;
    uint32_t blocked_truncated_surfaces;
    uint32_t prs3_stream_plans;
    uint32_t prs3_stream_plan_failures;
    uint32_t first_blocked_entry;
    uint32_t first_blocked_stream_offset;
    uint32_t first_blocked_stream_size;
    uint32_t first_blocked_expected_output_bytes;
} Nexus_V1_MenuBpkRendererHandoffReceipt;

/* ── Main engine struct ─────────────────────────────────────────────── */
struct Nexus_V1_Engine {
    /* Data source */
    Nexus_DataSource source;
    Nexus_ISOReader iso;
    char data_dir[512];

    /* Game state */
    Nexus_V1_GameState game;

    /* Current level */
    Nexus_V1_Level current_level;
    int level_loaded;

    /* 3D models (loaded on demand) */
    Nexus_V1_Model models[NEXUS_MAX_MODELS];
    int model_count;

    /* Font */
    Nexus_V1_Font font;
    int font_loaded;

    /* Champion pool — full champion roster + party management */
    Nexus_V1_ChampionPool champions;

    /* Startup UI surfaces. FACE.BIN carries the champion select portraits. */
    Nexus_UI_Manager ui;
    int ui_startup_surfaces_loaded;
    int ui_startup_surfaces_expected;
    int ui_startup_surfaces_fallback;
    int ui_faces_loaded;
    int ui_faces_expected;
    int ui_faces_fallback;
    int menu_bpk_decode_receipt_valid;
    int menu_bpk_decode_receipt_attempted;
    Nexus_V1_BpkRuntimeDecodeReceipt menu_bpk_decode_receipt;

    /* Per-level trigger/script runtime. SLEV*.BIN is real candidate data;
     * dispatch remains blocked until a source-locked parser exists. */
    Nexus_ScriptVM script_vm;
    Nexus_ScriptRuntimeReceipt script_runtime_receipt;

    /* Creature manager */
    Nexus_V1_CreatureManager creatures;

    /* Mechanics state — opaque pointer, allocated in nexus_v1_init().
     * Defined in nexus_v1_mechanics.c (source-locked DM1 game loop).
     * Source: DM1 CLIKMENU.C F0366, MOVESENS.C F0267. */
    struct Nexus_MechanicsState *mechanics;

    /* Audio */
    Nexus_SoundEngine audio;
    Nexus_SfxRuntimeReceipt sfx_runtime_receipt;
    int current_cd_track;
    int audio_enabled;

    /* Initialized flag */
    int initialized;
};

/* ── Engine API ─────────────────────────────────────────────────────── */

/* Initialize from data directory (auto-detects ISO vs extracted).
 * Returns 0 on success, -1 on no data found. */
int nexus_v1_init(Nexus_V1_Engine *engine, const char *data_dir);

/* Load a dungeon level (0-15). Calls nexus_v1_level_load().
 * Returns 0 on success, -1 on failure. */
int nexus_v1_load_level(Nexus_V1_Engine *engine, int level);

/* Load a 3D creature model by filename (e.g. "SCORPION.MNS").
 * Returns model index (>=0) on success, -1 on failure. */
int nexus_v1_load_model(Nexus_V1_Engine *engine, const char *name);

/* Read any file from the disc image or extracted directory.
 * Caller owns returned buffer; free with free(). */
uint8_t *nexus_v1_read_file(Nexus_V1_Engine *engine, const char *name, int *out_size);

/* Handle pending level transition (called after mechanics signals).
 * Source: DM1 CLIKMENU.C F0364. */
int nexus_v1_engine_level_change(Nexus_V1_Engine *engine, int *out_new_level);

/* Game tick — call each 55ms (18.2 Hz).
 * Source: DM1 CLIKMENU.C:269-323, F0366. */
void nexus_v1_tick(Nexus_V1_Engine *engine);

/* Shutdown and free all resources */
void nexus_v1_shutdown(Nexus_V1_Engine *engine);

int nexus_v1_startup_faces_loaded_count(const Nexus_V1_Engine *engine);
int nexus_v1_startup_faces_expected_count(const Nexus_V1_Engine *engine);
int nexus_v1_startup_faces_fallback_count(const Nexus_V1_Engine *engine);
int nexus_v1_startup_faces_ready(const Nexus_V1_Engine *engine);
int nexus_v1_startup_surfaces_loaded_count(const Nexus_V1_Engine *engine);
int nexus_v1_startup_surfaces_expected_count(const Nexus_V1_Engine *engine);
int nexus_v1_startup_surfaces_fallback_count(const Nexus_V1_Engine *engine);
int nexus_v1_startup_surfaces_ready(const Nexus_V1_Engine *engine);
int nexus_v1_menu_bpk_decode_receipt_ready(const Nexus_V1_Engine *engine);
int nexus_v1_menu_bpk_decode_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_BpkRuntimeDecodeReceipt *out_receipt);
int nexus_v1_menu_bpk_renderer_handoff_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_MenuBpkRendererHandoffReceipt *out_receipt);
const char *nexus_v1_menu_bpk_renderer_handoff_status_name(
    Nexus_V1_MenuBpkRendererHandoffStatus status);
int nexus_v1_current_level_dgn_renderer_handoff_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_DgnRendererHandoffReceipt *out_receipt);
int nexus_v1_current_level_script_runtime_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_ScriptRuntimeReceipt *out_receipt);
int nexus_v1_current_level_sfx_runtime_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_SfxRuntimeReceipt *out_receipt);

#endif /* NEXUS_V1_ENGINE_H */

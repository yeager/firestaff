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

/* A DGN view plan is not renderable until every referenced material has a
 * decoded DMDF/BPK surface. PRS3-only entries intentionally remain blocked:
 * no synthetic colour is substituted for an unverified Saturn surface. */
typedef struct {
    Nexus_V1_DgnRenderCommand commands[NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS];
    Nexus_V1_DgnRenderPlanReceipt receipt;
    int level;
    int party_x;
    int party_y;
    int party_dir;
    uint32_t generation;
    uint32_t geometry_generation;
    uint32_t rebuild_count;
    uint32_t cache_hit_count;
    uint32_t invalidation_count;
    int valid;
} Nexus_V1_DgnMaterialPlan;

/* A named material container may be observed and structurally parsed before
 * it is eligible for the DGN host.  The retail Track 1 listing currently
 * has MENU.BPK only; it does not establish FLOORS.BPK or WALLS.BPK hashes.
 * Consequently format recognition alone never promotes pixels to runtime. */
typedef struct {
    Nexus_V1_DgnMaterialCategory category;
    /* An exact FLOORS.BPK/WALLS.BPK source observation is not an identity
     * verification or an import approval. */
    int exact_name_observed;
    int source_present;
    int format_valid;
    int identity_verified;
    int host_route_permitted;
    int blocks_real_surface_render;
    int fallback_visuals_permitted;
    Nexus_V1_BpkArchiveInfo archive;
} Nexus_V1_DgnMaterialContainerReceipt;

/* Structure2's descriptor envelope and opaque payload live in the canonical
 * LEV00.DGN..LEV15.DGN Track 1 entries, not in MENU.BPK or an inferred
 * FLOORS/WALLS container. This receipt authenticates that source boundary
 * only. `materialization_bound` means the loaded level's bounded payload is
 * tied to its canonical source; it never means that payload bytes are
 * decoded or renderable. */
typedef struct {
    int level_index;
    char canonical_name[16];
    char canonical_md5[33];
    int exact_source_entry_observed;
    int hash_discovery_attempted;
    int canonical_hash_verified;
    int structure2_payload_envelope_valid;
    int materialization_bound;
    int payload_decoder_permitted;
    int fallback_visuals_permitted;
} Nexus_V1_DgnStructure2SourceReceipt;

/* Hash-bound ownership for level-local script and audio inputs. The receipt
 * establishes only the canonical Track 1 source; it never assigns opcode,
 * trigger, sample, or playback semantics to the bytes. */
typedef struct {
    char canonical_name[16];
    char canonical_md5[33];
    int exact_source_entry_observed;
    int hash_discovery_attempted;
    int canonical_hash_verified;
} Nexus_V1_LevelAuxSourceReceipt;

typedef struct {
    int level_index;
    Nexus_V1_LevelAuxSourceReceipt slev;
    Nexus_V1_LevelAuxSourceReceipt sal;
    Nexus_V1_LevelAuxSourceReceipt map;
    /* SDDRVS.TSK is the global Saturn sound-task image consumed with every
     * level bank.  Its identity is separate from per-level SAL/MAP bytes. */
    Nexus_V1_LevelAuxSourceReceipt sound_driver;
    int canonical_pair_bound;
    int fallback_visuals_permitted;
} Nexus_V1_LevelAuxRuntimeReceipt;

/* Canonical ownership for the two retail MNS banks consumed by Structure1B
 * material selectors.  A parseable file is not enough: each bank must be
 * tied to its known Track 1 identity before its pixels reach the viewport. */
typedef struct {
    Nexus_V1_LevelAuxSourceReceipt floor_mns;
    Nexus_V1_LevelAuxSourceReceipt wall_mns;
    int canonical_pair_bound;
    /* The two MNS files establish source identity only. Structure1B bytes
     * 3/4 are not a proved wall-selector grammar: real LEV00..15 values
     * exceed the 0..14 SN_WALL descriptor range. This stays false until a
     * Saturn executable/capture route proves the selector transform. */
    int structure1b_selector_binding_proven;
    int fallback_visuals_permitted;
} Nexus_V1_DgnStaticMaterialSourceReceipt;

/* Read-only real-media evidence for the fixed LEV00..LEV15 corpus.  It
 * counts only the already typed Structure1B material selectors and checks
 * them against the same banks used by the DGN viewport; it does not infer
 * meanings for any opaque DGN payload bytes. */
typedef struct {
    int attempted;
    int expected_level_count;
    int readable_level_count;
    int parsed_level_count;
    int geometry_ready_level_count;
    int structure1f_valid_level_count;
    int structure1f_typed_entry_count;
    int structure1g_present_level_count;
    int structure1g_valid_level_count;
    int structure1g_animated_texture_count;
    int structure1g_sequence_count;
    int structure1g_image_instruction_count;
    int structure1g_goto_instruction_count;
    int structure1g_structure2_image_instruction_bound_count;
    int structure1g_structure2_image_instruction_unbound_count;
    int structure1g_floor_animation_cell_count;
    int structure1g_floor_animation_bound_count;
    int structure2_valid_level_count;
    int structure2_texture_count;
    int structure1g_structure2_first_image_bound_count;
    int structure2_payload_envelope_valid_level_count;
    int structure2_opaque_payload_byte_count;
    int structure2_nonzero_descriptor_offset_count;
    int structure2_descriptor_offsets_in_opaque_payload_count;
    int structure2_descriptor_offsets_outside_opaque_payload_count;
    int structure2_descriptor_offset_unique_count;
    int structure2_descriptor_offset_reused_count;
    int structure2_local_payload_offset_pattern_level_count;
    int structure2_material_or_image_data_proven_level_count;
    int structure2_canonical_source_verified_level_count;
    int structure2_materialization_bound_level_count;
    Nexus_V1_DgnMaterialCategoryCoverageReceipt floor_coverage;
    Nexus_V1_DgnMaterialCategoryCoverageReceipt ceiling_coverage;
    Nexus_V1_DgnMaterialCategoryCoverageReceipt wall_coverage;
    Nexus_V1_DgnMaterialContainerReceipt floor_container;
    Nexus_V1_DgnMaterialContainerReceipt wall_container;
    Nexus_V1_DgnStaticMaterialSourceReceipt static_mns_sources;
    Nexus_V1_DgnStructure2SourceReceipt structure2_sources[16];
    int bpk_host_routes_complete;
    int static_mns_host_route_complete;
    int material_coverage_complete;
    int host_route_evidence_complete;
    int fallback_visuals_permitted;
} Nexus_V1_DgnMaterialCorpusReceipt;

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

    /* DGN material references resolve through these decoded DMDF banks. */
    Nexus_DMDFMaterialBank floor_materials;
    Nexus_DMDFMaterialBank wall_materials;
    /* Per-level Structure2 animation surfaces. These are decoded only from
     * bounded DGN descriptor spans; they never replace static MNS banks. */
    Nexus_DMDFMaterialBank animated_floor_materials;
    int animated_floor_material_route_valid;
    Nexus_V1_DgnMaterialContainerReceipt floor_bpk_container;
    Nexus_V1_DgnMaterialContainerReceipt wall_bpk_container;
    /* FLOORS/WALLS.BPK must cross the validated BPK host route before a
     * Structure1B material reference can reach the real DGN viewport. */
    int floor_bpk_host_route_attempted;
    int floor_bpk_host_route_valid;
    Nexus_V1_BpkMaterialHostRouteReceipt floor_bpk_host_route;
    int wall_bpk_host_route_attempted;
    int wall_bpk_host_route_valid;
    Nexus_V1_BpkMaterialHostRouteReceipt wall_bpk_host_route;
    /* Retail SN_FLOOR.MNS / SN_WALL.MNS TEXT sections are a separate,
     * direct-colour DMDF material route.  They are never inferred from
     * MENU.BPK or an unnamed archive. */
    int floor_mns_material_route_valid;
    int wall_mns_material_route_valid;
    Nexus_V1_DgnStaticMaterialSourceReceipt dgn_static_material_sources;
    Nexus_V1_DgnMaterialPlan dgn_material_plan;
    Nexus_V1_DgnMaterialCorpusReceipt dgn_material_corpus;
    Nexus_V1_DgnStructure2SourceReceipt current_level_structure2_source;

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
    int menu_bpk_upload_receipt_valid;
    Nexus_V1_BpkRuntimeUploadReceipt menu_bpk_upload_receipt;
    Nexus_V1_BpkRuntimeUploadRow
        menu_bpk_upload_rows[NEXUS_V1_BPK_UPLOAD_PLAN_MAX_ROWS];
    int menu_bpk_upload_row_count;

    /* Per-level trigger/script runtime. SLEV*.BIN is real candidate data;
     * dispatch remains blocked until a source-locked parser exists. */
    Nexus_ScriptVM script_vm;
    Nexus_ScriptRuntimeReceipt script_runtime_receipt;
    Nexus_V1_LevelAuxRuntimeReceipt level_aux_runtime_receipt;
    Nexus_V1_LevelAuxSourceReceipt sound_driver_source;

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

/* Inspects every canonical level without changing the currently loaded
 * level or promoting corpus coverage into a runtime launch gate. */
int nexus_v1_inspect_dgn_material_corpus(
    Nexus_V1_Engine *engine,
    Nexus_V1_DgnMaterialCorpusReceipt *out_receipt);

/* Read-only source identity receipt for a loaded LEVxx.DGN Structure2
 * payload. This is intentionally not a decoder or a material import route. */
int nexus_v1_current_level_structure2_source_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_DgnStructure2SourceReceipt *out_receipt);
int nexus_v1_current_level_aux_runtime_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_LevelAuxRuntimeReceipt *out_receipt);
int nexus_v1_dgn_static_material_source_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_DgnStaticMaterialSourceReceipt *out_receipt);

/* Return the DGN plan whose commands and material surfaces have been checked
 * together for this level and party pose. The returned pointer is owned by
 * `engine` and remains valid until the next prepare/invalidate/shutdown. */
const Nexus_V1_DgnMaterialPlan *nexus_v1_prepare_dgn_material_plan(
    Nexus_V1_Engine *engine, int party_x, int party_y, int party_dir);
void nexus_v1_invalidate_dgn_material_plan(Nexus_V1_Engine *engine);
/* Commit a party pose originating from champion start, save resume or live
 * movement. Any pose change invalidates the viewport material plan before it
 * can be reused against a different cell geometry. */
void nexus_v1_sync_dgn_runtime_pose(Nexus_V1_Engine *engine,
                                    int level, int party_x, int party_y,
                                    int party_dir);

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
int nexus_v1_menu_bpk_upload_plan_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_BpkRuntimeUploadReceipt *out_receipt);
int nexus_v1_menu_bpk_upload_plan_rows(
    const Nexus_V1_Engine *engine,
    Nexus_V1_BpkRuntimeUploadRow *out_rows,
    int max_rows);
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

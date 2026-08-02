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
typedef struct Nexus_V1_Prs3DgnPlacementAdapterReceipt Nexus_V1_Prs3DgnPlacementAdapterReceipt;
typedef struct Nexus_V1_Structure1FPlacementBindingReceipt
    Nexus_V1_Structure1FPlacementBindingReceipt;
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
#include "nexus_v1_projectiles.h"
#include "nexus_v1_ui_surfaces.h"
#include "nexus_v1_bpk_archive.h"
#include "nexus_v1_prs3_capture_trace_schema.h"
#include "nexus_v1_script_vm.h"
#include "nexus_v1_sound.h"
#include "nexus_v1_structure3_capture_manifest.h"
#include <stddef.h>
#include <stdint.h>
#include <stddef.h>
struct Nexus_V1_DgnMultiLevelCaptureAdjudicationReceipt;
struct Nexus_V1_SaturnSaveCaptureReceipt;

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
    NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_INVALID = 5,
    /* A parseable archive is not a retail menu route until it is bound to
     * the exact Track 1 MENU.BPK identity. */
    NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_BLOCKED_SOURCE = 6,
    NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_READY_DECODED = 7
} Nexus_V1_MenuBpkRendererHandoffStatus;

/* This is intentionally a startup diagnostic, not a decoder capability.
 * MENU.BPK is permitted to reach Saturn presentation only after each of these
 * independently evidenced steps is available.  The retail PRS3 route remains
 * blocked at AUTHENTIC_DECODER until an original-Saturn execution proves its
 * command grammar and output relation. */
typedef enum {
    NEXUS_V1_MENU_BPK_PRS3_PREREQUISITE_ARCHIVE_MISSING = 0,
    NEXUS_V1_MENU_BPK_PRS3_PREREQUISITE_SOURCE_UNVERIFIED = 1,
    NEXUS_V1_MENU_BPK_PRS3_PREREQUISITE_ARCHIVE_INVALID = 2,
    NEXUS_V1_MENU_BPK_PRS3_PREREQUISITE_FRAME_INCOMPLETE = 3,
    NEXUS_V1_MENU_BPK_PRS3_PREREQUISITE_AUTHENTIC_DECODER = 4,
    NEXUS_V1_MENU_BPK_PRS3_PREREQUISITE_SATURN_PRESENTATION = 5,
    NEXUS_V1_MENU_BPK_PRS3_PREREQUISITE_READY_STORED = 6
} Nexus_V1_MenuBpkPrs3PrerequisiteStatus;

/* Corpus-wide, hash-bound owner topology for one LEV. Structure1F rows are
 * joined through their decoded Structure1B owner and Structure1A model row to
 * the raw Structure3 model/face selectors. This establishes source ownership
 * only: model-entry, mesh-face, transform, texture, palette, and draw
 * semantics remain unproven. */
typedef struct {
    int valid;
    int level_index;
    int canonical_lev_source_bound;
    int structure1f_owner_row_count;
    int structure1a_resolved_row_count;
    int structure3_model_reference_count;
    int structure1f_face_selector_count;
    int structure3_model_face_selector_pair_count;
    int owner_model_selector_binding_complete;
    int owner_to_mesh_entry_mapping_proven;
    int no_draw_only;
    int fallback_visuals_permitted;
} Nexus_V1_DgnStructure1FOwnerModelSelectorCorpusReceipt;

typedef struct {
    Nexus_V1_MenuBpkRendererHandoffStatus status;
    Nexus_V1_MenuBpkPrs3PrerequisiteStatus prs3_prerequisite_status;
    Nexus_V1_BpkRuntimeDecodeRoute decode_route;
    int attempted;
    int receipt_valid;
    int canonical_source_hash_verified;
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
    int canonical_palette_trailer_bound;
    Nexus_V1_BpkPaletteTrailerReceipt palette_trailer;
} Nexus_V1_MenuBpkRendererHandoffReceipt;

/* A source-bound request for an original-Saturn observation of the canonical
 * MENU.BPK PALT record. PALT is only an opaque EOF record today: the target
 * asks a capture producer to establish or reject its relation to palette
 * reads and VDP1 state without treating it as decoded palette data. */
#define NEXUS_V1_MENU_BPK_PALT_CAPTURE_TARGET_MAGIC \
    "FIRESTAFF_NEXUS_MENU_BPK_PALT_CAPTURE_TARGET_V1"
typedef struct {
    int valid;
    char canonical_menu_bpk_name[16];
    char canonical_menu_bpk_md5[33];
    uint32_t palt_record_offset;
    uint32_t palt_record_bytes;
    uint32_t palt_entry_count;
    uint32_t palt_entry_bytes;
    uint64_t palt_entry_bytes_fnv1a64;
    int original_saturn_capture_required;
    int palt_memory_read_observation_required;
    int palette_state_observation_required;
    int vdp1_command_observation_required;
    int palt_palette_relation_proven;
    int decoder_promoted;
    int no_draw_only;
    int fallback_visuals_permitted;
} Nexus_V1_MenuBpkPaltCaptureTargetReceipt;

/* Admission for one externally produced original-Saturn observation of the
 * canonical PALT bytes. It proves source/capture identity only: matching
 * palette and VDP1 observations do not establish a colour format, a CLUT
 * relation, or a decoder. */
#define NEXUS_V1_MENU_BPK_PALT_TRACE_MAGIC \
    "FIRESTAFF_NEXUS_MENU_BPK_PALT_TRACE_V1"
typedef enum {
    NEXUS_V1_MENU_BPK_PALT_TRACE_MISSING = 0,
    NEXUS_V1_MENU_BPK_PALT_TRACE_BLOCKED_MALFORMED = 1,
    NEXUS_V1_MENU_BPK_PALT_TRACE_BLOCKED_TARGET_MISMATCH = 2,
    NEXUS_V1_MENU_BPK_PALT_TRACE_BLOCKED_OBSERVATIONS = 3,
    NEXUS_V1_MENU_BPK_PALT_TRACE_BLOCKED_PROVENANCE = 4,
    NEXUS_V1_MENU_BPK_PALT_TRACE_ADMITTED_OPAQUE = 5
} Nexus_V1_MenuBpkPaltTraceStatus;

typedef struct {
    Nexus_V1_MenuBpkPaltTraceStatus status;
    int capture_target_bound;
    int manifest_target_bound;
    int mednafen_debugger_provenance;
    int trace_sha256_present;
    int raw_trace_bytes_bound;
    uint64_t raw_trace_fnv1a64;
    size_t raw_trace_byte_count;
    int palt_memory_bytes_bound;
    uint64_t palt_memory_fnv1a64;
    size_t palt_memory_byte_count;
    int palette_state_bytes_bound;
    uint64_t palette_state_fnv1a64;
    size_t palette_state_byte_count;
    int vdp1_command_bytes_bound;
    uint64_t vdp1_command_fnv1a64;
    size_t vdp1_command_byte_count;
    int original_saturn_capture_verified;
    int opaque_trace_admitted;
    int palt_palette_relation_proven;
    int decoder_promoted;
    int no_draw_only;
    int fallback_visuals_permitted;
} Nexus_V1_MenuBpkPaltTraceAdmissionReceipt;

/* Canonical MENU.BPK PALT compared to the documented DGT2 BGR555 CLUT in
 * WARNING.BIN. Matching words prove only an indexed raw-word correlation;
 * they never associate PALT with a PRS3 entry or authorize palette use. */
typedef struct {
    int valid;
    int menu_bpk_source_hash_verified;
    int warning_source_hash_verified;
    Nexus_V1_BpkPaletteTrailerReceipt palt;
    int warning_dgt2_pp_bound;
    uint64_t warning_clut_fnv1a64;
    uint32_t matching_entry_count;
    uint32_t mismatched_entry_count;
    uint32_t bgr555_low15_matching_entry_count;
    uint32_t bgr555_low15_mismatched_entry_count;
    uint32_t high_bit_only_mismatch_count;
    uint32_t colour_word_mismatch_count;
    int indexed_word_alignment_proven;
    int bgr555_word_encoding_correlation_proven;
    int bgr555_low15_correlation_proven;
    int prs3_palette_association_proven;
    int palette_application_proven;
    int decoder_promoted;
    int no_draw_only;
    int fallback_visuals_permitted;
} Nexus_V1_MenuBpkPaltWarningPaletteCorrelationReceipt;

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
    Nexus_V1_DgnFaceMaterialReceipt structure3_face_material_source;
    int structure3_face_material_source_consumed;
    Nexus_V1_DgnStructure2FloorCommandSource
        structure2_floor_command_sources[NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS];
    Nexus_V1_DgnStructure2FloorCommandSourceReceipt
        structure2_floor_command_source_receipt;
    int structure2_source_level_index;
    int structure2_source_canonical_hash_verified;
    int structure2_source_envelope_valid;
    int external_prs3_placement_bound;
    uint64_t external_prs3_placement_trace_fnv1a64;
    uint64_t external_prs3_placement_header_fnv1a64;
    uint64_t external_prs3_placement_bitmap_fnv1a64;
    uint32_t external_prs3_placement_bitmap_offset;
    uint32_t external_prs3_placement_bitmap_size;
    uint64_t external_prs3_placement_palt_fnv1a64;
    uint32_t external_prs3_placement_palt_size;
    uint64_t external_prs3_placement_dgn_fnv1a64;
    uint32_t external_prs3_placement_descriptor_index;
    uint64_t external_prs3_placement_frame_sequence;
    uint64_t external_prs3_placement_command_sequence;
    uint64_t external_prs3_placement_route_epoch;
    int external_prs3_placement_descriptor_target_bound;
    uint64_t external_prs3_placement_descriptor_fnv1a64;
    uint32_t external_prs3_placement_image_anchor_offset;
    uint64_t external_prs3_placement_image_candidate_fnv1a64;
    uint32_t external_prs3_placement_palette_anchor_offset;
    uint64_t external_prs3_placement_palette_candidate_fnv1a64;
    int external_structure1f_placement_bound;
    uint64_t external_structure1f_placement_dgn_fnv1a64;
    uint32_t external_structure1f_placement_descriptor_index;
    uint64_t external_structure1f_placement_frame_sequence;
    uint64_t external_structure1f_placement_command_sequence;
    uint64_t external_structure1f_placement_descriptor_fnv1a64;
    int structure2_floor_command_sources_consumed;
    Nexus_V1_DgnStructure1FDirectFloorCommandSource
        structure1f_direct_floor_sources[NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS];
    Nexus_V1_DgnStructure1FDirectFloorCommandSourceReceipt
        structure1f_direct_floor_source_receipt;
    int structure1f_direct_floor_sources_consumed;
    Nexus_V1_DgnStructure1FItemMaterialBinding
        structure1f_item_command_bindings[NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS];
    Nexus_V1_DgnStructure1FItemMaterialReceipt
        structure1f_item_command_binding_receipt;
    int structure1f_item_command_sources_consumed;
    Nexus_V1_DgnCommandPacked4BppMaterial
        structure1f_item_floor_materials[NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS];
    Nexus_V1_DgnCommandPacked4BppMaterialReceipt
        structure1f_item_floor_material_receipt;
    int structure1f_item_floor_materials_consumed;
    Nexus_V1_DgnStructure1FStructure1ACommandSource
        structure1a_owned_cell_sources[NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS];
    Nexus_V1_DgnStructure1FStructure1ACommandSourceReceipt
        structure1a_owned_cell_source_receipt;
    int structure1a_owned_cell_sources_consumed;
    Nexus_V1_DgnStructure1AStructure3TopologyCandidate
        structure1a_structure3_topology_candidates[
            NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS];
    Nexus_V1_DgnStructure1AStructure3TopologyCandidateReceipt
        structure1a_structure3_topology_candidate_receipt;
    int structure1a_structure3_topology_candidates_consumed;
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
    /* Identity of the exact LEV bytes retained by the engine after the
     * canonical package lookup.  A later host/capture handoff must not use
     * a receipt for one package entry with a different in-memory DGN. */
    int loaded_bytes_bound;
    int loaded_dgn_size;
    uint64_t loaded_dgn_fnv1a64;
    int structure2_payload_envelope_valid;
    int materialization_bound;
    int payload_decoder_permitted;
    int fallback_visuals_permitted;
} Nexus_V1_DgnStructure2SourceReceipt;

/* One original-capture-bound Structure3 face source retained by the live
 * engine. The capture spans are intentionally opaque: this cache provides a
 * later verified Saturn renderer the same face, normal, and complete capture
 * packet that admission checked, not a host interpretation of texture,
 * palette, VDP1, transform, or culling bytes. */
typedef struct {
    int valid;
    int level_index;
    uint32_t entry_index;
    uint32_t face_ordinal;
    uint32_t face_offset;
    uint32_t face_length;
    uint64_t face_fnv1a64;
    uint64_t package_fnv1a64;
    Nexus_V1_DgnStructure3Face face;
    Nexus_V1_DgnStructure3Vector vertices[4];
    int vertex_slot_count;
    Nexus_V1_DgnStructure3Vector normal;
    uint8_t *texture_span;
    int texture_span_size;
    uint8_t *palette_state;
    int palette_state_size;
    uint8_t *vdp1_state;
    int vdp1_state_size;
    uint8_t *transform_state;
    int transform_state_size;
    uint8_t *normal_culling_state;
    int normal_culling_state_size;
    uint8_t *vdp1_command;
    int vdp1_command_size;
    uint64_t capture_session_fnv1a64;
    uint64_t capture_bundle_fnv1a64;
    uint64_t capture_trace_order_fnv1a64;
    int capture_bundle_hash_verified;
    int capture_trace_order_verified;
    int original_saturn_capture_verified;
    Nexus_V1_DgnStructure3FaceCaptureBindingReceipt binding;
    /* A separately selected Structure1F/Structure1A owner row may be
     * revalidated against this admitted face capture.  This preserves both
     * original source sides for a later Saturn trace comparison; it does not
     * prove model-index-to-entry mapping or any draw semantics. */
    int structure1a_owner_correlation_bound;
    int structure1a_owner_x;
    int structure1a_owner_y;
    int structure1f_entry_index;
    Nexus_V1_DgnStructure1FFamily structure1f_family;
    uint8_t structure1f_tag;
    uint8_t structure1f_face_selector;
    uint16_t structure1a_index;
    uint8_t structure1a_kind;
    uint8_t structure3_model_index;
    uint8_t z_rotation;
    uint32_t structure3_owner_payload_fnv1a32;
    int structure3_entry_mapping_proven;
    int blocks_real_dgn_mesh_render;
} Nexus_V1_DgnStructure3RuntimeSource;

/* The only Structure3 mesh input available to the DGN viewport.  All
 * pointers remain owned by the engine and are valid until the next level
 * replacement or shutdown.  This preserves the admitted face geometry and
 * opaque capture packet without interpreting any Saturn render state. */
typedef struct {
    int valid;
    int source_geometry_bound;
    int no_draw_only;
    int level_index;
    uint32_t entry_index;
    uint32_t face_ordinal;
    Nexus_V1_DgnStructure3Face face;
    const Nexus_V1_DgnStructure3Vector *vertices;
    int vertex_count;
    const Nexus_V1_DgnStructure3Vector *normal;
    const uint8_t *texture_span;
    int texture_span_size;
    const uint8_t *palette_state;
    int palette_state_size;
    const uint8_t *vdp1_state;
    int vdp1_state_size;
    const uint8_t *transform_state;
    int transform_state_size;
    const uint8_t *normal_culling_state;
    int normal_culling_state_size;
    const uint8_t *vdp1_command;
    int vdp1_command_size;
    int structure1a_owner_correlation_bound;
    int structure1a_owner_x;
    int structure1a_owner_y;
    int structure1f_entry_index;
    Nexus_V1_DgnStructure1FFamily structure1f_family;
    uint8_t structure1f_tag;
    uint8_t structure1f_face_selector;
    uint16_t structure1a_index;
    uint8_t structure1a_kind;
    uint8_t structure3_model_index;
    uint8_t z_rotation;
    uint32_t structure3_owner_payload_fnv1a32;
    int structure3_entry_mapping_proven;
    int blocks_real_dgn_mesh_render;
} Nexus_V1_DgnStructure3RenderPacket;

/* Result of binding a dual-source Structure1F/Structure1A target to an
 * already admitted original-Saturn Structure3 capture.  This is deliberately
 * a source correlation only, never a mesh/material/pixel interpretation. */
typedef struct {
    int active_canonical_lev_bound;
    int runtime_capture_attested;
    int target_source_revalidated;
    int owner_context_bound;
    int structure3_entry_mapping_proven;
    int no_draw_only;
    int blocks_real_dgn_mesh_render;
} Nexus_V1_DgnStructure1AStructure3RuntimeCorrelationReceipt;

/* Source-owned request receipt for an external Saturn capture producer. */
typedef struct {
    int active_canonical_lev_bound;
    int material_plan_prepared;
    int topology_candidate_bound;
    int target_built;
    int target_written;
    int no_draw_only;
    int fallback_visuals_permitted;
} Nexus_V1_DgnStructure1AStructure3CaptureTargetRouteReceipt;

/* Parsed hardware framing from an already source-bound Structure3 capture
 * command. It records only documented VDP1 command-table fields; it neither
 * establishes texel/palette semantics nor authorizes a renderer. */
typedef struct {
    int valid;
    int level_index;
    int source_byte_count;
    uint64_t source_bytes_fnv1a64;
    int original_saturn_capture_bound;
    int complete_vdp1_command_record;
    int command_format_parsed;
    int texture_primitive_observed;
    int texture_format_framed;
    int texture_span_size_matches_command;
    int coordinate_words_framed;
    Nexus_V1_Vdp1TextureCommand command;
    int pixel_format_proven;
    int palette_format_proven;
    int decoder_permitted;
    int no_draw_only;
    int fallback_visuals_permitted;
    int blocks_real_dgn_mesh_render;
} Nexus_V1_DgnStructure3Vdp1CommandFramingReceipt;

/* Correlates an authenticated full VDP1-VRAM snapshot with the bounded
 * CMDSRCA window of the same Structure3 capture. Matching bytes are capture
 * provenance only: no texture decoder, palette interpretation, or drawing
 * behavior is implied. */
typedef struct {
    int valid;
    int level_index;
    int source_byte_count;
    uint64_t source_bytes_fnv1a64;
    int original_saturn_capture_bound;
    int complete_vdp1_vram_snapshot;
    Nexus_V1_DgnStructure3Vdp1CommandFramingReceipt command_framing;
    int texture_lane_matches_vram_window;
    int pixel_format_proven;
    int palette_format_proven;
    int decoder_permitted;
    int no_draw_only;
    int fallback_visuals_permitted;
    int blocks_real_dgn_mesh_render;
} Nexus_V1_DgnStructure3Vdp1VramWindowReceipt;

/* A full authenticated VDP1-VRAM lane may prove that the exact captured
 * command record occurs once in VRAM. CMDLINK is still only an address field:
 * it does not establish command flow, a target opcode, or drawing. */
typedef struct {
    int valid;
    int level_index;
    int source_byte_count;
    uint64_t source_bytes_fnv1a64;
    int original_saturn_capture_bound;
    int complete_vdp1_vram_snapshot;
    Nexus_V1_DgnStructure3Vdp1CommandFramingReceipt command_framing;
    int command_record_occurrence_count;
    int command_record_unique_in_vram;
    uint32_t command_record_byte_offset;
    int command_link_target_bounded;
    uint32_t command_link_byte_offset;
    int command_link_target_record_framed;
    Nexus_V1_Vdp1TextureCommand command_link_target;
    int pixel_format_proven;
    int palette_format_proven;
    int decoder_permitted;
    int no_draw_only;
    int fallback_visuals_permitted;
    int blocks_real_dgn_mesh_render;
} Nexus_V1_DgnStructure3Vdp1CommandVramReceipt;

/* Renderer-bound provenance for the active canonical LEV entry.  This makes
 * the package-to-viewport boundary inspectable without assigning a Saturn
 * decoding or drawing meaning to any captured bytes. */
typedef struct {
    int valid;
    int package_source_bound;
    int structure3_payload_bound;
    int level_index;
    int source_byte_count;
    uint64_t source_bytes_fnv1a64;
    int structure3_payload_byte_count;
    uint32_t structure3_payload_fnv1a32;
    int original_saturn_capture_bound;
    int texture_span_bound;
    int palette_state_bound;
    int vdp1_state_bound;
    int transform_state_bound;
    int normal_culling_state_bound;
    int vdp1_command_bound;
    int vdp1_command_format_framed;
    int vdp1_texture_format_framed;
    int vdp1_coordinate_words_framed;
    Nexus_V1_DgnStructure3Vdp1CommandFramingReceipt vdp1_command_framing;
    int vdp1_vram_window_bound;
    Nexus_V1_DgnStructure3Vdp1VramWindowReceipt vdp1_vram_window;
    int vdp1_command_vram_bound;
    Nexus_V1_DgnStructure3Vdp1CommandVramReceipt vdp1_command_vram;
    int texture_decode_unproven;
    int palette_decode_unproven;
    int vdp1_draw_unproven;
    int transform_culling_unproven;
    int no_draw_only;
    int blocks_real_dgn_mesh_render;
    int fallback_visuals_permitted;
} Nexus_V1_DgnActiveLevelRendererSourceReceipt;

/* Active canonical LEV Structure3 directory, retained as source evidence for
 * capture tools. Directory offsets are bounded package facts only. */
typedef struct {
    int valid;
    int level_index;
    int source_byte_count;
    uint64_t source_bytes_fnv1a64;
    Nexus_V1_DgnStructure3DirectoryReceipt directory;
    int no_draw_only;
    int fallback_visuals_permitted;
} Nexus_V1_DgnActiveStructure3DirectoryReceipt;

/* Active canonical LEV Structure3 mesh facts for renderer/capture tooling.
 * This carries only source-bound topology/vector/face-normal evidence. It
 * deliberately does not authorize a Saturn transform, material, or draw. */
typedef struct {
    int valid;
    int level_index;
    int source_byte_count;
    uint64_t source_bytes_fnv1a64;
    Nexus_V1_DgnStructure3MeshSemanticHandoffReceipt mesh_semantics;
    int no_draw_only;
    int fallback_visuals_permitted;
} Nexus_V1_DgnActiveStructure3MeshSemanticReceipt;

/* Active canonical LEV Structure3 entry and face-row framing. Header offsets
 * and local index bounds are source facts only: they do not establish a
 * Saturn transform, surface, material, texture, palette, or draw command. */
typedef struct {
    int valid;
    int level_index;
    int source_byte_count;
    uint64_t source_bytes_fnv1a64;
    Nexus_V1_DgnStructure3EntryHeaderReceipt entry_headers;
    Nexus_V1_DgnStructure3FaceReceipt faces;
    int transform_semantics_proven;
    int no_draw_only;
    int fallback_visuals_permitted;
} Nexus_V1_DgnActiveStructure3FaceFramingReceipt;

/* Active canonical LEV Structure3 face topology and bounded material-selector
 * joins. Selectors remain identifiers only: this does not decode material
 * bytes, palettes, UVs, VDP1 commands, or authorize a draw. */
typedef struct {
    int valid;
    int level_index;
    int source_byte_count;
    uint64_t source_bytes_fnv1a64;
    Nexus_V1_DgnStructure3FaceReceipt faces;
    Nexus_V1_DgnStructure3FaceMaterialReceipt materials;
    int face_topology_material_binding_complete;
    int no_draw_only;
    int fallback_visuals_permitted;
} Nexus_V1_DgnActiveStructure3FaceMaterialReceipt;

/* Active canonical LEV ownership chain from Structure1F through its unique
 * Structure1B owner to the indexed Structure1A row and raw model/face
 * selectors. This is source topology only, never placement, transform,
 * material, pixel, or draw semantics. */
typedef struct {
    int valid;
    int level_index;
    int source_byte_count;
    uint64_t source_bytes_fnv1a64;
    Nexus_V1_DgnStructure1FSpatialReceipt spatial;
    Nexus_V1_DgnStructure1ABoundaryReceipt boundary;
    Nexus_V1_DgnStructure1ARelationReceipt relation;
    Nexus_V1_DgnStructure3ModelReferenceReceipt model_references;
    Nexus_V1_DgnStructure1FFaceSelectorReceipt face_selectors;
    Nexus_V1_DgnStructure3ModelFaceSelectorReceipt model_face_selectors;
    int owner_chain_complete;
    int no_draw_only;
    int fallback_visuals_permitted;
} Nexus_V1_DgnActiveStructure1AOwnerChainReceipt;

/* Active canonical LEV Structure2 descriptor envelope and optional
 * Structure1G global-to-local descriptor binding. The post-FFFF payload is
 * still opaque: no encoding, palette, pixels, animation, or draw follows. */
typedef struct {
    int valid;
    int level_index;
    int source_byte_count;
    uint64_t source_bytes_fnv1a64;
    int descriptor_count;
    int structure1g_entry_count;
    int structure1g_structure2_bindings_complete;
    Nexus_V1_DgnStructure2Payload payload;
    int descriptor_layout_complete;
    int no_draw_only;
    int fallback_visuals_permitted;
} Nexus_V1_DgnActiveStructure2DescriptorReceipt;

/* Source-owned request for an external Saturn capture of one exact Structure2
 * descriptor. The descriptor fields and post-FFFF bytes stay opaque; a
 * target is not a trace, format decoder, palette, or draw authorization. */
typedef struct {
    int valid;
    int level_index;
    int source_byte_count;
    uint64_t source_bytes_fnv1a64;
    int descriptor_index;
    int descriptor_byte_offset;
    Nexus_V1_DgnStructure2Texture descriptor;
    int opaque_payload_byte_offset;
    int opaque_payload_byte_count;
    uint64_t descriptor_bytes_fnv1a64;
    uint64_t opaque_payload_fnv1a64;
    uint32_t image_payload_anchor_offset;
    uint32_t image_payload_next_anchor_offset;
    uint32_t image_payload_candidate_byte_count;
    uint64_t image_payload_candidate_fnv1a64;
    uint32_t palette_payload_anchor_offset;
    uint32_t palette_payload_next_anchor_offset;
    uint32_t palette_payload_candidate_byte_count;
    uint64_t palette_payload_candidate_fnv1a64;
    int image_payload_candidate_bound;
    int palette_payload_candidate_bound;
    int shared_image_palette_payload_anchor;
    int capture_producer_required;
    int original_saturn_capture_required;
    int no_draw_only;
    int fallback_visuals_permitted;
} Nexus_V1_DgnStructure2DescriptorCaptureTarget;

/* Measured Structure2 descriptor facts from an active canonical LEV. The
 * observed encoding values are retained as raw classes, not bit-depth or
 * VDP1 mode claims. An absent palette offset also never implies a default
 * palette. Both candidate routes stay blocked until an authentic Saturn trace
 * proves actual pixel spans, palette addressing, and VDP1 use. */
typedef struct {
    int valid;
    int level_index;
    int source_byte_count;
    uint64_t source_bytes_fnv1a64;
    int descriptor_count;
    int image_payload_anchor_count;
    int palette_payload_anchor_count;
    int palette_payload_absent_count;
    int encoding_0x0008_count;
    int encoding_0x0008_palette_anchor_count;
    int encoding_0x0008_palette_absent_count;
    int encoding_0x0028_count;
    int encoding_0x0028_palette_anchor_count;
    int encoding_0x0028_palette_absent_count;
    int unobserved_encoding_count;
    int image_payload_anchors_complete;
    int descriptor_format_classes_complete;
    int pixel_span_proven;
    int palette_addressing_proven;
    int vdp1_format_proven;
    int decoder_permitted;
    int no_draw_only;
    int fallback_visuals_permitted;
} Nexus_V1_DgnStructure2FormatEvidenceReceipt;

typedef struct {
    int valid;
    int level_index;
    int descriptor_count;
    int decoded_count;
    int encoding_0x0008_decoded;
    int encoding_0x0028_decoded;
    int palette_overflow_count;
} Nexus_V1_DgnStructure2TextureDecodeReceipt;

/* One exact static-textured Structure3 face joined to its bounded Structure2
 * descriptor in the same canonical LEV. This is capture-producer input only:
 * the descriptor's post-FFFF bytes remain opaque and no pixel, palette, UV,
 * VDP1, transform, or draw meaning is implied. */
typedef struct {
    int valid;
    int level_index;
    int source_byte_count;
    uint64_t source_bytes_fnv1a64;
    uint32_t structure3_entry_index;
    uint32_t face_ordinal;
    int face_byte_offset;
    uint64_t face_bytes_fnv1a64;
    Nexus_V1_DgnStructure3Face face;
    uint8_t static_texture_selector;
    Nexus_V1_DgnStructure2DescriptorCaptureTarget descriptor_target;
    int static_selector_descriptor_bound;
    /* Exact Structure2-relative byte anchors. These establish only where an
     * external capture must observe source reads; zero palette offset remains
     * an observed absent anchor, not a default-palette interpretation. */
    int image_payload_byte_offset;
    int palette_payload_byte_offset;
    int image_payload_anchor_bound;
    int palette_payload_anchor_bound;
    /* Bounded next-anchor intervals are capture windows only; they do not
     * define image/palette lengths or any Saturn graphics format. */
    uint32_t image_payload_next_anchor_offset;
    uint32_t image_payload_candidate_byte_count;
    uint32_t palette_payload_next_anchor_offset;
    uint32_t palette_payload_candidate_byte_count;
    int image_payload_interval_bound;
    int palette_payload_interval_bound;
    int capture_producer_required;
    int original_saturn_capture_required;
    int no_draw_only;
    int fallback_visuals_permitted;
} Nexus_V1_DgnStructure3StaticMaterialCaptureTarget;

/* A producer-facing bundle with independently source-bound owner/face and
 * static material inputs. The owner-to-entry mapping remains explicitly
 * unproven; this requests a trace that can establish it, never a draw. */
#define NEXUS_V1_STRUCTURE1A_STRUCTURE3_MATERIAL_CAPTURE_TARGET_MAGIC \
    "FIRESTAFF_NEXUS_STRUCTURE1A_STRUCTURE3_MATERIAL_CAPTURE_TARGET_V1"
typedef struct {
    int valid;
    int level_index;
    Nexus_V1_DgnStructure1AStructure3CaptureTargetReceipt owner_face_target;
    Nexus_V1_DgnStructure3StaticMaterialCaptureTarget material_target;
    int owner_face_source_bound;
    int static_material_source_bound;
    int owner_to_entry_mapping_proven;
    int capture_producer_required;
    int original_saturn_capture_required;
    int no_draw_only;
    int fallback_visuals_permitted;
    int blocks_real_dgn_mesh_render;
} Nexus_V1_DgnStructure1AStructure3MaterialCaptureTarget;

/* One bounded Structure3 face in the active canonical LEV, joined to its
 * exact Structure2 static-material source anchors. This is the package-side
 * renderer input for an eventual Saturn-backed path, not a decoded surface:
 * the copied vectors remain original 16.16 rows and the descriptor payload
 * remains opaque until an independent trace proves pixel/palette/VDP1 rules. */
typedef struct {
    int valid;
    int source_geometry_bound;
    int material_descriptor_bound;
    int level_index;
    int source_byte_count;
    uint64_t source_bytes_fnv1a64;
    uint32_t structure3_entry_index;
    uint32_t face_ordinal;
    uint32_t face_offset;
    uint32_t face_length;
    uint64_t face_fnv1a64;
    uint64_t package_fnv1a64;
    uint64_t descriptor_fnv1a64;
    uint32_t image_offset;
    uint32_t image_length;
    uint32_t palette_offset;
    uint32_t palette_length;
    Nexus_V1_DgnStructure3Face face;
    Nexus_V1_DgnStructure3Vector vertices[4];
    int vertex_slot_count;
    Nexus_V1_DgnStructure3Vector normal;
    Nexus_V1_DgnStructure3StaticMaterialCaptureTarget material_target;
    int texture_surface_index;
    int texture_surface_valid;
    int transform_semantics_proven;
    int pixel_palette_vdp1_semantics_proven;
    int decoder_permitted;
    int no_draw_only;
    int fallback_visuals_permitted;
    int blocks_real_dgn_mesh_render;
} Nexus_V1_DgnStructure3PackageGeometryPacket;

/* A renderer may traverse every static-textured package face through this
 * receipt without taking ownership of pixels or inferring Saturn state. */
typedef struct {
    int valid;
    int level_index;
    int source_byte_count;
    uint64_t source_bytes_fnv1a64;
    int structure3_entry_count;
    int candidate_face_count;
    int static_material_face_count;
    int consumed_face_count;
    int complete;
    int transform_semantics_proven;
    int pixel_palette_vdp1_semantics_proven;
    int decoder_permitted;
    int no_draw_only;
    int fallback_visuals_permitted;
    int blocks_real_dgn_mesh_render;
} Nexus_V1_DgnStructure3PackageGeometrySceneReceipt;

typedef int (*Nexus_V1_DgnStructure3PackageGeometryConsumer)(
    void *context, const Nexus_V1_DgnStructure3PackageGeometryPacket *packet);

/* One texture-flagged 08xx Structure3 face joined to its bounded Structure1G
 * declaration and first local Structure2 descriptor. The sequence remains an
 * original byte route only: it does not select a frame, advance timing, or
 * decode pixel/palette/VDP1 state. */
typedef struct {
    int valid;
    int source_geometry_bound;
    int animation_declaration_bound;
    int first_descriptor_bound;
    int level_index;
    int source_byte_count;
    uint64_t source_bytes_fnv1a64;
    uint32_t structure3_entry_index;
    uint32_t face_ordinal;
    int face_byte_offset;
    uint64_t face_bytes_fnv1a64;
    Nexus_V1_DgnStructure3Face face;
    Nexus_V1_DgnStructure3Vector vertices[4];
    int vertex_slot_count;
    Nexus_V1_DgnStructure3Vector normal;
    int structure1g_entry_index;
    uint8_t animation_id;
    uint16_t first_image_index;
    uint16_t first_structure2_image_id;
    uint16_t sequence_word_offset;
    int sequence_instruction_count;
    int image_instruction_count;
    int goto_instruction_count;
    Nexus_V1_DgnStructure2DescriptorCaptureTarget first_descriptor_target;
    int animation_execution_permitted;
    int transform_semantics_proven;
    int pixel_palette_vdp1_semantics_proven;
    int decoder_permitted;
    int no_draw_only;
    int fallback_visuals_permitted;
    int blocks_real_dgn_mesh_render;
} Nexus_V1_DgnStructure3AnimatedMaterialPacket;

typedef struct {
    int valid;
    int level_index;
    int source_byte_count;
    uint64_t source_bytes_fnv1a64;
    int structure3_entry_count;
    int candidate_face_count;
    int animated_face_count;
    int consumed_face_count;
    int complete;
    int animation_execution_permitted;
    int transform_semantics_proven;
    int pixel_palette_vdp1_semantics_proven;
    int decoder_permitted;
    int no_draw_only;
    int fallback_visuals_permitted;
    int blocks_real_dgn_mesh_render;
} Nexus_V1_DgnStructure3AnimatedMaterialSceneReceipt;

typedef int (*Nexus_V1_DgnStructure3AnimatedMaterialConsumer)(
    void *context, const Nexus_V1_DgnStructure3AnimatedMaterialPacket *packet);

/* One declared non-control instruction from a bounded Structure1G sequence,
 * joined to its exact animated Structure3 face and local Structure2
 * descriptor. The instruction is observed in source order only: it neither
 * selects a frame nor executes the sequence. */
typedef struct {
    int valid;
    int source_geometry_bound;
    int animation_declaration_bound;
    int descriptor_bound;
    int level_index;
    uint32_t structure3_entry_index;
    uint32_t face_ordinal;
    int structure1g_entry_index;
    uint32_t image_instruction_ordinal;
    int instruction_byte_offset;
    uint64_t instruction_bytes_fnv1a64;
    uint16_t global_image_index;
    uint16_t structure2_image_id;
    Nexus_V1_DgnStructure2DescriptorCaptureTarget descriptor_target;
    int animation_execution_permitted;
    int pixel_palette_vdp1_semantics_proven;
    int decoder_permitted;
    int no_draw_only;
    int fallback_visuals_permitted;
    int blocks_real_dgn_mesh_render;
} Nexus_V1_DgnStructure3AnimatedMaterialImagePacket;

typedef struct {
    int valid;
    int level_index;
    int source_byte_count;
    uint64_t source_bytes_fnv1a64;
    int animated_face_count;
    int declared_image_instruction_count;
    int consumed_image_instruction_count;
    int complete;
    int animation_execution_permitted;
    int pixel_palette_vdp1_semantics_proven;
    int decoder_permitted;
    int no_draw_only;
    int fallback_visuals_permitted;
    int blocks_real_dgn_mesh_render;
} Nexus_V1_DgnStructure3AnimatedMaterialImageSceneReceipt;

/* Source-only coverage of the Structure2 payload anchors required by every
 * declared animated image instruction. Candidate intervals remain opaque: no
 * span length, palette format, pixel codec, VDP1 mode, or draw is inferred. */
typedef struct {
    int valid;
    int level_index;
    int source_byte_count;
    uint64_t source_bytes_fnv1a64;
    int declared_image_instruction_count;
    int consumed_image_instruction_count;
    int image_payload_anchor_count;
    int palette_payload_anchor_count;
    int complete;
    int pixel_palette_vdp1_semantics_proven;
    int decoder_permitted;
    int no_draw_only;
    int fallback_visuals_permitted;
    int blocks_real_dgn_mesh_render;
} Nexus_V1_DgnStructure3AnimatedMaterialPayloadSceneReceipt;

typedef int (*Nexus_V1_DgnStructure3AnimatedMaterialImageConsumer)(
    void *context, const Nexus_V1_DgnStructure3AnimatedMaterialImagePacket *packet);

/* A non-texture-flagged Structure3 face with its exact typed geometry. The
 * raw fill selector stays opaque: no flat-colour, blend, palette, VDP1, or
 * raster meaning is assigned before original Saturn evidence exists. */
typedef struct {
    int valid;
    int source_geometry_bound;
    int raw_fill_bound;
    int level_index;
    int source_byte_count;
    uint64_t source_bytes_fnv1a64;
    uint32_t structure3_entry_index;
    uint32_t face_ordinal;
    int face_byte_offset;
    uint64_t face_bytes_fnv1a64;
    Nexus_V1_DgnStructure3Face face;
    Nexus_V1_DgnStructure3Vector vertices[4];
    int vertex_slot_count;
    Nexus_V1_DgnStructure3Vector normal;
    uint16_t raw_fill_selector;
    int flat_fill_semantics_proven;
    int transform_semantics_proven;
    int pixel_palette_vdp1_semantics_proven;
    int decoder_permitted;
    int no_draw_only;
    int fallback_visuals_permitted;
    int blocks_real_dgn_mesh_render;
} Nexus_V1_DgnStructure3UntexturedFacePacket;

typedef struct {
    int valid;
    int level_index;
    int source_byte_count;
    uint64_t source_bytes_fnv1a64;
    int structure3_entry_count;
    int candidate_face_count;
    int untextured_face_count;
    int consumed_face_count;
    int complete;
    int flat_fill_semantics_proven;
    int transform_semantics_proven;
    int pixel_palette_vdp1_semantics_proven;
    int decoder_permitted;
    int no_draw_only;
    int fallback_visuals_permitted;
    int blocks_real_dgn_mesh_render;
} Nexus_V1_DgnStructure3UntexturedFaceSceneReceipt;

typedef int (*Nexus_V1_DgnStructure3UntexturedFaceConsumer)(
    void *context, const Nexus_V1_DgnStructure3UntexturedFacePacket *packet);

/* Complete active Structure3 source scene. The three category traversals are
 * jointly required before a later Saturn renderer may even consider the
 * scene; this receipt remains no-draw until independent format evidence. */
typedef struct {
    int valid;
    int level_index;
    int source_byte_count;
    uint64_t source_bytes_fnv1a64;
    Nexus_V1_DgnStructure3PackageGeometrySceneReceipt static_scene;
    Nexus_V1_DgnStructure3AnimatedMaterialSceneReceipt animated_scene;
    /* An 08xx category is complete only after every declared Structure1G
     * image instruction is source-bound to its local Structure2 descriptor. */
    Nexus_V1_DgnStructure3AnimatedMaterialImageSceneReceipt
        animated_image_scene;
    Nexus_V1_DgnStructure3AnimatedMaterialPayloadSceneReceipt
        animated_payload_scene;
    Nexus_V1_DgnStructure3UntexturedFaceSceneReceipt untextured_scene;
    int face_count;
    int traversed_face_count;
    int animated_image_coverage_complete;
    int animated_payload_coverage_complete;
    /* Every active Structure2 descriptor needs a bounded, source-owned
     * payload anchor before its faces can count as a complete material route. */
    int structure2_descriptor_count;
    int structure2_payload_anchor_count;
    int structure2_payload_anchors_consumed;
    int structure2_image_anchor_count;
    int structure2_palette_anchor_count;
    int structure2_payload_coverage_complete;
    int category_coverage_complete;
    int transform_semantics_proven;
    int pixel_palette_vdp1_semantics_proven;
    int decoder_permitted;
    int no_draw_only;
    int fallback_visuals_permitted;
    int blocks_real_dgn_mesh_render;
} Nexus_V1_DgnStructure3CompleteSourceSceneReceipt;

/* One exact Structure1F source row from the active canonical LEV. Direct
 * coordinates and Structure1A ownership are retained as separate raw source
 * facts; neither establishes placement, trigger, object, mesh, or draw
 * semantics. */
typedef struct {
    int valid;
    int level_index;
    int source_byte_count;
    uint64_t source_bytes_fnv1a64;
    int entry_index;
    Nexus_V1_DgnStructure1FEntry entry;
    uint32_t descriptor_offset;
    uint32_t descriptor_length;
    uint64_t descriptor_fnv1a64;
    uint64_t package_fnv1a64;
    int direct_coordinate_source;
    int structure1a_owner_source;
    int no_draw_only;
    int fallback_visuals_permitted;
    int blocks_real_dgn_mesh_render;
} Nexus_V1_DgnStructure1FSourcePacket;

typedef struct {
    int valid;
    int level_index;
    int source_byte_count;
    uint64_t source_bytes_fnv1a64;
    int entry_count;
    int consumed_entry_count;
    int direct_coordinate_entry_count;
    int structure1a_owner_entry_count;
    int item_entry_count;
    int floor_decoration_entry_count;
    int floor_sensor_entry_count;
    int alcove_entry_count;
    int wall_decoration_entry_count;
    int wall_sensor_entry_count;
    int family_coverage_complete;
    int no_draw_only;
    int fallback_visuals_permitted;
    int blocks_real_dgn_mesh_render;
} Nexus_V1_DgnStructure1FSourceSceneReceipt;

typedef int (*Nexus_V1_DgnStructure1FSourceConsumer)(
    void *context, const Nexus_V1_DgnStructure1FSourcePacket *packet);

/* One exact bounded Structure1C record. Its four bytes remain opaque: this
 * source route proves record identity and Structure1B reference occurrence,
 * never collision geometry, blocking behavior, or rendering. */
typedef struct {
    int valid;
    int level_index;
    int source_byte_count;
    uint64_t source_bytes_fnv1a64;
    int record_index;
    uint8_t raw_bytes[4];
    int referenced_by_structure1b;
    int reference_occurrence_count;
    int first_reference_x;
    int first_reference_y;
    int last_reference_x;
    int last_reference_y;
    int no_draw_only;
    int fallback_visuals_permitted;
    int blocks_real_dgn_mesh_render;
} Nexus_V1_DgnStructure1CSourcePacket;

typedef struct {
    int valid;
    int level_index;
    int source_byte_count;
    uint64_t source_bytes_fnv1a64;
    int record_count;
    int indexed_record_count;
    int consumed_record_count;
    int referenced_record_count;
    int unreferenced_record_count;
    int reference_occurrence_count;
    int no_draw_only;
    int fallback_visuals_permitted;
    int blocks_real_dgn_mesh_render;
} Nexus_V1_DgnStructure1CSourceSceneReceipt;

typedef int (*Nexus_V1_DgnStructure1CSourceConsumer)(
    void *context, const Nexus_V1_DgnStructure1CSourcePacket *packet);

/* One source-only lookup from an active Structure1B grid cell to its bounded
 * Structure1C record. This preserves the exact indexed owner relation but
 * never assigns collision, mesh, or draw semantics to the record bytes. */
typedef struct {
    int valid;
    int cell_x;
    int cell_y;
    uint16_t collision_ref;
    Nexus_V1_DgnStructure1CSourcePacket record;
    int no_draw_only;
    int fallback_visuals_permitted;
    int blocks_real_dgn_mesh_render;
} Nexus_V1_DgnStructure1CCellSourcePacket;

/* One descriptor-owned Structure2 payload anchor. The interval ends at the
 * next observed descriptor anchor (or payload end) only as a bounded capture
 * candidate; it is not an image span, palette span, or codec claim. */
typedef struct {
    int valid;
    int level_index;
    int source_byte_count;
    uint64_t source_bytes_fnv1a64;
    int descriptor_index;
    uint32_t descriptor_offset;
    uint32_t descriptor_length;
    uint64_t descriptor_fnv1a64;
    uint64_t package_fnv1a64;
    int palette_anchor;
    uint32_t payload_anchor_offset;
    uint32_t next_anchor_offset;
    uint32_t candidate_byte_count;
    Nexus_V1_DgnStructure2Texture descriptor;
    int no_draw_only;
    int fallback_visuals_permitted;
    int blocks_real_dgn_mesh_render;
} Nexus_V1_DgnStructure2PayloadAnchorPacket;

typedef struct {
    int valid;
    int level_index;
    int source_byte_count;
    uint64_t source_bytes_fnv1a64;
    int descriptor_count;
    int anchor_count;
    int consumed_anchor_count;
    int image_anchor_count;
    int palette_anchor_count;
    int unique_anchor_count;
    int reused_anchor_count;
    int candidate_interval_byte_count;
    int no_draw_only;
    int fallback_visuals_permitted;
    int blocks_real_dgn_mesh_render;
} Nexus_V1_DgnStructure2PayloadAnchorSceneReceipt;

typedef int (*Nexus_V1_DgnStructure2PayloadAnchorConsumer)(
    void *context, const Nexus_V1_DgnStructure2PayloadAnchorPacket *packet);

/* A raw external capture can be bound to an exact retail Structure2
 * descriptor, but capture admission never asserts a pixel, palette, or VDP1
 * decoder. Provenance is supplied by the capture owner, not inferred from a
 * manifest string. */
#define NEXUS_V1_STRUCTURE2_SATURN_RAW_TRACE_MAGIC \
    "FIRESTAFF_NEXUS_STRUCTURE2_SATURN_RAW_TRACE_V1"
typedef enum {
    NEXUS_V1_STRUCTURE2_TRACE_MISSING = 0,
    NEXUS_V1_STRUCTURE2_TRACE_BLOCKED_MALFORMED = 1,
    NEXUS_V1_STRUCTURE2_TRACE_BLOCKED_TARGET_MISMATCH = 2,
    NEXUS_V1_STRUCTURE2_TRACE_BLOCKED_RAW_TRACE = 3,
    NEXUS_V1_STRUCTURE2_TRACE_BLOCKED_PROVENANCE = 4,
    NEXUS_V1_STRUCTURE2_TRACE_ADMITTED_OPAQUE = 5
} Nexus_V1_DgnStructure2TraceStatus;

typedef struct {
    Nexus_V1_DgnStructure2TraceStatus status;
    int level_index;
    int descriptor_index;
    int capture_target_bound;
    int manifest_target_bound;
    int image_payload_candidate_bound;
    int palette_payload_candidate_bound;
    int raw_trace_bytes_bound;
    size_t raw_trace_byte_count;
    uint64_t raw_trace_fnv1a64;
    int original_saturn_capture_verified;
    int opaque_trace_admitted;
    int decoder_permitted;
    int no_draw_only;
    int fallback_visuals_permitted;
    int blocks_real_dgn_mesh_render;
} Nexus_V1_DgnStructure2TraceAdmissionReceipt;

/* Admission result for one external trace that binds the complete atomic
 * Structure1F/1A, Structure3, and Structure2 capture request. Successful
 * admission keeps every lane opaque and cannot authorize a decoder or draw. */
typedef enum {
    NEXUS_V1_OWNER_MATERIAL_TRACE_MISSING = 0,
    NEXUS_V1_OWNER_MATERIAL_TRACE_BLOCKED_BUNDLE = 1,
    NEXUS_V1_OWNER_MATERIAL_TRACE_BLOCKED_TARGET = 2,
    NEXUS_V1_OWNER_MATERIAL_TRACE_BLOCKED_STRUCTURE2 = 3,
    NEXUS_V1_OWNER_MATERIAL_TRACE_ADMITTED_OPAQUE = 4
} Nexus_V1_DgnOwnerMaterialTraceStatus;

typedef struct {
    Nexus_V1_DgnOwnerMaterialTraceStatus status;
    int level_index;
    int descriptor_index;
    int atomic_target_bound;
    int owner_face_bound;
    int structure2_trace_admitted;
    int original_saturn_capture_verified;
    int opaque_trace_admitted;
    int decoder_permitted;
    int no_draw_only;
    int fallback_visuals_permitted;
    int blocks_real_dgn_mesh_render;
} Nexus_V1_DgnOwnerMaterialTraceAdmissionReceipt;

/* Active party pose bound to the canonical LEV source and the bounded raw
 * Structure1A transform-selector evidence. This is camera input provenance,
 * not a decoded Saturn camera, transform, culling, or drawing rule. */
typedef struct {
    int valid;
    int level_index;
    int source_byte_count;
    uint64_t source_bytes_fnv1a64;
    int party_x;
    int party_y;
    int party_dir;
    int party_cell_geometry_valid;
    int party_square_type;
    uint16_t party_collision_ref;
    uint16_t party_post_grid_0x30_ref;
    Nexus_V1_DgnStructure1ATransformSelectorReceipt transform_selectors;
    int transform_selector_source_bound;
    int saturn_camera_semantics_proven;
    int saturn_transform_semantics_proven;
    int no_draw_only;
    int fallback_visuals_permitted;
} Nexus_V1_DgnActiveTransformCameraFramingReceipt;

/* Runtime admission receipt for an externally captured Structure3 packet.
 * Every raw lane remains opaque and the successful route remains no-draw. */
typedef struct {
    int active_canonical_lev_bound;
    int raw_capture_host_intake_invoked;
    int manifest_parsed;
    int all_trace_lanes_authenticated;
    int complete_source_binding;
    int engine_consume_invoked;
    int runtime_source_consumed;
    int no_draw_only;
    int fallback_visuals_permitted;
} Nexus_V1_DgnStructure3RuntimeCaptureIntakeReceipt;

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

/* Exact, source-owned PRS3 loader evidence available during boot. This joins
 * canonical DM.BIN code and MENU.BPK framing, but deliberately carries no
 * decoded pixels and never authorizes a draw. */
typedef struct {
    Nexus_V1_LevelAuxSourceReceipt dm_bin_source;
    int menu_bpk_source_hash_verified;
    int cross_asset_framing_verified;
    int sh2_loader_route_verified;
    int valid;
    int decoder_promoted;
    int runtime_decode_permitted;
    int fallback_visuals_permitted;
    Nexus_V1_Prs3CrossAssetFrameReceipt cross_asset;
    Nexus_V1_Prs3Sh2V1ExecutionReceipt sh2_loader;
} Nexus_V1_MenuBpkPrs3ExecutionEvidenceReceipt;

/* ITEM.IBS is a package-level source for direct Structure1Fa item records.
 * A verified bank can provide command provenance, never a drawable icon or
 * floor texture without separate original VDP1 evidence. */
typedef struct {
    Nexus_V1_LevelAuxSourceReceipt source;
    int parsed_bank_valid;
    int source_bound;
    int fallback_visuals_permitted;
} Nexus_V1_ItemIbsRuntimeSourceReceipt;

/* Active canonical LEV consumption of the documented Structure1F →
 * Structure1A → Structure3 face/normal ordinal relation. This establishes
 * only bounded source ownership; transform, material, palette, and draw
 * semantics remain explicitly unavailable. */
typedef struct {
    int valid;
    int level_index;
    int canonical_lev_source_bound;
    int source_byte_count;
    Nexus_V1_DgnStructure3AttachmentReceipt attachment;
    int face_mesh_ordinal_relation_proven;
    int no_draw_only;
    int fallback_visuals_permitted;
} Nexus_V1_DgnActiveStructure1FFaceMeshReceipt;

/* One direct, byte-proved Structure1F owner to Structure3 entry/face join
 * from the active canonical LEV. It is suitable as a precise future capture
 * subject, but never authorizes transform, texture, palette, VDP1, or draw. */
typedef struct {
    int valid;
    int level_index;
    int source_byte_count;
    uint64_t source_bytes_fnv1a64;
    int structure1f_entry_index;
    int owner_x;
    int owner_y;
    uint16_t structure1a_index;
    uint8_t structure3_model_index;
    uint8_t z_rotation;
    uint8_t face_ordinal;
    Nexus_V1_DgnStructure3CaptureTargetReceipt face_target;
    int model_to_entry_proven;
    int face_ordinal_proven;
    int no_draw_only;
    int fallback_visuals_permitted;
} Nexus_V1_DgnStructure1FDirectMeshBindingReceipt;

/* Exact package geometry selected by a source-bound Structure1F owner. The
 * model and face ordinal are proven source relations; transform, material,
 * palette, VDP1, and drawing semantics remain unavailable. */
typedef struct {
    int valid;
    Nexus_V1_DgnStructure1FDirectMeshBindingReceipt direct_mesh;
    Nexus_V1_DgnStructure3Face face;
    Nexus_V1_DgnStructure3Vector vertices[4];
    Nexus_V1_DgnStructure3Vector normal;
    int vertex_slot_count;
    int source_geometry_bound;
    int transform_semantics_proven;
    int material_semantics_proven;
    int pixel_palette_vdp1_semantics_proven;
    int decoder_permitted;
    int no_draw_only;
    int fallback_visuals_permitted;
    int blocks_real_dgn_mesh_render;
} Nexus_V1_DgnStructure1FDirectMeshGeometryPacket;

/* Source-owned transform capture target for one direct mesh owner. The
 * Structure1A selector table and raw owner rotation are retained together
 * with exact geometry for a future Saturn trace; they are not a matrix,
 * coordinate conversion, or host placement rule. */
typedef struct {
    int valid;
    Nexus_V1_DgnStructure1FDirectMeshGeometryPacket geometry;
    Nexus_V1_DgnStructure1ATransformTableReceipt transform_table;
    Nexus_V1_DgnStructure1ATransformSelectorReceipt transform_selectors;
    int transform_table_source_bound;
    int owner_transform_selector_source_bound;
    int capture_producer_required;
    int original_saturn_capture_required;
    int transform_semantics_proven;
    int no_draw_only;
    int fallback_visuals_permitted;
    int blocks_real_dgn_mesh_render;
} Nexus_V1_DgnStructure1FTransformCaptureTarget;

/* File format emitted for one source-bound direct Structure1F face. It is a
 * capture-producer request, not an executable transform or renderer input. */
#define NEXUS_V1_STRUCTURE1F_DIRECT_FACE_CAPTURE_TARGET_MAGIC \
    "FIRESTAFF_NEXUS_STRUCTURE1F_DIRECT_FACE_CAPTURE_TARGET_V1"

/* Package/host-side verification for an emitted direct-face request. This
 * proves only that the manifest still names the loaded canonical bytes; it
 * cannot promote an observation into a transform, material, or draw. */
typedef enum {
    NEXUS_V1_STRUCTURE1F_DIRECT_FACE_CAPTURE_MANIFEST_MISSING = 0,
    NEXUS_V1_STRUCTURE1F_DIRECT_FACE_CAPTURE_MANIFEST_BLOCKED_MALFORMED = 1,
    NEXUS_V1_STRUCTURE1F_DIRECT_FACE_CAPTURE_MANIFEST_BLOCKED_TARGET_MISMATCH = 2,
    NEXUS_V1_STRUCTURE1F_DIRECT_FACE_CAPTURE_MANIFEST_ACCEPTED_NO_DRAW = 3
} Nexus_V1_DgnStructure1FDirectFaceCaptureManifestStatus;

typedef struct {
    Nexus_V1_DgnStructure1FDirectFaceCaptureManifestStatus status;
    int package_bytes_bound;
    int manifest_target_bound;
    int owner_geometry_bound;
    int transform_selectors_bound;
    int original_saturn_capture_required;
    int no_draw_only;
    int fallback_visuals_permitted;
    int blocks_real_dgn_mesh_render;
} Nexus_V1_DgnStructure1FDirectFaceCaptureManifestReceipt;

/* Joins a verified direct Structure1F face request to one independently
 * authenticated six-lane Saturn capture. The join is deliberately opaque:
 * it proves only that the capture manifest names the exact package face.
 * Texture, palette, VDP1 command, transform, and draw semantics remain
 * unavailable until their formats are separately proven. */
typedef enum {
    NEXUS_V1_STRUCTURE1F_DIRECT_FACE_RAW_CAPTURE_MISSING = 0,
    NEXUS_V1_STRUCTURE1F_DIRECT_FACE_RAW_CAPTURE_BLOCKED_DIRECT_FACE = 1,
    NEXUS_V1_STRUCTURE1F_DIRECT_FACE_RAW_CAPTURE_BLOCKED_CAPTURE = 2,
    NEXUS_V1_STRUCTURE1F_DIRECT_FACE_RAW_CAPTURE_BLOCKED_FACE_MISMATCH = 3,
    NEXUS_V1_STRUCTURE1F_DIRECT_FACE_RAW_CAPTURE_ACCEPTED_OPAQUE = 4
} Nexus_V1_DgnStructure1FDirectFaceRawCaptureStatus;

typedef struct {
    Nexus_V1_DgnStructure1FDirectFaceRawCaptureStatus status;
    Nexus_V1_DgnStructure1FDirectFaceCaptureManifestReceipt direct_face;
    int raw_capture_authenticated;
    int raw_capture_source_bound;
    int direct_face_candidate_bound;
    int texture_lane_bound;
    int palette_lane_bound;
    int vdp1_state_lane_bound;
    int transform_lane_bound;
    int normal_culling_lane_bound;
    int vdp1_command_lane_bound;
    int no_draw_only;
    int fallback_visuals_permitted;
    int blocks_real_dgn_mesh_render;
} Nexus_V1_DgnStructure1FDirectFaceRawCaptureReceipt;

/* Exact no-draw join for one direct Structure1F face and its authenticated
 * VDP1 command, texture-VRAM window, and palette lane. CMDCOLR is retained as
 * an opaque source word: this receipt does not select a CRAM entry, unpack a
 * texel, or authorize a renderer. */
typedef enum {
    NEXUS_V1_STRUCTURE1F_VDP1_MATERIAL_MISSING = 0,
    NEXUS_V1_STRUCTURE1F_VDP1_MATERIAL_BLOCKED_DIRECT_CAPTURE = 1,
    NEXUS_V1_STRUCTURE1F_VDP1_MATERIAL_BLOCKED_RUNTIME_CAPTURE = 2,
    NEXUS_V1_STRUCTURE1F_VDP1_MATERIAL_BLOCKED_VDP1_LINK = 3,
    NEXUS_V1_STRUCTURE1F_VDP1_MATERIAL_ACCEPTED_OPAQUE = 4
} Nexus_V1_DgnStructure1FVdp1MaterialStatus;

typedef struct {
    Nexus_V1_DgnStructure1FVdp1MaterialStatus status;
    Nexus_V1_DgnStructure1FDirectFaceRawCaptureReceipt direct_capture;
    Nexus_V1_DgnStructure3Vdp1VramWindowReceipt texture_vram;
    Nexus_V1_DgnStructure3Vdp1CommandVramReceipt command_vram;
    uint16_t command_colour_control;
    uint64_t texture_lane_fnv1a64;
    uint64_t palette_lane_fnv1a64;
    int runtime_lanes_match_authenticated_capture;
    int texture_command_vram_bound;
    int palette_lane_bound;
    int command_colour_control_bound;
    int pixel_decode_proven;
    int palette_decode_proven;
    int decoder_permitted;
    int no_draw_only;
    int fallback_visuals_permitted;
    int blocks_real_dgn_mesh_render;
} Nexus_V1_DgnStructure1FVdp1MaterialReceipt;

typedef struct {
    Nexus_V1_DgnStructure1FVdp1MaterialReceipt material;
    Nexus_V1_Vdp1LookupDecodeReceipt lookup;
    int direct_face_capture_bound;
    int lookup_colour_codes_bound;
    int pixel_output_witness_verified;
    int no_draw_only;
    int fallback_visuals_permitted;
    int blocks_real_dgn_mesh_render;
} Nexus_V1_DgnStructure1FVdp1LookupDecodeReceipt;

/* Admission for a future original-Saturn transform observation of one direct
 * Structure1F owner. It binds source bytes and captured state identity only;
 * it never interprets transform words or authorizes a draw. */
#define NEXUS_V1_STRUCTURE1F_TRANSFORM_TRACE_MAGIC \
    "FIRESTAFF_NEXUS_STRUCTURE1F_TRANSFORM_TRACE_V1"
typedef enum {
    NEXUS_V1_STRUCTURE1F_TRANSFORM_TRACE_MISSING = 0,
    NEXUS_V1_STRUCTURE1F_TRANSFORM_TRACE_BLOCKED_MALFORMED = 1,
    NEXUS_V1_STRUCTURE1F_TRANSFORM_TRACE_BLOCKED_TARGET_MISMATCH = 2,
    NEXUS_V1_STRUCTURE1F_TRANSFORM_TRACE_BLOCKED_OBSERVATIONS = 3,
    NEXUS_V1_STRUCTURE1F_TRANSFORM_TRACE_BLOCKED_PROVENANCE = 4,
    NEXUS_V1_STRUCTURE1F_TRANSFORM_TRACE_ADMITTED_OPAQUE = 5
} Nexus_V1_DgnStructure1FTransformTraceStatus;

typedef struct {
    Nexus_V1_DgnStructure1FTransformTraceStatus status;
    int capture_target_bound;
    int manifest_target_bound;
    int raw_trace_bytes_bound;
    uint64_t raw_trace_fnv1a64;
    size_t raw_trace_byte_count;
    int transform_state_bytes_bound;
    uint64_t transform_state_fnv1a64;
    size_t transform_state_byte_count;
    int original_saturn_capture_verified;
    int opaque_trace_admitted;
    int transform_semantics_proven;
    int no_draw_only;
    int fallback_visuals_permitted;
    int blocks_real_dgn_mesh_render;
} Nexus_V1_DgnStructure1FTransformTraceAdmissionReceipt;

/* External files for one direct-owner Saturn transform observation. The
 * source-bound capture target is distinct from the debugger trace manifest,
 * execution lane, and transform snapshot. */
typedef struct {
    const char *capture_target_path;
    const char *manifest_path;
    const char *raw_trace_path;
    const char *transform_state_path;
    const char *attestation_path;
} Nexus_V1_DgnStructure1FTransformTracePaths;

/* Attestation is parsed from an independently produced sidecar. Firestaff
 * validates its binding but cannot create this assertion from trace bytes. */
#define NEXUS_V1_STRUCTURE1F_TRANSFORM_ATTESTATION_MAGIC \
    "FIRESTAFF_NEXUS_STRUCTURE1F_TRANSFORM_ATTESTATION_V1"
typedef enum {
    NEXUS_V1_STRUCTURE1F_TRANSFORM_ATTESTATION_MISSING = 0,
    NEXUS_V1_STRUCTURE1F_TRANSFORM_ATTESTATION_BLOCKED_MALFORMED = 1,
    NEXUS_V1_STRUCTURE1F_TRANSFORM_ATTESTATION_BLOCKED_TARGET_MISMATCH = 2,
    NEXUS_V1_STRUCTURE1F_TRANSFORM_ATTESTATION_BLOCKED_SIDECARS = 3,
    NEXUS_V1_STRUCTURE1F_TRANSFORM_ATTESTATION_ADMITTED_OPAQUE = 4
} Nexus_V1_DgnStructure1FTransformAttestationStatus;

typedef struct {
    Nexus_V1_DgnStructure1FTransformAttestationStatus status;
    int capture_target_bound;
    int manifest_target_bound;
    int raw_trace_bound;
    int transform_state_bound;
    int attestation_sha256_present;
    int independent_saturn_review_declared;
    int original_saturn_source_attested;
    int transform_semantics_proven;
    int no_draw_only;
    int fallback_visuals_permitted;
    int blocks_real_dgn_mesh_render;
} Nexus_V1_DgnStructure1FTransformTraceAttestationReceipt;

typedef struct {
    int sidecar_paths_distinct;
    int capture_target_bytes_read;
    int manifest_bytes_read;
    int raw_trace_bytes_read;
    int transform_state_bytes_read;
    int attestation_bytes_read;
    Nexus_V1_DgnStructure1FDirectFaceCaptureManifestReceipt capture_target;
    Nexus_V1_DgnStructure1FTransformTraceAttestationReceipt attestation;
    Nexus_V1_DgnStructure1FTransformTraceAdmissionReceipt admission;
    int no_draw_only;
    int fallback_visuals_permitted;
    int blocks_real_dgn_mesh_render;
} Nexus_V1_DgnStructure1FTransformTraceFileIntakeReceipt;

/* One direct Structure1F owner joined to the exact static Structure3 face
 * material target selected by the same documented Structure1A model/face
 * fields. This is capture provenance only: the Structure2 payload remains
 * opaque and no transform, palette, VDP1, or draw semantics are inferred. */
typedef struct {
    int valid;
    Nexus_V1_DgnStructure1FDirectMeshBindingReceipt direct_mesh;
    Nexus_V1_DgnStructure3StaticMaterialCaptureTarget static_material;
    int direct_face_material_bound;
    int capture_producer_required;
    int original_saturn_capture_required;
    int no_draw_only;
    int fallback_visuals_permitted;
    int blocks_real_dgn_mesh_render;
} Nexus_V1_DgnStructure1FDirectStaticMaterialCaptureTarget;

/* One source-faithful dungeon face subject joining the parser-observed
 * Structure1F row, its Structure1A owner/selector, the selected Structure3
 * face, and the active bounded Structure2 source envelope. The receipt remains
 * M11 inspection provenance only: it authorizes no transform, mesh, texture,
 * palette, or drawing semantics. */
typedef struct {
    int valid;
    Nexus_V1_DgnStructure1FSourcePacket structure1f_source;
    Nexus_V1_DgnStructure1FTransformCaptureTarget transform_target;
    Nexus_V1_DgnStructure2SourceReceipt structure2_source;
    int structure1f_record_source_bound;
    int face_mesh_adjacency_bound;
    int structure2_source_envelope_bound;
    int owner_transform_selector_bound;
    int transform_semantics_proven;
    int no_draw_only;
    int fallback_visuals_permitted;
    int blocks_real_dgn_mesh_render;
} Nexus_V1_DgnStructure1F2FaceAdjacencyTransformReceipt;

/* Exact parser-observed DGN container and Structure1F descriptor envelope for
 * one direct LEV route. The header and table remain opaque byte spans: this
 * records bounds/counts/identity only and grants no geometry or draw claim. */
typedef struct {
    int valid;
    uint32_t level_index;
    uint32_t header_offset;
    uint32_t header_length;
    uint64_t header_fnv1a64;
    uint32_t descriptor_offset;
    uint32_t descriptor_length;
    uint32_t descriptor_count;
    uint64_t descriptor_fnv1a64;
    uint64_t package_fnv1a64;
    int no_draw_only;
    int fallback_visuals_permitted;
    int blocks_real_dgn_mesh_render;
} Nexus_V1_DgnDirectLevHeaderDescriptorProvenance;

/* M11's selected parser-observed Structure1F row. The face and Structure2
 * fields are a bounded source-reference join only: neither selects mesh or
 * material semantics, and no pixel/palette/render path may consume it. */
typedef struct {
    int valid;
    uint32_t level_index;
    uint64_t route_epoch;
    uint64_t package_fnv1a64;
    int structure1f_entry_index;
    uint32_t descriptor_offset;
    uint32_t descriptor_length;
    uint64_t descriptor_fnv1a64;
    uint8_t structure3_model_index;
    uint8_t face_ordinal;
    int face_mesh_reference_bound;
    int material_reference_opaque;
    int no_draw_only;
    int fallback_visuals_permitted;
    int blocks_real_dgn_mesh_render;
} Nexus_V1_DgnM11Structure1FDescriptorIntakeReceipt;

/* The selected Structure1F row's already source-bound static Structure3 face
 * and Structure2 descriptor. Candidate image/palette intervals are opaque
 * capture windows, never texture/pixel/palette semantics. */
typedef struct {
    int valid;
    uint32_t level_index;
    uint64_t route_epoch;
    uint64_t package_fnv1a64;
    int structure1f_entry_index;
    uint32_t structure3_entry_index;
    uint32_t face_ordinal;
    uint32_t face_offset;
    uint32_t face_length;
    uint64_t face_fnv1a64;
    int structure2_descriptor_index;
    uint32_t descriptor_offset;
    uint32_t descriptor_length;
    uint64_t descriptor_fnv1a64;
    uint32_t image_candidate_offset;
    uint32_t image_candidate_length;
    uint64_t image_candidate_fnv1a64;
    uint32_t palette_candidate_offset;
    uint32_t palette_candidate_length;
    uint64_t palette_candidate_fnv1a64;
    int face_descriptor_bound;
    int candidates_opaque;
    int no_draw_only;
    int fallback_visuals_permitted;
    int blocks_real_dgn_mesh_render;
} Nexus_V1_DgnM11Structure2FaceDescriptorIntakeReceipt;

/* The parser-proven Structure3b topology framing for M11's selected direct
 * Structure1F route. Face rows retain documented vertex-index incidence and
 * the paired normal row as bounded source spans only. This is capture input,
 * never a mesh, transform, material, or draw admission. */
typedef struct {
    int valid;
    uint32_t level_index;
    uint64_t route_epoch;
    uint64_t package_fnv1a64;
    int structure1f_entry_index;
    uint32_t structure3_entry_index;
    uint32_t face_ordinal;
    uint32_t face_offset;
    uint32_t face_length;
    uint64_t face_fnv1a64;
    uint32_t vertex_table_offset;
    uint32_t vertex_table_length;
    uint64_t vertex_table_fnv1a64;
    uint32_t vertex_count;
    uint32_t face_vertex_index_count;
    uint64_t referenced_vertex_rows_fnv1a64;
    uint32_t normal_offset;
    uint32_t normal_length;
    uint64_t normal_fnv1a64;
    int topology_framing_bound;
    int capture_required;
    int no_draw_only;
    int fallback_visuals_permitted;
    int blocks_real_dgn_mesh_render;
} Nexus_V1_DgnM11Structure3TopologyDescriptorIntakeReceipt;

/* Engine-owned M11 dungeon admission for one direct, hash-verified LEV
 * identity. The nested geometry receipt remains no-draw provenance only. */
typedef struct {
    int valid;
    int level_index;
    uint64_t route_epoch;
    char dgn_md5[33];
    uint64_t dgn_byte_count;
    uint64_t dgn_fnv1a64;
    Nexus_V1_DgnDirectLevHeaderDescriptorProvenance header_descriptor;
    Nexus_V1_DgnM11Structure1FDescriptorIntakeReceipt structure1f_descriptor;
    Nexus_V1_DgnM11Structure2FaceDescriptorIntakeReceipt
        structure2_face_descriptor;
    Nexus_V1_DgnM11Structure3TopologyDescriptorIntakeReceipt
        structure3_topology_descriptor;
    Nexus_V1_DgnStructure1F2FaceAdjacencyTransformReceipt geometry;
    int no_draw_only;
    int fallback_visuals_permitted;
    int blocks_real_dgn_mesh_render;
} Nexus_V1_DgnM11DirectLevNoDrawReceipt;

/* One direct Structure1F owner joined to its exact non-textured Structure3
 * face packet. The raw fill selector is kept opaque for capture correlation;
 * it is never promoted to a flat colour, palette entry, or draw command. */
typedef struct {
    int valid;
    Nexus_V1_DgnStructure1FDirectMeshBindingReceipt direct_mesh;
    Nexus_V1_DgnStructure3UntexturedFacePacket untextured_face;
    int direct_face_untextured_bound;
    int capture_producer_required;
    int original_saturn_capture_required;
    int no_draw_only;
    int fallback_visuals_permitted;
    int blocks_real_dgn_mesh_render;
} Nexus_V1_DgnStructure1FDirectUntexturedFaceCaptureTarget;

/* One direct Structure1F owner joined to its exact 08xx Structure3 material
 * declaration. Sequence execution and Structure2 decoding stay blocked; the
 * packet preserves only source-owned descriptor and instruction provenance. */
typedef struct {
    int valid;
    Nexus_V1_DgnStructure1FDirectMeshBindingReceipt direct_mesh;
    Nexus_V1_DgnStructure3AnimatedMaterialPacket animated_material;
    int direct_face_animated_material_bound;
    int capture_producer_required;
    int original_saturn_capture_required;
    int no_draw_only;
    int fallback_visuals_permitted;
    int blocks_real_dgn_mesh_render;
} Nexus_V1_DgnStructure1FDirectAnimatedMaterialCaptureTarget;

/* A static table observed in the hash-verified retail DM.BIN executable.
 * This is a capture-producer anchor only: it does not prove that the SH-2
 * writes either VDP1 register, emits a command list, selects DGN geometry,
 * or establishes transform/palette/pixel/draw semantics. */
typedef struct {
    Nexus_V1_LevelAuxSourceReceipt source;
    int source_byte_count;
    uint64_t source_bytes_fnv1a64;
    int table_offset;
    int table_occurrence_count;
    int vdp1_register_base_0x25d00000_observed;
    int vdp1_register_offset_0x10_observed;
    int static_vdp1_register_table_proven;
    int vdp1_command_emission_proven;
    int dgn_binding_proven;
    int no_draw_only;
    int fallback_visuals_permitted;
} Nexus_V1_DmBinVdp1RegisterTableReceipt;

/* Static SH-2 literal-load references into one retail DM.BIN VDP1 state map.
 * The VDP1-VRAM literal is only a command-storage candidate. This does not
 * establish execution, command emission, DGN ownership, palette, transform,
 * or draw semantics. */
typedef struct {
    Nexus_V1_LevelAuxSourceReceipt source;
    int source_byte_count;
    uint64_t source_bytes_fnv1a64;
    int table_offset;
    int table_occurrence_count;
    int sh2_pc_relative_literal_load_count;
    int vdp1_register_literal_load_count;
    int vdp1_vram_literal_load_count;
    int first_sh2_literal_load_offset;
    int last_sh2_literal_load_offset;
    int static_sh2_literal_loads_proven;
    int vdp1_vram_command_storage_candidate_proven;
    int vdp1_command_emission_proven;
    int transform_semantics_proven;
    int palette_semantics_proven;
    int no_draw_only;
    int fallback_visuals_permitted;
} Nexus_V1_DmBinVdp1StateRouteReceipt;

/* A bounded static SH-2 dataflow proof for three 16-bit VDP1 state writes.
 * It proves only original instruction/literal relationships in DM.BIN; it
 * does not establish that the path executes in a given frame or emits a VDP1
 * command, and cannot authorize palette, transform, or drawing behavior. */
typedef struct {
    Nexus_V1_LevelAuxSourceReceipt source;
    int source_byte_count;
    uint64_t source_bytes_fnv1a64;
    int code_window_offset;
    int state_table_offset;
    int static_instruction_dataflow_proven;
    int vdp1_register_0x04_write_proven;
    uint16_t vdp1_register_0x04_value;
    int vdp1_command_control_candidate_proven;
    int vdp1_vram_base_literal_offset;
    int vdp1_vram_base_load_offset;
    int vdp1_vram_base_r14_store_offset;
    int vdp1_vram_base_r14_store_proven;
    int vdp1_vram_command_list_proven;
    int vdp1_register_0x06_write_proven;
    uint16_t vdp1_register_0x06_value;
    int vdp1_register_0x08_write_proven;
    uint16_t vdp1_register_0x08_value;
    int vdp1_register_0x0a_write_proven;
    uint16_t vdp1_register_0x0a_value;
    int vdp1_command_emission_proven;
    int palette_semantics_proven;
    int transform_semantics_proven;
    int no_draw_only;
    int fallback_visuals_permitted;
} Nexus_V1_DmBinVdp1StateWriteReceipt;

/* Canonical Track 1 identity for a Nexus file name, or NULL when no original
 * identity is known. Discovery consumers must still hash the actual bytes. */
const char *nexus_v1_known_file_md5(const char *name);

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

/* Joins the active level's hash-bound SLEV/SAL/MAP/SDDRVS sources with their
 * bounded runtime profiles. It is deliberately a no-runtime boundary: task
 * dispatch, SAL decode, playback, and fallback visuals remain unavailable. */
typedef enum {
    NEXUS_V1_LEVEL_AUX_ADMISSION_MISSING = 0,
    NEXUS_V1_LEVEL_AUX_ADMISSION_BLOCKED_SOURCE = 1,
    NEXUS_V1_LEVEL_AUX_ADMISSION_BLOCKED_SCRIPT = 2,
    NEXUS_V1_LEVEL_AUX_ADMISSION_BLOCKED_SOUND = 3,
    NEXUS_V1_LEVEL_AUX_ADMISSION_READY_NO_RUNTIME = 4
} Nexus_V1_LevelAuxAdmissionStatus;

typedef struct {
    Nexus_V1_LevelAuxAdmissionStatus status;
    int level_index;
    int canonical_sources_bound;
    int slev_task_profile_bound;
    int sal_map_profile_bound;
    int sound_driver_bound;
    int blocks_real_script_dispatch;
    int blocks_real_sfx_playback;
    int no_runtime_only;
    int fallback_visuals_permitted;
} Nexus_V1_LevelAuxAdmissionReceipt;

/* Engine-owned handoff for one raw SNDLEV MAP selector. The selector remains
 * an opaque on-disk value: this only proves a unique, bounded SAL byte window
 * from the active hash-verified level pair. It is never a host SFX event and
 * does not imply a Saturn decoder, driver ABI, or playback operation. */
typedef enum {
    NEXUS_V1_LEVEL_SOUND_ROUTE_MISSING = 0,
    NEXUS_V1_LEVEL_SOUND_ROUTE_BLOCKED_SOURCE = 1,
    NEXUS_V1_LEVEL_SOUND_ROUTE_BLOCKED_SELECTOR = 2,
    NEXUS_V1_LEVEL_SOUND_ROUTE_BOUND_OPAQUE = 3
} Nexus_V1_LevelSoundRouteStatus;

typedef struct {
    Nexus_V1_LevelSoundRouteStatus status;
    int level_index;
    int raw_map_selector;
    int map_attribute;
    int sal_offset;
    int sal_size;
    int canonical_sal_source_verified;
    int canonical_map_source_verified;
    int canonical_sound_driver_source_verified;
    int map_window_unique_and_bounded;
    int saturn_event_dispatch_proven;
    int sal_decode_proven;
    int playback_permitted;
    int blocks_real_sfx_playback;
    int fallback_visuals_permitted;
} Nexus_V1_LevelSoundRouteReceipt;

/* A request for an authentic Saturn sound-driver capture of one bounded SAL
 * window. It is derived from the active, hash-verified SAL/MAP/driver route;
 * it never assigns the raw selector an event meaning or enables decoding. */
#define NEXUS_V1_SAL_CAPTURE_TARGET_MAGIC \
    "FIRESTAFF_NEXUS_SAL_SATURN_CAPTURE_TARGET_V1"
typedef struct {
    int valid;
    int level_index;
    char canonical_sal_name[16];
    char canonical_sal_md5[33];
    uint64_t canonical_sal_fnv1a64;
    char canonical_map_name[16];
    char canonical_map_md5[33];
    uint64_t canonical_map_fnv1a64;
    char canonical_driver_name[16];
    char canonical_driver_md5[33];
    int raw_map_selector;
    int map_attribute;
    int sal_offset;
    int sal_size;
    int original_saturn_driver_capture_required;
    int sal_decode_proven;
    int playback_permitted;
    int no_playback_only;
    int fallback_visuals_permitted;
} Nexus_V1_LevelSoundCaptureTargetReceipt;

/* Admission boundary for a future original-Saturn SDDRVS trace. A matching
 * trace is evidence only: it cannot assign MAP selector semantics, decode a
 * SAL window, or request host playback. */
#define NEXUS_V1_SAL_TRACE_MAGIC "FIRESTAFF_NEXUS_SAL_DRIVER_TRACE_V1"
typedef enum {
    NEXUS_V1_SAL_TRACE_MISSING = 0,
    NEXUS_V1_SAL_TRACE_BLOCKED_MALFORMED = 1,
    NEXUS_V1_SAL_TRACE_BLOCKED_TARGET_MISMATCH = 2,
    NEXUS_V1_SAL_TRACE_ADMITTED_OPAQUE = 3
} Nexus_V1_LevelSoundTraceStatus;

typedef struct {
    Nexus_V1_LevelSoundTraceStatus status;
    int level_index;
    int raw_map_selector;
    int map_attribute;
    int sal_offset;
    int sal_size;
    uint64_t canonical_sal_fnv1a64;
    uint64_t canonical_map_fnv1a64;
    int capture_target_bound;
    int mednafen_debugger_provenance;
    int original_saturn_execution_claimed;
    int trace_sha256_present;
    int raw_trace_bytes_bound;
    uint64_t raw_trace_fnv1a64;
    size_t raw_trace_byte_count;
    uint32_t selector_dispatch_pc;
    uint32_t sal_read_pc;
    uint32_t driver_output_pc;
    int trace_chain_complete;
    int driver_dispatch_proven;
    int sal_decode_proven;
    int playback_permitted;
    int blocks_real_sfx_playback;
    int fallback_visuals_permitted;
} Nexus_V1_LevelSoundTraceAdmissionReceipt;

typedef enum {
    NEXUS_V1_SAL_TRACE_HOST_MISSING = 0,
    NEXUS_V1_SAL_TRACE_HOST_BLOCKED_TRACE = 1,
    NEXUS_V1_SAL_TRACE_HOST_BLOCKED_ACTIVE_ROUTE = 2,
    NEXUS_V1_SAL_TRACE_HOST_CONSUMED_OPAQUE = 3
} Nexus_V1_LevelSoundTraceHostStatus;

typedef struct {
    Nexus_V1_LevelSoundTraceHostStatus status;
    int level_index;
    int active_sal_target_revalidated;
    int admitted_trace_bound;
    int raw_map_selector;
    int map_attribute;
    int sal_offset;
    int sal_size;
    uint32_t selector_dispatch_pc;
    uint32_t sal_read_pc;
    uint32_t driver_output_pc;
    int host_consumed;
    int driver_dispatch_proven;
    int sal_decode_proven;
    int playback_permitted;
    int blocks_real_sfx_playback;
    int fallback_visuals_permitted;
} Nexus_V1_LevelSoundTraceHostReceipt;

/* Raw-trace occurrence proof for the three observations requested by a SAL
 * capture target. It establishes occurrence and chronology only; it is not a
 * driver ABI, selector mapping, sample decoder, or playback permission. */
typedef enum {
    NEXUS_V1_SAL_DISPATCH_EVIDENCE_MISSING = 0,
    NEXUS_V1_SAL_DISPATCH_EVIDENCE_BLOCKED_RAW = 1,
    NEXUS_V1_SAL_DISPATCH_EVIDENCE_BLOCKED_OBSERVATION = 2,
    NEXUS_V1_SAL_DISPATCH_EVIDENCE_OBSERVED = 3
} Nexus_V1_SalDispatchEvidenceStatus;

typedef struct {
    Nexus_V1_SalDispatchEvidenceStatus status;
    int level_index;
    int raw_trace_bound;
    uint64_t raw_trace_fnv1a64;
    size_t raw_trace_byte_count;
    int selector_dispatch_observed;
    int sal_read_observed;
    int driver_output_observed;
    size_t selector_dispatch_raw_offset;
    size_t sal_read_raw_offset;
    size_t driver_output_raw_offset;
    int observation_order_proven;
    int driver_dispatch_proven;
    int sal_decode_proven;
    int playback_permitted;
    int blocks_real_sfx_playback;
    int fallback_visuals_permitted;
} Nexus_V1_SalDispatchEvidenceReceipt;

/* Active-level ownership for the corpus-verified SLEV SH-2 entry profile.
 * This binds only the observed entry framing and literal locations to the
 * engine's current level. It does not identify task-body opcodes, callback
 * targets, trigger semantics, or permission to dispatch a script. */
typedef enum {
    NEXUS_V1_LEVEL_SCRIPT_ROUTE_MISSING = 0,
    NEXUS_V1_LEVEL_SCRIPT_ROUTE_BLOCKED_SOURCE = 1,
    NEXUS_V1_LEVEL_SCRIPT_ROUTE_BLOCKED_PROFILE = 2,
    NEXUS_V1_LEVEL_SCRIPT_ROUTE_BOUND_TASK_PROFILE = 3
} Nexus_V1_LevelScriptRouteStatus;

typedef struct {
    Nexus_V1_LevelScriptRouteStatus status;
    int level_index;
    int canonical_slev_source_verified;
    int candidate_source_bytes;
    int task_header_size;
    int task_word_count;
    int first_opcode;
    int setup_immediate;
    Nexus_SlevSetupImmediateProvenance setup_immediate_provenance;
    int primary_literal_offset;
    int primary_literal_address;
    Nexus_SlevLiteralProvenance primary_literal_provenance;
    int auxiliary_literal_offset;
    int auxiliary_literal_address;
    Nexus_SlevLiteralProvenance auxiliary_literal_provenance;
    int task_header_profile_bound;
    int saturn_task_dispatch_proven;
    int dispatch_permitted;
    int blocks_real_script_dispatch;
    int fallback_visuals_permitted;
} Nexus_V1_LevelScriptRouteReceipt;

/* A producer request for an authentic Saturn SH-2 execution capture. It is
 * derived only from the active source-bound SLEV entry profile and names the
 * observations still needed to establish task-body dispatch semantics. */
#define NEXUS_V1_SLEV_CAPTURE_TARGET_MAGIC \
    "FIRESTAFF_NEXUS_SLEV_SH2_CAPTURE_TARGET_V1"

typedef struct {
    int valid;
    int level_index;
    char canonical_slev_name[16];
    char canonical_slev_md5[33];
    int source_byte_count;
    uint64_t source_fnv1a64;
    int task_header_size;
    int first_opcode;
    int setup_immediate;
    int primary_literal_offset;
    int primary_literal_address;
    int auxiliary_literal_offset;
    int auxiliary_literal_address;
    int original_saturn_execution_required;
    int task_body_dispatch_proven;
    int no_dispatch_only;
    int fallback_visuals_permitted;
} Nexus_V1_LevelScriptCaptureTargetReceipt;

/* Admission state for a trace produced by the supported debugger workflow.
 * A successful receipt only proves that the external trace names the active
 * SLEV target coherently. It is intentionally insufficient to execute any
 * task byte or host callback. */
#define NEXUS_V1_SLEV_TRACE_MAGIC "FIRESTAFF_NEXUS_SLEV_SH2_TRACE_V1"
typedef enum {
    NEXUS_V1_SLEV_TRACE_MISSING = 0,
    NEXUS_V1_SLEV_TRACE_BLOCKED_MALFORMED = 1,
    NEXUS_V1_SLEV_TRACE_BLOCKED_TARGET_MISMATCH = 2,
    NEXUS_V1_SLEV_TRACE_ADMITTED_OPAQUE = 3,
    NEXUS_V1_SLEV_TRACE_BLOCKED_RAW_TRACE = 4
} Nexus_V1_LevelScriptTraceStatus;

typedef struct {
    Nexus_V1_LevelScriptTraceStatus status;
    int level_index;
    int capture_target_bound;
    int mednafen_debugger_provenance;
    int original_saturn_execution_claimed;
    int trace_sha256_present;
    uint64_t source_fnv1a64;
    int raw_trace_bytes_bound;
    uint64_t raw_trace_fnv1a64;
    size_t raw_trace_byte_count;
    uint32_t entry_pc;
    uint32_t task_body_pc;
    uint32_t task_body_opcode;
    uint32_t callback_or_write_pc;
    int callback_or_write_is_write;
    int trace_chain_complete;
    int task_body_dispatch_proven;
    int dispatch_permitted;
    int blocks_real_script_dispatch;
    int fallback_visuals_permitted;
} Nexus_V1_LevelScriptTraceAdmissionReceipt;

/* Runtime ownership of an already-admitted trace. This is a host receipt
 * only: it revalidates the active SLEV target but never invokes the observed
 * task-body opcode or callback/write location. */
typedef enum {
    NEXUS_V1_SLEV_TRACE_HOST_MISSING = 0,
    NEXUS_V1_SLEV_TRACE_HOST_BLOCKED_TRACE = 1,
    NEXUS_V1_SLEV_TRACE_HOST_BLOCKED_ACTIVE_ROUTE = 2,
    NEXUS_V1_SLEV_TRACE_HOST_CONSUMED_OPAQUE = 3
} Nexus_V1_LevelScriptTraceHostStatus;

typedef struct {
    Nexus_V1_LevelScriptTraceHostStatus status;
    int level_index;
    int active_slev_target_revalidated;
    int admitted_trace_bound;
    uint32_t entry_pc;
    uint32_t task_body_pc;
    uint32_t task_body_opcode;
    uint32_t callback_or_write_pc;
    int callback_or_write_is_write;
    int host_consumed;
    int task_body_dispatch_proven;
    int dispatch_permitted;
    int blocks_real_script_dispatch;
    int fallback_visuals_permitted;
} Nexus_V1_LevelScriptTraceHostReceipt;

typedef enum {
    NEXUS_V1_SLEV_DISPATCH_EVIDENCE_MISSING = 0,
    NEXUS_V1_SLEV_DISPATCH_EVIDENCE_BLOCKED_RAW = 1,
    NEXUS_V1_SLEV_DISPATCH_EVIDENCE_BLOCKED_OBSERVATION = 2,
    NEXUS_V1_SLEV_DISPATCH_EVIDENCE_OBSERVED = 3
} Nexus_V1_SlevDispatchEvidenceStatus;

typedef struct {
    Nexus_V1_SlevDispatchEvidenceStatus status;
    int level_index;
    int raw_trace_bound;
    uint64_t raw_trace_fnv1a64;
    size_t raw_trace_byte_count;
    int entry_observed;
    int task_body_observed;
    int callback_or_write_observed;
    size_t entry_raw_offset;
    size_t task_body_raw_offset;
    size_t callback_or_write_raw_offset;
    int observation_order_proven;
    int primary_literal_observed;
    int auxiliary_literal_observed;
    size_t primary_literal_raw_offset;
    size_t auxiliary_literal_raw_offset;
    int literal_observation_proven;
    int callback_or_write_is_write;
    int task_body_dispatch_proven;
    int dispatch_permitted;
    int blocks_real_script_dispatch;
    int fallback_visuals_permitted;
} Nexus_V1_SlevDispatchEvidenceReceipt;

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
    int structure1g_floor_animation_cell_count;
    int structure1g_floor_animation_bound_count;
    int structure1g_image_instruction_count;
    int structure1g_goto_instruction_count;
    int structure1g_structure2_image_instruction_bound_count;
    int structure1g_structure2_image_instruction_unbound_count;
    int structure1g_structure2_first_image_bound_count;
    int structure2_valid_level_count;
    int structure2_texture_count;
    int structure2_payload_envelope_valid_level_count;
    int structure2_opaque_payload_byte_count;
    int structure2_nonzero_descriptor_offset_count;
    int structure2_descriptor_offsets_in_opaque_payload_count;
    int structure2_descriptor_offsets_outside_opaque_payload_count;
    int structure2_descriptor_offsets_word_bounded_count;
    int structure2_descriptor_offsets_unaligned_count;
    int structure2_descriptor_offset_unique_count;
    int structure2_descriptor_offset_reused_count;
    int structure2_local_payload_offset_pattern_level_count;
    int structure2_local_payload_word_aligned_offset_pattern_level_count;
    int structure2_local_payload_word_bounded_offset_pattern_level_count;
    int structure2_canonical_source_verified_level_count;
    int structure2_materialization_bound_level_count;
    int structure2_material_or_image_data_proven_level_count;
    int plan_ready;
    int static_mns_host_route_complete;
    Nexus_V1_DgnMaterialCategoryCoverageReceipt floor_coverage;
    Nexus_V1_DgnMaterialCategoryCoverageReceipt ceiling_coverage;
    Nexus_V1_DgnMaterialCategoryCoverageReceipt wall_coverage;
    Nexus_V1_DgnMaterialContainerReceipt floor_container;
    Nexus_V1_DgnMaterialContainerReceipt wall_container;
    Nexus_V1_DgnStructure3PayloadReceipt structure3_payloads[16];
    Nexus_V1_DgnStructure3DirectoryReceipt structure3_directories[16];
    Nexus_V1_DgnStructure3EntryHeaderReceipt structure3_entry_headers[16];
    Nexus_V1_DgnStructure3ModelReferenceReceipt
        structure3_model_references[16];
    Nexus_V1_DgnStructure1ATransformSelectorReceipt
        structure1a_transform_selectors[16];
    Nexus_V1_DgnStructure1FFaceSelectorReceipt
        structure1f_face_selectors[16];
    Nexus_V1_DgnStructure1FRotationSelectorReceipt
        structure1f_rotation_selectors[16];
    Nexus_V1_DgnStructure1FFaceRotationPairReceipt
        structure1f_face_rotation_pairs[16];
    Nexus_V1_DgnStructure1FOffsetPairReceipt
        structure1f_offset_pairs[16];
    Nexus_V1_DgnStructure1FWallPayloadSelectorReceipt
        structure1f_wall_payload_selectors[16];
    Nexus_V1_DgnStructure1FWallSensorDestinationReceipt
        structure1f_wall_sensor_destinations[16];
    Nexus_V1_DgnStructure1FWallSensorControlSelectorReceipt
        structure1f_wall_sensor_control_selectors[16];
    Nexus_V1_DgnStructure1FWallSensorControlDestinationTupleReceipt
        structure1f_wall_sensor_control_destination_tuples[16];
    Nexus_V1_DgnStructure1FWallSensorModelRotationPairReceipt
        structure1f_wall_sensor_model_rotation_pairs[16];
    Nexus_V1_DgnStructure1FWallDecorationModelRotationPairReceipt
        structure1f_wall_decoration_model_rotation_pairs[16];
    Nexus_V1_DgnStructure1FAlcovePayloadSelectorReceipt
        structure1f_alcove_payload_selectors[16];
    Nexus_V1_DgnStructure1FAlcovePayloadRotationPairReceipt
        structure1f_alcove_payload_rotation_pairs[16];
    Nexus_V1_DgnStructure1FFloorSensorControlSelectorReceipt
        structure1f_floor_sensor_control_selectors[16];
    Nexus_V1_DgnStructure1FFloorSensorDestinationReceipt
        structure1f_floor_sensor_destinations[16];
    Nexus_V1_DgnStructure1FFloorSensorModelRotationPairReceipt
        structure1f_floor_sensor_model_rotation_pairs[16];
    Nexus_V1_DgnStructure1FFloorSensorExtentPairReceipt
        structure1f_floor_sensor_extent_pairs[16];
    Nexus_V1_DgnStructure1FFloorDecorationPayloadSelectorReceipt
        structure1f_floor_decoration_payload_selectors[16];
    Nexus_V1_DgnStructure1FFloorDecorationRotationSelectorReceipt
        structure1f_floor_decoration_rotation_selectors[16];
    Nexus_V1_DgnStructure1FFloorDecorationModelRotationPairReceipt
        structure1f_floor_decoration_model_rotation_pairs[16];
    Nexus_V1_DgnStructure1FFloorDecorationOffsetPairReceipt
        structure1f_floor_decoration_offset_pairs[16];
    Nexus_V1_DgnStructure1FFloorDecorationControlExtentReceipt
        structure1f_floor_decoration_control_extents[16];
    Nexus_V1_DgnStructure1FItemAttributePairReceipt
        structure1f_item_attribute_pairs[16];
    Nexus_V1_DgnStructure1FItemLocationPairReceipt
        structure1f_item_location_pairs[16];
    Nexus_V1_DgnStructure1FItemCoordinatePairReceipt
        structure1f_item_coordinate_pairs[16];
    Nexus_V1_DgnStructure3OrdinalCorrelationReceipt
        structure3_ordinal_correlations[16];
    int structure3_payload_declared_level_count;
    int structure3_payload_valid_level_count;
    int structure3_payload_byte_count;
    int structure3_payload_nonzero_byte_count;
    int structure3_payload_transition_count;
    int structure3_nonzero_byte_run_count;
    int structure3_longest_nonzero_byte_run;
    int structure3_zero_block_count;
    int structure3_nonzero_block_count;
    int structure3_nonzero_block_run_count;
    int structure3_longest_nonzero_block_run;
    int structure3_directory_valid_level_count;
    int structure3_directory_entry_count;
    int structure3_entry_header_valid_level_count;
    int structure3_entry_header_entry_count;
    int structure3_entry_header_first_region_element_count;
    int structure3_entry_header_second_region_element_count;
    int structure3_model_reference_complete_level_count;
    int structure1a_transform_selector_complete_level_count;
    int structure1f_face_selector_complete_level_count;
    int structure1f_rotation_selector_complete_level_count;
    int structure1f_face_rotation_pair_complete_level_count;
    int structure1f_offset_pair_complete_level_count;
    int structure1f_wall_payload_selector_complete_level_count;
    int structure1f_wall_sensor_destination_complete_level_count;
    int structure1f_wall_sensor_control_selector_complete_level_count;
    int structure1f_wall_sensor_control_destination_tuple_complete_level_count;
    int structure1f_wall_sensor_model_rotation_pair_complete_level_count;
    int structure1f_wall_decoration_model_rotation_pair_complete_level_count;
    int structure1f_alcove_payload_selector_complete_level_count;
    int structure1f_alcove_payload_rotation_pair_complete_level_count;
    int structure1f_floor_sensor_control_selector_complete_level_count;
    int structure1f_floor_sensor_destination_complete_level_count;
    int structure1f_floor_sensor_model_rotation_pair_complete_level_count;
    int structure1f_floor_sensor_extent_pair_complete_level_count;
    int structure1f_floor_decoration_payload_selector_complete_level_count;
    int structure1f_floor_decoration_rotation_selector_complete_level_count;
    int structure1f_floor_decoration_model_rotation_pair_complete_level_count;
    int structure1f_floor_decoration_offset_pair_complete_level_count;
    int structure1f_floor_decoration_control_extent_complete_level_count;
    int structure1f_item_attribute_pair_complete_level_count;
    int structure1f_item_location_pair_complete_level_count;
    int structure1f_item_coordinate_pair_complete_level_count;
    int structure1f_owner_model_selector_complete_level_count;
    int structure3_zero_based_block_ordinal_mapping_disproven_level_count;
    int structure3_one_based_block_ordinal_mapping_disproven_level_count;
    int structure3_zero_based_byte_run_ordinal_mapping_disproven_level_count;
    int structure3_one_based_byte_run_ordinal_mapping_disproven_level_count;
    int structure3_zero_based_run_ordinal_mapping_disproven_level_count;
    int structure3_one_based_run_ordinal_mapping_disproven_level_count;
    int structure3_direct_block_ordinal_mapping_disproven_level_count;
    int structure3_direct_byte_run_ordinal_mapping_disproven_level_count;
    int structure3_direct_run_ordinal_mapping_disproven_level_count;
    int structure3_zero_based_directory_ordinal_mapping_disproven_level_count;
    int structure3_one_based_directory_ordinal_mapping_disproven_level_count;
    int structure3_direct_directory_ordinal_mapping_disproven_level_count;
    Nexus_V1_DgnStructure1FOwnerModelSelectorCorpusReceipt
        structure1f_owner_model_selectors[16];
    Nexus_V1_DgnStaticMaterialSourceReceipt static_mns_sources;
    Nexus_V1_DgnStructure2SourceReceipt structure2_sources[16];
    int bpk_host_routes_complete;
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
    /* Owned canonical bytes remain available for authenticated Structure3
     * capture consumption; they are discarded on replacement/shutdown. */
    uint8_t *current_level_dgn_data;
    int current_level_dgn_size;
    Nexus_V1_DgnStructure3RuntimeSource structure3_runtime_source;

    /* DGN material references resolve through these decoded DMDF banks. */
    Nexus_DMDFMaterialBank floor_materials;
    Nexus_DMDFMaterialBank wall_materials;
    Nexus_DMDFMaterialBank animated_floor_materials;
    Nexus_DMDFTextureSurface structure2_surfaces[NEXUS_DGN_MAX_STRUCTURE2_TEXTURES];
    int structure2_surface_count;
    Nexus_V1_DgnStructure2TextureDecodeReceipt structure2_decode_receipt;
    Nexus_V1_ItemIbsBank item_ibs_bank;
    Nexus_V1_ItemIbsRuntimeSourceReceipt item_ibs_runtime_source;
    Nexus_V1_DgnStructure2SourceReceipt current_level_structure2_source;
    int external_prs3_placement_valid;
    int external_prs3_placement_level;
    uint64_t external_prs3_placement_route_epoch;
    uint64_t external_prs3_replay_last_route_epoch;
    int external_saturn_save_capture_valid;
    uint64_t external_saturn_save_card_fnv1a64;
    uint64_t external_saturn_save_route_epoch;
    uint64_t external_saturn_save_last_route_epoch;
    int external_dgn_campaign_capture_ready;
    int external_dgn_campaign_capture_level;
    uint64_t external_dgn_campaign_capture_dgn_fnv1a64;
    uint64_t external_dgn_campaign_capture_trace_fnv1a64;
    uint32_t external_dgn_campaign_capture_trace_size;
    uint64_t external_dgn_campaign_capture_frame_sequence;
    uint64_t external_dgn_campaign_capture_command_sequence;
    uint64_t external_prs3_placement_dgn_fnv1a64;
    uint32_t external_prs3_placement_descriptor_index;
    uint64_t external_prs3_placement_frame_sequence;
    uint64_t external_prs3_placement_command_sequence;
    uint64_t external_prs3_placement_descriptor_fnv1a64;
    uint64_t external_prs3_placement_trace_fnv1a64;
    uint32_t external_prs3_placement_trace_size;
    uint64_t external_prs3_placement_header_fnv1a64;
    uint64_t external_prs3_placement_bitmap_fnv1a64;
    uint32_t external_prs3_placement_bitmap_offset;
    uint32_t external_prs3_placement_bitmap_size;
    uint64_t external_prs3_placement_palt_fnv1a64;
    uint32_t external_prs3_placement_palt_size;
    int external_structure1f_placement_valid;
    uint64_t external_structure1f_placement_dgn_fnv1a64;
    uint32_t external_structure1f_placement_descriptor_index;
    uint64_t external_structure1f_placement_frame_sequence;
    uint64_t external_structure1f_placement_command_sequence;
    uint64_t external_structure1f_placement_descriptor_fnv1a64;
    Nexus_V1_DgnStaticMaterialSourceReceipt dgn_static_material_sources;
    int floor_mns_material_route_valid;
    int wall_mns_material_route_valid;
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
    Nexus_V1_DgnMaterialPlan dgn_material_plan;
    Nexus_V1_DgnMaterialCorpusReceipt dgn_material_corpus;

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
    Nexus_V1_LevelAuxSourceReceipt menu_bpk_source;
    uint64_t menu_bpk_package_fnv1a64;
    int menu_bpk_prs3_execution_evidence_valid;
    Nexus_V1_MenuBpkPrs3ExecutionEvidenceReceipt
        menu_bpk_prs3_execution_evidence;
    int menu_bpk_decode_receipt_valid;
    int menu_bpk_decode_receipt_attempted;
    Nexus_V1_BpkRuntimeDecodeReceipt menu_bpk_decode_receipt;
    int menu_bpk_upload_receipt_valid;
    Nexus_V1_BpkRuntimeUploadReceipt menu_bpk_upload_receipt;
    Nexus_V1_BpkRuntimeUploadRow
        menu_bpk_upload_rows[NEXUS_V1_BPK_UPLOAD_PLAN_MAX_ROWS];
    int menu_bpk_upload_row_count;
    int menu_bpk_no_draw_host_valid;
    uint64_t menu_bpk_no_draw_host_route_epoch;
    uint64_t menu_bpk_no_draw_host_last_route_epoch;
    uint64_t menu_bpk_no_draw_host_package_fnv1a64;
    uint32_t menu_bpk_no_draw_host_entry_index;
    uint32_t menu_bpk_no_draw_host_payload_offset;
    uint32_t menu_bpk_no_draw_host_payload_size;
    uint64_t menu_bpk_no_draw_host_payload_fnv1a64;
    Nexus_V1_BpkPrs3CompressionDescriptorReceipt
        menu_bpk_no_draw_host_compression;
    int m11_direct_lev_dungeon_no_draw_valid;
    uint64_t m11_direct_lev_dungeon_route_epoch;
    uint64_t m11_direct_lev_dungeon_last_route_epoch;
    Nexus_V1_DgnM11DirectLevNoDrawReceipt m11_direct_lev_dungeon;
    Nexus_V1_MenuBpkPaltTraceAdmissionReceipt menu_bpk_palt_trace_admission;

    /* Per-level trigger/script runtime. SLEV*.BIN is real candidate data;
     * dispatch remains blocked until a source-locked parser exists. */
    Nexus_ScriptVM script_vm;
    Nexus_ScriptRuntimeReceipt script_runtime_receipt;
    Nexus_V1_LevelScriptTraceAdmissionReceipt script_trace_admission;
    Nexus_V1_LevelScriptTraceHostReceipt script_trace_host_receipt;
    Nexus_V1_SlevDispatchEvidenceReceipt script_dispatch_evidence;
    Nexus_V1_LevelSoundTraceAdmissionReceipt sound_trace_admission;
    Nexus_V1_LevelSoundTraceHostReceipt sound_trace_host_receipt;
    Nexus_V1_SalDispatchEvidenceReceipt sound_dispatch_evidence;
    Nexus_V1_LevelAuxRuntimeReceipt level_aux_runtime_receipt;
    Nexus_V1_LevelAuxSourceReceipt sound_driver_source;

    /* Creature manager */
    Nexus_V1_CreatureManager creatures;

    /* Projectile manager */
    Nexus_ProjectileManager projectiles;

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

/* Compares the exact DGN byte buffer selected by the launcher with the
 * canonical MD5 from the Saturn asset catalog. Callers must not substitute a
 * path-level lookup for this check before Structure3 binding. */
int nexus_v1_dgn_bytes_match_canonical_md5(
    const uint8_t *data, int size, const char *canonical_md5);

/* Preserve the exact loaded-LEV identity at the engine-facing mesh handoff.
 * A missing identity downgrades any otherwise ready receipt to no-draw; it
 * does not infer face, texture, palette, or raster semantics. */
void nexus_v1_dgn_renderer_handoff_require_canonical_source(
    Nexus_V1_DgnRendererHandoffReceipt *receipt,
    int canonical_source_verified);

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
/* Read one bounded Structure3 mesh entry from the exact canonical LEV bytes
 * currently owned by the engine. This is the renderer-facing source route:
 * it refuses a stale level, an unverified package, or mutated bytes before
 * delegating to the typed entry decoder. It establishes no Saturn transform,
 * texture, palette, VDP1, or draw semantics. */
int nexus_v1_current_level_extract_structure3_mesh_entry(
    const Nexus_V1_Engine *engine, int entry_index,
    Nexus_V1_DgnStructure3Vector *out_vertices, int max_vertices,
    Nexus_V1_DgnStructure3Face *out_faces, int max_faces,
    Nexus_V1_DgnStructure3Vector *out_normals, int max_normals,
    Nexus_V1_DgnStructure3MeshEntryReceipt *out_receipt);
/* Commit one already-bound original-capture face and its complete opaque
 * capture packet into engine-owned source storage. This is the renderer's
 * source-data route, not a decode or draw route. */
int nexus_v1_engine_consume_structure3_capture(
    Nexus_V1_Engine *engine,
    const Nexus_V1_DgnStructure3FaceCaptureCandidate *candidate,
    const Nexus_V1_DgnStructure3FaceCaptureBindingReceipt *binding,
    const Nexus_V1_DgnStructure3CaptureImport *capture);
/* Consume a verified external manifest and its six raw trace lanes through
 * the active canonical LEV route. Success stores only opaque, engine-owned
 * bytes and continues to block drawing until separate Saturn semantics exist. */
int nexus_v1_engine_consume_structure3_raw_capture_manifest(
    Nexus_V1_Engine *engine, const char *manifest_text, size_t manifest_size,
    const Nexus_V1_DgnStructure3RawCapturePaths *paths,
    const Nexus_V1_DgnStructure3RawCaptureAttestation *attestation,
    Nexus_V1_DgnStructure3RuntimeCaptureIntakeReceipt *out_receipt);
/* Revalidate a dual-source owner target against the active canonical LEV and
 * one already-attested runtime capture. Success stores source context only;
 * it explicitly leaves model-entry mapping and draw blocked. */
int nexus_v1_engine_bind_structure1a_structure3_runtime_correlation(
    Nexus_V1_Engine *engine,
    const Nexus_V1_DgnStructure1AStructure3CaptureTargetReceipt *target,
    Nexus_V1_DgnStructure1AStructure3RuntimeCorrelationReceipt *out_receipt);
/* Build one external capture request through the active package/host route.
 * The selected owner candidate and face are retained as original bytes only;
 * this never supplies a renderer input or a draw permission. */
int nexus_v1_engine_build_structure1a_structure3_capture_target(
    Nexus_V1_Engine *engine, int topology_candidate_index,
    uint32_t structure3_entry_index, uint32_t structure3_face_ordinal,
    Nexus_V1_DgnStructure1AStructure3CaptureTargetReceipt *out_target,
    Nexus_V1_DgnStructure1AStructure3CaptureTargetRouteReceipt *out_receipt);
/* Build and atomically write a producer request only through the active
 * canonical LEV route. The output names source facts, never decoded pixels. */
int nexus_v1_engine_write_structure1a_structure3_capture_target(
    Nexus_V1_Engine *engine, int topology_candidate_index,
    uint32_t structure3_entry_index, uint32_t structure3_face_ordinal,
    const char *path,
    Nexus_V1_DgnStructure1AStructure3CaptureTargetRouteReceipt *out_receipt);
/* Stages the engine-owned, already-bound Structure3 face for the viewport.
 * This is a source/geometry handoff only: a successful packet always remains
 * no-draw until independent Saturn render semantics are established. */
int nexus_v1_current_level_structure3_render_packet(
    const Nexus_V1_Engine *engine,
    Nexus_V1_DgnStructure3RenderPacket *out_packet);
/* Parse the complete VDP1 command lane from an already authenticated
 * Structure3 capture. This is command framing only and remains no-draw. */
int nexus_v1_current_level_structure3_vdp1_command_framing_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_DgnStructure3Vdp1CommandFramingReceipt *out_receipt);
/* Compare an authenticated full VDP1-VRAM snapshot with the command-local
 * texture window. This validates capture provenance only and stays no-draw. */
int nexus_v1_current_level_structure3_vdp1_vram_window_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_DgnStructure3Vdp1VramWindowReceipt *out_receipt);

int nexus_v1_current_level_structure3_vdp1_command_vram_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_DgnStructure3Vdp1CommandVramReceipt *out_receipt);
/* Bind the active canonical LEV byte receipt to the viewport boundary. A
 * valid receipt remains no-draw even when an authenticated opaque capture is
 * present: texture, palette, VDP1, transform, and culling semantics are not
 * inferred here. */
int nexus_v1_current_level_dgn_renderer_source_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_DgnActiveLevelRendererSourceReceipt *out_receipt);
int nexus_v1_current_level_structure3_directory_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_DgnActiveStructure3DirectoryReceipt *out_receipt);
int nexus_v1_current_level_structure3_mesh_semantic_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_DgnActiveStructure3MeshSemanticReceipt *out_receipt);
int nexus_v1_current_level_structure3_face_framing_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_DgnActiveStructure3FaceFramingReceipt *out_receipt);
int nexus_v1_current_level_structure3_face_material_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_DgnActiveStructure3FaceMaterialReceipt *out_receipt);
int nexus_v1_current_level_structure1a_owner_chain_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_DgnActiveStructure1AOwnerChainReceipt *out_receipt);
int nexus_v1_current_level_structure2_descriptor_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_DgnActiveStructure2DescriptorReceipt *out_receipt);
int nexus_v1_engine_build_structure2_descriptor_capture_target(
    const Nexus_V1_Engine *engine, int descriptor_index,
    Nexus_V1_DgnStructure2DescriptorCaptureTarget *out_target);
int nexus_v1_engine_write_structure2_descriptor_capture_target(
    const Nexus_V1_Engine *engine, int descriptor_index, const char *path,
    Nexus_V1_DgnStructure2DescriptorCaptureTarget *out_target);
int nexus_v1_current_level_structure2_format_evidence_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_DgnStructure2FormatEvidenceReceipt *out_receipt);
uint32_t nexus_v1_saturn_15bit_to_rgba(uint16_t color);
int nexus_v1_current_level_decode_structure2_textures(
    const Nexus_V1_Engine *engine,
    Nexus_DMDFTextureSurface *out_surfaces, int max_surfaces,
    Nexus_V1_DgnStructure2TextureDecodeReceipt *out_receipt);
int nexus_v1_engine_build_structure3_static_material_capture_target(
    const Nexus_V1_Engine *engine, uint32_t structure3_entry_index,
    uint32_t face_ordinal,
    Nexus_V1_DgnStructure3StaticMaterialCaptureTarget *out_target);
/* Bundle an independently selected Structure1F/1A owner and one static
 * Structure3 material face for a real capture producer. No mapping between
 * the owner model index and entry index is inferred. */
int nexus_v1_engine_build_structure1a_structure3_material_capture_target(
    Nexus_V1_Engine *engine, int topology_candidate_index,
    uint32_t structure3_entry_index, uint32_t structure3_face_ordinal,
    Nexus_V1_DgnStructure1AStructure3MaterialCaptureTarget *out_target,
    Nexus_V1_DgnStructure1AStructure3CaptureTargetRouteReceipt *out_receipt);
/* Write one atomic external-Saturn capture request that preserves the owner,
 * typed Structure3 face, and bounded Structure2 material lanes. It requests
 * observations only and cannot become a decoder or renderer input. */
int nexus_v1_engine_write_structure1a_structure3_material_capture_target(
    Nexus_V1_Engine *engine, int topology_candidate_index,
    uint32_t structure3_entry_index, uint32_t structure3_face_ordinal,
    const char *path,
    Nexus_V1_DgnStructure1AStructure3MaterialCaptureTarget *out_target,
    Nexus_V1_DgnStructure1AStructure3CaptureTargetRouteReceipt *out_receipt);
/* Joins an exact typed Structure3 face/vertices/normal extraction to the
 * source-bound static Structure2 descriptor target. The returned package
 * packet is deliberately no-draw: it has no inferred transform, texel,
 * palette, VDP1, or fallback-visual semantics. */
int nexus_v1_current_level_structure3_package_geometry_packet(
    const Nexus_V1_Engine *engine, uint32_t structure3_entry_index,
    uint32_t face_ordinal,
    Nexus_V1_DgnStructure3PackageGeometryPacket *out_packet);
/* Traverse every source-bound static-textured Structure3 face in the active
 * canonical LEV. The consumer receives only no-draw package geometry packets;
 * dynamic/unknown material faces remain outside this route. */
int nexus_v1_current_level_visit_structure3_package_geometry(
    const Nexus_V1_Engine *engine,
    Nexus_V1_DgnStructure3PackageGeometryConsumer consumer, void *context,
    Nexus_V1_DgnStructure3PackageGeometrySceneReceipt *out_receipt);
/* Build one source-bound 08xx animated-face packet from the active canonical
 * LEV. It exposes only the original Structure1G declaration and first bound
 * descriptor; sequence execution and all rendering semantics stay blocked. */
int nexus_v1_current_level_structure3_animated_material_packet(
    const Nexus_V1_Engine *engine, uint32_t structure3_entry_index,
    uint32_t face_ordinal,
    Nexus_V1_DgnStructure3AnimatedMaterialPacket *out_packet);
/* Traverse every 08xx texture-flagged Structure3 face through source-bound
 * Structure1G and first-Structure2 inputs. No sequence execution or drawing
 * is allowed by this traversal. */
int nexus_v1_current_level_visit_structure3_animated_materials(
    const Nexus_V1_Engine *engine,
    Nexus_V1_DgnStructure3AnimatedMaterialConsumer consumer, void *context,
    Nexus_V1_DgnStructure3AnimatedMaterialSceneReceipt *out_receipt);
/* Traverses every declared Structure1G image instruction for every active
 * 08xx Structure3 face. GOTO words are retained by the face packet but never
 * followed here, so this is source framing rather than animation playback. */
int nexus_v1_current_level_visit_structure3_animated_material_images(
    const Nexus_V1_Engine *engine,
    Nexus_V1_DgnStructure3AnimatedMaterialImageConsumer consumer,
    void *context,
    Nexus_V1_DgnStructure3AnimatedMaterialImageSceneReceipt *out_receipt);
/* Require every active Structure1G image declaration to resolve to its exact
 * bounded Structure2 image anchor (and its nonzero palette anchor). */
int nexus_v1_current_level_visit_structure3_animated_material_payload_anchors(
    const Nexus_V1_Engine *engine,
    Nexus_V1_DgnStructure3AnimatedMaterialPayloadSceneReceipt *out_receipt);
/* Build one source-bound non-textured Structure3 face packet. Raw fill bytes
 * are retained solely for later capture correlation and cannot draw a colour. */
int nexus_v1_current_level_structure3_untextured_face_packet(
    const Nexus_V1_Engine *engine, uint32_t structure3_entry_index,
    uint32_t face_ordinal,
    Nexus_V1_DgnStructure3UntexturedFacePacket *out_packet);
/* Traverse every non-texture-flagged Structure3 face from the active
 * canonical LEV through source-only no-draw packets. */
int nexus_v1_current_level_visit_structure3_untextured_faces(
    const Nexus_V1_Engine *engine,
    Nexus_V1_DgnStructure3UntexturedFaceConsumer consumer, void *context,
    Nexus_V1_DgnStructure3UntexturedFaceSceneReceipt *out_receipt);
/* Require complete traversal coverage of static 00xx, animated 08xx, and
 * non-textured Structure3 face categories from one active canonical LEV. */
int nexus_v1_current_level_structure3_complete_source_scene_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_DgnStructure3CompleteSourceSceneReceipt *out_receipt);
/* Traverse all parsed Structure1F source rows from the active canonical LEV.
 * The packets are source transport only and never authorize object, trigger,
 * transform, material, texture, palette, pixel, or draw behavior. */
int nexus_v1_current_level_visit_structure1f_source_scene(
    const Nexus_V1_Engine *engine,
    Nexus_V1_DgnStructure1FSourceConsumer consumer, void *context,
    Nexus_V1_DgnStructure1FSourceSceneReceipt *out_receipt);

/* Resolve one active Structure1F row through the complete canonical source
 * scene. The returned row is source provenance for later mesh work only. */
int nexus_v1_current_level_lookup_structure1f_source_entry(
    const Nexus_V1_Engine *engine, int entry_index,
    Nexus_V1_DgnStructure1FSourcePacket *out_packet);
/* Traverse every addressable bounded Structure1C source record from the
 * canonical active LEV without assigning the record bytes a collision grammar. */
int nexus_v1_current_level_visit_structure1c_source_scene(
    const Nexus_V1_Engine *engine,
    Nexus_V1_DgnStructure1CSourceConsumer consumer, void *context,
    Nexus_V1_DgnStructure1CSourceSceneReceipt *out_receipt);

int nexus_v1_current_level_lookup_structure1c_cell_source(
    const Nexus_V1_Engine *engine, int cell_x, int cell_y,
    Nexus_V1_DgnStructure1CCellSourcePacket *out_packet);
/* Traverse descriptor-owned Structure2 payload anchors from the active
 * canonical LEV. Candidate intervals are source bounds only, never decoded
 * texture or palette spans. */
int nexus_v1_current_level_visit_structure2_payload_anchors(
    const Nexus_V1_Engine *engine,
    Nexus_V1_DgnStructure2PayloadAnchorConsumer consumer, void *context,
    Nexus_V1_DgnStructure2PayloadAnchorSceneReceipt *out_receipt);
int nexus_v1_engine_admit_structure2_descriptor_capture_trace(
    const Nexus_V1_Engine *engine, int descriptor_index,
    const char *manifest_text, size_t manifest_size,
    const uint8_t *raw_trace, size_t raw_trace_size,
    int original_saturn_capture_verified,
    Nexus_V1_DgnStructure2TraceAdmissionReceipt *out_receipt);
/* Consume one external trace only after it identifies the active atomic
 * owner/face/material target and passes the exact Structure2 admission gate.
 * The trace stays opaque until independent format proof exists. */
int nexus_v1_engine_admit_structure1a_structure3_material_capture_trace(
    Nexus_V1_Engine *engine, int topology_candidate_index,
    uint32_t structure3_entry_index, uint32_t structure3_face_ordinal,
    const char *manifest_text, size_t manifest_size,
    const uint8_t *raw_trace, size_t raw_trace_size,
    int original_saturn_capture_verified,
    Nexus_V1_DgnOwnerMaterialTraceAdmissionReceipt *out_receipt);
int nexus_v1_current_level_transform_camera_framing_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_DgnActiveTransformCameraFramingReceipt *out_receipt);
int nexus_v1_current_level_structure1f_face_mesh_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_DgnActiveStructure1FFaceMeshReceipt *out_receipt);
/* Resolve one Structure1F owner directly through the documented Structure1A
 * model index and its face ordinal. The result remains a source/capture
 * receipt; Saturn rendering semantics are deliberately unavailable. */
int nexus_v1_engine_build_structure1f_direct_mesh_binding(
    const Nexus_V1_Engine *engine, int structure1f_entry_index,
    Nexus_V1_DgnStructure1FDirectMeshBindingReceipt *out_receipt);
int nexus_v1_engine_build_structure1f_direct_mesh_geometry_packet(
    const Nexus_V1_Engine *engine, int structure1f_entry_index,
    Nexus_V1_DgnStructure1FDirectMeshGeometryPacket *out_packet);
int nexus_v1_engine_build_structure1f_transform_capture_target(
    const Nexus_V1_Engine *engine, int structure1f_entry_index,
    Nexus_V1_DgnStructure1FTransformCaptureTarget *out_target);
int nexus_v1_engine_write_structure1f_direct_face_capture_target(
    const Nexus_V1_Engine *engine, int structure1f_entry_index,
    const char *path, Nexus_V1_DgnStructure1FTransformCaptureTarget *out_target);
int nexus_v1_engine_consume_structure1f_direct_face_capture_manifest(
    const Nexus_V1_Engine *engine, int structure1f_entry_index,
    const char *manifest_text, size_t manifest_size,
    Nexus_V1_DgnStructure1FDirectFaceCaptureManifestReceipt *out_receipt);
/* Bind an authenticated raw Structure3 capture only when its exact candidate
 * names the source-proved direct Structure1F owner face. Success is still an
 * opaque no-draw handoff, not a Saturn decoder or renderer admission. */
int nexus_v1_engine_bind_structure1f_direct_face_raw_capture(
    const Nexus_V1_Engine *engine, int structure1f_entry_index,
    const char *direct_manifest_text, size_t direct_manifest_size,
    const Nexus_V1_DgnStructure3RawCaptureHostReceipt *raw_capture,
    Nexus_V1_DgnStructure1FDirectFaceRawCaptureReceipt *out_receipt);
/* Revalidate the active runtime copy of an accepted direct-face capture
 * against its command, full VRAM texture window, and palette lane. This is an
 * opaque material linkage only; pixel and palette decoding remain blocked. */
int nexus_v1_engine_bind_structure1f_vdp1_material_capture(
    const Nexus_V1_Engine *engine, int structure1f_entry_index,
    const char *direct_manifest_text, size_t direct_manifest_size,
    const Nexus_V1_DgnStructure3RawCaptureHostReceipt *raw_capture,
    Nexus_V1_DgnStructure1FVdp1MaterialReceipt *out_receipt);
/* Decode one authenticated direct-face mode-1 lookup texture into raw VDP1
 * colour codes. The caller may compare a separately captured witness, but no
 * palette/CRAM conversion or draw is enabled by this routine. */
int nexus_v1_engine_decode_structure1f_vdp1_lookup_texture(
    const Nexus_V1_Engine *engine, int structure1f_entry_index,
    const char *direct_manifest_text, size_t direct_manifest_size,
    const Nexus_V1_DgnStructure3RawCaptureHostReceipt *raw_capture,
    uint16_t *out_colour_codes, size_t out_colour_code_count,
    const uint16_t *expected_colour_codes, size_t expected_colour_code_count,
    Nexus_V1_DgnStructure1FVdp1LookupDecodeReceipt *out_receipt);
int nexus_v1_engine_admit_structure1f_transform_capture_trace(
    const Nexus_V1_Engine *engine, int structure1f_entry_index,
    const char *manifest_text, size_t manifest_size,
    const uint8_t *raw_trace, size_t raw_trace_size,
    const uint8_t *transform_state, size_t transform_state_size,
    int original_saturn_capture_verified,
    Nexus_V1_DgnStructure1FTransformTraceAdmissionReceipt *out_receipt);
int nexus_v1_engine_parse_structure1f_transform_trace_attestation(
    const Nexus_V1_Engine *engine, int structure1f_entry_index,
    const char *attestation_text, size_t attestation_size,
    const uint8_t *raw_trace, size_t raw_trace_size,
    const uint8_t *transform_state, size_t transform_state_size,
    Nexus_V1_DgnStructure1FTransformTraceAttestationReceipt *out_receipt);
int nexus_v1_engine_ingest_structure1f_transform_capture_trace(
    const Nexus_V1_Engine *engine, int structure1f_entry_index,
    const Nexus_V1_DgnStructure1FTransformTracePaths *paths,
    Nexus_V1_DgnStructure1FTransformTraceFileIntakeReceipt *out_receipt);
/* Bind one direct Structure1F source owner to its exact static Structure2
 * material target. Non-static, untextured, or unresolved faces remain
 * unavailable rather than selecting a substitute material. */
int nexus_v1_engine_build_structure1f_direct_static_material_capture_target(
    const Nexus_V1_Engine *engine, int structure1f_entry_index,
    Nexus_V1_DgnStructure1FDirectStaticMaterialCaptureTarget *out_target);
int nexus_v1_engine_build_structure1f2_face_adjacency_transform_receipt(
    const Nexus_V1_Engine *engine, int structure1f_entry_index,
    Nexus_V1_DgnStructure1F2FaceAdjacencyTransformReceipt *out_receipt);
int nexus_v1_engine_consume_structure1f2_face_adjacency_transform_no_draw(
    const Nexus_V1_Engine *engine,
    const Nexus_V1_DgnStructure1F2FaceAdjacencyTransformReceipt *receipt);
/* Bind one direct Structure1F source owner to its exact non-textured face.
 * Textured faces remain outside this raw-fill route, and no flat fill or
 * substitute image is permitted. */
int nexus_v1_engine_build_structure1f_direct_untextured_face_capture_target(
    const Nexus_V1_Engine *engine, int structure1f_entry_index,
    Nexus_V1_DgnStructure1FDirectUntexturedFaceCaptureTarget *out_target);
/* Bind one direct Structure1F source owner to its exact 08xx material
 * declaration. Static and non-textured faces remain unavailable here; no
 * animation, palette, pixel, or fallback behavior is inferred. */
int nexus_v1_engine_build_structure1f_direct_animated_material_capture_target(
    const Nexus_V1_Engine *engine, int structure1f_entry_index,
    Nexus_V1_DgnStructure1FDirectAnimatedMaterialCaptureTarget *out_target);
/* Find the one known static VDP1 register table in canonical retail DM.BIN.
 * A successful receipt deliberately remains insufficient for rendering. */
int nexus_v1_engine_dm_bin_vdp1_register_table_receipt(
    Nexus_V1_Engine *engine,
    Nexus_V1_DmBinVdp1RegisterTableReceipt *out_receipt);
/* Bind the canonical executable's VDP1-state literals to its statically
 * decoded SH-2 PC-relative loads. This remains capture evidence, not a live
 * command or renderer route. */
int nexus_v1_engine_dm_bin_vdp1_state_route_receipt(
    Nexus_V1_Engine *engine,
    Nexus_V1_DmBinVdp1StateRouteReceipt *out_receipt);
/* Verify the original static instruction/literal dataflow for three VDP1
 * state-register stores. This is not a live command-emission claim. */
int nexus_v1_engine_dm_bin_vdp1_state_write_receipt(
    Nexus_V1_Engine *engine,
    Nexus_V1_DmBinVdp1StateWriteReceipt *out_receipt);
int nexus_v1_current_level_aux_runtime_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_LevelAuxRuntimeReceipt *out_receipt);
int nexus_v1_current_level_aux_admission_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_LevelAuxAdmissionReceipt *out_receipt);
int nexus_v1_current_level_sound_route_receipt(
    const Nexus_V1_Engine *engine, int raw_map_selector,
    Nexus_V1_LevelSoundRouteReceipt *out_receipt);
int nexus_v1_engine_build_sal_capture_target(
    const Nexus_V1_Engine *engine, int raw_map_selector,
    Nexus_V1_LevelSoundCaptureTargetReceipt *out_target);
int nexus_v1_engine_write_sal_capture_target(
    const Nexus_V1_Engine *engine, int raw_map_selector, const char *path,
    Nexus_V1_LevelSoundCaptureTargetReceipt *out_target);
int nexus_v1_engine_admit_sal_driver_trace(
    Nexus_V1_Engine *engine, const char *trace_text, size_t trace_size,
    Nexus_V1_LevelSoundTraceAdmissionReceipt *out_receipt);
int nexus_v1_engine_admit_sal_driver_trace_with_raw(
    Nexus_V1_Engine *engine, const char *trace_text, size_t trace_size,
    const uint8_t *raw_trace, size_t raw_trace_size,
    Nexus_V1_LevelSoundTraceAdmissionReceipt *out_receipt);
int nexus_v1_current_level_sal_trace_admission_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_LevelSoundTraceAdmissionReceipt *out_receipt);
int nexus_v1_engine_consume_sal_driver_trace(
    Nexus_V1_Engine *engine,
    Nexus_V1_LevelSoundTraceHostReceipt *out_receipt);
int nexus_v1_current_level_sal_trace_host_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_LevelSoundTraceHostReceipt *out_receipt);
int nexus_v1_build_sal_dispatch_evidence(
    Nexus_V1_Engine *engine, const uint8_t *raw_trace, size_t raw_trace_size,
    Nexus_V1_SalDispatchEvidenceReceipt *out_receipt);
int nexus_v1_current_level_script_route_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_LevelScriptRouteReceipt *out_receipt);
int nexus_v1_engine_build_slev_capture_target(
    const Nexus_V1_Engine *engine,
    Nexus_V1_LevelScriptCaptureTargetReceipt *out_target);
int nexus_v1_engine_write_slev_capture_target(
    const Nexus_V1_Engine *engine, const char *path,
    Nexus_V1_LevelScriptCaptureTargetReceipt *out_target);
int nexus_v1_engine_admit_slev_execution_trace(
    Nexus_V1_Engine *engine, const char *trace_text, size_t trace_size,
    Nexus_V1_LevelScriptTraceAdmissionReceipt *out_receipt);
int nexus_v1_engine_admit_slev_execution_trace_with_raw(
    Nexus_V1_Engine *engine, const char *trace_text, size_t trace_size,
    const uint8_t *raw_trace, size_t raw_trace_size,
    Nexus_V1_LevelScriptTraceAdmissionReceipt *out_receipt);
int nexus_v1_current_level_slev_trace_admission_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_LevelScriptTraceAdmissionReceipt *out_receipt);
int nexus_v1_engine_consume_slev_execution_trace(
    Nexus_V1_Engine *engine,
    Nexus_V1_LevelScriptTraceHostReceipt *out_receipt);
int nexus_v1_current_level_slev_trace_host_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_LevelScriptTraceHostReceipt *out_receipt);
int nexus_v1_build_slev_dispatch_evidence(
    Nexus_V1_Engine *engine, const uint8_t *raw_trace,
    size_t raw_trace_size, Nexus_V1_SlevDispatchEvidenceReceipt *out_receipt);
int nexus_v1_dgn_static_material_source_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_DgnStaticMaterialSourceReceipt *out_receipt);

/* Return the DGN plan whose commands and material surfaces have been checked
 * together for this level and party pose. The returned pointer is owned by
 * `engine` and remains valid until the next prepare/invalidate/shutdown. */
const Nexus_V1_DgnMaterialPlan *nexus_v1_prepare_dgn_material_plan(
    Nexus_V1_Engine *engine, int party_x, int party_y, int party_dir);
int nexus_v1_engine_set_external_prs3_placement_receipt(
    Nexus_V1_Engine *engine,
    const Nexus_V1_Prs3DgnPlacementAdapterReceipt *receipt);
int nexus_v1_engine_set_external_prs3_replay_placement_receipt(
    Nexus_V1_Engine *engine, uint64_t route_epoch,
    uint64_t expected_trace_fnv1a64, uint64_t expected_dgn_fnv1a64,
    uint64_t expected_bitmap_candidate_fnv1a64,
    const Nexus_V1_Prs3DgnPlacementAdapterReceipt *receipt);
int nexus_v1_engine_set_external_structure1f_placement_binding(
    Nexus_V1_Engine *engine,
    const Nexus_V1_Structure1FPlacementBindingReceipt *receipt);
/* Read-only admission evidence for an externally observed Structure1F
 * placement. This carries identity only; it never admits a material decoder
 * or a draw route. */
typedef struct {
    int valid;
    int no_draw_only;
    int blocks_real_dgn_mesh_render;
    int fallback_visuals_permitted;
    uint64_t dgn_fnv1a64;
    uint32_t descriptor_index;
    uint64_t frame_sequence;
    uint64_t command_sequence;
    uint64_t descriptor_fnv1a64;
} Nexus_V1_ExternalStructure1FPlacementReceipt;
int nexus_v1_current_external_structure1f_placement_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_ExternalStructure1FPlacementReceipt *out_receipt);
/* External capture evidence is scoped to one loaded DGN route. It is never
 * persisted in saves and must be cleared before a level reload or transition. */
void nexus_v1_engine_clear_external_prs3_placement_receipt(
    Nexus_V1_Engine *engine);
int nexus_v1_engine_set_saturn_save_capture_receipt(
    Nexus_V1_Engine *engine, uint64_t route_epoch,
    const struct Nexus_V1_SaturnSaveCaptureReceipt *receipt);
int nexus_v1_engine_saturn_save_capture_ready(const Nexus_V1_Engine *engine,
                                               uint64_t route_epoch,
                                               uint64_t card_fnv1a64);
int nexus_v1_engine_set_dgn_multi_level_capture_adjudication(
    Nexus_V1_Engine *engine,
    const struct Nexus_V1_DgnMultiLevelCaptureAdjudicationReceipt *receipt);
int nexus_v1_engine_current_level_dgn_capture_ready(
    const Nexus_V1_Engine *engine);
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
/* Source identity for MENU.BPK. A parseable archive is not eligible for the
 * retail menu route until this receipt is hash-verified. */
int nexus_v1_menu_bpk_source_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_LevelAuxSourceReceipt *out_receipt);
/* Returns the hash-bound direct/virtual source identity for WARNING.BIN.
 * Callers still must revalidate the bytes they present. */
int nexus_v1_warning_bin_source_receipt(
    Nexus_V1_Engine *engine,
    Nexus_V1_LevelAuxSourceReceipt *out_receipt);
/* Returns a source-bound DM.BIN/MENU.BPK loader receipt only. It never
 * supplies decoded pixels or opens a MENU.BPK render route. */
int nexus_v1_menu_bpk_prs3_execution_evidence_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_MenuBpkPrs3ExecutionEvidenceReceipt *out_receipt);
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
int nexus_v1_engine_set_menu_bpk_no_draw_host_receipt(
    Nexus_V1_Engine *engine, uint64_t route_epoch,
    uint64_t package_fnv1a64,
    const Nexus_V1_BpkRuntimeUploadReceipt *upload,
    const Nexus_V1_BpkRuntimeUploadRow *row);
int nexus_v1_engine_menu_bpk_no_draw_host_ready(
    const Nexus_V1_Engine *engine, uint64_t route_epoch,
    uint64_t package_fnv1a64);
int nexus_v1_engine_set_m11_direct_lev_dungeon_no_draw_receipt(
    Nexus_V1_Engine *engine, uint64_t route_epoch, int level_index,
    const char *dgn_md5, uint64_t dgn_byte_count, uint64_t dgn_fnv1a64,
    const Nexus_V1_DgnDirectLevHeaderDescriptorProvenance *header_descriptor,
    const Nexus_V1_DgnStructure1F2FaceAdjacencyTransformReceipt *geometry);
int nexus_v1_engine_m11_direct_lev_dungeon_no_draw_ready(
    const Nexus_V1_Engine *engine, uint64_t route_epoch, int level_index,
    const char *dgn_md5, uint64_t dgn_byte_count, uint64_t dgn_fnv1a64,
    Nexus_V1_DgnM11DirectLevNoDrawReceipt *out_receipt);
int nexus_v1_menu_bpk_renderer_handoff_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_MenuBpkRendererHandoffReceipt *out_receipt);
const char *nexus_v1_menu_bpk_renderer_handoff_status_name(
    Nexus_V1_MenuBpkRendererHandoffStatus status);
const char *nexus_v1_menu_bpk_prs3_prerequisite_status_name(
    Nexus_V1_MenuBpkPrs3PrerequisiteStatus status);
/* Short user-facing explanation for the same source-bound prerequisite.
 * It reports package/capture state only and never suggests a decoder exists. */
const char *nexus_v1_menu_bpk_prs3_prerequisite_message(
    Nexus_V1_MenuBpkPrs3PrerequisiteStatus status);
int nexus_v1_engine_build_menu_bpk_palt_capture_target(
    const Nexus_V1_Engine *engine,
    Nexus_V1_MenuBpkPaltCaptureTargetReceipt *out_target);
int nexus_v1_engine_menu_bpk_palt_warning_palette_correlation(
    Nexus_V1_Engine *engine,
    Nexus_V1_MenuBpkPaltWarningPaletteCorrelationReceipt *out_receipt);
int nexus_v1_engine_write_menu_bpk_palt_capture_target(
    const Nexus_V1_Engine *engine, const char *path,
    Nexus_V1_MenuBpkPaltCaptureTargetReceipt *out_target);
int nexus_v1_engine_admit_menu_bpk_palt_trace(
    Nexus_V1_Engine *engine, const char *manifest_text, size_t manifest_size,
    const uint8_t *raw_trace, size_t raw_trace_size,
    const uint8_t *palt_memory, size_t palt_memory_size,
    const uint8_t *palette_state, size_t palette_state_size,
    const uint8_t *vdp1_command, size_t vdp1_command_size,
    int original_saturn_capture_verified,
    Nexus_V1_MenuBpkPaltTraceAdmissionReceipt *out_receipt);
int nexus_v1_current_level_dgn_renderer_handoff_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_DgnRendererHandoffReceipt *out_receipt);
/* Returns the active level's source-locked Structure3 selector receipt.
 * Geometry readiness is derived from the restored Structure3 mesh
 * extractor over the exact MD5-authenticated retained buffer
 * (nexus_v1_level_structure3_mesh_geometry_ready), not from
 * level.geometry_info.mesh_ready, which gates collision/post-grid record
 * validation and stays 0 for the whole retail LEV00-LEV15 corpus. The
 * receipt stays capture-required and no-draw: can_submit_raster_input
 * remains 0 until an original Saturn VDP1 capture exists. */
int nexus_v1_current_level_dgn_face_material_source_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_DgnFaceMaterialReceipt *out_receipt);
int nexus_v1_current_level_structure2_source_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_DgnStructure2SourceReceipt *out_receipt);
int nexus_v1_dgn_static_material_source_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_DgnStaticMaterialSourceReceipt *out_receipt);
int nexus_v1_current_level_aux_runtime_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_LevelAuxRuntimeReceipt *out_receipt);
int nexus_v1_current_level_script_runtime_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_ScriptRuntimeReceipt *out_receipt);
int nexus_v1_current_level_sfx_runtime_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_SfxRuntimeReceipt *out_receipt);

#endif /* NEXUS_V1_ENGINE_H */

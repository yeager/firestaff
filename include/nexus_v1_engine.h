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
#include "nexus_v1_structure3_capture_manifest.h"
#include <stdint.h>

/* ── Constants ─────────────────────────────────────────────────────── */
#define NEXUS_MAX_MODELS 64
#define NEXUS_V1_DGN_RUNTIME_DIRECT_SOURCE_MAX 512

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
    /* Runtime-owned, no-draw Structure1G -> Structure2 source bindings for
     * animated floor commands. These preserve package provenance while the
     * original payload codec remains unavailable. */
    Nexus_V1_DgnStructure2FloorCommandSource
        structure2_floor_command_sources[NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS];
    Nexus_V1_DgnStructure2FloorCommandSourceReceipt
        structure2_floor_command_source_receipt;
    /* Exact authenticated LEVxx.DGN facts that admitted the Structure2
     * command-source receipt. They remain source ownership only. */
    int structure2_source_level_index;
    int structure2_source_canonical_hash_verified;
    int structure2_source_envelope_valid;
    int structure2_floor_command_sources_consumed;
    Nexus_V1_DgnStructure1FItemMaterialBinding
        structure1f_item_command_bindings[NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS];
    Nexus_V1_DgnStructure1FItemMaterialReceipt
        structure1f_item_command_binding_receipt;
    int structure1f_item_command_sources_consumed;
    /* Descriptor-0008 retains an authenticated packed 4bpp span and local
     * palette at its floor command. It stays no-draw until original VDP1
     * command provenance proves the texel order and placement. */
    Nexus_V1_DgnCommandPacked4BppMaterial
        structure1f_item_floor_materials[NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS];
    Nexus_V1_DgnCommandPacked4BppMaterialReceipt
        structure1f_item_floor_material_receipt;
    int structure1f_item_floor_materials_consumed;
    Nexus_V1_DgnStructure1FDirectFloorCommandSource
        structure1f_direct_floor_sources[NEXUS_V1_DGN_RUNTIME_DIRECT_SOURCE_MAX];
    Nexus_V1_DgnStructure1FDirectFloorCommandSourceReceipt
        structure1f_direct_floor_source_receipt;
    int structure1f_direct_floor_sources_consumed;
    /* Structure1A-owned alcove/wall rows retain their exact owner-cell anchor
     * in the runtime plan. They remain no-draw pending Saturn semantics. */
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
    int capture_producer_required;
    int original_saturn_capture_required;
    int no_draw_only;
    int fallback_visuals_permitted;
} Nexus_V1_DgnStructure2DescriptorCaptureTarget;

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
    int structure2_descriptor_offsets_word_bounded_count;
    /* Corpus-only layout measurements.  These deliberately describe numeric
     * descriptor targets, not any payload record or graphics meaning. */
    int structure2_descriptor_offsets_unaligned_count;
    int structure2_descriptor_offset_unique_count;
    int structure2_descriptor_offset_reused_count;
    int structure2_local_payload_offset_pattern_level_count;
    int structure2_local_payload_word_aligned_offset_pattern_level_count;
    int structure2_local_payload_word_bounded_offset_pattern_level_count;
    int structure2_material_or_image_data_proven_level_count;
    int structure2_canonical_source_verified_level_count;
    int structure2_materialization_bound_level_count;
    /* Per-level raw Structure3 payload observations. These only correlate the
     * documented bounded container span with Structure1A model references;
     * they do not assign any byte a face, vertex, mesh, texture, or pixel
     * meaning. */
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
    Nexus_V1_DgnStructure3PayloadReceipt structure3_payloads[16];
    Nexus_V1_DgnStructure3DirectoryReceipt structure3_directories[16];
    Nexus_V1_DgnStructure3EntryHeaderReceipt structure3_entry_headers[16];
    Nexus_V1_DgnStructure3ModelReferenceReceipt
        structure3_model_references[16];
    Nexus_V1_DgnStructure1ATransformSelectorReceipt
        structure1a_transform_selectors[16];
    Nexus_V1_DgnStructure1FFaceSelectorReceipt structure1f_face_selectors[16];
    Nexus_V1_DgnStructure1FRotationSelectorReceipt
        structure1f_rotation_selectors[16];
    Nexus_V1_DgnStructure1FFaceRotationPairReceipt
        structure1f_face_rotation_pairs[16];
    Nexus_V1_DgnStructure1FOffsetPairReceipt structure1f_offset_pairs[16];
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
    /* Owned canonical bytes remain available for authenticated Structure3
     * capture consumption; they are discarded on replacement/shutdown. */
    uint8_t *current_level_dgn_data;
    int current_level_dgn_size;
    Nexus_V1_DgnStructure3RuntimeSource structure3_runtime_source;

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
    Nexus_V1_ItemIbsBank item_ibs_bank;
    Nexus_V1_ItemIbsRuntimeSourceReceipt item_ibs_runtime_source;

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
    Nexus_V1_LevelScriptTraceAdmissionReceipt script_trace_admission;
    Nexus_V1_LevelScriptTraceHostReceipt script_trace_host_receipt;
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
int nexus_v1_current_level_transform_camera_framing_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_DgnActiveTransformCameraFramingReceipt *out_receipt);
int nexus_v1_current_level_structure1f_face_mesh_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_DgnActiveStructure1FFaceMeshReceipt *out_receipt);
int nexus_v1_current_level_aux_runtime_receipt(
    const Nexus_V1_Engine *engine,
    Nexus_V1_LevelAuxRuntimeReceipt *out_receipt);
int nexus_v1_current_level_sound_route_receipt(
    const Nexus_V1_Engine *engine, int raw_map_selector,
    Nexus_V1_LevelSoundRouteReceipt *out_receipt);
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
    const Nexus_V1_Engine *engine, const uint8_t *raw_trace,
    size_t raw_trace_size, Nexus_V1_SlevDispatchEvidenceReceipt *out_receipt);
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

#ifndef NEXUS_V1_DGN_FACE_MATERIAL_PROVENANCE_H
#define NEXUS_V1_DGN_FACE_MATERIAL_PROVENANCE_H

#include <stdint.h>

/*
 * Renderer-facing admission gate for already-decoded Structure3 face
 * selectors.  It deliberately does not parse DGN geometry or interpret a
 * selector as a texture, palette, or draw command.  Its job is to retain the
 * real-media identity through the point where a raster input is assembled.
 */

#define NEXUS_V1_DGN_FACE_MATERIAL_MAX_FACES 4096

typedef enum {
    NEXUS_V1_DGN_FACE_MATERIAL_SOURCE_NONE = 0,
    NEXUS_V1_DGN_FACE_MATERIAL_SOURCE_RETAIL_DGN = 1,
    NEXUS_V1_DGN_FACE_MATERIAL_SOURCE_SYNTHETIC = 2,
    NEXUS_V1_DGN_FACE_MATERIAL_SOURCE_DERIVED = 3
} Nexus_V1_DgnFaceMaterialSource;

typedef enum {
    NEXUS_V1_DGN_FACE_MATERIAL_SELECTOR_STATIC = 0,
    NEXUS_V1_DGN_FACE_MATERIAL_SELECTOR_ANIMATED = 1
} Nexus_V1_DgnFaceMaterialSelectorKind;

typedef struct {
    uint16_t face_ordinal;
    uint16_t material_selector;
    Nexus_V1_DgnFaceMaterialSelectorKind selector_kind;
} Nexus_V1_DgnFaceMaterialBinding;

typedef struct {
    Nexus_V1_DgnFaceMaterialSource source;
    /* Exact buffer reopened by the launcher for the active LEV entry. */
    const uint8_t *dgn_bytes;
    int dgn_size;
    /*
     * Canonical bytes retained by the hash-verified retail catalog.  The
     * face-binding route requires byte identity with the reopened buffer;
     * naming a source retail is not sufficient.
     */
    const uint8_t *canonical_dgn_bytes;
    int canonical_dgn_size;
    /* Set only by the caller that authenticated canonical_dgn_bytes. */
    int canonical_source_verified;
    const Nexus_V1_DgnFaceMaterialBinding *bindings;
    int face_count;
    /*
     * Bounded Structure2 descriptor count from the same canonical DGN.
     * Static Structure3 material selectors must resolve into this table
     * before the package/host handoff is considered source-bound.
     */
    int structure2_descriptor_count;
    int material_selector_count;
    /*
     * Renderer-neutral geometry receipt facts from the same Structure3 path.
     * This gate consumes only counts and closed render flags, not geometry
     * vertices or draw commands.  geometry_material_face_count is the number
     * of static/animated texture faces, not color-fill faces.
     */
    int geometry_source_bound;
    int geometry_material_face_count;
    int geometry_can_submit_geometry;
    int geometry_can_submit_textured_raster;
    int geometry_fallback_visuals_permitted;
} Nexus_V1_DgnFaceMaterialInput;

typedef enum {
    NEXUS_V1_DGN_FACE_MATERIAL_READY = 0,
    NEXUS_V1_DGN_FACE_MATERIAL_BLOCKED_SOURCE,
    NEXUS_V1_DGN_FACE_MATERIAL_BLOCKED_INPUT,
    NEXUS_V1_DGN_FACE_MATERIAL_BLOCKED_BINDING
} Nexus_V1_DgnFaceMaterialStatus;

typedef struct {
    Nexus_V1_DgnFaceMaterialStatus status;
    int canonical_source_verified;
    int reopened_bytes_match_canonical;
    int canonical_dgn_size;
    int face_count;
    int structure2_descriptor_count;
    int static_selector_count;
    int animated_selector_count;
    int geometry_source_bound;
    int geometry_material_face_count;
    int geometry_material_face_count_matches;
    int geometry_can_submit_geometry;
    int geometry_textured_raster_blocked;
    int structure3_mesh_materials_bound;
    int structure2_descriptor_route_bound;
    int selector_bindings_complete;
    int material_semantics_proven;
    int package_host_route_bound;
    int no_draw_only;
    int blocks_real_dgn_mesh_render;
    int permits_fallback_visuals;
    int original_saturn_capture_required;
    int original_saturn_capture_available;
    int can_submit_raster_input;
} Nexus_V1_DgnFaceMaterialReceipt;

typedef enum {
    NEXUS_V1_DGN_PACKAGE_HOST_CONSUMER_INVALID = 0,
    NEXUS_V1_DGN_PACKAGE_HOST_CONSUMER_BLOCKED_MATERIAL = 1,
    NEXUS_V1_DGN_PACKAGE_HOST_CONSUMER_BLOCKED_ROUTE = 2,
    NEXUS_V1_DGN_PACKAGE_HOST_CONSUMER_READY_NO_DRAW = 3
} Nexus_V1_DgnPackageHostConsumerStatus;

typedef struct {
    const Nexus_V1_DgnFaceMaterialReceipt *material_receipt;
    int host_route_requested;
    int package_route_consumed;
    int synthetic_material_route_requested;
    int expected_level_index;
    int observed_level_index;
    int expected_canonical_dgn_size;
    int observed_canonical_dgn_size;
    int expected_face_count;
    int observed_face_count;
    int expected_structure2_descriptor_count;
    int observed_structure2_descriptor_count;
} Nexus_V1_DgnPackageHostConsumerInput;

typedef struct {
    Nexus_V1_DgnPackageHostConsumerStatus status;
    int material_receipt_ready;
    int host_route_requested;
    int package_route_consumed;
    int level_index_matches;
    int canonical_dgn_size_matches;
    int face_count_matches;
    int structure2_descriptor_count_matches;
    int observed_level_index;
    int observed_canonical_dgn_size;
    int observed_face_count;
    int observed_structure2_descriptor_count;
    int static_selector_count;
    int animated_selector_count;
    int geometry_source_bound;
    int geometry_material_face_count;
    int geometry_can_submit_geometry;
    int geometry_textured_raster_blocked;
    int material_selector_counts_match_faces;
    int static_selectors_within_structure2_descriptors;
    int structure2_descriptor_route_bound;
    int selector_bindings_complete;
    int source_route_consumed_by_host;
    int real_dgn_source_consumed_by_host;
    int synthetic_material_route_rejected;
    int structure2_structure3_admission_bound;
    int package_host_route_bound;
    int original_saturn_capture_required;
    int original_saturn_rendering_proven;
    int material_semantics_proven;
    int material_pixel_promotion_blocked;
    int can_submit_raster_input;
    int fallback_visuals_permitted;
    int blocks_real_dgn_mesh_render;
    int no_draw_only;
} Nexus_V1_DgnPackageHostConsumerReceipt;

struct Nexus_V1_Prs3Vdp1ReviewedOutputUploadReceipt;

typedef struct {
    const Nexus_V1_DgnPackageHostConsumerReceipt *dgn_host;
    const struct Nexus_V1_Prs3Vdp1ReviewedOutputUploadReceipt
        *prs3_output_upload;
    int startup_route_requested;
    int dgn_route_requested;
} Nexus_V1_DgnMenuPrs3RouteInput;

typedef struct {
    int dgn_package_host_bound;
    int prs3_output_upload_bound;
    int startup_route_requested;
    int dgn_route_requested;
    int route_proof_bound;
    int original_saturn_capture_required;
    int independent_saturn_capture_required;
    int original_saturn_capture_authenticated;
    int reviewed_decoder_required;
    int runtime_dgn_render_permitted;
    int startup_menu_render_permitted;
    int material_pixel_promotion_blocked;
    int prs3_runtime_upload_blocked;
    int fallback_visuals_permitted;
    int no_draw_only;
    int blocks_real_dgn_mesh_render;
    int prs3_decoded_output_proof_bound;
    int prs3_decoded_output_sidecar_bound;
    int prs3_reviewed_upload_path_bound;
    int prs3_menu_bpk_upload_reviewed;
    int prs3_original_saturn_provenance_verified;
    int prs3_independent_authentication_required;
    int prs3_source_bound_no_runtime;
    int dgn_level_index;
    int dgn_canonical_dgn_size;
    int dgn_face_count;
    int dgn_structure2_descriptor_count;
    int dgn_static_selector_count;
    int dgn_animated_selector_count;
    int dgn_geometry_source_bound;
    int dgn_geometry_material_face_count;
    int dgn_geometry_can_submit_geometry;
    int dgn_geometry_textured_raster_blocked;
    int dgn_material_selector_counts_match_faces;
    int dgn_static_selectors_within_structure2_descriptors;
    int dgn_structure2_descriptor_route_bound;
    int dgn_selector_bindings_complete;
    int dgn_material_pixel_promotion_blocked;
    int dgn_can_submit_raster_input;
    int dgn_fallback_visuals_permitted;
    int dgn_no_draw_only;
    int dgn_blocks_real_dgn_mesh_render;
    uint32_t prs3_entry_index;
    uint32_t prs3_stream_offset;
    uint32_t prs3_stream_size;
    uint32_t prs3_expected_output_bytes;
    uint64_t prs3_output_fnv1a64;
} Nexus_V1_DgnMenuPrs3RouteReceipt;

/* Returns 1 only when a complete retail-DGN binding table is admissible. */
int nexus_v1_dgn_face_material_validate(
    const Nexus_V1_DgnFaceMaterialInput *input,
    Nexus_V1_DgnFaceMaterialReceipt *out_receipt);

/* Consume the material receipt at a package/host boundary without promoting
 * material semantics, Saturn rendering, raster input submission, or fallback
 * visuals. */
int nexus_v1_dgn_package_host_consumer_gate(
    const Nexus_V1_DgnPackageHostConsumerInput *input,
    Nexus_V1_DgnPackageHostConsumerReceipt *out_receipt);

/* Join DGN material package-host evidence with PRS3 MENU.BPK output/upload
 * evidence. This proves the two real-data routes are both source-bound, but
 * it deliberately keeps DGN rendering, menu rendering, runtime upload, and
 * fallback visuals closed until independent Saturn capture and decoder review
 * exist. */
int nexus_v1_dgn_menu_prs3_route_gate(
    const Nexus_V1_DgnMenuPrs3RouteInput *input,
    Nexus_V1_DgnMenuPrs3RouteReceipt *out_receipt);

const char *nexus_v1_dgn_face_material_status_name(
    Nexus_V1_DgnFaceMaterialStatus status);

const char *nexus_v1_dgn_package_host_consumer_status_name(
    Nexus_V1_DgnPackageHostConsumerStatus status);

#endif

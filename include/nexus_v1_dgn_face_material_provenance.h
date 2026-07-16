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
    int structure3_mesh_materials_bound;
    int structure2_descriptor_route_bound;
    int selector_bindings_complete;
    int package_host_route_bound;
    int no_draw_only;
    int blocks_real_dgn_mesh_render;
    int permits_fallback_visuals;
    int can_submit_raster_input;
} Nexus_V1_DgnFaceMaterialReceipt;

/* Returns 1 only when a complete retail-DGN binding table is admissible. */
int nexus_v1_dgn_face_material_validate(
    const Nexus_V1_DgnFaceMaterialInput *input,
    Nexus_V1_DgnFaceMaterialReceipt *out_receipt);

const char *nexus_v1_dgn_face_material_status_name(
    Nexus_V1_DgnFaceMaterialStatus status);

#endif

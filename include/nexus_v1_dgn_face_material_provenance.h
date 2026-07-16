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
    NEXUS_V1_DGN_FACE_MATERIAL_SELECTOR_COLOR = 0,
    NEXUS_V1_DGN_FACE_MATERIAL_SELECTOR_STATIC = 1,
    NEXUS_V1_DGN_FACE_MATERIAL_SELECTOR_ANIMATED = 2
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
    int material_selector_count;
    /*
     * Optional material-host route evidence from the BPK/DMDF boundary.
     * These fields let Structure3 face admission consume a fail-closed
     * material receipt without promoting PRS3 pixels or VDP1 draw commands.
     */
    int material_host_route_bound;
    int material_host_route_prs3_blocked;
    int material_host_route_pixel_promotion_blocked;
    int material_host_route_decoder_promoted;
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
    int color_selector_count;
    int static_selector_count;
    int animated_selector_count;
    int selector_bindings_complete;
    int permits_fallback_visuals;
    int can_submit_raster_input;
    int can_submit_textured_draw;
    int textured_draw_blocked;
    int static_texture_draw_blocked;
    int animated_texture_draw_blocked;
    int structure2_material_required;
    int structure1g_material_required;
    int structure2_pixel_semantics_required;
    int structure1g_animation_semantics_required;
    int material_host_route_bound;
    int material_host_route_prs3_blocked;
    int material_host_route_pixel_promotion_blocked;
    int material_host_route_decoder_promoted;
    int material_admission_blocked;
    int structure3_texture_admission_blocked;
    int material_bank_mutation_blocked;
    int vdp1_command_required;
    int vdp1_command_proven;
    int vdp1_draw_list_blocked;
} Nexus_V1_DgnFaceMaterialReceipt;

/* Returns 1 only when a complete retail-DGN binding table is admissible. */
int nexus_v1_dgn_face_material_validate(
    const Nexus_V1_DgnFaceMaterialInput *input,
    Nexus_V1_DgnFaceMaterialReceipt *out_receipt);

const char *nexus_v1_dgn_face_material_status_name(
    Nexus_V1_DgnFaceMaterialStatus status);

#endif

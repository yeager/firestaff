#ifndef NEXUS_V1_DGN_MESH_H
#define NEXUS_V1_DGN_MESH_H

#include <stdint.h>
#include "nexus_v1_dgn_face_material_provenance.h"

/*
 * A renderer-neutral packet for parser-validated Structure3 geometry.  This
 * module neither decodes texture bytes nor chooses a VDP1 ordering.  It
 * preserves the documented 16.16 positions and Structure3b face layout so a
 * later, independently proven material path can consume real DGN geometry.
 */

#define NEXUS_V1_DGN_MESH_MAX_VERTICES 4096
#define NEXUS_V1_DGN_MESH_MAX_FACES 4096
#define NEXUS_V1_DGN_MESH_MAX_CORNERS (NEXUS_V1_DGN_MESH_MAX_FACES * 4)

typedef struct {
    int32_t x;
    int32_t y;
    int32_t z;
} Nexus_V1_DgnMeshFixedVertex;

typedef struct {
    uint16_t vertex_index[4];
    uint8_t flags;
    uint16_t fill_selector;
} Nexus_V1_DgnMeshSourceFace;

typedef enum {
    NEXUS_V1_DGN_MESH_FILL_COLOR = 0,
    NEXUS_V1_DGN_MESH_FILL_STATIC_TEXTURE,
    NEXUS_V1_DGN_MESH_FILL_ANIMATED_TEXTURE
} Nexus_V1_DgnMeshFillKind;

typedef struct {
    int first_corner;
    int corner_count;
    uint8_t flags;
    uint16_t fill_selector;
    Nexus_V1_DgnMeshFillKind fill_kind;
} Nexus_V1_DgnMeshFace;

typedef struct {
    const Nexus_V1_DgnMeshFixedVertex *vertices;
    int vertex_count;
    const Nexus_V1_DgnMeshSourceFace *faces;
    int face_count;
    /* These are copied from the existing canonical DGN/parser receipts. */
    int canonical_source_verified;
    int topology_receipt_valid;
    int fixed_point_vectors_valid;
} Nexus_V1_DgnMeshInput;

typedef enum {
    NEXUS_V1_DGN_MESH_READY_GEOMETRY = 0,
    NEXUS_V1_DGN_MESH_BLOCKED_INPUT,
    NEXUS_V1_DGN_MESH_BLOCKED_SOURCE,
    NEXUS_V1_DGN_MESH_BLOCKED_TOPOLOGY,
    NEXUS_V1_DGN_MESH_BLOCKED_FACE
} Nexus_V1_DgnMeshStatus;

typedef struct {
    Nexus_V1_DgnMeshStatus status;
    Nexus_V1_DgnMeshFixedVertex vertices[NEXUS_V1_DGN_MESH_MAX_VERTICES];
    uint16_t corner_vertex_indexes[NEXUS_V1_DGN_MESH_MAX_CORNERS];
    Nexus_V1_DgnMeshFace faces[NEXUS_V1_DGN_MESH_MAX_FACES];
    int vertex_count;
    int face_count;
    int corner_count;
    int triangle_count;
    int quad_count;
    int color_fill_count;
    int static_texture_face_count;
    int animated_texture_face_count;
    int canonical_source_verified;
    int permits_fallback_visuals;
    int can_submit_geometry;
    int can_submit_textured_raster;
    int textured_raster_blocked;
    int static_texture_raster_blocked;
    int animated_texture_raster_blocked;
    int material_provenance_required;
    int structure2_material_required;
    int structure1g_material_required;
    int structure2_pixel_semantics_required;
    int structure1g_animation_semantics_required;
    int material_bank_mutation_blocked;
    int vdp1_draw_list_blocked;
    int vdp1_provenance_required;
} Nexus_V1_DgnMesh;

/* Builds only real, parser-validated DGN geometry. Returns 1 on success. */
int nexus_v1_dgn_mesh_build(const Nexus_V1_DgnMeshInput *input,
                            Nexus_V1_DgnMesh *out_mesh);

const char *nexus_v1_dgn_mesh_status_name(Nexus_V1_DgnMeshStatus status);

#endif

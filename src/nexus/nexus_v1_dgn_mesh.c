#include "nexus_v1_dgn_mesh.h"

#include <string.h>

static void nexus_v1_dgn_mesh_set_status(Nexus_V1_DgnMesh *mesh,
                                         Nexus_V1_DgnMeshStatus status)
{
    memset(mesh, 0, sizeof(*mesh));
    mesh->status = status;
}

static Nexus_V1_DgnMeshFillKind nexus_v1_dgn_mesh_fill_kind(
    const Nexus_V1_DgnMeshSourceFace *face)
{
    if ((face->flags & 0x40U) == 0U) return NEXUS_V1_DGN_MESH_FILL_COLOR;
    if ((face->fill_selector & 0xff00U) == 0x0800U)
        return NEXUS_V1_DGN_MESH_FILL_ANIMATED_TEXTURE;
    return NEXUS_V1_DGN_MESH_FILL_STATIC_TEXTURE;
}

int nexus_v1_dgn_mesh_build(const Nexus_V1_DgnMeshInput *input,
                            Nexus_V1_DgnMesh *out_mesh)
{
    int face_index;

    if (!out_mesh) return 0;
    nexus_v1_dgn_mesh_set_status(out_mesh, NEXUS_V1_DGN_MESH_BLOCKED_INPUT);
    if (!input || !input->vertices || !input->faces || input->vertex_count <= 0 ||
        input->vertex_count > NEXUS_V1_DGN_MESH_MAX_VERTICES ||
        input->face_count <= 0 || input->face_count > NEXUS_V1_DGN_MESH_MAX_FACES) {
        return 0;
    }
    if (!input->canonical_source_verified) {
        nexus_v1_dgn_mesh_set_status(out_mesh,
                                     NEXUS_V1_DGN_MESH_BLOCKED_SOURCE);
        return 0;
    }
    if (!input->topology_receipt_valid || !input->fixed_point_vectors_valid) {
        nexus_v1_dgn_mesh_set_status(out_mesh,
                                     NEXUS_V1_DGN_MESH_BLOCKED_TOPOLOGY);
        return 0;
    }

    out_mesh->canonical_source_verified = 1;
    out_mesh->vertex_count = input->vertex_count;
    out_mesh->face_count = input->face_count;
    memcpy(out_mesh->vertices, input->vertices,
           (size_t)input->vertex_count * sizeof(*input->vertices));
    for (face_index = 0; face_index < input->face_count; ++face_index) {
        const Nexus_V1_DgnMeshSourceFace *source = &input->faces[face_index];
        Nexus_V1_DgnMeshFace *face = &out_mesh->faces[face_index];
        int corner_count = source->vertex_index[2] == source->vertex_index[3] ? 3 : 4;
        int corner;

        for (corner = 0; corner < corner_count; ++corner) {
            if (source->vertex_index[corner] >= input->vertex_count) {
                nexus_v1_dgn_mesh_set_status(out_mesh,
                                             NEXUS_V1_DGN_MESH_BLOCKED_FACE);
                return 0;
            }
        }
        face->first_corner = out_mesh->corner_count;
        face->corner_count = corner_count;
        face->flags = source->flags;
        face->fill_selector = source->fill_selector;
        face->fill_kind = nexus_v1_dgn_mesh_fill_kind(source);
        for (corner = 0; corner < corner_count; ++corner) {
            out_mesh->corner_vertex_indexes[out_mesh->corner_count++] =
                source->vertex_index[corner];
        }
        if (corner_count == 3) ++out_mesh->triangle_count;
        else ++out_mesh->quad_count;
        if (face->fill_kind == NEXUS_V1_DGN_MESH_FILL_COLOR)
            ++out_mesh->color_fill_count;
        else if (face->fill_kind == NEXUS_V1_DGN_MESH_FILL_STATIC_TEXTURE)
            ++out_mesh->static_texture_face_count;
        else
            ++out_mesh->animated_texture_face_count;
    }

    out_mesh->status = NEXUS_V1_DGN_MESH_READY_GEOMETRY;
    out_mesh->can_submit_geometry = 1;
    /* Structure2/Structure1G payloads are not decoded texture surfaces yet. */
    out_mesh->can_submit_textured_raster = 0;
    out_mesh->static_texture_raster_blocked =
        out_mesh->static_texture_face_count > 0 ? 1 : 0;
    out_mesh->animated_texture_raster_blocked =
        out_mesh->animated_texture_face_count > 0 ? 1 : 0;
    out_mesh->material_provenance_required =
        (out_mesh->static_texture_face_count > 0 ||
         out_mesh->animated_texture_face_count > 0)
            ? 1
            : 0;
    out_mesh->structure2_material_required =
        out_mesh->static_texture_raster_blocked;
    out_mesh->structure1g_material_required =
        out_mesh->animated_texture_raster_blocked;
    out_mesh->structure2_pixel_semantics_required =
        out_mesh->static_texture_raster_blocked;
    out_mesh->structure1g_animation_semantics_required =
        out_mesh->animated_texture_raster_blocked;
    out_mesh->material_bank_mutation_blocked =
        out_mesh->material_provenance_required;
    out_mesh->vdp1_provenance_required =
        out_mesh->material_provenance_required;
    out_mesh->vdp1_draw_list_blocked =
        out_mesh->vdp1_provenance_required;
    out_mesh->textured_raster_blocked =
        out_mesh->material_provenance_required;
    return 1;
}

const char *nexus_v1_dgn_mesh_status_name(Nexus_V1_DgnMeshStatus status)
{
    switch (status) {
    case NEXUS_V1_DGN_MESH_READY_GEOMETRY: return "ready-geometry";
    case NEXUS_V1_DGN_MESH_BLOCKED_SOURCE: return "blocked-source";
    case NEXUS_V1_DGN_MESH_BLOCKED_TOPOLOGY: return "blocked-topology";
    case NEXUS_V1_DGN_MESH_BLOCKED_FACE: return "blocked-face";
    case NEXUS_V1_DGN_MESH_BLOCKED_INPUT: return "blocked-input";
    }
    return "blocked-input";
}

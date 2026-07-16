#include "nexus_v1_dgn_mesh.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void expect(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

int main(void)
{
    static const Nexus_V1_DgnMeshFixedVertex vertices[] = {
        {0, 0, 0}, {65536, 0, 0}, {65536, 65536, 0}, {0, 65536, 0}
    };
    Nexus_V1_DgnMeshSourceFace faces[] = {
        {{0, 1, 2, 2}, 0x40U, 0x0012U},
        {{0, 2, 3, 1}, 0x40U, 0x0801U},
        {{0, 1, 3, 3}, 0x00U, 0x8a11U}
    };
    Nexus_V1_DgnMeshInput input;
    Nexus_V1_DgnMesh mesh;

    memset(&input, 0, sizeof(input));
    input.vertices = vertices;
    input.vertex_count = 4;
    input.faces = faces;
    input.face_count = 3;
    input.canonical_source_verified = 1;
    input.topology_receipt_valid = 1;
    input.fixed_point_vectors_valid = 1;

    expect(nexus_v1_dgn_mesh_build(&input, &mesh) &&
               mesh.status == NEXUS_V1_DGN_MESH_READY_GEOMETRY &&
               mesh.vertex_count == 4 && mesh.face_count == 3 &&
               mesh.corner_count == 10 && mesh.triangle_count == 2 &&
               mesh.quad_count == 1 && mesh.static_texture_face_count == 1 &&
               mesh.animated_texture_face_count == 1 && mesh.color_fill_count == 1 &&
               mesh.faces[1].first_corner == 3 && mesh.faces[1].corner_count == 4 &&
               mesh.corner_vertex_indexes[5] == 3 && mesh.can_submit_geometry &&
               !mesh.can_submit_textured_raster &&
               mesh.textured_raster_blocked &&
               mesh.static_texture_raster_blocked &&
               mesh.animated_texture_raster_blocked &&
               mesh.material_provenance_required &&
               mesh.structure2_material_required &&
               mesh.structure1g_material_required &&
               mesh.structure2_pixel_semantics_required &&
               mesh.structure1g_animation_semantics_required &&
               mesh.material_bank_mutation_blocked &&
               mesh.vdp1_provenance_required &&
               mesh.vdp1_draw_list_blocked &&
               !mesh.permits_fallback_visuals,
           "parser-validated DGN geometry blocks textured raster without material/VDP1 proof");

    input.canonical_source_verified = 0;
    expect(!nexus_v1_dgn_mesh_build(&input, &mesh) &&
               mesh.status == NEXUS_V1_DGN_MESH_BLOCKED_SOURCE &&
               !mesh.can_submit_geometry &&
               !mesh.textured_raster_blocked &&
               !mesh.static_texture_raster_blocked &&
               !mesh.animated_texture_raster_blocked &&
               !mesh.material_bank_mutation_blocked &&
               !mesh.vdp1_draw_list_blocked &&
               !mesh.permits_fallback_visuals,
           "unverified source bytes cannot create a DGN mesh packet");
    input.canonical_source_verified = 1;

    input.fixed_point_vectors_valid = 0;
    expect(!nexus_v1_dgn_mesh_build(&input, &mesh) &&
               mesh.status == NEXUS_V1_DGN_MESH_BLOCKED_TOPOLOGY,
           "invalid fixed-point vectors block mesh construction");
    input.fixed_point_vectors_valid = 1;

    {
        Nexus_V1_DgnMeshSourceFace color_face[] = {
            {{0, 1, 2, 2}, 0x00U, 0x0000U}
        };
        input.faces = color_face;
        input.face_count = 1;
        expect(nexus_v1_dgn_mesh_build(&input, &mesh) &&
                   mesh.status == NEXUS_V1_DGN_MESH_READY_GEOMETRY &&
                   mesh.color_fill_count == 1 &&
                   mesh.static_texture_face_count == 0 &&
                   mesh.animated_texture_face_count == 0 &&
                   mesh.can_submit_geometry &&
                   !mesh.textured_raster_blocked &&
                   !mesh.static_texture_raster_blocked &&
                   !mesh.animated_texture_raster_blocked &&
                   !mesh.material_provenance_required &&
                   !mesh.structure2_material_required &&
                   !mesh.structure1g_material_required &&
                   !mesh.structure2_pixel_semantics_required &&
                   !mesh.structure1g_animation_semantics_required &&
                   !mesh.material_bank_mutation_blocked &&
                   !mesh.vdp1_provenance_required &&
                   !mesh.vdp1_draw_list_blocked &&
                   !mesh.permits_fallback_visuals,
               "color-only Structure3 geometry does not invent a textured raster blocker");
        input.faces = faces;
        input.face_count = 3;
    }

    faces[2].vertex_index[2] = 4;
    expect(!nexus_v1_dgn_mesh_build(&input, &mesh) &&
               mesh.status == NEXUS_V1_DGN_MESH_BLOCKED_FACE &&
               strcmp(nexus_v1_dgn_mesh_status_name(mesh.status), "blocked-face") == 0,
           "out-of-range Structure3 face indexes fail closed");

    if (failures) return 1;
    puts("Nexus DGN mesh passed");
    return 0;
}

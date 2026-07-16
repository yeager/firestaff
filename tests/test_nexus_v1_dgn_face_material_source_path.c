#include "nexus_v1_dgn_face_material_provenance.h"
#include "nexus_v1_dgn_mesh.h"
#include "nexus_v1_dungeon.h"

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

static void put16(uint8_t *dst, unsigned value)
{
    dst[0] = (uint8_t)(value >> 8);
    dst[1] = (uint8_t)value;
}

static void put32(uint8_t *dst, unsigned value)
{
    dst[0] = (uint8_t)(value >> 24);
    dst[1] = (uint8_t)(value >> 16);
    dst[2] = (uint8_t)(value >> 8);
    dst[3] = (uint8_t)value;
}

int main(void)
{
    uint8_t dgn[128];
    uint8_t canonical[sizeof(dgn)];
    Nexus_V1_Level level;
    Nexus_V1_DgnFaceMaterialBinding bindings[3];
    Nexus_V1_DgnMeshFixedVertex vertices[4];
    Nexus_V1_DgnMeshSourceFace mesh_faces[3];
    Nexus_V1_DgnMeshInput mesh_input;
    Nexus_V1_DgnMesh mesh;
    Nexus_V1_DgnFaceMaterialInput input;
    Nexus_V1_DgnFaceMaterialReceipt receipt;
    int binding_count = 0;

    memset(dgn, 0, sizeof(dgn));
    put32(dgn, 1);              /* Structure3 directory entry count. */
    put32(dgn + 4, 8);          /* First entry. */
    put16(dgn + 14, 3);         /* Entry-local face count. */
    put32(dgn + 24, 32);        /* Structure3b face region. */
    dgn[52] = 0x40;             /* Static fill at face 1. */
    put16(dgn + 54, 7);
    dgn[64] = 0x40;             /* Structure1G animated fill at face 2. */
    put16(dgn + 66, 0x0803);
    memcpy(canonical, dgn, sizeof(dgn));

    memset(&level, 0, sizeof(level));
    level.structure3_payload.valid = 1;
    level.structure3_payload.byte_offset = 0;
    level.structure3_payload.byte_size = (int)sizeof(dgn);
    level.structure3_directory.valid = 1;
    level.structure3_directory.entry_count = 1;
    level.structure3_directory.directory_byte_count = 8;
    level.structure3_entry_headers.valid = 1;
    level.structure3_faces.valid = 1;
    level.structure3_faces.face_count = 3;
    level.structure3_face_materials.valid = 1;
    level.structure3_face_materials.selector_bindings_complete = 1;

    expect(nexus_v1_level_collect_structure3_face_material_bindings(
               &level, dgn, (int)sizeof(dgn), bindings, 3, &binding_count) == 0 &&
               binding_count == 2 &&
               bindings[0].selector_kind == NEXUS_V1_DGN_FACE_MATERIAL_SELECTOR_STATIC &&
               bindings[0].material_selector == 7 &&
               bindings[1].selector_kind == NEXUS_V1_DGN_FACE_MATERIAL_SELECTOR_ANIMATED &&
               bindings[1].material_selector == 3,
           "the active Structure3 bytes produce only documented static/animated material bindings");

    memset(vertices, 0, sizeof(vertices));
    vertices[1].x = 65536;
    vertices[2].x = 65536;
    vertices[2].y = 65536;
    vertices[3].y = 65536;
    memset(mesh_faces, 0, sizeof(mesh_faces));
    mesh_faces[0].vertex_index[0] = 0;
    mesh_faces[0].vertex_index[1] = 1;
    mesh_faces[0].vertex_index[2] = 2;
    mesh_faces[0].vertex_index[3] = 2;
    mesh_faces[0].flags = 0x00U;
    mesh_faces[1].vertex_index[0] = 0;
    mesh_faces[1].vertex_index[1] = 2;
    mesh_faces[1].vertex_index[2] = 3;
    mesh_faces[1].vertex_index[3] = 3;
    mesh_faces[1].flags = 0x40U;
    mesh_faces[1].fill_selector = 7U;
    mesh_faces[2].vertex_index[0] = 0;
    mesh_faces[2].vertex_index[1] = 1;
    mesh_faces[2].vertex_index[2] = 3;
    mesh_faces[2].vertex_index[3] = 3;
    mesh_faces[2].flags = 0x40U;
    mesh_faces[2].fill_selector = 0x0803U;
    memset(&mesh_input, 0, sizeof(mesh_input));
    mesh_input.vertices = vertices;
    mesh_input.vertex_count = 4;
    mesh_input.faces = mesh_faces;
    mesh_input.face_count = 3;
    mesh_input.canonical_source_verified = 1;
    mesh_input.topology_receipt_valid = 1;
    mesh_input.fixed_point_vectors_valid = 1;
    expect(nexus_v1_dgn_mesh_build(&mesh_input, &mesh) == 1 &&
               mesh.status == NEXUS_V1_DGN_MESH_READY_GEOMETRY &&
               mesh.can_submit_geometry &&
               !mesh.can_submit_textured_raster &&
               mesh.textured_raster_blocked &&
               mesh.color_fill_count == 1 &&
               mesh.static_texture_face_count == 1 &&
               mesh.animated_texture_face_count == 1 &&
               !mesh.permits_fallback_visuals,
           "the matching Structure3 geometry proof stays renderer-neutral");

    memset(&input, 0, sizeof(input));
    input.source = NEXUS_V1_DGN_FACE_MATERIAL_SOURCE_RETAIL_DGN;
    input.dgn_bytes = dgn;
    input.dgn_size = (int)sizeof(dgn);
    input.canonical_dgn_bytes = canonical;
    input.canonical_dgn_size = (int)sizeof(canonical);
    input.canonical_source_verified = 1;
    input.bindings = bindings;
    input.face_count = binding_count;
    input.material_selector_count = 256;
    input.structure2_descriptor_count = 8;
    input.geometry_source_bound = mesh.canonical_source_verified;
    input.geometry_material_face_count =
        mesh.static_texture_face_count + mesh.animated_texture_face_count;
    input.geometry_can_submit_geometry = mesh.can_submit_geometry;
    input.geometry_can_submit_textured_raster = mesh.can_submit_textured_raster;
    input.geometry_fallback_visuals_permitted = mesh.permits_fallback_visuals;
    expect(nexus_v1_dgn_face_material_validate(&input, &receipt) == 1 &&
               receipt.status == NEXUS_V1_DGN_FACE_MATERIAL_READY &&
               receipt.face_count == 2 &&
               receipt.structure2_descriptor_count == 8 &&
               receipt.static_selector_count == 1 &&
               receipt.animated_selector_count == 1 &&
               receipt.geometry_source_bound &&
               receipt.geometry_material_face_count == 2 &&
               receipt.geometry_material_face_count_matches &&
               receipt.geometry_can_submit_geometry &&
               receipt.geometry_textured_raster_blocked &&
               receipt.structure3_mesh_materials_bound &&
               receipt.structure2_descriptor_route_bound &&
               receipt.selector_bindings_complete &&
               !receipt.material_semantics_proven &&
               receipt.package_host_route_bound &&
               receipt.no_draw_only &&
               receipt.blocks_real_dgn_mesh_render &&
               receipt.original_saturn_capture_required &&
               !receipt.original_saturn_capture_available &&
               !receipt.can_submit_raster_input &&
               !receipt.permits_fallback_visuals,
           "the real-buffer binding table reaches only the no-fallback source boundary");

    input.geometry_material_face_count = 1;
    expect(!nexus_v1_dgn_face_material_validate(&input, &receipt) &&
               receipt.status == NEXUS_V1_DGN_FACE_MATERIAL_BLOCKED_SOURCE &&
               !receipt.can_submit_raster_input &&
               !receipt.permits_fallback_visuals,
           "material admission rejects a geometry proof that omits a material face");
    input.geometry_material_face_count =
        mesh.static_texture_face_count + mesh.animated_texture_face_count;

    ++dgn[67];
    expect(!nexus_v1_dgn_face_material_validate(&input, &receipt) &&
               receipt.status == NEXUS_V1_DGN_FACE_MATERIAL_BLOCKED_SOURCE &&
               !receipt.can_submit_raster_input,
           "a changed retained LEV buffer cannot reuse canonical Structure3 admission");

    if (failures) return 1;
    puts("Nexus DGN face/material source path passed");
    return 0;
}

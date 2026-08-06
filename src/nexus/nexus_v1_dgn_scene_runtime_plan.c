#include "nexus_v1_dgn_scene_runtime_plan.h"

#include <stdlib.h>
#include <string.h>

static void reset_receipt(Nexus_V1_DgnSceneRuntimePlanReceipt *receipt,
                          Nexus_V1_DgnSceneRuntimePlanStatus status)
{
    memset(receipt, 0, sizeof(*receipt));
    receipt->status = status;
    receipt->selected_structure1f_entry_index = -1;
    receipt->selected_owner_x = -1;
    receipt->selected_owner_y = -1;
    receipt->selected_structure1a_index = -1;
    receipt->selected_structure3_model_index = -1;
    receipt->selected_face_ordinal = -1;
    receipt->texture_submit_blocked = 1;
    receipt->raster_submit_blocked = 1;
    receipt->fallback_geometry_permitted = 0;
    receipt->fallback_visuals_permitted = 0;
    receipt->no_draw_only = 1;
}

static void step_for_dir(int dir, int *dx, int *dy)
{
    static const int step_x[4] = {0, 1, 0, -1};
    static const int step_y[4] = {-1, 0, 1, 0};
    int index = dir & 3;
    *dx = step_x[index];
    *dy = step_y[index];
}

static Nexus_V1_DgnMeshFillKind fill_kind_from_face(
    const Nexus_V1_DgnStructure3Face *face)
{
    if ((face->flags & 0x40U) == 0U) return NEXUS_V1_DGN_MESH_FILL_COLOR;
    if ((face->fill_selector & 0xff00U) == 0x0800U)
        return NEXUS_V1_DGN_MESH_FILL_ANIMATED_TEXTURE;
    return NEXUS_V1_DGN_MESH_FILL_STATIC_TEXTURE;
}

int nexus_v1_dgn_scene_runtime_plan_build(
    const Nexus_V1_DgnSceneRuntimePlanInput *input,
    Nexus_V1_DgnSceneRuntimePlanReceipt *out_receipt)
{
    Nexus_V1_DgnStructure3MeshEntryReceipt mesh_entry;
    Nexus_V1_DgnStructure3Vector *vertices = NULL;
    Nexus_V1_DgnStructure3Vector *normals = NULL;
    Nexus_V1_DgnStructure3Face *faces = NULL;
    Nexus_V1_DgnMeshFixedVertex *mesh_vertices = NULL;
    Nexus_V1_DgnMeshSourceFace *mesh_faces = NULL;
    Nexus_V1_DgnMeshInput mesh_input;
    Nexus_V1_DgnMesh mesh;
    const Nexus_V1_DgnStructure1FEntry *selected = NULL;
    int dx, dy;
    int left_dx, left_dy;
    int right_dx, right_dy;
    int index;

    if (!out_receipt) return 0;
    reset_receipt(out_receipt,
                  NEXUS_V1_DGN_SCENE_RUNTIME_PLAN_BLOCKED_INPUT);
    if (!input || !input->level || !input->dgn_data || input->dgn_size <= 0 ||
        !input->dgn_source_hash_verified ||
        input->party_x < 0 || input->party_x >= NEXUS_MAX_MAP_SIZE ||
        input->party_y < 0 || input->party_y >= NEXUS_MAX_MAP_SIZE) {
        return 0;
    }
    out_receipt->source_bound = 1;
    out_receipt->level_index = input->level_index;
    out_receipt->party_x = input->party_x;
    out_receipt->party_y = input->party_y;
    out_receipt->party_dir = input->party_dir & 3;
    step_for_dir(out_receipt->party_dir, &dx, &dy);
    step_for_dir((out_receipt->party_dir + 3) & 3, &left_dx, &left_dy);
    step_for_dir((out_receipt->party_dir + 1) & 3, &right_dx, &right_dy);
    out_receipt->forward_x = input->party_x + dx;
    out_receipt->forward_y = input->party_y + dy;
    out_receipt->left_x = input->party_x + left_dx;
    out_receipt->left_y = input->party_y + left_dy;
    out_receipt->right_x = input->party_x + right_dx;
    out_receipt->right_y = input->party_y + right_dy;
    out_receipt->party_cell_bound =
        nexus_v1_level_get_square(input->level, input->party_x,
                                  input->party_y) >= 0;
    out_receipt->forward_cell_bound =
        nexus_v1_level_get_square(input->level, out_receipt->forward_x,
                                  out_receipt->forward_y) >= 0;
    out_receipt->side_cells_bound =
        nexus_v1_level_get_square(input->level, out_receipt->left_x,
                                  out_receipt->left_y) >= 0 &&
        nexus_v1_level_get_square(input->level, out_receipt->right_x,
                                  out_receipt->right_y) >= 0;
    if (!out_receipt->party_cell_bound) return 0;

    out_receipt->source_cell_count = 1 + out_receipt->forward_cell_bound +
        (out_receipt->side_cells_bound ? 2 : 0);

    for (index = 0; index < input->level->structure1f_entry_count; ++index) {
        const Nexus_V1_DgnStructure1FEntry *entry =
            &input->level->structure1f_entries[index];
        if (entry->structure1a_relation_valid &&
            (int)entry->structure1a_structure3_model_index <
                input->level->structure3_directory.entry_count &&
            entry->face < input->level->structure3_entry_face_counts[
                entry->structure1a_structure3_model_index]) {
            selected = entry;
            out_receipt->selected_structure1f_entry_index = index;
            break;
        }
    }
    if (!selected) {
        /* A free-standing Structure3 model is not an active dungeon face.
         * The former first-entry selection promoted an arbitrary mesh when
         * the Structure1F -> Structure1A owner chain was absent. Keep the
         * real adjacent-cell receipt, but require that source-owned chain
         * before constructing any scene geometry. */
        out_receipt->status =
            NEXUS_V1_DGN_SCENE_RUNTIME_PLAN_BLOCKED_STRUCTURE1F;
        return 0;
    } else {
        out_receipt->structure1f_visible_owned_entry_count = 1;
        out_receipt->structure1f_owned_source_count = 1;
        out_receipt->topology_candidate_count = 1;
        out_receipt->selected_owner_x = selected->structure1a_owner_x;
        out_receipt->selected_owner_y = selected->structure1a_owner_y;
        out_receipt->selected_structure1a_index =
            selected->structure1a_index;
        out_receipt->selected_structure3_model_index =
            selected->structure1a_structure3_model_index;
        out_receipt->selected_face_ordinal = selected->face;
        out_receipt->selected_z_rotation = selected->structure1a_z_rotation;
        out_receipt->structure1f_face_selector_bound = 1;
        out_receipt->structure1a_model_rotation_bound = 1;
        out_receipt->face_ordinal_within_model_bound = 1;
    }

    memset(&mesh_entry, 0, sizeof(mesh_entry));
    (void)nexus_v1_level_extract_structure3_mesh_entry(
        input->level, input->dgn_data, input->dgn_size,
        out_receipt->selected_structure3_model_index, NULL, 0, NULL, 0,
        NULL, 0, &mesh_entry);
    if (!mesh_entry.source_identity_valid || mesh_entry.vertex_count <= 0 ||
        mesh_entry.face_count <= 0 ||
        out_receipt->selected_face_ordinal >= mesh_entry.face_count) {
        out_receipt->status =
            NEXUS_V1_DGN_SCENE_RUNTIME_PLAN_BLOCKED_MESH_ENTRY;
        return 0;
    }

    vertices = (Nexus_V1_DgnStructure3Vector *)calloc(
        (size_t)mesh_entry.vertex_count, sizeof(*vertices));
    normals = (Nexus_V1_DgnStructure3Vector *)calloc(
        (size_t)mesh_entry.normal_count, sizeof(*normals));
    faces = (Nexus_V1_DgnStructure3Face *)calloc(
        (size_t)mesh_entry.face_count, sizeof(*faces));
    mesh_vertices = (Nexus_V1_DgnMeshFixedVertex *)calloc(
        (size_t)mesh_entry.vertex_count, sizeof(*mesh_vertices));
    mesh_faces = (Nexus_V1_DgnMeshSourceFace *)calloc(
        (size_t)mesh_entry.face_count, sizeof(*mesh_faces));
    if (!vertices || !normals || !faces || !mesh_vertices || !mesh_faces) {
        out_receipt->status =
            NEXUS_V1_DGN_SCENE_RUNTIME_PLAN_BLOCKED_MESH_ENTRY;
        goto cleanup;
    }
    if (nexus_v1_level_extract_structure3_mesh_entry(
            input->level, input->dgn_data, input->dgn_size,
            out_receipt->selected_structure3_model_index, vertices,
            mesh_entry.vertex_count, faces, mesh_entry.face_count, normals,
            mesh_entry.normal_count, &mesh_entry) != 0 ||
        !mesh_entry.valid || mesh_entry.transform_or_draw_semantics_proven) {
        out_receipt->status =
            NEXUS_V1_DGN_SCENE_RUNTIME_PLAN_BLOCKED_MESH_ENTRY;
        goto cleanup;
    }
    for (index = 0; index < mesh_entry.vertex_count; ++index) {
        mesh_vertices[index].x = vertices[index].x;
        mesh_vertices[index].y = vertices[index].y;
        mesh_vertices[index].z = vertices[index].z;
    }
    for (index = 0; index < mesh_entry.face_count; ++index) {
        mesh_faces[index].vertex_index[0] = faces[index].vertex_indexes[0];
        mesh_faces[index].vertex_index[1] = faces[index].vertex_indexes[1];
        mesh_faces[index].vertex_index[2] = faces[index].vertex_indexes[2];
        mesh_faces[index].vertex_index[3] = faces[index].vertex_indexes[3];
        mesh_faces[index].flags = faces[index].flags;
        mesh_faces[index].fill_selector = faces[index].fill_selector;
    }
    memset(&mesh_input, 0, sizeof(mesh_input));
    mesh_input.vertices = mesh_vertices;
    mesh_input.vertex_count = mesh_entry.vertex_count;
    mesh_input.faces = mesh_faces;
    mesh_input.face_count = mesh_entry.face_count;
    mesh_input.canonical_source_verified = 1;
    mesh_input.topology_receipt_valid = 1;
    mesh_input.fixed_point_vectors_valid = 1;
    memset(&mesh, 0, sizeof(mesh));
    if (nexus_v1_dgn_mesh_build(&mesh_input, &mesh) != 1 ||
        mesh.status != NEXUS_V1_DGN_MESH_READY_GEOMETRY ||
        !mesh.can_submit_geometry || mesh.can_submit_textured_raster ||
        mesh.permits_fallback_visuals) {
        out_receipt->status =
            NEXUS_V1_DGN_SCENE_RUNTIME_PLAN_BLOCKED_MESH_BUILD;
        goto cleanup;
    }

    out_receipt->mesh_entry_bound = 1;
    out_receipt->mesh_vertex_count = mesh.vertex_count;
    out_receipt->mesh_face_count = mesh.face_count;
    out_receipt->mesh_normal_count = mesh_entry.normal_count;
    out_receipt->mesh_triangle_count = mesh.triangle_count;
    out_receipt->mesh_quad_count = mesh.quad_count;
    out_receipt->mesh_color_fill_count = mesh.color_fill_count;
    out_receipt->mesh_static_texture_face_count =
        mesh.static_texture_face_count;
    out_receipt->mesh_animated_texture_face_count =
        mesh.animated_texture_face_count;
    out_receipt->selected_face_fill_selector =
        faces[out_receipt->selected_face_ordinal].fill_selector;
    out_receipt->selected_face_flags =
        faces[out_receipt->selected_face_ordinal].flags;
    out_receipt->selected_face_fill_kind =
        fill_kind_from_face(&faces[out_receipt->selected_face_ordinal]);
    out_receipt->material_index_bound =
        out_receipt->selected_face_fill_kind != NEXUS_V1_DGN_MESH_FILL_COLOR;
    out_receipt->geometry_consumer_ready = 1;
    out_receipt->texture_submit_blocked =
        input->vdp1_consumer_evidence_available ? 0 : 1;
    out_receipt->raster_submit_blocked = 1;
    out_receipt->m11_runtime_handoff_permitted = 0;
    out_receipt->fallback_geometry_permitted = 0;
    out_receipt->fallback_visuals_permitted = 0;
    out_receipt->no_draw_only = 1;
    out_receipt->status =
        NEXUS_V1_DGN_SCENE_RUNTIME_PLAN_READY_GEOMETRY_NO_DRAW;

cleanup:
    free(vertices);
    free(normals);
    free(faces);
    free(mesh_vertices);
    free(mesh_faces);
    return out_receipt->status ==
        NEXUS_V1_DGN_SCENE_RUNTIME_PLAN_READY_GEOMETRY_NO_DRAW;
}

const char *nexus_v1_dgn_scene_runtime_plan_status_name(
    Nexus_V1_DgnSceneRuntimePlanStatus status)
{
    switch (status) {
    case NEXUS_V1_DGN_SCENE_RUNTIME_PLAN_READY_GEOMETRY_NO_DRAW:
        return "ready-geometry-no-draw";
    case NEXUS_V1_DGN_SCENE_RUNTIME_PLAN_BLOCKED_INPUT:
        return "blocked-input";
    case NEXUS_V1_DGN_SCENE_RUNTIME_PLAN_BLOCKED_VIEW_PLAN:
        return "blocked-view-plan";
    case NEXUS_V1_DGN_SCENE_RUNTIME_PLAN_BLOCKED_STRUCTURE1F:
        return "blocked-structure1f";
    case NEXUS_V1_DGN_SCENE_RUNTIME_PLAN_BLOCKED_MESH_ENTRY:
        return "blocked-mesh-entry";
    case NEXUS_V1_DGN_SCENE_RUNTIME_PLAN_BLOCKED_MESH_BUILD:
        return "blocked-mesh-build";
    default:
        return "blocked-input";
    }
}

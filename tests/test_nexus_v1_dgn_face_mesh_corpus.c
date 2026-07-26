#include "nexus_v1_dungeon.h"
#include "asset_find_by_hash.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_fail;

static const char *expected_dgn_md5(int level) {
    static const char *const hashes[16] = {
        "603ec9c531a92539babdda84ab09e78e", "751e1442bf7dccbd41bf146b5be144ab",
        "e2cb85d9fedc27f894a84e0f465fcde1", "19637d6b59849565f64565aed786d7ea",
        "85abc1b822e5c66ec4e99f1f676c140e", "ed5d54ab0ac1c927c1346dd966c8a5cc",
        "58c336ff6146e7216f0081e726823ea1", "c19e6038a017a320515ecbb66f6da197",
        "9bfc31bea631345a3660c2645be0e95b", "32a6450f29eb7babd73fcbe7a0310f22",
        "2928440e9c21457929f1323a28a42f70", "d7be5cd0d6e5c10afe99ec9950614fad",
        "db1cf70d6730615f73f191fad5e11e32", "f8876d0181d79727013236a6b597b99b",
        "a634dd5e95567ecbbbc332350c8cf12b", "5e6e237074f1e6b0decc629868a51f3c"
    };
    return level >= 0 && level < 16 ? hashes[level] : NULL;
}

#define CHECK(cond, msg) do { \
    if (!(cond)) { \
        printf("FAIL: %s\n", msg); \
        ++g_fail; \
    } \
} while (0)

static uint8_t *read_file(const char *path, int *out_size) {
    FILE *file;
    long size;
    uint8_t *data;

    if (out_size) *out_size = 0;
    file = fopen(path, "rb");
    if (!file) return NULL;
    if (fseek(file, 0, SEEK_END) != 0 ||
        (size = ftell(file)) <= 0 ||
        fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return NULL;
    }
    data = (uint8_t *)malloc((size_t)size);
    if (!data) {
        fclose(file);
        return NULL;
    }
    if (fread(data, 1, (size_t)size, file) != (size_t)size) {
        free(data);
        fclose(file);
        return NULL;
    }
    fclose(file);
    if (out_size) *out_size = (int)size;
    return data;
}

/* Hash only the bounded, typed Structure3 source rows. This makes a future
 * capture bind to the exact retail mesh corpus without assigning transform,
 * texture, palette, or draw semantics to those rows. */
static uint32_t fnv1a_byte(uint32_t hash, uint8_t value) {
    return (hash ^ value) * 16777619u;
}

static uint32_t fnv1a_u16(uint32_t hash, uint16_t value) {
    hash = fnv1a_byte(hash, (uint8_t)(value >> 8));
    return fnv1a_byte(hash, (uint8_t)value);
}

static uint32_t fnv1a_u32(uint32_t hash, uint32_t value) {
    hash = fnv1a_u16(hash, (uint16_t)(value >> 16));
    return fnv1a_u16(hash, (uint16_t)value);
}

int main(void) {
    const char *data_dir = getenv("FIRESTAFF_NEXUS_DATA_DIR");
    int level_index;
    int checked = 0;
    int entry_total = 0;
    int face_total = 0;
    int unit_pair_total = 0;
    int non_unit_pair_total = 0;
    int selector_complete_levels = 0;
    int textured_face_total = 0;
    int static_selector_total = 0;
    int static_bound_total = 0;
    int static_unique_selector_total = 0;
    int static_reused_selector_total = 0;
    int animated_selector_total = 0;
    int animated_bound_total = 0;
    int animated_unique_selector_total = 0;
    int animated_reused_selector_total = 0;
    int geometry_face_total = 0;
    int geometry_nondegenerate_total = 0;
    int geometry_degenerate_total = 0;
    int geometry_maximum_component_absolute_value = 0;
    int edge_slot_total = 0;
    int edge_nondegenerate_total = 0;
    int edge_degenerate_total = 0;
    int edge_unique_total = 0;
    int edge_boundary_total = 0;
    int edge_paired_total = 0;
    int edge_multi_incident_total = 0;
    int edge_opposite_total = 0;
    int edge_same_direction_total = 0;
    int edge_maximum_incidence = 0;
    int normal_geometry_face_total = 0;
    int normal_geometry_orthogonal_face_total = 0;
    int normal_geometry_nonorthogonal_face_total = 0;
    int normal_geometry_edge_test_total = 0;
    int normal_geometry_orthogonal_edge_test_total = 0;
    int normal_geometry_positive_total = 0;
    int normal_geometry_negative_total = 0;
    int normal_geometry_zero_total = 0;
    int capture_blocked_level_total = 0;
    uint32_t mesh_source_hash = 2166136261u;

    if (!data_dir || !data_dir[0]) {
        puts("SKIP: FIRESTAFF_NEXUS_DATA_DIR is not set");
        return 0;
    }

    for (level_index = 0; level_index < 16; ++level_index) {
        char path[2048];
        uint8_t *data;
        int size;
        Nexus_V1_Level level;
        Nexus_V1_DgnStructure3FaceReceipt faces;
        Nexus_V1_DgnStructure3FaceMaterialReceipt materials;
        Nexus_V1_DgnStructure3VectorReceipt vectors;
        Nexus_V1_DgnStructure3FaceGeometryReceipt geometry;
        Nexus_V1_DgnStructure3FaceNormalGeometryReceipt normal_geometry;
        Nexus_V1_DgnStructure3FaceEdgeReceipt edges;
        Nexus_V1_DgnStructure3FaceNormalPairReceipt pairs;
        Nexus_V1_DgnStructure3MeshSemanticHandoffReceipt mesh_semantics;
        Nexus_V1_DgnStructure3AttachmentReceipt attachments;
        Nexus_V1_DgnRenderCommand commands[NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS];
        Nexus_V1_DgnRenderPlanReceipt plan;

        snprintf(path, sizeof(path), "%s/LEV%02d.DGN", data_dir, level_index);
        data = read_file(path, &size);
        CHECK(data != NULL, "retail DGN file opens and reads");
        if (!data) continue;
        CHECK(expected_dgn_md5(level_index) &&
                  asset_file_matches_md5(path, expected_dgn_md5(level_index)),
              "retail DGN corpus file matches its canonical MD5 before parsing");
        if (!expected_dgn_md5(level_index) ||
            !asset_file_matches_md5(path, expected_dgn_md5(level_index))) {
            free(data);
            continue;
        }
        memset(&level, 0, sizeof(level));
        CHECK(nexus_v1_level_load(&level, data, size, level_index) == 0,
              "retail DGN level parses");
        if (nexus_v1_level_structure3_face_receipt(&level, &faces) != 0 ||
            nexus_v1_level_structure3_face_material_receipt(&level, &materials) != 0 ||
            nexus_v1_level_structure3_vector_receipt(&level, &vectors) != 0 ||
            nexus_v1_level_structure3_face_geometry_receipt(&level, &geometry) != 0 ||
            nexus_v1_level_structure3_face_normal_geometry_receipt(
                &level, &normal_geometry) != 0 ||
            nexus_v1_level_structure3_face_edge_receipt(&level, &edges) != 0 ||
            nexus_v1_level_structure3_face_normal_pair_receipt(&level, &pairs) != 0 ||
            nexus_v1_level_structure3_mesh_semantic_handoff_receipt(
                &level, &mesh_semantics) != 0) {
            CHECK(0, "Structure3 face/normal receipts are available");
            free(data);
            continue;
        }
        CHECK(faces.valid && vectors.valid && pairs.face_receipt_valid &&
              pairs.vector_receipt_valid && pairs.pairing_valid && pairs.valid,
              "retail Structure3 face rows pair with bounded unit normal rows");
        CHECK(pairs.entry_count == faces.entry_count &&
              pairs.complete_entry_pair_count == faces.entry_count &&
              pairs.face_normal_pair_count == faces.face_count &&
              pairs.unit_length_face_normal_pair_count == faces.normal_count &&
              pairs.non_unit_length_face_normal_pair_count == 0,
              "every retail Structure3 face has exactly one unit normal row");
        CHECK(materials.face_receipt_valid && materials.valid &&
              materials.textured_face_count == faces.textured_face_count &&
              materials.selector_bindings_complete &&
              materials.selector_reuse_accounting_valid &&
              !materials.material_or_draw_semantics_proven,
              "retail Structure3 texture selectors are bounded but remain no-draw");
        CHECK(geometry.face_receipt_valid && geometry.vector_receipt_valid &&
              geometry.valid && geometry.cross_product_measurement_safe &&
              geometry.measurement_face_count == faces.face_count &&
              geometry.nondegenerate_face_count + geometry.degenerate_face_count ==
                  geometry.measurement_face_count &&
              !geometry.surface_or_draw_semantics_proven,
              "retail face coordinates retain an accounting-complete no-draw degeneracy receipt");
        CHECK(normal_geometry.face_receipt_valid &&
              normal_geometry.vector_receipt_valid &&
              normal_geometry.face_normal_pairing_valid &&
              normal_geometry.arithmetic_envelope_safe &&
              normal_geometry.accounting_valid && normal_geometry.valid &&
              normal_geometry.measured_face_count == faces.face_count &&
              normal_geometry.orthogonal_face_count +
                      normal_geometry.nonorthogonal_face_count ==
                  normal_geometry.measured_face_count &&
              normal_geometry.orthogonal_edge_test_count <=
                  normal_geometry.edge_test_count &&
              normal_geometry.positive_cross_normal_dot_count +
                      normal_geometry.negative_cross_normal_dot_count +
                      normal_geometry.zero_cross_normal_dot_count ==
                  normal_geometry.measured_face_count &&
              !normal_geometry.normal_plane_or_draw_semantics_proven,
              "retail face-normal arithmetic remains bounded and no-draw");
        CHECK(edges.face_receipt_valid && edges.valid && edges.accounting_valid &&
              edges.face_count == faces.face_count &&
              edges.nondegenerate_face_edge_reference_count +
                      edges.degenerate_face_edge_reference_count ==
                  edges.face_edge_slot_count &&
              edges.boundary_face_edge_count + edges.paired_face_edge_count +
                      edges.multi_incident_face_edge_count ==
                  edges.unique_face_edge_count &&
              edges.opposite_direction_paired_face_edge_count +
                      edges.same_direction_paired_face_edge_count ==
                  edges.paired_face_edge_count &&
              !edges.winding_or_draw_semantics_proven,
              "retail face-row edge incidence remains bounded and no-draw");
        memset(commands, 0, sizeof(commands));
        CHECK(nexus_v1_level_build_dgn_view_render_plan(
                  &level, 0, 0, 0, commands,
                  NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS, &plan) == 0 &&
              plan.structure3_face_materials.valid &&
              plan.structure3_face_materials.selector_bindings_complete &&
              plan.structure3_face_normal_pairs.valid &&
              plan.structure3_face_normal_pairs.face_normal_pair_count ==
                  faces.face_count &&
              plan.structure3_face_geometry.valid &&
              plan.structure3_face_geometry.measurement_face_count ==
                  faces.face_count &&
              plan.structure3_face_geometry.nondegenerate_face_count ==
                  faces.face_count &&
              plan.structure3_face_edges.valid &&
              plan.structure3_face_edges.face_edge_slot_count ==
                  edges.face_edge_slot_count &&
              plan.structure3_face_normal_geometry.valid &&
              plan.structure3_face_normal_geometry.measured_face_count ==
                  faces.face_count &&
              plan.structure3_attachments.complete &&
              plan.structure3_attachments.record_to_face_normal_semantics_proven &&
              !plan.structure3_attachments.normal_plane_transform_or_draw_semantics_proven &&
              !plan.structure3_face_edges.winding_or_draw_semantics_proven &&
              !plan.structure3_face_geometry.surface_or_draw_semantics_proven &&
              !plan.structure3_face_materials.material_or_draw_semantics_proven &&
              !plan.structure3_face_normal_pairs.normal_plane_or_draw_semantics_proven,
              "renderer-facing plan preserves retail Structure3 no-draw receipts");
        CHECK(!pairs.normal_plane_or_draw_semantics_proven &&
              !vectors.transform_or_draw_semantics_proven &&
              !faces.draw_semantics_proven,
              "face-normal rows do not authorize plane or draw semantics");
        CHECK(mesh_semantics.source_facts_complete &&
              mesh_semantics.source_face_geometry_valid &&
              mesh_semantics.source_face_normal_geometry_valid &&
              mesh_semantics.entry_count == pairs.entry_count &&
              mesh_semantics.face_count == pairs.face_normal_pair_count &&
              mesh_semantics.normal_count == pairs.face_normal_pair_count &&
              mesh_semantics.original_capture_required &&
              !mesh_semantics.original_capture_available &&
              !mesh_semantics.normal_plane_semantics_proven &&
              !mesh_semantics.transform_semantics_proven &&
              !mesh_semantics.texture_palette_semantics_proven &&
              !mesh_semantics.draw_semantics_proven &&
              !mesh_semantics.renderer_handoff_ready &&
              mesh_semantics.blocks_real_dgn_mesh_render,
              "retail mesh evidence reaches a capture-blocked renderer handoff");
        CHECK(nexus_v1_level_structure3_attachment_receipt(&level, &attachments) == 0 &&
              attachments.complete && attachments.record_to_face_normal_semantics_proven &&
              attachments.structure1f_bound_entry_count ==
                  attachments.face_normal_bound_count &&
              attachments.out_of_range_model_selector_count == 0 &&
              attachments.out_of_range_face_selector_count == 0 &&
              !attachments.normal_plane_transform_or_draw_semantics_proven,
              "hash-verified Structure1A/Structure1F selectors bind only to bounded face-normal ordinals");
        {
            Nexus_V1_Level mutated = level;
            Nexus_V1_DgnStructure3AttachmentReceipt rejected;
            int record_index;

            for (record_index = 0;
                 record_index < mutated.structure1f_entry_count;
                 ++record_index) {
                if (mutated.structure1f_entries[record_index].family >=
                    NEXUS_V1_DGN_STRUCTURE1F_ALCOVES) {
                    mutated.structure1f_entries[record_index]
                        .structure1a_structure3_model_index =
                        (uint8_t)mutated.structure3_directory.entry_count;
                    break;
                }
            }
            if (record_index < mutated.structure1f_entry_count) {
                CHECK(nexus_v1_level_structure3_attachment_receipt(
                          &mutated, &rejected) == 0 && !rejected.complete &&
                      !rejected.record_to_face_normal_semantics_proven &&
                      rejected.out_of_range_model_selector_count == 1 &&
                      !rejected.normal_plane_transform_or_draw_semantics_proven,
                      "an out-of-range Structure1A model selector rejects the whole attachment receipt");
            }
        }
        {
            Nexus_V1_DgnStructure3FaceCaptureCandidate candidate;
            Nexus_V1_DgnStructure3FaceCaptureBindingReceipt capture;

            memset(&candidate, 0, sizeof(candidate));
            CHECK(nexus_v1_dgn_bind_structure3_face_capture_candidate(
                      &level, data, size, 0, 0, &candidate, NULL, 0, NULL, 0,
                      NULL, 0, NULL, 0, NULL, 0, NULL, 0, &capture) != 0 &&
                  !capture.candidate_framing_valid &&
                  !capture.complete_source_binding && !capture.renderer_handoff_ready &&
                  capture.blocks_real_dgn_mesh_render,
                  "a retail DGN level stays blocked until an original capture supplies every span");
            if (!capture.candidate_framing_valid &&
                !capture.complete_source_binding &&
                !capture.renderer_handoff_ready &&
                capture.blocks_real_dgn_mesh_render) {
                ++capture_blocked_level_total;
            }
        }
        {
            int entry_index;
            int extracted_vertex_total = 0;
            int extracted_face_total = 0;
            int extracted_normal_total = 0;

            for (entry_index = 0; entry_index < faces.entry_count; ++entry_index) {
                Nexus_V1_DgnStructure3MeshEntryReceipt mesh_entry;
                Nexus_V1_DgnStructure3Vector *vertices;
                Nexus_V1_DgnStructure3Face *mesh_faces;
                Nexus_V1_DgnStructure3Vector *normals;
                int face_index;

                memset(&mesh_entry, 0, sizeof(mesh_entry));
                (void)nexus_v1_level_extract_structure3_mesh_entry(
                    &level, data, size, entry_index, NULL, 0, NULL, 0,
                    NULL, 0, &mesh_entry);
                CHECK(mesh_entry.source_identity_valid &&
                      mesh_entry.vertex_count >= 0 && mesh_entry.face_count >= 0 &&
                      mesh_entry.normal_count == mesh_entry.face_count,
                      "mesh extractor reports bounded entry requirements without partial rows");
                vertices = (Nexus_V1_DgnStructure3Vector *)calloc(
                    (size_t)mesh_entry.vertex_count, sizeof(*vertices));
                mesh_faces = (Nexus_V1_DgnStructure3Face *)calloc(
                    (size_t)mesh_entry.face_count, sizeof(*mesh_faces));
                normals = (Nexus_V1_DgnStructure3Vector *)calloc(
                    (size_t)mesh_entry.normal_count, sizeof(*normals));
                CHECK(nexus_v1_level_extract_structure3_mesh_entry(
                          &level, data, size, entry_index, vertices,
                          mesh_entry.vertex_count, mesh_faces, mesh_entry.face_count,
                          normals, mesh_entry.normal_count, &mesh_entry) == 0 &&
                      mesh_entry.valid && mesh_entry.source_identity_valid &&
                      !mesh_entry.transform_or_draw_semantics_proven,
                      "mesh extractor copies bounded typed source rows without draw semantics");
                mesh_source_hash = fnv1a_byte(mesh_source_hash,
                                               (uint8_t)level_index);
                mesh_source_hash = fnv1a_u16(mesh_source_hash,
                                              (uint16_t)entry_index);
                mesh_source_hash = fnv1a_u16(mesh_source_hash,
                                              (uint16_t)mesh_entry.vertex_count);
                mesh_source_hash = fnv1a_u16(mesh_source_hash,
                                              (uint16_t)mesh_entry.face_count);
                for (face_index = 0; face_index < mesh_entry.vertex_count;
                     ++face_index) {
                    mesh_source_hash = fnv1a_u32(mesh_source_hash,
                        (uint32_t)vertices[face_index].x);
                    mesh_source_hash = fnv1a_u32(mesh_source_hash,
                        (uint32_t)vertices[face_index].y);
                    mesh_source_hash = fnv1a_u32(mesh_source_hash,
                        (uint32_t)vertices[face_index].z);
                }
                extracted_vertex_total += mesh_entry.vertex_count;
                extracted_face_total += mesh_entry.face_count;
                extracted_normal_total += mesh_entry.normal_count;
                for (face_index = 0; face_index < mesh_entry.face_count;
                     ++face_index) {
                    int slot_count = mesh_faces[face_index].triangle ? 3 : 4;
                    int slot;

                    mesh_source_hash = fnv1a_u16(mesh_source_hash,
                                                   mesh_faces[face_index].vertex_indexes[0]);
                    mesh_source_hash = fnv1a_u16(mesh_source_hash,
                                                   mesh_faces[face_index].vertex_indexes[1]);
                    mesh_source_hash = fnv1a_u16(mesh_source_hash,
                                                   mesh_faces[face_index].vertex_indexes[2]);
                    mesh_source_hash = fnv1a_u16(mesh_source_hash,
                                                   mesh_faces[face_index].vertex_indexes[3]);
                    mesh_source_hash = fnv1a_byte(mesh_source_hash,
                                                   mesh_faces[face_index].flags);
                    mesh_source_hash = fnv1a_byte(mesh_source_hash,
                                                   mesh_faces[face_index].raw_byte_9);
                    mesh_source_hash = fnv1a_u16(mesh_source_hash,
                                                   mesh_faces[face_index].fill_selector);
                    for (slot = 0; slot < slot_count; ++slot) {
                        CHECK(mesh_faces[face_index].vertex_indexes[slot] <
                                  mesh_entry.vertex_count,
                              "typed mesh face indexes remain entry-local and bounded");
                    }
                }
                for (face_index = 0; face_index < mesh_entry.normal_count;
                     ++face_index) {
                    mesh_source_hash = fnv1a_u32(mesh_source_hash,
                        (uint32_t)normals[face_index].x);
                    mesh_source_hash = fnv1a_u32(mesh_source_hash,
                        (uint32_t)normals[face_index].y);
                    mesh_source_hash = fnv1a_u32(mesh_source_hash,
                        (uint32_t)normals[face_index].z);
                }
                free(vertices);
                free(mesh_faces);
                free(normals);
            }
            CHECK(extracted_vertex_total == faces.vertex_count &&
                  extracted_face_total == faces.face_count &&
                  extracted_normal_total == faces.normal_count,
                  "typed mesh rows preserve every corpus vertex face and normal count");
        }
        {
            Nexus_V1_DgnStructure3MeshEntryReceipt tampered_receipt;
            uint8_t *tampered = (uint8_t *)malloc((size_t)size);

            CHECK(tampered != NULL, "tamper fixture allocates");
            if (tampered) {
                memcpy(tampered, data, (size_t)size);
                tampered[level.structure3_payload.byte_offset] ^= 0x01U;
                memset(&tampered_receipt, 0, sizeof(tampered_receipt));
                CHECK(nexus_v1_level_extract_structure3_mesh_entry(
                          &level, tampered, size, 0, NULL, 0, NULL, 0,
                          NULL, 0, &tampered_receipt) != 0 &&
                      !tampered_receipt.source_identity_valid &&
                      !tampered_receipt.valid,
                      "mesh extractor rejects a payload that differs from the loaded source");
                free(tampered);
            }
        }
        free(data);
        ++checked;
        entry_total += pairs.entry_count;
        face_total += pairs.face_normal_pair_count;
        unit_pair_total += pairs.unit_length_face_normal_pair_count;
        non_unit_pair_total += pairs.non_unit_length_face_normal_pair_count;
        textured_face_total += materials.textured_face_count;
        static_selector_total += materials.static_texture_selector_count;
        static_bound_total += materials.static_texture_bound_count;
        static_unique_selector_total += materials.static_texture_unique_selector_count;
        static_reused_selector_total += materials.static_texture_reused_selector_count;
        animated_selector_total += materials.animated_texture_selector_count;
        animated_bound_total += materials.animated_texture_bound_count;
        animated_unique_selector_total += materials.animated_texture_unique_selector_count;
        animated_reused_selector_total += materials.animated_texture_reused_selector_count;
        geometry_face_total += geometry.measurement_face_count;
        geometry_nondegenerate_total += geometry.nondegenerate_face_count;
        geometry_degenerate_total += geometry.degenerate_face_count;
        edge_slot_total += edges.face_edge_slot_count;
        edge_nondegenerate_total += edges.nondegenerate_face_edge_reference_count;
        edge_degenerate_total += edges.degenerate_face_edge_reference_count;
        edge_unique_total += edges.unique_face_edge_count;
        edge_boundary_total += edges.boundary_face_edge_count;
        edge_paired_total += edges.paired_face_edge_count;
        edge_multi_incident_total += edges.multi_incident_face_edge_count;
        edge_opposite_total += edges.opposite_direction_paired_face_edge_count;
        edge_same_direction_total += edges.same_direction_paired_face_edge_count;
        normal_geometry_face_total += normal_geometry.measured_face_count;
        normal_geometry_orthogonal_face_total += normal_geometry.orthogonal_face_count;
        normal_geometry_nonorthogonal_face_total += normal_geometry.nonorthogonal_face_count;
        normal_geometry_edge_test_total += normal_geometry.edge_test_count;
        normal_geometry_orthogonal_edge_test_total +=
            normal_geometry.orthogonal_edge_test_count;
        normal_geometry_positive_total += normal_geometry.positive_cross_normal_dot_count;
        normal_geometry_negative_total += normal_geometry.negative_cross_normal_dot_count;
        normal_geometry_zero_total += normal_geometry.zero_cross_normal_dot_count;
        if (edges.maximum_face_edge_incidence > edge_maximum_incidence)
            edge_maximum_incidence = edges.maximum_face_edge_incidence;
        if (geometry.maximum_component_absolute_value >
            geometry_maximum_component_absolute_value) {
            geometry_maximum_component_absolute_value =
                geometry.maximum_component_absolute_value;
        }
        if (materials.selector_bindings_complete) ++selector_complete_levels;
    }

    printf("Structure3 face-normal corpus: levels=%d entries=%d pairs=%d unit=%d nonunit=%d\n",
           checked, entry_total, face_total, unit_pair_total, non_unit_pair_total);
    printf("Structure3 texture-selector corpus: textured=%d static=%d/%d unique=%d reused=%d animated=%d/%d unique=%d reused=%d\n",
           textured_face_total, static_bound_total, static_selector_total,
           static_unique_selector_total, static_reused_selector_total,
           animated_bound_total, animated_selector_total,
           animated_unique_selector_total, animated_reused_selector_total);
    printf("Structure3 typed mesh source hash: %08x\n", mesh_source_hash);
    printf("Structure3 geometric-degeneracy corpus: faces=%d nondegenerate=%d degenerate=%d max-component=%d\n",
           geometry_face_total, geometry_nondegenerate_total,
           geometry_degenerate_total, geometry_maximum_component_absolute_value);
    printf("Structure3 face-edge corpus: slots=%d nondegenerate=%d degenerate=%d unique=%d boundary=%d paired=%d multi-incident=%d opposite=%d same=%d max-incidence=%d\n",
           edge_slot_total, edge_nondegenerate_total, edge_degenerate_total,
           edge_unique_total, edge_boundary_total, edge_paired_total,
           edge_multi_incident_total, edge_opposite_total, edge_same_direction_total,
           edge_maximum_incidence);
    printf("Structure3 face-normal geometry corpus: faces=%d orthogonal=%d nonorthogonal=%d edges=%d orthogonal-edges=%d positive=%d negative=%d zero=%d\n",
           normal_geometry_face_total, normal_geometry_orthogonal_face_total,
           normal_geometry_nonorthogonal_face_total, normal_geometry_edge_test_total,
           normal_geometry_orthogonal_edge_test_total, normal_geometry_positive_total,
           normal_geometry_negative_total, normal_geometry_zero_total);
    printf("Structure3 source-only capture gate: blocked-levels=%d\n",
           capture_blocked_level_total);
    CHECK(checked == 16, "all retail LEV00 through LEV15 files were checked");
    CHECK(entry_total == 1144 && face_total == 18478 &&
          unit_pair_total == 18478 && non_unit_pair_total == 0,
          "retail face-normal pair totals remain corpus-verified and no-draw");
    CHECK(selector_complete_levels == 16,
          "all retail levels preserve selector joins through the renderer-facing receipt");
    CHECK(textured_face_total == 17821 &&
              static_selector_total == 17401 && static_bound_total == 17401 &&
              animated_selector_total == 420 && animated_bound_total == 420,
          "retail texture selector joins remain corpus-verified without decoding pixels");
    CHECK(static_unique_selector_total + static_reused_selector_total ==
              static_selector_total &&
              animated_unique_selector_total + animated_reused_selector_total ==
                  animated_selector_total,
          "retail selector reuse remains completely accounted without texture semantics");
    CHECK(static_unique_selector_total == 1291 &&
              static_reused_selector_total == 16110 &&
              animated_unique_selector_total == 44 &&
              animated_reused_selector_total == 376,
          "retail selector identity reuse remains corpus-locked without decoding textures");
    CHECK(mesh_source_hash == 0xd3f42b1fu,
          "typed mesh source rows retain the verified retail corpus identity");
    CHECK(geometry_face_total == 18478 &&
              geometry_nondegenerate_total == 18478 &&
              geometry_degenerate_total == 0 &&
              geometry_maximum_component_absolute_value == 450560,
          "retail face geometry remains nondegenerate within the measured coordinate envelope");
    CHECK(edge_slot_total == 73226 && edge_nondegenerate_total == 73041 &&
              edge_degenerate_total == 185 && edge_unique_total == 47321 &&
              edge_boundary_total == 22240 && edge_paired_total == 24739 &&
              edge_multi_incident_total == 342 && edge_opposite_total == 20962 &&
              edge_same_direction_total == 3777 && edge_maximum_incidence == 4,
          "retail face-edge incidence remains corpus-locked without winding semantics");
    CHECK(normal_geometry_face_total == 18478 &&
              normal_geometry_orthogonal_face_total == 11876 &&
              normal_geometry_nonorthogonal_face_total == 6602 &&
              normal_geometry_edge_test_total == 54748 &&
              normal_geometry_orthogonal_edge_test_total == 39003 &&
              normal_geometry_positive_total == 15877 &&
              normal_geometry_negative_total == 2601 &&
              normal_geometry_zero_total == 0,
          "retail face-normal arithmetic remains corpus-locked without normal-use semantics");
    CHECK(capture_blocked_level_total == 16,
          "every retail DGN level rejects source-only capture binding and remains no-draw");
    return g_fail == 0 ? 0 : 1;
}

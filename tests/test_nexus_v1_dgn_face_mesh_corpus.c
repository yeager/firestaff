#include "nexus_v1_dungeon.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_fail;

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
    uint32_t mesh_source_hash = 2166136261u;

    if (!data_dir || !data_dir[0]) {
        puts("SKIP: FIRESTAFF_NEXUS_DATA_DIR is not set");
        return 0;
    }

    for (level_index = 0; level_index < 16; ++level_index) {
        char path[1024];
        uint8_t *data;
        int size;
        Nexus_V1_Level level;
        Nexus_V1_DgnStructure3FaceReceipt faces;
        Nexus_V1_DgnStructure3FaceMaterialReceipt materials;
        Nexus_V1_DgnStructure3VectorReceipt vectors;
        Nexus_V1_DgnStructure3FaceNormalPairReceipt pairs;
        Nexus_V1_DgnStructure3MeshSemanticHandoffReceipt mesh_semantics;
        Nexus_V1_DgnRenderCommand commands[NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS];
        Nexus_V1_DgnRenderPlanReceipt plan;

        snprintf(path, sizeof(path), "%s/LEV%02d.DGN", data_dir, level_index);
        data = read_file(path, &size);
        CHECK(data != NULL, "retail DGN file opens and reads");
        if (!data) continue;
        memset(&level, 0, sizeof(level));
        CHECK(nexus_v1_level_load(&level, data, size, level_index) == 0,
              "retail DGN level parses");
        if (nexus_v1_level_structure3_face_receipt(&level, &faces) != 0 ||
            nexus_v1_level_structure3_face_material_receipt(&level, &materials) != 0 ||
            nexus_v1_level_structure3_vector_receipt(&level, &vectors) != 0 ||
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
        memset(commands, 0, sizeof(commands));
        CHECK(nexus_v1_level_build_dgn_view_render_plan(
                  &level, 0, 0, 0, commands,
                  NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS, &plan) == 0 &&
              plan.structure3_face_materials.valid &&
              plan.structure3_face_materials.selector_bindings_complete &&
              plan.structure3_face_normal_pairs.valid &&
              plan.structure3_face_normal_pairs.face_normal_pair_count ==
                  faces.face_count &&
              !plan.structure3_face_materials.material_or_draw_semantics_proven &&
              !plan.structure3_face_normal_pairs.normal_plane_or_draw_semantics_proven,
              "renderer-facing plan preserves retail Structure3 no-draw receipts");
        CHECK(!pairs.normal_plane_or_draw_semantics_proven &&
              !vectors.transform_or_draw_semantics_proven &&
              !faces.draw_semantics_proven,
              "face-normal rows do not authorize plane or draw semantics");
        CHECK(mesh_semantics.source_facts_complete &&
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
    return g_fail == 0 ? 0 : 1;
}

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

int main(void) {
    const char *data_dir = getenv("FIRESTAFF_NEXUS_DATA_DIR");
    int level_index;
    int checked = 0;
    int entry_total = 0;
    int face_total = 0;
    int unit_pair_total = 0;
    int non_unit_pair_total = 0;

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
        Nexus_V1_DgnStructure3VectorReceipt vectors;
        Nexus_V1_DgnStructure3FaceNormalPairReceipt pairs;
        Nexus_V1_DgnStructure3MeshSemanticHandoffReceipt mesh_semantics;

        snprintf(path, sizeof(path), "%s/LEV%02d.DGN", data_dir, level_index);
        data = read_file(path, &size);
        CHECK(data != NULL, "retail DGN file opens and reads");
        if (!data) continue;
        memset(&level, 0, sizeof(level));
        CHECK(nexus_v1_level_load(&level, data, size, level_index) == 0,
              "retail DGN level parses");
        free(data);
        if (nexus_v1_level_structure3_face_receipt(&level, &faces) != 0 ||
            nexus_v1_level_structure3_vector_receipt(&level, &vectors) != 0 ||
            nexus_v1_level_structure3_face_normal_pair_receipt(&level, &pairs) != 0 ||
            nexus_v1_level_structure3_mesh_semantic_handoff_receipt(
                &level, &mesh_semantics) != 0) {
            CHECK(0, "Structure3 face/normal receipts are available");
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
        ++checked;
        entry_total += pairs.entry_count;
        face_total += pairs.face_normal_pair_count;
        unit_pair_total += pairs.unit_length_face_normal_pair_count;
        non_unit_pair_total += pairs.non_unit_length_face_normal_pair_count;
    }

    printf("Structure3 face-normal corpus: levels=%d entries=%d pairs=%d unit=%d nonunit=%d\n",
           checked, entry_total, face_total, unit_pair_total, non_unit_pair_total);
    CHECK(checked == 16, "all retail LEV00 through LEV15 files were checked");
    CHECK(entry_total == 1144 && face_total == 18478 &&
          unit_pair_total == 18478 && non_unit_pair_total == 0,
          "retail face-normal pair totals remain corpus-verified and no-draw");
    return g_fail == 0 ? 0 : 1;
}

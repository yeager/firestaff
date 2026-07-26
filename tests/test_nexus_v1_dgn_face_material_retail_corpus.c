#include "asset_find_by_hash.h"
#include "nexus_v1_dgn_face_material_provenance.h"
#include "nexus_v1_dungeon.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int failures;

#define CHECK(condition, message) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAIL: %s\n", message); \
        ++failures; \
    } \
} while (0)

static const char *expected_dgn_md5(int level_index)
{
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

    return level_index >= 0 && level_index < 16 ? hashes[level_index] : NULL;
}

static uint8_t *read_file(const char *path, int *out_size)
{
    FILE *file;
    long size;
    uint8_t *data;

    *out_size = 0;
    file = fopen(path, "rb");
    if (!file || fseek(file, 0, SEEK_END) != 0 ||
        (size = ftell(file)) <= 0 || fseek(file, 0, SEEK_SET) != 0) {
        if (file) fclose(file);
        return NULL;
    }
    data = (uint8_t *)malloc((size_t)size);
    if (!data || fread(data, 1, (size_t)size, file) != (size_t)size) {
        free(data);
        fclose(file);
        return NULL;
    }
    fclose(file);
    *out_size = (int)size;
    return data;
}

int main(void)
{
    const char *data_dir = getenv("FIRESTAFF_NEXUS_DATA_DIR");
    int admitted_levels = 0;
    int textured_face_total = 0;
    int static_selector_total = 0;
    int animated_selector_total = 0;
    int level_index;
    char first_path[1024];

    if (!data_dir || !data_dir[0]) {
        puts("SKIP: FIRESTAFF_NEXUS_DATA_DIR is not set");
        return 77;
    }
    snprintf(first_path, sizeof(first_path), "%s/LEV00.DGN", data_dir);
    if (access(first_path, R_OK) != 0) {
        puts("SKIP: retail Nexus DGN corpus is not staged");
        return 77;
    }

    for (level_index = 0; level_index < 16; ++level_index) {
        char path[2048];
        uint8_t *data;
        int size;
        int binding_count = 0;
        int geometry_ready;
        int geometry_face_total = 0;
        Nexus_V1_Level level;
        Nexus_V1_DgnStructure3FaceReceipt faces;
        Nexus_V1_DgnStructure3FaceMaterialReceipt parsed_materials;
        Nexus_V1_DgnFaceMaterialBinding bindings[
            NEXUS_V1_DGN_FACE_MATERIAL_MAX_FACES];
        Nexus_V1_DgnFaceMaterialInput input;
        Nexus_V1_DgnFaceMaterialReceipt receipt;

        snprintf(path, sizeof(path), "%s/LEV%02d.DGN", data_dir, level_index);
        data = read_file(path, &size);
        CHECK(data != NULL && expected_dgn_md5(level_index) &&
                  asset_file_matches_md5(path, expected_dgn_md5(level_index)),
              "each retail DGN buffer is present and hash-verified");
        if (!data || !expected_dgn_md5(level_index) ||
            !asset_file_matches_md5(path, expected_dgn_md5(level_index))) {
            free(data);
            continue;
        }

        memset(&level, 0, sizeof(level));
        CHECK(nexus_v1_level_load(&level, data, size, level_index) == 0 &&
                  nexus_v1_level_structure3_face_receipt(&level, &faces) == 0 &&
                  nexus_v1_level_structure3_face_material_receipt(
                      &level, &parsed_materials) == 0,
              "retail DGN parser supplies bounded face, mesh, and material receipts");
        CHECK(nexus_v1_level_collect_structure3_face_material_bindings(
                  &level, data, size, bindings,
                  NEXUS_V1_DGN_FACE_MATERIAL_MAX_FACES, &binding_count) == 0 &&
                  binding_count == parsed_materials.textured_face_count,
              "retail selector bindings account for every parsed textured face");

        /* Geometry readiness comes from the restored Structure3 mesh
         * extractor (the round-14 scene_runtime_plan route), not from the
         * stale pre-restoration level.geometry_info.mesh_ready field,
         * which stays 0 for the whole retail corpus. */
        geometry_ready = nexus_v1_level_structure3_mesh_geometry_ready(
            &level, data, size, &geometry_face_total);
        CHECK(geometry_ready && geometry_face_total == faces.face_count,
              "restored mesh extractor binds every retail mesh entry as ready geometry without raster or fallback");

        memset(&input, 0, sizeof(input));
        input.source = NEXUS_V1_DGN_FACE_MATERIAL_SOURCE_RETAIL_DGN;
        input.dgn_bytes = data;
        input.dgn_size = size;
        input.canonical_dgn_bytes = data;
        input.canonical_dgn_size = size;
        input.canonical_source_verified = 1;
        input.bindings = bindings;
        input.face_count = binding_count;
        input.structure2_descriptor_count = level.structure2_texture_count;
        input.material_selector_count = 256;
        input.geometry_source_bound = geometry_ready;
        input.geometry_material_face_count = binding_count;
        input.geometry_can_submit_geometry = geometry_ready;
        input.geometry_can_submit_textured_raster = 0;
        input.geometry_fallback_visuals_permitted = 0;
        CHECK(nexus_v1_dgn_face_material_validate(&input, &receipt) == 1 &&
                  receipt.status == NEXUS_V1_DGN_FACE_MATERIAL_READY &&
                  receipt.face_count == binding_count &&
                  receipt.static_selector_count ==
                      parsed_materials.static_texture_selector_count &&
                  receipt.animated_selector_count ==
                      parsed_materials.animated_texture_selector_count &&
                  receipt.no_draw_only && receipt.blocks_real_dgn_mesh_render &&
                  !receipt.can_submit_raster_input &&
                  !receipt.permits_fallback_visuals,
              "retail face/mesh material admission remains source-bound and no-draw");
        if (receipt.status == NEXUS_V1_DGN_FACE_MATERIAL_READY) {
            ++admitted_levels;
            textured_face_total += receipt.face_count;
            static_selector_total += receipt.static_selector_count;
            animated_selector_total += receipt.animated_selector_count;
        }
        free(data);
    }

    CHECK(admitted_levels == 16, "all retail LEV00 through LEV15 packages admit");
    CHECK(textured_face_total == 17821 && static_selector_total == 17401 &&
              animated_selector_total == 420,
          "retail face/material admission retains the locked selector census");
    if (failures) return 1;
    puts("Nexus DGN retail face/material admission corpus passed");
    return 0;
}

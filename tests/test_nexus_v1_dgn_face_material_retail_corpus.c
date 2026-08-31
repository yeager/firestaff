#include "nexus_v1_dgn_face_material_provenance.h"
#include "nexus_v1_engine.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures;

#define CHECK(condition, message) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAIL: %s\n", message); \
        ++failures; \
    } \
} while (0)

int main(void)
{
    const char *data_dir = getenv("FIRESTAFF_NEXUS_DATA_DIR");
    int admitted_levels = 0;
    int textured_face_total = 0;
    int static_selector_total = 0;
    int animated_selector_total = 0;
    int level_index;
    Nexus_V1_Engine source_engine;

    if (!data_dir || !data_dir[0]) {
        puts("SKIP: FIRESTAFF_NEXUS_DATA_DIR is not set");
        return 77;
    }
    memset(&source_engine, 0, sizeof(source_engine));
    if (nexus_v1_init(&source_engine, data_dir) != 0) {
        puts("SKIP: retail Nexus source is unavailable");
        return 77;
    }

    for (level_index = 0; level_index < 16; ++level_index) {
        char name[16];
        uint8_t *data;
        int size;
        Nexus_V1_LevelAuxSourceReceipt source;
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

        snprintf(name, sizeof(name), "LEV%02d.DGN", level_index);
        memset(&source, 0, sizeof(source));
        (void)nexus_v1_named_asset_source_receipt(&source_engine, name, &source);
        data = nexus_v1_read_file(&source_engine, name, &size);
        CHECK(data != NULL && size > 0 && source.canonical_hash_verified,
              "each retail DGN member is readable and hash-verified in its original container");
        if (!data || size <= 0 || !source.canonical_hash_verified) {
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

    nexus_v1_shutdown(&source_engine);

    CHECK(admitted_levels == 16, "all retail LEV00 through LEV15 packages admit");
    CHECK(textured_face_total == 17821 && static_selector_total == 17401 &&
              animated_selector_total == 420,
          "retail face/material admission retains the locked selector census");
    if (failures) return 1;
    puts("Nexus DGN retail face/material admission corpus passed");
    return 0;
}

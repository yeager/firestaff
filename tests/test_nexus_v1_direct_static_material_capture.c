#include "nexus_v1_engine.h"
#include "asset_find_by_hash.h"

#include <stdint.h>
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
    char path[1024];
    FILE *file;
    long file_size;
    uint8_t *data = NULL;
    Nexus_V1_Engine engine;
    Nexus_V1_Level level;
    int source_entry;
    int found = 0;

    if (!data_dir || !data_dir[0]) {
        puts("skip: FIRESTAFF_NEXUS_DATA_DIR is not set");
        return 0;
    }
    CHECK(snprintf(path, sizeof(path), "%s/LEV01.DGN", data_dir) <
              (int)sizeof(path) &&
          asset_file_matches_md5(path, "751e1442bf7dccbd41bf146b5be144ab"),
          "canonical LEV01 source is hash verified");
    file = failures == 0 ? fopen(path, "rb") : NULL;
    CHECK(file != NULL, "canonical LEV01 opens");
    if (file) {
        CHECK(fseek(file, 0L, SEEK_END) == 0 &&
              (file_size = ftell(file)) > 0L &&
              fseek(file, 0L, SEEK_SET) == 0,
              "canonical LEV01 has bounded source bytes");
        if (failures == 0) {
            data = (uint8_t *)malloc((size_t)file_size);
            CHECK(data != NULL && fread(data, 1U, (size_t)file_size, file) ==
                  (size_t)file_size, "canonical LEV01 reads completely");
        }
        fclose(file);
    }
    memset(&engine, 0, sizeof(engine));
    memset(&level, 0, sizeof(level));
    if (failures == 0) {
        CHECK(nexus_v1_level_load(&level, data, (int)file_size, 1) == 0,
              "canonical LEV01 parses before direct material selection");
    }
    if (failures == 0) {
        engine.level_loaded = 1;
        engine.game.current_level = 1;
        engine.current_level = level;
        engine.current_level_dgn_data = data;
        engine.current_level_dgn_size = (int)file_size;
        engine.current_level_structure2_source.level_index = 1;
        engine.current_level_structure2_source.canonical_hash_verified = 1;
        engine.current_level_structure2_source.materialization_bound = 1;
        engine.current_level_structure2_source.structure2_payload_envelope_valid = 1;
        engine.current_level_structure2_source.loaded_bytes_bound = 1;
        engine.current_level_structure2_source.loaded_dgn_size = (int)file_size;
        /* Match the active source receipt's FNV-1a identity requirement. */
        {
            uint64_t hash = UINT64_C(1469598103934665603);
            size_t index;
            for (index = 0U; index < (size_t)file_size; ++index) {
                hash ^= data[index];
                hash *= UINT64_C(1099511628211);
            }
            engine.current_level_structure2_source.loaded_dgn_fnv1a64 = hash;
        }
    }
    if (failures == 0) {
        for (source_entry = 0;
             source_entry < engine.current_level.structure1f_entry_count;
             ++source_entry) {
            Nexus_V1_DgnStructure1FDirectStaticMaterialCaptureTarget target;

            memset(&target, 0, sizeof(target));
            if (nexus_v1_engine_build_structure1f_direct_static_material_capture_target(
                    &engine, source_entry, &target) != 1) {
                continue;
            }
            found = 1;
            CHECK(target.valid && target.direct_face_material_bound &&
                  target.direct_mesh.structure1f_entry_index == source_entry &&
                  target.static_material.structure3_entry_index ==
                      target.direct_mesh.structure3_model_index &&
                  target.static_material.face_ordinal ==
                      target.direct_mesh.face_ordinal &&
                  target.static_material.static_selector_descriptor_bound &&
                  target.static_material.image_payload_anchor_bound &&
                  target.static_material.image_payload_interval_bound &&
                  target.capture_producer_required &&
                  target.original_saturn_capture_required &&
                  target.no_draw_only && !target.fallback_visuals_permitted &&
                  target.blocks_real_dgn_mesh_render,
                  "direct owner selects only its exact static source material lane");
            break;
        }
        CHECK(!found,
              "canonical LEV01 direct owners cannot invent a static material lane");
    }
    {
        Nexus_V1_DgnStructure1FDirectStaticMaterialCaptureTarget target;
        memset(&target, 0, sizeof(target));
        CHECK(nexus_v1_engine_build_structure1f_direct_static_material_capture_target(
                  &engine, engine.current_level.structure1f_entry_count, &target) == 0 &&
              !target.valid && target.no_draw_only &&
              !target.fallback_visuals_permitted && target.blocks_real_dgn_mesh_render,
              "out-of-range owner cannot manufacture a material or fallback");
    }
    free(data);
    if (failures != 0) {
        fprintf(stderr, "test_nexus_v1_direct_static_material_capture: FAIL %d\n",
                failures);
        return 1;
    }
    puts("test_nexus_v1_direct_static_material_capture: PASS");
    return 0;
}

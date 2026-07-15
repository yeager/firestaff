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

static uint64_t fnv1a64(const uint8_t *data, size_t size)
{
    uint64_t hash = UINT64_C(1469598103934665603);
    size_t index;
    for (index = 0U; index < size; ++index) {
        hash ^= data[index];
        hash *= UINT64_C(1099511628211);
    }
    return hash;
}

static int count_canonical_transform_capture_targets(const char *data_dir)
{
    int level_index;
    int target_count = 0;
    for (level_index = 0; level_index < 16; ++level_index) {
        char name[16], path[1024];
        FILE *file;
        long file_size;
        uint8_t *data = NULL;
        Nexus_V1_Engine engine;
        Nexus_V1_Level level;
        int source_entry;

        snprintf(name, sizeof(name), "LEV%02d.DGN", level_index);
        if (!nexus_v1_known_file_md5(name) ||
            snprintf(path, sizeof(path), "%s/%s", data_dir, name) >=
                (int)sizeof(path) ||
            !asset_file_matches_md5(path, nexus_v1_known_file_md5(name)) ||
            !(file = fopen(path, "rb"))) return -1;
        if (fseek(file, 0L, SEEK_END) != 0 || (file_size = ftell(file)) <= 0L ||
            fseek(file, 0L, SEEK_SET) != 0 ||
            !(data = (uint8_t *)malloc((size_t)file_size)) ||
            fread(data, 1U, (size_t)file_size, file) != (size_t)file_size) {
            free(data);
            fclose(file);
            return -1;
        }
        fclose(file);
        memset(&engine, 0, sizeof(engine));
        memset(&level, 0, sizeof(level));
        if (nexus_v1_level_load(&level, data, (int)file_size, level_index) != 0) {
            free(data);
            return -1;
        }
        engine.level_loaded = 1;
        engine.game.current_level = level_index;
        engine.current_level = level;
        engine.current_level_dgn_data = data;
        engine.current_level_dgn_size = (int)file_size;
        engine.current_level_structure2_source.level_index = level_index;
        engine.current_level_structure2_source.canonical_hash_verified = 1;
        engine.current_level_structure2_source.materialization_bound = 1;
        engine.current_level_structure2_source.structure2_payload_envelope_valid = 1;
        engine.current_level_structure2_source.loaded_bytes_bound = 1;
        engine.current_level_structure2_source.loaded_dgn_size = (int)file_size;
        engine.current_level_structure2_source.loaded_dgn_fnv1a64 =
            fnv1a64(data, (size_t)file_size);
        for (source_entry = 0;
             source_entry < engine.current_level.structure1f_entry_count;
             ++source_entry) {
            Nexus_V1_DgnStructure1FTransformCaptureTarget target;
            if (nexus_v1_engine_build_structure1f_transform_capture_target(
                    &engine, source_entry, &target) == 1 && target.valid &&
                target.geometry.source_geometry_bound &&
                target.geometry.vertex_slot_count >= 3 &&
                target.geometry.vertex_slot_count <= 4 &&
                target.owner_transform_selector_source_bound &&
                target.transform_selectors.resolved_selector_count > 0 &&
                !target.transform_semantics_proven &&
                target.capture_producer_required &&
                target.original_saturn_capture_required && target.no_draw_only &&
                !target.fallback_visuals_permitted &&
                target.blocks_real_dgn_mesh_render) {
                ++target_count;
            }
        }
        free(data);
    }
    return target_count;
}

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
    int untextured_found = 0;
    int animated_found = 0;
    int transform_found = 0;

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
        CHECK(count_canonical_transform_capture_targets(data_dir) > 0,
              "retail LEV00-15 binds direct mesh geometry to raw transform selectors");
        for (source_entry = 0;
             source_entry < engine.current_level.structure1f_entry_count;
             ++source_entry) {
            Nexus_V1_DgnStructure1FTransformCaptureTarget target;

            memset(&target, 0, sizeof(target));
            if (nexus_v1_engine_build_structure1f_transform_capture_target(
                    &engine, source_entry, &target) != 1) {
                continue;
            }
            transform_found = 1;
            CHECK(target.valid && target.geometry.source_geometry_bound &&
                  target.geometry.direct_mesh.structure1f_entry_index == source_entry &&
                  target.owner_transform_selector_source_bound &&
                  target.transform_selectors.resolved_selector_count > 0 &&
                  !target.transform_semantics_proven &&
                  target.capture_producer_required &&
                  target.original_saturn_capture_required && target.no_draw_only &&
                  !target.fallback_visuals_permitted &&
                  target.blocks_real_dgn_mesh_render,
                  "direct owner retains raw transform selectors without assigning semantics");
            break;
        }
        CHECK(transform_found,
              "canonical LEV01 exposes at least one source-only transform capture target");
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
        for (source_entry = 0;
             source_entry < engine.current_level.structure1f_entry_count;
             ++source_entry) {
            Nexus_V1_DgnStructure1FDirectUntexturedFaceCaptureTarget target;

            memset(&target, 0, sizeof(target));
            if (nexus_v1_engine_build_structure1f_direct_untextured_face_capture_target(
                    &engine, source_entry, &target) != 1) {
                continue;
            }
            untextured_found = 1;
            CHECK(target.valid && target.direct_face_untextured_bound &&
                  target.direct_mesh.structure1f_entry_index == source_entry &&
                  target.untextured_face.structure3_entry_index ==
                      target.direct_mesh.structure3_model_index &&
                  target.untextured_face.face_ordinal ==
                      target.direct_mesh.face_ordinal &&
                  target.untextured_face.raw_fill_bound &&
                  !target.untextured_face.flat_fill_semantics_proven &&
                  !target.untextured_face.transform_semantics_proven &&
                  !target.untextured_face.pixel_palette_vdp1_semantics_proven &&
                  !target.untextured_face.decoder_permitted &&
                  target.capture_producer_required &&
                  target.original_saturn_capture_required && target.no_draw_only &&
                  !target.fallback_visuals_permitted &&
                  target.blocks_real_dgn_mesh_render,
                  "direct owner retains only its exact opaque non-textured face");
            break;
        }
        CHECK(!untextured_found,
              "canonical LEV01 direct owners cannot invent an untextured face route");
        for (source_entry = 0;
             source_entry < engine.current_level.structure1f_entry_count;
             ++source_entry) {
            Nexus_V1_DgnStructure1FDirectAnimatedMaterialCaptureTarget target;

            memset(&target, 0, sizeof(target));
            if (nexus_v1_engine_build_structure1f_direct_animated_material_capture_target(
                    &engine, source_entry, &target) != 1) {
                continue;
            }
            animated_found = 1;
            CHECK(target.valid && target.direct_face_animated_material_bound &&
                  target.direct_mesh.structure1f_entry_index == source_entry &&
                  target.animated_material.structure3_entry_index ==
                      target.direct_mesh.structure3_model_index &&
                  target.animated_material.face_ordinal ==
                      target.direct_mesh.face_ordinal &&
                  target.animated_material.animation_declaration_bound &&
                  !target.animated_material.animation_execution_permitted &&
                  !target.animated_material.pixel_palette_vdp1_semantics_proven &&
                  !target.animated_material.decoder_permitted &&
                  target.capture_producer_required &&
                  target.original_saturn_capture_required && target.no_draw_only &&
                  !target.fallback_visuals_permitted &&
                  target.blocks_real_dgn_mesh_render,
                  "direct owner retains only its exact 08xx material declaration");
            break;
        }
        CHECK(!animated_found,
              "canonical LEV01 direct owners cannot invent an 08xx material route");
    }
    {
        Nexus_V1_DgnStructure1FTransformCaptureTarget transform_target;
        Nexus_V1_DgnStructure1FDirectStaticMaterialCaptureTarget static_target;
        Nexus_V1_DgnStructure1FDirectUntexturedFaceCaptureTarget untextured_target;
        Nexus_V1_DgnStructure1FDirectAnimatedMaterialCaptureTarget animated_target;
        memset(&transform_target, 0, sizeof(transform_target));
        CHECK(nexus_v1_engine_build_structure1f_transform_capture_target(
                  &engine, engine.current_level.structure1f_entry_count,
                  &transform_target) == 0 && !transform_target.valid &&
              transform_target.no_draw_only &&
              !transform_target.fallback_visuals_permitted &&
              transform_target.blocks_real_dgn_mesh_render,
              "out-of-range owner cannot manufacture transform semantics or fallback");
        memset(&static_target, 0, sizeof(static_target));
        CHECK(nexus_v1_engine_build_structure1f_direct_static_material_capture_target(
                  &engine, engine.current_level.structure1f_entry_count,
                  &static_target) == 0 && !static_target.valid &&
              static_target.no_draw_only && !static_target.fallback_visuals_permitted &&
              static_target.blocks_real_dgn_mesh_render,
              "out-of-range owner cannot manufacture a material or fallback");
        memset(&untextured_target, 0, sizeof(untextured_target));
        CHECK(nexus_v1_engine_build_structure1f_direct_untextured_face_capture_target(
                  &engine, engine.current_level.structure1f_entry_count,
                  &untextured_target) == 0 && !untextured_target.valid &&
              untextured_target.no_draw_only &&
              !untextured_target.fallback_visuals_permitted &&
              untextured_target.blocks_real_dgn_mesh_render,
              "out-of-range owner cannot manufacture an untextured face or fallback");
        memset(&animated_target, 0, sizeof(animated_target));
        CHECK(nexus_v1_engine_build_structure1f_direct_animated_material_capture_target(
                  &engine, engine.current_level.structure1f_entry_count,
                  &animated_target) == 0 && !animated_target.valid &&
              animated_target.no_draw_only &&
              !animated_target.fallback_visuals_permitted &&
              animated_target.blocks_real_dgn_mesh_render,
              "out-of-range owner cannot manufacture animation or fallback");
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

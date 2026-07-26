/* Engine-level regression for the 2026-07-20 round-16 re-base of
 * nexus_v1_current_level_dgn_face_material_source_receipt: geometry
 * readiness is now derived from the restored Structure3 mesh extractor
 * (nexus_v1_level_structure3_mesh_geometry_ready) instead of
 * level.geometry_info.mesh_ready, which stays 0 for the whole retail
 * LEV00-LEV15 corpus. Against the real hash-verified retail buffers the
 * engine receipt must now reach NEXUS_V1_DGN_FACE_MATERIAL_READY while
 * remaining capture-required and no-draw (can_submit_raster_input stays
 * 0 until an original Saturn VDP1 capture exists). Skip-safe: without
 * the staged retail corpus the test exits 77. */

#include "nexus_v1_engine.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int failures = 0;

#define CHECK(cond) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
        ++failures; \
    } \
} while (0)

static uint8_t *read_file(const char *path, int *out_size)
{
    FILE *file;
    long length;
    uint8_t *bytes;

    *out_size = 0;
    file = fopen(path, "rb");
    if (!file || fseek(file, 0, SEEK_END) != 0 ||
            (length = ftell(file)) <= 0 || fseek(file, 0, SEEK_SET) != 0) {
        if (file) fclose(file);
        return NULL;
    }
    bytes = (uint8_t *)malloc((size_t)length);
    if (!bytes || fread(bytes, 1, (size_t)length, file) != (size_t)length) {
        free(bytes);
        fclose(file);
        return NULL;
    }
    fclose(file);
    *out_size = (int)length;
    return bytes;
}

static void check_real_level(const char *data_dir, int level_index)
{
    char path[2048];
    uint8_t *data;
    uint8_t *tampered;
    int size = 0;
    Nexus_V1_Engine *engine;
    Nexus_V1_DgnStructure3FaceMaterialReceipt parsed;
    Nexus_V1_DgnFaceMaterialReceipt receipt;

    snprintf(path, sizeof(path), "%s/LEV%02d.DGN", data_dir, level_index);
    data = read_file(path, &size);
    CHECK(data != NULL);
    if (!data) return;

    engine = (Nexus_V1_Engine *)calloc(1U, sizeof(*engine));
    CHECK(engine != NULL);
    if (!engine) {
        free(data);
        return;
    }
    engine->game.current_level = level_index;
    CHECK(nexus_v1_level_load(
              &engine->current_level, data, size, level_index) == 0);
    engine->level_loaded = 1;
    engine->current_level_dgn_data = data;
    engine->current_level_dgn_size = size;
    CHECK(engine->current_level.geometry_info.mesh_ready == 0);

    memset(&parsed, 0, sizeof(parsed));
    CHECK(nexus_v1_level_structure3_face_material_receipt(
              &engine->current_level, &parsed) == 0 && parsed.valid);

    memset(&receipt, 0, sizeof(receipt));
    CHECK(nexus_v1_current_level_dgn_face_material_source_receipt(
              engine, &receipt) == 0);
    CHECK(receipt.status == NEXUS_V1_DGN_FACE_MATERIAL_READY);
    CHECK(receipt.face_count == parsed.textured_face_count);
    CHECK(receipt.static_selector_count ==
          parsed.static_texture_selector_count);
    CHECK(receipt.animated_selector_count ==
          parsed.animated_texture_selector_count);
    CHECK(receipt.geometry_source_bound == 1);
    CHECK(receipt.geometry_can_submit_geometry == 1);
    CHECK(receipt.geometry_material_face_count ==
          parsed.textured_face_count);
    CHECK(receipt.geometry_material_face_count_matches == 1);
    CHECK(receipt.structure3_mesh_materials_bound == 1);
    CHECK(receipt.selector_bindings_complete == 1);
    CHECK(receipt.no_draw_only == 1);
    CHECK(receipt.blocks_real_dgn_mesh_render == 1);
    CHECK(receipt.can_submit_raster_input == 0);
    CHECK(receipt.permits_fallback_visuals == 0);
    CHECK(receipt.original_saturn_capture_required == 1);
    CHECK(receipt.original_saturn_capture_available == 0);
    CHECK(receipt.material_semantics_proven == 0);

    /* A tampered buffer no longer matches the canonical MD5 and must
     * stay BLOCKED_SOURCE. */
    tampered = (uint8_t *)malloc((size_t)size);
    CHECK(tampered != NULL);
    if (tampered) {
        memcpy(tampered, data, (size_t)size);
        tampered[size / 2] ^= 0x01U;
        engine->current_level_dgn_data = tampered;
        memset(&receipt, 0, sizeof(receipt));
        CHECK(nexus_v1_current_level_dgn_face_material_source_receipt(
                  engine, &receipt) == 0);
        CHECK(receipt.status == NEXUS_V1_DGN_FACE_MATERIAL_BLOCKED_SOURCE);
        free(tampered);
        engine->current_level_dgn_data = data;
    }

    free(engine);
    free(data);
}

int main(void)
{
    const char *data_dir = getenv("FIRESTAFF_NEXUS_DATA_DIR");
    char first_path[1024];
    Nexus_V1_DgnFaceMaterialReceipt receipt;

    /* Argument and not-loaded rejections need no data. The function
     * returns 0 with a BLOCKED_SOURCE receipt for these paths. */
    memset(&receipt, 0, sizeof(receipt));
    CHECK(nexus_v1_current_level_dgn_face_material_source_receipt(
              NULL, &receipt) == 0);
    CHECK(receipt.status == NEXUS_V1_DGN_FACE_MATERIAL_BLOCKED_SOURCE);
    {
        Nexus_V1_Engine *empty =
            (Nexus_V1_Engine *)calloc(1U, sizeof(*empty));
        CHECK(empty != NULL);
        if (empty) {
            memset(&receipt, 0, sizeof(receipt));
            CHECK(nexus_v1_current_level_dgn_face_material_source_receipt(
                      empty, &receipt) == 0);
            CHECK(receipt.status ==
                  NEXUS_V1_DGN_FACE_MATERIAL_BLOCKED_SOURCE);
            free(empty);
        }
    }

    if (!data_dir || !data_dir[0]) {
        puts("SKIP: FIRESTAFF_NEXUS_DATA_DIR is not set");
        return 77;
    }
    snprintf(first_path, sizeof(first_path), "%s/LEV00.DGN", data_dir);
    if (access(first_path, R_OK) != 0) {
        puts("SKIP: retail Nexus DGN corpus is not staged");
        return 77;
    }

    check_real_level(data_dir, 0);
    check_real_level(data_dir, 1);
    check_real_level(data_dir, 8);

    if (failures) {
        fprintf(stderr, "FAILURES: %d\n", failures);
        return 1;
    }
    puts("PASS nexus_v1_engine_dgn_face_material_source_receipt");
    return 0;
}

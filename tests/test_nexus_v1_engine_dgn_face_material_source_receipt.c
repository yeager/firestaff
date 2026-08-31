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

static int failures = 0;

#define CHECK(cond) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
        ++failures; \
    } \
} while (0)

static void check_real_level(Nexus_V1_Engine *source_engine, int level_index)
{
    char name[16];
    uint8_t *data;
    uint8_t *tampered;
    int size = 0;
    Nexus_V1_Engine *engine;
    Nexus_V1_DgnStructure3FaceMaterialReceipt parsed;
    Nexus_V1_DgnFaceMaterialReceipt receipt;

    Nexus_V1_LevelAuxSourceReceipt source;

    snprintf(name, sizeof(name), "LEV%02d.DGN", level_index);
    memset(&source, 0, sizeof(source));
    (void)nexus_v1_named_asset_source_receipt(source_engine, name, &source);
    data = nexus_v1_read_file(source_engine, name, &size);
    CHECK(data != NULL);
    CHECK(source.canonical_hash_verified);
    if (!data || !source.canonical_hash_verified) {
        free(data);
        return;
    }

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
    Nexus_V1_Engine source_engine;
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
    memset(&source_engine, 0, sizeof(source_engine));
    if (nexus_v1_init(&source_engine, data_dir) != 0) {
        puts("SKIP: retail Nexus source is unavailable");
        return 77;
    }

    check_real_level(&source_engine, 0);
    check_real_level(&source_engine, 1);
    check_real_level(&source_engine, 8);
    nexus_v1_shutdown(&source_engine);

    if (failures) {
        fprintf(stderr, "FAILURES: %d\n", failures);
        return 1;
    }
    puts("PASS nexus_v1_engine_dgn_face_material_source_receipt");
    return 0;
}

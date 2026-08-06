#include "nexus_v1_dgn_scene_runtime_plan.h"

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

static int nexus_path(const char *name, char *out, size_t out_size)
{
    const char *dir;
    const char *home;

    if (!name || !out || out_size == 0U) return 0;
    dir = getenv("FIRESTAFF_NEXUS_DATA_DIR");
    if (dir && dir[0]) {
        return snprintf(out, out_size, "%s/%s", dir, name) > 0;
    }
    home = getenv("HOME");
    if (!home || !home[0]) return 0;
    return snprintf(out, out_size, "%s/.firestaff/data/nexus/%s",
                    home, name) > 0;
}

static int read_file(const char *path, uint8_t **out_data, size_t *out_size)
{
    FILE *file;
    long size;
    uint8_t *data;

    if (out_data) *out_data = NULL;
    if (out_size) *out_size = 0U;
    if (!path || !out_data || !out_size) return 0;
    file = fopen(path, "rb");
    if (!file) return 0;
    if (fseek(file, 0L, SEEK_END) != 0 || (size = ftell(file)) <= 0L ||
        fseek(file, 0L, SEEK_SET) != 0) {
        fclose(file);
        return 0;
    }
    data = (uint8_t *)malloc((size_t)size);
    if (!data) {
        fclose(file);
        return 0;
    }
    if (fread(data, 1U, (size_t)size, file) != (size_t)size) {
        free(data);
        fclose(file);
        return 0;
    }
    fclose(file);
    *out_data = data;
    *out_size = (size_t)size;
    return 1;
}

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

static void test_lev00_start_cell_scene_plan(void)
{
    char path[1024];
    uint8_t *data = NULL;
    size_t size = 0U;
    Nexus_V1_Level level;
    Nexus_V1_DgnSceneRuntimePlanInput input;
    Nexus_V1_DgnSceneRuntimePlanReceipt receipt;

    if (!nexus_path("LEV00.DGN", path, sizeof(path)) ||
        !read_file(path, &data, &size)) {
        puts("SKIP: local Nexus LEV00.DGN corpus not present");
        free(data);
        return;
    }

    CHECK(size == 147456U &&
              fnv1a64(data, size) == UINT64_C(0xe715281f66445610),
          "LEV00.DGN matches the canonical retail source identity");
    memset(&level, 0, sizeof(level));
    CHECK(nexus_v1_level_load(&level, data, (int)size, 0) == 0,
          "LEV00.DGN parses before scene planning");

    memset(&input, 0, sizeof(input));
    input.level = &level;
    input.dgn_data = data;
    input.dgn_size = (int)size;
    input.level_index = 0;
    input.dgn_source_hash_verified = 1;
    {
        int found = 0;
        int y;
        for (y = 0; y < NEXUS_MAX_MAP_SIZE && !found; ++y) {
            int x;
            for (x = 0; x < NEXUS_MAX_MAP_SIZE && !found; ++x) {
                int dir;
                if (nexus_v1_level_get_square(&level, x, y) < 0) continue;
                for (dir = 0; dir < 4 && !found; ++dir) {
                    input.party_x = x;
                    input.party_y = y;
                    input.party_dir = dir;
                    found = nexus_v1_dgn_scene_runtime_plan_build(
                        &input, &receipt) == 0 &&
                        receipt.status ==
                            NEXUS_V1_DGN_SCENE_RUNTIME_PLAN_BLOCKED_STRUCTURE1F;
                }
            }
        }
        CHECK(found,
              "LEV00 scene plan blocks without a source-owned Structure1F face");
    }
    CHECK(strcmp(nexus_v1_dgn_scene_runtime_plan_status_name(receipt.status),
                 "blocked-structure1f") == 0,
          "scene plan reports the missing Structure1F owner chain");
    CHECK(receipt.source_bound &&
              receipt.level_index == 0 &&
              receipt.party_x >= 0 &&
              receipt.party_y >= 0 &&
              receipt.party_dir >= 0 &&
              receipt.party_dir < 4 &&
              receipt.party_cell_bound &&
              receipt.forward_cell_bound &&
              receipt.side_cells_bound,
          "party camera relation is bound to real adjacent LEV00 cells");
    CHECK(receipt.source_cell_count > 0 &&
              receipt.view_command_count == 0 &&
              receipt.floor_command_count == 0 &&
              receipt.wall_command_count == 0 &&
              receipt.ceiling_command_count == 0 &&
              receipt.selected_structure1f_entry_index == -1 &&
              receipt.selected_structure3_model_index == -1 &&
              receipt.selected_face_ordinal == -1 &&
              !receipt.mesh_entry_bound &&
              !receipt.geometry_consumer_ready &&
              receipt.texture_submit_blocked &&
              receipt.raster_submit_blocked &&
              !receipt.m11_runtime_handoff_permitted &&
              !receipt.fallback_geometry_permitted &&
              !receipt.fallback_visuals_permitted &&
              receipt.no_draw_only,
          "production scene consumer keeps real adjacent cells but opens no arbitrary mesh route");

    input.dgn_source_hash_verified = 0;
    CHECK(nexus_v1_dgn_scene_runtime_plan_build(&input, &receipt) == 0 &&
              receipt.status == NEXUS_V1_DGN_SCENE_RUNTIME_PLAN_BLOCKED_INPUT &&
              !receipt.geometry_consumer_ready &&
              receipt.texture_submit_blocked &&
              receipt.raster_submit_blocked &&
              !receipt.fallback_geometry_permitted &&
              !receipt.fallback_visuals_permitted,
          "unverified DGN source cannot enter scene planning");

    free(data);
}

static void test_lev01_owned_scene_plan(void)
{
    char path[1024];
    uint8_t *data = NULL;
    size_t size = 0U;
    Nexus_V1_Level level;
    Nexus_V1_DgnSceneRuntimePlanInput input;
    Nexus_V1_DgnSceneRuntimePlanReceipt receipt;
    int found = 0;
    int y;

    if (!nexus_path("LEV01.DGN", path, sizeof(path)) ||
        !read_file(path, &data, &size)) {
        puts("SKIP: local Nexus LEV01.DGN corpus not present");
        free(data);
        return;
    }
    CHECK(size == 280576U &&
              nexus_v1_level_load(&level, data, (int)size, 1) == 0,
          "LEV01.DGN parses before owned scene planning");
    if (size != 280576U || nexus_v1_level_load(&level, data, (int)size, 1) != 0) {
        free(data);
        return;
    }
    memset(&input, 0, sizeof(input));
    input.level = &level;
    input.dgn_data = data;
    input.dgn_size = (int)size;
    input.level_index = 1;
    input.dgn_source_hash_verified = 1;
    for (y = 0; y < NEXUS_MAX_MAP_SIZE && !found; ++y) {
        int x;
        for (x = 0; x < NEXUS_MAX_MAP_SIZE && !found; ++x) {
            int dir;
            if (nexus_v1_level_get_square(&level, x, y) < 0) continue;
            for (dir = 0; dir < 4 && !found; ++dir) {
                input.party_x = x;
                input.party_y = y;
                input.party_dir = dir;
                found = nexus_v1_dgn_scene_runtime_plan_build(
                            &input, &receipt) == 1;
            }
        }
    }
    CHECK(found &&
              receipt.status ==
                  NEXUS_V1_DGN_SCENE_RUNTIME_PLAN_READY_GEOMETRY_NO_DRAW &&
              receipt.structure1f_visible_owned_entry_count == 1 &&
              receipt.structure1f_owned_source_count == 1 &&
              receipt.selected_structure1f_entry_index >= 0 &&
              receipt.selected_structure1a_index >= 0 &&
              receipt.selected_structure3_model_index >= 0 &&
              receipt.structure1f_face_selector_bound &&
              receipt.structure1a_model_rotation_bound &&
              receipt.mesh_entry_bound && receipt.geometry_consumer_ready &&
              receipt.texture_submit_blocked && receipt.raster_submit_blocked &&
              !receipt.m11_runtime_handoff_permitted &&
              !receipt.fallback_geometry_permitted &&
              !receipt.fallback_visuals_permitted,
          "LEV01 scene plan uses a source-owned Structure1F to Structure3 chain without raster promotion");
    free(data);
}

int main(void)
{
    test_lev00_start_cell_scene_plan();
    test_lev01_owned_scene_plan();
    if (failures) {
        fprintf(stderr, "Nexus DGN scene runtime plan: %d failure(s)\n",
                failures);
        return 1;
    }
    puts("Nexus DGN scene runtime plan: PASS");
    return 0;
}

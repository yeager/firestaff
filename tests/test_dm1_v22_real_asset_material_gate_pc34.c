#include "dm1_v2_asset_pipeline_pc34.h"
#include "dm1_v2_presentation_mode_pc34.h"
#include "fs_portable_compat.h"
#include "m11_v22_inplace_draw_pc34.h"
#include "m11_v22_shape_cache_pc34.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SKIP_RETURN_CODE 77

static int failures = 0;

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        failures++; \
    } \
} while (0)

static int file_exists(const char* path) {
    FILE* fp;
    if (!path || path[0] == '\0') return 0;
    fp = fopen(path, "rb");
    if (!fp) return 0;
    fclose(fp);
    return 1;
}

static uint32_t fnv1a_bytes(const void* data, size_t len) {
    const unsigned char* p = (const unsigned char*)data;
    uint32_t h = 2166136261u;
    size_t i;
    for (i = 0; i < len; ++i) {
        h = (h ^ (uint32_t)p[i]) * 16777619u;
    }
    return h;
}

static char* read_text_file(const char* path, size_t* out_len) {
    FILE* fp = fopen(path, "rb");
    long size;
    char* data;
    if (out_len) *out_len = 0U;
    if (!fp) return NULL;
    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return NULL;
    }
    size = ftell(fp);
    if (size < 0 || size > 1024L * 1024L) {
        fclose(fp);
        return NULL;
    }
    if (fseek(fp, 0, SEEK_SET) != 0) {
        fclose(fp);
        return NULL;
    }
    data = (char*)malloc((size_t)size + 1U);
    if (!data) {
        fclose(fp);
        return NULL;
    }
    if (fread(data, 1, (size_t)size, fp) != (size_t)size) {
        free(data);
        fclose(fp);
        return NULL;
    }
    fclose(fp);
    data[size] = '\0';
    if (out_len) *out_len = (size_t)size;
    return data;
}

static int build_path(char* out, size_t out_size,
                      const char* a, const char* b) {
    int n;
    if (!out || out_size == 0U || !a || !b) return 0;
    n = snprintf(out, out_size, "%s/%s", a, b);
    return n > 0 && (size_t)n < out_size;
}

static int build_home_path(char* out, size_t out_size,
                           const char* home, const char* suffix) {
    int n;
    if (!out || out_size == 0U || !home || !suffix) return 0;
    n = snprintf(out, out_size, "%s/%s", home, suffix);
    return n > 0 && (size_t)n < out_size;
}

static void require_manifest_token(const char* manifest,
                                   const char* token,
                                   const char* label) {
    int ok = manifest && token && strstr(manifest, token) != NULL;
    if (!ok) {
        fprintf(stderr, "missing DM1 V2.2 real-asset manifest token: %s\n",
                label ? label : token);
    }
    CHECK(ok);
}

static uint32_t bitmap_signature(const uint32_t* rgba, int w, int h) {
    if (!rgba || w <= 0 || h <= 0) return 0U;
    return fnv1a_bytes(rgba, (size_t)w * (size_t)h * sizeof(uint32_t));
}

static const uint32_t* require_cell_bitmap(int depth,
                                           int lateral,
                                           int* out_w,
                                           int* out_h,
                                           const char* expected_asset_id) {
    const char* asset_id = m11_v22_inplace_get_cell_asset_id(depth, lateral);
    const uint32_t* rgba;
    CHECK(asset_id != NULL);
    if (expected_asset_id) {
        CHECK(asset_id != NULL && strcmp(asset_id, expected_asset_id) == 0);
    }
    rgba = m11_v22_inplace_get_cell_bitmap(depth, lateral, out_w, out_h);
    CHECK(rgba != NULL);
    CHECK(out_w && *out_w >= 16);
    CHECK(out_h && *out_h >= 16);
    return rgba;
}

static int nonzero_pixel_count(const unsigned char* fb, size_t len) {
    int count = 0;
    size_t i;
    if (!fb) return 0;
    for (i = 0; i < len; ++i) {
        if (fb[i] != 0U) count++;
    }
    return count;
}

int main(void) {
    const char* home = getenv("HOME");
    char modern_dir[FSP_PATH_MAX];
    char manifest_path[FSP_PATH_MAX];
    char cache_path[FSP_PATH_MAX];
    char data_dir[FSP_PATH_MAX];
    char required_file[FSP_PATH_MAX];
    char* manifest = NULL;
    size_t manifest_len = 0U;
    int wall_w = 0, wall_h = 0;
    int floor_w = 0, floor_h = 0;
    int pit_w = 0, pit_h = 0;
    int stairs_w = 0, stairs_h = 0;
    uint32_t wall_sig;
    uint32_t floor_sig;
    uint32_t pit_sig;
    uint32_t stairs_sig;
    unsigned char raw_cells[3][3] = {
        { 0x00, 0x20, 0x40 },
        { 0x68, 0x80, 0xA0 },
        { 0x00, 0x20, 0x40 }
    };
    unsigned char fb[320 * 200];
    int painted;
    uint32_t frame_sig;
    int frame_nonzero;

    if (!home || home[0] == '\0') {
        puts("SKIP dm1_v22_real_asset_material_gate_pc34: HOME unset; "
             "requires ~/.firestaff/assets/dm1/modern/");
        return SKIP_RETURN_CODE;
    }

    CHECK(build_home_path(modern_dir, sizeof(modern_dir), home,
                          ".firestaff/assets/dm1/modern"));
    CHECK(build_path(manifest_path, sizeof(manifest_path), modern_dir,
                     "modern_asset_manifest.json"));
    CHECK(build_path(cache_path, sizeof(cache_path), modern_dir,
                     "v22_inplace_cache.bin"));
    CHECK(build_home_path(data_dir, sizeof(data_dir), home,
                          ".firestaff/data/dm1"));
    if (failures) return 1;

    if (!file_exists(manifest_path) || !file_exists(cache_path)) {
        printf("SKIP dm1_v22_real_asset_material_gate_pc34: missing real "
               "DM1 V2.2 material inputs; required files are %s and %s\n",
               manifest_path, cache_path);
        return SKIP_RETURN_CODE;
    }

    manifest = read_text_file(manifest_path, &manifest_len);
    CHECK(manifest != NULL && manifest_len > 0U);
    if (!manifest) return 1;

    require_manifest_token(manifest, "\"wall_d3_carved_hero_01\"",
                           "wall_d3_carved_hero_01");
    require_manifest_token(manifest, "\"floor_plain_hero_01\"",
                           "floor_plain_hero_01");
    require_manifest_token(manifest, "\"floor_pit_hero_01\"",
                           "floor_pit_hero_01");
    require_manifest_token(manifest, "\"creature_demon_hero_01\"",
                           "creature_demon_hero_01");
    require_manifest_token(manifest, "\"champion_warrior_hero_01\"",
                           "champion_warrior_hero_01");
    require_manifest_token(manifest, "\"door_hero_01\"",
                           "door_hero_01");

    CHECK(build_path(required_file, sizeof(required_file), modern_dir,
                     "wall_shapes/wall_d3_carved_hero_01.png"));
    CHECK(file_exists(required_file));
    CHECK(build_path(required_file, sizeof(required_file), modern_dir,
                     "floor_shapes/floor_plain_hero_01.png"));
    CHECK(file_exists(required_file));
    CHECK(build_path(required_file, sizeof(required_file), modern_dir,
                     "floor_shapes/floor_pit_hero_01.png"));
    CHECK(file_exists(required_file));
    CHECK(build_path(required_file, sizeof(required_file), modern_dir,
                     "creature_shapes/creature_demon_hero_01.png"));
    CHECK(file_exists(required_file));
    CHECK(build_path(required_file, sizeof(required_file), modern_dir,
                     "champion_portraits/champion_warrior_hero_01.png"));
    CHECK(file_exists(required_file));
    CHECK(build_path(required_file, sizeof(required_file), modern_dir,
                     "door_shapes/door_hero_01.png"));
    CHECK(file_exists(required_file));

    m11_v22_set_manifest_path(data_dir);
    CHECK(m11_v22_validate_manifest(manifest_path) == 1);
    CHECK(m11_v22_modern_assets_available() == 1);

    dm1_v2_presentation_mode_reset();
    dm1_v2_presentation_mode_set_modern_pack_available(1);
    dm1_v2_presentation_mode_set(DM1_V2_PM_V22_MODERN);
    CHECK(dm1_v2_presentation_mode_is_v22() == 1);

    m11_v22_inplace_draw_shutdown();
    CHECK(m11_v22_inplace_draw_init() == 1);
    CHECK(m11_v22_inplace_draw_active() == 1);

    m11_v22_shape_cache_update(0, (const unsigned char (*)[3])raw_cells);
    CHECK(m11_v22_shape_cache_populated() == 1);

    wall_sig = bitmap_signature(
        require_cell_bitmap(1, -1, &wall_w, &wall_h, "wall_d3_carved_01"),
        wall_w, wall_h);
    floor_sig = bitmap_signature(
        require_cell_bitmap(1, 0, &floor_w, &floor_h, "floor_plain_01"),
        floor_w, floor_h);
    pit_sig = bitmap_signature(
        require_cell_bitmap(1, 1, &pit_w, &pit_h, "floor_pit_01"),
        pit_w, pit_h);
    stairs_sig = bitmap_signature(
        require_cell_bitmap(2, -1, &stairs_w, &stairs_h,
                            "floor_stairs_down_01"),
        stairs_w, stairs_h);

    CHECK(wall_sig != 0U);
    CHECK(floor_sig != 0U);
    CHECK(pit_sig != 0U);
    CHECK(stairs_sig != 0U);
    CHECK(wall_sig != floor_sig);
    CHECK(wall_sig != pit_sig);
    CHECK(floor_sig != pit_sig);
    CHECK(stairs_sig != floor_sig);

    memset(fb, 0, sizeof(fb));
    painted = m11_v22_inplace_render_pass(fb, 320, 200);
    frame_sig = fnv1a_bytes(fb, sizeof(fb));
    frame_nonzero = nonzero_pixel_count(fb, sizeof(fb));
    CHECK(painted >= 4);
    CHECK(frame_sig != 0U);
    CHECK(frame_nonzero > 0);

    CHECK(m11_v22_inplace_draw_source_evidence() != NULL);
    CHECK(strstr(m11_v22_inplace_draw_source_evidence(), "ReDMCSB") != NULL);
    CHECK(strstr(m11_v22_inplace_draw_source_evidence(),
                 "v22_inplace_cache.bin") != NULL);

    m11_v22_inplace_draw_shutdown();
    free(manifest);

    if (failures) {
        fprintf(stderr, "dm1_v22_real_asset_material_gate_pc34: %d failure(s); "
                "manifest=%s cache=%s\n", failures, manifest_path, cache_path);
        return 1;
    }

    printf("dm1_v22_real_asset_material_gate_pc34: material receipt "
           "wall=0x%08x floor=0x%08x pit=0x%08x stairs=0x%08x "
           "frame=0x%08x nonzero=%d painted=%d\n",
           (unsigned)wall_sig, (unsigned)floor_sig, (unsigned)pit_sig,
           (unsigned)stairs_sig, (unsigned)frame_sig, frame_nonzero, painted);
    return 0;
}

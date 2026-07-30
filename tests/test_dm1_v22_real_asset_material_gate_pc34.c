#include "dm1_v2_asset_pipeline_pc34.h"
#include "dm1_v2_presentation_mode_pc34.h"
#include "dm1_v22_finished_art_material_gate_pc34.h"
#include "dm1_v22_real_art_runtime_gate_pc34.h"
#include "m11_v22_inplace_draw_pc34.h"
#include "m11_v22_shape_cache_pc34.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    if (size < 0 || size > 1024L * 1024L || fseek(fp, 0, SEEK_SET) != 0) {
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
    return build_path(out, out_size, home, suffix);
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
    const unsigned char* bytes = (const unsigned char*)rgba;
    uint32_t hash = 2166136261u;
    size_t i;
    if (!rgba || w <= 0 || h <= 0) return 0U;
    for (i = 0; i < (size_t)w * (size_t)h * sizeof(*rgba); ++i) {
        hash = (hash ^ bytes[i]) * 16777619u;
    }
    return hash;
}

static uint32_t byte_signature(const unsigned char* bytes, size_t len) {
    uint32_t hash = 2166136261u;
    size_t i;
    if (!bytes) return 0U;
    for (i = 0; i < len; ++i) {
        hash = (hash ^ bytes[i]) * 16777619u;
    }
    return hash;
}

static const uint32_t* require_cell_bitmap(int depth, int lateral,
                                           int* out_w, int* out_h,
                                           const char* expected_asset_id) {
    const char* asset_id = m11_v22_inplace_get_cell_asset_id(depth, lateral);
    const uint32_t* rgba;
    CHECK(asset_id != NULL);
    CHECK(asset_id && strcmp(asset_id, expected_asset_id) == 0);
    rgba = m11_v22_inplace_get_cell_bitmap(depth, lateral, out_w, out_h);
    CHECK(rgba != NULL);
    CHECK(out_w && *out_w >= 16);
    CHECK(out_h && *out_h >= 16);
    return rgba;
}

static int nonzero_pixel_count(const unsigned char* fb, size_t len) {
    int count = 0;
    size_t i;
    for (i = 0; fb && i < len; ++i) {
        if (fb[i] != 0U) count++;
    }
    return count;
}

int main(void) {
    const char* home = getenv("HOME");
    char modern_dir[4096];
    char manifest_path[4096];
    char cache_path[4096];
    char data_dir[4096];
    char* manifest;
    size_t manifest_len = 0U;
    unsigned char raw_cells[3][3] = {
        { 0x00, 0x20, 0x40 },
        { 0x68, 0x80, 0xA0 },
        { 0x00, 0x20, 0x40 }
    };
    unsigned char fb[320 * 200];
    int wall_w = 0, wall_h = 0, floor_w = 0, floor_h = 0;
    int pit_w = 0, pit_h = 0;
    uint32_t wall_sig, floor_sig, pit_sig, frame_sig;
    int painted, frame_nonzero;
    DM1_V22_RealArtRuntimeGate_PC34 runtime_gate;

    if (!home || home[0] == '\0' ||
        !build_home_path(modern_dir, sizeof(modern_dir), home,
                         ".firestaff/assets/dm1/modern") ||
        !build_path(manifest_path, sizeof(manifest_path), modern_dir,
                    "modern_asset_manifest.json") ||
        !build_path(cache_path, sizeof(cache_path), modern_dir,
                    "v22_inplace_cache.bin") ||
        !build_home_path(data_dir, sizeof(data_dir), home,
                         ".firestaff/data/dm1")) {
        puts("dm1_v22_real_asset_material_gate_pc34: SKIP no original-art home");
        return 0;
    }

    if (!file_exists(manifest_path) || !file_exists(cache_path)) {
        puts("dm1_v22_real_asset_material_gate_pc34: SKIP original DM1 V2.2 "
             "manifest/cache not installed");
        return 0;
    }

    memset(&runtime_gate, 0, sizeof(runtime_gate));
    if (!dm1_v22_real_art_runtime_gate_refresh_pc34(data_dir, &runtime_gate) ||
        !runtime_gate.admitted) {
        puts("dm1_v22_real_asset_material_gate_pc34: SKIP manifest is not "
             "operator-reviewed FINISHED_REAL original art");
        return 0;
    }

    manifest = read_text_file(manifest_path, &manifest_len);
    CHECK(manifest != NULL && manifest_len > 0U);
    if (!manifest) return 1;
    require_manifest_token(manifest, "\"wall_d3_carved_hero_01\"", "wall");
    require_manifest_token(manifest, "\"floor_plain_hero_01\"", "floor");
    require_manifest_token(manifest, "\"floor_pit_hero_01\"", "pit");
    require_manifest_token(manifest, "\"creature_demon_hero_01\"", "creature");
    require_manifest_token(manifest, "\"champion_warrior_hero_01\"", "champion");
    require_manifest_token(manifest, "\"door_hero_01\"", "door");
    require_manifest_token(manifest, "\"field_teleporter_hero_01\"", "field");

    m11_v22_set_manifest_path(data_dir);
    /* The generic validator intentionally reports partial while optional
     * UI/stairs categories have no reviewed replacement. V2.2 admission is
     * owned by the stricter finished-art material/receipt gates above. */
    CHECK(m11_v22_validate_manifest(manifest_path) == 0);
    CHECK(m11_v22_modern_assets_available() == 1);
    dm1_v2_presentation_mode_reset();
    dm1_v2_presentation_mode_set_modern_pack_available(1);
    dm1_v2_presentation_mode_set(DM1_V2_PM_V22_MODERN);
    CHECK(dm1_v2_presentation_mode_is_v22() == 1);

    m11_v22_inplace_draw_shutdown();
    CHECK(m11_v22_inplace_draw_init() == 1);
    CHECK(m11_v22_inplace_draw_active() == 1);
    m11_v22_shape_cache_update(0, (const unsigned char (*)[3])raw_cells);
    wall_sig = bitmap_signature(require_cell_bitmap(1, -1, &wall_w, &wall_h,
                                                    "wall_d3_carved_hero_01"), wall_w, wall_h);
    floor_sig = bitmap_signature(require_cell_bitmap(1, 0, &floor_w, &floor_h,
                                                     "floor_plain_hero_01"), floor_w, floor_h);
    pit_sig = bitmap_signature(require_cell_bitmap(1, 1, &pit_w, &pit_h,
                                                   "floor_pit_hero_01"), pit_w, pit_h);
    CHECK(wall_sig && floor_sig && pit_sig);
    CHECK(wall_sig != floor_sig && floor_sig != pit_sig);
    memset(fb, 0, sizeof(fb));
    painted = m11_v22_inplace_render_pass(fb, 320, 200);
    frame_sig = byte_signature(fb, sizeof(fb));
    frame_nonzero = nonzero_pixel_count(fb, sizeof(fb));
    CHECK(painted >= 3);
    CHECK(frame_sig != 0U && frame_nonzero > 0);
    m11_v22_inplace_draw_shutdown();
    free(manifest);

    if (failures) return 1;
    printf("dm1_v22_real_asset_material_gate_pc34: PASS painted=%d\n", painted);
    return 0;
}

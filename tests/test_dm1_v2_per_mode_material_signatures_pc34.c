#include "dm1_v2_presentation_mode_pc34.h"
#include "dm1_v2_viewport_renderer_pc34.h"
#include "fs_portable_compat.h"
#include "m11_v22_inplace_draw_pc34.h"
#include "m11_v22_shape_cache_pc34.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int failures = 0;

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        failures++; \
    } \
} while (0)

#define CHECK_EQ_U32(actual, expected) do { \
    uint32_t a__ = (uint32_t)(actual); \
    uint32_t e__ = (uint32_t)(expected); \
    if (a__ != e__) { \
        fprintf(stderr, "FAIL %s:%d: %s expected 0x%08x got 0x%08x\n", \
                __FILE__, __LINE__, #actual, (unsigned)e__, (unsigned)a__); \
        failures++; \
    } \
} while (0)

static uint32_t fnv1a_bytes(const void* data, size_t len) {
    const unsigned char* p = (const unsigned char*)data;
    uint32_t h = 2166136261u;
    size_t i;
    for (i = 0; i < len; ++i) {
        h = (h ^ (uint32_t)p[i]) * 16777619u;
    }
    return h;
}

static uint32_t fnv1a_string(const char* s) {
    uint32_t h = 2166136261u;
    while (*s) {
        h = (h ^ (uint32_t)(uint8_t)*s++) * 16777619u;
    }
    return h;
}

static void put_u32(unsigned char* p, uint32_t v) {
    memcpy(p, &v, sizeof(v));
}

typedef struct ProbeCacheEntry {
    const char* category;
    const char* asset_id;
    uint32_t rgba[4];
} ProbeCacheEntry;

static void put_cache_entry(unsigned char* entry,
                            const ProbeCacheEntry* fixture,
                            uint32_t rgba_offset) {
    memset(entry, 0, 32);
    put_u32(entry + 0, fnv1a_string(fixture->category));
    put_u32(entry + 4, fnv1a_string(fixture->asset_id));
    put_u32(entry + 8, 2u);
    put_u32(entry + 12, 2u);
    put_u32(entry + 16, 4u * (uint32_t)sizeof(uint32_t));
    put_u32(entry + 20, rgba_offset);
}

static int write_minimal_dm1_v22_cache(const char* cache_path) {
    FILE* fp;
    unsigned char header[32];
    unsigned char entries[5][32];
    const ProbeCacheEntry fixtures[5] = {
        {
            "wall_shapes",
            "wall_d3_carved_01",
            { 0x00ff0000u, 0x00ff0000u, 0x00ff0000u, 0x00ff0000u }
        },
        {
            "floor_shapes",
            "floor_plain_01",
            { 0x0000ff00u, 0x0000ff00u, 0x0000ff00u, 0x0000ff00u }
        },
        {
            "floor_shapes",
            "floor_pit_01",
            { 0x000000ffu, 0x000000ffu, 0x000000ffu, 0x000000ffu }
        },
        {
            "floor_shapes",
            "floor_stairs_down_01",
            { 0x00ffff00u, 0x00ffff00u, 0x00ffff00u, 0x00ffff00u }
        },
        {
            "field_shapes",
            "field_teleporter_01",
            { 0x00ff00ffu, 0x00ff00ffu, 0x00ff00ffu, 0x00ff00ffu }
        }
    };
    size_t i;
    uint32_t data_offset;

    memset(header, 0, sizeof(header));
    memset(entries, 0, sizeof(entries));
    memcpy(header, "FSV22C\0\0", 8);
    put_u32(header + 8, 1u);
    put_u32(header + 12, (uint32_t)(sizeof(fixtures) / sizeof(fixtures[0])));
    data_offset = (uint32_t)(sizeof(header) + sizeof(entries));
    for (i = 0; i < sizeof(fixtures) / sizeof(fixtures[0]); ++i) {
        put_cache_entry(entries[i],
                        &fixtures[i],
                        data_offset + (uint32_t)(i * sizeof(fixtures[i].rgba)));
    }

    fp = fopen(cache_path, "wb");
    if (!fp) return 0;
    if (fwrite(header, 1, sizeof(header), fp) != sizeof(header) ||
        fwrite(entries, 1, sizeof(entries), fp) != sizeof(entries)) {
        fclose(fp);
        return 0;
    }
    for (i = 0; i < sizeof(fixtures) / sizeof(fixtures[0]); ++i) {
        if (fwrite(fixtures[i].rgba, 1, sizeof(fixtures[i].rgba), fp) !=
            sizeof(fixtures[i].rgba)) {
            fclose(fp);
            return 0;
        }
    }
    return fclose(fp) == 0;
}

static int setup_probe_home(char* out_cache_path, size_t out_size) {
    char modern_dir[FSP_PATH_MAX];
    int n = snprintf(modern_dir, sizeof(modern_dir),
                     "firestaff-dm1-v2-per-mode-home/.firestaff/assets/dm1/modern");
    if (n <= 0 || (size_t)n >= sizeof(modern_dir)) return 0;
    if (!FSP_CreateDirectoryRecursive(modern_dir)) return 0;
    if (FSP_SetEnv("HOME", "firestaff-dm1-v2-per-mode-home", 1) != 0) return 0;

    n = snprintf(out_cache_path, out_size, "%s/v22_inplace_cache.bin", modern_dir);
    return n > 0 && (size_t)n < out_size;
}

static void build_canonical_composition(DM1_V2_ViewportCompositionInput* input) {
    dm1_v2_vp_composition_init(input);
    input->squares[3][1].element = DM1_V2_ELEMENT_WALL;
    input->squares[2][0].element = DM1_V2_ELEMENT_PIT;
    input->squares[2][2].element = DM1_V2_ELEMENT_TELEPORTER;
    input->squares[1][1].element = DM1_V2_ELEMENT_DOOR_FRONT;
    input->squares[0][1].element = DM1_V2_ELEMENT_STAIRS_FRONT;
    input->squares[1][0].hasObjects = 1;
    input->squares[1][2].hasField = 1;
}

static uint8_t material_index_for_color(DM1_V2_Color c) {
    if (c.r == 0 && c.g == 0 && c.b == 0) return 0x00;
    if (c.r <= 80 && c.g <= 80 && c.b <= 80) return 0x08;
    if (c.r <= 120 && c.g <= 120 && c.b <= 120) return 0x07;
    if (c.r <= 155 && c.g <= 155 && c.b <= 155) return 0x07;
    return 0x0f;
}

static void copy_flat_viewport_to_v21_source(const DM1_V2_ViewportState* vp,
                                             uint8_t* dst) {
    int x;
    int y;
    memset(dst, 0, 320u * 200u);
    for (y = 0; y < DM1_V2_VIEWPORT_H; ++y) {
        for (x = 0; x < DM1_V2_VIEWPORT_W; ++x) {
            dst[(y + 16) * 320 + (x + 48)] =
                material_index_for_color(dm1_v2_vp_get_pixel(vp, x, y));
        }
    }
}

static void test_v20_flat_material_signature(uint32_t* out_signature,
                                             DM1_V2_ViewportState* out_vp) {
    DM1_V2_ViewportCompositionInput input;
    DM1_V2_DrawCommand commands[DM1_V2_MAX_DRAW_COMMANDS];
    int count;

    dm1_v2_presentation_mode_reset();
    dm1_v2_presentation_mode_set(DM1_V2_PM_V20_FILTERED);
    CHECK(dm1_v2_presentation_mode_is_v20() == 1);

    build_canonical_composition(&input);
    count = dm1_v2_vp_emit_d0_d3_draw_list(&input, commands,
                                           DM1_V2_MAX_DRAW_COMMANDS);
    CHECK(count == 11);
    CHECK(commands[0].op == DM1_V2_DRAW_FLOOR_CEILING);
    CHECK(commands[count - 1].op == DM1_V2_DRAW_STAIRS_FRONT);

    dm1_v2_vp_init(out_vp);
    CHECK(dm1_v2_vp_render_composition_flat(out_vp, &input) == 1);
    *out_signature = fnv1a_bytes(out_vp->framebuffer,
                                 sizeof(out_vp->framebuffer));

    CHECK(dm1_v2_vp_get_pixel(out_vp, 112, 72).r == 73);
    CHECK(dm1_v2_vp_get_pixel(out_vp, 112, 20).r == 146);
    CHECK_EQ_U32(*out_signature, 0x2b0dd7ddu);
}

static void test_v21_epx_material_signature(const DM1_V2_ViewportState* flat_vp,
                                            uint32_t* out_signature) {
    const uint32_t* rgba;
    uint8_t source_before[320 * 200];
    uint8_t* src;
    int out_w = 0;
    int out_h = 0;
    size_t rgba_size;
    const int sample_x = (48 + 112) * 2;
    const int sample_y = (16 + 72) * 2;
    uint32_t p00;
    uint32_t p10;
    uint32_t p01;
    uint32_t p11;

    dm1_v2_presentation_mode_reset();
    dm1_v2_presentation_mode_set(DM1_V2_PM_V21_UPSCALED);
    CHECK(dm1_v2_presentation_mode_is_v21() == 1);

    v21_viewport_init(2);
    src = v21_viewport_get_v1_framebuffer_mut();
    CHECK(src != NULL);
    copy_flat_viewport_to_v21_source(flat_vp, src);
    memcpy(source_before, src, sizeof(source_before));

    v21_viewport_render_full_pipeline();
    rgba = v21_viewport_get_rgba(&out_w, &out_h);
    CHECK(rgba != NULL);
    CHECK(out_w == 640);
    CHECK(out_h == 400);
    CHECK(memcmp(source_before, src, sizeof(source_before)) == 0);

    p00 = rgba[sample_y * out_w + sample_x];
    p10 = rgba[sample_y * out_w + sample_x + 1];
    p01 = rgba[(sample_y + 1) * out_w + sample_x];
    p11 = rgba[(sample_y + 1) * out_w + sample_x + 1];
    CHECK(p00 == p10 && p00 == p01 && p00 == p11);
    CHECK((p00 & 0x00ffffffu) != 0u);

    rgba_size = (size_t)out_w * (size_t)out_h * sizeof(uint32_t);
    *out_signature = fnv1a_bytes(rgba, rgba_size);
    CHECK_EQ_U32(*out_signature, 0x3fae57cdu);
}

static unsigned char cell_center_pixel(const unsigned char* fb,
                                       int fbW,
                                       int depth,
                                       int lateral) {
    static const int centers[3][3][2] = {
        { { 42, 118 }, {108, 118 }, {173, 118 } },
        { { 42,  87 }, {108,  87 }, {173,  87 } },
        { { 42,  56 }, {108,  56 }, {173,  56 } }
    };
    return fb[centers[depth - 1][lateral + 1][1] * fbW +
              centers[depth - 1][lateral + 1][0]];
}

static void test_v22_inplace_material_signature(uint32_t* out_signature) {
    char cache_path[FSP_PATH_MAX];
    unsigned char raw_cells[3][3] = {
        { 0x00, 0x20, 0x40 },
        { 0x68, 0x80, 0xa0 },
        { 0x00, 0x20, 0x40 }
    };
    unsigned char fb[320 * 200];
    const char* asset_id;
    int painted;

    memset(cache_path, 0, sizeof(cache_path));
    CHECK(setup_probe_home(cache_path, sizeof(cache_path)) == 1);
    CHECK(write_minimal_dm1_v22_cache(cache_path) == 1);

    dm1_v2_presentation_mode_reset();
    dm1_v2_presentation_mode_set_modern_pack_available(1);
    dm1_v2_presentation_mode_set(DM1_V2_PM_V22_MODERN);
    CHECK(dm1_v2_presentation_mode_is_v22() == 1);

    m11_v22_inplace_draw_shutdown();
    CHECK(m11_v22_inplace_draw_init() == 1);
    CHECK(m11_v22_inplace_draw_active() == 1);

    m11_v22_shape_cache_update(0, (const unsigned char (*)[3])raw_cells);
    CHECK(strcmp(m11_v22_inplace_get_cell_asset_id(1, -1),
                 "wall_d3_carved_01") == 0);
    CHECK(strcmp(m11_v22_inplace_get_cell_asset_id(1, 0),
                 "floor_plain_01") == 0);
    CHECK(strcmp(m11_v22_inplace_get_cell_asset_id(1, 1),
                 "floor_pit_01") == 0);
    CHECK(strcmp(m11_v22_inplace_get_cell_asset_id(2, -1),
                 "floor_stairs_down_01") == 0);
    asset_id = m11_v22_inplace_get_cell_asset_id(2, 1);
    CHECK(asset_id != NULL && strcmp(asset_id, "field_teleporter_01") == 0);

    memset(fb, 0, sizeof(fb));
    painted = m11_v22_inplace_render_pass(fb, 320, 200);
    CHECK(painted == 9);
    CHECK(cell_center_pixel(fb, 320, 1, -1) == 0x30);
    CHECK(cell_center_pixel(fb, 320, 1, 0) == 0x0c);
    CHECK(cell_center_pixel(fb, 320, 1, 1) == 0x03);
    CHECK(cell_center_pixel(fb, 320, 2, -1) == 0x3c);
    CHECK(cell_center_pixel(fb, 320, 2, 1) == 0x33);

    *out_signature = fnv1a_bytes(fb, sizeof(fb));
    CHECK_EQ_U32(*out_signature, 0xbe1c77fdu);
    m11_v22_inplace_draw_shutdown();
}

int main(void) {
    DM1_V2_ViewportState flat_vp;
    uint32_t v20_signature = 0;
    uint32_t v21_signature = 0;
    uint32_t v22_signature = 0;
    const char* evidence;

    memset(&flat_vp, 0, sizeof(flat_vp));

    test_v20_flat_material_signature(&v20_signature, &flat_vp);
    test_v21_epx_material_signature(&flat_vp, &v21_signature);
    test_v22_inplace_material_signature(&v22_signature);

    CHECK(v20_signature != 0u);
    CHECK(v21_signature != 0u);
    CHECK(v22_signature != 0u);
    CHECK(v20_signature != v21_signature);
    CHECK(v20_signature != v22_signature);
    CHECK(v21_signature != v22_signature);

    evidence = m11_v22_inplace_draw_source_evidence();
    CHECK(evidence != NULL && strstr(evidence, "ReDMCSB") != NULL);
    evidence = v21_viewport_source_evidence();
    CHECK(evidence != NULL && strstr(evidence, "V1 320x200 indexed") != NULL);

    if (failures) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }

    printf("dm1_v2_per_mode_material_signatures_pc34: "
           "v20=0x%08x v21=0x%08x v22=0x%08x\n",
           (unsigned)v20_signature,
           (unsigned)v21_signature,
           (unsigned)v22_signature);
    return 0;
}

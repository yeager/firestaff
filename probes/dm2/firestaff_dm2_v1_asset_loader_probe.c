/*
 * firestaff_dm2_v1_asset_loader_probe.c — DM2 V1 Asset Loader Verification
 *
 * Compiles and exercises the dm2_v1_asset_loader.c API against
 * the verified DM2 PC English GRAPHICS.DAT (if available).
 *
 * Build:
 *   gcc -I include -I src/shared -I src/dm2 \
 *       probes/dm2/firestaff_dm2_v1_asset_loader_probe.c \
 *       src/dm2/dm2_v1_asset_loader.c \
 *       -o build/dm2_v1_asset_loader_probe -lm
 *
 * (2026-06-16) The historical src/shared/firestaff_pc34_core_amalgam.c
 * link entry is removed; that file is unbuilt legacy code (see
 * docs/audits/REDMSB_FIRESTAFF_AUDIT_2026-06-16.md Bug 2).  The
 * DM2 V1 asset-loader probe links against firestaff_m10 instead.
 *
 * Run:
 *   SDL_VIDEODRIVER=dummy ./build/dm2_v1_asset_loader_probe \
 *       ~/.firestaff/data/dm2/GRAPHICS.DAT
 */

#include "dm2_v1_asset_loader.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int errors = 0;
static int passed = 0;

#define PROBE_ASSERT(cond, fmt, ...) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL: " fmt "\n", ##__VA_ARGS__); \
        errors++; \
    } else { \
        fprintf(stderr, "PASS: " fmt "\n", ##__VA_ARGS__); \
        passed++; \
    } \
} while (0)

static uint8_t *load_file(const char *path, long *out_size) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    uint8_t *buf = malloc((size_t)sz + 1);
    if (!buf) { fclose(f); return NULL; }
    size_t got = fread(buf, 1, (size_t)sz, f);
    buf[got] = 0;
    fclose(f);
    *out_size = (long)got;
    return buf;
}

static uint32_t fnv1a32_pixels(const uint8_t *pixels, size_t count) {
    uint32_t h = 2166136261u;
    size_t i;
    if (!pixels) return 0;
    for (i = 0; i < count; ++i) {
        h ^= pixels[i];
        h *= 16777619u;
    }
    return h;
}

static int distinct_pixel_count(const uint8_t *pixels, size_t count) {
    uint8_t seen[256] = {0};
    int n = 0;
    size_t i;
    if (!pixels) return 0;
    for (i = 0; i < count; ++i) {
        if (!seen[pixels[i]]) {
            seen[pixels[i]] = 1;
            ++n;
        }
    }
    return n;
}

int main(int argc, char **argv) {
    const char *gfx_path;
    long file_size = 0;
    uint8_t *raw = NULL;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <GRAPHICS.DAT path>\n", argv[0]);
        return 1;
    }
    gfx_path = argv[1];

    fprintf(stderr, "=== DM2 V1 Asset Loader Probe ===\n");
    fprintf(stderr, "Source: docs/dm2_v1_phase2_data_formats_H2254.md §3\n");
    fprintf(stderr, "Source: docs/dm2_graphics.md — GDAT categories (240), IMG3/IMG9\n");
    fprintf(stderr, "Source: SKULL.ASM T560 — dungeon viewport rendering\n");
    fprintf(stderr, "Source: SKULL.ASM T600 — outdoor viewport rendering\n");
    fprintf(stderr, "Input: %s\n\n", gfx_path);

    raw = load_file(gfx_path, &file_size);
    if (!raw) {
        fprintf(stderr, "NOTE: GRAPHICS.DAT not found at %s — testing API only\n", gfx_path);
        /* Continue with null data to test API surface */
    }

    /* ── Test dm2_v1_asset_loader_init ── */
    fprintf(stderr, "\n--- Testing dm2_v1_asset_loader_init --- \n");
    DM2_V1_AssetLoader loader;
    int init_result = -1;
    if (raw) {
        init_result = dm2_v1_asset_loader_init(&loader, raw, (size_t)file_size);
        PROBE_ASSERT(init_result == 0,
                     "dm2_v1_asset_loader_init returns 0");
        PROBE_ASSERT(loader.loaded == 1,
                     "loader.loaded = 1 after init");
        PROBE_ASSERT(loader.category_count > 0,
                     "loader.category_count > 0 (got %d)", loader.category_count);
    } else {
        init_result = dm2_v1_asset_loader_init(&loader, NULL, 0);
        PROBE_ASSERT(init_result == -1,
                     "dm2_v1_asset_loader_init(NULL) = -1");
    }

    /* ── Test dm2_v1_asset_loader_verify ── */
    fprintf(stderr, "\n--- Testing dm2_v1_asset_loader_verify --- \n");
    if (raw) {
        int verify_result = dm2_v1_asset_loader_verify(&loader);
        /* DM2 PC English should pass size check (~8.6 MB) */
        if (file_size >= 8*1024*1024 && file_size <= 10*1024*1024) {
            PROBE_ASSERT(verify_result == 1,
                         "DM2 PC English GRAPHICS.DAT passes size verification");
        } else {
            fprintf(stderr, "NOTE: GRAPHICS.DAT size %ld not in DM2 range\n", file_size);
        }
    }

    /* ── Test GDAT2 field name lookup ── */
    fprintf(stderr, "\n--- Testing dm2_v1_asset_gdat2_field_name --- \n");
    const char *name;

    name = dm2_v1_asset_gdat2_field_name(0x060000);
    PROBE_ASSERT(strstr(name, "Animation") != NULL,
                 "Field 0x060000 = Animation");

    name = dm2_v1_asset_gdat2_field_name(0x0F0000);
    PROBE_ASSERT(strstr(name, "DoorStrength") != NULL,
                 "Field 0x0F0000 = DoorStrength");

    name = dm2_v1_asset_gdat2_field_name(0x040000);
    PROBE_ASSERT(strstr(name, "ColorKey") != NULL,
                 "Field 0x040000 = ColorKey1_Cyan");

    name = dm2_v1_asset_gdat2_field_name(0x0C0000);
    PROBE_ASSERT(strstr(name, "ColorKey") != NULL,
                 "Field 0x0C0000 = ColorKey2_DarkGreen");

    name = dm2_v1_asset_gdat2_field_name(0x200000);
    PROBE_ASSERT(strstr(name, "Mirrored") != NULL,
                 "Field 0x200000 = AnimatedMirroredDoor");

    name = dm2_v1_asset_gdat2_field_name(0x850000);
    PROBE_ASSERT(strstr(name, "AmbientLight") != NULL,
                 "Field 0x850000 = DefaultAmbientLight");

    name = dm2_v1_asset_gdat2_field_name(0x870000);
    PROBE_ASSERT(strstr(name, "Darkness") != NULL || strstr(name, "SightDistance") != NULL,
                 "Field 0x870000 = AmbientDarkness_SightDistance");

    name = dm2_v1_asset_gdat2_field_name(0x999999);
    PROBE_ASSERT(strstr(name, "Unknown") != NULL,
                 "Field 0x999999 = UnknownField");

    /* ── Test image loading boundary ── */
    fprintf(stderr, "\n--- Testing dm2_v1_asset_load_image boundary --- \n");
    if (raw) {
        int w = 0, h = 0;
        DM2_ImageFormat fmt = DM2_IMG_FMT_UNKNOWN;
        uint8_t *pixels = dm2_v1_asset_load_image_field(
            &loader,
            DM2_GDAT_CATEGORY_GRAPHICSSET,
            0,
            0,
            &w,
            &h,
            &fmt);
        PROBE_ASSERT(pixels != NULL && w == 224 && h == 1 && fmt == DM2_IMG_FMT_U4,
                     "GDAT GRAPHICSSET floor U4 strip realizes from real image entry (%dx%d fmt=%d)",
                     w, h, (int)fmt);
        dm2_v1_asset_free_pixels(pixels);
        w = h = 0;
        fmt = DM2_IMG_FMT_UNKNOWN;
        pixels = dm2_v1_asset_load_image_field(
            &loader,
            DM2_GDAT_CATEGORY_GRAPHICSSET,
            0,
            1,
            &w,
            &h,
            &fmt);
        PROBE_ASSERT(pixels != NULL && w == 224 && h == 1 && fmt == DM2_IMG_FMT_U4,
                     "GDAT GRAPHICSSET ceiling U4 strip realizes from real image entry (%dx%d fmt=%d)",
                     w, h, (int)fmt);
        dm2_v1_asset_free_pixels(pixels);

        w = h = 0;
        fmt = DM2_IMG_FMT_UNKNOWN;
        pixels = dm2_v1_asset_load_image_field(
            &loader,
            DM2_GDAT_CATEGORY_INTERFACE_GENERAL,
            2,
            7,
            &w,
            &h,
            &fmt);
        PROBE_ASSERT(pixels != NULL && w == 16 && h == 16 &&
                         fmt == DM2_IMG_FMT_IMG3 &&
                         distinct_pixel_count(pixels, (size_t)w * (size_t)h) == 4 &&
                         fnv1a32_pixels(pixels, (size_t)w * (size_t)h) == 0x880ceb3cu,
                     "GDAT C4 IMG3 decoder realizes even-width interface image hash");
        dm2_v1_asset_free_pixels(pixels);

        w = h = 0;
        fmt = DM2_IMG_FMT_UNKNOWN;
        pixels = dm2_v1_asset_load_image_field(
            &loader,
            DM2_GDAT_CATEGORY_INTERFACE_GENERAL,
            3,
            2,
            &w,
            &h,
            &fmt);
        PROBE_ASSERT(pixels != NULL && w == 29 && h == 23 &&
                         fmt == DM2_IMG_FMT_IMG3 &&
                         distinct_pixel_count(pixels, (size_t)w * (size_t)h) == 13 &&
                         fnv1a32_pixels(pixels, (size_t)w * (size_t)h) == 0x76bb682du,
                     "GDAT C4 IMG3 decoder realizes odd-width row-padded image hash (distinct=%d hash=0x%08x)",
                     distinct_pixel_count(pixels, (size_t)w * (size_t)h),
                     fnv1a32_pixels(pixels, (size_t)w * (size_t)h));
        dm2_v1_asset_free_pixels(pixels);

        w = h = 0;
        fmt = DM2_IMG_FMT_UNKNOWN;
        pixels = dm2_v1_asset_load_image_field(
            &loader,
            DM2_GDAT_CATEGORY_TITLE,
            0,
            1,
            &w,
            &h,
            &fmt);
        PROBE_ASSERT(pixels != NULL && w == 320 && h == 200 &&
                         fmt == DM2_IMG_FMT_IMG9 &&
                         distinct_pixel_count(pixels, (size_t)w * (size_t)h) == 149 &&
                         fnv1a32_pixels(pixels, (size_t)w * (size_t)h) == 0x63ce78dbu,
                     "GDAT C8 IMG9 decoder realizes 320x200 title image hash");
        dm2_v1_asset_free_pixels(pixels);

        w = h = 0;
        fmt = DM2_IMG_FMT_UNKNOWN;
        pixels = dm2_v1_asset_load_image_field(
            &loader,
            DM2_GDAT_CATEGORY_GRAPHICSSET,
            1,
            0x06,
            &w,
            &h,
            &fmt);
        PROBE_ASSERT(pixels != NULL && w == 34 && h == 136 &&
                         fmt == DM2_IMG_FMT_IMG9 &&
                         fnv1a32_pixels(pixels, (size_t)w * (size_t)h) == 0xb4aa5742u,
                     "GDAT GRAPHICSSET front door-frame IMG9 realizes from real image entry");
        dm2_v1_asset_free_pixels(pixels);

        w = h = 0;
        fmt = DM2_IMG_FMT_UNKNOWN;
        pixels = dm2_v1_asset_load_image_field(
            &loader,
            DM2_GDAT_CATEGORY_GRAPHICSSET,
            1,
            0x07,
            &w,
            &h,
            &fmt);
        PROBE_ASSERT(pixels != NULL && w == 18 && h == 98 &&
                         fmt == DM2_IMG_FMT_IMG9 &&
                         fnv1a32_pixels(pixels, (size_t)w * (size_t)h) == 0x05c4a91cu,
                     "GDAT GRAPHICSSET D1C door-frame IMG9 realizes from real image entry");
        dm2_v1_asset_free_pixels(pixels);

        w = h = 0;
        fmt = DM2_IMG_FMT_UNKNOWN;
        pixels = dm2_v1_asset_load_image_field(
            &loader,
            DM2_GDAT_CATEGORY_GRAPHICSSET,
            1,
            0x09,
            &w,
            &h,
            &fmt);
        PROBE_ASSERT(pixels != NULL && w == 13 && h == 71 &&
                         fmt == DM2_IMG_FMT_IMG9 &&
                         fnv1a32_pixels(pixels, (size_t)w * (size_t)h) == 0xfafc4208u,
                     "GDAT GRAPHICSSET D2C door-frame IMG9 realizes from real image entry");
        dm2_v1_asset_free_pixels(pixels);

        w = h = 0;
        fmt = DM2_IMG_FMT_UNKNOWN;
        pixels = dm2_v1_asset_load_image_field(
            &loader,
            DM2_GDAT_CATEGORY_DOORS,
            0,
            0,
            &w,
            &h,
            &fmt);
        PROBE_ASSERT(pixels != NULL && w == 96 && h == 88 &&
                         fmt == DM2_IMG_FMT_IMG9 &&
                         fnv1a32_pixels(pixels, (size_t)w * (size_t)h) == 0x0525c623u,
                     "GDAT DOORS D0C/D1C panel IMG9 realizes from real image entry");
        dm2_v1_asset_free_pixels(pixels);

        w = h = 0;
        fmt = DM2_IMG_FMT_UNKNOWN;
        pixels = dm2_v1_asset_load_image_field(
            &loader,
            DM2_GDAT_CATEGORY_DOORS,
            0,
            1,
            &w,
            &h,
            &fmt);
        PROBE_ASSERT(pixels != NULL && w == 64 && h == 61 &&
                         fmt == DM2_IMG_FMT_IMG9 &&
                         fnv1a32_pixels(pixels, (size_t)w * (size_t)h) == 0x84cb658du,
                     "GDAT DOORS D2C panel IMG9 realizes from real image entry");
        dm2_v1_asset_free_pixels(pixels);
    }

    /* ── Test category entry count ── */
    fprintf(stderr, "\n--- Testing dm2_v1_asset_category_entry_count --- \n");
    int count = dm2_v1_asset_category_entry_count(&loader, DM2_GDAT_CATEGORY_GRAPHICSSET);
    PROBE_ASSERT(count > 0,
                 "GDAT graphics-set category has indexed entries: %d", count);
    PROBE_ASSERT(dm2_v1_asset_category_entry_count(&loader, DM2_GDAT_CATEGORY_WALL_GFX) > 0,
                 "GDAT wall graphics category has indexed entries");
    PROBE_ASSERT(dm2_v1_asset_category_entry_count(&loader, DM2_GDAT_CATEGORY_FLOOR_GFX) > 0,
                 "GDAT floor graphics category has indexed entries");

    /* ── Test asset load API ── */
    fprintf(stderr, "\n--- Testing dm2_v1_asset_load --- \n");
    const uint8_t *asset = dm2_v1_asset_load(&loader, DM2_GDAT_CATEGORY_TECHDATA, 0, 0);
    PROBE_ASSERT(asset != NULL,
                 "GDAT raw lookup resolves category/index/field data");

    /* ── Null guards ── */
    dm2_v1_asset_loader_free(NULL); /* must not crash */

    /* ── Source evidence ── */
    const char *evidence = dm2_v1_asset_loader_source_evidence();
    PROBE_ASSERT(evidence != NULL && strlen(evidence) > 10,
                 "dm2_v1_asset_loader_source_evidence() returns non-empty string");

    if (raw) free(raw);

    fprintf(stderr, "\n=== Results: %d passed, %d failed ===\n", passed, errors);
    if (errors > 0) {
        fprintf(stderr, "PROBE FAILED\n");
        return 1;
    }
    fprintf(stderr, "PROBE PASSED\n");
    return 0;
}

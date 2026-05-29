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
 *       src/shared/firestaff_pc34_core_amalgam.c \
 *       -o build/dm2_v1_asset_loader_probe -lm
 *
 * Run:
 *   SDL_VIDEODRIVER=dummy ./build/dm2_v1_asset_loader_probe \
 *       ~/.firestaff/data/dm2/GRAPHICS.DAT
 */

#include "dm2_v1_asset_loader.h"
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

    /* ── Test image loading (stub) ── */
    fprintf(stderr, "\n--- Testing dm2_v1_asset_load_image (stub) --- \n");
    if (raw) {
        int w = 0, h = 0;
        DM2_ImageFormat fmt = DM2_IMG_FMT_UNKNOWN;
        uint8_t *pixels = dm2_v1_asset_load_image(&loader, 0, 0, &w, &h, &fmt);
        if (pixels) {
            PROBE_ASSERT(w > 0 && h > 0,
                         "Stub image load: w=%d h=%d", w, h);
            PROBE_ASSERT(fmt == DM2_IMG_FMT_IMG3 || fmt == DM2_IMG_FMT_UNKNOWN,
                         "Stub image format is valid");
            dm2_v1_asset_free_pixels(pixels);
        } else {
            fprintf(stderr, "NOTE: dm2_v1_asset_load_image returned NULL (expected for stub)\n");
        }
    }

    /* ── Test category entry count (deferred) ── */
    fprintf(stderr, "\n--- Testing dm2_v1_asset_category_entry_count (deferred) --- \n");
    int count = dm2_v1_asset_category_entry_count(&loader, 0);
    (void)count; /* Deferred - just verify it doesn't crash */
    PROBE_ASSERT(1, "dm2_v1_asset_category_entry_count doesn't crash");

    /* ── Test asset load API (deferred) ── */
    fprintf(stderr, "\n--- Testing dm2_v1_asset_load (deferred) --- \n");
    const uint8_t *asset = dm2_v1_asset_load(&loader, 0, 0, 0);
    (void)asset; /* Deferred - just verify it doesn't crash */
    PROBE_ASSERT(1, "dm2_v1_asset_load doesn't crash");

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
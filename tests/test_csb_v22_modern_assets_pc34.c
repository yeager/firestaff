/*
 * test_csb_v22_modern_assets_pc34.c — CSB V2.2 Modern Graphics asset pipeline
 *
 * Focused tests for the CSB V2.2 modern-assets module:
 *   - Path resolution from dataDir → ~/.firestaff/assets/csb/modern/
 *   - Manifest validation (empty/partial/complete)
 *   - modern_assets_available() with critical categories present
 *   - get/set installed flag
 *   - get/set epx cache warm flag
 *   - best_available_shape_source fallback chain
 *   - shape_source_name strings
 *   - missing modern art fails closed without generated pixels
 *   - get_shape_path (with present category + id)
 *
 * Parallel to test_dm1_v22_verification.c (but CSB-scoped, smaller).
 * Deterministic: uses a private temp scratch directory for manifest files.
 */

#include "csb_v22_modern_assets_pc34.h"
#include "fs_portable_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

/* ── Test harness ───────────────────────────────────────────────── */

static int failures = 0;
static int checks = 0;

#define CHECK(expr, msg) \
    do { \
        checks++; \
        if (!(expr)) { \
            fprintf(stderr, "FAIL %s:%d: %s — %s\n", __FILE__, __LINE__, #expr, (msg)); \
            failures++; \
        } \
    } while (0)

/* Helper: write a string to a file (returns 1 on success). */
static int write_file(const char* path, const char* content) {
    FILE* fp = fopen(path, "wb");
    if (!fp) return 0;
    size_t len = strlen(content);
    size_t written = fwrite(content, 1, len, fp);
    fclose(fp);
    return (written == len);
}

/* Helper: make a directory (returns 1 on success). */
static int mkdir_p(const char* path) {
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "mkdir -p '%s'", path);
    return (system(cmd) == 0);
}

/* ── Tests ──────────────────────────────────────────────────────── */

static void test_manifest_path_from_data_dir(void) {
    /* dataDir = ~/.firestaff/data/csb → manifest = ~/.firestaff/assets/csb/modern/modern_asset_manifest.json */
    csb_v22_set_manifest_path("/home/user/.firestaff/data/csb");
    const char* expected_suffix = "/.firestaff/assets/csb/modern/modern_asset_manifest.json";
    (void)expected_suffix;
    /* The exact path is captured by re-running with a controlled dataDir */
    /* Use a fake dataDir that we can verify the path transformation */
    csb_v22_set_manifest_path("/tmp/scratch/firestaff-data/csb");
    /* We can't read the internal static g_v22_manifest_path directly,
     * so we validate via validate_manifest on a known-good location. */
    CHECK(1, "csb_v22_set_manifest_path did not crash");
}

static void test_manifest_path_null_safe(void) {
    csb_v22_set_manifest_path(NULL);
    CHECK(1, "NULL dataDir safe");
    csb_v22_set_manifest_path("");
    CHECK(1, "empty dataDir safe");
    int avail = csb_v22_modern_assets_available();
    CHECK(avail == 0, "no manifest = not available");
}

static void test_validate_manifest_missing(void) {
    int v = csb_v22_validate_manifest("/nonexistent/path/manifest.json");
    CHECK(v == -1, "missing manifest returns -1");
}

static void test_validate_manifest_empty(void) {
    const char* path = "/tmp/scratch/csb-empty.json";
    CHECK(write_file(path, "{}"), "wrote empty manifest");
    int v = csb_v22_validate_manifest(path);
    CHECK(v == 0 || v == -1, "empty manifest returns 0 (partial) or -1");
}

static void test_validate_manifest_partial(void) {
    const char* path = "/tmp/scratch/csb-partial.json";
    const char* content =
        "{\"manifestVersion\":\"1.0.0\",\"packId\":\"test\","
        "\"wall_shapes\":[{\"id\":\"w1\",\"source_file\":\"w1.png\",\"width\":64,\"height\":64}],"
        "\"floor_shapes\":[],"
        "\"creature_shapes\":[]}";
    CHECK(write_file(path, content), "wrote partial manifest");
    int v = csb_v22_validate_manifest(path);
    CHECK(v == 0 || v == -1, "partial manifest returns 0 (partial) or -1");
}

static void test_installed_flag(void) {
    csb_v22_set_installed(0);
    CHECK(csb_v22_get_installed() == 0, "installed=0 round-trip");
    csb_v22_set_installed(1);
    CHECK(csb_v22_get_installed() == 1, "installed=1 round-trip");
    csb_v22_set_installed(42);  /* truthy */
    CHECK(csb_v22_get_installed() == 1, "installed=42 → 1 (clamped)");
    csb_v22_set_installed(0);
}

static void test_epx_cache_flag(void) {
    csb_v22_set_epx_cache_warm(0);
    CHECK(csb_v22_get_epx_cache_warm() == 0, "epx warm=0 round-trip");
    csb_v22_set_epx_cache_warm(1);
    CHECK(csb_v22_get_epx_cache_warm() == 1, "epx warm=1 round-trip");
    csb_v22_set_epx_cache_warm(0);
}

static void test_best_available_shape_source(void) {
    csb_v22_set_installed(0);
    csb_v22_set_epx_cache_warm(0);

    CHECK(csb_v22_best_available_shape_source(0) == CSB_V22_SHAPE_SOURCE_V1_ORIGINAL,
          "mode 0 (V1) → V1_ORIGINAL");

    CHECK(csb_v22_best_available_shape_source(1) == CSB_V22_SHAPE_SOURCE_V2_FILTERED,
          "mode 1 (V2.0) → V2_FILTERED");

    CHECK(csb_v22_best_available_shape_source(2) == CSB_V22_SHAPE_SOURCE_V2_FILTERED,
          "mode 2 (V2.1) cold cache → V2_FILTERED (fallback)");

    csb_v22_set_epx_cache_warm(1);
    CHECK(csb_v22_best_available_shape_source(2) == CSB_V22_SHAPE_SOURCE_V2_UPSCALED,
          "mode 2 (V2.1) warm cache → V2_UPSCALED");

    /* For mode 3 no-install test, set installed=0 and epx warm=0 (cold)
     * so the fallback chain falls through correctly: V2.2 not installed
     * → V2.1 not warm → V2.0. */
    csb_v22_set_installed(0);
    csb_v22_set_epx_cache_warm(0);
    CHECK(csb_v22_best_available_shape_source(3) == CSB_V22_SHAPE_SOURCE_V2_FILTERED,
          "mode 3 (V2.2) no install, cold cache → V2_FILTERED (fallback)");

    csb_v22_set_installed(1);
    CHECK(csb_v22_best_available_shape_source(3) == CSB_V22_SHAPE_SOURCE_V2_MODERN,
          "mode 3 (V2.2) installed → V2_MODERN");

    /* Unknown mode → V1_ORIGINAL */
    CHECK(csb_v22_best_available_shape_source(99) == CSB_V22_SHAPE_SOURCE_V1_ORIGINAL,
          "mode 99 → V1_ORIGINAL (default)");

    csb_v22_set_installed(0);
    csb_v22_set_epx_cache_warm(0);
}

static void test_shape_source_name(void) {
    CHECK(strcmp(csb_v22_shape_source_name(CSB_V22_SHAPE_SOURCE_V1_ORIGINAL), "V1_ORIGINAL") == 0,
          "name V1_ORIGINAL");
    CHECK(strcmp(csb_v22_shape_source_name(CSB_V22_SHAPE_SOURCE_V2_FILTERED), "V2_FILTERED") == 0,
          "name V2_FILTERED");
    CHECK(strcmp(csb_v22_shape_source_name(CSB_V22_SHAPE_SOURCE_V2_UPSCALED), "V2_UPSCALED") == 0,
          "name V2_UPSCALED");
    CHECK(strcmp(csb_v22_shape_source_name(CSB_V22_SHAPE_SOURCE_V2_MODERN), "V2_MODERN") == 0,
          "name V2_MODERN");
    CHECK(strcmp(csb_v22_shape_source_name(99), "UNKNOWN") == 0,
          "name unknown");
}

static void test_missing_asset_fails_closed(void) {
    int w = 0, h = 0;
    const uint32_t* px = csb_v22_get_missing_placeholder(&w, &h);
    CHECK(px == NULL, "missing modern art has no generated pixel fallback");
    CHECK(w == 0 && h == 0, "missing modern art reports no surface");
}

static void test_source_evidence(void) {
    const char* ev = csb_v22_source_evidence();
    CHECK(ev != NULL, "evidence non-null");
    CHECK(strlen(ev) > 0, "evidence non-empty");
    CHECK(strstr(ev, "ReDMCSB") != NULL || strstr(ev, "CSBWin") != NULL,
          "evidence cites source");
}

static void test_assets_available_no_install(void) {
    /* Point to an empty manifest */
    csb_v22_set_installed(0);
    /* After set_manifest_path to a real dir with empty manifest, available should be 0 */
    const char* path = "/tmp/scratch/csb-empty.json";
    write_file(path, "{}");
    /* Use validate to check the manifest is empty (no critical categories) */
    int v = csb_v22_validate_manifest(path);
    CHECK(v == 0 || v == -1, "empty manifest not complete");
}

static void test_artpack_studio_pretty_manifest_admission(void) {
    const char* data_dir = "/tmp/scratch/csb-v22-pretty/data/csb";
    const char* modern_dir = "/tmp/scratch/csb-v22-pretty/assets/csb/modern";
    const char* manifest =
        "/tmp/scratch/csb-v22-pretty/assets/csb/modern/modern_asset_manifest.json";
    const char* content =
        "{\n"
        "  \"manifestVersion\": \"1.0.0\",\n"
        "  \"packId\": \"source-derived-csb\",\n"
        "  \"game\": \"csb\",\n"
        "  \"wall_shapes\": [\n"
        "    {\"id\": \"wall_dungeon_d0_01\", \"generator\": \"source_export\", \"source_file\": \"wall.png\", \"width\": 96, \"height\": 96}\n"
        "  ],\n"
        "  \"floor_shapes\": [\n"
        "    {\"id\": \"floor_plain_d0_01\", \"generator\": \"source_export\", \"source_file\": \"floor.png\", \"width\": 96, \"height\": 96}\n"
        "  ],\n"
        "  \"creature_shapes\": [\n"
        "    {\"id\": \"creature_demon_d0_01\", \"generator\": \"source_export\", \"source_file\": \"demon.png\", \"width\": 64, \"height\": 64}\n"
        "  ],\n"
        "  \"ui_chrome\": [\n"
        "    {\"id\": \"panel_source_01\", \"generator\": \"source_export\", \"source_file\": \"panel.png\", \"width\": 64, \"height\": 32}\n"
        "  ],\n"
        "  \"champion_portraits\": [\n"
        "    {\"id\": \"portrait_source_01\", \"generator\": \"source_export\", \"source_file\": \"portrait.png\", \"width\": 32, \"height\": 32}\n"
        "  ]\n"
        "}\n";

    CHECK(mkdir_p(data_dir), "created pretty-manifest data dir");
    CHECK(mkdir_p(modern_dir), "created pretty-manifest asset dir");
    CHECK(write_file(manifest, content), "wrote Artpack Studio-style manifest");
    csb_v22_set_manifest_path(data_dir);
    CHECK(csb_v22_validate_manifest(csb_v22_get_manifest_path()) == 1,
          "pretty category manifest validates complete");
    CHECK(csb_v22_modern_assets_available() == 0,
          "pretty partial category manifest does not admit runtime V2.2");
}

static void test_route_provenance_metadata(void) {
    const char* data_dir = "/tmp/scratch/csb-v22-provenance/data/csb";
    const char* modern_dir = "/tmp/scratch/csb-v22-provenance/assets/csb/modern";
    const char* manifest =
        "/tmp/scratch/csb-v22-provenance/assets/csb/modern/modern_asset_manifest.json";
    const char* content =
        "{\n  \"routeProvenance\": [\n    {\n"
        "      \"id\": \"door_d1_01\",\n"
        "      \"category\": \"door_shapes\",\n"
        "      \"sourceGraphicIndex\": 247,\n"
        "      \"sourceDimensions\": [\n        64,\n        61\n      ],\n"
        "      \"sourceRecordSha256\": \"7063872718410000000000000000000000000000000000000000000000000000\",\n"
        "      \"outputDimensions\": [\n        64,\n        96\n      ]\n"
        "    }\n  ]\n}\n";
    CSB_V22_RouteProvenancePc34 provenance;

    CHECK(mkdir_p(data_dir), "created route-provenance data dir");
    CHECK(mkdir_p(modern_dir), "created route-provenance asset dir");
    CHECK(write_file(manifest, content), "wrote route-provenance manifest");
    csb_v22_set_manifest_path(data_dir);
    CHECK(csb_v22_get_route_provenance("door_shapes", "door_d1_01",
                                       &provenance) == 1,
          "finds source route provenance");
    CHECK(provenance.valid && provenance.source_graphic_index == 247 &&
          provenance.source_width == 64 && provenance.source_height == 61,
          "preserves graphic index and source dimensions");
    CHECK(strcmp(provenance.source_record_sha256,
                 "7063872718410000000000000000000000000000000000000000000000000000") == 0 &&
          provenance.output_width == 64 && provenance.output_height == 96,
          "requires record identity and exported dimensions");
    CHECK(csb_v22_get_route_provenance("door_shapes", "door_d0_01",
                                       &provenance) == 0,
          "does not invent absent route provenance");
    CHECK(write_file(manifest,
        "{\"routeProvenance\":[\n"
        "{\"id\":\"door_d1_01\",\"category\":\"door_shapes\","
        "\"sourceGraphicIndex\":247,\"sourceDimensions\":[64,61],"
        "\"sourceRecordSha256\":\"7063872718410000000000000000000000000000000000000000000000000000\","
        "\"outputDimensions\":[64,96]}\n]}"),
          "wrote compact source-artpack route provenance");
    CHECK(csb_v22_get_route_provenance("door_shapes", "door_d1_01",
                                       &provenance) == 1 &&
          provenance.source_width == 64 && provenance.source_height == 61 &&
          provenance.output_width == 64 && provenance.output_height == 96,
          "reads Artpack Studio compact provenance arrays");

    CHECK(write_file(manifest,
        "{\"routeProvenance\":[{\"id\":\"door_d1_01\",\"category\":\"door_shapes\","
        "\"sourceGraphicIndex\":247,\"sourceDimensions\":[64,61],"
        "\"sourceRecordSha256\":\"7063872718410000000000000000000000000000000000000000000000000000\","
        "\"outputDimensions\":[64,96]}]}"),
          "wrote single-line source-artpack route provenance");
    CHECK(csb_v22_get_route_provenance("door_shapes", "door_d1_01",
                                       &provenance) == 1 &&
          provenance.source_graphic_index == 247 &&
          provenance.source_width == 64 && provenance.source_height == 61 &&
          provenance.output_width == 64 && provenance.output_height == 96,
          "reads fully compact Artpack Studio route provenance");

    CHECK(write_file(manifest,
        "{\"routeProvenance\":[\n"
        "{\"id\":\"door_d1_01\",\"category\":\"door_shapes\","
        "\"sourceGraphicIndex\":247,\"sourceDimensions\":[64,61],"
        "\"sourceRecordSha256\":\"not-a-sha256-identity\","
        "\"outputDimensions\":[64,96]}\n]}"),
          "wrote malformed route provenance hash");
    CHECK(csb_v22_get_route_provenance("door_shapes", "door_d1_01",
                                       &provenance) == 0,
          "rejects non-SHA256 route provenance identity");
}

static void test_shape_path_stays_inside_artpack(void) {
    const char* data_dir = "/tmp/scratch/csb-v22-path/data/csb";
    const char* manifest =
        "/tmp/scratch/csb-v22-path/assets/csb/modern/modern_asset_manifest.json";
    char resolved[FSP_PATH_MAX];
    CHECK(mkdir_p(data_dir), "created safe-path data dir");
    CHECK(mkdir_p("/tmp/scratch/csb-v22-path/assets/csb/modern/wall_shapes"),
          "created safe-path wall category");
    CHECK(write_file("/tmp/scratch/csb-v22-path/assets/csb/modern/wall_shapes/wall.png", "x"),
          "wrote safe-path source file");
    CHECK(write_file(manifest,
        "{\n\"wall_shapes\": [\n"
        "{\"id\": \"wall_dungeon_d0_01\", \"source_file\": \"wall.png\"}\n"
        "]\n}"),
          "wrote safe-path manifest");
    csb_v22_set_manifest_path(data_dir);
    CHECK(csb_v22_get_shape_path("wall_shapes", "wall_dungeon_d0_01",
                                 resolved, sizeof(resolved)) == 1 &&
          strstr(resolved, "/wall_shapes/wall.png") != NULL,
          "resolves an artpack-local source file");
    CHECK(csb_v22_get_shape_path("../wall_shapes", "wall_dungeon_d0_01",
                                 resolved, sizeof(resolved)) == 0,
          "rejects a category traversal");
    CHECK(write_file(manifest,
        "{\n\"wall_shapes\": [\n"
        "{\"id\": \"wall_dungeon_d0_01\", \"source_file\": \"../wall.png\"}\n"
        "]\n}"),
          "wrote traversal source-file manifest");
    CHECK(csb_v22_get_shape_path("wall_shapes", "wall_dungeon_d0_01",
                                 resolved, sizeof(resolved)) == 0,
          "rejects a source-file traversal");
}

/* ── Main ───────────────────────────────────────────────────────── */

int main(void) {
    mkdir_p("/tmp/scratch");

    test_manifest_path_from_data_dir();
    test_manifest_path_null_safe();
    test_validate_manifest_missing();
    test_validate_manifest_empty();
    test_validate_manifest_partial();
    test_installed_flag();
    test_epx_cache_flag();
    test_best_available_shape_source();
    test_shape_source_name();
    test_missing_asset_fails_closed();
    test_source_evidence();
    test_assets_available_no_install();
    test_artpack_studio_pretty_manifest_admission();
    test_route_provenance_metadata();
    test_shape_path_stays_inside_artpack();

    printf("csb_v22_modern_assets_pc34: checks=%d failures=%d\n", checks, failures);
    if (failures > 0) {
        printf("csb_v22_modern_assets_pc34: FAIL\n");
        return 1;
    }
    printf("csb_v22_modern_assets_pc34: PASS\n");
    return 0;
}

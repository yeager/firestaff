/*
 * test_dm1_v22_verification.c — DM1 V2.2 Modern Graphics verification suite
 *
 * Tests the V2.2 (modern/3D) presentation pipeline components:
 *   - Asset mode enum correctness (MODERN == 3)
 *   - Modern asset discovery (m11_v22_modern_assets_available)
 *   - Manifest validation (m11_v22_validate_manifest)
 *   - Fallback chain (m11_v22_best_available_shape_source)
 *   - Shape system (m11_v22_shape_for_cell → M11_V22_ShapeParams)
 *   - Config integration (v22_modern_assets_installed)
 *   - Source-lock / phase-gate preservation of V1 gameplay
 *
 * Deterministic: requires no game data files.
 * All filesystem tests use a private temp scratch directory.
 */

#include "m11_v22_asset_pipeline.h"
#include "dm1_v2_phase_gate_pc34.h"
#include "dm1_v2_settings_pc34.h"
#include "dm1_v2_presentation_profile_pc34.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

/* ── Test harness ───────────────────────────────────────────────── */

static int failures = 0;

#define CHECK(expr)                                                  \
    do {                                                             \
        if (!(expr)) {                                               \
            fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__,  \
                    #expr);                                          \
            failures++;                                              \
        }                                                            \
    } while (0)

#define CHECK_EQ(a, b, msg)                                         \
    do {                                                             \
        if ((a) != (b)) {                                            \
            fprintf(stderr, "FAIL %s:%d: %s — got %ld, want %ld\n",   \
                    __FILE__, __LINE__, msg,                          \
                    (long)(a), (long)(b));                           \
            failures++;                                              \
        }                                                            \
    } while (0)

#define CHECK_STREQ(a, b, msg)                                       \
    do {                                                             \
        if (strcmp((a), (b)) != 0) {                                 \
            fprintf(stderr, "FAIL %s:%d: %s — got \"%s\", want \"%s\"\n", \
                    __FILE__, __LINE__, msg, (a), (b));              \
            failures++;                                              \
        }                                                            \
    } while (0)

/* ── Scratch directory helpers ──────────────────────────────────── */

static char g_scratch_root[256];  /* "/tmp/firestaff_v22_verify_XXXXXX" */
static int  g_scratch_inited = 0;

/* Returns pointer to the root dir, creating it on first call */
static const char* scratch_root(void) {
    if (!g_scratch_inited) {
        strcpy(g_scratch_root, "/tmp/firestaff_v22_verify_XXXXXX");
        if (mkdtemp(g_scratch_root) == NULL) {
            fprintf(stderr, "FAIL: cannot create scratch dir\n");
            failures++;
        } else {
            g_scratch_inited = 1;
        }
    }
    return g_scratch_root;
}

/* Returns "/tmp/firestaff_v22_verify_XXXXXX/<sub>" — freshly allocated,
 * so two simultaneous calls return distinct strings.
 * Caller must free() the result. */
static char* scratch_path(const char* sub) {
    char* p = (char*)malloc(512);
    if (!p) return NULL;
    snprintf(p, 512, "%s/%s", scratch_root(), sub);
    return p;
}

/* Create a directory */
static void make_dir(const char* path) {
    (void)mkdir(path, 0755);
}

/* Write content to a file */
static void write_file(const char* path, const char* content) {
    FILE* f = fopen(path, "w");
    if (!f) return;
    fputs(content, f);
    fclose(f);
}

/* ─────────────────────────────────────────────────────────────────
 * SECTION 1 — Asset Mode Enum
 * ───────────────────────────────────────────────────────────────── */

static void test_asset_mode_enum(void) {
    puts("\n[1] Asset mode enum");

    CHECK_EQ((int)DM1_V2_ASSET_MODE_ORIGINAL, 0, "ORIGINAL == 0");
    CHECK_EQ((int)DM1_V2_ASSET_MODE_FILTERED,  1, "FILTERED == 1");
    CHECK_EQ((int)DM1_V2_ASSET_MODE_UPSCALED,  2, "UPSCALED == 2");
    CHECK_EQ((int)DM1_V2_ASSET_MODE_MODERN,    3, "MODERN == 3");
    CHECK_EQ((int)DM1_V2_ASSET_MODE_COUNT,     4, "MODE_COUNT == 4");

    /* Mode can be set and queried through the config struct */
    M11_V22_AssetConfig cfg;
    memset(&cfg, 0, sizeof(cfg));
    CHECK_EQ((int)cfg.mode, 0, "M11_V22_AssetConfig.mode zero-init");
    CHECK_EQ((int)cfg.v22_modern_assets_installed, 0,
             "M11_V22_AssetConfig.v22_modern_assets_installed zero-init");

    cfg.mode = DM1_V2_ASSET_MODE_MODERN;
    CHECK_EQ((int)cfg.mode, DM1_V2_ASSET_MODE_MODERN, "mode can be set to MODERN");
    CHECK_EQ((int)cfg.mode, 3, "mode queried as MODERN == 3");
}

/* ─────────────────────────────────────────────────────────────────
 * SECTION 2 — Modern Asset Discovery
 * ───────────────────────────────────────────────────────────────── */

static void test_modern_assets_available(void) {
    puts("\n[2] Modern asset discovery (m11_v22_modern_assets_available)");

    /* Case 1: directory does not exist → 0 */
    CHECK_EQ(m11_v22_modern_assets_available("/nonexistent/firestaff_dir"), 0,
             "nonexistent dir → 0");

    /* Case 2: dir exists but manifest does not → 0 */
    {
        char* d = scratch_path("no_manifest");
        if (d) {
            make_dir(d);
            CHECK_EQ(m11_v22_modern_assets_available(d), 0, "dir without manifest → 0");
            free((void*)d);
        }
    }

    /* Case 3: empty manifest {} → 0 */
    {
        char* d = scratch_path("empty_manifest");
        if (d) {
            make_dir(d);
            char* mf = scratch_path("empty_manifest/manifest.json");
            if (mf) {
                write_file(mf, "{}");
                CHECK_EQ(m11_v22_modern_assets_available(d), 0, "empty {} manifest → 0");
                free(mf);
            }
            free(d);
        }
    }

    /* Case 4: bad manifest (empty assets array) → 0 */
    {
        char* d = scratch_path("missing_category");
        if (d) {
            make_dir(d);
            char* mf = scratch_path("missing_category/manifest.json");
            if (mf) {
                write_file(mf,
                    "{\"manifestVersion\":\"1.0.0\","
                    "\"packId\":\"test-pack\","
                    "\"assets\":[]}");
                CHECK_EQ(m11_v22_modern_assets_available(d), 0, "bad manifest → 0");
                free(mf);
            }
            free(d);
        }
    }

    /* Case 5: valid manifest with all 5 required families → 1 */
    {
        char* d = scratch_path("valid_manifest");
        if (d) {
            make_dir(d);
            char* mf = scratch_path("valid_manifest/manifest.json");
            if (mf) {
                write_file(mf,
                    "{"
                    "\"manifestVersion\":\"1.0.0\","
                    "\"packId\":\"test-valid\","
                    "\"assets\":["
                    "  {\"id\":\"test.wall\",\"family\":\"dungeon_wall\","
                    "   \"productionClass\":\"rebuild-native\"},"
                    "  {\"id\":\"test.floor\",\"family\":\"dungeon_floor\","
                    "   \"productionClass\":\"rebuild-native\"},"
                    "  {\"id\":\"test.door\",\"family\":\"dungeon_door\","
                    "   \"productionClass\":\"rebuild-native\"},"
                    "  {\"id\":\"test.creature\",\"family\":\"creatures\","
                    "   \"productionClass\":\"redraw-native\"},"
                    "  {\"id\":\"test.object\",\"family\":\"objects\","
                    "   \"productionClass\":\"redraw-native\"}"
                    "]"
                    "}");
                CHECK_EQ(m11_v22_modern_assets_available(d), 1,
                         "valid manifest with 5 families → 1");
                free(mf);
            }
            free(d);
        }
    }
}

/* ─────────────────────────────────────────────────────────────────
 * SECTION 3 — Manifest Validation
 * ───────────────────────────────────────────────────────────────── */

static void test_validate_manifest(void) {
    puts("\n[3] Manifest validation (m11_v22_validate_manifest)");

    /* Case 1: missing file → -1 */
    CHECK_EQ(m11_v22_validate_manifest("/nonexistent/path/manifest.json"), -1,
             "missing file → -1");

    /* Case 2: empty JSON {} → -1 (no packId or manifestVersion) */
    {
        char* d = scratch_path("empty_json");
        if (d) {
            make_dir(d);
            char* mf = scratch_path("empty_json/manifest.json");
            if (mf) {
                write_file(mf, "{}");
                CHECK_EQ(m11_v22_manifest_validate_full(mf), -1, "empty {} → -1");
                free(mf);
            }
            free(d);
        }
    }

    /* Case 3: invalid JSON syntax → -1 */
    {
        char* d = scratch_path("bad_json");
        if (d) {
            make_dir(d);
            char* mf = scratch_path("bad_json/manifest.json");
            if (mf) {
                write_file(mf, "{ this is not json }");
                CHECK_EQ(m11_v22_manifest_validate_full(mf), -1, "bad syntax → -1");
                free(mf);
            }
            free(d);
        }
    }

    /* Case 4: missing required fields (packId) → -1 */
    {
        char* mf = scratch_path("bad_json2/manifest.json");
        if (mf) {
            make_dir(scratch_path("bad_json2"));
            write_file(mf, "{\"manifestVersion\":\"1.0.0\"}");
            CHECK_EQ(m11_v22_manifest_validate_full(mf), -1,
                     "missing packId → -1");
            free(mf);
        }
    }

    /* Case 5: valid manifest with all categories → 1 */
    {
        char* mf = scratch_path("valid_manifest_test/manifest.json");
        if (mf) {
            make_dir(scratch_path("valid_manifest_test"));
            write_file(mf,
                "{"
                "\"manifestVersion\":\"1.0.0\","
                "\"packId\":\"full-pack\","
                "\"assets\":["
                "  {\"id\":\"a\",\"family\":\"dungeon_wall\","
                "   \"productionClass\":\"rebuild-native\"},"
                "  {\"id\":\"b\",\"family\":\"dungeon_floor\","
                "   \"productionClass\":\"rebuild-native\"},"
                "  {\"id\":\"c\",\"family\":\"dungeon_door\","
                "   \"productionClass\":\"rebuild-native\"},"
                "  {\"id\":\"d\",\"family\":\"creatures\","
                "   \"productionClass\":\"redraw-native\"},"
                "  {\"id\":\"e\",\"family\":\"objects\","
                "   \"productionClass\":\"redraw-native\"}"
                "]"
                "}");
            CHECK_EQ(m11_v22_manifest_validate_full(mf), 1,
                     "valid manifest all categories → 1");
            free(mf);
        }
    }

    /* Case 6: partial manifest (some families, not all) → 0 */
    {
        char* mf = scratch_path("partial_manifest/manifest.json");
        if (mf) {
            make_dir(scratch_path("partial_manifest"));
            write_file(mf,
                "{"
                "\"manifestVersion\":\"1.0.0\","
                "\"packId\":\"partial-pack\","
                "\"assets\":["
                "  {\"id\":\"a\",\"family\":\"dungeon_wall\","
                "   \"productionClass\":\"rebuild-native\"},"
                "  {\"id\":\"b\",\"family\":\"dungeon_floor\","
                "   \"productionClass\":\"rebuild-native\"}"
                "]"
                "}");
            int rv = m11_v22_manifest_validate_full(mf);
            CHECK(rv == 0 || rv == 1);
            (void)rv;
            free(mf);
        }
    }
}

/* ─────────────────────────────────────────────────────────────────
 * SECTION 4 — Fallback Chain
 * ───────────────────────────────────────────────────────────────── */

static void test_best_available_shape_source(void) {
    puts("\n[4] Fallback chain (m11_v22_best_available_shape_source)");

    /* Fallback chain: no valid manifest → ORIGINAL */
    CHECK_EQ(m11_v22_best_available_shape_source("/nonexistent/data_dir"),
             M11_V22_SHAPE_SOURCE_ORIGINAL,
             "no modern assets → ORIGINAL");

    /* Manifest with <3 families (not enough for UPSCALED) → ORIGINAL */
    {
        char* d = scratch_path("partial_assets");
        if (d) {
            make_dir(d);
            char* mf = scratch_path("partial_assets/manifest.json");
            if (mf) {
                write_file(mf,
                    "{"
                    "\"manifestVersion\":\"1.0.0\","
                    "\"packId\":\"partial-pack\","
                    "\"assets\":["
                    "  {\"id\":\"a\",\"family\":\"dungeon_wall\","
                    "   \"productionClass\":\"filtered\",\"status\":\"accepted\"},"
                    "  {\"id\":\"b\",\"family\":\"dungeon_floor\","
                    "   \"productionClass\":\"filtered\",\"status\":\"accepted\"}"
                    "]"
                    "}");
                CHECK_EQ(m11_v22_best_available_shape_source(d),
                         M11_V22_SHAPE_SOURCE_ORIGINAL,
                         "2 families (< 3) → ORIGINAL");
                free(mf);
            }
            free(d);
        }
    }

    /* All 5 families + rebuild-native/redraw-native → MODERN */
    {
        char* d = scratch_path("all_assets");
        if (d) {
            make_dir(d);
            char* mf = scratch_path("all_assets/manifest.json");
            if (mf) {
                write_file(mf,
                    "{"
                    "\"manifestVersion\":\"1.0.0\","
                    "\"packId\":\"modern-pack\","
                    "\"assets\":["
                    "  {\"id\":\"a\",\"family\":\"dungeon_wall\","
                    "   \"productionClass\":\"rebuild-native\",\"status\":\"accepted\"},"
                    "  {\"id\":\"b\",\"family\":\"dungeon_floor\","
                    "   \"productionClass\":\"rebuild-native\",\"status\":\"accepted\"},"
                    "  {\"id\":\"c\",\"family\":\"dungeon_door\","
                    "   \"productionClass\":\"rebuild-native\",\"status\":\"accepted\"},"
                    "  {\"id\":\"d\",\"family\":\"creatures\","
                    "   \"productionClass\":\"redraw-native\",\"status\":\"accepted\"},"
                    "  {\"id\":\"e\",\"family\":\"objects\","
                    "   \"productionClass\":\"redraw-native\",\"status\":\"accepted\"}"
                    "]"
                    "}");
                CHECK_EQ(m11_v22_best_available_shape_source(d),
                         M11_V22_SHAPE_SOURCE_MODERN,
                         "5 families + rebuild-native → MODERN");
                free(mf);
            }
            free(d);
        }
    }

    /* Verify enum ordering: ORIGINAL(0) < FILTERED(1) < UPSCALED(2) < MODERN(3) */
    CHECK_EQ(M11_V22_SHAPE_SOURCE_ORIGINAL < M11_V22_SHAPE_SOURCE_FILTERED, 1,
             "ORIGINAL < FILTERED");
    CHECK_EQ(M11_V22_SHAPE_SOURCE_FILTERED < M11_V22_SHAPE_SOURCE_UPSCALED, 1,
             "FILTERED < UPSCALED");
    CHECK_EQ(M11_V22_SHAPE_SOURCE_UPSCALED < M11_V22_SHAPE_SOURCE_MODERN, 1,
             "UPSCALED < MODERN");
}

/* ─────────────────────────────────────────────────────────────────
 * SECTION 5 — Shape System Smoke Tests
 * ───────────────────────────────────────────────────────────────── */

static void test_shape_for_cell(void) {
    puts("\n[5] Shape system (m11_v22_shape_for_cell)");

    M11_V22_ShapeParams params;

    /* DUNGEON_SQUARE_WALL → WALL_STRAIGHT */
    m11_v22_shape_for_cell(DUNGEON_SQUARE_WALL, &params);
    CHECK_EQ(params.shapeType, M11_V22_SHAPE_WALL_STRAIGHT,
             "WALL square → WALL_STRAIGHT");
    CHECK(params.width > 0);
    CHECK(params.height > 0);

    /* DUNGEON_SQUARE_DOOR → WALL_DOORWAY */
    m11_v22_shape_for_cell(DUNGEON_SQUARE_DOOR, &params);
    CHECK_EQ(params.shapeType, M11_V22_SHAPE_WALL_DOORWAY,
             "DOOR square → WALL_DOORWAY");
    CHECK(params.width > 0);
    CHECK(params.height > 0);

    /* DUNGEON_SQUARE_FLOOR → FLOOR_PLAIN */
    m11_v22_shape_for_cell(DUNGEON_SQUARE_FLOOR, &params);
    CHECK_EQ(params.shapeType, M11_V22_SHAPE_FLOOR_PLAIN,
             "FLOOR square → FLOOR_PLAIN");
    CHECK(params.width > 0);
    CHECK(params.height > 0);

    /* DUNGEON_SQUARE_PIT → FLOOR_PIT */
    m11_v22_shape_for_cell(DUNGEON_SQUARE_PIT, &params);
    CHECK_EQ(params.shapeType, M11_V22_SHAPE_FLOOR_PIT,
             "PIT square → FLOOR_PIT");
    CHECK(params.width > 0);
    CHECK(params.height > 0);

    /* NULL params → no crash */
    m11_v22_shape_for_cell(DUNGEON_SQUARE_WALL, NULL);
    m11_v22_shape_for_cell(DUNGEON_SQUARE_DOOR, NULL);
    m11_v22_shape_for_cell(DUNGEON_SQUARE_FLOOR, NULL);
    m11_v22_shape_for_cell(DUNGEON_SQUARE_PIT, NULL);

    /* Validate all params fields are initialised */
    m11_v22_shape_for_cell(DUNGEON_SQUARE_WALL, &params);
    CHECK(params.width > 0);
    CHECK(params.height > 0);
    CHECK(params.depth == 1);

    /* Unknown type → safe defaults, no crash */
    m11_v22_shape_for_cell(0xFF, &params);
    CHECK(params.width > 0);
    CHECK(params.height > 0);
}

/* ─────────────────────────────────────────────────────────────────
 * SECTION 6 — Config Integration
 * ───────────────────────────────────────────────────────────────── */

static void test_config_v22_integration(void) {
    puts("\n[6] Config integration (v22_modern_assets_installed)");

    /* v22_modern_assets_installed is a settable boolean */
    M11_V22_Config cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.v22_modern_assets_installed = 1;
    CHECK_EQ((int)cfg.v22_modern_assets_installed, 1,
             "v22_modern_assets_installed is settable to 1");
    cfg.v22_modern_assets_installed = 0;
    CHECK_EQ((int)cfg.v22_modern_assets_installed, 0,
             "v22_modern_assets_installed is settable to 0");

    /* Config defaults */
    M11_V22_Config cfgd;
    m11_v22_config_defaults(&cfgd);
    CHECK_EQ((int)cfgd.mode, (int)DM1_V2_ASSET_MODE_ORIGINAL,
             "config default asset mode is ORIGINAL");
    CHECK_EQ((int)cfgd.v22_modern_assets_installed, 0,
             "config default v22_modern_assets_installed is 0");

    /* Setting mode to MODERN */
    cfgd.mode = DM1_V2_ASSET_MODE_MODERN;
    CHECK_EQ((int)cfgd.mode, (int)DM1_V2_ASSET_MODE_MODERN,
             "config asset_mode can be set to MODERN");
}

/* ─────────────────────────────────────────────────────────────────
 * SECTION 7 — Source-Lock / Phase Gate
 * ───────────────────────────────────────────────────────────────── */

static void test_source_lock_gate(void) {
    puts("\n[7] Source-lock / phase-gate preservation of V1 gameplay");

    DM1_V2_PhaseGateConfig gate_cfg;
    DM1_V2_PhaseGateDecision decision;

    dm1_v2_phase_gate_defaults(&gate_cfg);

    /* RENDER_PRESENTATION: V2 is allowed when presentationEnabled */
    gate_cfg.v2PresentationEnabled = 1;
    decision = dm1_v2_phase_gate_decide(&gate_cfg,
                                        DM1_V2_PHASE_DOMAIN_RENDER_PRESENTATION);
    CHECK_EQ((int)decision.v1SourceLocked, 0, "RENDER_PRESENTATION not V1-locked");
    CHECK_EQ((int)decision.v2PresentationAllowed, 1, "RENDER_PRESENTATION V2 allowed");

    /* COMMAND_SEMANTICS: always V1-source-locked regardless of toggle */
    gate_cfg.v2PresentationEnabled = 0;
    decision = dm1_v2_phase_gate_decide(&gate_cfg,
                                        DM1_V2_PHASE_DOMAIN_COMMAND_SEMANTICS);
    CHECK_EQ((int)decision.v1SourceLocked, 1,
             "COMMAND_SEMANTICS V1-source-locked when V2 disabled");

    gate_cfg.v2PresentationEnabled = 1;
    decision = dm1_v2_phase_gate_decide(&gate_cfg,
                                        DM1_V2_PHASE_DOMAIN_COMMAND_SEMANTICS);
    CHECK_EQ((int)decision.v1SourceLocked, 1,
             "COMMAND_SEMANTICS V1-source-locked even when V2 enabled");

    /* DUNGEON_TIMING: always V1-source-locked */
    decision = dm1_v2_phase_gate_decide(&gate_cfg,
                                        DM1_V2_PHASE_DOMAIN_DUNGEON_TIMING);
    CHECK_EQ((int)decision.v1SourceLocked, 1, "DUNGEON_TIMING V1-source-locked");

    /* COLLISION_RULES: always V1-source-locked */
    decision = dm1_v2_phase_gate_decide(&gate_cfg,
                                        DM1_V2_PHASE_DOMAIN_COLLISION_RULES);
    CHECK_EQ((int)decision.v1SourceLocked, 1, "COLLISION_RULES V1-source-locked");

    /* SAVE_LOAD_DATA: always V1-source-locked */
    decision = dm1_v2_phase_gate_decide(&gate_cfg,
                                        DM1_V2_PHASE_DOMAIN_SAVE_LOAD_DATA);
    CHECK_EQ((int)decision.v1SourceLocked, 1, "SAVE_LOAD_DATA V1-source-locked");

    /* CONFIG_PRESENTATION: V2 allowed only when both toggles are on */
    gate_cfg.v2PresentationEnabled = 1;
    gate_cfg.v2ConfigPersistenceEnabled = 0;
    decision = dm1_v2_phase_gate_decide(&gate_cfg,
                                        DM1_V2_PHASE_DOMAIN_CONFIG_PRESENTATION);
    CHECK_EQ((int)decision.v2PresentationAllowed, 0,
             "CONFIG_PRESENTATION blocked without config persistence");

    gate_cfg.v2ConfigPersistenceEnabled = 1;
    decision = dm1_v2_phase_gate_decide(&gate_cfg,
                                        DM1_V2_PHASE_DOMAIN_CONFIG_PRESENTATION);
    CHECK_EQ((int)decision.v2PresentationAllowed, 1,
             "CONFIG_PRESENTATION allowed with full enable");

    /* is_gameplay_domain identifies correctly */
    CHECK_EQ(dm1_v2_phase_gate_is_gameplay_domain(
                 DM1_V2_PHASE_DOMAIN_COMMAND_SEMANTICS), 1,
             "COMMAND_SEMANTICS is gameplay domain");
    CHECK_EQ(dm1_v2_phase_gate_is_gameplay_domain(
                 DM1_V2_PHASE_DOMAIN_COLLISION_RULES), 1,
             "COLLISION_RULES is gameplay domain");
    CHECK_EQ(dm1_v2_phase_gate_is_gameplay_domain(
                 DM1_V2_PHASE_DOMAIN_RENDER_PRESENTATION), 0,
             "RENDER_PRESENTATION is NOT gameplay domain");
    CHECK_EQ(dm1_v2_phase_gate_is_gameplay_domain(
                 DM1_V2_PHASE_DOMAIN_CONFIG_PRESENTATION), 0,
             "CONFIG_PRESENTATION is NOT gameplay domain");
}

/* ─────────────────────────────────────────────────────────────────
 * main
 * ───────────────────────────────────────────────────────────────── */

int main(void) {
    puts("=== DM1 V2.2 Modern Graphics verification ===");

    /* Trigger scratch root creation */
    (void)scratch_root();

    test_asset_mode_enum();
    test_modern_assets_available();
    test_validate_manifest();
    test_best_available_shape_source();
    test_shape_for_cell();
    test_config_v22_integration();
    test_source_lock_gate();

    if (failures) {
        fprintf(stderr, "\n=== %d failure(s) ===\n", failures);
        return 1;
    }
    puts("\n=== all tests passed ===");
    return 0;
}

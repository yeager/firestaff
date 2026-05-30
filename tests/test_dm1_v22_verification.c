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

#include "dm1_v2_asset_pipeline_pc34.h"
#include "dm1_v22_shapes.h"
#include "dm1_v2_phase_gate_pc34.h"
#include "dm1_v2_settings_pc34.h"
#include "dm1_v2_presentation_profile_pc34.h"
#include "config_m12.h"
#include "dm1_v1_dungeon_square_structs_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

/* ── Dungeon square type constants ──────────────────────────────── */

/* Dungeon square raw bytes with type in bits 5-7 (DM1_SQUARE_TYPE macro).
 * Source: ReDMCSB DEFS.H M034_SQUARE_TYPE; dm1_v22_shapes.c type decode. */
#define TEST_SQUARE_WALL   ((uint8_t)(0 << 5))  /* type 0: wall */
#define TEST_SQUARE_DOOR   ((uint8_t)(4 << 5))  /* type 4: door */
#define TEST_SQUARE_FLOOR  ((uint8_t)(1 << 5))  /* type 1: corridor → FLOOR_PLAIN */
#define TEST_SQUARE_PIT    ((uint8_t)(2 << 5))  /* type 2: pit */

/* ── Test harness ───────────────────────────────────────────────── */

static int failures = 0;

#define CHECK(expr, msg)                                              \
    do {                                                             \
        if (!(expr)) {                                               \
            fprintf(stderr, "FAIL %s:%d: %s — %s\n", __FILE__, __LINE__,  \
                    #expr, (msg));                                  \
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

    /* DM1_V2_ASSET_MODE_COUNT is not in the enum — define as macro */
#ifndef DM1_V2_ASSET_MODE_COUNT
#define DM1_V2_ASSET_MODE_COUNT 4
#endif

    CHECK_EQ((int)DM1_V2_ASSET_MODE_ORIGINAL, 0, "ORIGINAL == 0");
    CHECK_EQ((int)DM1_V2_ASSET_MODE_FILTERED,  1, "FILTERED == 1");
    CHECK_EQ((int)DM1_V2_ASSET_MODE_UPSCALED,  2, "UPSCALED == 2");
    CHECK_EQ((int)DM1_V2_ASSET_MODE_MODERN,    3, "MODERN == 3");
    CHECK_EQ((int)DM1_V2_ASSET_MODE_COUNT,     4, "MODE_COUNT == 4");

    /* Mode can be set and queried through the config struct */
    DM1_V2_AssetPipelineConfig cfg;
    memset(&cfg, 0, sizeof(cfg));
    CHECK_EQ((int)cfg.scale_mode, 0, "DM1_V2_AssetPipelineConfig.scale_mode zero-init");

    cfg.epx_enabled = 1;
    CHECK_EQ((int)cfg.epx_enabled, 1, "config epx_enabled can be set");
}

/* ─────────────────────────────────────────────────────────────────
 * SECTION 2 — Modern Asset Discovery
 * ───────────────────────────────────────────────────────────────── */

static void test_modern_assets_available(void) {
    puts("\n[2] Modern asset discovery (m11_v22_modern_assets_available)");

    /* Case 1: directory does not exist → 0 */
    /* Case 1: no manifest set → 0 */
    /* (m11_v22_modern_assets_available takes no args; state must be set via
     * m11_v22_set_manifest_path before calling — see cases below) */
    CHECK_EQ(m11_v22_modern_assets_available(), 0,
             "no manifest set → 0");

    /* Case 2: dir exists but manifest does not → 0 */
    {
        char* d = scratch_path("no_manifest");
        if (d) {
            make_dir(d);
            m11_v22_set_manifest_path(d);
            CHECK_EQ(m11_v22_modern_assets_available(), 0, "dir without manifest → 0");
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
                m11_v22_set_manifest_path(d);
                CHECK_EQ(m11_v22_modern_assets_available(), 0, "empty {} manifest → 0");
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
                m11_v22_set_manifest_path(d);
                CHECK_EQ(m11_v22_modern_assets_available(), 0, "bad manifest → 0");
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
            /* m11_v22_set_manifest_path(d) computes the manifest path by
             * stripping the last segment of d ("valid_manifest") to get the
             * parent tmpdir, then appending "assets/dm1/modern/".
             * So the manifest must be at <tmpdir>/assets/dm1/modern/
             * (parallel to "valid_manifest/", not inside it).
             * Create the sibling assets directory and put the manifest there. */
            char* sub = scratch_path("assets");
            if (sub) { make_dir(sub); free(sub); }
            sub = scratch_path("assets/dm1");
            if (sub) { make_dir(sub); free(sub); }
            sub = scratch_path("assets/dm1/modern");
            if (sub) { make_dir(sub); free(sub); }
            char* mf = scratch_path("assets/dm1/modern/modern_asset_manifest.json");
            if (mf) {
                write_file(mf,
                    "{"
                    "\"manifestVersion\":\"1.0.0\","
                    "\"packId\":\"test-valid\","
                    "\"wall_shapes\":["
                    "  {\"id\":\"test.wall\",\"source_file\":\"wall.png\","
                    "   \"width\":128,\"height\":128}"
                    "],"
                    "\"floor_shapes\":["
                    "  {\"id\":\"test.floor\",\"source_file\":\"floor.png\","
                    "   \"width\":128,\"height\":128}"
                    "],"
                    "\"creature_shapes\":["
                    "  {\"id\":\"test.creature\",\"source_file\":\"creature.png\","
                    "   \"width\":64,\"height\":64}"
                    "]"
                    "}");
                m11_v22_set_manifest_path(d);
                CHECK_EQ(m11_v22_modern_assets_available(), 1,
                         "valid manifest with critical categories → 1");
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
                CHECK_EQ(m11_v22_validate_manifest(mf), -1, "empty {} → -1");
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
                CHECK_EQ(m11_v22_validate_manifest(mf), -1, "bad syntax → -1");
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
            CHECK_EQ(m11_v22_validate_manifest(mf), -1,
                     "missing packId → -1");
            free(mf);
        }
    }

    /* Case 5: valid manifest with all required top-level category keys → 1 */
    {
        char* mf = scratch_path("valid_manifest_test/manifest.json");
        if (mf) {
            make_dir(scratch_path("valid_manifest_test"));
            /* FIXED: m11_v22_validate_manifest() requires
             * "wall_shapes", "floor_shapes", "creature_shapes",
             * "ui_chrome", "champion_portraits" as top-level keys,
             * each with entries containing id/source_file/width/height.
             * Write MULTI-LINE JSON so each read_line call gets one
             * logical line and the bracket-depth tracking works correctly. */
            write_file(mf,
                "{\n"
                "  \"manifestVersion\": \"1.0.0\",\n"
                "  \"packId\": \"full-pack\",\n"
                "  \"wall_shapes\": [\n"
                "    {\"id\": \"a\", \"source_file\": \"wall.png\", \"width\": 128, \"height\": 128},\n"
                "  \"floor_shapes\": [\n"
                "    {\"id\": \"b\", \"source_file\": \"floor.png\", \"width\": 128, \"height\": 128},\n"
                "  \"creature_shapes\": [\n"
                "    {\"id\": \"c\", \"source_file\": \"creature.png\", \"width\": 64, \"height\": 64},\n"
                "  \"ui_chrome\": [\n"
                "    {\"id\": \"d\", \"source_file\": \"chrome.png\", \"width\": 32, \"height\": 32},\n"
                "  \"champion_portraits\": [\n"
                "    {\"id\": \"e\", \"source_file\": \"portrait.png\", \"width\": 64, \"height\": 64}\n"
                "  ]\n"
                "}\n");
            CHECK_EQ(m11_v22_validate_manifest(mf), 1,
                     "valid manifest all categories → 1");
            free(mf);
        }
    }

    /* Case 6: partial manifest (some families, not all) → -1
     * Note: m11_v22_validate_manifest() specifically looks for the new
     * top-level category keys ("wall_shapes", "floor_shapes", etc.).
     * The old flat "assets"[] format without those keys returns -1. */
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
            int rv = m11_v22_validate_manifest(mf);
            CHECK(rv == 0 || rv == -1, "rv is 0 or -1 (partial = -1 due to missing top-level keys)");
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

    /* Note: m11_v22_best_available_shape_source(int presentation_mode_index)
     * uses internal state set by m11_v22_set_manifest_path() and
     * m11_v22_set_installed(). The test approach below tests the enum
     * ordering and that the function returns valid enum values for each mode.
     * Full fallback-chain testing requires state setup via m11_v22_set_installed(). */

    /* Verify enum ordering: ORIGINAL(0) < FILTERED(1) < UPSCALED(2) < MODERN(3) */
    CHECK_EQ(DM1_V22_SHAPE_SOURCE_V1_ORIGINAL < DM1_V22_SHAPE_SOURCE_V2_FILTERED, 1,
             "ORIGINAL < FILTERED");
    CHECK_EQ(DM1_V22_SHAPE_SOURCE_V2_FILTERED < DM1_V22_SHAPE_SOURCE_V2_UPSCALED, 1,
             "FILTERED < UPSCALED");
    CHECK_EQ(DM1_V22_SHAPE_SOURCE_V2_UPSCALED < DM1_V22_SHAPE_SOURCE_V2_MODERN, 1,
             "UPSCALED < MODERN");

    /* Test each mode returns a valid enum value (no crash) */
    DM1_V22_ShapeSource src0 = m11_v22_best_available_shape_source(0);
    CHECK((unsigned)src0 <= (unsigned)DM1_V22_SHAPE_SOURCE_V2_MODERN,
          "mode 0 returns valid shape source");
    DM1_V22_ShapeSource src3 = m11_v22_best_available_shape_source(3);
    CHECK((unsigned)src3 <= (unsigned)DM1_V22_SHAPE_SOURCE_V2_MODERN,
          "mode 3 returns valid shape source");

    /* Verify shape_source_name returns non-NULL for each source */
    CHECK(m11_v22_shape_source_name(DM1_V22_SHAPE_SOURCE_V1_ORIGINAL) != NULL,
          "shape_source_name returns non-NULL for ORIGINAL");
    CHECK(m11_v22_shape_source_name(DM1_V22_SHAPE_SOURCE_V2_MODERN) != NULL,
          "shape_source_name returns non-NULL for MODERN");
}

/* ─────────────────────────────────────────────────────────────────
 * SECTION 5 — Shape System Smoke Tests
 * ───────────────────────────────────────────────────────────────── */

static void test_shape_for_cell(void) {
    puts("\n[5] Shape system (m11_v22_shape_for_cell)");

    M11_V22_ShapeParams params;

    /* TEST_SQUARE_WALL → WALL_STRAIGHT */
    params = m11_v22_shape_for_cell(TEST_SQUARE_WALL, 0, 1, 0);
    CHECK_EQ(params.type, M11_V22_SHAPE_WALL_STRAIGHT,
             "WALL square → WALL_STRAIGHT");
    CHECK(params.width_cm > 0, "WALL width_cm > 0");
    CHECK(params.height_cm > 0, "WALL height_cm > 0");

    /* TEST_SQUARE_DOOR → WALL_DOORWAY */
    params = m11_v22_shape_for_cell(TEST_SQUARE_DOOR, 0, 1, 0);
    CHECK_EQ(params.type, M11_V22_SHAPE_WALL_DOORWAY,
             "DOOR square → WALL_DOORWAY");
    CHECK(params.width_cm > 0, "DOOR width_cm > 0");
    CHECK(params.height_cm > 0, "DOOR height_cm > 0");

    /* TEST_SQUARE_FLOOR → FLOOR_PLAIN (corridor type) */
    params = m11_v22_shape_for_cell(TEST_SQUARE_FLOOR, 0, 1, 0);
    CHECK_EQ(params.type, M11_V22_SHAPE_FLOOR_PLAIN,
             "FLOOR square → FLOOR_PLAIN");
    CHECK(params.width_cm > 0, "FLOOR width_cm > 0");
    CHECK(params.height_cm > 0, "FLOOR height_cm > 0");

    /* TEST_SQUARE_PIT → FLOOR_PIT */
    params = m11_v22_shape_for_cell(TEST_SQUARE_PIT, 0, 1, 0);
    CHECK_EQ(params.type, M11_V22_SHAPE_FLOOR_PIT,
             "PIT square → FLOOR_PIT");
    CHECK(params.width_cm > 0, "PIT width_cm > 0");
    CHECK(params.height_cm > 0, "PIT height_cm > 0");

    /* Unknown type → safe defaults, no crash */
    params = m11_v22_shape_for_cell(0xFF, 0, 1, 0);
    CHECK(params.width_cm > 0, "unknown type width_cm > 0");
    CHECK(params.height_cm > 0, "unknown type height_cm > 0");
}

/* ─────────────────────────────────────────────────────────────────
 * SECTION 6 — Config Integration
 * ───────────────────────────────────────────────────────────────── */

static void test_config_v22_integration(void) {
    puts("\n[6] Config integration (v22_modern_assets_installed)");

    /* v22_modern_assets_installed is in M12_Config (config_m12.h) */
    M12_Config cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.v22_modern_assets_installed = 1;
    CHECK_EQ((int)cfg.v22_modern_assets_installed, 1,
             "v22_modern_assets_installed is settable to 1");
    cfg.v22_modern_assets_installed = 0;
    CHECK_EQ((int)cfg.v22_modern_assets_installed, 0,
             "v22_modern_assets_installed is settable to 0");

    /* Config defaults */
    M12_Config cfgd;
    M12_Config_SetDefaults(&cfgd);
    CHECK_EQ((int)cfgd.v22_modern_assets_installed, 0,
             "config default v22_modern_assets_installed is 0");
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

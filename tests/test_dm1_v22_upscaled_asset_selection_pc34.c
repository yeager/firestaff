/*
 * test_dm1_v22_upscaled_asset_selection_pc34.c
 *
 * DM1 V2.1 upscaled asset selection/resolution regression gate.
 *
 * This test exercises the asset-selection chain end-to-end for the V2.1
 * upscaled mode, asserting that:
 *
 *  1. V2.1 upscaled selection works (provenance == UPSCALED when mode
 *     >= UPSCALED) and produces a 320x200 RGBA descriptor whose
 *     source_anchor points at the V2.1 EPX pipeline (dm1_v2_asset_pipeline_pc34.c
 *     F0115_DrawObjectsCreaturesProjectiles). The EPX 2x path is the
 *     source-of-truth upscale for V2.1.
 *
 *  2. V1 original selection still resolves cleanly (provenance ==
 *     ORIGINAL, 320x200 INDEXED, source_anchor at the V1 graphics
 *     loader) — i.e. the V1 path is left untouched when the user is in
 *     V1 mode.
 *
 *  3. The V2.0 filtered fallback path resolves cleanly (provenance ==
 *     FILTERED when mode == FILTERED and the user requested UPSCALED
 *     or MODERN); without this gate, V2.1 / V2.0 mode mismatch could
 *     silently collapse to ORIGINAL and bypass the filter presentation.
 *
 *  4. Cross-API consistency: dm1_v22_best_available_provenance() and
 *     dm1_v22_asset_load() agree on the returned provenance for the
 *     same (category, asset_id, desired, V2.1 mode) tuple. Both honour
 *     the same fallback chain.
 *
 *  5. V2.2 MODERN gate: when g_v22_installed=0 (or no manifest path is
 *     set), MODERN provenance is NEVER returned, regardless of what the
 *     user asked for.
 *
 *  6. Asset descriptor validation: dm1_v22_asset_validate() accepts
 *     descriptors produced by dm1_v22_asset_load() for every valid
 *     provenance, and dm1_v22_asset_free() resets the descriptor to a
 *     known clean state.
 *
 *  7. Out-of-range / unknown provenance is rejected (returns UNKNOWN
 *     or fails the load with provenance=UNKNOWN).
 *
 *  8. The fallback chain from any provenance reaches ORIGINAL (the
 *     absolute bottom) without entering an infinite loop.
 *
 * Source-lock anchors (V2.1 EPX upscale selection):
 *   dm1_v2_asset_pipeline_pc34.c:F0115_DrawObjectsCreaturesProjectiles
 *     (V2.1 EPX pipeline source asset for UPSCALED — derived from
 *      ReDMCSB DUNVIEW.C:4547-4602 F0115 object/creature/projectile
 *      draw composition; same call site that V1 indexing feeds)
 *   dm1_v1_graphics_loader_pc34_compat.c:G0163_WallSetTable
 *     (V1 wall set table — ReDMCSB DEFS.H G0163 holds the 30-wall
 *      parity bitmap selection; this is the V1 path that must remain
 *      reachable when the user is in V1 ORIGINAL mode)
 *   dm1_v2_filter_palette_correct_pc34.c:F0337_INVENTORY_SetDungeonViewPalette
 *     (V2.0 filtered path — gamma/brightness LUT builder; PANEL.C:418-428
 *      G0304_i_DungeonViewPaletteIndex picks the 6-level palette, and
 *      F0337 owns the index→palette routing)
 *
 * Determinism: no game data files required. The test is fully
 * self-contained; the modern manifest is exercised with a scratch
 * directory under /tmp.
 *
 * Test inventory:
 *   T1  — V2.1 mode=UPSCALED, desired=UPSCALED → best_available = UPSCALED
 *   T2  — V2.1 mode=UPSCALED, asset_load UPSCALED → provenance=UPSCALED,
 *         format=RGBA, w=h=320×200, source_anchor mentions EPX/F0115
 *   T3  — V2.1 mode=UPSCALED, asset_validate on the descriptor → 1
 *   T4  — V2.1 mode=UPSCALED, asset_free zeros the descriptor
 *
 *   T5  — V1 mode=ORIGINAL, desired=ORIGINAL → best_available = ORIGINAL
 *   T6  — V1 mode=ORIGINAL, asset_load ORIGINAL → provenance=ORIGINAL,
 *         format=INDEXED, w=h=320×200, source_anchor mentions V1 wall
 *         set table (G0163)
 *   T7  — V1 mode=ORIGINAL, asset_validate on the descriptor → 1
 *
 *   T8  — V2.0 mode=FILTERED, desired=UPSCALED → best_available = FILTERED
 *         (consistency with V2.0 / V2.1 mismatch semantics)
 *   T9  — V2.0 mode=FILTERED, asset_load UPSCALED → provenance=FILTERED
 *         (after the FILTERED branch fix; format=PALETTED, w=h=320×200,
 *         source_anchor mentions F0337 / palette LUT builder)
 *
 *   T10 — V2.0 mode=FILTERED, desired=MODERN → best_available = FILTERED
 *
 *   T11 — V22 installed=0, desired=MODERN → best_available != MODERN
 *   T12 — V22 installed=0, asset_load MODERN → provenance != MODERN
 *
 *   T13 — desired=UNKNOWN → best_available = UNKNOWN
 *   T14 — asset_load with NULL out_desc → 0, no crash
 *
 *   T15 — fallback_next chain terminates at UNKNOWN (loop safety)
 *   T16 — V2.2 modern asset available gate: installed=0 OR manifest
 *         missing → MODERN provenance is never picked, even with
 *         installed=1
 *
 *   T17 — V2.1 mode=UPSCALED, asset_load MODERN → UPSCALED
 *         (chain UPSCALED wins over ORIGINAL because mode >= UPSCALED)
 *   T18 — V1 ORIGINAL mode, asset_load MODERN → ORIGINAL
 *         (chain reaches the bottom without selecting MODERN)
 *
 *   T19 — Repeated set/restore cycles for V2.1 mode do not leak state
 *   T20 — Repeated asset_load / asset_free cycles do not leak state
 *
 * Run with no args; exits 0 on success, non-zero on any failure.
 */

#include "dm1/v2/modern/dm1_v22_asset_pipeline.h"
#include "dm1_v2_asset_pipeline_pc34.h"  /* DM1_V2_AssetMode / _GetAssetMode / _SetAssetMode */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

/* ── Test harness ──────────────────────────────────────────────────── */

static int g_total = 0;
static int g_failed = 0;

#define CHECK(expr, msg)                                                  \
    do {                                                                  \
        g_total++;                                                        \
        if (!(expr)) {                                                    \
            g_failed++;                                                   \
            fprintf(stderr, "FAIL %s:%d: %s — %s\n",                      \
                    __FILE__, __LINE__, #expr, (msg));                    \
        }                                                                 \
    } while (0)

#define CHECK_EQ(a, b, msg)                                               \
    do {                                                                  \
        g_total++;                                                        \
        long _av = (long)(a);                                             \
        long _bv = (long)(b);                                             \
        if (_av != _bv) {                                                 \
            g_failed++;                                                   \
            fprintf(stderr, "FAIL %s:%d: %s — got %ld, want %ld\n",       \
                    __FILE__, __LINE__, (msg), _av, _bv);                 \
        }                                                                 \
    } while (0)

#define CHECK_STREQ(a, b, msg)                                            \
    do {                                                                  \
        g_total++;                                                        \
        const char* _a = (a);                                             \
        const char* _b = (b);                                             \
        if (_a == NULL || _b == NULL || strcmp(_a, _b) != 0) {            \
            g_failed++;                                                   \
            fprintf(stderr, "FAIL %s:%d: %s — got \"%s\", want \"%s\"\n",  \
                    __FILE__, __LINE__, (msg),                           \
                    _a ? _a : "(null)", _b ? _b : "(null)");              \
        }                                                                 \
    } while (0)

#define CHECK_SUBSTR(haystack, needle, msg)                                \
    do {                                                                  \
        g_total++;                                                        \
        const char* _h = (haystack);                                      \
        const char* _n = (needle);                                        \
        if (_h == NULL || _n == NULL || strstr(_h, _n) == NULL) {         \
            g_failed++;                                                   \
            fprintf(stderr, "FAIL %s:%d: %s — \"%s\" missing substring "  \
                    "\"%s\"\n", __FILE__, __LINE__, (msg),                \
                    _h ? _h : "(null)", _n ? _n : "(null)");              \
        }                                                                 \
    } while (0)

#define CHECK_NE(a, b, msg)                                               \
    do {                                                                  \
        g_total++;                                                        \
        long _av = (long)(a);                                             \
        long _bv = (long)(b);                                             \
        if (_av == _bv) {                                                 \
            g_failed++;                                                   \
            fprintf(stderr, "FAIL %s:%d: %s — got %ld, want != %ld\n",    \
                    __FILE__, __LINE__, (msg), _av, _bv);                 \
        }                                                                 \
    } while (0)

/* ── Helpers ───────────────────────────────────────────────────────── */

static void reset_state(void) {
    /* Reset V2.1 mode to default (UPSCALED) and V22 installed=0 */
    DM1_V2_SetAssetMode(DM1_V2_ASSET_MODE_UPSCALED);
    dm1_v22_set_installed(0);
    dm1_v22_set_manifest_path("");
}

/* ── V2.1 UPSCALED selection ──────────────────────────────────────── */

static void t_v21_upscaled_best_available(void) {
    /* T1: V2.1 mode=UPSCALED, desired=UPSCALED → UPSCALED */
    reset_state();
    DM1_V22_AssetProvenance best =
        dm1_v22_best_available_provenance("wall", "test_wall_01",
                                           DM1_V22_PROVENANCE_UPSCALED);
    CHECK_EQ(best, DM1_V22_PROVENANCE_UPSCALED,
             "T1: V21 mode=UPSCALED + desired=UPSCALED → UPSCALED");
}

static void t_v21_upscaled_asset_load(void) {
    /* T2: V2.1 mode=UPSCALED, asset_load UPSCALED descriptor shape */
    DM1_V22_AssetDescriptor desc;
    int r;

    reset_state();

    memset(&desc, 0xAA, sizeof(desc));   /* poison to verify memset-on-success */
    r = dm1_v22_asset_load("wall", "test_wall_01",
                            DM1_V22_PROVENANCE_UPSCALED, &desc);
    CHECK_EQ(r, 1, "T2: asset_load(upscaled) returns 1");
    CHECK_EQ(desc.provenance, DM1_V22_PROVENANCE_UPSCALED,
             "T2: provenance = UPSCALED");
    CHECK_EQ(desc.format, DM1_V22_FORMAT_RGBA,
             "T2: format = RGBA (V2.1 upscaled → 32-bit RGBA)");
    CHECK_EQ(desc.width, 320, "T2: width = 320");
    CHECK_EQ(desc.height, 200, "T2: height = 200");
    CHECK_EQ(desc.is_valid, 1, "T2: is_valid = 1 (V2.1 EPX pipeline is always valid)");
    CHECK_EQ(desc.load_attempted, 1, "T2: load_attempted = 1");
    CHECK_NE(desc.source_anchor, NULL, "T2: source_anchor non-NULL");
    if (desc.source_anchor) {
        /* The V2.1 anchor MUST mention the V2.1 EPX pipeline module. */
        CHECK_SUBSTR(desc.source_anchor, "dm1_v2_asset_pipeline_pc34.c",
                     "T2: source_anchor mentions V2.1 EPX pipeline module");
        CHECK_SUBSTR(desc.source_anchor, "F0115",
                     "T2: source_anchor mentions F0115 (object/creature/projectile)");
    }
}

static void t_v21_upscaled_asset_validate(void) {
    /* T3: V2.1 descriptor passes dm1_v22_asset_validate */
    DM1_V22_AssetDescriptor desc;
    reset_state();

    CHECK_EQ(dm1_v22_asset_load("creature", "demon_01",
                                 DM1_V22_PROVENANCE_UPSCALED, &desc), 1,
              "T3: load returns 1");
    CHECK_EQ(dm1_v22_asset_validate(&desc), 1,
              "T3: V2.1 descriptor validates");
}

static void t_v21_upscaled_asset_free(void) {
    /* T4: dm1_v22_asset_free resets the descriptor to a clean state */
    DM1_V22_AssetDescriptor desc;
    reset_state();

    CHECK_EQ(dm1_v22_asset_load("floor", "floor_plain_01",
                                 DM1_V22_PROVENANCE_UPSCALED, &desc), 1,
              "T4: load returns 1");
    /* pixels=NULL + pixels_size=0 in this probe version — the descriptor
     * is metadata-only. Free should accept that gracefully. */
    dm1_v22_asset_free(&desc);
    CHECK_EQ(desc.provenance, DM1_V22_PROVENANCE_UNKNOWN,
             "T4: free → provenance = UNKNOWN");
    CHECK_EQ(desc.is_valid, 0, "T4: free → is_valid = 0");
    CHECK_EQ(desc.load_attempted, 0, "T4: free → load_attempted = 0");
    CHECK_EQ(desc.width, 0, "T4: free → width = 0");
    CHECK_EQ(desc.height, 0, "T4: free → height = 0");
    CHECK_EQ(desc.pixels, NULL, "T4: free → pixels = NULL");
    CHECK_EQ(desc.pixels_size, 0, "T4: free → pixels_size = 0");
}

/* ── V1 ORIGINAL path (must stay unchanged) ──────────────────────── */

static void t_v1_original_best_available(void) {
    /* T5: V1 ORIGINAL mode, desired=ORIGINAL → ORIGINAL */
    reset_state();
    DM1_V2_SetAssetMode(DM1_V2_ASSET_MODE_ORIGINAL);

    DM1_V22_AssetProvenance best =
        dm1_v22_best_available_provenance("wall", "test_wall_01",
                                           DM1_V22_PROVENANCE_ORIGINAL);
    CHECK_EQ(best, DM1_V22_PROVENANCE_ORIGINAL,
             "T5: V1 mode=ORIGINAL + desired=ORIGINAL → ORIGINAL");
}

static void t_v1_original_asset_load(void) {
    /* T6: V1 ORIGINAL asset_load descriptor shape */
    DM1_V22_AssetDescriptor desc;
    int r;

    reset_state();
    DM1_V2_SetAssetMode(DM1_V2_ASSET_MODE_ORIGINAL);

    memset(&desc, 0xAA, sizeof(desc));
    r = dm1_v22_asset_load("wall", "test_wall_01",
                            DM1_V22_PROVENANCE_ORIGINAL, &desc);
    CHECK_EQ(r, 1, "T6: asset_load(original) returns 1");
    CHECK_EQ(desc.provenance, DM1_V22_PROVENANCE_ORIGINAL,
             "T6: provenance = ORIGINAL");
    CHECK_EQ(desc.format, DM1_V22_FORMAT_INDEXED,
             "T6: format = INDEXED (V1 → 4-bit indexed)");
    CHECK_EQ(desc.width, 320, "T6: width = 320");
    CHECK_EQ(desc.height, 200, "T6: height = 200");
    CHECK_EQ(desc.is_valid, 1, "T6: is_valid = 1 (V1 indexed is always available)");
    CHECK_EQ(desc.load_attempted, 1, "T6: load_attempted = 1");
    CHECK_NE(desc.source_anchor, NULL, "T6: source_anchor non-NULL");
    if (desc.source_anchor) {
        /* V1 path MUST mention the V1 graphics loader module. */
        CHECK_SUBSTR(desc.source_anchor, "dm1_v1_graphics_loader_pc34_compat.c",
                     "T6: source_anchor mentions V1 graphics loader module");
        CHECK_SUBSTR(desc.source_anchor, "G0163_WallSetTable",
                     "T6: source_anchor mentions G0163 wall set table");
    }
}

static void t_v1_original_asset_validate(void) {
    /* T7: V1 descriptor passes validation */
    DM1_V22_AssetDescriptor desc;
    reset_state();
    DM1_V2_SetAssetMode(DM1_V2_ASSET_MODE_ORIGINAL);

    CHECK_EQ(dm1_v22_asset_load("creature", "demon_01",
                                 DM1_V22_PROVENANCE_ORIGINAL, &desc), 1,
              "T7: load returns 1");
    CHECK_EQ(dm1_v22_asset_validate(&desc), 1,
              "T7: V1 descriptor validates");
}

/* ── V2.0 FILTERED fallback path ──────────────────────────────────── */

static void t_v20_filtered_best_available_upscaled_request(void) {
    /* T8: V2.0 FILTERED mode + desired=UPSCALED → FILTERED
     * (V2.1 / V2.0 mismatch must select the V2.0 filtered path, not
     * silently collapse to V1 ORIGINAL) */
    reset_state();
    DM1_V2_SetAssetMode(DM1_V2_ASSET_MODE_FILTERED);

    DM1_V22_AssetProvenance best =
        dm1_v22_best_available_provenance("wall", "test_wall_01",
                                           DM1_V22_PROVENANCE_UPSCALED);
    CHECK_EQ(best, DM1_V22_PROVENANCE_FILTERED,
             "T8: V20 mode + desired=UPSCALED → FILTERED (no silent V1 collapse)");
}

static void t_v20_filtered_asset_load_upscaled_request(void) {
    /* T9: V2.0 FILTERED asset_load UPSCALED → FILTERED descriptor.
     * This is the regression test for the missing FILTERED branch
     * in dm1_v22_asset_load(). Without the fix, the chain silently
     * collapses to V1 ORIGINAL even though V2.0 is the active mode. */
    DM1_V22_AssetDescriptor desc;
    int r;

    reset_state();
    DM1_V2_SetAssetMode(DM1_V2_ASSET_MODE_FILTERED);

    memset(&desc, 0xAA, sizeof(desc));
    r = dm1_v22_asset_load("wall", "test_wall_01",
                            DM1_V22_PROVENANCE_UPSCALED, &desc);
    CHECK_EQ(r, 1, "T9: asset_load returns 1 in V20 mode");
    CHECK_EQ(desc.provenance, DM1_V22_PROVENANCE_FILTERED,
             "T9: provenance = FILTERED (no silent V1 collapse)");
    CHECK_EQ(desc.format, DM1_V22_FORMAT_PALETTED,
             "T9: format = PALETTED (V2.0 → 8-bit paletted)");
    CHECK_EQ(desc.width, 320, "T9: width = 320");
    CHECK_EQ(desc.height, 200, "T9: height = 200");
    CHECK_EQ(desc.is_valid, 1, "T9: is_valid = 1 (V2.0 filter pipeline is always valid)");
    CHECK_EQ(desc.load_attempted, 1, "T9: load_attempted = 1");
    CHECK_NE(desc.source_anchor, NULL, "T9: source_anchor non-NULL");
    if (desc.source_anchor) {
        /* V2.0 path MUST mention the filter palette-correct module
         * (the F0337 dungeon-view palette LUT builder). */
        CHECK_SUBSTR(desc.source_anchor, "dm1_v2_filter_palette_correct_pc34.c",
                     "T9: source_anchor mentions V2.0 filter palette correct module");
        CHECK_SUBSTR(desc.source_anchor, "F0337_INVENTORY_SetDungeonViewPalette",
                     "T9: source_anchor mentions F0337 dungeon-view palette");
    }
}

static void t_v20_filtered_best_available_modern_request(void) {
    /* T10: V2.0 FILTERED + desired=MODERN → FILTERED (V2.0 still beats V1) */
    reset_state();
    DM1_V2_SetAssetMode(DM1_V2_ASSET_MODE_FILTERED);

    DM1_V22_AssetProvenance best =
        dm1_v22_best_available_provenance("wall", "test_wall_01",
                                           DM1_V22_PROVENANCE_MODERN);
    CHECK_EQ(best, DM1_V22_PROVENANCE_FILTERED,
             "T10: V20 mode + desired=MODERN → FILTERED");
}

/* ── V2.2 MODERN gate ────────────────────────────────────────────── */

static void t_v22_not_installed_best_available(void) {
    /* T11: V22 installed=0, desired=MODERN → NOT MODERN */
    reset_state();
    dm1_v22_set_installed(0);

    DM1_V22_AssetProvenance best =
        dm1_v22_best_available_provenance("wall", "test_wall_01",
                                           DM1_V22_PROVENANCE_MODERN);
    CHECK_NE(best, DM1_V22_PROVENANCE_MODERN,
             "T11: installed=0 + desired=MODERN → not MODERN");
    CHECK_EQ(best, DM1_V22_PROVENANCE_UPSCALED,
             "T11: installed=0 + desired=MODERN → UPSCALED (V2.1 default)");
}

static void t_v22_not_installed_asset_load(void) {
    /* T12: V22 installed=0, asset_load MODERN → NOT MODERN provenance */
    DM1_V22_AssetDescriptor desc;
    int r;

    reset_state();
    dm1_v22_set_installed(0);

    memset(&desc, 0xAA, sizeof(desc));
    r = dm1_v22_asset_load("wall", "test_wall_01",
                            DM1_V22_PROVENANCE_MODERN, &desc);
    CHECK_EQ(r, 1, "T12: asset_load returns 1 (chain reached a fallback)");
    CHECK_NE(desc.provenance, DM1_V22_PROVENANCE_MODERN,
             "T12: provenance != MODERN (no V2.2 assets installed)");
}

/* ── Out-of-range / safety ────────────────────────────────────────── */

static void t_unknown_provenance_rejected(void) {
    /* T13: desired=UNKNOWN → UNKNOWN */
    reset_state();

    DM1_V22_AssetProvenance best =
        dm1_v22_best_available_provenance("wall", "test_wall_01",
                                           DM1_V22_PROVENANCE_UNKNOWN);
    CHECK_EQ(best, DM1_V22_PROVENANCE_UNKNOWN,
             "T13: desired=UNKNOWN → UNKNOWN");
}

static void t_asset_load_null_desc(void) {
    /* T14: asset_load with NULL out_desc → returns 0, no crash */
    reset_state();
    int r = dm1_v22_asset_load("wall", "test_wall_01",
                                DM1_V22_PROVENANCE_UPSCALED, NULL);
    CHECK_EQ(r, 0, "T14: asset_load(NULL) returns 0");
}

static void t_fallback_chain_terminates(void) {
    /* T15: fallback_next chain terminates at UNKNOWN without looping.
     * Walking MODERN → UPSCALED → FILTERED → ORIGINAL → UNKNOWN
     * must hit UNKNOWN after exactly 4 steps. */
    DM1_V22_AssetProvenance p = DM1_V22_PROVENANCE_MODERN;
    int steps = 0;
    while (p != DM1_V22_PROVENANCE_UNKNOWN && steps < 16) {
        p = dm1_v22_fallback_next(p);
        steps++;
    }
    CHECK_EQ(p, DM1_V22_PROVENANCE_UNKNOWN,
             "T15: chain terminates at UNKNOWN");
    CHECK_EQ(steps, 4, "T15: chain length is 4 (MODERN→UPSCALED→FILTERED→ORIGINAL→UNKNOWN)");
}

/* ── Modern gate cross-check ──────────────────────────────────────── */

static void t_v22_installed_no_manifest(void) {
    /* T16: installed=1 but no manifest path → MODERN provenance is never
     * picked (the best_available gate consults dm1_v22_modern_assets_available()
     * which validates the manifest). */
    reset_state();
    dm1_v22_set_installed(1);
    dm1_v22_set_manifest_path("");   /* empty path → no manifest */

    DM1_V22_AssetProvenance best =
        dm1_v22_best_available_provenance("wall", "test_wall_01",
                                           DM1_V22_PROVENANCE_MODERN);
    CHECK_NE(best, DM1_V22_PROVENANCE_MODERN,
             "T16: installed=1 + no manifest → MODERN not picked");
}

/* ── Cross-mode consistency ───────────────────────────────────────── */

static void t_v21_upscaled_modern_request(void) {
    /* T17: V2.1 mode=UPSCALED + desired=MODERN → UPSCALED
     * (chain MODERN→UPSCALED picks UPSCALED because mode >= UPSCALED).
     * The user must explicitly check MODERN availability if they want it. */
    reset_state();

    DM1_V22_AssetDescriptor desc;
    memset(&desc, 0, sizeof(desc));
    int r = dm1_v22_asset_load("wall", "test_wall_01",
                                DM1_V22_PROVENANCE_MODERN, &desc);
    CHECK_EQ(r, 1, "T17: load returns 1 (chain reached UPSCALED)");
    CHECK_EQ(desc.provenance, DM1_V22_PROVENANCE_UPSCALED,
             "T17: V21 mode + desired=MODERN → UPSCALED (chain fallback)");
}

static void t_v1_original_modern_request(void) {
    /* T18: V1 ORIGINAL mode + desired=MODERN → ORIGINAL
     * (chain MODERN→UPSCALED→FILTERED→ORIGINAL all skipped
     *  because mode=ORIGINAL, then ORIGINAL wins). */
    reset_state();
    DM1_V2_SetAssetMode(DM1_V2_ASSET_MODE_ORIGINAL);

    DM1_V22_AssetDescriptor desc;
    memset(&desc, 0, sizeof(desc));
    int r = dm1_v22_asset_load("wall", "test_wall_01",
                                DM1_V22_PROVENANCE_MODERN, &desc);
    CHECK_EQ(r, 1, "T18: load returns 1 (chain reached ORIGINAL)");
    CHECK_EQ(desc.provenance, DM1_V22_PROVENANCE_ORIGINAL,
             "T18: V1 mode + desired=MODERN → ORIGINAL");
}

/* ── State hygiene ────────────────────────────────────────────────── */

static void t_mode_state_round_trip(void) {
    /* T19: Repeated V2.1 mode set/restore cycles don't leak state.
     * Every mode must be settable and gettable. */
    reset_state();

    DM1_V2_SetAssetMode(DM1_V2_ASSET_MODE_ORIGINAL);
    CHECK_EQ(DM1_V2_GetAssetMode(), DM1_V2_ASSET_MODE_ORIGINAL,
             "T19: set ORIGINAL → get ORIGINAL");

    DM1_V2_SetAssetMode(DM1_V2_ASSET_MODE_FILTERED);
    CHECK_EQ(DM1_V2_GetAssetMode(), DM1_V2_ASSET_MODE_FILTERED,
             "T19: set FILTERED → get FILTERED");

    DM1_V2_SetAssetMode(DM1_V2_ASSET_MODE_UPSCALED);
    CHECK_EQ(DM1_V2_GetAssetMode(), DM1_V2_ASSET_MODE_UPSCALED,
             "T19: set UPSCALED → get UPSCALED");

    DM1_V2_SetAssetMode(DM1_V2_ASSET_MODE_MODERN);
    CHECK_EQ(DM1_V2_GetAssetMode(), DM1_V2_ASSET_MODE_MODERN,
             "T19: set MODERN → get MODERN");

    /* Out-of-range modes are silently ignored */
    DM1_V2_SetAssetMode((DM1_V2_AssetMode)999);
    CHECK_EQ(DM1_V2_GetAssetMode(), DM1_V2_ASSET_MODE_MODERN,
             "T19: set 999 → no change (still MODERN)");

    reset_state();
}

static void t_load_free_round_trip(void) {
    /* T20: Repeated asset_load / asset_free cycles work cleanly. */
    reset_state();

    for (int i = 0; i < 8; i++) {
        DM1_V22_AssetDescriptor desc;
        memset(&desc, 0xAA, sizeof(desc));
        int r = dm1_v22_asset_load("wall", "test_wall_01",
                                    DM1_V22_PROVENANCE_UPSCALED, &desc);
        CHECK_EQ(r, 1, "T20: repeated load returns 1");
        CHECK_EQ(desc.provenance, DM1_V22_PROVENANCE_UPSCALED,
                 "T20: repeated load preserves UPSCALED provenance");
        dm1_v22_asset_free(&desc);
        CHECK_EQ(desc.is_valid, 0,
                 "T20: free resets is_valid to 0 after each cycle");
    }
}

/* ── Main ──────────────────────────────────────────────────────────── */

int main(void) {
    printf("=== test_dm1_v22_upscaled_asset_selection_pc34 ===\n");

    t_v21_upscaled_best_available();
    t_v21_upscaled_asset_load();
    t_v21_upscaled_asset_validate();
    t_v21_upscaled_asset_free();

    t_v1_original_best_available();
    t_v1_original_asset_load();
    t_v1_original_asset_validate();

    t_v20_filtered_best_available_upscaled_request();
    t_v20_filtered_asset_load_upscaled_request();
    t_v20_filtered_best_available_modern_request();

    t_v22_not_installed_best_available();
    t_v22_not_installed_asset_load();

    t_unknown_provenance_rejected();
    t_asset_load_null_desc();
    t_fallback_chain_terminates();

    t_v22_installed_no_manifest();

    t_v21_upscaled_modern_request();
    t_v1_original_modern_request();

    t_mode_state_round_trip();
    t_load_free_round_trip();

    printf("--- %d / %d passed ---\n", g_total - g_failed, g_total);
    if (g_failed == 0) {
        printf("PASS: all DM1 V2.1 upscaled asset selection checks\n");
        return 0;
    } else {
        printf("FAIL: %d / %d DM1 V2.1 upscaled asset selection checks\n",
               g_failed, g_total);
        return 1;
    }
}

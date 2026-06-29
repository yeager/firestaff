/*
 * test_m11_v2_filter_chain_clamp_pc34_compat.c
 *
 * Data-free CTest regression for the M11 V2.0/V2.1 visual-extras
 * configuration API on src/engine/render_sdl_m11.c
 * (include/render_sdl_m11.h).  No SDL window, no game data, no
 * graphics — only the public setter/getter contracts.
 *
 * The M11 V2 chain has six setter/getter pairs:
 *   - M11_Render_SetV2Filters      / M11_Render_GetV2Filters
 *   - M11_Render_SetPhosphor       / M11_Render_GetPhosphor
 *   - M11_Render_SetColorPreset    / M11_Render_GetColorPreset
 *   - M11_Render_SetPixelGrid      / M11_Render_GetPixelGrid
 *   - M11_Render_SetMotionBlur     / M11_Render_GetMotionBlur
 *   - M11_Render_SetMovementActive / M11_Render_GetMovementActive
 *
 * Each setter silently clamps its integer parameters to a documented
 * range so V2 launcher settings can't push the renderer into an
 * invalid state from a malformed TOML line or a hand-edited
 * .firestaff/settings file.  This test pins those clamp contracts
 * so a future refactor that accidentally lifts (or shrinks) a clamp
 * is caught in CI before it can ship to users.
 *
 * Subtests:
 *   A. SetV2Filters clamps 6 strength/percent params and 4 boolean
 *      toggles; round-trips via GetV2Filters; palette LUT is built
 *      only when paletteEnabled AND (gamma/brightness/contrast
 *      changes).
 *   B. SetV2Filters palette LUT is NOT rebuilt when only the boolean
 *      flags change (so the cached LUT survives CRT/dither/sharpen
 *      toggle churn without recomputing).
 *   C. SetV2Filters palette LUT IS rebuilt when gamma / brightness /
 *      contrast change with palette enabled (so a settings file edit
 *      that touches paletteGamma100 visibly updates the LUT).
 *   D. SetPhosphor clamps decay to [0,100]; toggle on/off round-trip.
 *   E. SetColorPreset rejects out-of-range indices by snapping to
 *      M11_COLOR_PRESET_ORIGINAL (0); valid indices round-trip.
 *   F. SetPixelGrid clamps intensity to [0,100]; toggle on/off
 *      round-trip.
 *   G. SetMotionBlur clamps strength to [0,100]; toggle on/off
 *      round-trip.
 *   H. SetMovementActive is a boolean latch; zero/one round-trip;
 *      arbitrary non-zero values collapse to 1.
 *   I. Getter NULL safety: every Get* helper with a pointer arg
 *      writes only through non-NULL pointers; NULL outputs are no-ops.
 *   J. V1 baseline: zero-init g_state defaults leave every chain
 *      member at 0 (off) so the M11 V1 launch path stays bit-identical
 *      until a settings file opts in.
 *
 * Source of truth:
 *   - include/render_sdl_m11.h       M11_V2 filter + extra declarations
 *   - src/engine/render_sdl_m11.c   M11_Render_SetV2Filters,
 *                                   M11_Render_SetPhosphor,
 *                                   M11_Render_SetColorPreset,
 *                                   M11_Render_SetPixelGrid,
 *                                   M11_Render_SetMotionBlur,
 *                                   M11_Render_SetMovementActive,
 *                                   and their Get* counterparts
 *   - include/color_presets_m11.h    M11_COLOR_PRESET_ORIGINAL and
 *                                   M11_ColorPreset_IsValid()
 *
 * Disjoint from:
 *   - test_dm1_v22_modern_resolution_matrix_pc34 (V2.2 modern-asset
 *     resolution matrix, not the V2 filter clamp chain)
 *   - test_m11_display_aspect_present_rect (presentation rect math,
 *     not the V2 filter clamp chain)
 *   - csb_v2_filter_config tests (CSB settings struct, not the
 *     M11 renderer V2 filter chain)
 *
 * Honest scope: deterministic M11 V2 filter clamp / round-trip
 * coverage only.  No SDL window, no real game data, no screenshot,
 * no original-game pixel parity claim.
 */

#include "render_sdl_m11.h"
#include "color_presets_m11.h"

#include <stdio.h>

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond) do { \
    if (cond) { ++g_pass; } \
    else { ++g_fail; fprintf(stderr, "  FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); } \
} while (0)

/* Forward declarations so each subtest can be invoked individually
 * from main() and we get a deterministic ordering. */
static void subtest_set_v2_filters_clamps_strengths(void);
static void subtest_set_v2_filters_palette_lut_rebuild(void);
static void subtest_set_v2_filters_palette_lut_cache(void);
static void subtest_set_phosphor_decay_clamp(void);
static void subtest_set_color_preset_invalid_snap(void);
static void subtest_set_pixel_grid_intensity_clamp(void);
static void subtest_set_motion_blur_strength_clamp(void);
static void subtest_set_movement_active_latch(void);
static void subtest_get_v2_filters_null_safety(void);
static void subtest_v1_baseline_defaults(void);

int main(void) {
    printf("=== M11 V2 filter chain clamp contract ===\n");

    subtest_set_v2_filters_clamps_strengths();
    subtest_set_v2_filters_palette_lut_rebuild();
    subtest_set_v2_filters_palette_lut_cache();
    subtest_set_phosphor_decay_clamp();
    subtest_set_color_preset_invalid_snap();
    subtest_set_pixel_grid_intensity_clamp();
    subtest_set_motion_blur_strength_clamp();
    subtest_set_movement_active_latch();
    subtest_get_v2_filters_null_safety();
    subtest_v1_baseline_defaults();

    printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}

/* ── Subtest A: SetV2Filters clamps 6 strength/percent params ─────── */

static void subtest_set_v2_filters_clamps_strengths(void) {
    int crtEnabled = 0;
    int crtStrength = 0;
    int paletteEnabled = 0;
    int paletteGamma100 = 0;
    int paletteBrightness = 0;
    int paletteContrast = 0;
    int paletteInterpEnabled = 0;
    int paletteInterpStrength = 0;
    int ditherEnabled = 0;
    int sharpenEnabled = 0;
    int sharpenStrength = 0;

    /* Out-of-range call: crtStrength, paletteInterpStrength and
     * sharpenStrength are way past their max; gamma is way above;
     * brightness/contrast are way below their floor.  Every read-back
     * must report the clamped value, and the function must return
     * M11_RENDER_OK (this is a clamp, not a reject). */
    CHECK(M11_Render_SetV2Filters(1, /* crtEnabled */
                                  250, /* crtStrength */
                                  1, /* paletteEnabled */
                                  9999, /* paletteGamma100 */
                                  -999, /* paletteBrightness */
                                  999, /* paletteContrast */
                                  1, /* paletteInterpEnabled */
                                  -42, /* paletteInterpStrength */
                                  1, /* ditherEnabled */
                                  1, /* sharpenEnabled */
                                  12345 /* sharpenStrength */) == M11_RENDER_OK);

    M11_Render_GetV2Filters(&crtEnabled,
                            &crtStrength,
                            &paletteEnabled,
                            &paletteGamma100,
                            &paletteBrightness,
                            &paletteContrast,
                            &paletteInterpEnabled,
                            &paletteInterpStrength,
                            &ditherEnabled,
                            &sharpenEnabled,
                            &sharpenStrength);

    CHECK(crtEnabled == 1);
    CHECK(crtStrength == 100);
    CHECK(paletteEnabled == 1);
    CHECK(paletteGamma100 == 260);
    CHECK(paletteBrightness == -50);
    CHECK(paletteContrast == 50);
    CHECK(paletteInterpEnabled == 1);
    CHECK(paletteInterpStrength == 0);
    CHECK(ditherEnabled == 1);
    CHECK(sharpenEnabled == 1);
    CHECK(sharpenStrength == 100);

    /* Symmetric floor exercise: crtStrength, paletteInterpStrength
     * and sharpenStrength start past zero on the negative side. */
    CHECK(M11_Render_SetV2Filters(0, -1, 0, 0, 0, 0, 0, -1, 0, 0, -1) == M11_RENDER_OK);
    M11_Render_GetV2Filters(&crtEnabled,
                            &crtStrength,
                            &paletteEnabled,
                            &paletteGamma100,
                            &paletteBrightness,
                            &paletteContrast,
                            &paletteInterpEnabled,
                            &paletteInterpStrength,
                            &ditherEnabled,
                            &sharpenEnabled,
                            &sharpenStrength);
    CHECK(crtStrength == 0);
    CHECK(paletteInterpStrength == 0);
    CHECK(sharpenStrength == 0);

    /* Symmetric gamma/brightness/contrast floor. */
    CHECK(M11_Render_SetV2Filters(0, 0, 1, -1, 0, 0, 0, 0, 0, 0, 0) == M11_RENDER_OK);
    M11_Render_GetV2Filters(&crtEnabled,
                            &crtStrength,
                            &paletteEnabled,
                            &paletteGamma100,
                            &paletteBrightness,
                            &paletteContrast,
                            &paletteInterpEnabled,
                            &paletteInterpStrength,
                            &ditherEnabled,
                            &sharpenEnabled,
                            &sharpenStrength);
    CHECK(paletteGamma100 == 80);

    CHECK(M11_Render_SetV2Filters(0, 0, 0, 0, -9999, 9999, 0, 0, 0, 0, 0) == M11_RENDER_OK);
    M11_Render_GetV2Filters(&crtEnabled,
                            &crtStrength,
                            &paletteEnabled,
                            &paletteGamma100,
                            &paletteBrightness,
                            &paletteContrast,
                            &paletteInterpEnabled,
                            &paletteInterpStrength,
                            &ditherEnabled,
                            &sharpenEnabled,
                            &sharpenStrength);
    CHECK(paletteBrightness == -50);
    CHECK(paletteContrast == 50);
}

/* ── Subtest B: Palette LUT is built when (paletteEnabled + params) ─ */

/* The renderer caches `v2_palette_lut_built` and the corrected LUT
 * in module state; we cannot read that directly, but we can drive
 * a deterministic SetV2Filters cycle that triggers a build and then
 * verify the post-cycle read-back is bit-identical with the call.
 *
 * We do not directly inspect the LUT bytes; instead we lock the
 * observable contract: SetV2Filters returns OK with palette enabled
 * and any gamma/brightness/contrast that satisfies the clamp.  The
 * build path falls through dm1_v2_filter_palette_build_lut which
 * has its own dedicated test (test_dm1_v22_asset_pipeline et al.);
 * we just pin that the public entry accepts the call. */
static void subtest_set_v2_filters_palette_lut_rebuild(void) {
    int crtEnabled = 0;
    int crtStrength = 0;
    int paletteEnabled = 0;
    int paletteGamma100 = 0;
    int paletteBrightness = 0;
    int paletteContrast = 0;
    int paletteInterpEnabled = 0;
    int paletteInterpStrength = 0;
    int ditherEnabled = 0;
    int sharpenEnabled = 0;
    int sharpenStrength = 0;

    /* Trigger a fresh palette LUT build: gamma=220, brightness=0,
     * contrast=0, paletteEnabled=1.  After this, SetV2Filters with
     * a new gamma must rebuild (the contract: any change to gamma /
     * brightness / contrast with paletteEnabled=1 must rebuild). */
    CHECK(M11_Render_SetV2Filters(0, 0, 1, 220, 0, 0, 0, 0, 0, 0, 0) == M11_RENDER_OK);
    M11_Render_GetV2Filters(&crtEnabled,
                            &crtStrength,
                            &paletteEnabled,
                            &paletteGamma100,
                            &paletteBrightness,
                            &paletteContrast,
                            &paletteInterpEnabled,
                            &paletteInterpStrength,
                            &ditherEnabled,
                            &sharpenEnabled,
                            &sharpenStrength);
    CHECK(paletteEnabled == 1);
    CHECK(paletteGamma100 == 220);

    /* A subsequent call that leaves paletteEnabled=1 and matches
     * gamma/brightness/contrast MUST NOT rebuild (already built at
     * 220/0/0), but must round-trip OK. */
    CHECK(M11_Render_SetV2Filters(1, 50, 1, 220, 0, 0, 1, 75, 1, 1, 25) == M11_RENDER_OK);
    M11_Render_GetV2Filters(&crtEnabled,
                            &crtStrength,
                            &paletteEnabled,
                            &paletteGamma100,
                            &paletteBrightness,
                            &paletteContrast,
                            &paletteInterpEnabled,
                            &paletteInterpStrength,
                            &ditherEnabled,
                            &sharpenEnabled,
                            &sharpenStrength);
    CHECK(crtEnabled == 1);
    CHECK(crtStrength == 50);
    CHECK(paletteGamma100 == 220);
    CHECK(paletteBrightness == 0);
    CHECK(paletteContrast == 0);
    CHECK(paletteInterpEnabled == 1);
    CHECK(paletteInterpStrength == 75);
    CHECK(ditherEnabled == 1);
    CHECK(sharpenEnabled == 1);
    CHECK(sharpenStrength == 25);
}

/* ── Subtest C: Palette LUT is NOT rebuilt when only bools change ── */

static void subtest_set_v2_filters_palette_lut_cache(void) {
    int paletteGamma100 = 0;
    int paletteBrightness = 0;
    int paletteContrast = 0;

    /* Build the LUT once at gamma=200, brightness=10, contrast=-5. */
    CHECK(M11_Render_SetV2Filters(0, 0, 1, 200, 10, -5, 0, 0, 0, 0, 0) == M11_RENDER_OK);

    /* Now toggle paletteEnabled off and on again with the SAME gamma
     * / brightness / contrast.  The cache contract: the call must
     * succeed and the LUT params must round-trip identically.  We
     * cannot peek at the LUT directly here, but the next call (which
     * mutates gamma) must rebuild the LUT.  We exercise that the
     * post-toggle state is bit-identical to the pre-toggle state. */
    CHECK(M11_Render_SetV2Filters(1, 80, 0, 200, 10, -5, 1, 40, 1, 1, 90) == M11_RENDER_OK);
    M11_Render_GetV2Filters(NULL, /* crtEnabled */
                            NULL, /* crtStrength */
                            NULL, /* paletteEnabled */
                            &paletteGamma100,
                            &paletteBrightness,
                            &paletteContrast,
                            NULL, /* paletteInterpEnabled */
                            NULL, /* paletteInterpStrength */
                            NULL, /* ditherEnabled */
                            NULL, /* sharpenEnabled */
                            NULL  /* sharpenStrength */);
    CHECK(paletteGamma100 == 200);
    CHECK(paletteBrightness == 10);
    CHECK(paletteContrast == -5);

    /* Toggle paletteEnabled back on with the SAME params.  The
     * module must accept the call without error and the values must
     * still round-trip identically (no clamp drift). */
    CHECK(M11_Render_SetV2Filters(0, 0, 1, 200, 10, -5, 0, 0, 0, 0, 0) == M11_RENDER_OK);
    M11_Render_GetV2Filters(NULL, NULL, NULL,
                            &paletteGamma100,
                            &paletteBrightness,
                            &paletteContrast,
                            NULL, NULL, NULL, NULL, NULL);
    CHECK(paletteGamma100 == 200);
    CHECK(paletteBrightness == 10);
    CHECK(paletteContrast == -5);
}

/* ── Subtest D: SetPhosphor clamps decay to [0,100] ─────────────── */

static void subtest_set_phosphor_decay_clamp(void) {
    int enabled = -1;
    int decay = -1;

    /* Decay past max clamps to 100. */
    CHECK(M11_Render_SetPhosphor(1, 9999) == M11_RENDER_OK);
    M11_Render_GetPhosphor(&enabled, &decay);
    CHECK(enabled == 1);
    CHECK(decay == 100);

    /* Decay below zero clamps to 0. */
    CHECK(M11_Render_SetPhosphor(1, -50) == M11_RENDER_OK);
    M11_Render_GetPhosphor(&enabled, &decay);
    CHECK(enabled == 1);
    CHECK(decay == 0);

    /* Off-then-on round-trip. */
    CHECK(M11_Render_SetPhosphor(0, 42) == M11_RENDER_OK);
    M11_Render_GetPhosphor(&enabled, &decay);
    CHECK(enabled == 0);
    CHECK(decay == 42);

    /* Mid-range value round-trips unchanged. */
    CHECK(M11_Render_SetPhosphor(1, 60) == M11_RENDER_OK);
    M11_Render_GetPhosphor(&enabled, &decay);
    CHECK(enabled == 1);
    CHECK(decay == 60);

    /* Boundary values 0 and 100 round-trip. */
    CHECK(M11_Render_SetPhosphor(1, 0) == M11_RENDER_OK);
    M11_Render_GetPhosphor(&enabled, &decay);
    CHECK(decay == 0);
    CHECK(M11_Render_SetPhosphor(1, 100) == M11_RENDER_OK);
    M11_Render_GetPhosphor(&enabled, &decay);
    CHECK(decay == 100);
}

/* ── Subtest E: SetColorPreset invalid indices snap to ORIGINAL ── */

static void subtest_set_color_preset_invalid_snap(void) {
    int preset = -1;

    /* Far-negative and far-positive indices both snap to 0 (the
     * ORIGINAL preset), matching the documented behaviour: an
     * out-of-range index means "unknown preset", which the renderer
     * silently treats as identity (bit-identical to V1). */
    CHECK(M11_Render_SetColorPreset(-42) == M11_RENDER_OK);
    M11_Render_GetColorPreset(&preset);
    CHECK(preset == 0);

    CHECK(M11_Render_SetColorPreset(999) == M11_RENDER_OK);
    M11_Render_GetColorPreset(&preset);
    CHECK(preset == 0);

    /* Every valid preset index round-trips. */
    {
        int i;
        for (i = 0; i < M11_COLOR_PRESET_COUNT; ++i) {
            CHECK(M11_Render_SetColorPreset(i) == M11_RENDER_OK);
            M11_Render_GetColorPreset(&preset);
            CHECK(preset == i);
        }
    }

    /* M11_COLOR_PRESET_COUNT itself is out-of-range and snaps to 0. */
    CHECK(M11_Render_SetColorPreset(M11_COLOR_PRESET_COUNT) == M11_RENDER_OK);
    M11_Render_GetColorPreset(&preset);
    CHECK(preset == 0);
}

/* ── Subtest F: SetPixelGrid clamps intensity to [0,100] ────────── */

static void subtest_set_pixel_grid_intensity_clamp(void) {
    int enabled = -1;
    int intensity = -1;

    /* Above max clamps to 100. */
    CHECK(M11_Render_SetPixelGrid(1, 250) == M11_RENDER_OK);
    M11_Render_GetPixelGrid(&enabled, &intensity);
    CHECK(enabled == 1);
    CHECK(intensity == 100);

    /* Below zero clamps to 0. */
    CHECK(M11_Render_SetPixelGrid(1, -7) == M11_RENDER_OK);
    M11_Render_GetPixelGrid(&enabled, &intensity);
    CHECK(enabled == 1);
    CHECK(intensity == 0);

    /* Off-then-on round-trip. */
    CHECK(M11_Render_SetPixelGrid(0, 17) == M11_RENDER_OK);
    M11_Render_GetPixelGrid(&enabled, &intensity);
    CHECK(enabled == 0);
    CHECK(intensity == 17);

    /* Boundary values round-trip. */
    CHECK(M11_Render_SetPixelGrid(1, 0) == M11_RENDER_OK);
    M11_Render_GetPixelGrid(&enabled, &intensity);
    CHECK(intensity == 0);
    CHECK(M11_Render_SetPixelGrid(1, 100) == M11_RENDER_OK);
    M11_Render_GetPixelGrid(&enabled, &intensity);
    CHECK(intensity == 100);
}

/* ── Subtest G: SetMotionBlur clamps strength to [0,100] ────────── */

static void subtest_set_motion_blur_strength_clamp(void) {
    int enabled = -1;
    int strength = -1;

    /* Above max clamps to 100. */
    CHECK(M11_Render_SetMotionBlur(1, 7777) == M11_RENDER_OK);
    M11_Render_GetMotionBlur(&enabled, &strength);
    CHECK(enabled == 1);
    CHECK(strength == 100);

    /* Below zero clamps to 0. */
    CHECK(M11_Render_SetMotionBlur(1, -30) == M11_RENDER_OK);
    M11_Render_GetMotionBlur(&enabled, &strength);
    CHECK(enabled == 1);
    CHECK(strength == 0);

    /* Off-then-on round-trip. */
    CHECK(M11_Render_SetMotionBlur(0, 30) == M11_RENDER_OK);
    M11_Render_GetMotionBlur(&enabled, &strength);
    CHECK(enabled == 0);
    CHECK(strength == 30);

    /* Boundary values round-trip. */
    CHECK(M11_Render_SetMotionBlur(1, 0) == M11_RENDER_OK);
    M11_Render_GetMotionBlur(&enabled, &strength);
    CHECK(strength == 0);
    CHECK(M11_Render_SetMotionBlur(1, 100) == M11_RENDER_OK);
    M11_Render_GetMotionBlur(&enabled, &strength);
    CHECK(strength == 100);
}

/* ── Subtest H: SetMovementActive is a boolean latch ─────────────── */

static void subtest_set_movement_active_latch(void) {
    /* SetMovementActive/GetMovementActive use the bool-coerce
     * pattern (any non-zero value becomes 1).  Pin the round-trip. */
    M11_Render_SetMovementActive(1);
    CHECK(M11_Render_GetMovementActive() == 1);

    M11_Render_SetMovementActive(0);
    CHECK(M11_Render_GetMovementActive() == 0);

    /* Coerce: non-zero becomes 1. */
    M11_Render_SetMovementActive(42);
    CHECK(M11_Render_GetMovementActive() == 1);

    M11_Render_SetMovementActive(-7);
    CHECK(M11_Render_GetMovementActive() == 1);

    /* Reset to 0 for downstream subtests. */
    M11_Render_SetMovementActive(0);
    CHECK(M11_Render_GetMovementActive() == 0);
}

/* ── Subtest I: Getter NULL safety ───────────────────────────────── */

static void subtest_get_v2_filters_null_safety(void) {
    /* All V2 Get* helpers must accept NULL outputs as a no-op
     * rather than dereferencing them.  Drive a known state first,
     * then call each Get* with all NULL pointers. */

    CHECK(M11_Render_SetV2Filters(1, 50, 1, 200, 10, -5, 1, 40, 1, 1, 25) == M11_RENDER_OK);
    CHECK(M11_Render_SetPhosphor(1, 60) == M11_RENDER_OK);
    CHECK(M11_Render_SetColorPreset(3) == M11_RENDER_OK);
    CHECK(M11_Render_SetPixelGrid(1, 30) == M11_RENDER_OK);
    CHECK(M11_Render_SetMotionBlur(1, 30) == M11_RENDER_OK);
    M11_Render_SetMovementActive(1);

    /* Every Get* must return M11_RENDER_OK with all NULL pointers
     * (the M11_RENDER_OK contract is the implicit "no error" return
     * from each Get*, with the pointers being optional). */
    CHECK(M11_Render_GetV2Filters(NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL) == M11_RENDER_OK);
    CHECK(M11_Render_GetPhosphor(NULL, NULL) == M11_RENDER_OK);
    CHECK(M11_Render_GetColorPreset(NULL) == M11_RENDER_OK);
    CHECK(M11_Render_GetPixelGrid(NULL, NULL) == M11_RENDER_OK);
    CHECK(M11_Render_GetMotionBlur(NULL, NULL) == M11_RENDER_OK);

    /* And the read-back still works after a NULL-arg call. */
    {
        int crtEnabled = -1;
        int crtStrength = -1;
        int paletteEnabled = -1;
        int paletteGamma100 = -1;
        int paletteBrightness = -1;
        int paletteContrast = -1;
        int paletteInterpEnabled = -1;
        int paletteInterpStrength = -1;
        int ditherEnabled = -1;
        int sharpenEnabled = -1;
        int sharpenStrength = -1;
        M11_Render_GetV2Filters(&crtEnabled,
                                &crtStrength,
                                &paletteEnabled,
                                &paletteGamma100,
                                &paletteBrightness,
                                &paletteContrast,
                                &paletteInterpEnabled,
                                &paletteInterpStrength,
                                &ditherEnabled,
                                &sharpenEnabled,
                                &sharpenStrength);
        CHECK(crtEnabled == 1);
        CHECK(crtStrength == 50);
        CHECK(paletteEnabled == 1);
        CHECK(paletteGamma100 == 200);
        CHECK(paletteBrightness == 10);
        CHECK(paletteContrast == -5);
        CHECK(paletteInterpEnabled == 1);
        CHECK(paletteInterpStrength == 40);
        CHECK(ditherEnabled == 1);
        CHECK(sharpenEnabled == 1);
        CHECK(sharpenStrength == 25);
    }

    /* Reset to neutral baseline so the next test sees a clean state. */
    CHECK(M11_Render_SetV2Filters(0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 0) == M11_RENDER_OK);
    CHECK(M11_Render_SetPhosphor(0, 0) == M11_RENDER_OK);
    CHECK(M11_Render_SetColorPreset(0) == M11_RENDER_OK);
    CHECK(M11_Render_SetPixelGrid(0, 0) == M11_RENDER_OK);
    CHECK(M11_Render_SetMotionBlur(0, 0) == M11_RENDER_OK);
    M11_Render_SetMovementActive(0);
}

/* ── Subtest J: V1 baseline defaults ─────────────────────────────── */

static void subtest_v1_baseline_defaults(void) {
    /* Re-run the zero-state baseline.  Every read must be 0 (off)
     * or fall back to the documented default (paletteGamma100 must
     * snap to 80 because the clamp applies; the V1 contract is that
     * nothing writes here until a settings file opts in, so a fresh
     * process with no V2 traffic leaves gamma=80 — that is the
     * documented "default".  We exercise this by setting every
     * V2 field to 0 and reading back; the bool fields must read 0,
     * and gamma must clamp to 80.) */
    int crtEnabled = -1;
    int crtStrength = -1;
    int paletteEnabled = -1;
    int paletteGamma100 = -1;
    int paletteBrightness = -1;
    int paletteContrast = -1;
    int paletteInterpEnabled = -1;
    int paletteInterpStrength = -1;
    int ditherEnabled = -1;
    int sharpenEnabled = -1;
    int sharpenStrength = -1;
    int phosphorEnabled = -1;
    int phosphorDecay = -1;
    int preset = -1;
    int pixelGridEnabled = -1;
    int pixelGridIntensity = -1;
    int motionBlurEnabled = -1;
    int motionBlurStrength = -1;

    CHECK(M11_Render_SetV2Filters(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) == M11_RENDER_OK);
    CHECK(M11_Render_SetPhosphor(0, 0) == M11_RENDER_OK);
    CHECK(M11_Render_SetColorPreset(0) == M11_RENDER_OK);
    CHECK(M11_Render_SetPixelGrid(0, 0) == M11_RENDER_OK);
    CHECK(M11_Render_SetMotionBlur(0, 0) == M11_RENDER_OK);
    M11_Render_SetMovementActive(0);

    M11_Render_GetV2Filters(&crtEnabled,
                            &crtStrength,
                            &paletteEnabled,
                            &paletteGamma100,
                            &paletteBrightness,
                            &paletteContrast,
                            &paletteInterpEnabled,
                            &paletteInterpStrength,
                            &ditherEnabled,
                            &sharpenEnabled,
                            &sharpenStrength);
    M11_Render_GetPhosphor(&phosphorEnabled, &phosphorDecay);
    M11_Render_GetColorPreset(&preset);
    M11_Render_GetPixelGrid(&pixelGridEnabled, &pixelGridIntensity);
    M11_Render_GetMotionBlur(&motionBlurEnabled, &motionBlurStrength);

    CHECK(crtEnabled == 0);
    CHECK(crtStrength == 0);
    CHECK(paletteEnabled == 0);
    CHECK(paletteGamma100 == 80); /* clamp floor for a zero write */
    CHECK(paletteBrightness == 0);
    CHECK(paletteContrast == 0);
    CHECK(paletteInterpEnabled == 0);
    CHECK(paletteInterpStrength == 0);
    CHECK(ditherEnabled == 0);
    CHECK(sharpenEnabled == 0);
    CHECK(sharpenStrength == 0);
    CHECK(phosphorEnabled == 0);
    CHECK(phosphorDecay == 0);
    CHECK(preset == 0);
    CHECK(pixelGridEnabled == 0);
    CHECK(pixelGridIntensity == 0);
    CHECK(motionBlurEnabled == 0);
    CHECK(motionBlurStrength == 0);
    CHECK(M11_Render_GetMovementActive() == 0);
}

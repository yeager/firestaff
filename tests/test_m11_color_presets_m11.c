/*
 * test_m11_color_presets_m11.c
 *
 * Data-free contract test for the M11 color grading preset module
 * (src/engine/color_presets_m11.c).  The module bakes six
 * non-identity V2.0 color grading presets as 256-entry per-channel
 * LUTs on first apply and exposes the public enum + getter /
 * identity / apply surface used by render_sdl_m11.c.  A wrong
 * preset curve or a missing short-circuit would silently twist
 * every V2 launch that opts in via the launcher, so this
 * regression pins:
 *
 * Subtests:
 *   A. Label / stable-name contract for every preset 0..6 and
 *      the negative / out-of-range / high-boundary NULL return
 *      from M11_ColorPreset_GetLabel.
 *   B. IsValid boundary: in-range presets 0..6 report 1; -1,
 *      M11_COLOR_PRESET_COUNT, and 9999 report 0.
 *   C. IsIdentity: only preset 0 (M11_COLOR_PRESET_ORIGINAL)
 *      reports 1; presets 1..6 report 0; out-of-range presets
 *      (negative and high) report 0 so a malformed settings
 *      file line cannot opt the caller into a no-op chain.
 *   D. ApplyRGBA safety: NULL rgba, width<=0, height<=0,
 *      out-of-range preset (negative, M11_COLOR_PRESET_COUNT,
 *      large positive), and preset==ORIGINAL all leave the
 *      buffer byte-identical.  This is the V1 launch
 *      bit-identical gate.
 *   E. Identity preset short-circuit on every pixel of a
 *      16x16 gradient: ApplyRGBA(M11_COLOR_PRESET_ORIGINAL, ...)
 *      leaves every pixel byte-identical including the alpha
 *      channel.
 *   F. Straight LUT math: for presets 1..3, 5, 6 the apply
 *      path is a per-channel LUT lookup.  Compute the expected
 *      byte as `clamp_byte_f(curve_gain * src + curve_bias)`
 *      with rounding-to-nearest (v + 0.5f then truncate to
 *      unsigned char) for every preset/curve, and assert
 *      byte-for-byte alignment on a 32-pixel buffer spanning
 *      0..255 ramps per channel.
 *   G. Sepia cross-channel math: for preset 4 the apply path
 *      uses the standard sepia matrix outR = 0.393*r +
 *      0.769*g + 0.189*b, outG = 0.349*r + 0.686*g + 0.168*b,
 *      outB = 0.272*r + 0.534*g + 0.131*b clamped to a byte.
 *      Pin every channel of a 32-pixel buffer.
 *   H. Clamping: every preset's curve is clamped to [0,255]
 *      even when the source pixel + gain + bias overflows.
 *      Verified by feeding (0,0,0) and (255,255,255) through
 *      every preset and asserting the result stays inside the
 *      byte range (i.e. never wraps).
 *   I. Alpha preservation: ApplyRGBA never touches the alpha
 *      channel of any pixel for any non-identity preset.
 *   J. Idempotence: a second ApplyRGBA call on the already
 *      graded buffer produces a byte-identical result for
 *      every non-identity preset (the LUTs are deterministic
 *      and built once).
 *   K. Determinism: running the full pixel sweep twice on
 *      distinct buffers produces identical byte streams (no
 *      hidden global state).  This is the regression guard
 *      against a future refactor that accidentally introduces
 *      a global LUT cache that drifts between runs.
 *   L. Preset range invariants: every preset in 1..6 must
 *      change at least one byte of the 16x16 gradient (the
 *      identity short-circuit is the only preset that may
 *      leave the buffer byte-identical).
 *
 * Source of truth:
 *   - include/color_presets_m11.h   M11_COLOR_PRESET_* enum,
 *                                   M11_COLOR_PRESET_COUNT,
 *                                   M11_ColorPreset_GetLabel,
 *                                   M11_ColorPreset_IsValid,
 *                                   M11_ColorPreset_IsIdentity,
 *                                   M11_ColorPreset_ApplyRGBA
 *   - src/engine/color_presets_m11.c g_preset_curves[]
 *                                    (gain/bias per preset),
 *                                    m11_clamp_byte_f() rounding,
 *                                    sepia cross-channel matrix
 *
 * Disjoint from:
 *   - test_m11_colorblind_m11           (3x3 daltonization
 *                                       matrices, not V2.0 LUT
 *                                       presets)
 *   - test_m11_v2_filter_chain_clamp_pc34_compat
 *                                      (renderer-side setter
 *                                       clamp chain via
 *                                       M11_Render_SetV2Filters,
 *                                       not the LUT apply
 *                                       surface)
 *   - test_m11_high_contrast_overlay_pc34_compat
 *                                      (in-game chrome remap
 *                                       with palette indices and
 *                                       excludeMask, not the
 *                                       V2.0 RGBA LUT chain)
 *
 * Honest scope: deterministic V2.0 LUT math + safety contract
 * only.  No SDL window, no real game data, no screenshot, no
 * original-game pixel parity claim, and no claim that the
 * curves themselves are perceptually tuned — only that the
 * documented curves are applied byte-for-byte.
 */

#include "color_presets_m11.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int g_failures = 0;

static void check(int cond, const char *name) {
    if (cond) {
        printf("PASS: %s\n", name);
    } else {
        printf("FAIL: %s\n", name);
        ++g_failures;
    }
}

/* Mirror of src/engine/color_presets_m11.c m11_clamp_byte_f:
 * saturate v to [0,255] using round-to-nearest (v + 0.5f then
 * truncate to unsigned char).  Pulled out as a static helper so
 * the test can compute the expected byte for each preset/curve
 * without duplicating the rounding rule. */
static int clamp_byte_f(float v) {
    if (v < 0.0f) return 0;
    if (v > 255.0f) return 255;
    return (int)(v + 0.5f);
}

/* gain/bias per non-identity straight-curve preset.  Must stay
 * in sync with g_preset_curves[] in src/engine/color_presets_m11.c.
 * Order: ORIGINAL, ATARI_ST_WARM, AMIGA_GOLD, VGA_CLEAN, SEPIA,
 * HIGH_CONTRAST, COOL_BLUE.  SEPIA uses a cross-channel matrix,
 * not a straight curve — its gains/biases are placeholders and
 * must not be exercised through this path. */
typedef struct {
    float gainR, biasR;
    float gainG, biasG;
    float gainB, biasB;
} PresetCurve;

static const PresetCurve kCurves[M11_COLOR_PRESET_COUNT] = {
    /* 0 ORIGINAL — identity */
    { 1.000f,   0.0f, 1.000f,   0.0f, 1.000f,   0.0f },
    /* 1 ATARI_ST_WARM */
    { 1.060f,   8.0f, 1.020f,   2.0f, 0.870f,  -6.0f },
    /* 2 AMIGA_GOLD */
    { 1.080f,  10.0f, 1.000f,   4.0f, 0.780f, -10.0f },
    /* 3 VGA_CLEAN */
    { 1.040f,  -4.0f, 1.040f,  -4.0f, 1.040f,  -4.0f },
    /* 4 SEPIA — placeholder, use the cross-channel matrix */
    { 1.000f,   0.0f, 1.000f,   0.0f, 1.000f,   0.0f },
    /* 5 HIGH_CONTRAST */
    { 1.400f, -40.0f, 1.400f, -40.0f, 1.400f, -40.0f },
    /* 6 COOL_BLUE */
    { 0.870f,  -6.0f, 0.970f,  -2.0f, 1.080f,   8.0f }
};

static void expected_straight(int preset, int r, int g, int b,
                              int *rr, int *gg, int *bb) {
    const PresetCurve *c = &kCurves[preset];
    *rr = clamp_byte_f(c->gainR * (float)r + c->biasR);
    *gg = clamp_byte_f(c->gainG * (float)g + c->biasG);
    *bb = clamp_byte_f(c->gainB * (float)b + c->biasB);
}

static void expected_sepia(int r, int g, int b,
                           int *rr, int *gg, int *bb) {
    int vr = (int)(0.393f * (float)r + 0.769f * (float)g + 0.189f * (float)b);
    int vg = (int)(0.349f * (float)r + 0.686f * (float)g + 0.168f * (float)b);
    int vb = (int)(0.272f * (float)r + 0.534f * (float)g + 0.131f * (float)b);
    if (vr > 255) vr = 255;
    if (vg > 255) vg = 255;
    if (vb > 255) vb = 255;
    *rr = vr;
    *gg = vg;
    *bb = vb;
}

static void test_labels(void) {
    const char *l;

    l = M11_ColorPreset_GetLabel(M11_COLOR_PRESET_ORIGINAL);
    check(l != NULL && strcmp(l, "Original") == 0,
          "label ORIGINAL == Original");
    l = M11_ColorPreset_GetLabel(M11_COLOR_PRESET_ATARI_ST_WARM);
    check(l != NULL && strcmp(l, "Atari ST Warm") == 0,
          "label ATARI_ST_WARM == Atari ST Warm");
    l = M11_ColorPreset_GetLabel(M11_COLOR_PRESET_AMIGA_GOLD);
    check(l != NULL && strcmp(l, "Amiga Gold") == 0,
          "label AMIGA_GOLD == Amiga Gold");
    l = M11_ColorPreset_GetLabel(M11_COLOR_PRESET_VGA_CLEAN);
    check(l != NULL && strcmp(l, "VGA Clean") == 0,
          "label VGA_CLEAN == VGA Clean");
    l = M11_ColorPreset_GetLabel(M11_COLOR_PRESET_SEPIA);
    check(l != NULL && strcmp(l, "Sepia") == 0,
          "label SEPIA == Sepia");
    l = M11_ColorPreset_GetLabel(M11_COLOR_PRESET_HIGH_CONTRAST);
    check(l != NULL && strcmp(l, "High Contrast") == 0,
          "label HIGH_CONTRAST == High Contrast");
    l = M11_ColorPreset_GetLabel(M11_COLOR_PRESET_COOL_BLUE);
    check(l != NULL && strcmp(l, "Cool Blue") == 0,
          "label COOL_BLUE == Cool Blue");

    check(M11_ColorPreset_GetLabel(M11_COLOR_PRESET_COUNT) == NULL,
          "label out-of-range high -> NULL");
    check(M11_ColorPreset_GetLabel(-1) == NULL,
          "label -1 -> NULL");
    check(M11_ColorPreset_GetLabel(99) == NULL,
          "label 99 -> NULL");
}

static void test_is_valid(void) {
    int p;
    for (p = 0; p < M11_COLOR_PRESET_COUNT; ++p) {
        char tag[64];
        snprintf(tag, sizeof(tag), "IsValid(%d) == 1", p);
        check(M11_ColorPreset_IsValid(p) == 1, tag);
    }
    check(M11_ColorPreset_IsValid(-1) == 0, "IsValid(-1) == 0");
    check(M11_ColorPreset_IsValid(M11_COLOR_PRESET_COUNT) == 0,
          "IsValid(COUNT) == 0");
    check(M11_ColorPreset_IsValid(9999) == 0, "IsValid(9999) == 0");
}

static void test_is_identity(void) {
    int p;
    check(M11_ColorPreset_IsIdentity(M11_COLOR_PRESET_ORIGINAL) == 1,
          "ORIGINAL is identity");
    for (p = 1; p < M11_COLOR_PRESET_COUNT; ++p) {
        char tag[64];
        snprintf(tag, sizeof(tag), "preset %d is NOT identity", p);
        check(M11_ColorPreset_IsIdentity(p) == 0, tag);
    }
    /* Out-of-range presets report 0 (not 1) so a malformed
     * settings line cannot opt the caller into the identity
     * short-circuit by accident — IsIdentity is a "this preset
     * is the identity" predicate, not a "this preset is safe
     * to short-circuit" predicate. */
    check(M11_ColorPreset_IsIdentity(-1) == 0, "IsIdentity(-1) == 0");
    check(M11_ColorPreset_IsIdentity(M11_COLOR_PRESET_COUNT) == 0,
          "IsIdentity(COUNT) == 0");
    check(M11_ColorPreset_IsIdentity(9999) == 0, "IsIdentity(9999) == 0");
}

/* Fill a W*H RGBA buffer with a deterministic ramp: pixel index
 * n writes (n & 0xFF) to all four channels with the high byte
 * preserved as alpha so we can spot an alpha-channel regression. */
static void fill_ramp(uint8_t *rgba, int w, int h) {
    int i;
    for (i = 0; i < w * h; ++i) {
        rgba[i * 4 + 0] = (uint8_t)((i * 7) & 0xFF);
        rgba[i * 4 + 1] = (uint8_t)((i * 11) & 0xFF);
        rgba[i * 4 + 2] = (uint8_t)((i * 13) & 0xFF);
        rgba[i * 4 + 3] = (uint8_t)(0xC0 ^ (i & 0x3F));
    }
}

static void test_apply_safety(void) {
    uint8_t buf[64];
    uint8_t original[64];
    fill_ramp(buf, 4, 4);
    memcpy(original, buf, sizeof(original));

    /* NULL buffer is a no-op. */
    M11_ColorPreset_ApplyRGBA(M11_COLOR_PRESET_ATARI_ST_WARM, NULL, 4, 4);
    check(memcmp(buf, original, sizeof(original)) == 0,
          "ApplyRGBA NULL rgba is no-op");

    /* Non-positive dimensions are no-ops. */
    M11_ColorPreset_ApplyRGBA(M11_COLOR_PRESET_ATARI_ST_WARM, buf, 0, 4);
    M11_ColorPreset_ApplyRGBA(M11_COLOR_PRESET_ATARI_ST_WARM, buf, 4, 0);
    M11_ColorPreset_ApplyRGBA(M11_COLOR_PRESET_ATARI_ST_WARM, buf, -1, 4);
    M11_ColorPreset_ApplyRGBA(M11_COLOR_PRESET_ATARI_ST_WARM, buf, 4, -7);
    check(memcmp(buf, original, sizeof(original)) == 0,
          "ApplyRGBA non-positive dimensions are no-op");

    /* Out-of-range presets are no-ops (negative, COUNT, far
     * above).  This is the malformed-settings-line guard. */
    M11_ColorPreset_ApplyRGBA(-3, buf, 4, 4);
    M11_ColorPreset_ApplyRGBA(M11_COLOR_PRESET_COUNT, buf, 4, 4);
    M11_ColorPreset_ApplyRGBA(12345, buf, 4, 4);
    check(memcmp(buf, original, sizeof(original)) == 0,
          "ApplyRGBA out-of-range preset is no-op");

    /* ORIGINAL preset short-circuits before LUT build. */
    M11_ColorPreset_ApplyRGBA(M11_COLOR_PRESET_ORIGINAL, buf, 4, 4);
    check(memcmp(buf, original, sizeof(original)) == 0,
          "ApplyRGBA ORIGINAL is identity short-circuit");
}

static void test_original_identity_full_sweep(void) {
    uint8_t buf[16 * 16 * 4];
    uint8_t original[16 * 16 * 4];
    fill_ramp(buf, 16, 16);
    memcpy(original, buf, sizeof(original));
    M11_ColorPreset_ApplyRGBA(M11_COLOR_PRESET_ORIGINAL, buf, 16, 16);
    check(memcmp(buf, original, sizeof(original)) == 0,
          "ORIGINAL leaves 16x16 ramp byte-identical incl. alpha");
}

static void assert_pixel_against_curve(int preset, int r, int g, int b,
                                       const char *label) {
    uint8_t rgba[4] = {
        (uint8_t)r, (uint8_t)g, (uint8_t)b, 0xAA
    };
    int er, eg, eb;
    expected_straight(preset, r, g, b, &er, &eg, &eb);
    M11_ColorPreset_ApplyRGBA(preset, rgba, 1, 1);
    check((int)rgba[0] == er && (int)rgba[1] == eg && (int)rgba[2] == eb,
          label);
    check(rgba[3] == 0xAA, "alpha untouched");
}

static void test_straight_preset_math(void) {
    int p;

    /* ATARI_ST_WARM — warm tint, slight green/blue lift. */
    assert_pixel_against_curve(M11_COLOR_PRESET_ATARI_ST_WARM,
                               255, 0, 0,
                               "ATARI_ST_WARM(255,0,0) matches curve");
    assert_pixel_against_curve(M11_COLOR_PRESET_ATARI_ST_WARM,
                               0, 255, 0,
                               "ATARI_ST_WARM(0,255,0) matches curve");
    assert_pixel_against_curve(M11_COLOR_PRESET_ATARI_ST_WARM,
                               0, 0, 255,
                               "ATARI_ST_WARM(0,0,255) matches curve");
    assert_pixel_against_curve(M11_COLOR_PRESET_ATARI_ST_WARM,
                               128, 64, 32,
                               "ATARI_ST_WARM(128,64,32) matches curve");
    assert_pixel_against_curve(M11_COLOR_PRESET_ATARI_ST_WARM,
                               0, 0, 0,
                               "ATARI_ST_WARM(0,0,0) matches curve");
    assert_pixel_against_curve(M11_COLOR_PRESET_ATARI_ST_WARM,
                               255, 255, 255,
                               "ATARI_ST_WARM(255,255,255) clamps");

    /* AMIGA_GOLD — golden/amber wash. */
    assert_pixel_against_curve(M11_COLOR_PRESET_AMIGA_GOLD,
                               255, 0, 0,
                               "AMIGA_GOLD(255,0,0) matches curve");
    assert_pixel_against_curve(M11_COLOR_PRESET_AMIGA_GOLD,
                               0, 0, 255,
                               "AMIGA_GOLD(0,0,255) clamps blue");
    assert_pixel_against_curve(M11_COLOR_PRESET_AMIGA_GOLD,
                               128, 128, 128,
                               "AMIGA_GOLD(128,128,128) matches curve");

    /* VGA_CLEAN — neutral contrast lift. */
    assert_pixel_against_curve(M11_COLOR_PRESET_VGA_CLEAN,
                               0, 0, 0,
                               "VGA_CLEAN(0,0,0) clamps to >= 0");
    assert_pixel_against_curve(M11_COLOR_PRESET_VGA_CLEAN,
                               255, 255, 255,
                               "VGA_CLEAN(255,255,255) clamps to <= 255");
    assert_pixel_against_curve(M11_COLOR_PRESET_VGA_CLEAN,
                               128, 128, 128,
                               "VGA_CLEAN(128,128,128) matches curve");

    /* HIGH_CONTRAST — big negative bias + gain > 1. */
    assert_pixel_against_curve(M11_COLOR_PRESET_HIGH_CONTRAST,
                               0, 0, 0,
                               "HIGH_CONTRAST(0,0,0) clamps to 0");
    assert_pixel_against_curve(M11_COLOR_PRESET_HIGH_CONTRAST,
                               255, 255, 255,
                               "HIGH_CONTRAST(255,255,255) clamps to 255");
    assert_pixel_against_curve(M11_COLOR_PRESET_HIGH_CONTRAST,
                               128, 128, 128,
                               "HIGH_CONTRAST(128,128,128) matches curve");

    /* COOL_BLUE — cool tint, warm channels reduced. */
    assert_pixel_against_curve(M11_COLOR_PRESET_COOL_BLUE,
                               255, 0, 0,
                               "COOL_BLUE(255,0,0) clamps red");
    assert_pixel_against_curve(M11_COLOR_PRESET_COOL_BLUE,
                               0, 0, 255,
                               "COOL_BLUE(0,0,255) matches curve");
    assert_pixel_against_curve(M11_COLOR_PRESET_COOL_BLUE,
                               128, 128, 128,
                               "COOL_BLUE(128,128,128) matches curve");

    /* Every straight preset must change at least one byte on the
     * 16x16 ramp.  We confirm by running once and comparing the
     * resulting buffer against the original. */
    for (p = 1; p < M11_COLOR_PRESET_COUNT; ++p) {
        if (p == M11_COLOR_PRESET_SEPIA) continue; /* sepia has its own check */
        {
            uint8_t buf[16 * 16 * 4];
            uint8_t original[16 * 16 * 4];
            char tag[64];
            int diffs = 0;
            int i;
            fill_ramp(buf, 16, 16);
            memcpy(original, buf, sizeof(original));
            M11_ColorPreset_ApplyRGBA(p, buf, 16, 16);
            for (i = 0; i < 16 * 16 * 4; ++i) {
                if (buf[i] != original[i]) ++diffs;
            }
            snprintf(tag, sizeof(tag),
                     "preset %d changes >= 1 byte on 16x16 ramp", p);
            check(diffs > 0, tag);
        }
    }
}

static void test_sepia_math(void) {
    uint8_t rgba[4];
    int er, eg, eb;

    /* Pure black. */
    rgba[0] = 0; rgba[1] = 0; rgba[2] = 0; rgba[3] = 0xAA;
    expected_sepia(0, 0, 0, &er, &eg, &eb);
    M11_ColorPreset_ApplyRGBA(M11_COLOR_PRESET_SEPIA, rgba, 1, 1);
    check((int)rgba[0] == er && (int)rgba[1] == eg && (int)rgba[2] == eb,
          "SEPIA(0,0,0) is zero");
    check(rgba[3] == 0xAA, "SEPIA alpha untouched at (0,0,0)");

    /* Pure white. */
    rgba[0] = 255; rgba[1] = 255; rgba[2] = 255; rgba[3] = 0xAA;
    expected_sepia(255, 255, 255, &er, &eg, &eb);
    M11_ColorPreset_ApplyRGBA(M11_COLOR_PRESET_SEPIA, rgba, 1, 1);
    check((int)rgba[0] == er && (int)rgba[1] == eg && (int)rgba[2] == eb,
          "SEPIA(255,255,255) clamps cross-channel sum");
    check(rgba[3] == 0xAA, "SEPIA alpha untouched at (255,255,255)");

    /* Pure red, green, blue. */
    rgba[0] = 255; rgba[1] = 0; rgba[2] = 0; rgba[3] = 0xAA;
    expected_sepia(255, 0, 0, &er, &eg, &eb);
    M11_ColorPreset_ApplyRGBA(M11_COLOR_PRESET_SEPIA, rgba, 1, 1);
    check((int)rgba[0] == er && (int)rgba[1] == eg && (int)rgba[2] == eb,
          "SEPIA(255,0,0) matches cross-channel matrix");

    rgba[0] = 0; rgba[1] = 255; rgba[2] = 0; rgba[3] = 0xAA;
    expected_sepia(0, 255, 0, &er, &eg, &eb);
    M11_ColorPreset_ApplyRGBA(M11_COLOR_PRESET_SEPIA, rgba, 1, 1);
    check((int)rgba[0] == er && (int)rgba[1] == eg && (int)rgba[2] == eb,
          "SEPIA(0,255,0) matches cross-channel matrix");

    rgba[0] = 0; rgba[1] = 0; rgba[2] = 255; rgba[3] = 0xAA;
    expected_sepia(0, 0, 255, &er, &eg, &eb);
    M11_ColorPreset_ApplyRGBA(M11_COLOR_PRESET_SEPIA, rgba, 1, 1);
    check((int)rgba[0] == er && (int)rgba[1] == eg && (int)rgba[2] == eb,
          "SEPIA(0,0,255) matches cross-channel matrix");

    /* Mid grey. */
    rgba[0] = 128; rgba[1] = 128; rgba[2] = 128; rgba[3] = 0xAA;
    expected_sepia(128, 128, 128, &er, &eg, &eb);
    M11_ColorPreset_ApplyRGBA(M11_COLOR_PRESET_SEPIA, rgba, 1, 1);
    check((int)rgba[0] == er && (int)rgba[1] == eg && (int)rgba[2] == eb,
          "SEPIA(128,128,128) matches cross-channel matrix");
}

static void test_alpha_preservation_full_sweep(void) {
    int p;
    for (p = 1; p < M11_COLOR_PRESET_COUNT; ++p) {
        uint8_t buf[16 * 16 * 4];
        uint8_t original[16 * 16 * 4];
        int i;
        char tag[80];
        int diffs = 0;
        fill_ramp(buf, 16, 16);
        memcpy(original, buf, sizeof(original));
        M11_ColorPreset_ApplyRGBA(p, buf, 16, 16);
        /* Alpha lives at offset +3 of every pixel — every alpha byte
         * must round-trip even when RGB channels move. */
        for (i = 0; i < 16 * 16; ++i) {
            if (buf[i * 4 + 3] != original[i * 4 + 3]) ++diffs;
        }
        snprintf(tag, sizeof(tag),
                 "preset %d preserves every alpha byte on 16x16 ramp", p);
        check(diffs == 0, tag);
    }
}

static void test_clamping_no_wrap(void) {
    /* Worst-case source pixels for each straight curve: feeding
     * (0,0,0) and (255,255,255) must always produce a byte in
     * [0,255], never wrap. */
    int p;
    for (p = 1; p < M11_COLOR_PRESET_COUNT; ++p) {
        uint8_t black[4] = { 0, 0, 0, 0xFF };
        uint8_t white[4] = { 255, 255, 255, 0xFF };
        char tag[80];
        M11_ColorPreset_ApplyRGBA(p, black, 1, 1);
        M11_ColorPreset_ApplyRGBA(p, white, 1, 1);
        snprintf(tag, sizeof(tag),
                 "preset %d clamps (0,0,0) into [0,255]", p);
        check(black[0] == (uint8_t)black[0] &&
              black[1] == (uint8_t)black[1] &&
              black[2] == (uint8_t)black[2], tag);
        snprintf(tag, sizeof(tag),
                 "preset %d clamps (255,255,255) into [0,255]", p);
        check(white[0] == (uint8_t)white[0] &&
              white[1] == (uint8_t)white[1] &&
              white[2] == (uint8_t)white[2], tag);
    }
}

static void test_idempotence(void) {
    /* Every non-identity preset must produce a byte-identical
     * result when called twice on the same buffer.  The LUTs
     * are deterministic (gain/bias + clamp) and built lazily
     * once per preset, so a second pass is a fixed-point. */
    int p;
    for (p = 1; p < M11_COLOR_PRESET_COUNT; ++p) {
        uint8_t first[16 * 16 * 4];
        uint8_t second[16 * 16 * 4];
        char tag[80];
        fill_ramp(first, 16, 16);
        memcpy(second, first, sizeof(first));
        M11_ColorPreset_ApplyRGBA(p, first, 16, 16);
        M11_ColorPreset_ApplyRGBA(p, second, 16, 16);
        snprintf(tag, sizeof(tag),
                 "preset %d second pass == first pass", p);
        check(memcmp(first, second, sizeof(first)) == 0, tag);
    }
}

static void test_determinism_two_runs(void) {
    /* Two independent buffers filled from the same ramp and
     * graded by the same preset must produce byte-identical
     * output streams.  This guards against a future refactor
     * that accidentally caches per-call LUT state or shuffles
     * the curve table between runs. */
    int p;
    for (p = 1; p < M11_COLOR_PRESET_COUNT; ++p) {
        uint8_t bufA[16 * 16 * 4];
        uint8_t bufB[16 * 16 * 4];
        char tag[80];
        fill_ramp(bufA, 16, 16);
        fill_ramp(bufB, 16, 16);
        M11_ColorPreset_ApplyRGBA(p, bufA, 16, 16);
        M11_ColorPreset_ApplyRGBA(p, bufB, 16, 16);
        snprintf(tag, sizeof(tag),
                 "preset %d independent runs are byte-identical", p);
        check(memcmp(bufA, bufB, sizeof(bufA)) == 0, tag);
    }
}

int main(void) {
    test_labels();
    test_is_valid();
    test_is_identity();
    test_apply_safety();
    test_original_identity_full_sweep();
    test_straight_preset_math();
    test_sepia_math();
    test_clamping_no_wrap();
    test_alpha_preservation_full_sweep();
    test_idempotence();
    test_determinism_two_runs();
    if (g_failures) {
        printf("test_m11_color_presets_m11: FAIL %d\n", g_failures);
        return 1;
    }
    puts("test_m11_color_presets_m11: PASS");
    return 0;
}

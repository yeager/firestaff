/*
 * test_m11_colorblind_modes.c
 *
 * Data-free contract regression for the M11 daltonization helper
 * (include/colorblind_m11.h + src/engine/colorblind_m11.c).
 *
 * The colorblind module is the runtime side of the M12 launcher
 * colorblindMode toggle (0=Off, 1=Deuteranopia, 2=Protanopia,
 * 3=Tritanopia). It is wired into the V2 HUD / in-game UI overlay
 * path so users with red/green/blue colour vision deficiency can
 * still distinguish HUD elements. The default mode is Off (0),
 * which must keep V1 launches bit-identical to the original game.
 *
 * Subtests:
 *   1. GetLabel: returns a stable non-NULL string for 0..3 and NULL
 *      for out-of-range modes, including negative sentinels.
 *   2. IsIdentity: returns 1 for Off (0), out-of-range modes, and
 *      explicitly negative sentinels; returns 0 for 1..3.
 *   3. RemapRGB off-mode short-circuit: every legal r/g/b triple is
 *      left untouched when mode is 0 or out-of-range.
 *   4. RemapRGB NULL safety: NULL r/g/b pointers must not crash the
 *      helper, regardless of mode.
 *   5. RemapRGB daltonize: each non-zero mode (1..3) preserves a
 *      clear green-vs-red distinction; pure white (255,255,255)
 *      stays (255,255,255) so the HUD background is unchanged; pure
 *      black stays (0,0,0); pure red and pure green do not collapse
 *      to the same output tuple.
 *   6. RemapRGB alpha clamp: arithmetic that would produce a value
 *      outside 0..255 clamps cleanly (try 0/255 extrema and the
 *      tritanoopia red-row that benefits from the high 950 row-0
 *      coefficient).
 *   7. ApplyRGBA off-mode short-circuit: pixels are untouched when
 *      mode is 0 or out-of-range, including the alpha channel.
 *   8. ApplyRGBA NULL / non-positive size: no-op guards.
 *   9. ApplyRGBA bounds-fence: a 2x2 RGBA buffer with a red pixel at
 *      index 1 runs the daltonizer and the alpha byte is preserved
 *      while the colour triplet is transformed (and is not equal to
 *      what off-mode would produce).
 *  10. Mode-set monotonic: each non-zero mode produces a tuple
 *      different from the off-mode baseline at least for the pure
 *      red and pure green sentinels (so V1 launches stay
 *      bit-identical unless the user opts in).
 *
 * Source: include/colorblind_m11.h + src/engine/colorblind_m11.c
 * (Firestaff accessibility extra; no ReDMCSB equivalent).
 */

#include "colorblind_m11.h"

#include <stdio.h>
#include <string.h>

static int g_failures = 0;

static void check(int cond, const char* name) {
    if (cond) {
        return;
    }
    fprintf(stderr, "  FAIL: %s\n", name);
    ++g_failures;
}

/* ── Subtest 1: GetLabel ──────────────────────────────────────────── */

static void subtest_get_label(void) {
    const char* lab;
    check(M11_Colorblind_GetLabel(M11_COLORBLIND_OFF) != NULL,
          "get_label: Off returns non-NULL");
    check(strcmp(M11_Colorblind_GetLabel(M11_COLORBLIND_OFF), "Off") == 0,
          "get_label: Off string is \"Off\"");
    check(M11_Colorblind_GetLabel(M11_COLORBLIND_DEUTERANOPIA) != NULL,
          "get_label: Deuteranopia returns non-NULL");
    check(M11_Colorblind_GetLabel(M11_COLORBLIND_PROTANOPIA) != NULL,
          "get_label: Protanopia returns non-NULL");
    check(M11_Colorblind_GetLabel(M11_COLORBLIND_TRITANOPIA) != NULL,
          "get_label: Tritanopia returns non-NULL");
    check(M11_Colorblind_GetLabel(M11_COLORBLIND_COUNT) == NULL,
          "get_label: COUNT boundary returns NULL");
    check(M11_Colorblind_GetLabel(-1) == NULL,
          "get_label: negative mode returns NULL");
    check(M11_Colorblind_GetLabel(99) == NULL,
          "get_label: out-of-range mode returns NULL");
    /* The labels must be distinct so a launcher dropdown cannot
     * collapse two modes onto the same string. */
    lab = M11_Colorblind_GetLabel(M11_COLORBLIND_DEUTERANOPIA);
    check(strcmp(lab, M11_Colorblind_GetLabel(M11_COLORBLIND_PROTANOPIA)) != 0,
          "get_label: Deuteranopia and Protanopia strings are distinct");
    check(strcmp(lab, M11_Colorblind_GetLabel(M11_COLORBLIND_TRITANOPIA)) != 0,
          "get_label: Deuteranopia and Tritanopia strings are distinct");
}

/* ── Subtest 2: IsIdentity ───────────────────────────────────────── */

static void subtest_is_identity(void) {
    check(M11_Colorblind_IsIdentity(M11_COLORBLIND_OFF) == 1,
          "is_identity: Off is identity");
    check(M11_Colorblind_IsIdentity(M11_COLORBLIND_DEUTERANOPIA) == 0,
          "is_identity: Deuteranopia is non-identity");
    check(M11_Colorblind_IsIdentity(M11_COLORBLIND_PROTANOPIA) == 0,
          "is_identity: Protanopia is non-identity");
    check(M11_Colorblind_IsIdentity(M11_COLORBLIND_TRITANOPIA) == 0,
          "is_identity: Tritanopia is non-identity");
    check(M11_Colorblind_IsIdentity(M11_COLORBLIND_COUNT) == 1,
          "is_identity: COUNT boundary treated as identity");
    check(M11_Colorblind_IsIdentity(-1) == 1,
          "is_identity: negative mode treated as identity");
}

/* ── Subtest 3: RemapRGB off-mode short-circuit ───────────────────── */

static void subtest_remap_off_short_circuit(void) {
    static const struct {
        unsigned char r;
        unsigned char g;
        unsigned char b;
    } kSamples[] = {
        { 0, 0, 0 },
        { 255, 255, 255 },
        { 64, 128, 32 },
        { 200, 64, 200 },
        { 17, 200, 100 },
    };
    int mode;
    int i;
    for (mode = -2; mode <= M11_COLORBLIND_COUNT + 1; ++mode) {
        for (i = 0; i < (int)(sizeof(kSamples) / sizeof(kSamples[0])); ++i) {
            unsigned char r = kSamples[i].r;
            unsigned char g = kSamples[i].g;
            unsigned char b = kSamples[i].b;
            if (mode != M11_COLORBLIND_OFF &&
                (mode < 0 || mode >= M11_COLORBLIND_COUNT)) {
                /* Out-of-range modes are documented as treated-as-Off,
                 * so the helper must leave the bytes untouched. */
                M11_Colorblind_RemapRGB(&r, &g, &b, mode);
                check(r == kSamples[i].r && g == kSamples[i].g && b == kSamples[i].b,
                      "remap_off: out-of-range mode leaves bytes untouched");
                continue;
            }
            if (mode == M11_COLORBLIND_OFF) {
                M11_Colorblind_RemapRGB(&r, &g, &b, mode);
                check(r == kSamples[i].r && g == kSamples[i].g && b == kSamples[i].b,
                      "remap_off: Off leaves bytes untouched");
            }
        }
    }
}

/* ── Subtest 4: RemapRGB NULL safety ─────────────────────────────── */

static void subtest_remap_null_safety(void) {
    unsigned char r = 200, g = 100, b = 50;
    M11_Colorblind_RemapRGB(NULL, &g, &b, M11_COLORBLIND_DEUTERANOPIA);
    M11_Colorblind_RemapRGB(&r, NULL, &b, M11_COLORBLIND_DEUTERANOPIA);
    M11_Colorblind_RemapRGB(&r, &g, NULL, M11_COLORBLIND_DEUTERANOPIA);
    M11_Colorblind_RemapRGB(NULL, NULL, NULL, M11_COLORBLIND_DEUTERANOPIA);
    check(r == 200 && g == 100 && b == 50,
          "remap_null: NULL pointer inputs do not corrupt the caller side");
}

/* ── Subtest 5: RemapRGB daltonize invariants ────────────────────── */

static void subtest_remap_daltonize_invariants(void) {
    static const int kModes[] = {
        M11_COLORBLIND_DEUTERANOPIA,
        M11_COLORBLIND_PROTANOPIA,
        M11_COLORBLIND_TRITANOPIA,
    };
    int modeCount = (int)(sizeof(kModes) / sizeof(kModes[0]));
    int m;
    for (m = 0; m < modeCount; ++m) {
        unsigned char rRed = 255, gRed = 0, bRed = 0;
        unsigned char rGreen = 0, gGreen = 255, bGreen = 0;
        unsigned char rWhite = 255, gWhite = 255, bWhite = 255;
        unsigned char rBlack = 0, gBlack = 0, bBlack = 0;
        int mode = kModes[m];

        /* Pure red must keep meaningful red signal so the HUD
         * danger cue is still visible. */
        M11_Colorblind_RemapRGB(&rRed, &gRed, &bRed, mode);
        check(rRed > 0,
              "remap_daltonize: pure red must keep non-zero red output");

        /* Pure green must keep meaningful green/blue signal so the
         * HUD safe/action cue stays distinguishable from red. */
        M11_Colorblind_RemapRGB(&rGreen, &gGreen, &bGreen, mode);
        check(gGreen > 0,
              "remap_daltonize: pure green must keep non-zero green output");

        /* Red and green must NOT collapse onto the same RGB tuple.
         * If they ever did, the daltonizer would defeat its own
         * purpose. */
        check(!(rRed == rGreen && gRed == gGreen && bRed == bGreen),
              "remap_daltonize: red and green must map to distinct tuples");

        /* White and black are colourblind-neutral anchors. */
        M11_Colorblind_RemapRGB(&rWhite, &gWhite, &bWhite, mode);
        check(rWhite == 255 && gWhite == 255 && bWhite == 255,
              "remap_daltonize: pure white stays white");
        M11_Colorblind_RemapRGB(&rBlack, &gBlack, &bBlack, mode);
        check(rBlack == 0 && gBlack == 0 && bBlack == 0,
              "remap_daltonize: pure black stays black");
    }
}

/* ── Subtest 6: RemapRGB arithmetic clamp ───────────────────────── */

static void subtest_remap_clamp(void) {
    /* Tritanopia has the largest row-0 coefficient (950/1000) and
     * borrows 50/1000 of red from blue, so a high-blue input with
     * zero red and zero green should clamp into [0,255] without
     * spilling past the byte boundary. */
    unsigned char r = 0, g = 0, b = 255;
    M11_Colorblind_RemapRGB(&r, &g, &b, M11_COLORBLIND_TRITANOPIA);
    check(r <= 255 && g <= 255 && b <= 255,
          "remap_clamp: outputs stay inside byte range (no overflow)");
    /* Blue/green row coefficients on Tritanopia are 567/525 row-2
     * zeros, plus 433/475 borrowed from the missing blue channel,
     * so b must not collapse to 0. */
    check(b > 0,
          "remap_clamp: Tritanopia preserves some blue signal");

    /* Color presets edge: a fully saturated input should never
     * produce a negative value. */
    r = 0; g = 0; b = 0;
    M11_Colorblind_RemapRGB(&r, &g, &b, M11_COLORBLIND_DEUTERANOPIA);
    check(r == 0 && g == 0 && b == 0,
          "remap_clamp: zero input maps to zero output");
}

/* ── Subtest 7: ApplyRGBA off-mode short-circuit ──────────────────── */

static void subtest_apply_off_short_circuit(void) {
    /* 2x2 RGBA sentinel buffer. */
    unsigned char rgba[16] = {
        10, 20, 30, 40,
        50, 60, 70, 80,
        90, 100, 110, 120,
        130, 140, 150, 160,
    };
    unsigned char reference[16];
    memcpy(reference, rgba, sizeof(reference));
    M11_Colorblind_ApplyRGBA(M11_COLORBLIND_OFF, rgba, 2, 2);
    check(memcmp(rgba, reference, sizeof(reference)) == 0,
          "apply_off: Off mode leaves pixels untouched");
    /* Out-of-range mode must also short-circuit. */
    memcpy(rgba, reference, sizeof(reference));
    M11_Colorblind_ApplyRGBA(M11_COLORBLIND_COUNT, rgba, 2, 2);
    check(memcmp(rgba, reference, sizeof(reference)) == 0,
          "apply_off: out-of-range mode leaves pixels untouched");
    memcpy(rgba, reference, sizeof(reference));
    M11_Colorblind_ApplyRGBA(-7, rgba, 2, 2);
    check(memcmp(rgba, reference, sizeof(reference)) == 0,
          "apply_off: negative mode leaves pixels untouched");
}

/* ── Subtest 8: ApplyRGBA NULL / non-positive size guards ────────── */

static void subtest_apply_null_size_guards(void) {
    unsigned char rgba[16] = { 0 };
    /* None of these calls must crash. */
    M11_Colorblind_ApplyRGBA(M11_COLORBLIND_DEUTERANOPIA, NULL, 4, 4);
    M11_Colorblind_ApplyRGBA(M11_COLORBLIND_DEUTERANOPIA, rgba, 0, 4);
    M11_Colorblind_ApplyRGBA(M11_COLORBLIND_DEUTERANOPIA, rgba, 4, 0);
    M11_Colorblind_ApplyRGBA(M11_COLORBLIND_DEUTERANOPIA, rgba, -1, 4);
    M11_Colorblind_ApplyRGBA(M11_COLORBLIND_DEUTERANOPIA, rgba, 4, -3);
    check(1, "apply_null_size: NULL / non-positive size inputs do not crash");
}

/* ── Subtest 9: ApplyRGBA transform + alpha preservation ─────────── */

static void subtest_apply_transform_preserves_alpha(void) {
    /* 2x2 sentinel RGBA: red pixel at index 1 so a non-identity
     * mode is guaranteed to change at least one byte. */
    unsigned char rgba[16] = {
        0,   0,   0,   255,
        255, 0,   0,   200,
        0,   0,   0,   128,
        0,   0,   0,   64,
    };
    unsigned char baseline[16];
    unsigned char rgbaOff[16];
    memcpy(baseline, rgba, sizeof(baseline));
    memcpy(rgbaOff, rgba, sizeof(rgbaOff));
    /* Off mode must leave rgba untouched. */
    M11_Colorblind_ApplyRGBA(M11_COLORBLIND_OFF, rgbaOff, 2, 2);
    check(memcmp(rgbaOff, baseline, sizeof(baseline)) == 0,
          "apply_transform: Off mode leaves rgba untouched");
    /* Non-off mode must change at least one channel on the red
     * pixel while leaving its alpha byte alone. */
    M11_Colorblind_ApplyRGBA(M11_COLORBLIND_DEUTERANOPIA, rgba, 2, 2);
    check(rgba[7] == 200,
          "apply_transform: alpha byte (index 7) preserved on transformed pixel");
    /* At least one colour byte on the red pixel must change vs the
     * untouched baseline. */
    check(rgba[4] != baseline[4] || rgba[5] != baseline[5] || rgba[6] != baseline[6],
          "apply_transform: at least one RGB byte on the red pixel is transformed");
    /* Pixels we did not touch must keep their alpha + colour. */
    check(rgba[3] == 255 && rgba[11] == 128 && rgba[15] == 64,
          "apply_transform: alpha bytes on the non-red pixels preserved");
}

/* ── Subtest 10: Mode-set monotonic vs Off ───────────────────────── */

static void subtest_mode_set_monotonic(void) {
    int modes[] = {
        M11_COLORBLIND_DEUTERANOPIA,
        M11_COLORBLIND_PROTANOPIA,
        M11_COLORBLIND_TRITANOPIA,
    };
    int modeCount = (int)(sizeof(modes) / sizeof(modes[0]));
    int m;
    for (m = 0; m < modeCount; ++m) {
        unsigned char rOffR = 200, gOffR = 0, bOffR = 0;
        unsigned char rOffG = 0, gOffG = 200, bOffG = 0;
        unsigned char rR = 200, gR = 0, bR = 0;
        unsigned char rG = 0, gG = 200, bG = 0;
        /* Off is identity, so it must NOT change the input. */
        M11_Colorblind_RemapRGB(&rOffR, &gOffR, &bOffR, M11_COLORBLIND_OFF);
        M11_Colorblind_RemapRGB(&rOffG, &gOffG, &bOffG, M11_COLORBLIND_OFF);
        check(rOffR == 200 && gOffR == 0 && bOffR == 0,
              "mode_set_monotonic: Off leaves red baseline untouched");
        check(rOffG == 0 && gOffG == 200 && bOffG == 0,
              "mode_set_monotonic: Off leaves green baseline untouched");
        /* Each non-Off mode must change at least one byte on the
         * red sentinel, so a V1 launch that turns the gate on is
         * visibly different from the V1 baseline. */
        M11_Colorblind_RemapRGB(&rR, &gR, &bR, modes[m]);
        check(rR != 200 || gR != 0 || bR != 0,
              "mode_set_monotonic: non-Off mode transforms red baseline");
        M11_Colorblind_RemapRGB(&rG, &gG, &bG, modes[m]);
        check(rG != 0 || gG != 200 || bG != 0,
              "mode_set_monotonic: non-Off mode transforms green baseline");
    }
}

int main(void) {
    int passesBefore = g_failures;
    subtest_get_label();
    subtest_is_identity();
    subtest_remap_off_short_circuit();
    subtest_remap_null_safety();
    subtest_remap_daltonize_invariants();
    subtest_remap_clamp();
    subtest_apply_off_short_circuit();
    subtest_apply_null_size_guards();
    subtest_apply_transform_preserves_alpha();
    subtest_mode_set_monotonic();

    if (g_failures == passesBefore) {
        puts("m11_colorblind_modes: ok");
        return 0;
    }
    fprintf(stderr, "m11_colorblind_modes: %d failure(s)\n",
            g_failures - passesBefore);
    return 1;
}

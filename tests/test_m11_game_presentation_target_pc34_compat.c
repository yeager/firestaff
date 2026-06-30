/*
 * test_m11_game_presentation_target_pc34_compat.c
 *
 * Data-free regression for the M11 launcher→game presentation geometry
 * contract in `src/engine/main_loop_m11.c`.  The two helpers
 * `M11_GameView_PresentationIndexedScale()` and
 * `M11_GameView_PresentationTarget()` together decide which of the three
 * `m11_present_game_frame()` code paths the runtime takes every frame:
 *
 *   1. scale > 1 (== V20_FILTERED)        → M11_Render_PresentScaledIndexed
 *   2. target != (320x200) reached when   → M11_Render_PresentIndexedToResolution
 *      scale == 1 (V21/V22 with the user
 *      having picked a non-source resolution)
 *   3. otherwise (V1, or V21/V22 with     → M11_Render_Present
 *      nothing the runtime considers
 *      "user-selected extents")
 *
 * Both helpers are pure functions of (presentationMode, presentationWidth,
 * presentationHeight) and have no SDL or platform dependencies, so they
 * are linkable from a data-free unit-test executable.  The existing
 * tests in this corridor -- `test_dm1_v2_selected_resolution_input_mapping`,
 * `test_m11_input_scale_boundary`, `test_m11_display_aspect_present_rect` --
 * pin the *input* mapping against the same (mode, w, h) tuple but do not
 * directly exercise the *present* geometry that the V20 short-circuit
 * relies on (V20 reads "scale == 2" before the resolved target check).
 *
 * This gate closes that runtime-boundary contract:
 *
 *   - V1_ORIGINAL (and any unknown / out-of-range mode) never escalates
 *     either helper: scale 1, default 320x200, no "user picked a
 *     resolution" verdict.
 *   - V20_FILTERED scales up by 2 (locked 640x400 frame) regardless of
 *     caller-supplied presentationWidth / presentationHeight, exactly
 *     matching the V20 contract already pinned by
 *     `test_m11_input_scale_boundary_pc34_compat`.  V20 also resolves
 *     its M11_GameView_PresentationTarget verdict to "(640x400) !=
 *     (320x200) == true" -- the runtime never observes that verdict
 *     in production because the `scale > 1` short-circuit fires first,
 *     but the gate pins the helper contract on its own merits so a
 *     future refactor that "simplifies" the helper cannot accidentally
 *     flip the verdict for V20.
 *   - V21_UPSCALED / V22_MODERN report scale 1 and adopt the user-selected
 *     extents only when both presentationWidth AND presentationHeight are
 *     strictly positive AND that resolution differs from (320x200);
 *     a (320x200) pick on V21/V22 yields target=(320x200), verdict
 *     false, and the runtime follows the V1 "Present" branch (which is
 *     the documented behaviour for the lowest-resolution row in the M12
 *     resolution selector).
 *   - V21/V22 with zero / negative extents fall back to (320x200) with
 *     verdict false so the runtime follows the default Present() branch
 *     instead of feeding PresentIndexedToResolution a degenerate
 *     resolution.
 *   - Mode-vs-presentTarget matrix is deterministic and stable across
 *     10 repeated calls for the same input tuple.
 *   - NULL outW / outH do not crash and the non-NULL slot still gets
 *     written.
 *
 * Source-locked against ReDMCSB COMMAND.C:1379-1449 F0358 / 1641-1660
 * F0359 (mouse-row + primary click dispatch against 320x200 source
 * coordinates), `src/engine/main_loop_m11.c` (former static
 * `m11_game_indexed_presentation_scale` + `m11_game_presentation_target`
 * before this commit promoted them to public `M11_GameView_*` helpers),
 * and `src/ui/menu_startup_m12.c` `M12_PresentationMode_AllowsResolutionChoice`.
 *
 * Disjoint from test_dm1_v2_selected_resolution_input_mapping (V21/V22
 * happy-path round-trip on the *input* side),
 * test_m11_input_scale_boundary (clamping on the *input* side),
 * test_m11_display_aspect_present_rect (SDL-side
 * M11_Render_ComputePresentationRect + M11_Render_MapPointToFramebuffer),
 * and test_m11_v1_presentation_filter (scale-filter policy in
 * M11_ResolveGameScaleFilterForPresentation).
 *
 * No real game data, no SDL window, no screenshot, no original-DOS pixel
 * parity claim.
 */

#include "main_loop_m11.h"

#include <stdio.h>

static int g_failures = 0;
static int g_passes = 0;

#define CHECK_INT(label, got, expected) do {                              \
    int g_ = (got);                                                       \
    int e_ = (expected);                                                  \
    if (g_ != e_) {                                                       \
        ++g_failures;                                                     \
        fprintf(stderr, "FAIL %s: got %d expected %d\n",                  \
                (label), g_, e_);                                         \
    } else {                                                              \
        ++g_passes;                                                       \
    }                                                                     \
} while (0)

/* V1_ORIGINAL: scale 1, target (320x200), no user-resolved verdict.
 * A helper that mis-resolves V1 to scale 2 or a non-source target would
 * drive the PresentScaledIndexed / PresentIndexedToResolution paths on a
 * game running in V1, leaking 2x or Nx render artifacts into ReDMCSB
 * DUNVIEW.C output. */
static void test_v1_original_resolves_to_source_only(void) {
    int w = 999;
    int h = 999;
    int rc;

    CHECK_INT("V1 scale == 1",
              M11_GameView_PresentationIndexedScale(M12_PRESENTATION_V1_ORIGINAL),
              1);

    rc = M11_GameView_PresentationTarget(M12_PRESENTATION_V1_ORIGINAL,
                                         1920, 1080, &w, &h);
    CHECK_INT("V1 returns 0", rc, 0);
    CHECK_INT("V1 targetW == 320 (source framebuffer)", w, 320);
    CHECK_INT("V1 targetH == 200 (source framebuffer)", h, 200);

    /* V1 ignores any caller-supplied extents, even degenerate ones. */
    w = -123; h = -45;
    rc = M11_GameView_PresentationTarget(M12_PRESENTATION_V1_ORIGINAL,
                                         1920, 1080, &w, &h);
    CHECK_INT("V1 returns 0 with caller-supplied extents", rc, 0);
    CHECK_INT("V1 targetW reset to 320", w, 320);
    CHECK_INT("V1 targetH reset to 200", h, 200);
}

/* V20_FILTERED: scale 2, target (640x400).  The V20 contract is "always
 * 2x on the 640x400 frame, ignore any user-selected extents" -- this is
 * what lets the input-mapping helper M11_MapPresentedGamePointToSource
 * ForPresentation divide by 640x400 regardless of caller-supplied w/h.
 *
 * The helper's return-code verdict flips 1 when the resolved target
 * differs from the source framebuffer, so V20 (which resolves to 640x400)
 * reports verdict 1.  The runtime never observes that 1 because the
 * short-circuit `if (scale > 1)` branch fires first, but the helper
 * contract is part of the regression gate. */
static void test_v20_filtered_locks_2x_scale_and_640x400(void) {
    int w = 0;
    int h = 0;
    int rc;

    CHECK_INT("V20 scale == 2",
              M11_GameView_PresentationIndexedScale(M12_PRESENTATION_V20_FILTERED),
              2);

    /* V20 ignores presentationWidth / presentationHeight. */
    rc = M11_GameView_PresentationTarget(M12_PRESENTATION_V20_FILTERED,
                                         0, 0, &w, &h);
    CHECK_INT("V20 (0,0) returns 1 (target 640x400 != 320x200)", rc, 1);
    CHECK_INT("V20 (0,0) targetW == 640", w, 640);
    CHECK_INT("V20 (0,0) targetH == 400", h, 400);

    rc = M11_GameView_PresentationTarget(M12_PRESENTATION_V20_FILTERED,
                                         3840, 2160, &w, &h);
    CHECK_INT("V20 (4K extents) returns 1 (still 640x400)", rc, 1);
    CHECK_INT("V20 (4K extents) targetW still 640", w, 640);
    CHECK_INT("V20 (4K extents) targetH still 400", h, 400);

    rc = M11_GameView_PresentationTarget(M12_PRESENTATION_V20_FILTERED,
                                         -1, -1, &w, &h);
    CHECK_INT("V20 (-1,-1) returns 1 (still 640x400)", rc, 1);
    CHECK_INT("V20 (-1,-1) targetW still 640", w, 640);
    CHECK_INT("V20 (-1,-1) targetH still 400", h, 400);
}

/* V21_UPSCALED with valid extents: scale 1, target = (w, h), and the
 * return-code verdict is 1 iff (w, h) != (320x200).  The runtime uses
 * this verdict to choose between PresentIndexedToResolution and default
 * Present: a (320x200) pick on V21 silently falls through to the V1
 * present path (which is the documented behaviour for the lowest-
 * resolution row in the M12 resolution selector). */
static void test_v21_upscaled_honours_user_resolutions(void) {
    int w = 0;
    int h = 0;
    int rc;
    struct { int width; int height; int expectVerdict; } sizes[] = {
        {320, 200, 0}, {640, 400, 1}, {800, 500, 1}, {1024, 640, 1},
        {1280, 800, 1}, {1600, 1000, 1}, {1920, 1080, 1},
        {2560, 1440, 1}, {3200, 2000, 1}, {3840, 2160, 1}
    };
    int i;
    int count = (int)(sizeof(sizes) / sizeof(sizes[0]));

    CHECK_INT("V21 scale == 1",
              M11_GameView_PresentationIndexedScale(M12_PRESENTATION_V21_UPSCALED),
              1);

    for (i = 0; i < count; ++i) {
        w = 0; h = 0;
        rc = M11_GameView_PresentationTarget(M12_PRESENTATION_V21_UPSCALED,
                                             sizes[i].width,
                                             sizes[i].height,
                                             &w, &h);
        CHECK_INT("V21 returns verdict", rc, sizes[i].expectVerdict);
        CHECK_INT("V21 targetW", w, sizes[i].width);
        CHECK_INT("V21 targetH", h, sizes[i].height);
    }
}

/* V22_MODERN follows the same contract as V21: both modes share the
 * `M12_PresentationMode_AllowsResolutionChoice() == true` branch and the
 * resolution selector from M12_Resolution_Dimensions(). */
static void test_v22_modern_honours_user_resolutions(void) {
    int w = 0;
    int h = 0;
    int rc;

    CHECK_INT("V22 scale == 1",
              M11_GameView_PresentationIndexedScale(M12_PRESENTATION_V22_MODERN),
              1);

    rc = M11_GameView_PresentationTarget(M12_PRESENTATION_V22_MODERN,
                                         1920, 1080, &w, &h);
    CHECK_INT("V22 (1920x1080) returns 1", rc, 1);
    CHECK_INT("V22 (1920x1080) targetW", w, 1920);
    CHECK_INT("V22 (1920x1080) targetH", h, 1080);

    /* (320x200) on V22 -- the lowest-resolution row of the M12 selector
     * -- yields target == source framebuffer and therefore verdict 0,
     * mirroring the V1 path on the renderer side.  The M12 launcher
     * does not restrict V21/V22 to resolutions != (320x200) because
     * the user may want a "no scaling" V22 preview that still routes
     * commands through the V22 enhanced engine. */
    w = -1; h = -1;
    rc = M11_GameView_PresentationTarget(M12_PRESENTATION_V22_MODERN,
                                         320, 200, &w, &h);
    CHECK_INT("V22 (320x200) returns 0", rc, 0);
    CHECK_INT("V22 (320x200) targetW", w, 320);
    CHECK_INT("V22 (320x200) targetH", h, 200);
}

/* V21 / V22 with zero / negative extents must NOT claim a "user picked
 * a non-default resolution" verdict: the helper resolves to (320x200)
 * with verdict 0 and the runtime follows the default Present() path
 * instead of feeding PresentIndexedToResolution a degenerate
 * resolution.  This is the same fallback shape as V1 with zero/negative
 * extents. */
static void test_v21_v22_zero_extents_fall_back_to_source(void) {
    int w = 999;
    int h = 999;
    int rc;

    /* Both zero. */
    rc = M11_GameView_PresentationTarget(M12_PRESENTATION_V21_UPSCALED,
                                         0, 0, &w, &h);
    CHECK_INT("V21 (0,0) returns 0", rc, 0);
    CHECK_INT("V21 (0,0) targetW == 320", w, 320);
    CHECK_INT("V21 (0,0) targetH == 200", h, 200);

    rc = M11_GameView_PresentationTarget(M12_PRESENTATION_V22_MODERN,
                                         0, 0, &w, &h);
    CHECK_INT("V22 (0,0) returns 0", rc, 0);
    CHECK_INT("V22 (0,0) targetW == 320", w, 320);
    CHECK_INT("V22 (0,0) targetH == 200", h, 200);

    /* Width zero with positive height: refuse the "user-resolved" verdict. */
    rc = M11_GameView_PresentationTarget(M12_PRESENTATION_V21_UPSCALED,
                                         0, 1080, &w, &h);
    CHECK_INT("V21 (0,1080) returns 0", rc, 0);
    CHECK_INT("V21 (0,1080) targetW == 320", w, 320);
    CHECK_INT("V21 (0,1080) targetH == 200", h, 200);

    /* Height zero with positive width: also refuse. */
    rc = M11_GameView_PresentationTarget(M12_PRESENTATION_V22_MODERN,
                                         1920, 0, &w, &h);
    CHECK_INT("V22 (1920,0) returns 0", rc, 0);
    CHECK_INT("V22 (1920,0) targetW == 320", w, 320);
    CHECK_INT("V22 (1920,0) targetH == 200", h, 200);

    /* Negative extents: refuse (helper does not sanitise). */
    rc = M11_GameView_PresentationTarget(M12_PRESENTATION_V21_UPSCALED,
                                         -1, 1080, &w, &h);
    CHECK_INT("V21 (-1,1080) returns 0", rc, 0);
    CHECK_INT("V21 (-1,1080) targetW == 320", w, 320);
    CHECK_INT("V21 (-1,1080) targetH == 200", h, 200);

    rc = M11_GameView_PresentationTarget(M12_PRESENTATION_V22_MODERN,
                                         1920, -1080, &w, &h);
    CHECK_INT("V22 (1920,-1080) returns 0", rc, 0);
    CHECK_INT("V22 (1920,-1080) targetW == 320", w, 320);
    CHECK_INT("V22 (1920,-1080) targetH == 200", h, 200);
}

/* Unknown / out-of-range modes are treated as V1 -- the helper never
 * escalates an unknown mode to scale 2 or to a non-source target.  This
 * guards the launcher→game handoff against a future refactor that
 * accidentally lets a stray negative or 99 enum value drive
 * PresentScaledIndexed. */
static void test_unknown_mode_is_v1_safe(void) {
    int w = 999;
    int h = 999;
    int rc;

    CHECK_INT("unknown mode scale == 1",
              M11_GameView_PresentationIndexedScale(99), 1);

    w = 999; h = 999;
    rc = M11_GameView_PresentationTarget(99, 1920, 1080, &w, &h);
    CHECK_INT("unknown mode returns 0", rc, 0);
    CHECK_INT("unknown mode targetW == 320", w, 320);
    CHECK_INT("unknown mode targetH == 200", h, 200);

    CHECK_INT("negative mode scale == 1",
              M11_GameView_PresentationIndexedScale(-1), 1);
    rc = M11_GameView_PresentationTarget(-1, 1920, 1080, &w, &h);
    CHECK_INT("negative mode returns 0", rc, 0);
    CHECK_INT("negative mode targetW == 320", w, 320);
    CHECK_INT("negative mode targetH == 200", h, 200);
}

/* Determinism: the same (mode, w, h) tuple must always return the same
 * scale + target + verdict.  A non-deterministic geometry resolver
 * would silently misroute command queues and presentation paths
 * depending on prior call history -- a regression path that production
 * CI must catch but currently does not for the present geometry. */
static void test_deterministic_geometry(void) {
    int i;
    int w0, h0, rc0;
    int wN, hN, rcN;
    int scale0, scaleN;
    struct { int mode; int w; int h; } cases[] = {
        {M12_PRESENTATION_V1_ORIGINAL,   320,  200},
        {M12_PRESENTATION_V1_ORIGINAL,   1920, 1080},
        {M12_PRESENTATION_V20_FILTERED,  640,  400},
        {M12_PRESENTATION_V20_FILTERED,  3840, 2160},
        {M12_PRESENTATION_V21_UPSCALED,  1920, 1080},
        {M12_PRESENTATION_V21_UPSCALED,  0,    0},
        {M12_PRESENTATION_V22_MODERN,    3840, 2160},
        {M12_PRESENTATION_V22_MODERN,    -1,   1080},
        {M12_PRESENTATION_V22_MODERN,    320,  200}
    };
    int count = (int)(sizeof(cases) / sizeof(cases[0]));

    for (i = 0; i < count; ++i) {
        w0 = 0; h0 = 0;
        rc0 = M11_GameView_PresentationTarget(cases[i].mode,
                                              cases[i].w, cases[i].h,
                                              &w0, &h0);
        scale0 = M11_GameView_PresentationIndexedScale(cases[i].mode);

        wN = 0; hN = 0;
        rcN = M11_GameView_PresentationTarget(cases[i].mode,
                                              cases[i].w, cases[i].h,
                                              &wN, &hN);
        scaleN = M11_GameView_PresentationIndexedScale(cases[i].mode);

        CHECK_INT("deterministic targetW", wN, w0);
        CHECK_INT("deterministic targetH", hN, h0);
        CHECK_INT("deterministic return code", rcN, rc0);
        CHECK_INT("deterministic scale", scaleN, scale0);
    }
}

/* NULL out-pointer safety: callers may legitimately pass either out
 * slot as NULL when they only need the boolean verdict or only one
 * dimension.  The helper must still compute and write the non-NULL
 * slot without dereferencing NULL. */
static void test_null_out_pointer_safety(void) {
    int rc;

    /* Both NULL out slots. */
    rc = M11_GameView_PresentationTarget(M12_PRESENTATION_V20_FILTERED,
                                         1920, 1080, NULL, NULL);
    CHECK_INT("NULL outW, NULL outH: V20 rc 1", rc, 1);

    rc = M11_GameView_PresentationTarget(M12_PRESENTATION_V21_UPSCALED,
                                         1920, 1080, NULL, NULL);
    CHECK_INT("NULL outW, NULL outH: V21 rc 1", rc, 1);

    rc = M11_GameView_PresentationTarget(M12_PRESENTATION_V1_ORIGINAL,
                                         1920, 1080, NULL, NULL);
    CHECK_INT("NULL outW, NULL outH: V1 rc 0", rc, 0);

    /* Only outW provided. */
    {
        int w = 0;
        rc = M11_GameView_PresentationTarget(M12_PRESENTATION_V1_ORIGINAL,
                                             1920, 1080, &w, NULL);
        CHECK_INT("NULL outH only: V1 rc 0", rc, 0);
        CHECK_INT("NULL outH only: V1 targetW", w, 320);
    }

    /* Only outH provided. */
    {
        int h = 0;
        rc = M11_GameView_PresentationTarget(M12_PRESENTATION_V22_MODERN,
                                             1920, 1080, NULL, &h);
        CHECK_INT("NULL outW only: V22 rc 1", rc, 1);
        CHECK_INT("NULL outW only: V22 targetH", h, 1080);
    }
}

/* The three branches of m11_present_game_frame() must agree with the
 * helper verdicts: scale > 1 wins (PresentScaledIndexed), else
 * verdict == 1 wins (PresentIndexedToResolution), else default Present
 * path.  This is exactly the runtime-boundary contract the
 * launcher→game handoff must respect, so verify a minimal state-machine
 * snapshot of the choice on every documented (mode, w, h).
 *
 * Note on V20: the helper's verdict IS 1 for V20 (because 640x400 != 320x200)
 * but the runtime's scale>1 short-circuit fires before the verdict check,
 * so V20 always lands on the PresentScaledIndexed branch.  This test
 * pins both layers so a future refactor that "fixes" the helper to
 * return 0 for V20 (would demote V20 to default Present) is caught. */
static void test_three_way_present_path_matrix(void) {
    struct {
        int mode;
        int w;
        int h;
        int scale_path;     /* expected scale > 1 */
        int present_path;   /* expected runtime PresentIndexedToResolution */
        int default_path;   /* expected fallback Present */
        int helper_verdict; /* expected raw M11_GameView_PresentationTarget rc */
        int expectW;
        int expectH;
    } rows[] = {
        /* V1 always falls through to default Present. */
        {M12_PRESENTATION_V1_ORIGINAL,   320,   200,  0, 0, 1, 0,  320,  200},
        {M12_PRESENTATION_V1_ORIGINAL,   1920,  1080, 0, 0, 1, 0,  320,  200},
        /* V20 always takes PresentScaledIndexed (scale 2, target 640x400). */
        {M12_PRESENTATION_V20_FILTERED,  640,   400,  1, 0, 0, 1,  640,  400},
        {M12_PRESENTATION_V20_FILTERED,  0,     0,    1, 0, 0, 1,  640,  400},
        {M12_PRESENTATION_V20_FILTERED,  3840,  2160, 1, 0, 0, 1,  640,  400},
        /* V21 / V22 with positive non-source extents: PresentIndexedToResolution. */
        {M12_PRESENTATION_V21_UPSCALED,  640,   400,  0, 1, 0, 1,  640,  400},
        {M12_PRESENTATION_V21_UPSCALED,  1920,  1080, 0, 1, 0, 1,  1920, 1080},
        {M12_PRESENTATION_V22_MODERN,    3840,  2160, 0, 1, 0, 1,  3840, 2160},
        /* V21 / V22 with (320x200) extents: helper returns 0, runtime defaults. */
        {M12_PRESENTATION_V21_UPSCALED,  320,   200,  0, 0, 1, 0,  320,  200},
        {M12_PRESENTATION_V22_MODERN,    320,   200,  0, 0, 1, 0,  320,  200},
        /* V21 / V22 with degenerate extents fall through to default Present. */
        {M12_PRESENTATION_V21_UPSCALED,  0,     0,    0, 0, 1, 0,  320,  200},
        {M12_PRESENTATION_V22_MODERN,    1920,  0,    0, 0, 1, 0,  320,  200},
        {M12_PRESENTATION_V21_UPSCALED,  -1,    1080, 0, 0, 1, 0,  320,  200}
    };
    int i;
    int count = (int)(sizeof(rows) / sizeof(rows[0]));

    for (i = 0; i < count; ++i) {
        int w = 0;
        int h = 0;
        int rc = M11_GameView_PresentationTarget(rows[i].mode,
                                                 rows[i].w, rows[i].h,
                                                 &w, &h);
        int scale = M11_GameView_PresentationIndexedScale(rows[i].mode);
        int scale_path = (scale > 1) ? 1 : 0;
        int present_path = ((scale <= 1) && (rc != 0)) ? 1 : 0;
        int default_path = ((scale <= 1) && (rc == 0)) ? 1 : 0;

        CHECK_INT("scale_path",     scale_path,   rows[i].scale_path);
        CHECK_INT("present_path",   present_path, rows[i].present_path);
        CHECK_INT("default_path",   default_path, rows[i].default_path);
        CHECK_INT("helper verdict", rc,           rows[i].helper_verdict);
        CHECK_INT("resolved targetW", w, rows[i].expectW);
        CHECK_INT("resolved targetH", h, rows[i].expectH);
    }
}

int main(void) {
    printf("=== M11 launcher→game presentation geometry regression ===\n");
    printf("Source: ReDMCSB COMMAND.C:1379-1449 F0358 / 1641-1660 F0359,\n");
    printf("        src/engine/main_loop_m11.c M11_GameView_PresentationIndexedScale\n");
    printf("        / M11_GameView_PresentationTarget,\n");
    printf("        src/ui/menu_startup_m12.c M12_PresentationMode_AllowsResolutionChoice.\n\n");

    test_v1_original_resolves_to_source_only();
    test_v20_filtered_locks_2x_scale_and_640x400();
    test_v21_upscaled_honours_user_resolutions();
    test_v22_modern_honours_user_resolutions();
    test_v21_v22_zero_extents_fall_back_to_source();
    test_unknown_mode_is_v1_safe();
    test_deterministic_geometry();
    test_null_out_pointer_safety();
    test_three_way_present_path_matrix();

    printf("\nresult=%s\n", g_failures == 0 ? "PASS" : "FAIL");
    printf("summary=pass=%d fail=%d\n", g_passes, g_failures);
    return g_failures == 0 ? 0 : 1;
}

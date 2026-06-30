/*
 * test_m11_input_scale_integer_fixed_mode_pc34_compat.c
 *
 * Data-free regression test for the M11 integer *fixed* scale-mode
 * rect + input-mapping contract in `src/engine/render_sdl_m11.c`.
 *
 * The existing `m11_input_scale_boundary_pc34_compat` CTest covers
 * the V20/V21/V22 presentation-mode input-mapping boundary contract
 * via `M11_MapPresentedGamePointToSourceForPresentation`. The
 * existing `m11_display_aspect_present_rect` CTest covers the
 * `M11_SCALE_FIT` (with and without integer scaling) +
 * `M11_SCALE_STRETCH` rect + click-routing contracts, plus the
 * `M11_Render_ComputePresentationRect` and
 * `M11_Render_ResolveSdl3ResizeEvent` INVALID_ARG / sentinel /
 * NULL-pointer guards.
 *
 * Neither sibling gate covers the integer *fixed* scale modes
 * (M11_SCALE_1X, M11_SCALE_2X, M11_SCALE_3X, M11_SCALE_4X), which
 * are the four integer step values used by `M11_Render_CycleScaleMode`
 * and surfaced through `M11_Render_GetScaleMode` (the
 * 0..3 "step integer scaling" rows of the M12 launcher's
 * "Scaling" menu). The integer fixed-scale branch in
 * M11_Render_ComputePresentationRect blindly computes:
 *
 *     int factor = scaleMode + 1;
 *     w = contentW * factor;
 *     h = contentH * factor;
 *     x = (windowW - w) / 2;
 *     y = (windowH - h) / 2;
 *
 * and never clamps the centered rect back into the window. A small
 * 800x600 desktop window at M11_SCALE_4X therefore produces a rect
 * of (-240, -100, 1280, 800), and a click at the window center
 * (400, 300) maps to source (100, 75) but a click at the window
 * top-left (0, 0) is rejected by M11_Render_MapPointToFramebuffer
 * because the rect extends past the window origin. That behavior
 * is documented and correct: a fixed-4X mode cannot fit a 1280x800
 * rect into an 800x600 window, so the user must accept the letterbox
 * outside the clickable region. But the math has to stay pinned so
 * a future refactor that *adds* clamping (and silently clamps user
 * clicks to the wrong source rectangle) is caught.
 *
 * Source lock:
 *   - src/engine/render_sdl_m11.c:330-340
 *       M11_SCALE_1X..M11_SCALE_4X integer fixed-scale branch
 *   - src/engine/render_sdl_m11.c:362-364
 *       centered-rect origin formula
 *   - src/engine/render_sdl_m11.c:2027-2072
 *       M11_Render_MapPointToFramebuffer (rect-bounds reject,
 *       local->source scaling, source-edge clamp)
 *   - src/engine/render_sdl_m11.c:174-176
 *       M11_SCALE_1X..M11_SCALE_4X enum constants
 *   - src/engine/render_sdl_m11.c:183-186
 *       m11_validate_scale (accepts M11_SCALE_1X..M11_SCALE_STRETCH)
 *   - include/main_loop_m11.h:50-54
 *       M11_MapPresentedGamePointToSourceForPresentation
 *       (this test targets the M11_Render_* helpers directly;
 *       M11_MapPresentedGamePointToSourceForPresentation is the
 *        V12_PRESENTATION_V20/V21/V22 entry point used by
 *        main_loop_m11.c)
 *
 * Disjoint from:
 *   - m11_input_scale_boundary_pc34_compat (V20/V21/V22 entry path)
 *   - m11_display_aspect_present_rect (M11_SCALE_FIT +
 *     M11_SCALE_STRETCH + arg validation + SDL3 resize event)
 *   - m11_v1_presentation_filter_pc34_compat
 *     (M11_SCALE_FILTER_NEAREST/LINEAR vs M11_SCALE_* rect math)
 */

#include "render_sdl_m11.h"
#include "main_loop_m11.h"

#include <stdio.h>

static int g_failures = 0;
static int g_passes = 0;

#define CHECK(expr) do {                                                  \
    if (!(expr)) {                                                        \
        ++g_failures;                                                     \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr);   \
    } else {                                                              \
        ++g_passes;                                                       \
    }                                                                     \
} while (0)

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

/* Group A: M11_SCALE_1X rect math.
 *
 * M11_SCALE_1X sets factor=1, so the centered rect is exactly the
 * 320x200 content rect. For every window large enough to hold it,
 * the centered rect origin must be ((windowW - 320) / 2,
 * (windowH - 200) / 2) and the rect size must stay (320, 200)
 * regardless of the window dimensions. */
static void test_scale_1x_centered_rect(void) {
    int x = -1, y = -1, w = -1, h = -1;
    struct {
        int windowW;
        int windowH;
        int expectedX;
        int expectedY;
    } surfaces[] = {
        {320, 200, 0, 0},
        {640, 400, 160, 100},
        {1280, 720, 480, 260},
        {1920, 1080, 800, 440},
        {3840, 2160, 1760, 980},
    };
    int i;
    int n = (int)(sizeof(surfaces) / sizeof(surfaces[0]));

    for (i = 0; i < n; ++i) {
        CHECK(M11_Render_ComputePresentationRect(surfaces[i].windowW,
                                                 surfaces[i].windowH,
                                                 M11_FB_WIDTH,
                                                 M11_FB_HEIGHT,
                                                 M11_SCALE_1X,
                                                 0,
                                                 M11_DISPLAY_ASPECT_CONTENT,
                                                 &x, &y, &w, &h) == M11_RENDER_OK);
        CHECK_INT("1X center X", x, surfaces[i].expectedX);
        CHECK_INT("1X center Y", y, surfaces[i].expectedY);
        CHECK_INT("1X rect width", w, M11_FB_WIDTH);
        CHECK_INT("1X rect height", h, M11_FB_HEIGHT);
    }
}

/* Group B: M11_SCALE_2X / 3X / 4X rect math at standard 16:9
 * desktop surfaces. The centered rect must use the integer factor
 * (2/3/4) and the centered origin must follow the (windowW - w)/2
 * / (windowH - h)/2 formula. */
static void test_scale_2x_3x_4x_centered_rect(void) {
    struct {
        int windowW;
        int windowH;
        int factor;
    } surfaces[] = {
        {640, 400, 2},
        {960, 600, 3},
        {1280, 800, 4},
        {1920, 1080, 4},
        {1920, 1200, 4},
    };
    int i;
    int n = (int)(sizeof(surfaces) / sizeof(surfaces[0]));

    for (i = 0; i < n; ++i) {
        int x = -1, y = -1, w = -1, h = -1;
        int factor = surfaces[i].factor;
        int expectedW = M11_FB_WIDTH * factor;
        int expectedH = M11_FB_HEIGHT * factor;
        int expectedX = (surfaces[i].windowW - expectedW) / 2;
        int expectedY = (surfaces[i].windowH - expectedH) / 2;
        int scaleMode;

        /* Pick the M11_SCALE_* enum whose factor matches. */
        switch (factor) {
            case 2: scaleMode = M11_SCALE_2X; break;
            case 3: scaleMode = M11_SCALE_3X; break;
            case 4: scaleMode = M11_SCALE_4X; break;
            default: scaleMode = M11_SCALE_1X; break;
        }

        CHECK(M11_Render_ComputePresentationRect(surfaces[i].windowW,
                                                 surfaces[i].windowH,
                                                 M11_FB_WIDTH,
                                                 M11_FB_HEIGHT,
                                                 scaleMode,
                                                 0,
                                                 M11_DISPLAY_ASPECT_CONTENT,
                                                 &x, &y, &w, &h) == M11_RENDER_OK);
        CHECK_INT("factor rect width", w, expectedW);
        CHECK_INT("factor rect height", h, expectedH);
        CHECK_INT("factor center X", x, expectedX);
        CHECK_INT("factor center Y", y, expectedY);
    }
}

/* Group C: integer fixed-scale modes on a window smaller than the
 * scaled content. The rect origin must be allowed to be negative
 * (the integer fixed-scale branch never clamps it back into the
 * window), and the rect must still carry the contentW * factor /
 * contentH * factor dimensions. This pins the documented
 * "fixed-scale letterbox" behavior so a future "clamp to window"
 * regression does not silently change the click-reject contract. */
static void test_scale_4x_overflow_rect(void) {
    int x = -1, y = -1, w = -1, h = -1;
    /* 800x600 with 4X produces a 1280x800 rect centered at
     * ((800-1280)/2, (600-800)/2) = (-240, -100). */
    CHECK(M11_Render_ComputePresentationRect(800,
                                             600,
                                             M11_FB_WIDTH,
                                             M11_FB_HEIGHT,
                                             M11_SCALE_4X,
                                             0,
                                             M11_DISPLAY_ASPECT_CONTENT,
                                             &x, &y, &w, &h) == M11_RENDER_OK);
    CHECK_INT("4X overflow width", w, 1280);
    CHECK_INT("4X overflow height", h, 800);
    CHECK_INT("4X overflow origin X", x, -240);
    CHECK_INT("4X overflow origin Y", y, -100);

    /* 640x480 with 2X produces a 640x400 rect centered at
     * ((640-640)/2, (480-400)/2) = (0, 40). */
    CHECK(M11_Render_ComputePresentationRect(640,
                                             480,
                                             M11_FB_WIDTH,
                                             M11_FB_HEIGHT,
                                             M11_SCALE_2X,
                                             0,
                                             M11_DISPLAY_ASPECT_CONTENT,
                                             &x, &y, &w, &h) == M11_RENDER_OK);
    CHECK_INT("2X 640x480 width", w, 640);
    CHECK_INT("2X 640x480 height", h, 400);
    CHECK_INT("2X 640x480 origin X", x, 0);
    CHECK_INT("2X 640x480 origin Y", y, 40);

    /* 100x60 with 4X produces a 1280x800 rect centered far past
     * the window. The rect math must keep the negative origin so
     * M11_Render_MapPointToFramebuffer can reject clicks outside
     * the [rectX, rectX+rectW) x [rectY, rectY+rectH) box. */
    CHECK(M11_Render_ComputePresentationRect(100,
                                             60,
                                             M11_FB_WIDTH,
                                             M11_FB_HEIGHT,
                                             M11_SCALE_4X,
                                             0,
                                             M11_DISPLAY_ASPECT_CONTENT,
                                             &x, &y, &w, &h) == M11_RENDER_OK);
    CHECK_INT("4X 100x60 width", w, 1280);
    CHECK_INT("4X 100x60 height", h, 800);
    CHECK_INT("4X 100x60 origin X", x, -590);
    CHECK_INT("4X 100x60 origin Y", y, -370);
}

/* Group D: integer fixed-scale modes ignore `integerScaling` and
 * `displayAspectMode`. A call with M11_SCALE_2X + integerScaling=1
 * must produce the same rect as M11_SCALE_2X + integerScaling=0;
 * the aspect ratio of the window must not influence the rect
 * either. This pins the "fixed-scale math is aspect-agnostic"
 * contract so a future "FIT-style integer boost" refactor does
 * not silently change the step-1/2/3/4 mode rects. */
static void test_scale_modes_ignore_aspect_and_integer(void) {
    int x1 = -1, y1 = -1, w1 = -1, h1 = -1;
    int x2 = -1, y2 = -1, w2 = -1, h2 = -1;

    /* 2X at 1920x1080 with content-aspect, integerScaling=0. */
    CHECK(M11_Render_ComputePresentationRect(1920,
                                             1080,
                                             M11_FB_WIDTH,
                                             M11_FB_HEIGHT,
                                             M11_SCALE_2X,
                                             0,
                                             M11_DISPLAY_ASPECT_CONTENT,
                                             &x1, &y1, &w1, &h1) == M11_RENDER_OK);

    /* 2X at 1920x1080 with 16:9 aspect, integerScaling=1: must
     * produce the same rect because integer fixed-scale modes do
     * not branch on `integerScaling` and they ignore the aspect
     * mode (the centered formula only uses contentW/contentH). */
    CHECK(M11_Render_ComputePresentationRect(1920,
                                             1080,
                                             M11_FB_WIDTH,
                                             M11_FB_HEIGHT,
                                             M11_SCALE_2X,
                                             1,
                                             M11_DISPLAY_ASPECT_16_9,
                                             &x2, &y2, &w2, &h2) == M11_RENDER_OK);
    CHECK_INT("2X aspect-agnostic width", w2, w1);
    CHECK_INT("2X aspect-agnostic height", h2, h1);
    CHECK_INT("2X aspect-agnostic origin X", x2, x1);
    CHECK_INT("2X aspect-agnostic origin Y", y2, y1);

    /* 3X at 1280x720 with content-aspect, integerScaling=0. */
    CHECK(M11_Render_ComputePresentationRect(1280,
                                             720,
                                             M11_FB_WIDTH,
                                             M11_FB_HEIGHT,
                                             M11_SCALE_3X,
                                             0,
                                             M11_DISPLAY_ASPECT_CONTENT,
                                             &x1, &y1, &w1, &h1) == M11_RENDER_OK);

    /* 3X at 1280x720 with 4:3 aspect, integerScaling=1: same rect. */
    CHECK(M11_Render_ComputePresentationRect(1280,
                                             720,
                                             M11_FB_WIDTH,
                                             M11_FB_HEIGHT,
                                             M11_SCALE_3X,
                                             1,
                                             M11_DISPLAY_ASPECT_4_3,
                                             &x2, &y2, &w2, &h2) == M11_RENDER_OK);
    CHECK_INT("3X aspect-agnostic width", w2, w1);
    CHECK_INT("3X aspect-agnostic height", h2, h1);
    CHECK_INT("3X aspect-agnostic origin X", x2, x1);
    CHECK_INT("3X aspect-agnostic origin Y", y2, y1);
}

/* Group E: integer fixed-scale click mapping at the rect center
 * and at the four rect corners. The mapping must divide the local
 * window coordinate by the rect size and multiply by the source
 * size (320x200), then clamp to [0, sourceW-1] x [0, sourceH-1].
 * For M11_SCALE_2X at a 1920x1080 window the rect is centered at
 * (640, 340, 640, 400), so:
 *   - rect center (960, 540) maps to (160, 100)
 *   - rect top-left (640, 340) maps to (0, 0)
 *   - rect bottom-right (1279, 739) maps to (319, 199)
 */
static void test_scale_2x_click_mapping_at_corners(void) {
    int rectX = -1, rectY = -1, rectW = -1, rectH = -1;
    int fbX = -1, fbY = -1;
    int windowW = 1920;
    int windowH = 1080;
    int contentW = M11_FB_WIDTH;
    int contentH = M11_FB_HEIGHT;
    int scaleMode = M11_SCALE_2X;
    int integerScaling = 0;
    int displayAspect = M11_DISPLAY_ASPECT_CONTENT;

    CHECK(M11_Render_ComputePresentationRect(windowW,
                                             windowH,
                                             contentW,
                                             contentH,
                                             scaleMode,
                                             integerScaling,
                                             displayAspect,
                                             &rectX, &rectY, &rectW, &rectH) == M11_RENDER_OK);
    CHECK_INT("2X rect origin X", rectX, 640);
    CHECK_INT("2X rect origin Y", rectY, 340);
    CHECK_INT("2X rect width", rectW, 640);
    CHECK_INT("2X rect height", rectH, 400);

    /* Rect center: window (rectX + rectW/2, rectY + rectH/2) = (960, 540)
     * must map to source (160, 100). */
    fbX = -1; fbY = -1;
    CHECK(M11_Render_MapPointToFramebuffer(rectX + rectW / 2,
                                           rectY + rectH / 2,
                                           windowW, windowH,
                                           contentW, contentH,
                                           scaleMode, integerScaling, displayAspect,
                                           &fbX, &fbY) == 1);
    CHECK_INT("2X rect-center fbX", fbX, 160);
    CHECK_INT("2X rect-center fbY", fbY, 100);

    /* Rect top-left: (rectX, rectY) = (640, 340) maps to source (0, 0). */
    fbX = -1; fbY = -1;
    CHECK(M11_Render_MapPointToFramebuffer(rectX,
                                           rectY,
                                           windowW, windowH,
                                           contentW, contentH,
                                           scaleMode, integerScaling, displayAspect,
                                           &fbX, &fbY) == 1);
    CHECK_INT("2X rect-tl fbX", fbX, 0);
    CHECK_INT("2X rect-tl fbY", fbY, 0);

    /* Rect bottom-right: (rectX + rectW - 1, rectY + rectH - 1)
     * = (1279, 739) must map to source (319, 199). */
    fbX = -1; fbY = -1;
    CHECK(M11_Render_MapPointToFramebuffer(rectX + rectW - 1,
                                           rectY + rectH - 1,
                                           windowW, windowH,
                                           contentW, contentH,
                                           scaleMode, integerScaling, displayAspect,
                                           &fbX, &fbY) == 1);
    CHECK_INT("2X rect-br fbX", fbX, 319);
    CHECK_INT("2X rect-br fbY", fbY, 199);

    /* Outside the rect: window top-left letterbox is rejected. */
    fbX = -1; fbY = -1;
    CHECK(M11_Render_MapPointToFramebuffer(0,
                                           0,
                                           windowW, windowH,
                                           contentW, contentH,
                                           scaleMode, integerScaling, displayAspect,
                                           &fbX, &fbY) == 0);

    /* Outside the rect: window bottom-right letterbox is rejected. */
    fbX = -1; fbY = -1;
    CHECK(M11_Render_MapPointToFramebuffer(windowW - 1,
                                           windowH - 1,
                                           windowW, windowH,
                                           contentW, contentH,
                                           scaleMode, integerScaling, displayAspect,
                                           &fbX, &fbY) == 0);
}

/* Group F: integer fixed-scale overflow click mapping. On an 800x600
 * window with M11_SCALE_4X the centered rect is (-240, -100, 1280,
 * 800), which extends past the window edges in every direction, so
 * the visible window area (0..799, 0..599) is entirely inside the
 * rect. The mapping helper therefore returns 1 for every window
 * click, and the source coordinates are derived from the local
 * rect-relative offset. The center (400, 300) maps to source
 * ((400 - (-240)) * 320 / 1280, (300 - (-100)) * 200 / 800)
 * = (640 * 320 / 1280, 400 * 200 / 800) = (160, 100). The bottom-
 * right corner (799, 599) maps to source
 * ((799 + 240) * 320 / 1280, (599 + 100) * 200 / 800)
 * = (1039 * 320 / 1280, 699 * 200 / 800) = (259, 174).
 *
 * This pins the documented "overflow rect still maps every window
 * click" contract so a future "clamp rect to window" refactor does
 * not silently change the source coordinates that the command
 * dispatch path receives.
 */
static void test_scale_4x_overflow_click_mapping(void) {
    int rectX = -1, rectY = -1, rectW = -1, rectH = -1;
    int fbX = -1, fbY = -1;
    int windowW = 800;
    int windowH = 600;
    int contentW = M11_FB_WIDTH;
    int contentH = M11_FB_HEIGHT;
    int scaleMode = M11_SCALE_4X;
    int integerScaling = 0;
    int displayAspect = M11_DISPLAY_ASPECT_CONTENT;

    CHECK(M11_Render_ComputePresentationRect(windowW,
                                             windowH,
                                             contentW,
                                             contentH,
                                             scaleMode,
                                             integerScaling,
                                             displayAspect,
                                             &rectX, &rectY, &rectW, &rectH) == M11_RENDER_OK);
    CHECK_INT("4X overflow rect origin X", rectX, -240);
    CHECK_INT("4X overflow rect origin Y", rectY, -100);

    /* Window center (400, 300): inside the rect. */
    fbX = -1; fbY = -1;
    CHECK(M11_Render_MapPointToFramebuffer(400,
                                           300,
                                           windowW, windowH,
                                           contentW, contentH,
                                           scaleMode, integerScaling, displayAspect,
                                           &fbX, &fbY) == 1);
    CHECK_INT("4X overflow center fbX", fbX, 160);
    CHECK_INT("4X overflow center fbY", fbY, 100);

    /* Window top-left (0, 0): inside the rect (rectX=-240 < 0
     * < rectX+rectW=1040, rectY=-100 < 0 < rectY+rectH=700).
     * Source = (60, 25). */
    fbX = -1; fbY = -1;
    CHECK(M11_Render_MapPointToFramebuffer(0,
                                           0,
                                           windowW, windowH,
                                           contentW, contentH,
                                           scaleMode, integerScaling, displayAspect,
                                           &fbX, &fbY) == 1);
    CHECK_INT("4X overflow tl fbX", fbX, 60);
    CHECK_INT("4X overflow tl fbY", fbY, 25);

    /* Window bottom-right (799, 599): inside the rect (799 < 1040
     * and 599 < 700). Source = (259, 174). */
    fbX = -1; fbY = -1;
    CHECK(M11_Render_MapPointToFramebuffer(windowW - 1,
                                           windowH - 1,
                                           windowW, windowH,
                                           contentW, contentH,
                                           scaleMode, integerScaling, displayAspect,
                                           &fbX, &fbY) == 1);
    CHECK_INT("4X overflow br fbX", fbX, 259);
    CHECK_INT("4X overflow br fbY", fbY, 174);
}

/* Group G: deterministic remap contract. A given
 * (windowW, windowH, scaleMode, integerScaling, displayAspect,
 *  windowX, windowY) tuple must always map to the same
 * (fbX, fbY). Run the same point through the mapper ten times
 * and confirm the result is stable. A non-deterministic mapping
 * would silently misroute the command queue because the runtime
 * dispatch path reads the mapped source point multiple times per
 * tick (src/engine/main_loop_m11.c:180-200). */
static void test_deterministic_remap(void) {
    int i;
    struct {
        int windowW;
        int windowH;
        int scaleMode;
        int integerScaling;
        int displayAspect;
        int windowX;
        int windowY;
    } samples[] = {
        {320, 200, M11_SCALE_1X, 0, M11_DISPLAY_ASPECT_CONTENT, 160, 100},
        {640, 400, M11_SCALE_2X, 0, M11_DISPLAY_ASPECT_CONTENT, 320, 200},
        {960, 600, M11_SCALE_3X, 0, M11_DISPLAY_ASPECT_CONTENT, 480, 300},
        {1920, 1080, M11_SCALE_4X, 0, M11_DISPLAY_ASPECT_CONTENT, 960, 540},
        {1280, 720, M11_SCALE_2X, 1, M11_DISPLAY_ASPECT_16_9, 640, 360},
    };
    int n = (int)(sizeof(samples) / sizeof(samples[0]));

    for (i = 0; i < n; ++i) {
        int j;
        int x0 = -1, y0 = -1;
        int xN = -1, yN = -1;

        for (j = 0; j < 10; ++j) {
            xN = samples[i].windowX;
            yN = samples[i].windowY;
            CHECK(M11_Render_MapPointToFramebuffer(xN,
                                                   yN,
                                                   samples[i].windowW,
                                                   samples[i].windowH,
                                                   M11_FB_WIDTH,
                                                   M11_FB_HEIGHT,
                                                   samples[i].scaleMode,
                                                   samples[i].integerScaling,
                                                   samples[i].displayAspect,
                                                   &xN,
                                                   &yN) == 1);
            if (j == 0) {
                x0 = xN;
                y0 = yN;
            } else {
                CHECK_INT("deterministic remap X", xN, x0);
                CHECK_INT("deterministic remap Y", yN, y0);
            }
        }
    }
}

/* Group H: return-value contract for out-of-rect and out-of-range
 * inputs. M11_Render_MapPointToFramebuffer must return 0 (and
 * leave the out slots at their caller-supplied values) for:
 *   (a) window coordinates outside the rect (top/bottom/left/right
 *       letterbox edges),
 *   (b) NULL out-pointer arguments,
 *   (c) contentW <= 0 or contentH <= 0,
 *   (d) an unknown scale mode that fails the M11_Render_ComputePresentationRect
 *       validation guard.
 * These match the "out-of-rect rejection" contract that
 * `check_map_point_rejection_invariants` and
 * `check_arg_validation_invariants` pin for M11_SCALE_FIT; this
 * group locks the same contract for the integer fixed-scale modes
 * so a future refactor that special-cases them is caught. */
static void test_scale_mode_rejection_contract(void) {
    int fbX = -123;
    int fbY = -456;
    int rectX = -1, rectY = -1, rectW = -1, rectH = -1;

    /* (a) Window top-left outside a 2X rect at 1920x1080. */
    CHECK(M11_Render_ComputePresentationRect(1920,
                                             1080,
                                             M11_FB_WIDTH,
                                             M11_FB_HEIGHT,
                                             M11_SCALE_2X,
                                             0,
                                             M11_DISPLAY_ASPECT_CONTENT,
                                             &rectX, &rectY, &rectW, &rectH) == M11_RENDER_OK);
    fbX = -123; fbY = -456;
    CHECK(M11_Render_MapPointToFramebuffer(0,
                                           0,
                                           1920, 1080,
                                           M11_FB_WIDTH, M11_FB_HEIGHT,
                                           M11_SCALE_2X, 0,
                                           M11_DISPLAY_ASPECT_CONTENT,
                                           &fbX, &fbY) == 0);
    CHECK_INT("2X letterbox preserves outFbX", fbX, -123);
    CHECK_INT("2X letterbox preserves outFbY", fbY, -456);

    /* (b) NULL out-pointer for fbX. */
    fbY = -456;
    CHECK(M11_Render_MapPointToFramebuffer(rectX + rectW / 2,
                                           rectY + rectH / 2,
                                           1920, 1080,
                                           M11_FB_WIDTH, M11_FB_HEIGHT,
                                           M11_SCALE_2X, 0,
                                           M11_DISPLAY_ASPECT_CONTENT,
                                           NULL,
                                           &fbY) == 0);
    CHECK_INT("2X NULL fbX preserves outFbY", fbY, -456);

    /* (b) NULL out-pointer for fbY. */
    fbX = -123;
    CHECK(M11_Render_MapPointToFramebuffer(rectX + rectW / 2,
                                           rectY + rectH / 2,
                                           1920, 1080,
                                           M11_FB_WIDTH, M11_FB_HEIGHT,
                                           M11_SCALE_2X, 0,
                                           M11_DISPLAY_ASPECT_CONTENT,
                                           &fbX,
                                           NULL) == 0);
    CHECK_INT("2X NULL fbY preserves outFbX", fbX, -123);

    /* (c) contentW <= 0: mapper must return 0. */
    fbX = -123; fbY = -456;
    CHECK(M11_Render_MapPointToFramebuffer(960,
                                           540,
                                           1920, 1080,
                                           0, 200,
                                           M11_SCALE_2X, 0,
                                           M11_DISPLAY_ASPECT_CONTENT,
                                           &fbX, &fbY) == 0);
    CHECK_INT("2X contentW<=0 preserves outFbX", fbX, -123);
    CHECK_INT("2X contentW<=0 preserves outFbY", fbY, -456);

    /* (d) Unknown scale mode (M11_SCALE_STRETCH + 1): mapper must
     * return 0 via the validation guard, and the out slots stay at
     * the caller-supplied sentinel. */
    fbX = -123; fbY = -456;
    CHECK(M11_Render_MapPointToFramebuffer(960,
                                           540,
                                           1920, 1080,
                                           M11_FB_WIDTH, M11_FB_HEIGHT,
                                           M11_SCALE_STRETCH + 1, 0,
                                           M11_DISPLAY_ASPECT_CONTENT,
                                           &fbX, &fbY) == 0);
    CHECK_INT("unknown scale preserves outFbX", fbX, -123);
    CHECK_INT("unknown scale preserves outFbY", fbY, -456);
}

/* Group I: integer fixed-scale rect size is independent of the
 * display aspect mode. The centered formula in
 * M11_Render_ComputePresentationRect for M11_SCALE_1X..M11_SCALE_4X
 * only uses contentW/contentH and never reads the aspect mode, so
 * a 1280x800 window with M11_SCALE_4X produces the same rect
 * under all three aspect modes (4:3, 16:9, content). This pins
 * the "fixed-scale ignores aspect" contract for content that the
 * v2 presentation runtime reads at scale-1/2/3/4. */
static void test_scale_rect_independent_of_aspect(void) {
    int aspects[] = {
        M11_DISPLAY_ASPECT_4_3,
        M11_DISPLAY_ASPECT_16_9,
        M11_DISPLAY_ASPECT_CONTENT,
    };
    int scales[] = {
        M11_SCALE_1X,
        M11_SCALE_2X,
        M11_SCALE_3X,
        M11_SCALE_4X,
    };
    int ai, si;

    for (si = 0; si < (int)(sizeof(scales) / sizeof(scales[0])); ++si) {
        int baselineX = -1, baselineY = -1, baselineW = -1, baselineH = -1;
        for (ai = 0; ai < (int)(sizeof(aspects) / sizeof(aspects[0])); ++ai) {
            int x = -1, y = -1, w = -1, h = -1;
            int factor = scales[si] + 1;
            int expectedW = M11_FB_WIDTH * factor;
            int expectedH = M11_FB_HEIGHT * factor;

            CHECK(M11_Render_ComputePresentationRect(1280,
                                                     800,
                                                     M11_FB_WIDTH,
                                                     M11_FB_HEIGHT,
                                                     scales[si],
                                                     0,
                                                     aspects[ai],
                                                     &x, &y, &w, &h) == M11_RENDER_OK);
            CHECK_INT("aspect-independent width", w, expectedW);
            CHECK_INT("aspect-independent height", h, expectedH);
            CHECK_INT("aspect-independent origin X", x, (1280 - expectedW) / 2);
            CHECK_INT("aspect-independent origin Y", y, (800 - expectedH) / 2);

            if (ai == 0) {
                baselineX = x;
                baselineY = y;
                baselineW = w;
                baselineH = h;
            } else {
                CHECK_INT("aspect-independent X stable", x, baselineX);
                CHECK_INT("aspect-independent Y stable", y, baselineY);
                CHECK_INT("aspect-independent W stable", w, baselineW);
                CHECK_INT("aspect-independent H stable", h, baselineH);
            }
        }
    }
}

int main(void) {
    printf("=== M11 input-scale integer fixed-mode regression ===\n");
    printf("Source: src/engine/render_sdl_m11.c:330-340 (1X..4X branch),\n");
    printf("        src/engine/render_sdl_m11.c:362-364 (centered origin),\n");
    printf("        src/engine/render_sdl_m11.c:2027-2072 (MapPointToFramebuffer).\n\n");

    test_scale_1x_centered_rect();
    test_scale_2x_3x_4x_centered_rect();
    test_scale_4x_overflow_rect();
    test_scale_modes_ignore_aspect_and_integer();
    test_scale_2x_click_mapping_at_corners();
    test_scale_4x_overflow_click_mapping();
    test_deterministic_remap();
    test_scale_mode_rejection_contract();
    test_scale_rect_independent_of_aspect();

    printf("\nresult=%s\n", g_failures == 0 ? "PASS" : "FAIL");
    printf("summary=pass=%d fail=%d\n", g_passes, g_failures);
    return g_failures == 0 ? 0 : 1;
}

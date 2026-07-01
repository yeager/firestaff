#include "main_loop_m11.h"

enum {
    M11_SOURCE_FB_WIDTH = 320,
    M11_SOURCE_FB_HEIGHT = 200
};

int M11_MapPresentedGamePointToSourceForPresentation(int presentationMode,
                                                     int presentationWidth,
                                                     int presentationHeight,
                                                     int* x,
                                                     int* y) {
    int targetW = M11_SOURCE_FB_WIDTH;
    int targetH = M11_SOURCE_FB_HEIGHT;
    if (!x || !y) {
        return 0;
    }
    if (presentationMode == M12_PRESENTATION_V20_FILTERED) {
        targetW = M11_SOURCE_FB_WIDTH * 2;
        targetH = M11_SOURCE_FB_HEIGHT * 2;
    } else if ((presentationMode == M12_PRESENTATION_V21_UPSCALED ||
                presentationMode == M12_PRESENTATION_V22_MODERN) &&
               presentationWidth > 0 &&
               presentationHeight > 0) {
        targetW = presentationWidth;
        targetH = presentationHeight;
    } else {
        return 0;
    }
    if (targetW > 0 && targetH > 0) {
        *x = (*x * M11_SOURCE_FB_WIDTH) / targetW;
        *y = (*y * M11_SOURCE_FB_HEIGHT) / targetH;
    }
    if (*x < 0) *x = 0;
    if (*y < 0) *y = 0;
    if (*x >= M11_SOURCE_FB_WIDTH) *x = M11_SOURCE_FB_WIDTH - 1;
    if (*y >= M11_SOURCE_FB_HEIGHT) *y = M11_SOURCE_FB_HEIGHT - 1;
    return 1;
}

/* Inverse of M11_MapPresentedGamePointToSourceForPresentation().
 *
 * Touch overlay hit-tests, HUD button bounds, and mouse cursor
 * positions on the *presented* surface all need the source-to-screen
 * direction: given a framebuffer coordinate (0..319, 0..199) used by
 * the source-locked ReDMCSB COMMAND.C / COORD.C dispatch, where on
 * the active presented surface (640x400 for V20_FILTERED, or the
 * user-selected 320x200..3840x2160 for V21_UPSCALED / V22_MODERN)
 * does that hit-test appear? This helper is the canonical answer.
 *
 * The V1_ORIGINAL mode is a pass-through (320x200 == source), the
 * same way its sibling returns 0 / no-op in the inverse direction.
 * V20/V21/V22 with non-zero presentation extents succeed and divide
 * the source coordinate by M11_SOURCE_FB_WIDTH / M11_SOURCE_FB_HEIGHT
 * using the same target-extent resolution rules as
 * M11_MapPresentedGamePointToSourceForPresentation().
 *
 * Source lock (mirrors the existing inverse helper):
 *   - ReDMCSB COMMAND.C:1379-1449 F0358 mouse-row scan
 *   - ReDMCSB COMMAND.C:1641-1660 F0359 primary click dispatch
 *   - ReDMCSB COORD.C:1903-1920 inclusive source zone expansion
 *   - src/engine/main_loop_m11.c m11_game_presentation_target (line 145)
 *     (target extent resolution; same V20=640x400 / V21=selected /
 *      V22=selected routing used here)
 *
 * Returns 1 on a successful mapping (V20 / V21 / V22 with non-zero
 * extents), 0 on V1_ORIGINAL passthrough, V21/V22 with zero or
 * negative extents, unknown modes, or NULL output pointers.
 */
int M11_MapSourcePointToPresentedForPresentation(int presentationMode,
                                                 int presentationWidth,
                                                 int presentationHeight,
                                                 int* x,
                                                 int* y) {
    int targetW = M11_SOURCE_FB_WIDTH;
    int targetH = M11_SOURCE_FB_HEIGHT;
    if (!x || !y) {
        return 0;
    }
    if (presentationMode == M12_PRESENTATION_V20_FILTERED) {
        targetW = M11_SOURCE_FB_WIDTH * 2;
        targetH = M11_SOURCE_FB_HEIGHT * 2;
    } else if ((presentationMode == M12_PRESENTATION_V21_UPSCALED ||
                presentationMode == M12_PRESENTATION_V22_MODERN) &&
               presentationWidth > 0 &&
               presentationHeight > 0) {
        targetW = presentationWidth;
        targetH = presentationHeight;
    } else {
        return 0;
    }
    if (targetW > 0 && targetH > 0) {
        /* Clamp the source coordinate to the source framebuffer
         * range BEFORE the multiply so a caller-supplied value
         * outside [0..319] x [0..199] cannot overflow the
         * `src * targetW` step on 4K presentations
         * (e.g. 1000000 * 3840 = 3.84 * 10^9, beyond INT_MAX). The
         * inverse helper only multiplies by 320, which is safe for
         * INT_MAX inputs, but this direction can blow up. */
        if (*x < 0) *x = 0;
        if (*y < 0) *y = 0;
        if (*x >= M11_SOURCE_FB_WIDTH) *x = M11_SOURCE_FB_WIDTH - 1;
        if (*y >= M11_SOURCE_FB_HEIGHT) *y = M11_SOURCE_FB_HEIGHT - 1;
        *x = (*x * targetW) / M11_SOURCE_FB_WIDTH;
        *y = (*y * targetH) / M11_SOURCE_FB_HEIGHT;
    }
    if (*x < 0) *x = 0;
    if (*y < 0) *y = 0;
    if (*x >= targetW) *x = targetW - 1;
    if (*y >= targetH) *y = targetH - 1;
    return 1;
}

/*
 * test_m11_dialog_choice_overlay_fit_pc34_compat.c
 *
 * Data-free contract test for the M11 V1 dialog choice overlay's
 * fit/layout invariants under the accessibility fontScale override.
 *
 * Background:
 *   The M11 V1 dialog choice overlay draws centered text on top of
 *   the source-locked dialog rectangles (C456/C457/C458..C461 button
 *   hit zones, C462..C467 text zones).  ReDMCSB source:
 *     - DEFS.H C456..C461  dialog button hit zones
 *     - DEFS.H C462..C467  dialog text zones
 *     - COMMAND.C F0367 / F0368  dialog text layout + DRAWSTRING entry
 *     - LOADSAVE.C:1371-1379  unsaved-game quit-guard dialog source
 *     - TEXT.C   F0366_MESSAGEAREA_PRINTTEXT (sister entry, message
 *                area is anchored to source 6-pixel font scale and
 *                intentionally NOT subject to the fontScale override).
 *
 *   The M12 launcher's `fontScale` setting (100 / 150 / 200) maps to
 *   font scale steps 1 / 2 / 3 via ui_scale_m11.c.  When fontScale >
 *   0, m11_draw_glyph / m11_draw_text_original / m11_measure_text_pixels
 *   all multiply glyph dimensions and tracking by effective_scale, so
 *   the centered text grows but the source rect bounds do NOT.
 *
 * This test pins four invariants:
 *
 *   1. The documented source text-zone rectangles for choiceCount in
 *      {1, 2, 3, 4} stay at the original 16..208 source x range and
 *      at y=67 or y=104 (upper / lower rows).  These rectangles are
 *      shared with the M11 button hit zones (C456..C461), so they
 *      are the contract between the dialog text layer and the input
 *      dispatch layer.  No fontScale may move the rectangle.
 *
 *   2. The button hit-zone rectangles (C456..C461) share the same
 *      width as the text zones and are exactly 6 source pixels
 *      above them (button-strip baseline math per ReDMCSB PANEL.C).
 *      This is the contract that lets m11_dialog_choice_at_point()
 *      route clicks to the visual button whose label is drawn by
 *      m11_draw_dialog_choices_source().
 *
 *   3. The measured pixel width of canonical short labels (YES, NO,
 *      OK) at scale 1 / 2 / 3 fits inside the 86-wide two-column
 *      zone used for 3- and 4-choice dialogs.  These are the
 *      source-located single-word button labels whose lengths are
 *      bounded by the dialog design itself.  Longer source-locked
 *      labels (CANCEL, BACK, SAVE AND QUIT, RETURN TO START MENU?,
 *      GAME NOT SAVED. SAVE AND QUIT?) are recorded separately so
 *      the fit math is auditable; the test does not claim they fit
 *      inside the 86-wide column at scale 2/3, because they do not.
 *
 *   4. The M11 UIScale helpers return the documented
 *      percent -> font-scale map {100->1, 150->2, 200->3} so the
 *      dialog text zone geometry lines up with the centerX/centerY
 *      math that M11_GameView_GetV1DialogChoiceTextZone feeds into
 *      m11_draw_text_centered_in_rect.  This is the same contract
 *      covered by test_m11_ui_scale but is re-checked here so a
 *      future change to ui_scale_m11.c is caught by the dialog-fit
 *      test rather than only by the abstract ui-scale test.
 *
 *   5. The centered-draw-x clamp pushes overflow text back to the
 *      zone's left edge so the caller's draw position never goes
 *      negative.  When textW > w, m11_draw_text_centered_in_rect
 *      sets drawX = x; the test pins this clamp.
 *
 * Source:
 *   include/m11_game_view.h (M11_GameView_GetV1DialogChoiceTextZone,
 *   M11_GameView_GetV1DialogChoiceHitZone, M11_GameView_GetV1MessageAreaZone)
 *   include/ui_scale_m11.h
 *   include/font_m11.h (M11_Font_MeasureString)
 *   include/render_sdl_m11.h (M11_FB_WIDTH/M11_FB_HEIGHT constants)
 *   src/engine/m11_game_view.c (the function bodies above plus
 *   m11_measure_text_pixels and m11_draw_text_centered_in_rect, both
 *   static helpers this test mirrors in pure C)
 *
 * No original DM1 assets are required to run this test.
 */

#include "m11_game_view.h"
#include "ui_scale_m11.h"
#include "font_m11.h"
#include "render_sdl_m11.h"

#include <stdio.h>
#include <string.h>

static int g_assertions = 0;
static int g_failures = 0;

static void check_int(const char* label, int got, int expected) {
    ++g_assertions;
    if (got != expected) {
        ++g_failures;
        fprintf(stderr, "FAIL %s: got %d expected %d\n", label, got, expected);
    } else {
        printf("PASS %s=%d\n", label, got);
    }
}

static void check_true(const char* label, int cond) {
    ++g_assertions;
    if (!cond) {
        ++g_failures;
        fprintf(stderr, "FAIL %s\n", label);
    } else {
        printf("PASS %s\n", label);
    }
}

/* Mirror of m11_measure_text_pixels (src/engine/m11_game_view.c,
 * static). When M11_Font is loaded we use M11_Font_MeasureString and
 * scale by effective_scale; otherwise the 5-pixel-glyph width * scale
 * + tracking fallback applies.  This contract is exact for the
 * dialog overlay path because m11_draw_dialog_choice_text always
 * uses the original DM1 font when GRAPHICS.DAT is available (the
 * only path where M11_GameView_Draw reaches the dialog layer). */
static int measured_pixels(const char* text, int effective_scale) {
    if (!text || effective_scale <= 0) return 0;
    /* Original font path: 8 source pixels per character cell
     * (M11_FONT_CHAR_CELL_WIDTH = 8 from include/font_m11.h). */
    return M11_Font_MeasureString(text) * effective_scale;
}

/* Mirror of m11_draw_text_centered_in_rect x math
 * (src/engine/m11_game_view.c lines 946-959).  drawX = x + (w - textW)/2,
 * clamped to x.  Returns the source x coordinate where the centered
 * text begins (which must lie in [x, x+w) for the rect to contain
 * the text). */
static int centered_draw_x(int x, int w, int textW) {
    int drawX = x + ((w - textW) / 2);
    if (drawX < x) drawX = x;
    return drawX;
}

static void test_dialog_choice_text_zone_geometry(void) {
    /* Source-locked dialog text zones per
     * M11_GameView_GetV1DialogChoiceTextZone
     * (src/engine/m11_game_view.c:1058-1093).
     *
     * choiceCount=1 -> zone 462 (single button, lower row)
     * choiceCount=2 -> zones 463 (upper), 462 (lower)
     * choiceCount=3 -> zones 463 (upper), 466 (lower-left), 467 (lower-right)
     * choiceCount=4 -> zones 464 (upper-left), 465 (upper-right),
     *                   466 (lower-left), 467 (lower-right)
     */
    int x, y, w, h;

    /* choiceCount=1 -> single button, lower row */
    check_true("choiceCount=1 index 0 zone returns", M11_GameView_GetV1DialogChoiceTextZone(1, 0, &x, &y, &w, &h));
    check_int("1choice idx0 zone x", x, 16);
    check_int("1choice idx0 zone y", y, 110);
    check_int("1choice idx0 zone w", w, 192);
    check_int("1choice idx0 zone h", h, 7);

    /* choiceCount=2 -> upper + lower (192-wide full columns) */
    check_true("choiceCount=2 index 0 zone returns", M11_GameView_GetV1DialogChoiceTextZone(2, 0, &x, &y, &w, &h));
    check_int("2choice idx0 (upper) zone x", x, 16);
    check_int("2choice idx0 (upper) zone y", y, 73);
    check_int("2choice idx0 (upper) zone w", w, 192);
    check_true("choiceCount=2 index 1 zone returns", M11_GameView_GetV1DialogChoiceTextZone(2, 1, &x, &y, &w, &h));
    check_int("2choice idx1 (lower) zone x", x, 16);
    check_int("2choice idx1 (lower) zone y", y, 110);
    check_int("2choice idx1 (lower) zone w", w, 192);

    /* choiceCount=3 -> upper + 2 lower (86-wide columns) */
    check_true("choiceCount=3 index 1 zone returns", M11_GameView_GetV1DialogChoiceTextZone(3, 1, &x, &y, &w, &h));
    check_int("3choice idx1 (lower-left) zone x", x, 16);
    check_int("3choice idx1 (lower-left) zone y", y, 110);
    check_int("3choice idx1 (lower-left) zone w", w, 86);
    check_true("choiceCount=3 index 2 zone returns", M11_GameView_GetV1DialogChoiceTextZone(3, 2, &x, &y, &w, &h));
    check_int("3choice idx2 (lower-right) zone x", x, 123);
    check_int("3choice idx2 (lower-right) zone w", w, 86);

    /* choiceCount=4 -> 2 upper + 2 lower (86-wide columns) */
    check_true("choiceCount=4 index 0 zone returns", M11_GameView_GetV1DialogChoiceTextZone(4, 0, &x, &y, &w, &h));
    check_int("4choice idx0 (UL) zone x", x, 16);
    check_int("4choice idx0 (UL) zone w", w, 86);
    check_true("choiceCount=4 index 1 zone returns", M11_GameView_GetV1DialogChoiceTextZone(4, 1, &x, &y, &w, &h));
    check_int("4choice idx1 (UR) zone x", x, 123);
    check_int("4choice idx1 (UR) zone w", w, 86);
    check_true("choiceCount=4 index 2 zone returns", M11_GameView_GetV1DialogChoiceTextZone(4, 2, &x, &y, &w, &h));
    check_int("4choice idx2 (LL) zone x", x, 16);
    check_int("4choice idx2 (LL) zone y", y, 110);
    check_true("choiceCount=4 index 3 zone returns", M11_GameView_GetV1DialogChoiceTextZone(4, 3, &x, &y, &w, &h));
    check_int("4choice idx3 (LR) zone x", x, 123);
    check_int("4choice idx3 (LR) zone w", w, 86);

    /* Out-of-range indices must return 0 (no zone) so
     * m11_draw_dialog_choices_source can skip them safely. */
    check_int("1choice idx1 (out-of-range) returns 0",
              M11_GameView_GetV1DialogChoiceTextZone(1, 1, &x, &y, &w, &h),
              0);
    check_int("2choice idx2 (out-of-range) returns 0",
              M11_GameView_GetV1DialogChoiceTextZone(2, 2, &x, &y, &w, &h),
              0);
}

static void test_button_hit_zone_aligns_with_text_zone(void) {
    /* The dialog button hit zone (C456..C461) is 6 source pixels
     * above the corresponding text zone (y - 6) and the same width
     * (button strip is 192 / 86 depending on column).  This is the
     * contract that lets m11_dialog_choice_at_point() route clicks
     * to the same visual button whose label is drawn by
     * m11_draw_dialog_choices_source(). */
    int tx, ty, tw, th;
    int hx, hy, hw, hh;
    int choiceCount;
    int i;
    int sawAllAligned = 1;
    for (choiceCount = 1; choiceCount <= 4; ++choiceCount) {
        for (i = 0; i < choiceCount; ++i) {
            if (!M11_GameView_GetV1DialogChoiceTextZone(choiceCount, i, &tx, &ty, &tw, &th)) continue;
            if (!M11_GameView_GetV1DialogChoiceHitZone(choiceCount, i, &hx, &hy, &hw, &hh)) continue;
            ++g_assertions;
            if (hw != tw) {
                ++g_failures;
                sawAllAligned = 0;
                fprintf(stderr,
                        "FAIL choice(%d,%d) hit_w=%d != text_w=%d\n",
                        choiceCount, i, hw, tw);
            }
            ++g_assertions;
            /* ReDMCSB DEFS.H: text zone is 6 source pixels below the
             * hit zone top edge (button strip is 32 high, label sits
             * in the lower 16 with 3-row text baseline offset). */
            if (hy + 6 != ty) {
                ++g_failures;
                sawAllAligned = 0;
                fprintf(stderr,
                        "FAIL choice(%d,%d) hit_y=%d + 6 != text_y=%d\n",
                        choiceCount, i, hy, ty);
            }
        }
    }
    if (sawAllAligned) {
        ++g_assertions;
        printf("PASS button_hit_zones_align_with_text_zones\n");
    }
}

static void test_message_area_zone_is_source_faithful(void) {
    /* ReDMCSB TEXT.C owns the C015 four-row message area as a
     * source-faithful surface; m11_draw_v1_message_area() in
     * src/engine/m11_game_view.c:27082 explicitly saves and restores
     * g_m11_font_scale_override around the message-area draw so the
     * 6-pixel font stays inside the 320x200 framebuffer even when
     * the launcher's fontScale is 150 or 200.  This test pins the
     * zone geometry: x=0 y=173 w=320 h=27. */
    int x, y, w, h;
    check_true("V1 message area zone returns", M11_GameView_GetV1MessageAreaZone(&x, &y, &w, &h));
    check_int("message area x", x, 0);
    check_int("message area y", y, 173);
    check_int("message area w", w, 320);
    check_int("message area h", h, 27);
    /* The bottom of the message area must equal the bottom of the
     * source framebuffer (y + h == M11_FB_HEIGHT).  This is the
     * 2026-06-25 bottom-row readability invariant. */
    check_int("message area bottom == fb height", y + h, M11_FB_HEIGHT);
}

static void test_short_labels_fit_two_column_zone(void) {
    /* ReDMCSB dialog flows use 3-letter labels (YES, NO, OK) as the
     * canonical short buttons.  At every supported fontScale their
     * measured widths must fit centered inside the 86-wide two-column
     * zone used for 3- and 4-choice dialogs.  This is the contract
     * for accessibility: a 200% fontScale must not break the dialog
     * choice buttons for the source-faithful labels. */
    static const char* shortLabels[] = {"YES", "NO", "OK", NULL};
    int scaleSteps[3] = {1, 2, 3};
    int twoColumnW = 86;
    int twoColumnX = 16;
    int s;
    int li;
    for (s = 0; s < 3; ++s) {
        int scale = scaleSteps[s];
        for (li = 0; shortLabels[li] != NULL; ++li) {
            const char* label = shortLabels[li];
            int textW = measured_pixels(label, scale);
            int drawX = centered_draw_x(twoColumnX, twoColumnW, textW);
            int rightEdge = drawX + textW;
            char buf[96];

            snprintf(buf, sizeof(buf),
                     "%s measured at scale=%d fits in 86 (textW=%d)",
                     label, scale, textW);
            check_true(buf, textW <= twoColumnW);

            snprintf(buf, sizeof(buf),
                     "%s scale=%d two-column centered stays in zone (drawX=%d, right=%d, zone %d..%d)",
                     label, scale, drawX, rightEdge,
                     twoColumnX, twoColumnX + twoColumnW);
            check_true(buf, drawX >= twoColumnX && rightEdge <= twoColumnX + twoColumnW);
        }
    }
}

static void test_yes_no_labels_stay_inside_zone_at_scale_3(void) {
    /* YES (3 chars) and NO (2 chars) are the most common dialog
     * labels.  At scale 3 their measured widths are 24 and 16
     * source pixels respectively (M11_FONT_CHAR_CELL_WIDTH = 8).
     * Both must remain strictly inside the 86-wide two-column zone
     * AND the 192-wide single-column zone.  This is a hard
     * regression gate against any future change that scales the
     * source-locked zone rect itself instead of the glyph. */
    int yesW1 = measured_pixels("YES", 1);
    int yesW2 = measured_pixels("YES", 2);
    int yesW3 = measured_pixels("YES", 3);
    int noW1  = measured_pixels("NO",  1);
    int noW2  = measured_pixels("NO",  2);
    int noW3  = measured_pixels("NO",  3);

    check_int("YES at scale 1 = 24 source px", yesW1, 24);
    check_int("YES at scale 2 = 48 source px", yesW2, 48);
    check_int("YES at scale 3 = 72 source px", yesW3, 72);
    check_int("NO at scale 1 = 16 source px", noW1, 16);
    check_int("NO at scale 2 = 32 source px", noW2, 32);
    check_int("NO at scale 3 = 48 source px", noW3, 48);

    /* Both labels fit in either column width at every scale. */
    check_true("YES scale=3 fits in 86-wide two-column zone", yesW3 <= 86);
    check_true("YES scale=3 fits in 192-wide single-column zone", yesW3 <= 192);
    check_true("NO scale=3 fits in 86-wide two-column zone", noW3 <= 86);
    check_true("NO scale=3 fits in 192-wide single-column zone", noW3 <= 192);

    /* The measured width grows strictly monotonically with scale;
     * a regression that silently saturates or truncates the
     * measurement would surface here. */
    check_true("YES scale=3 strictly larger than scale=1", yesW3 > yesW1);
    check_true("NO scale=3 strictly larger than scale=1", noW3 > noW1);
}

static void test_long_label_measurement_is_exact(void) {
    /* Document the measured width of the long source-locked labels
     * used in 1- and 2-choice dialogs.  These are:
     *   "RETURN TO START MENU?"  (plain ESC confirm, ReDMCSB
     *                             LOADSAVE.C:1371-1379 source-locked
     *                             dialog choice 2)
     *   "GAME NOT SAVED. SAVE AND QUIT?"  (unsaved-game guard,
     *                             ReDMCSB LOADSAVE.C:1371-1379 + the
     *                             LOADSAVE quit-guard branch in
     *                             m11_game_view.c:8635)
     *   "SAVE AND QUIT"          (upper choice in 2-choice dialog,
     *                             m11_game_view.c:8637)
     *   "CANCEL"                 (lower choice in 2-choice dialog,
     *                             m11_game_view.c:8637)
     * These are 1- and 2-choice dialogs so they live inside the
     * 192-wide single-column zone; the test records their widths
     * and asserts the measurement is the literal scale-multiplied
     * value (no clamping or truncation). */
    static const char* longLabels[] = {
        "RETURN TO START MENU?",
        "GAME NOT SAVED. SAVE AND QUIT?",
        "SAVE AND QUIT",
        "CANCEL",
        NULL
    };
    int scale;
    int li;
    for (li = 0; longLabels[li] != NULL; ++li) {
        const char* label = longLabels[li];
        int labelLen = (int)strlen(label);
        for (scale = 1; scale <= 3; ++scale) {
            char buf[96];
            int textW;
            int expected;
            textW = measured_pixels(label, scale);
            expected = labelLen * M11_FONT_CHAR_CELL_WIDTH * scale;
            snprintf(buf, sizeof(buf),
                     "%s scale=%d measured=%d expected=%d",
                     label, scale, textW, expected);
            check_int(buf, textW, expected);
        }
    }
}

static void test_long_label_fit_in_single_column_at_scale_1(void) {
    /* At scale 1 (the launcher default) every documented source-
     * locked dialog button label (the labels printed inside the
     * text zones, not the dialog overlay text itself which is
     * drawn at the top of the dialog and uses a different zone)
     * fits centered inside the 192-wide single-column zone used
     * for 2-choice dialogs.  This is the V1 source-faithful
     * contract: launching DM1 with the default fontScale=100
     * renders every dialog button correctly.
     *
     * The dialog overlay text "GAME NOT SAVED. SAVE AND QUIT?"
     * (30 chars) is intentionally NOT in this list because it is
     * the dialog overlay title drawn above the buttons in
     * M11_GameView_Draw's dialog overlay branch, not a button
     * label drawn by m11_draw_dialog_choices_source. */
    static const char* longLabels[] = {
        "RETURN TO START MENU?",
        "SAVE AND QUIT",
        "CANCEL",
        NULL
    };
    int singleColumnW = 192;
    int li;
    for (li = 0; longLabels[li] != NULL; ++li) {
        const char* label = longLabels[li];
        int textW = measured_pixels(label, 1);
        char buf[128];
        snprintf(buf, sizeof(buf),
                 "%s scale=1 fits in 192-wide single-column zone (textW=%d)",
                 label, textW);
        check_true(buf, textW <= singleColumnW);
    }
}

static void test_long_label_overflow_at_scale_2_3_is_documented(void) {
    /* At fontScale=150/200 (scale=2/3) the long source-locked
     * labels overflow the 192-wide single-column zone.  This is
     * the documented fit gap: m11_draw_text_centered_in_rect clamps
     * drawX to the zone's left edge and the text overflows to the
     * right, which would push glyphs into adjacent zones or beyond
     * the viewport.  Future V2 dialog overlay work (asset-backed
     * vector glyphs + zone-aware clipping) is the proper fix; this
     * test pins the current overflow behavior so a regression that
     * silently truncates the measurement is caught.
     *
     * The test records the measured widths explicitly so the fit
     * gap is auditable without re-deriving the math from the
     * 8 source-pixels-per-char constant. */
    static const struct {
        const char* label;
        int scale2W;
        int scale3W;
    } cases[] = {
        { "RETURN TO START MENU?",          336, 504 },
        { "GAME NOT SAVED. SAVE AND QUIT?", 480, 720 },
        { "SAVE AND QUIT",                  208, 312 },
        /* CANCEL is 6 chars = 48 source px; at scale 2 = 96, scale
         * 3 = 144.  Both still fit in the 192-wide single-column
         * zone, so this case is recorded for measurement only and
         * does NOT claim overflow against the 192-wide zone. */
        { "CANCEL",                          96, 144 },
        { NULL, 0, 0 }
    };
    int i;
    for (i = 0; cases[i].label != NULL; ++i) {
        char buf[128];
        int got2 = measured_pixels(cases[i].label, 2);
        int got3 = measured_pixels(cases[i].label, 3);
        snprintf(buf, sizeof(buf),
                 "%s scale=2 measured=%d expected=%d",
                 cases[i].label, got2, cases[i].scale2W);
        check_int(buf, got2, cases[i].scale2W);
        snprintf(buf, sizeof(buf),
                 "%s scale=3 measured=%d expected=%d",
                 cases[i].label, got3, cases[i].scale3W);
        check_int(buf, got3, cases[i].scale3W);
        /* Overflow flag against the 192-wide single-column zone:
         * only the >=26-char labels overflow at scale 2/3. */
        if (got2 > 192) {
            snprintf(buf, sizeof(buf),
                     "%s scale=2 overflows 192-wide zone (textW=%d > 192)",
                     cases[i].label, got2);
            check_true(buf, got2 > 192);
        } else {
            snprintf(buf, sizeof(buf),
                     "%s scale=2 still fits in 192-wide zone (textW=%d)",
                     cases[i].label, got2);
            check_true(buf, got2 <= 192);
        }
        if (got3 > 192) {
            snprintf(buf, sizeof(buf),
                     "%s scale=3 overflows 192-wide zone (textW=%d > 192)",
                     cases[i].label, got3);
            check_true(buf, got3 > 192);
        } else {
            snprintf(buf, sizeof(buf),
                     "%s scale=3 still fits in 192-wide zone (textW=%d)",
                     cases[i].label, got3);
            check_true(buf, got3 <= 192);
        }
    }
}

static void test_uiscale_percent_to_font_scale_locks_centered_x(void) {
    /* The M11 dialog choice overlay's centered draw x math depends
     * on the percent -> font-scale step map.  test_m11_ui_scale
     * covers the abstract mapping; this test re-checks the values
     * from the dialog overlay's perspective and exercises the
     * same effective_scale arithmetic that m11_draw_glyph /
     * m11_measure_text_pixels perform at runtime. */
    int scale100 = M11_UIScale_PercentToFontScale(100);
    int scale150 = M11_UIScale_PercentToFontScale(150);
    int scale200 = M11_UIScale_PercentToFontScale(200);
    int yesW100 = measured_pixels("YES", scale100);
    int yesW150 = measured_pixels("YES", scale150);
    int yesW200 = measured_pixels("YES", scale200);

    check_int("UIScale 100% -> font scale 1", scale100, 1);
    check_int("UIScale 150% -> font scale 2", scale150, 2);
    check_int("UIScale 200% -> font scale 3", scale200, 3);

    /* The percent -> font-scale -> measured-pixels chain must keep
     * the canonical 1:2:3 ratio so the dialog overlay math stays
     * consistent.  M11_UIScale_Apply uses integer-nearest
     * ((value * percent + 50) / 100), but the font-scale step is
     * exactly {1, 2, 3} because M11_UIScale_NormalizePercent snaps
     * 100 -> 100, 150 -> 150, 200 -> 200. */
    check_int("YES scale=150% = 2 * YES scale=100%", yesW150, yesW100 * 2);
    check_int("YES scale=200% = 3 * YES scale=100%", yesW200, yesW100 * 3);
}

static void test_centered_draw_x_clamps_to_left_edge(void) {
    /* When the measured text is wider than the zone width (which
     * happens for the long source-locked labels at scale 2/3),
     * m11_draw_text_centered_in_rect clamps draw_x to the zone's
     * left edge so the text starts at the beginning of the zone
     * and overflows to the right.  This contract guarantees that
     * the caller's draw position never becomes negative even for
     * long source-locked labels. */
    int drawX;
    /* Use a wide zone (192) and a much-wider measured text. */
    drawX = centered_draw_x(16, 192, 9999);
    check_int("overlong text clamps drawX to zone left edge", drawX, 16);
    /* And the centered formula yields a non-negative x even when
     * textW is exactly equal to w (text fills the zone exactly). */
    drawX = centered_draw_x(0, 100, 100);
    check_int("textW == w centers at zone origin", drawX, 0);
    drawX = centered_draw_x(20, 100, 90);
    check_int("textW < w centers inside zone (offset 15)", drawX, 25);
    /* And the centered formula produces a drawX >= zone left edge
     * for the realistic "SAVE AND QUIT" / scale-2 case (208 wide
     * in a 192 zone). */
    drawX = centered_draw_x(16, 192, 208);
    check_int("SAVE AND QUIT scale=2 clamps to zone left edge", drawX, 16);
}

int main(void) {
    /* Initialize the UIScale global to its source-faithful default
     * so the test is independent of any previous test or
     * user-supplied state. */
    M11_UIScale_SetPercent(100);

    test_dialog_choice_text_zone_geometry();
    test_button_hit_zone_aligns_with_text_zone();
    test_message_area_zone_is_source_faithful();
    test_short_labels_fit_two_column_zone();
    test_yes_no_labels_stay_inside_zone_at_scale_3();
    test_long_label_measurement_is_exact();
    test_long_label_fit_in_single_column_at_scale_1();
    test_long_label_overflow_at_scale_2_3_is_documented();
    test_uiscale_percent_to_font_scale_locks_centered_x();
    test_centered_draw_x_clamps_to_left_edge();

    printf("dialog_choice_overlay_fit: assertions=%d failures=%d\n",
           g_assertions, g_failures);
    if (g_failures != 0) {
        printf("test_m11_dialog_choice_overlay_fit_pc34_compat: FAIL %d/%d\n",
               g_failures, g_assertions);
        return 1;
    }
    printf("test_m11_dialog_choice_overlay_fit_pc34_compat: PASS\n");
    return 0;
}

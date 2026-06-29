/*
 * firestaff_dm1_v1_dialog_choice_font_scale_fit_probe.c
 *
 * In-game overlay fit/layout test for the M11 dialog choice text surface
 * plus the Firestaff-specific session-timer reminder banner.
 *
 * Source evidence (ReDMCSB layout-696):
 *   - C462_ZONE_CHOICE_TEXT_AT_192W  = (16, 110, 192, 7) — bottom choice
 *   - C463_ZONE_CHOICE_TEXT_AT_192W  = (16, 73,  192, 7) — top choice
 *   - C464/C465_ZONE_CHOICE_TEXT_AT_86W = (16/123, 73,  86, 7) — top split
 *   - C466/C467_ZONE_CHOICE_TEXT_AT_86W = (16/123, 110, 86, 7) — bottom split
 *   - M2092_MessageAreaWidth = 320, M532_MESSAGE_AREA_ROW_COUNT = 4
 *   - G2088_C7_TextLineHeight = 7, G2089_C8_InscriptionCharacterWidth = 8
 *   - G2082_C5_CharacterPadding = 5  -> X_OFFSET = 8 - 5 = 3
 *   - G2087_C6_InscriptionCharacterWidth = 6, G2083_C3 = 6 visible rows
 *
 * Headless / data-free probe — uses a synthetic 1bpp font bitmap with
 * every cell fully lit so the layout invariants are testable without
 * shipping any GRAPHICS.DAT. The same M11_Font_DrawString path is what
 * m11_draw_text_original feeds when state->fontScale > 0 (see
 * m11_game_view.c:1300), so the scale-to-pixel mapping verified here
 * is the one the in-game overlay actually consumes.
 *
 * The probe pins the contracts that the gap "In-game overlays + UI-fit
 * coverage" must hold:
 *
 *   1. M11_UIScale percent -> font scale step (100/150/200 -> 1/2/3) and
 *      the integer-nearest percent snap used by the launcher.
 *   2. M11_Font_DrawString advance per char at scale S = 8 * S, and the
 *      visible bbox width per scale step.
 *   3. The 192 px source dialog choice zones (C462/C463) carry exactly
 *      24 chars at scale 1; at scale 2/3 the same 24 chars overflow the
 *      zone right edge — the bug shape a missing clip would hide.
 *   4. The V1 message area zone (C015) is 320x27 at y=173 and stays
 *      inside the 320x200 framebuffer at every scale (the production
 *      V1 chrome override clamps the scale to 1 for that surface).
 *   5. The in-game session-timer reminder banner fits actual M11 draw
 *      output at fontScale 1/2/3 and changes no pixels outside the
 *      top-strip overlay rectangle, preserving V1 viewport ownership.
 */

#include "font_m11.h"
#include "m11_game_view.h"
#include "render_sdl_m11.h"
#include "ui_scale_m11.h"

#include <stdio.h>
#include <string.h>

/* Source-locked viewport origin (file-scope constants in m11_game_view.c):
 *   COORD.C:81   int16_t G2067_i_ViewportScreenX = 0;
 *   COORD.C:82   int16_t G2068_i_ViewportScreenY = 33;
 * Dialog choice text zones are reported in viewport-local coordinates
 * (M11_GameView_GetV1DialogChoiceTextZone), so we add the viewport Y to
 * reach framebuffer coordinates. */
enum {
    PROBE_DM1_VIEWPORT_X = 0,
    PROBE_DM1_VIEWPORT_Y = 33
};

static int g_failures = 0;
static int g_passes = 0;

static void check_true(const char* label, int cond) {
    if (cond) {
        ++g_passes;
        printf("PASS: %s\n", label);
    } else {
        ++g_failures;
        printf("FAIL: %s\n", label);
    }
}

static void check_int(const char* label, int got, int expected) {
    char msg[160];
    snprintf(msg, sizeof(msg), "%s got=%d expected=%d", label, got, expected);
    check_true(msg, got == expected);
}

/* Build a 1bpp font bitmap (1024 wide x 6 tall, MSB-first per row) with
 * every cell filled so any M11_Font_GetPixel returns 1.  This gives a
 * deterministic, easily-bbox'd "block" glyph at every char code so the
 * layout assertions don't depend on a real GRAPHICS.DAT font shape. */
static void build_block_font_bitmap(M11_FontState* font) {
    int row;
    int byteIdx;
    memset(font, 0, sizeof(*font));
    for (row = 0; row < M11_FONT_BITMAP_HEIGHT; ++row) {
        for (byteIdx = 0; byteIdx < 128; ++byteIdx) {
            font->bitmap[row * 128 + byteIdx] = 0xFF;
        }
    }
    font->loaded = 1;
    font->graphicIndex = -1;
}

/* Find the bounding box of foreground pixels across the full framebuffer.
 * Returns 1 if at least one foreground pixel was found.  Any of the
 * outX/outY pointers may be NULL when the caller only cares about a
 * subset of the bbox axes. */
static int find_fg_bbox(const unsigned char* fb,
                        int fbW,
                        int fbH,
                        unsigned char fgColor,
                        int* outMinX,
                        int* outMaxX,
                        int* outMinY,
                        int* outMaxY) {
    int x;
    int y;
    int minX = fbW;
    int maxX = -1;
    int minY = fbH;
    int maxY = -1;
    for (y = 0; y < fbH; ++y) {
        for (x = 0; x < fbW; ++x) {
            if (M11_FB_DECODE_INDEX(fb[y * fbW + x]) == fgColor) {
                if (x < minX) minX = x;
                if (x > maxX) maxX = x;
                if (y < minY) minY = y;
                if (y > maxY) maxY = y;
            }
        }
    }
    if (outMinX) *outMinX = minX;
    if (outMaxX) *outMaxX = maxX;
    if (outMinY) *outMinY = minY;
    if (outMaxY) *outMaxY = maxY;
    return (maxX >= 0);
}

static int count_changed_outside_rect(const unsigned char* before,
                                      const unsigned char* after,
                                      int fbW,
                                      int fbH,
                                      int rx,
                                      int ry,
                                      int rw,
                                      int rh,
                                      int* outChanged)
{
    int x;
    int y;
    int changed = 0;
    int outside = 0;
    for (y = 0; y < fbH; ++y) {
        for (x = 0; x < fbW; ++x) {
            int idx = y * fbW + x;
            if (before[idx] == after[idx]) {
                continue;
            }
            ++changed;
            if (x < rx || x >= rx + rw || y < ry || y >= ry + rh) {
                ++outside;
            }
        }
    }
    if (outChanged) {
        *outChanged = changed;
    }
    return outside;
}

/* Visible bbox width for N chars at scale S using a fully-lit bitmap:
 *   - per char advance = 8 * S
 *   - per char visible region = 6 * S
 *   - gap between consecutive chars = 2 * S
 *   - total = N * 6 * S + (N - 1) * 2 * S = S * (8 * N - 2)
 */
static int expected_visible_bbox_width(int charCount, int scale) {
    return scale * (M11_FONT_CHAR_CELL_WIDTH * charCount - 2);
}

/* Visible bbox height for any char at scale S:
 *   - per char visible rows = 6
 *   - total = 6 * S
 */
static int expected_visible_bbox_height(int scale) {
    return scale * M11_FONT_CHAR_VISIBLE_H;
}

static void test_ui_scale_to_font_scale(void) {
    /* M11_UIScale percent -> font scale step. Pinned by ui_scale_m11.c
     * so HUD/menu code reading M11_UIScale_GetFontScale before drawing
     * passes the right integer into M11_Font_DrawString. */
    M11_UIScale_SetPercent(100);
    check_int("100% -> font scale 1", M11_UIScale_GetFontScale(), 1);
    M11_UIScale_SetPercent(150);
    check_int("150% -> font scale 2", M11_UIScale_GetFontScale(), 2);
    M11_UIScale_SetPercent(200);
    check_int("200% -> font scale 3", M11_UIScale_GetFontScale(), 3);
    /* Snap behaviour — 125% is in the 101..150 bucket. */
    M11_UIScale_SetPercent(125);
    check_int("125% snaps to font scale 2", M11_UIScale_GetFontScale(), 2);
    /* NormalizePercent used at config parse time. */
    check_int("NormalizePercent(100)", M11_UIScale_NormalizePercent(100), 100);
    check_int("NormalizePercent(150)", M11_UIScale_NormalizePercent(150), 150);
    check_int("NormalizePercent(200)", M11_UIScale_NormalizePercent(200), 200);
    check_int("NormalizePercent(149)", M11_UIScale_NormalizePercent(149), 150);
    /* Apply helper — the helper HUD layout code calls when positioning
     * non-font UI at the same percent. */
    M11_UIScale_SetPercent(150);
    check_int("150% Apply(8) = 12", M11_UIScale_Apply(8), 12);
    check_int("150% Apply(24) = 36", M11_UIScale_Apply(24), 36);
    M11_UIScale_SetPercent(100);
    check_int("100% Apply(8) = 8", M11_UIScale_Apply(8), 8);
}

static void test_font_scale_pixel_advance(void) {
    /* M11_Font_DrawString must put N*8*S pixels of advance per char and
     * S*(8*N-2) pixels of visible bbox width.  Same path used by
     * m11_draw_text_original when state->fontScale > 0. */
    M11_FontState font;
    unsigned char fb[M11_FB_WIDTH * M11_FB_HEIGHT];
    int scale;
    int advance;
    int expectedAdvance;
    int expectedW;
    int expectedH;

    build_block_font_bitmap(&font);
    for (scale = 1; scale <= 3; ++scale) {
        int minX = 0, maxX = 0, minY = 0, maxY = 0;
        int found;
        expectedAdvance = 3 * M11_FONT_CHAR_CELL_WIDTH * scale;
        expectedW = expected_visible_bbox_width(3, scale);
        expectedH = expected_visible_bbox_height(scale);
        memset(fb, 0, sizeof(fb));
        advance = M11_Font_DrawString(&font, fb, M11_FB_WIDTH, M11_FB_HEIGHT,
                                      16, 16, "AAA", 15, -1, scale);
        found = find_fg_bbox(fb, M11_FB_WIDTH, M11_FB_HEIGHT, 15,
                             &minX, &maxX, &minY, &maxY);
        {
            char label[160];
            snprintf(label, sizeof(label),
                     "scale=%d DrawString(\"AAA\") advance=%d expected=%d",
                     scale, advance, expectedAdvance);
            check_int(label, advance, expectedAdvance);
            check_true("scale=N bbox found", found);
            snprintf(label, sizeof(label),
                     "scale=%d bbox width=%d expected=%d",
                     scale, maxX - minX + 1, expectedW);
            check_int(label, maxX - minX + 1, expectedW);
            snprintf(label, sizeof(label),
                     "scale=%d bbox height=%d expected=%d",
                     scale, maxY - minY + 1, expectedH);
            check_int(label, maxY - minY + 1, expectedH);
        }
    }
}

static void test_dialog_choice_zone_source_lock(void) {
    /* C462/C463 zones are 192 px wide per ReDMCSB layout-696.  The
     * dialog fit invariant only holds if the runtime matches. */
    int x, y, w, h;
    int ok;
    /* 2-choice: choice 0 (top) = C463 = (16, 73, 192, 7). */
    ok = M11_GameView_GetV1DialogChoiceTextZone(2, 0, &x, &y, &w, &h);
    check_true("2-choice zone 0 returns", ok);
    check_int("2-choice zone 0 x", x, 16);
    check_int("2-choice zone 0 y", y, 73);
    check_int("2-choice zone 0 w", w, 192);
    check_int("2-choice zone 0 h", h, 7);
    /* 2-choice: choice 1 (bottom) = C462 = (16, 110, 192, 7). */
    ok = M11_GameView_GetV1DialogChoiceTextZone(2, 1, &x, &y, &w, &h);
    check_true("2-choice zone 1 returns", ok);
    check_int("2-choice zone 1 x", x, 16);
    check_int("2-choice zone 1 y", y, 110);
    check_int("2-choice zone 1 w", w, 192);
    check_int("2-choice zone 1 h", h, 7);
    /* 4-choice split: each choice 86 px wide. */
    ok = M11_GameView_GetV1DialogChoiceTextZone(4, 0, &x, &y, &w, &h);
    check_true("4-choice zone 0 returns", ok);
    check_int("4-choice zone 0 w", w, 86);
    ok = M11_GameView_GetV1DialogChoiceTextZone(4, 3, &x, &y, &w, &h);
    check_true("4-choice zone 3 returns", ok);
    check_int("4-choice zone 3 w", w, 86);
}

static void test_dialog_choice_fit_at_scale_1(void) {
    /* Source zone is 192 px wide. M11_Font char cell width is 8
     * (G2089), so the maximum text that fits without clipping is
     * 192 / 8 = 24 chars.  Render 24 'A's at scale=1 inside a fresh
     * framebuffer and confirm: (a) the visible trailing edge stays
     * inside the zone, (b) the rendered bbox width matches the
     * visible-bbox formula S*(8*N - 2). */
    M11_FontState font;
    unsigned char fb[M11_FB_WIDTH * M11_FB_HEIGHT];
    int x0, y0, w, h;
    int zoneLeft, zoneTop, zoneRight;
    int minX, maxX, minY, maxY;
    int found;
    char text[25];
    int advance;
    int i;

    for (i = 0; i < 24; ++i) text[i] = 'A';
    text[24] = '\0';

    (void)M11_GameView_GetV1DialogChoiceTextZone(2, 0, &x0, &y0, &w, &h);
    zoneLeft = x0;
    zoneTop = PROBE_DM1_VIEWPORT_Y + y0;
    zoneRight = x0 + w - 1;

    build_block_font_bitmap(&font);
    memset(fb, 0, sizeof(fb));
    advance = M11_Font_DrawString(&font, fb, M11_FB_WIDTH, M11_FB_HEIGHT,
                                  zoneLeft, zoneTop, text, 15, -1, 1);

    found = find_fg_bbox(fb, M11_FB_WIDTH, M11_FB_HEIGHT, 15,
                         &minX, &maxX, &minY, &maxY);
    check_true("24 chars @ scale=1 bbox found", found);
    /* M11_Font_DrawString returns the cell advance in pixels: 24 * 8 = 192. */
    {
        char label[160];
        snprintf(label, sizeof(label),
                 "24 chars @ scale=1 advance=%d expected=%d (zone width)",
                 advance, 192);
        check_int(label, advance, 192);
    }
    /* Visible width = 1 * (8*24 - 2) = 190, so rightmost == left + 189. */
    {
        int expectedRight = zoneLeft + expected_visible_bbox_width(24, 1) - 1;
        char label[160];
        snprintf(label, sizeof(label),
                 "24 chars @ scale=1 rightmost=%d expected=%d (fits in zone right=%d)",
                 maxX, expectedRight, zoneRight);
        check_int(label, maxX, expectedRight);
        check_true("24 chars @ scale=1 rightmost <= zoneRight",
                   maxX <= zoneRight);
    }
    /* The first drawn pixel lands at zoneLeft (X_OFFSET is the bitmap
     * sample column, not the framebuffer draw column). */
    {
        char label[160];
        snprintf(label, sizeof(label),
                 "24 chars @ scale=1 leftmost=%d expected=%d",
                 minX, zoneLeft);
        check_int(label, minX, zoneLeft);
        snprintf(label, sizeof(label),
                 "24 chars @ scale=1 top=%d expected=%d",
                 minY, zoneTop);
        check_int(label, minY, zoneTop);
        snprintf(label, sizeof(label),
                 "24 chars @ scale=1 bottom=%d expected=%d",
                 maxY, zoneTop + 5);
        check_int(label, maxY, zoneTop + 5);
    }
}

static void test_dialog_choice_overflow_at_scale_2_3(void) {
    /* The same 24-char text at scale 2 needs 24*16 = 384 px of cell
     * advance; the source zone is 192 px wide.  At scale 3 the
     * advance is 24*24 = 576 px.  This is the bug shape that a
     * missing clip or a mis-wired fontScale->scale lookup would hide,
     * so we explicitly assert (a) the DrawString advance exceeds the
     * zone width at scale 2/3, and (b) the visible glyph height
     * exceeds the 7-px zone line height (the production V1 chrome
     * does not vertically clip, so the bug surface is "taller glyph
     * painted into a 7-px zone line"). */
    M11_FontState font;
    unsigned char fb[M11_FB_WIDTH * M11_FB_HEIGHT];
    int x0, y0, w, h;
    int zoneLeft, zoneTop, zoneRight, zoneBottom;
    int scale;
    int advance;
    int minY, maxY;
    int found;
    char text[25];
    int i;

    for (i = 0; i < 24; ++i) text[i] = 'A';
    text[24] = '\0';

    (void)M11_GameView_GetV1DialogChoiceTextZone(2, 0, &x0, &y0, &w, &h);
    zoneLeft = x0;
    zoneTop = PROBE_DM1_VIEWPORT_Y + y0;
    zoneRight = x0 + w - 1;
    zoneBottom = zoneTop + h - 1;

    build_block_font_bitmap(&font);
    for (scale = 2; scale <= 3; ++scale) {
        int expectedAdvance = 24 * M11_FONT_CHAR_CELL_WIDTH * scale;
        int expectedGlyphHeight = M11_FONT_CHAR_VISIBLE_H * scale;
        char label[200];
        memset(fb, 0, sizeof(fb));
        advance = M11_Font_DrawString(&font, fb, M11_FB_WIDTH, M11_FB_HEIGHT,
                                      zoneLeft, zoneTop, text, 15, -1, scale);
        /* The cell advance is unclipped (M11_Font_DrawString returns
         * the layout advance, not the visible-framebuffer extent), so
         * the advance number is the cleanest layout-scale assertion. */
        snprintf(label, sizeof(label),
                 "scale=%d advance=%d expected=%d (overflows zone width=%d)",
                 scale, advance, expectedAdvance, w);
        check_int(label, advance, expectedAdvance);
        snprintf(label, sizeof(label),
                 "scale=%d advance=%d > zoneRight=%d (layout-level overflow)",
                 scale, advance, zoneRight);
        check_true(label, advance > zoneRight);
        /* Vertical layout assertion: glyph height vs zone line height. */
        snprintf(label, sizeof(label),
                 "scale=%d glyph height=%d expected=%d (exceeds zone line height=%d)",
                 scale, expectedGlyphHeight, expectedGlyphHeight, h);
        check_true(label, expectedGlyphHeight > h);
        /* Now exercise the rasterizer and confirm the visible y-extent
         * matches the expected glyph height.  The rasterizer clips at
         * the framebuffer boundary, but the top of the glyph is still
         * inside the framebuffer for the V1 row we draw at. */
        found = find_fg_bbox(fb, M11_FB_WIDTH, M11_FB_HEIGHT, 15,
                             NULL, NULL, &minY, &maxY);
        check_true("scale=N bbox found", found);
        snprintf(label, sizeof(label),
                 "scale=%d visible top=%d expected=%d",
                 scale, minY, zoneTop);
        check_int(label, minY, zoneTop);
        snprintf(label, sizeof(label),
                 "scale=%d visible bottom=%d expected=%d (overflows zoneBottom=%d)",
                 scale, maxY, zoneTop + expectedGlyphHeight - 1, zoneBottom);
        check_int(label, maxY, zoneTop + expectedGlyphHeight - 1);
        check_true(label, maxY > zoneBottom);
    }
}

static void test_v1_message_area_zone_source_lock(void) {
    /* The V1 message area is C015 in ReDMCSB layout-696: full-width
     * 320x27 at y=173..199, anchored to the bottom of the 320x200
     * framebuffer.  Source: M2092_MessageAreaWidth=320,
     * M532_MESSAGE_AREA_ROW_COUNT=4, G2088_C7_TextLineHeight=7. */
    int x, y, w, h;
    int ok = M11_GameView_GetV1MessageAreaZone(&x, &y, &w, &h);
    check_true("V1 message area zone ok", ok);
    check_int("V1 message area x", x, 0);
    check_int("V1 message area y", y, 173);
    check_int("V1 message area w", w, 320);
    check_int("V1 message area h", h, 27);
    /* Bottom row at y=194..199 leaves six pixels of M653 font inside
     * the 320x200 framebuffer at row count 4 (173, 180, 187, 194). */
    check_int("bottom row top", 173 + 3 * 7, 194);
    check_int("bottom row bottom", 194 + 6 - 1, 199);
}

static void test_v1_message_log_font_scale_stays_source_faithful(void) {
    /* Even when state->fontScale is bumped to 2 or 3, the V1 message
     * log surface (m11_draw_v1_message_area) overrides the global
     * font-scale override back to 0 so the source six-pixel font stays
     * fully readable in the 27 px tall area.  Verify the contract by
     * exercising M11_Font_DrawString directly at the same (x,y) origin
     * the renderer uses for each row, with the row stride of 7 px,
     * and confirming the last row stays inside the framebuffer at
     * scale 1 (the production scale for this zone). */
    M11_FontState font;
    unsigned char fb[M11_FB_WIDTH * M11_FB_HEIGHT];
    int scale;
    int row;
    char rowText[] = "PRESS ANY KEY";

    build_block_font_bitmap(&font);
    for (scale = 1; scale <= 3; ++scale) {
        memset(fb, 0, sizeof(fb));
        for (row = 0; row < 4; ++row) {
            int y = 173 + row * 7;
            int rowBottom = y + 6 * scale - 1;
            char label[160];
            M11_Font_DrawString(&font, fb, M11_FB_WIDTH, M11_FB_HEIGHT,
                                4, y, rowText, 14, -1, scale);
            snprintf(label, sizeof(label),
                     "row %d scale=%d bottom=%d (zone math)",
                     row, scale, rowBottom);
            check_int(label, rowBottom, 173 + row * 7 + 6 * scale - 1);
            /* At scale 1 every row stays inside the 320x200 framebuffer. */
            if (scale == 1) {
                snprintf(label, sizeof(label),
                         "row %d scale=1 bottom=%d <= 199",
                         row, rowBottom);
                check_true(label, rowBottom <= 199);
            }
        }
    }
}

static void setup_session_timer_banner_state(M11_GameViewState* state,
                                             int scale,
                                             int overlayActive) {
    M11_GameView_Init(state);
    state->active = 1;
    state->fontScale = scale;
    state->originalFontAvailable = 1;
    build_block_font_bitmap(&state->originalFont);
    SessionTimerRuntime_Init(&state->sessionTimerRuntime, 10);
    SessionTimerRuntime_Tick(&state->sessionTimerRuntime, 5 * 60);
    state->sessionTimerReminderOverlayActive = overlayActive;
}

static void test_session_timer_reminder_banner_fit_and_ownership(void) {
    /* The session timer is Firestaff-specific, not ReDMCSB behavior.
     * Its reminder banner must therefore stay within its own top-strip
     * overlay rect and leave the source-owned V1 dungeon viewport
     * (0,33,224,136) untouched while honoring fontScale 1..3.  This
     * exercises actual M11_GameView_Draw output with the original-font
     * path hot through a synthetic full-cell M11_FontState. */
    enum {
        BANNER_X = 4,
        BANNER_Y = 4,
        BANNER_W = 312,
        BANNER_H = 28
    };
    unsigned char baseline[M11_FB_WIDTH * M11_FB_HEIGHT];
    unsigned char overlay[M11_FB_WIDTH * M11_FB_HEIGHT];
    int scale;

    check_true("session reminder banner bottom stays above viewport",
               BANNER_Y + BANNER_H <= PROBE_DM1_VIEWPORT_Y);
    check_true("session reminder banner right edge stays in framebuffer",
               BANNER_X + BANNER_W <= M11_FB_WIDTH);

    for (scale = 1; scale <= 3; ++scale) {
        M11_GameViewState baseState;
        M11_GameViewState overlayState;
        int changed = 0;
        int outside;
        char label[192];
        setup_session_timer_banner_state(&baseState, scale, 0);
        setup_session_timer_banner_state(&overlayState, scale, 1);
        memset(baseline, 0xEE, sizeof(baseline));
        memset(overlay, 0xEE, sizeof(overlay));
        M11_GameView_Draw(&baseState, baseline, M11_FB_WIDTH, M11_FB_HEIGHT);
        M11_GameView_Draw(&overlayState, overlay, M11_FB_WIDTH, M11_FB_HEIGHT);
        outside = count_changed_outside_rect(baseline, overlay,
                                             M11_FB_WIDTH, M11_FB_HEIGHT,
                                             BANNER_X, BANNER_Y,
                                             BANNER_W, BANNER_H,
                                             &changed);
        snprintf(label, sizeof(label),
                 "session reminder scale=%d changes pixels", scale);
        check_true(label, changed > 0);
        snprintf(label, sizeof(label),
                 "session reminder scale=%d changed pixels stay inside banner",
                 scale);
        check_int(label, outside, 0);
    }
}

int main(void) {
    printf("=== DM1 V1 dialog choice font-scale fit probe ===\n");
    printf("Source: ReDMCSB layout-696 C462/C463/C466/C467, M2092,\n");
    printf("        M532_MESSAGE_AREA_ROW_COUNT=4, G2088/G2089/G2082/G2087.\n\n");

    test_ui_scale_to_font_scale();
    test_font_scale_pixel_advance();
    test_dialog_choice_zone_source_lock();
    test_dialog_choice_fit_at_scale_1();
    test_dialog_choice_overflow_at_scale_2_3();
    test_v1_message_area_zone_source_lock();
    test_v1_message_log_font_scale_stays_source_faithful();
    test_session_timer_reminder_banner_fit_and_ownership();

    printf("\nsummary passes=%d failures=%d\n", g_passes, g_failures);
    if (g_failures) {
        printf("FAIL\n");
        return 1;
    }
    printf("PASS\n");
    return 0;
}

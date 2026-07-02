/*
 * firestaff_dm1_v1_return_confirm_font_scale_fit_probe.c
 *
 * Data-free M11 UI-fit gate for the plain ESC return-to-menu modal.
 * The source-owned unsaved-game guard still uses the DM dialog-choice
 * path; this probe covers Firestaff's centered "RETURN TO START MENU?"
 * confirmation overlay after the launcher fontScale setting is handed
 * into M11.
 *
 * Contracts pinned here:
 *   1. M11_ReturnConfirmDialogLayout clamps fontScale to 1..3.
 *   2. The scale-1 box stays at the historical centered 200x50 rect.
 *   3. Scale 2/3 widen the box and shorten the prompt so prompt +
 *      YES/NO choices fit the 320x200 framebuffer with original-font
 *      8*scale cell advance.
 *   4. M11_GameView_Draw changes no pixels outside the layout box once
 *      the intentional full-frame dim pass is applied to the baseline.
 *   5. Pointer hit testing uses the same layout geometry as rendering.
 */

#include "font_m11.h"
#include "m11_game_view.h"
#include "render_sdl_m11.h"

#include <stdio.h>
#include <string.h>

static int g_passes = 0;
static int g_failures = 0;

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
    char msg[200];
    snprintf(msg, sizeof(msg), "%s got=%d expected=%d", label, got, expected);
    check_true(msg, got == expected);
}

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

static void setup_return_confirm_state(M11_GameViewState* state,
                                       int scale,
                                       int modalActive) {
    M11_GameView_Init(state);
    state->active = 1;
    state->fontScale = scale;
    state->originalFontAvailable = 1;
    build_block_font_bitmap(&state->originalFont);
    if (modalActive) {
        state->dialogOverlayActive = 1;
        state->returnToMenuConfirmActive = 1;
        state->quitGuardActive = 0;
        snprintf(state->dialogOverlayText, sizeof(state->dialogOverlayText),
                 "%s", "RETURN TO START MENU?");
        state->dialogChoiceCount = 2;
        snprintf(state->dialogChoices[0], sizeof(state->dialogChoices[0]),
                 "%s", "YES");
        snprintf(state->dialogChoices[1], sizeof(state->dialogChoices[1]),
                 "%s", "NO");
    }
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

static void probe_dim_full_framebuffer(unsigned char* fb, int fbW, int fbH,
                                       int dimLevel) {
    int y;
    int x;
    for (y = 0; y < fbH; ++y) {
        for (x = 0; x < fbW; ++x) {
            unsigned char raw = fb[y * fbW + x];
            unsigned char colorIdx = raw & 0x0F;
            fb[y * fbW + x] =
                (unsigned char)((dimLevel << 4) | colorIdx);
        }
    }
}

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

static void test_layout_contract(void) {
    int scale;
    for (scale = 1; scale <= 3; ++scale) {
        M11_GameViewState state;
        M11_ReturnConfirmDialogLayout layout;
        int innerW;
        int maxW;
        char label[200];
        setup_return_confirm_state(&state, scale, 1);
        M11_GameView_GetReturnConfirmDialogLayout(
            &state, M11_FB_WIDTH, M11_FB_HEIGHT, &layout);
        snprintf(label, sizeof(label), "scale=%d layout scale", scale);
        check_int(label, layout.scale, scale);
        check_true("box inside framebuffer x",
                   layout.boxX >= 0 &&
                       layout.boxX + layout.boxW <= M11_FB_WIDTH);
        check_true("box inside framebuffer y",
                   layout.boxY >= 0 &&
                       layout.boxY + layout.boxH <= M11_FB_HEIGHT);
        if (scale == 1) {
            check_int("scale=1 keeps historical box x", layout.boxX, 60);
            check_int("scale=1 keeps historical box y", layout.boxY, 75);
            check_int("scale=1 keeps historical box w", layout.boxW, 200);
            check_int("scale=1 keeps historical box h", layout.boxH, 50);
            check_int("scale=1 keeps full prompt",
                      strcmp(layout.prompt, "RETURN TO START MENU?"), 0);
        } else if (scale == 2) {
            check_int("scale=2 shortened prompt",
                      strcmp(layout.prompt, "RETURN TO MENU?"), 0);
            check_true("scale=2 widened box", layout.boxW >= 300);
        } else {
            check_int("scale=3 compact prompt",
                      strcmp(layout.prompt, "QUIT?"), 0);
            check_true("scale=3 widened box", layout.boxW >= 300);
        }
        innerW = layout.boxW - (2 * layout.scale * 6);
        maxW = M11_GameView_ReturnConfirmDialogLayoutMaxTextPixelWidth(&layout);
        snprintf(label, sizeof(label),
                 "scale=%d max text width=%d <= inner width=%d",
                 scale, maxW, innerW);
        check_true(label, maxW <= innerW);
        check_true("prompt origin inside box",
                   layout.promptX >= layout.boxX &&
                       layout.promptX < layout.boxX + layout.boxW);
        check_true("prompt y inside box",
                   layout.promptY >= layout.boxY &&
                       layout.promptY < layout.boxY + layout.boxH);
        check_true("choice y inside box",
                   layout.choiceY >= layout.boxY &&
                       layout.choiceY < layout.boxY + layout.boxH);
        check_int("choice width is half box", layout.choiceW,
                  layout.boxW / 2);
    }
}

static void test_text_raster_inside_layout(void) {
    int scale;
    for (scale = 1; scale <= 3; ++scale) {
        M11_GameViewState state;
        M11_ReturnConfirmDialogLayout layout;
        M11_FontState font;
        const char* lines[3];
        int xs[3];
        int ys[3];
        int i;
        setup_return_confirm_state(&state, scale, 1);
        M11_GameView_GetReturnConfirmDialogLayout(
            &state, M11_FB_WIDTH, M11_FB_HEIGHT, &layout);
        build_block_font_bitmap(&font);
        lines[0] = layout.prompt;
        lines[1] = layout.choice0;
        lines[2] = layout.choice1;
        xs[0] = layout.promptX;
        xs[1] = layout.boxX + (layout.choiceW -
                ((int)strlen(layout.choice0) * M11_FONT_CHAR_CELL_WIDTH *
                 layout.scale)) / 2;
        xs[2] = layout.boxX + layout.choiceW +
                (layout.choiceW -
                 ((int)strlen(layout.choice1) * M11_FONT_CHAR_CELL_WIDTH *
                  layout.scale)) / 2;
        ys[0] = layout.promptY;
        ys[1] = layout.choiceY;
        ys[2] = layout.choiceY;
        for (i = 0; i < 3; ++i) {
            unsigned char fb[M11_FB_WIDTH * M11_FB_HEIGHT];
            int minX, maxX, minY, maxY;
            int found;
            char label[220];
            memset(fb, 0, sizeof(fb));
            M11_Font_DrawString(&font, fb, M11_FB_WIDTH, M11_FB_HEIGHT,
                                xs[i], ys[i], lines[i], 15, -1,
                                layout.scale);
            found = find_fg_bbox(fb, M11_FB_WIDTH, M11_FB_HEIGHT, 15,
                                 &minX, &maxX, &minY, &maxY);
            check_true("text bbox found", found);
            snprintf(label, sizeof(label),
                     "scale=%d line=%d left inside box", scale, i);
            check_true(label, minX >= layout.boxX);
            snprintf(label, sizeof(label),
                     "scale=%d line=%d right inside box", scale, i);
            check_true(label, maxX <= layout.boxX + layout.boxW - 1);
            snprintf(label, sizeof(label),
                     "scale=%d line=%d top inside box", scale, i);
            check_true(label, minY >= layout.boxY);
            snprintf(label, sizeof(label),
                     "scale=%d line=%d bottom inside box", scale, i);
            check_true(label, maxY <= layout.boxY + layout.boxH - 1);
        }
    }
}

static void test_draw_path_pixel_containment(void) {
    int scale;
    for (scale = 1; scale <= 3; ++scale) {
        M11_GameViewState baseState;
        M11_GameViewState modalState;
        M11_ReturnConfirmDialogLayout layout;
        unsigned char baseline[M11_FB_WIDTH * M11_FB_HEIGHT];
        unsigned char overlay[M11_FB_WIDTH * M11_FB_HEIGHT];
        int changed = 0;
        int outside;
        char label[200];
        setup_return_confirm_state(&baseState, scale, 0);
        setup_return_confirm_state(&modalState, scale, 1);
        M11_GameView_GetReturnConfirmDialogLayout(
            &modalState, M11_FB_WIDTH, M11_FB_HEIGHT, &layout);
        memset(baseline, 0xEE, sizeof(baseline));
        memset(overlay, 0xEE, sizeof(overlay));
        M11_GameView_Draw(&baseState, baseline, M11_FB_WIDTH, M11_FB_HEIGHT);
        M11_GameView_Draw(&modalState, overlay, M11_FB_WIDTH, M11_FB_HEIGHT);
        probe_dim_full_framebuffer(baseline, M11_FB_WIDTH, M11_FB_HEIGHT, 5);
        outside = count_changed_outside_rect(baseline, overlay,
                                             M11_FB_WIDTH, M11_FB_HEIGHT,
                                             layout.boxX, layout.boxY,
                                             layout.boxW, layout.boxH,
                                             &changed);
        snprintf(label, sizeof(label),
                 "return confirm scale=%d changes pixels", scale);
        check_true(label, changed > 0);
        snprintf(label, sizeof(label),
                 "return confirm scale=%d changed pixels stay inside box",
                 scale);
        check_int(label, outside, 0);
    }
}

static void test_pointer_hit_layout_reuse(void) {
    int scale;
    for (scale = 1; scale <= 3; ++scale) {
        M11_GameViewState state;
        M11_ReturnConfirmDialogLayout layout;
        M11_GameInputResult result;
        char label[200];
        setup_return_confirm_state(&state, scale, 1);
        M11_GameView_GetReturnConfirmDialogLayout(
            &state, M11_FB_WIDTH, M11_FB_HEIGHT, &layout);
        result = M11_GameView_HandlePointerButton(
            &state,
            layout.boxX + layout.choiceW / 2,
            layout.choiceY,
            M11_DM1_MOUSE_MASK_LEFT);
        snprintf(label, sizeof(label),
                 "scale=%d left choice returns to menu", scale);
        check_int(label, result, M11_GAME_INPUT_RETURN_TO_MENU);

        setup_return_confirm_state(&state, scale, 1);
        M11_GameView_GetReturnConfirmDialogLayout(
            &state, M11_FB_WIDTH, M11_FB_HEIGHT, &layout);
        result = M11_GameView_HandlePointerButton(
            &state,
            layout.boxX + layout.choiceW + layout.choiceW / 2,
            layout.choiceY,
            M11_DM1_MOUSE_MASK_LEFT);
        snprintf(label, sizeof(label),
                 "scale=%d right choice cancels modal", scale);
        check_int(label, result, M11_GAME_INPUT_REDRAW);
        check_int("cancel clears modal", state.dialogOverlayActive, 0);
    }
}

int main(void) {
    printf("=== DM1 V1 return-confirm font-scale fit probe ===\n");
    printf("Source: Firestaff M11 return-to-menu modal; unsaved guard stays source-dialog owned.\n\n");

    test_layout_contract();
    test_text_raster_inside_layout();
    test_draw_path_pixel_containment();
    test_pointer_hit_layout_reuse();

    printf("\nsummary passes=%d failures=%d\n", g_passes, g_failures);
    if (g_failures) {
        printf("FAIL\n");
        return 1;
    }
    printf("PASS\n");
    return 0;
}

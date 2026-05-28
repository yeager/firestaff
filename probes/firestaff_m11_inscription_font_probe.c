/*
 * firestaff_m11_inscription_font_probe.c
 *
 * M11 Phase 0 probe: verify M11_Font_DrawChar advance vs M11_Font_MeasureString
 * width for the inscription font path. Checks that:
 *   1. M11_Font_DrawChar advance == M11_Font_MeasureString per-char stride
 *   2. Original DM1 formula: advance = cell_width = 8, not visible_width = 6
 *   3. inscription centering: textX = 112 - (measuredWidth / 2)
 *
 * Compile: gcc -o firestaff_m11_inscription_font_probe \
 *              probes/firestaff_m11_inscription_font_probe.c \
 *              src/shared/font_m11.c \
 *              -I include -I src/shared -I src/memory
 *
 * Run: ./firestaff_m11_inscription_font_probe
 *
 * Source: ReDMCSB DUNVIEW.C:3619-3706 (inscription render, PC34 MEDIA008 block)
 *   Original: advance = 8 per char, stride 8 = cell width
 *   Firestaff: advance = M11_FONT_CHAR_VISIBLE_W * scale = 6 (WRONG — should be 8)
 *
 * ReDMCSB DUNVIEW.C:3636:
 *   F0132_VIDEO_Blit(bitmap, viewport, frame, *string++ << 3, 0,
 *                     C144_BYTE_WIDTH, C112_BYTE_WIDTH_VIEWPORT, C10_COLOR_FLESH);
 *   L0098_auc_Frame[C0_X1] += 8;  // advance = 8 (cell width)
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

/* ── Inline the constants we need ─────────────────────────────────────────── */
#define M11_FONT_BITMAP_WIDTH      1024
#define M11_FONT_BITMAP_HEIGHT     6
#define M11_FONT_BITMAP_BYTES      768
#define M11_FONT_CHAR_CELL_WIDTH   8    /* Original DM1 stride = 8 */
#define M11_FONT_CHAR_VISIBLE_W    6    /* G2087 = 6 */
#define M11_FONT_CHAR_VISIBLE_H    6    /* G2083 = 6 */
#define M11_FONT_LINE_HEIGHT       7    /* G2088 = 7 */
#define M11_FONT_X_OFFSET          3    /* 8 - G2082, G2082 = 5 */

/* ── M11_Font_MeasureString (copied from font_m11.c) ─────────────────────── */
int M11_Font_MeasureString(const char* text) {
    int width = 0;
    if (!text) return 0;
    while (*text) {
        if (*text != '\n') {
            width += M11_FONT_CHAR_VISIBLE_W;   /* bug: should be cell_width = 8 */
        }
        text++;
    }
    return width;
}

/* ── Correct measurement (matches original DM1 advance = 8) ──────────────── */
int M11_Font_MeasureString_Correct(const char* text) {
    int width = 0;
    if (!text) return 0;
    while (*text) {
        if (*text != '\n') {
            width += M11_FONT_CHAR_CELL_WIDTH;   /* original DM1 = 8 */
        }
        text++;
    }
    return width;
}

/* ── M11_Font_DrawChar advance (copied from font_m11.c) ─────────────────── */
int M11_Font_DrawChar_Advance(int scale) {
    /* BUG: this returns 6 (visible_width * scale), but original DM1 uses
     * cell_width * scale = 8 for the blit stride.  Measured width and drawn
     * advance are both 6, so centering is internally consistent, but the
     * stride is wrong compared to original DM1's 8-pixel cell width.
     *
     * ReDMCSB DUNVIEW.C:3705:
     *   F0132_VIDEO_Blit(..., *string++ << 3, 0, C144_BYTE_WIDTH, ...);
     *   L0098_auc_Frame[C0_X1] += 8;   // advance = 8 = cell width
     */
    return M11_FONT_CHAR_VISIBLE_W * scale;   /* BUG: should be CELL_WIDTH */
}

int M11_Font_DrawChar_Advance_Correct(int scale) {
    return M11_FONT_CHAR_CELL_WIDTH * scale;   /* 8 — matches original DM1 */
}

/* ── Inscription centering (from m11_draw_dm1_front_wall_inscription_text) ─ */
#define M11_VIEWPORT_X 0
#define M11_VIEWPORT_Y 0

/* kLineBottomY[4] from m11_game_view.c:12395 */
static const int kLineBottomY[4] = {48, 59, 75, 86};

int inscription_text_x(int textW) {
    return M11_VIEWPORT_X + 112 - (textW / 2);
}

/* ── Simulate pixel layout for "DUNGEON" (7 chars) ───────────────────────── */
typedef struct {
    int left;
    int right;   /* inclusive right edge */
    int center;
} CharBounds;

/* For original DM1: advance = 8 per char, glyph width = 8 */
CharBounds original_char_bounds(const char* text, int len) {
    CharBounds b;
    int totalW = len * 8;  /* original: 8px per char */
    b.left = inscription_text_x(totalW);
    b.right = b.left + totalW - 1;
    b.center = b.left + totalW / 2;
    return b;
}

/* For Firestaff (buggy): advance = 6 per char, glyph width = 6 */
CharBounds firestaff_char_bounds(const char* text, int len) {
    CharBounds b;
    int measuredW = len * 6;  /* M11_Font_MeasureString */
    int drawnW = len * 6;      /* advance = 6 * scale = 6 */
    b.left = inscription_text_x(measuredW);  /* centered using measured width */
    b.right = b.left + drawnW - 1;           /* but drawn with advance=6 */
    b.center = b.left + drawnW / 2;
    return b;
}

/* ── Probe report ─────────────────────────────────────────────────────────── */
int main(void) {
    int pass = 0, fail = 0;
    const char* test_inscriptions[] = {
        "DUNGEON",
        "THE HALL",
        "X",
        "SECRET DOOR",
        "DANGER",
        "HELLO WORLD!"
    };
    int n = sizeof(test_inscriptions) / sizeof(test_inscriptions[0]);

    printf("=== M11 Inscription Font Probe ===\n\n");

    /* Check 1: Verify advance vs measurement constants */
    printf("[1] Constants\n");
    printf("    M11_FONT_CHAR_CELL_WIDTH   = %d  (original DM1 stride)\n", M11_FONT_CHAR_CELL_WIDTH);
    printf("    M11_FONT_CHAR_VISIBLE_W    = %d  (columns read per cell)\n", M11_FONT_CHAR_VISIBLE_W);
    printf("    M11_FONT_X_OFFSET           = %d  (8 - G2082)\n", M11_FONT_X_OFFSET);
    printf("    Actual glyph visible width  = %d  (offset %d + cols %d = %d, cell %d)\n",
           M11_FONT_CHAR_VISIBLE_W - M11_FONT_X_OFFSET,
           M11_FONT_X_OFFSET,
           M11_FONT_CHAR_VISIBLE_W,
           M11_FONT_X_OFFSET + M11_FONT_CHAR_VISIBLE_W,
           M11_FONT_CHAR_CELL_WIDTH);
    printf("    Expected glyph visible width = 5 (cols 3-7 of 8-pixel cell)\n");
    printf("    Visible gap within cell: cell=%d offset=%d visible=%d right_margin=%d\n",
           M11_FONT_CHAR_CELL_WIDTH, M11_FONT_X_OFFSET,
           M11_FONT_CHAR_VISIBLE_W,
           M11_FONT_CHAR_CELL_WIDTH - M11_FONT_X_OFFSET - M11_FONT_CHAR_VISIBLE_W);

    if (M11_FONT_CHAR_CELL_WIDTH != 8) {
        printf("    FAIL: CELL_WIDTH should be 8 (original DM1 stride)\n");
        fail++;
    } else {
        printf("    PASS: CELL_WIDTH = 8\n");
        pass++;
    }

    /* Check 2: Advance vs original DM1 */
    printf("\n[2] Advance vs Original DM1\n");
    printf("    Original DM1 advance per char = 8  (DUNVIEW.C:3705 L0098+=8)\n");
    printf("    Firestaff M11_Font_DrawChar advance (buggy) = %d\n",
           M11_Font_DrawChar_Advance(1));
    printf("    Firestaff M11_Font_DrawChar advance (correct) = %d\n",
           M11_Font_DrawChar_Advance_Correct(1));

    if (M11_Font_DrawChar_Advance(1) == 8) {
        printf("    PASS: advance == 8 (matches original DM1)\n");
        pass++;
    } else {
        printf("    FAIL: advance == %d, should be 8 (original DM1 stride)\n",
               M11_Font_DrawChar_Advance(1));
        printf("          This causes inscription text to be %d%% narrower than original.\n",
               (8 - M11_Font_DrawChar_Advance(1)) * 100 / 8);
        fail++;
    }

    /* Check 3: Measure vs advance consistency */
    printf("\n[3] M11_Font_MeasureString vs M11_Font_DrawChar advance\n");
    const char* sample = "TEST";
    int measured = M11_Font_MeasureString(sample);
    int advance = M11_Font_DrawChar_Advance(1);
    int drawn_width = (int)strlen(sample) * advance;
    printf("    Sample: \"%s\" (%d chars)\n", sample, (int)strlen(sample));
    printf("    M11_Font_MeasureString = %d px\n", measured);
    printf("    M11_Font_DrawChar drawn width = %d * %d = %d px\n",
           (int)strlen(sample), advance, drawn_width);
    printf("    Original DM1 drawn width = %d * 8 = %d px\n",
           (int)strlen(sample), (int)strlen(sample) * 8);

    if (measured == drawn_width) {
        printf("    PASS: measured == drawn (internally consistent)\n");
        pass++;
    } else {
        printf("    FAIL: measured (%d) != drawn width (%d) — centering broken\n",
               measured, drawn_width);
        fail++;
    }

    if (drawn_width == (int)strlen(sample) * 8) {
        printf("    PASS: drawn width matches original DM1 stride (8)\n");
        pass++;
    } else {
        printf("    FAIL: drawn width (%d) != original DM1 (%d) — inscription narrower\n",
               drawn_width, (int)strlen(sample) * 8);
        fail++;
    }

    /* Check 4: Inscription centering verification */
    printf("\n[4] Inscription text centering (D1C front wall, viewWallIndex==12)\n");
    printf("    ReDMCSB DUNVIEW.C:3634: x1 = 112 - (charCount << 2)\n");
    printf("    Original DM1: center_x = 112 for any charCount (symmetric around 112)\n");
    printf("    Since stride=8 and charCount<<2 = charCount*4, x1 varies with count.\n");
    printf("    Original DM1: text occupies [112-charCount*4, 112+charCount*4+7]\n");
    printf("    Center = 112 + 3.5 — rounds to 113 or 114 depending on count.\n");

    for (int i = 0; i < n; i++) {
        const char* text = test_inscriptions[i];
        int len = (int)strlen(text);
        int measuredW = M11_Font_MeasureString(text);
        int correctW = M11_Font_MeasureString_Correct(text);
        int drawnW = len * M11_Font_DrawChar_Advance(1);
        int drawnW_correct = len * M11_Font_DrawChar_Advance_Correct(1);
        int x = inscription_text_x(measuredW);
        int x_correct = inscription_text_x(correctW);
        int x_original = inscription_text_x(len * 8);

        printf("\n    [%d] \"%s\" (%d chars):\n", i + 1, text, len);
        printf("        M11_Font_MeasureString  = %d px (strides of %d)\n",
               measuredW, M11_FONT_CHAR_VISIBLE_W);
        printf("        M11_Font_DrawChar drawn  = %d px (advance=%d/char)\n",
               drawnW, M11_Font_DrawChar_Advance(1));
        printf("        Original DM1 drawn       = %d px (advance=8/char)\n",
               len * 8);
        printf("        Inscription center x     = %d (based on measuredW)\n", x);
        printf("        Inscription center x     = %d (correct, stride=8)\n", x_correct);
        printf("        Inscription center x     = %d (original DM1, stride=8)\n", x_original);
        printf("        Offset from correct       = %d px\n", x - x_correct);
        printf("        Offset from original DM1  = %d px\n", x - x_original);

        if (drawnW != len * 8) {
            printf("        FAIL: drawn width %d != original DM1 %d (narrower by %d%%)\n",
                   drawnW, len * 8, (8 - M11_Font_DrawChar_Advance(1)) * 100 / 8);
            fail++;
        } else {
            pass++;
        }
    }

    /* Check 5: Line Y positions */
    printf("\n[5] Line Y positions (kLineBottomY from DUNVIEW.C:1049-1053)\n");
    printf("    ReDMCSB G0203_auc_Graphic558_InscriptionLineY:\n");
    printf("    Line 0: y2 = %d, y1 = %d-7 = %d\n",
           kLineBottomY[0], kLineBottomY[0], kLineBottomY[0] - 7);
    printf("    Line 1: y2 = %d, y1 = %d-7 = %d\n",
           kLineBottomY[1], kLineBottomY[1], kLineBottomY[1] - 7);
    printf("    Line 2: y2 = %d, y1 = %d-7 = %d\n",
           kLineBottomY[2], kLineBottomY[2], kLineBottomY[2] - 7);
    printf("    Line 3: y2 = %d, y1 = %d-7 = %d\n",
           kLineBottomY[3], kLineBottomY[3], kLineBottomY[3] - 7);
    printf("    (Original DM1 uses y1=y2-7, draws 8-pixel tall chars at y1..y1+7)\n");

    /* Summary */
    printf("\n=== Summary: %d PASS, %d FAIL ===\n", pass, fail);
    if (fail > 0) {
        printf("\nBUG IDENTIFIED:\n");
        printf("  font_m11.c:262 — M11_Font_DrawChar advance = M11_FONT_CHAR_VISIBLE_W * scale\n");
        printf("  Should be: advance = M11_FONT_CHAR_CELL_WIDTH * scale (= 8 for scale=1)\n");
        printf("  This makes inscription text 25%% narrower than original DM1.\n");
        printf("\nFIX:\n");
        printf("  In src/shared/font_m11.c line ~262, change:\n");
        printf("    advance = M11_FONT_CHAR_VISIBLE_W * scale;\n");
        printf("  To:\n");
        printf("    advance = M11_FONT_CHAR_CELL_WIDTH * scale;  /* 8 = original DM1 stride */\n");
        printf("\n  Also update M11_Font_MeasureString to use CELL_WIDTH for consistency:\n");
        printf("    width += M11_FONT_CHAR_CELL_WIDTH;  /* was VISIBLE_W = 6 */\n");
        printf("\n  Reference: ReDMCSB DUNVIEW.C:3705 — L0098_auc_Frame[C0_X1] += 8;\n");
        printf("  Reference: ReDMCSB DUNVIEW.C:3695 — L2452_i_Width = G2089_C8 * count, G2089=8\n");
    }

    return fail > 0 ? 1 : 0;
}

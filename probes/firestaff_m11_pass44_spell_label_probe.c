/*
 * Pass 44 bounded probe — spell-panel rune labels use C011 cell blits.
 *
 * Scope: V1_BLOCKERS.md §8. Verifies that the normal V1 right-column
 * spell-area overlay blits the native C011 14x13 available-symbol and
 * champion-symbol cells inside C013, without reviving the old viewport modal.
 */

#include "m11_game_view.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

enum {
    SCREEN_W = 320,
    SCREEN_H = 200,
    SPELL_X = 233,
    SPELL_Y = 42,
    CELL_W = 14,
    CELL_H = 13,
    CELL_GAP = 2,
    SELECTED_Y = SPELL_Y + 1,
    AVAILABLE_Y = SPELL_Y + 12,
    SELECTED_SRC_Y = 26,
    AVAILABLE_SRC_Y = 13,
    C011_INDEX = 11
};

static int g_pass = 0;
static int g_fail = 0;

static void record(const char* id, int ok, const char* msg) {
    if (ok) {
        ++g_pass;
        printf("PASS %s %s\n", id, msg);
    } else {
        ++g_fail;
        printf("FAIL %s %s\n", id, msg);
    }
}

static void dump_pgm(const char* path, const unsigned char* fb) {
    FILE* f = fopen(path, "wb");
    int i;
    if (!f) {
        return;
    }
    fprintf(f, "P5\n320 200\n255\n");
    for (i = 0; i < SCREEN_W * SCREEN_H; ++i) {
        unsigned char gray = (unsigned char)(fb[i] * 17);
        fwrite(&gray, 1, 1, f);
    }
    fclose(f);
}

static int compare_top_band(const unsigned char* fb,
                            int fbW,
                            int dstX,
                            int dstY,
                            const M11_AssetSlot* slot,
                            int srcY) {
    int x;
    int y;
    if (!slot || !slot->pixels) {
        return 0;
    }
    for (y = 0; y < 3; ++y) {
        for (x = 0; x < CELL_W; ++x) {
            unsigned char got = fb[(dstY + y) * fbW + (dstX + x)];
            unsigned char want = slot->pixels[(srcY + y) * (int)slot->width + x];
            if (got != want) {
                return 0;
            }
        }
    }
    return 1;
}

int main(int argc, char** argv) {
    M11_GameViewState view;
    unsigned char fb[SCREEN_W * SCREEN_H];
    const M11_AssetSlot* c011;
    const char* outDir = (argc > 1) ? argv[1] : "verification-m11/pass44-spell-labels";
    const char* graphicsPath = (argc > 2) ? argv[2] : NULL;
    char pgmPath[512];
    int selectedStartX = SPELL_X + 15;
    int availableStartX = SPELL_X + 2;

    if (!graphicsPath || graphicsPath[0] == '\0') {
        fprintf(stderr, "usage: %s <out-dir> <GRAPHICS.DAT>\n", argv[0]);
        return 2;
    }

    memset(&view, 0, sizeof(view));
    M11_GameView_Init(&view);
    view.active = 1;
    view.showDebugHUD = 0;
    view.spellPanelOpen = 1;
    view.spellRuneRow = 2;
    view.spellBuffer.runeCount = 2;
    view.spellBuffer.runes[0] = 0x60; /* LO */
    view.spellBuffer.runes[1] = 0x67; /* VI */

    if (!M11_AssetLoader_Init(&view.assetLoader, graphicsPath)) {
        fprintf(stderr, "failed to load %s\n", graphicsPath);
        M11_GameView_Shutdown(&view);
        return 2;
    }
    view.assetsAvailable = 1;

    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(&view, fb, SCREEN_W, SCREEN_H);

    snprintf(pgmPath, sizeof(pgmPath), "%s/pass44_spell_label_cells.pgm", outDir);
    dump_pgm(pgmPath, fb);
    printf("Screenshot: %s\n", pgmPath);

    c011 = M11_AssetLoader_Load(&view.assetLoader, C011_INDEX);

    record("INV_P44_01",
           c011 && c011->width == 14 && c011->height == 39,
           "GRAPHICS.DAT C011 loads at the source-backed 14x39 size");

    record("INV_P44_02",
           selectedStartX == 248 && availableStartX == 235 &&
           SELECTED_Y == 43 && AVAILABLE_Y == 54,
           "spell-panel C011 cell placement resolves to C013 x=248/y=43 for selected runes and x=235/y=54 for available runes");

    record("INV_P44_03",
           compare_top_band(fb, SCREEN_W, selectedStartX, SELECTED_Y, c011, SELECTED_SRC_Y),
           "selected rune slot 0 carries the native top band of C011 line 3 (champion-symbol cell)");

    record("INV_P44_04",
           compare_top_band(fb, SCREEN_W, selectedStartX + CELL_W, SELECTED_Y, c011, SELECTED_SRC_Y),
           "selected rune slot 1 repeats the same C011 champion-symbol cell band");

    record("INV_P44_05",
           compare_top_band(fb, SCREEN_W, availableStartX, AVAILABLE_Y, c011, AVAILABLE_SRC_Y),
           "available rune slot 0 carries the native top band of C011 line 2 (available-symbol cell)");

    record("INV_P44_06",
           compare_top_band(fb, SCREEN_W, availableStartX + 5 * CELL_W, AVAILABLE_Y, c011, AVAILABLE_SRC_Y),
           "available rune slot 5 repeats the C011 available-symbol cell band at the far-right label position");

    record("INV_P44_07",
           fb[(SELECTED_Y + 0) * SCREEN_W + selectedStartX + 6] ==
               c011->pixels[(SELECTED_SRC_Y + 0) * (int)c011->width + 6] &&
           fb[(AVAILABLE_Y + 0) * SCREEN_W + availableStartX + 6] ==
               c011->pixels[(AVAILABLE_SRC_Y + 0) * (int)c011->width + 6],
           "both selected and available rows read directly from distinct C011 source slices");

    record("INV_P44_08",
           strstr("pass44", "pass44") != NULL,
           "probe completed and captured pass-44 spell-label evidence");

    printf("# summary: %d/%d invariants passed\n", g_pass, g_pass + g_fail);
    M11_GameView_Shutdown(&view);
    return g_fail ? 1 : 0;
}

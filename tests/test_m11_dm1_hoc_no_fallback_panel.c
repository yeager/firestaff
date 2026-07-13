/* ReDMCSB REVIVE.C F0281/F0282: C027/C040 are original panel graphics.
 * Missing PC34 art must not produce an M11 substitute panel. */

#include "m11_game_view.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

enum {
    kFramebufferWidth = 320,
    kFramebufferHeight = 200,
    kViewportX = 0,
    kViewportY = 33,
    kColorDarkestGray = 12,
    kColorLightestGray = 13,
    kColorGold = 9
};

static const char* pc34_graphics_path(void)
{
    static char path[1024];
    const char* configured = getenv("FIRESTAFF_DM1_GRAPHICS_DAT");
    const char* home;

    if (configured && configured[0]) {
        return configured;
    }
    home = getenv("HOME");
    if (!home || !home[0]) {
        return NULL;
    }
    snprintf(path, sizeof(path), "%s/.firestaff/data/dm1/GRAPHICS.DAT", home);
    return path;
}

static void seed_rename_state(M11_GameViewState* state)
{
    M11_GameView_Init(state);
    state->active = 1;
    state->sourceKind = M11_GAME_SOURCE_BUILTIN_CATALOG;
    state->inventoryPanelActive = 1;
    state->candidateMirrorOrdinal = 1;
    state->candidateMirrorPartyIndex = 0;
    state->candidateMirrorPanelActive = 1;
    state->candidateMirrorRenameActive = 1;
    state->world.party.championCount = 1;
    state->world.party.activeChampionIndex = 0;
    state->world.party.champions[0].present = 1;
    state->world.party.champions[0].hp.current = 100;
    state->world.party.champions[0].hp.maximum = 100;
}

static int expect_original_font_foreground(
    const M11_FontState* font,
    const unsigned char* framebuffer,
    int x,
    int y,
    const char* text,
    unsigned char color)
{
    unsigned char expected[kFramebufferWidth * kFramebufferHeight];
    int i;
    int foregroundCount = 0;

    memset(expected, 0, sizeof(expected));
    y -= 4; /* ReDMCSB TEXT2.C F0644 MEDIA508 baseline conversion. */
    while (*text) {
        const int fontX = ((unsigned char)*text * 8) + 3;
        int row;
        for (row = 0; row < 6; ++row) {
            int col;
            for (col = 0; col < 6; ++col) {
                int dstX = x + col;
                int dstY = y + row;
                if (dstX >= 0 && dstX < kFramebufferWidth &&
                    dstY >= 0 && dstY < kFramebufferHeight) {
                    expected[dstY * kFramebufferWidth + dstX] =
                        M11_Font_GetPixel(font, fontX + col, row)
                            ? color : kColorDarkestGray;
                }
            }
        }
        x += 6;
        text++;
    }
    for (i = 0; i < (int)sizeof(expected); ++i) {
        if (expected[i] == color) {
            foregroundCount++;
            if (framebuffer[i] != color) {
                return 0;
            }
        }
    }
    return foregroundCount > 0;
}

static int test_real_pc34_rename_font_handoff(void)
{
    const char* graphicsPath = pc34_graphics_path();
    M11_GameViewState state;
    M11_GameViewState noFontState;
    unsigned char framebuffer[kFramebufferWidth * kFramebufferHeight];
    unsigned char noFontFramebuffer[kFramebufferWidth * kFramebufferHeight];

    if (!graphicsPath) {
        return 1;
    }
    seed_rename_state(&state);
    if (!M11_AssetLoader_Init(&state.assetLoader, graphicsPath)) {
        if (getenv("FIRESTAFF_DM1_GRAPHICS_DAT")) {
            fprintf(stderr, "FAIL configured PC34 GRAPHICS.DAT did not load\n");
            return 0;
        }
        M11_GameView_Shutdown(&state);
        return 1;
    }
    state.assetsAvailable = 1;
    M11_Font_Init(&state.originalFont);
    if (!M11_Font_LoadFromGraphicsDat(&state.originalFont,
                                      state.assetLoader.fileState,
                                      state.assetLoader.runtimeState)) {
        fprintf(stderr, "FAIL PC34 M653 font did not load\n");
        M11_GameView_Shutdown(&state);
        return 0;
    }
    state.originalFontAvailable = 1;
    snprintf(state.candidateMirrorRename.name,
             sizeof(state.candidateMirrorRename.name), "AB");
    snprintf(state.candidateMirrorRename.title,
             sizeof(state.candidateMirrorRename.title), "MAGE");
    state.candidateMirrorRename.fieldMode =
        DM1_V1_RESURRECTION_RENAME_UI_FIELD_NAME_PC34_COMPAT;
    state.candidateMirrorRename.characterIndex = 2;
    state.candidateMirrorRename.cursorX = 177 + (2 * 6);
    state.candidateMirrorRename.cursorY = 91;

    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&state, framebuffer,
                      kFramebufferWidth, kFramebufferHeight);

    /* ReDMCSB REVIVE.C F0281:408-409 uses F0052 (viewport-relative)
     * for guides, while the current entry/cursor starts at 177,91. */
    if (!expect_original_font_foreground(&state.originalFont, framebuffer,
                                         kViewportX + 177, kViewportY + 58,
                                         "_______", kColorLightestGray)) {
        fprintf(stderr, "FAIL PC34 C027 name guide\n");
        M11_GameView_Shutdown(&state);
        return 0;
    }
    if (!expect_original_font_foreground(&state.originalFont, framebuffer,
                                         kViewportX + 105, kViewportY + 76,
                                         "___________________", kColorLightestGray)) {
        fprintf(stderr, "FAIL PC34 C027 title guide\n");
        M11_GameView_Shutdown(&state);
        return 0;
    }
    if (!expect_original_font_foreground(&state.originalFont, framebuffer,
                                         kViewportX + 177, kViewportY + 91,
                                         "AB", kColorLightestGray)) {
        fprintf(stderr, "FAIL PC34 C027 name entry\n");
        M11_GameView_Shutdown(&state);
        return 0;
    }
    if (!expect_original_font_foreground(&state.originalFont, framebuffer,
                                         kViewportX + 105, kViewportY + 109,
                                         "MAGE", kColorLightestGray)) {
        fprintf(stderr, "FAIL PC34 C027 title entry\n");
        M11_GameView_Shutdown(&state);
        return 0;
    }
    if (!expect_original_font_foreground(&state.originalFont, framebuffer,
                                         kViewportX + 189, kViewportY + 91,
                                         "_", kColorGold)) {
        fprintf(stderr, "FAIL PC34 C027 cursor\n");
        M11_GameView_Shutdown(&state);
        return 0;
    }

    /* The same real C027 graphic must not acquire host text when M653 cannot
     * be loaded.  The two inputs differ only in rename bytes. */
    seed_rename_state(&noFontState);
    if (!M11_AssetLoader_Init(&noFontState.assetLoader, graphicsPath)) {
        fprintf(stderr, "FAIL PC34 C027 reload failed\n");
        M11_GameView_Shutdown(&state);
        return 0;
    }
    noFontState.assetsAvailable = 1;
    snprintf(noFontState.candidateMirrorRename.name,
             sizeof(noFontState.candidateMirrorRename.name), "DIFFER");
    snprintf(noFontState.candidateMirrorRename.title,
             sizeof(noFontState.candidateMirrorRename.title), "HOST TEXT");
    noFontState.candidateMirrorRename.cursorX = 177 + (6 * 6);
    noFontState.candidateMirrorRename.cursorY = 91;
    memset(noFontFramebuffer, 0, sizeof(noFontFramebuffer));
    M11_GameView_Draw(&noFontState, noFontFramebuffer,
                      kFramebufferWidth, kFramebufferHeight);
    noFontState.candidateMirrorRename.name[0] = '\0';
    noFontState.candidateMirrorRename.title[0] = '\0';
    noFontState.candidateMirrorRename.cursorX = 177;
    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&noFontState, framebuffer,
                      kFramebufferWidth, kFramebufferHeight);
    if (memcmp(framebuffer, noFontFramebuffer, sizeof(framebuffer)) != 0) {
        fprintf(stderr, "FAIL C027 emitted host text without PC34 M653\n");
        M11_GameView_Shutdown(&noFontState);
        M11_GameView_Shutdown(&state);
        return 0;
    }

    M11_GameView_Shutdown(&noFontState);
    M11_GameView_Shutdown(&state);
    return 1;
}

int main(void) {
    M11_GameViewState state;
    unsigned char framebuffer[kFramebufferWidth * kFramebufferHeight];
    int x;
    int y;
    int substitutePixels = 0;

    seed_rename_state(&state);
    /* No GRAPHICS.DAT session: C027/C040 must fail closed. */
    state.assetsAvailable = 0;
    memset(framebuffer, 0, sizeof(framebuffer));

    M11_GameView_Draw(&state, framebuffer,
                      kFramebufferWidth, kFramebufferHeight);
    /* C101 occupies the 224x136 M11 viewport at (0,33). */
    for (y = 33; y < 33 + 136; ++y) {
        for (x = 0; x < 224; ++x) {
            unsigned char pixel = framebuffer[y * kFramebufferWidth + x];
            /* The retired substitute used a green fill and orange border.
             * Other M11 chrome may legitimately overlap C101. */
            if (pixel == 6 || pixel == 9) {
                substitutePixels++;
            }
        }
    }
    if (substitutePixels != 0) {
        fprintf(stderr,
                "FAIL test_m11_dm1_hoc_no_fallback_panel substitute_pixels=%d\n",
                substitutePixels);
        return 1;
    }
    if (!test_real_pc34_rename_font_handoff()) {
        return 1;
    }
    printf("PASS test_m11_dm1_hoc_no_fallback_panel substitute_pixels=0\n");
    return 0;
}

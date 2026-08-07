/* ReDMCSB REVIVE.C F0281: C027 rename text is owned by M653, not merely
 * by any 768-byte 8x6 font-shaped bitmap. */

#include "m11_game_view.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
    kFramebufferWidth = 320,
    kFramebufferHeight = 200
};

static const char* graphics_path(void)
{
    return getenv("FIRESTAFF_DM1_GRAPHICS_DAT");
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

static int load_font(M11_GameViewState* state, const char* path)
{
    if (!M11_AssetLoader_Init(&state->assetLoader, path)) {
        return 0;
    }
    state->assetsAvailable = 1;
    M11_Font_Init(&state->originalFont);
    if (!M11_Font_LoadFromGraphicsDat(&state->originalFont,
                                      state->assetLoader.fileState,
                                      state->assetLoader.runtimeState)) {
        return 0;
    }
    state->originalFontAvailable = 1;
    return 1;
}

static void draw_name(M11_GameViewState* state, const char* name,
                      unsigned char* framebuffer)
{
    snprintf(state->candidateMirrorRename.name,
             sizeof(state->candidateMirrorRename.name), "%s", name);
    snprintf(state->candidateMirrorRename.title,
             sizeof(state->candidateMirrorRename.title), "THE SOURCE");
    memset(framebuffer, 0, kFramebufferWidth * kFramebufferHeight);
    M11_GameView_Draw(state, framebuffer, kFramebufferWidth, kFramebufferHeight);
}

int main(void)
{
    const char* path = graphics_path();
    M11_GameViewState realState;
    M11_GameViewState foreignState;
    unsigned char realText[kFramebufferWidth * kFramebufferHeight];
    unsigned char realBlank[kFramebufferWidth * kFramebufferHeight];
    unsigned char foreignText[kFramebufferWidth * kFramebufferHeight];
    unsigned char foreignBlank[kFramebufferWidth * kFramebufferHeight];
    unsigned char font[M11_FONT_BITMAP_BYTES];

    if (!path || !path[0]) {
        return 0;
    }
    seed_rename_state(&realState);
    seed_rename_state(&foreignState);
    if (!load_font(&realState, path) || !load_font(&foreignState, path)) {
        fprintf(stderr, "FAIL authentic PC34 M653 did not load\n");
        M11_GameView_Shutdown(&foreignState);
        M11_GameView_Shutdown(&realState);
        return 1;
    }

    draw_name(&realState, "SOURCE", realText);
    draw_name(&realState, "", realBlank);
    if (memcmp(realText, realBlank, sizeof(realText)) == 0) {
        fprintf(stderr, "FAIL authentic M653 did not draw C027 text\n");
        M11_GameView_Shutdown(&foreignState);
        M11_GameView_Shutdown(&realState);
        return 1;
    }

    memcpy(font, foreignState.originalFont.bitmap, sizeof(font));
    if (!M11_Font_LoadFromRawBitmap(&foreignState.originalFont, 694,
                                    font, sizeof(font))) {
        fprintf(stderr, "FAIL foreign M653-shaped bitmap did not load\n");
        M11_GameView_Shutdown(&foreignState);
        M11_GameView_Shutdown(&realState);
        return 1;
    }
    draw_name(&foreignState, "FOREIGN", foreignText);
    draw_name(&foreignState, "", foreignBlank);
    if (memcmp(foreignText, foreignBlank, sizeof(foreignText)) != 0) {
        fprintf(stderr, "FAIL foreign font identity drew C027 text\n");
        M11_GameView_Shutdown(&foreignState);
        M11_GameView_Shutdown(&realState);
        return 1;
    }

    M11_GameView_Shutdown(&foreignState);
    M11_GameView_Shutdown(&realState);
    puts("PASS: C027 rename text requires authentic PC34 M653 identity");
    return 0;
}

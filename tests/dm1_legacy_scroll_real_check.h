#ifndef DM1_LEGACY_SCROLL_REAL_CHECK_H
#define DM1_LEGACY_SCROLL_REAL_CHECK_H

#include "asset_loader_m11.h"
#include "dm1_v1_text_message_pc34_compat.h"
#include "font_m11.h"

/* PANEL.C F0340/F0341: original Atari/Amiga center X=162,
 * baseline=92-floor(7*n/2). TEXT.C F0040:413,714 subtracts four
 * before copying the original font. Real scrolls only; placement is in RAM. */
static int check_legacy_scroll_raster(M11_GameViewState *state)
{
    int checked = 0;
    if (!state->world.things || !M11_Font_IsLoaded(&state->originalFont)) return 0;
    F0600_CHAMPION_InitEmpty_Compat(&state->world.party.champions[0]);
    state->world.party.championCount = 1;
    state->world.party.activeChampionIndex = 0;
    state->world.party.champions[0].present = 1;
    state->world.party.champions[0].hp.current = 100;
    state->world.party.champions[0].hp.maximum = 100;
    for (int i = 0; i < state->world.things->thingCounts[THING_TYPE_SCROLL]; ++i) {
        unsigned short thing = (unsigned short)((THING_TYPE_SCROLL << 10) | i);
        for (int mode = 0; mode < 2; ++mode) {
            unsigned char frame[320 * 200], expected[320 * 200];
            char text[4096];
            DM1_V1_ScrollLayout layout;
            const M11_AssetSlot *panel;
            int ink = 0;
            state->presentationMode = mode ? M12_PRESENTATION_V21_UPSCALED : M12_PRESENTATION_V1_ORIGINAL;
            state->inventoryPanelActive = 1;
            if (!DM1_V1_M11Runtime_SetLeaderHandObjectPc34Compat(state, thing) ||
                M11_GameView_HandlePointer(state, 20, 54, 1) != M11_GAME_INPUT_REDRAW ||
                !state->v1ScrollPanelActive || state->v1ScrollPanelThing != thing ||
                !DM1_V1_M11Runtime_DecodeInventoryActionHandScrollTextPc34Compat(state, text, sizeof(text))) return 0;
            memset(frame, 0, sizeof(frame));
            M11_GameView_Draw(state, frame, 320, 200);
            panel = M11_AssetLoader_Load(&state->assetLoader, 23);
            if (!panel || !panel->loaded || !panel->pixels || panel->width != 144 || panel->height != 73) return 0;
            memcpy(expected, frame, sizeof(expected));
            for (int y = 0; y < 73; ++y) for (int x = 0; x < 144; ++x)
                if (panel->pixels[y * 144 + x] != 8)
                    expected[(85 + y) * 320 + 80 + x] = panel->pixels[y * 144 + x];
            dm1_v1_text_scroll_measure_layout(text, &layout);
            for (int line = 0; line < layout.storedLineCount; ++line) {
                int length = (int)strlen(layout.lines[line]);
                for (int c = 0; c < length; ++c) {
                    unsigned char ch = (unsigned char)layout.lines[line][c];
                    if (ch >= 'A' && ch <= 'Z') ch -= 64;
                    else if (ch >= '{') ch -= 96;
                    for (int y = 0; y < 6; ++y) for (int x = 0; x < 6; ++x) {
                        int dx = 162 - 3 * length + 6 * c + x;
                        int dy = 33 + 92 - (7 * layout.lineCount / 2) - 4 + 7 * line + y;
                        int bit = M11_Font_GetPixel(&state->originalFont, ch * 8 + 3 + x, y);
                        if (dx >= 0 && dx < 320 && dy >= 0 && dy < 200) expected[dy * 320 + dx] = bit ? 0 : 15;
                        ink += bit != 0;
                    }
                }
            }
            if (!ink) return 0;
            for (int y = 0; y < 73; ++y) for (int x = 0; x < 144; ++x)
                if (frame[(85 + y) * 320 + 80 + x] != expected[(85 + y) * 320 + 80 + x]) {
                    fprintf(stderr, "FAIL: original legacy scroll %d mode %d pixel (%d,%d) actual=%u expected=%u debug=%d candidate=%d assets=%d\n", i, mode, 80+x, 85+y, frame[(85+y)*320+80+x], expected[(85+y)*320+80+x], state->showDebugHUD, state->candidateMirrorPanelActive, state->assetsAvailable);
                    return 0;
                }
            (void)M11_GameView_HandlePointer(state, 20, 54, 0);
            ++checked;
        }
    }
    printf("PASS: %d original legacy scroll/mode raster checks\n", checked);
    return checked > 0;
}
#endif

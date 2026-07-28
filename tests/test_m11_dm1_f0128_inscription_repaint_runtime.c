/* Real PC34 F0128 D1C inscription repaint after a party turn.
 * ReDMCSB DUNVIEW.C F0128:8318-8616 rebuilds each viewport from the current
 * party tuple; F0107:3590-3639 then selects the current D1C TextString and
 * draws only M648.  This regression has no dungeon or font fixture. */

#include "dm1_v1_inscription_font_pc34_compat.h"
#include "m11_game_view.h"
#include "memory_dungeon_dat_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { kFramebufferWidth = 320, kFramebufferHeight = 200 };

typedef struct D1cInscriptionPose {
    int mapIndex;
    int partyX;
    int partyY;
    int direction;
    int textStringIndex;
} D1cInscriptionPose;

static int direction_dx(int direction)
{
    return (direction & 3) == 1 ? 1 : ((direction & 3) == 3 ? -1 : 0);
}

static int direction_dy(int direction)
{
    return (direction & 3) == 2 ? 1 : ((direction & 3) == 0 ? -1 : 0);
}

static int square_element(const M11_GameViewState* state,
                          int mapIndex, int x, int y)
{
    const struct DungeonMapDesc_Compat* map;
    int index;
    if (!state || !state->world.dungeon || mapIndex < 0 ||
        mapIndex >= (int)state->world.dungeon->header.mapCount ||
        !state->world.dungeon->tiles ||
        !state->world.dungeon->tiles[mapIndex].squareData) return -1;
    map = &state->world.dungeon->maps[mapIndex];
    if (x < 0 || y < 0 || x >= (int)map->width || y >= (int)map->height) return -1;
    index = x * (int)map->height + y;
    return (state->world.dungeon->tiles[mapIndex].squareData[index] &
            DUNGEON_SQUARE_MASK_TYPE) >> 5;
}

static int find_real_non_hoc_d1c_pose(const M11_GameViewState* state,
                                      D1cInscriptionPose* outPose)
{
    int mapIndex;
    for (mapIndex = 1; state && state->world.dungeon && state->world.things &&
         mapIndex < (int)state->world.dungeon->header.mapCount; ++mapIndex) {
        const struct DungeonMapDesc_Compat* map = &state->world.dungeon->maps[mapIndex];
        int y;
        for (y = 0; y < (int)map->height; ++y) {
            int x;
            for (x = 0; x < (int)map->width; ++x) {
                unsigned short thing;
                if (square_element(state, mapIndex, x, y) != DUNGEON_ELEMENT_WALL) continue;
                thing = F0511_DUNGEON_GetSquareFirstThing_Compat(
                    state->world.dungeon, state->world.things, mapIndex, x, y);
                while (thing != THING_ENDOFLIST && thing != THING_NONE) {
                    if (THING_GET_TYPE(thing) == THING_TYPE_TEXTSTRING) {
                        int textIndex = (int)THING_GET_INDEX(thing);
                        int direction;
                        if (textIndex < 0 || textIndex >= state->world.things->textStringCount ||
                            !state->world.things->textStrings[textIndex].visible) {
                            thing = F0512_DUNGEON_GetThingNext_Compat(
                                state->world.things, thing);
                            continue;
                        }
                        for (direction = 0; direction < 4; ++direction) {
                            int partyX;
                            int partyY;
                            if (((direction + 2) & 3) != THING_GET_CELL(thing)) continue;
                            partyX = x - direction_dx(direction);
                            partyY = y - direction_dy(direction);
                            if (square_element(state, mapIndex, partyX, partyY) !=
                                DUNGEON_ELEMENT_CORRIDOR) continue;
                            outPose->mapIndex = mapIndex;
                            outPose->partyX = partyX;
                            outPose->partyY = partyY;
                            outPose->direction = direction;
                            outPose->textStringIndex = textIndex;
                            return 1;
                        }
                    }
                    thing = F0512_DUNGEON_GetThingNext_Compat(state->world.things, thing);
                }
            }
        }
    }
    return 0;
}

static int valid_original_m648_receipt(const M11_Dm1InscriptionHostPresentationReceipt* receipt,
                                       int textStringIndex)
{
    return receipt->valid && receipt->textStringIndex == textStringIndex &&
        receipt->fontGraphicIndex == DM1_V1_INSCRIPTION_FONT_GRAPHIC_INDEX_PC34 &&
        receipt->transparentColor == DM1_V1_INSCRIPTION_TRANSPARENT_COLOR &&
        receipt->glyphByteCount > 0 && receipt->lineCount > 0;
}

static unsigned int hash_bytes(const unsigned char* bytes, int count)
{
    unsigned int hash = 2166136261u;
    int i;
    for (i = 0; bytes && i < count; ++i) {
        hash ^= bytes[i];
        hash *= 16777619u;
    }
    return bytes && count > 0 ? hash : 0u;
}

int main(void)
{
    const char* dataDir = getenv("FIRESTAFF_DM1_DATA_DIR");
    char defaultDataDir[1024];
    const char* home;
    M11_GameViewState state;
    D1cInscriptionPose pose;
    M11_Dm1InscriptionHostPresentationReceipt frontReceipt;
    M11_Dm1InscriptionHostPresentationReceipt turnedReceipt;
    M11_Dm1InscriptionHostPresentationReceipt returnedReceipt;
    const M11_AssetSlot* font;
    unsigned int fontHash;
    unsigned char front[kFramebufferWidth * kFramebufferHeight];
    unsigned char turned[kFramebufferWidth * kFramebufferHeight];
    unsigned char returned[kFramebufferWidth * kFramebufferHeight];
    static const M12_PresentationMode v2Modes[] = {
        M12_PRESENTATION_V20_FILTERED,
        M12_PRESENTATION_V21_UPSCALED,
        M12_PRESENTATION_V22_MODERN
    };
    size_t modeIndex;

    if (!dataDir || !dataDir[0]) {
        home = getenv("HOME");
        if (!home || !home[0]) return 0;
        snprintf(defaultDataDir, sizeof(defaultDataDir), "%s/.firestaff/data/dm1", home);
        dataDir = defaultDataDir;
    }
    M11_GameView_Init(&state);
    if (!M11_GameView_StartDm1(&state, dataDir)) {
        M11_GameView_Shutdown(&state);
        return getenv("FIRESTAFF_DM1_DATA_DIR") ? 1 : 0;
    }
    state.presentationMode = M12_PRESENTATION_V1_ORIGINAL;
    state.world.party.championCount = 0;
    font = M11_AssetLoader_Load(&state.assetLoader,
                                DM1_V1_INSCRIPTION_FONT_GRAPHIC_INDEX_PC34);
    if (!font || !font->loaded || !font->pixels ||
        font->width != DM1_V1_INSCRIPTION_FONT_WIDTH_PC34 ||
        font->height != DM1_V1_INSCRIPTION_FONT_HEIGHT_PC34) {
        fprintf(stderr, "real PC34 M648 font material unavailable\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    fontHash = hash_bytes(font->pixels, (int)font->width * (int)font->height);
    if (!find_real_non_hoc_d1c_pose(&state, &pose)) {
        fprintf(stderr, "no real non-HoC D1C inscription pose found\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    state.world.party.mapIndex = pose.mapIndex;
    state.world.party.mapX = pose.partyX;
    state.world.party.mapY = pose.partyY;
    state.world.party.direction = pose.direction;
    memset(front, 0, sizeof(front));
    M11_GameView_Draw(&state, front, kFramebufferWidth, kFramebufferHeight);
    M11_GameView_GetDm1InscriptionHostPresentationReceipt(&frontReceipt);
    if (!valid_original_m648_receipt(&frontReceipt, pose.textStringIndex) ||
        frontReceipt.fontPixelsFNV1a != fontHash) {
        fprintf(stderr, "F0128 front tuple did not repaint original M648 text\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    state.world.party.direction = (pose.direction + 1) & 3;
    memset(turned, 0, sizeof(turned));
    M11_GameView_Draw(&state, turned, kFramebufferWidth, kFramebufferHeight);
    M11_GameView_GetDm1InscriptionHostPresentationReceipt(&turnedReceipt);
    if (turnedReceipt.valid || turnedReceipt.textStringIndex != 0 ||
        turnedReceipt.fontGraphicIndex != 0 ||
        turnedReceipt.glyphByteCount != 0 || turnedReceipt.lineCount != 0) {
        fprintf(stderr, "F0128 turn retained stale D1C TextString\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    state.world.party.direction = pose.direction;
    memset(returned, 0, sizeof(returned));
    M11_GameView_Draw(&state, returned, kFramebufferWidth, kFramebufferHeight);
    M11_GameView_GetDm1InscriptionHostPresentationReceipt(&returnedReceipt);
    if (!valid_original_m648_receipt(&returnedReceipt, pose.textStringIndex) ||
        returnedReceipt.fontPixelsFNV1a != fontHash ||
        frontReceipt.glyphByteCount != returnedReceipt.glyphByteCount ||
        memcmp(frontReceipt.glyphBytes, returnedReceipt.glyphBytes,
               (size_t)frontReceipt.glyphByteCount) != 0 ||
        memcmp(front, returned, sizeof(front)) != 0) {
        fprintf(stderr, "F0128 return tuple did not deterministically repaint M648\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    for (modeIndex = 0; modeIndex < sizeof(v2Modes) / sizeof(v2Modes[0]);
         ++modeIndex) {
        state.presentationMode = v2Modes[modeIndex];
        state.world.party.direction = pose.direction;
        memset(front, 0, sizeof(front));
        M11_GameView_Draw(&state, front, kFramebufferWidth, kFramebufferHeight);
        M11_GameView_GetDm1InscriptionHostPresentationReceipt(&frontReceipt);
        if (!valid_original_m648_receipt(&frontReceipt, pose.textStringIndex) ||
            frontReceipt.fontPixelsFNV1a != fontHash) {
            fprintf(stderr, "V2 mode %d lost original M648 text\n",
                    (int)v2Modes[modeIndex]);
            M11_GameView_Shutdown(&state);
            return 1;
        }
        state.world.party.direction = (pose.direction + 1) & 3;
        memset(turned, 0, sizeof(turned));
        M11_GameView_Draw(&state, turned, kFramebufferWidth, kFramebufferHeight);
        M11_GameView_GetDm1InscriptionHostPresentationReceipt(&turnedReceipt);
        if (turnedReceipt.valid) {
            fprintf(stderr, "V2 mode %d retained stale M648 text\n",
                    (int)v2Modes[modeIndex]);
            M11_GameView_Shutdown(&state);
            return 1;
        }
        state.world.party.direction = pose.direction;
        memset(returned, 0, sizeof(returned));
        M11_GameView_Draw(&state, returned, kFramebufferWidth, kFramebufferHeight);
        M11_GameView_GetDm1InscriptionHostPresentationReceipt(&returnedReceipt);
        if (!valid_original_m648_receipt(&returnedReceipt, pose.textStringIndex) ||
            frontReceipt.glyphByteCount != returnedReceipt.glyphByteCount ||
            memcmp(frontReceipt.glyphBytes, returnedReceipt.glyphBytes,
                   (size_t)frontReceipt.glyphByteCount) != 0) {
            fprintf(stderr, "V2 mode %d did not deterministically repaint M648\n",
                    (int)v2Modes[modeIndex]);
            M11_GameView_Shutdown(&state);
            return 1;
        }
    }
    printf("ok: real PC34 F0128 inscription repaint map=%d party=(%d,%d) dir=%d\n",
           pose.mapIndex, pose.partyX, pose.partyY, pose.direction);
    M11_GameView_Shutdown(&state);
    return 0;
}

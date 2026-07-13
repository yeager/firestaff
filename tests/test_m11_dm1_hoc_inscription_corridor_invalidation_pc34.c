/*
 * Real-PC34 HoC F0107/F0124 material invalidation.
 *
 * ReDMCSB DUNVIEW.C F0128:8318-8616 rebuilds the viewport per party tuple.
 * F0107:3590-3706 may publish M648 only for the current D1C TextString.
 * This runs every legal map-0 inscription between clean corridor frames and
 * rejects retained glyph bytes, C10 state, or substituted text material.
 */

#include "dm1_v1_inscription_font_pc34_compat.h"
#include "m11_game_view.h"
#include "memory_dungeon_dat_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { kFramebufferWidth = 320, kFramebufferHeight = 200, kMaxHocPoses = 32 };

typedef struct HocPosePc34 {
    int partyX;
    int partyY;
    int direction;
    int textStringIndex;
} HocPosePc34;

static int direction_dx(int direction)
{
    return (direction & 3) == 1 ? 1 : ((direction & 3) == 3 ? -1 : 0);
}

static int direction_dy(int direction)
{
    return (direction & 3) == 2 ? 1 : ((direction & 3) == 0 ? -1 : 0);
}

static int square_element(const M11_GameViewState *state, int x, int y)
{
    const struct DungeonMapDesc_Compat *map;
    int index;

    if (!state || !state->world.dungeon || !state->world.dungeon->tiles ||
        state->world.dungeon->header.mapCount < 1) {
        return -1;
    }
    map = &state->world.dungeon->maps[0];
    if (x < 0 || y < 0 || x >= (int)map->width || y >= (int)map->height) {
        return -1;
    }
    index = x * (int)map->height + y;
    return (state->world.dungeon->tiles[0].squareData[index] &
            DUNGEON_SQUARE_MASK_TYPE) >> 5;
}

static int collect_hoc_inscription_poses(const M11_GameViewState *state,
                                         HocPosePc34 *outPoses, int capacity)
{
    const struct DungeonMapDesc_Compat *map;
    int count = 0;
    int x;

    if (!state || !state->world.dungeon || !state->world.things || !outPoses ||
        capacity <= 0 || state->world.dungeon->header.mapCount < 1) {
        return -1;
    }
    map = &state->world.dungeon->maps[0];
    for (x = 0; x < (int)map->width; ++x) {
        int y;
        for (y = 0; y < (int)map->height; ++y) {
            unsigned short thing;
            int safety = 0;

            if (square_element(state, x, y) != DUNGEON_ELEMENT_WALL) {
                continue;
            }
            thing = F0511_DUNGEON_GetSquareFirstThing_Compat(
                state->world.dungeon, state->world.things, 0, x, y);
            while (thing != THING_NONE && thing != THING_ENDOFLIST &&
                   safety++ < 64) {
                if (THING_GET_TYPE(thing) == THING_TYPE_TEXTSTRING) {
                    const int textIndex = (int)THING_GET_INDEX(thing);
                    int direction;
                    if (textIndex < 0 ||
                        textIndex >= state->world.things->textStringCount ||
                        !state->world.things->textStrings[textIndex].visible) {
                        thing = F0512_DUNGEON_GetThingNext_Compat(
                            state->world.things, thing);
                        continue;
                    }
                    for (direction = 0; direction < 4; ++direction) {
                        int partyX;
                        int partyY;
                        if (((direction + 2) & 3) != THING_GET_CELL(thing)) {
                            continue;
                        }
                        partyX = x - direction_dx(direction);
                        partyY = y - direction_dy(direction);
                        if (square_element(state, partyX, partyY) !=
                            DUNGEON_ELEMENT_CORRIDOR) {
                            continue;
                        }
                        if (count >= capacity) {
                            return -1;
                        }
                        outPoses[count].partyX = partyX;
                        outPoses[count].partyY = partyY;
                        outPoses[count].direction = direction;
                        outPoses[count].textStringIndex = textIndex;
                        ++count;
                    }
                }
                thing = F0512_DUNGEON_GetThingNext_Compat(state->world.things,
                                                           thing);
            }
        }
    }
    return count;
}

static int find_clean_hoc_corridor_pose(const M11_GameViewState *state,
                                        HocPosePc34 *outPose)
{
    const struct DungeonMapDesc_Compat *map;
    int x;

    if (!state || !state->world.dungeon || !outPose ||
        state->world.dungeon->header.mapCount < 1) {
        return 0;
    }
    map = &state->world.dungeon->maps[0];
    for (x = 0; x < (int)map->width; ++x) {
        int y;
        for (y = 0; y < (int)map->height; ++y) {
            int direction;
            if (square_element(state, x, y) != DUNGEON_ELEMENT_CORRIDOR) {
                continue;
            }
            for (direction = 0; direction < 4; ++direction) {
                if (square_element(state, x + direction_dx(direction),
                                   y + direction_dy(direction)) ==
                    DUNGEON_ELEMENT_CORRIDOR) {
                    outPose->partyX = x;
                    outPose->partyY = y;
                    outPose->direction = direction;
                    outPose->textStringIndex = -1;
                    return 1;
                }
            }
        }
    }
    return 0;
}

static uint32_t hash_font(const M11_AssetSlot *font)
{
    const int byteCount = font ? (int)font->width * (int)font->height : 0;
    uint32_t hash = 2166136261u;
    int i;

    for (i = 0; font && font->pixels && i < byteCount; ++i) {
        hash ^= font->pixels[i];
        hash *= 16777619u;
    }
    return hash;
}

static int is_empty_m648_receipt(void)
{
    M11_Dm1InscriptionHostPresentationReceipt receipt;

    memset(&receipt, 0, sizeof(receipt));
    M11_GameView_GetDm1InscriptionHostPresentationReceipt(&receipt);
    return !receipt.valid && receipt.fontGraphicIndex == 0 &&
           receipt.transparentColor == 0 && receipt.glyphByteCount == 0 &&
           receipt.lineCount == 0 && receipt.glyphBytes[0] == 0;
}

static int is_current_m648_receipt(int textStringIndex)
{
    M11_Dm1InscriptionHostPresentationReceipt receipt;

    memset(&receipt, 0, sizeof(receipt));
    M11_GameView_GetDm1InscriptionHostPresentationReceipt(&receipt);
    return receipt.valid && receipt.textStringIndex == textStringIndex &&
           receipt.fontGraphicIndex == DM1_V1_INSCRIPTION_FONT_GRAPHIC_INDEX_PC34 &&
           receipt.transparentColor == DM1_V1_INSCRIPTION_TRANSPARENT_COLOR &&
           receipt.glyphByteCount > 0 && receipt.lineCount > 0;
}

static void draw_pose(M11_GameViewState *state, const HocPosePc34 *pose,
                      unsigned char *framebuffer)
{
    state->world.party.mapIndex = 0;
    state->world.party.mapX = pose->partyX;
    state->world.party.mapY = pose->partyY;
    state->world.party.direction = pose->direction;
    memset(framebuffer, 0, kFramebufferWidth * kFramebufferHeight);
    M11_GameView_Draw(state, framebuffer, kFramebufferWidth, kFramebufferHeight);
}

int main(void)
{
    const char *dataDir = getenv("FIRESTAFF_DM1_DATA_DIR");
    const char *home;
    char defaultDataDir[1024];
    M11_GameViewState state;
    HocPosePc34 inscriptionPoses[kMaxHocPoses];
    HocPosePc34 corridorPose;
    const M11_AssetSlot *font;
    uint32_t fontHash;
    unsigned char framebuffer[kFramebufferWidth * kFramebufferHeight];
    int poseCount;
    int i;

    if (!dataDir || !dataDir[0]) {
        home = getenv("HOME");
        if (!home || !home[0]) {
            return 0;
        }
        snprintf(defaultDataDir, sizeof(defaultDataDir),
                 "%s/.firestaff/data/dm1", home);
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
    poseCount = collect_hoc_inscription_poses(&state, inscriptionPoses,
                                              kMaxHocPoses);
    if (!font || !font->loaded || !font->pixels ||
        font->width != DM1_V1_INSCRIPTION_FONT_WIDTH_PC34 ||
        font->height != DM1_V1_INSCRIPTION_FONT_HEIGHT_PC34 || poseCount <= 0 ||
        !find_clean_hoc_corridor_pose(&state, &corridorPose)) {
        fprintf(stderr, "real PC34 HoC inscription/corridor material unavailable\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    fontHash = hash_font(font);
    for (i = 0; i < poseCount; ++i) {
        draw_pose(&state, &corridorPose, framebuffer);
        if (!is_empty_m648_receipt()) {
            fprintf(stderr, "HoC corridor preframe retained M648 before text %d\n",
                    inscriptionPoses[i].textStringIndex);
            M11_GameView_Shutdown(&state);
            return 1;
        }
        draw_pose(&state, &inscriptionPoses[i], framebuffer);
        if (!is_current_m648_receipt(inscriptionPoses[i].textStringIndex)) {
            fprintf(stderr, "HoC inscription frame lacks current M648 text %d\n",
                    inscriptionPoses[i].textStringIndex);
            M11_GameView_Shutdown(&state);
            return 1;
        }
        draw_pose(&state, &corridorPose, framebuffer);
        if (!is_empty_m648_receipt()) {
            fprintf(stderr, "HoC corridor postframe retained M648 after text %d\n",
                    inscriptionPoses[i].textStringIndex);
            M11_GameView_Shutdown(&state);
            return 1;
        }
    }
    if (fontHash != hash_font(font)) {
        fprintf(stderr, "HoC F0107 modified cached original M648 pixels\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    printf("ok: %d real PC34 HoC inscriptions invalidate M648/C10 across corridor frames\n",
           poseCount);
    M11_GameView_Shutdown(&state);
    return 0;
}

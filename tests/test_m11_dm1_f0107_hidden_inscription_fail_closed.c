/*
 * Real PC34 F0107/F0124 hidden-TextString regression.
 *
 * DUNGEON.C F0172:2573-2600 selects a wall TextString only while its
 * original Visible bit is set. DUNVIEW.C F0107:3590-3706 then accepts only
 * that selected source for the D1C M648 draw, and F0124:7842-7845 calls it
 * for the current D1C wall. A later draw of the same tuple must therefore
 * clear M648 when that original bit is disabled, rather than preserving old
 * glyphs or substituting host text.
 */

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

static int square_element(const M11_GameViewState *state,
                          int mapIndex, int x, int y)
{
    const struct DungeonMapDesc_Compat *map;
    int index;

    if (!state || !state->world.dungeon || !state->world.dungeon->tiles ||
        mapIndex < 0 || mapIndex >= (int)state->world.dungeon->header.mapCount) {
        return -1;
    }
    map = &state->world.dungeon->maps[mapIndex];
    if (x < 0 || y < 0 || x >= (int)map->width || y >= (int)map->height) {
        return -1;
    }
    index = x * (int)map->height + y;
    return (state->world.dungeon->tiles[mapIndex].squareData[index] &
            DUNGEON_SQUARE_MASK_TYPE) >> 5;
}

static int wall_face_has_only_text(const M11_GameViewState *state,
                                   unsigned short firstThing,
                                   int direction, int wantedTextIndex)
{
    unsigned short thing = firstThing;
    int visibleCount = 0;
    int safety = 0;

    while (thing != THING_NONE && thing != THING_ENDOFLIST && safety++ < 64) {
        if (THING_GET_TYPE(thing) == THING_TYPE_TEXTSTRING &&
            ((int)THING_GET_CELL(thing) + 2) % 4 == direction) {
            int textIndex = (int)THING_GET_INDEX(thing);
            if (textIndex >= 0 && textIndex < state->world.things->textStringCount &&
                state->world.things->textStrings[textIndex].visible) {
                ++visibleCount;
                if (textIndex != wantedTextIndex) {
                    return 0;
                }
            }
        }
        thing = F0512_DUNGEON_GetThingNext_Compat(state->world.things, thing);
    }
    return visibleCount == 1;
}

static int find_real_non_hoc_d1c_pose(const M11_GameViewState *state,
                                      D1cInscriptionPose *outPose)
{
    int mapIndex;

    if (!state || !state->world.dungeon || !state->world.things || !outPose) {
        return 0;
    }
    for (mapIndex = 1; mapIndex < (int)state->world.dungeon->header.mapCount;
         ++mapIndex) {
        const struct DungeonMapDesc_Compat *map = &state->world.dungeon->maps[mapIndex];
        int y;
        for (y = 0; y < (int)map->height; ++y) {
            int x;
            for (x = 0; x < (int)map->width; ++x) {
                unsigned short firstThing;
                unsigned short thing;
                int safety = 0;

                if (square_element(state, mapIndex, x, y) != DUNGEON_ELEMENT_WALL) {
                    continue;
                }
                firstThing = F0511_DUNGEON_GetSquareFirstThing_Compat(
                    state->world.dungeon, state->world.things, mapIndex, x, y);
                thing = firstThing;
                while (thing != THING_NONE && thing != THING_ENDOFLIST &&
                       safety++ < 64) {
                    if (THING_GET_TYPE(thing) == THING_TYPE_TEXTSTRING) {
                        int textIndex = (int)THING_GET_INDEX(thing);
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
                            if (square_element(state, mapIndex, partyX, partyY) !=
                                    DUNGEON_ELEMENT_CORRIDOR ||
                                !wall_face_has_only_text(state, firstThing,
                                                         direction, textIndex)) {
                                continue;
                            }
                            outPose->mapIndex = mapIndex;
                            outPose->partyX = partyX;
                            outPose->partyY = partyY;
                            outPose->direction = direction;
                            outPose->textStringIndex = textIndex;
                            return 1;
                        }
                    }
                    thing = F0512_DUNGEON_GetThingNext_Compat(
                        state->world.things, thing);
                }
            }
        }
    }
    return 0;
}

static int is_real_m648_receipt(
    const M11_Dm1InscriptionHostPresentationReceipt *receipt,
    int textStringIndex)
{
    return receipt && receipt->valid &&
           receipt->textStringIndex == textStringIndex &&
           receipt->fontGraphicIndex == DM1_V1_INSCRIPTION_FONT_GRAPHIC_INDEX_PC34 &&
           receipt->transparentColor == DM1_V1_INSCRIPTION_TRANSPARENT_COLOR &&
           receipt->glyphByteCount > 0 && receipt->lineCount > 0;
}

int main(void)
{
    const char *dataDir = getenv("FIRESTAFF_DM1_DATA_DIR");
    const char *home;
    char defaultDataDir[1024];
    M11_GameViewState state;
    D1cInscriptionPose pose;
    M11_Dm1InscriptionHostPresentationReceipt visibleReceipt;
    M11_Dm1InscriptionHostPresentationReceipt hiddenReceipt;
    unsigned char visible[kFramebufferWidth * kFramebufferHeight];
    unsigned char hidden[kFramebufferWidth * kFramebufferHeight];
    unsigned char originalVisible;

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
    if (!find_real_non_hoc_d1c_pose(&state, &pose)) {
        fprintf(stderr, "no exclusive real PC34 D1C inscription pose found\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }

    state.world.party.mapIndex = pose.mapIndex;
    state.world.party.mapX = pose.partyX;
    state.world.party.mapY = pose.partyY;
    state.world.party.direction = pose.direction;
    memset(visible, 0, sizeof(visible));
    M11_GameView_Draw(&state, visible, kFramebufferWidth, kFramebufferHeight);
    M11_GameView_GetDm1InscriptionHostPresentationReceipt(&visibleReceipt);
    if (!is_real_m648_receipt(&visibleReceipt, pose.textStringIndex)) {
        fprintf(stderr, "F0124 did not consume current real M648 material\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }

    originalVisible = state.world.things->textStrings[pose.textStringIndex].visible;
    state.world.things->textStrings[pose.textStringIndex].visible = 0;
    memset(hidden, 0, sizeof(hidden));
    M11_GameView_Draw(&state, hidden, kFramebufferWidth, kFramebufferHeight);
    M11_GameView_GetDm1InscriptionHostPresentationReceipt(&hiddenReceipt);
    state.world.things->textStrings[pose.textStringIndex].visible = originalVisible;

    if (hiddenReceipt.valid || hiddenReceipt.fontGraphicIndex != 0 ||
        hiddenReceipt.glyphByteCount != 0 || hiddenReceipt.lineCount != 0 ||
        hiddenReceipt.glyphBytes[0] != 0) {
        fprintf(stderr,
                "F0107 retained or substituted M648 after real TextString visibility cleared\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    printf("ok: real PC34 F0107 hidden TextString clears M648 map=%d party=(%d,%d) dir=%d text=%d\n",
           pose.mapIndex, pose.partyX, pose.partyY, pose.direction,
           pose.textStringIndex);
    M11_GameView_Shutdown(&state);
    return 0;
}

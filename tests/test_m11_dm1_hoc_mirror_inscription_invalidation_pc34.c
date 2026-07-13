/*
 * Real PC34 HoC M648 invalidation through a C127 mirror open/close.
 *
 * ReDMCSB DUNVIEW.C F0128 rebuilds each tuple and F0107 only draws M648 for
 * the current visible D1C TextString. The C127 branch in F0107:3913-3928
 * instead consumes C346/C026. REVIVE.C F0280/F0282 opens and closes the
 * candidate panel without making the unrelated M648 receipt drawable.
 */

#include "dm1_v1_inscription_font_pc34_compat.h"
#include "m11_game_view.h"
#include "memory_dungeon_dat_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { kFramebufferWidth = 320, kFramebufferHeight = 200 };

typedef struct HocPosePc34 {
    int x;
    int y;
    int direction;
    int textStringIndex;
    int mirrorOrdinal;
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
        state->world.dungeon->header.mapCount == 0) return -1;
    map = &state->world.dungeon->maps[0];
    if (x < 0 || y < 0 || x >= (int)map->width || y >= (int)map->height) return -1;
    index = x * (int)map->height + y;
    return (state->world.dungeon->tiles[0].squareData[index] &
            DUNGEON_SQUARE_MASK_TYPE) >> 5;
}

static int find_hoc_pose(const M11_GameViewState *state, int wantedType,
                         HocPosePc34 *outPose)
{
    const struct DungeonMapDesc_Compat *map;
    int y;
    if (!state || !state->world.dungeon || !state->world.things || !outPose ||
        state->world.dungeon->header.mapCount == 0) return 0;
    map = &state->world.dungeon->maps[0];
    for (y = 0; y < (int)map->height; ++y) {
        int x;
        for (x = 0; x < (int)map->width; ++x) {
            unsigned short thing;
            int safety = 0;
            if (square_element(state, x, y) != DUNGEON_ELEMENT_WALL) continue;
            thing = F0511_DUNGEON_GetSquareFirstThing_Compat(
                state->world.dungeon, state->world.things, 0, x, y);
            while (thing != THING_NONE && thing != THING_ENDOFLIST && safety++ < 64) {
                const int direction = ((int)THING_GET_CELL(thing) + 2) & 3;
                const int partyX = x - direction_dx(direction);
                const int partyY = y - direction_dy(direction);
                if (THING_GET_TYPE(thing) == wantedType &&
                    square_element(state, partyX, partyY) == DUNGEON_ELEMENT_CORRIDOR) {
                    const int index = (int)THING_GET_INDEX(thing);
                    if (wantedType == THING_TYPE_TEXTSTRING &&
                        index >= 0 && index < state->world.things->textStringCount &&
                        state->world.things->textStrings[index].visible) {
                        outPose->x = partyX; outPose->y = partyY;
                        outPose->direction = direction; outPose->textStringIndex = index;
                        outPose->mirrorOrdinal = -1;
                        return 1;
                    }
                    if (wantedType == THING_TYPE_SENSOR &&
                        index >= 0 && index < state->world.things->sensorCount &&
                        state->world.things->sensors[index].sensorType == 127) {
                        outPose->x = partyX; outPose->y = partyY;
                        outPose->direction = direction; outPose->textStringIndex = -1;
                        outPose->mirrorOrdinal = (int)state->world.things->sensors[index].sensorData;
                        return 1;
                    }
                }
                thing = F0512_DUNGEON_GetThingNext_Compat(state->world.things, thing);
            }
        }
    }
    return 0;
}

static void draw_pose(M11_GameViewState *state, const HocPosePc34 *pose,
                      unsigned char *framebuffer)
{
    state->world.party.mapIndex = 0;
    state->world.party.mapX = pose->x;
    state->world.party.mapY = pose->y;
    state->world.party.direction = pose->direction;
    memset(framebuffer, 0, kFramebufferWidth * kFramebufferHeight);
    M11_GameView_Draw(state, framebuffer, kFramebufferWidth, kFramebufferHeight);
}

static int is_real_m648(const M11_Dm1InscriptionHostPresentationReceipt *receipt,
                        int textStringIndex)
{
    return receipt && receipt->valid && receipt->textStringIndex == textStringIndex &&
           receipt->fontGraphicIndex == DM1_V1_INSCRIPTION_FONT_GRAPHIC_INDEX_PC34 &&
           receipt->transparentColor == DM1_V1_INSCRIPTION_TRANSPARENT_COLOR &&
           receipt->glyphByteCount > 0 && receipt->lineCount > 0;
}

static int is_clear_m648(const M11_Dm1InscriptionHostPresentationReceipt *receipt)
{
    return receipt && !receipt->valid && receipt->fontGraphicIndex == 0 &&
           receipt->transparentColor == 0 && receipt->glyphByteCount == 0 &&
           receipt->lineCount == 0 && receipt->glyphBytes[0] == 0;
}

static uint32_t hash_pixels(const unsigned char *pixels, int byteCount)
{
    uint32_t hash = 2166136261u;
    int i;
    for (i = 0; pixels && i < byteCount; ++i) { hash ^= pixels[i]; hash *= 16777619u; }
    return hash;
}

int main(void)
{
    const char *dataDir = getenv("FIRESTAFF_DM1_DATA_DIR");
    const char *home;
    char defaultDataDir[1024];
    M11_GameViewState state;
    HocPosePc34 inscription, mirror;
    M11_Dm1InscriptionHostPresentationReceipt firstReceipt, mirrorReceipt, finalReceipt;
    const M11_AssetSlot *font;
    unsigned char framebuffer[kFramebufferWidth * kFramebufferHeight];
    uint32_t fontHash;

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
        font->height != DM1_V1_INSCRIPTION_FONT_HEIGHT_PC34 ||
        !find_hoc_pose(&state, THING_TYPE_TEXTSTRING, &inscription) ||
        !find_hoc_pose(&state, THING_TYPE_SENSOR, &mirror)) {
        fprintf(stderr, "authentic PC34 HoC M648/C127 corpus route unavailable\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    fontHash = hash_pixels(font->pixels, (int)font->width * (int)font->height);

    draw_pose(&state, &inscription, framebuffer);
    M11_GameView_GetDm1InscriptionHostPresentationReceipt(&firstReceipt);
    if (!is_real_m648(&firstReceipt, inscription.textStringIndex)) goto fail_initial;

    draw_pose(&state, &mirror, framebuffer);
    M11_GameView_GetDm1InscriptionHostPresentationReceipt(&mirrorReceipt);
    if (M11_GameView_GetFrontMirrorOrdinal(&state) != mirror.mirrorOrdinal ||
        !is_clear_m648(&mirrorReceipt)) goto fail_mirror;
    if (!M11_GameView_SelectFrontMirrorCandidate(&state) || !state.candidateMirrorPanelActive)
        goto fail_open;
    draw_pose(&state, &mirror, framebuffer);
    M11_GameView_GetDm1InscriptionHostPresentationReceipt(&mirrorReceipt);
    if (!state.candidateMirrorPanelActive || !is_clear_m648(&mirrorReceipt)) goto fail_open;
    if (!M11_GameView_CancelMirrorCandidate(&state) || state.candidateMirrorPanelActive)
        goto fail_cancel;
    draw_pose(&state, &mirror, framebuffer);
    M11_GameView_GetDm1InscriptionHostPresentationReceipt(&mirrorReceipt);
    if (M11_GameView_GetFrontMirrorOrdinal(&state) != mirror.mirrorOrdinal ||
        !is_clear_m648(&mirrorReceipt)) goto fail_cancel;

    draw_pose(&state, &inscription, framebuffer);
    M11_GameView_GetDm1InscriptionHostPresentationReceipt(&finalReceipt);
    if (!is_real_m648(&finalReceipt, inscription.textStringIndex) ||
        finalReceipt.glyphByteCount != firstReceipt.glyphByteCount ||
        memcmp(finalReceipt.glyphBytes, firstReceipt.glyphBytes,
               (size_t)firstReceipt.glyphByteCount) != 0 ||
        fontHash != hash_pixels(font->pixels, (int)font->width * (int)font->height)) goto fail_restore;
    printf("ok: real PC34 HoC C127 open/cancel invalidates M648/C10 and restores text=%d mirror=%d\n",
           inscription.textStringIndex, mirror.mirrorOrdinal);
    M11_GameView_Shutdown(&state);
    return 0;

fail_initial: fprintf(stderr, "initial HoC F0107 frame lacks real M648/C10 material\n"); goto fail;
fail_mirror: fprintf(stderr, "C127 mirror frame retained unrelated M648 material\n"); goto fail;
fail_open: fprintf(stderr, "open C127 panel retained M648/C10 material\n"); goto fail;
fail_cancel: fprintf(stderr, "closed C127 panel retained M648/C10 material\n"); goto fail;
fail_restore: fprintf(stderr, "F0128 did not restore original M648/C10 after mirror close\n");
fail:
    M11_GameView_Shutdown(&state);
    return 1;
}

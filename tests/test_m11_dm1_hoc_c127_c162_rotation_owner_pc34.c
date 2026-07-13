/* Real PC34 HoC C127/C162 ownership across a second mirror view.
 * REVIVE.C F0280 appends one candidate before its panel opens; F0282 C162
 * removes that appended slot only. DUNVIEW.C F0128/F0107 rebuilds M648 per
 * tuple, while C127 uses the C346/C026 mirror branch. */

#include "dm1_v1_inscription_font_pc34_compat.h"
#include "m11_game_view.h"
#include "memory_dungeon_dat_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { kFramebufferWidth = 320, kFramebufferHeight = 200 };
typedef struct HocPosePc34 { int x, y, direction, ordinal, textIndex; } HocPosePc34;

static int dx(int direction) { return (direction & 3) == 1 ? 1 : ((direction & 3) == 3 ? -1 : 0); }
static int dy(int direction) { return (direction & 3) == 2 ? 1 : ((direction & 3) == 0 ? -1 : 0); }

static int square_element(const M11_GameViewState *state, int x, int y)
{
    const struct DungeonMapDesc_Compat *map;
    int index;
    if (!state || !state->world.dungeon || !state->world.dungeon->tiles ||
        state->world.dungeon->header.mapCount == 0) return -1;
    map = &state->world.dungeon->maps[0];
    if (x < 0 || y < 0 || x >= (int)map->width || y >= (int)map->height) return -1;
    index = x * (int)map->height + y;
    return (state->world.dungeon->tiles[0].squareData[index] & DUNGEON_SQUARE_MASK_TYPE) >> 5;
}

static void draw_at(M11_GameViewState *state, const HocPosePc34 *pose,
                    unsigned char *framebuffer)
{
    state->world.party.mapIndex = 0;
    state->world.party.mapX = pose->x;
    state->world.party.mapY = pose->y;
    state->world.party.direction = pose->direction;
    memset(framebuffer, 0, kFramebufferWidth * kFramebufferHeight);
    M11_GameView_Draw(state, framebuffer, kFramebufferWidth, kFramebufferHeight);
}

static int is_real_m648(const M11_Dm1InscriptionHostPresentationReceipt *receipt)
{
    return receipt && receipt->valid && receipt->textStringIndex >= 0 &&
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

static int find_text_pose(M11_GameViewState *state, HocPosePc34 *out,
                          unsigned char *framebuffer,
                          M11_Dm1InscriptionHostPresentationReceipt *receipt)
{
    const struct DungeonMapDesc_Compat *map = &state->world.dungeon->maps[0];
    int y;
    for (y = 0; y < (int)map->height; ++y) {
        int x;
        for (x = 0; x < (int)map->width; ++x) {
            int direction;
            if (square_element(state, x, y) != DUNGEON_ELEMENT_CORRIDOR) continue;
            for (direction = 0; direction < 4; ++direction) {
                out->x = x; out->y = y; out->direction = direction;
                draw_at(state, out, framebuffer);
                M11_GameView_GetDm1InscriptionHostPresentationReceipt(receipt);
                if (is_real_m648(receipt)) {
                    out->textIndex = receipt->textStringIndex;
                    out->ordinal = -1;
                    return 1;
                }
            }
        }
    }
    return 0;
}

static int find_two_mirrors(const M11_GameViewState *state,
                            HocPosePc34 *first, HocPosePc34 *second)
{
    const struct DungeonMapDesc_Compat *map;
    int y;
    int found = 0;
    if (!state || !state->world.dungeon || !state->world.things || !first || !second) return 0;
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
                if (THING_GET_TYPE(thing) == THING_TYPE_SENSOR) {
                    const int sensorIndex = (int)THING_GET_INDEX(thing);
                    const int direction = ((int)THING_GET_CELL(thing) + 2) & 3;
                    const int partyX = x - dx(direction);
                    const int partyY = y - dy(direction);
                    if (sensorIndex >= 0 && sensorIndex < state->world.things->sensorCount &&
                        state->world.things->sensors[sensorIndex].sensorType == 127 &&
                        square_element(state, partyX, partyY) == DUNGEON_ELEMENT_CORRIDOR) {
                        HocPosePc34 *out;
                        if (found > 0 &&
                            (int)state->world.things->sensors[sensorIndex].sensorData ==
                                first->ordinal) {
                            thing = F0512_DUNGEON_GetThingNext_Compat(state->world.things, thing);
                            continue;
                        }
                        out = found == 0 ? first : second;
                        out->x = partyX; out->y = partyY; out->direction = direction;
                        out->ordinal = (int)state->world.things->sensors[sensorIndex].sensorData;
                        out->textIndex = -1;
                        if (++found == 2) return 1;
                    }
                }
                thing = F0512_DUNGEON_GetThingNext_Compat(state->world.things, thing);
            }
        }
    }
    return 0;
}

int main(void)
{
    const char *dataDir = getenv("FIRESTAFF_DM1_DATA_DIR");
    const char *home;
    char defaultDataDir[1024];
    M11_GameViewState state;
    HocPosePc34 text, mirrorA, mirrorB;
    M11_Dm1InscriptionHostPresentationReceipt first, receipt, restored;
    const M11_AssetSlot *font;
    unsigned char framebuffer[kFramebufferWidth * kFramebufferHeight];
    uint32_t fontHash;
    int partyCount;
    int candidateIndex;

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
    font = M11_AssetLoader_Load(&state.assetLoader, DM1_V1_INSCRIPTION_FONT_GRAPHIC_INDEX_PC34);
    if (!font || !font->loaded || !font->pixels ||
        font->width != DM1_V1_INSCRIPTION_FONT_WIDTH_PC34 ||
        font->height != DM1_V1_INSCRIPTION_FONT_HEIGHT_PC34 ||
        !find_text_pose(&state, &text, framebuffer, &first) ||
        !find_two_mirrors(&state, &mirrorA, &mirrorB)) goto unavailable;
    fontHash = hash_pixels(font->pixels, (int)font->width * (int)font->height);
    partyCount = state.world.party.championCount;

    draw_at(&state, &mirrorA, framebuffer);
    M11_GameView_GetDm1InscriptionHostPresentationReceipt(&receipt);
    if (M11_GameView_GetFrontMirrorOrdinal(&state) != mirrorA.ordinal || !is_clear_m648(&receipt)) goto fail_a;
    if (!M11_GameView_SelectFrontMirrorCandidate(&state) || !state.candidateMirrorPanelActive ||
        state.candidateMirrorOrdinal != mirrorA.ordinal) goto fail_a;
    candidateIndex = state.candidateMirrorPartyIndex;
    if (candidateIndex < 0 || candidateIndex >= CHAMPION_MAX_PARTY ||
        !state.world.party.champions[candidateIndex].present) goto fail_a;

    draw_at(&state, &mirrorB, framebuffer);
    M11_GameView_GetDm1InscriptionHostPresentationReceipt(&receipt);
    if (M11_GameView_GetFrontMirrorOrdinal(&state) != mirrorB.ordinal ||
        !state.candidateMirrorPanelActive || state.candidateMirrorOrdinal != mirrorA.ordinal ||
        state.candidateMirrorPartyIndex != candidateIndex || !is_clear_m648(&receipt)) goto fail_b;
    if (!M11_GameView_CancelMirrorCandidate(&state) || state.candidateMirrorPanelActive ||
        state.candidateMirrorOrdinal != -1 || state.candidateMirrorPartyIndex != -1 ||
        state.world.party.championCount != partyCount ||
        state.world.party.champions[candidateIndex].present) goto fail_cancel;

    draw_at(&state, &mirrorB, framebuffer);
    M11_GameView_GetDm1InscriptionHostPresentationReceipt(&receipt);
    if (M11_GameView_GetFrontMirrorOrdinal(&state) != mirrorB.ordinal || !is_clear_m648(&receipt) ||
        !M11_GameView_SelectFrontMirrorCandidate(&state) ||
        state.candidateMirrorOrdinal != mirrorB.ordinal) goto fail_b;
    if (!M11_GameView_CancelMirrorCandidate(&state)) goto fail_cancel;

    draw_at(&state, &text, framebuffer);
    M11_GameView_GetDm1InscriptionHostPresentationReceipt(&restored);
    if (!is_real_m648(&restored) || restored.textStringIndex != text.textIndex ||
        restored.glyphByteCount != first.glyphByteCount ||
        memcmp(restored.glyphBytes, first.glyphBytes, (size_t)first.glyphByteCount) != 0 ||
        fontHash != hash_pixels(font->pixels, (int)font->width * (int)font->height)) goto fail_restore;
    printf("ok: real PC34 C162 clears only C127=%d while rotated to C127=%d; M648/C10 restores text=%d\n",
           mirrorA.ordinal, mirrorB.ordinal, text.textIndex);
    M11_GameView_Shutdown(&state);
    return 0;
unavailable: fprintf(stderr, "authentic PC34 HoC C127/C162 corpus route unavailable\n"); goto fail;
fail_a: fprintf(stderr, "source C127 panel/material route failed\n"); goto fail;
fail_b: fprintf(stderr, "rotated C127 stole candidate ownership or M648 receipt\n"); goto fail;
fail_cancel: fprintf(stderr, "C162 did not clear only source candidate\n"); goto fail;
fail_restore: fprintf(stderr, "M648/C10 did not restore after C162\n");
fail: M11_GameView_Shutdown(&state); return 1;
}

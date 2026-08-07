/* Real PC34 HoC C160 -> ordinary non-inscription redraw regression.
 * REVIVE.C F0282 closes C040 and clears its candidate; DUNVIEW.C F0128
 * rebuilds the next viewport tuple before F0107 may publish M648/C10. */

#include "dm1_v1_inscription_font_pc34_compat.h"
#include "m11_game_view.h"
#include "memory_dungeon_dat_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { kFramebufferWidth = 320, kFramebufferHeight = 200 };
typedef struct HocPosePc34 { int x, y, direction, ordinal; } HocPosePc34;

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
           receipt->glyphByteCount > 0;
}

static int is_clear_m648(const M11_Dm1InscriptionHostPresentationReceipt *receipt)
{
    return receipt && !receipt->valid && receipt->fontGraphicIndex == 0 &&
           receipt->transparentColor == 0 && receipt->glyphByteCount == 0 &&
           receipt->lineCount == 0 && receipt->glyphBytes[0] == 0;
}

static int is_clear_unreadable(const M11_Dm1UnreadableInscriptionHostPresentationReceipt *receipt)
{
    return receipt && !receipt->valid && receipt->textStringIndex == 0 &&
           receipt->graphicIndex == 0 && receipt->transparentColor == 0 &&
           receipt->lineCount == 0 && receipt->width == 0 && receipt->height == 0;
}

static uint32_t hash_pixels(const unsigned char *pixels, int byteCount)
{
    uint32_t hash = 2166136261u;
    int i;
    for (i = 0; pixels && i < byteCount; ++i) { hash ^= pixels[i]; hash *= 16777619u; }
    return hash;
}

static int find_text_pose(M11_GameViewState *state, HocPosePc34 *out,
                          unsigned char *framebuffer)
{
    const struct DungeonMapDesc_Compat *map = &state->world.dungeon->maps[0];
    M11_Dm1InscriptionHostPresentationReceipt receipt;
    int y;
    for (y = 0; y < (int)map->height; ++y) {
        int x;
        for (x = 0; x < (int)map->width; ++x) {
            int direction;
            if (square_element(state, x, y) != DUNGEON_ELEMENT_CORRIDOR) continue;
            for (direction = 0; direction < 4; ++direction) {
                out->x = x; out->y = y; out->direction = direction;
                draw_at(state, out, framebuffer);
                M11_GameView_GetDm1InscriptionHostPresentationReceipt(&receipt);
                if (is_real_m648(&receipt)) return 1;
            }
        }
    }
    return 0;
}

static int find_clear_corridor_pose(M11_GameViewState *state, HocPosePc34 *out,
                                    unsigned char *framebuffer)
{
    const struct DungeonMapDesc_Compat *map = &state->world.dungeon->maps[0];
    M11_Dm1InscriptionHostPresentationReceipt primary;
    M11_Dm1UnreadableInscriptionHostPresentationReceipt unreadable;
    int y;
    for (y = 0; y < (int)map->height; ++y) {
        int x;
        for (x = 0; x < (int)map->width; ++x) {
            int direction;
            if (square_element(state, x, y) != DUNGEON_ELEMENT_CORRIDOR) continue;
            for (direction = 0; direction < 4; ++direction) {
                out->x = x; out->y = y; out->direction = direction;
                draw_at(state, out, framebuffer);
                M11_GameView_GetDm1InscriptionHostPresentationReceipt(&primary);
                M11_GameView_GetDm1UnreadableInscriptionHostPresentationReceipt(&unreadable);
                if (is_clear_m648(&primary) && is_clear_unreadable(&unreadable)) return 1;
            }
        }
    }
    return 0;
}

static int find_mirror_pose(const M11_GameViewState *state, HocPosePc34 *out)
{
    const struct DungeonMapDesc_Compat *map;
    int y;
    if (!state || !state->world.dungeon || !state->world.things || !out) return 0;
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
                    int sensorIndex = (int)THING_GET_INDEX(thing);
                    int direction = ((int)THING_GET_CELL(thing) + 2) & 3;
                    int partyX = x - dx(direction);
                    int partyY = y - dy(direction);
                    if (sensorIndex >= 0 && sensorIndex < state->world.things->sensorCount &&
                        state->world.things->sensors[sensorIndex].sensorType == 127 &&
                        square_element(state, partyX, partyY) == DUNGEON_ELEMENT_CORRIDOR) {
                        out->x = partyX; out->y = partyY; out->direction = direction;
                        out->ordinal = (int)state->world.things->sensors[sensorIndex].sensorData;
                        return 1;
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
    M11_GameViewState state;
    HocPosePc34 text, clearCorridor, mirror;
    M11_Dm1InscriptionHostPresentationReceipt primary;
    M11_Dm1UnreadableInscriptionHostPresentationReceipt unreadable;
    const M11_AssetSlot *font;
    unsigned char framebuffer[kFramebufferWidth * kFramebufferHeight];
    uint32_t fontHash;
    int partyCount;
    int candidateIndex;

    if (!dataDir || !dataDir[0]) {
        puts("SKIP: FIRESTAFF_DM1_DATA_DIR is not selected");
        return 0;
    }
    M11_GameView_Init(&state);
    if (!M11_GameView_StartDm1(&state, dataDir)) {
        M11_GameView_Shutdown(&state);
        fputs("configured PC34 corpus could not start\n", stderr);
        return 1;
    }
    state.presentationMode = M12_PRESENTATION_V1_ORIGINAL;
    state.world.party.championCount = 0;
    font = M11_AssetLoader_Load(&state.assetLoader,
                                DM1_V1_INSCRIPTION_FONT_GRAPHIC_INDEX_PC34);
    if (!font || !font->loaded || !font->pixels ||
        !find_text_pose(&state, &text, framebuffer) ||
        !find_clear_corridor_pose(&state, &clearCorridor, framebuffer) ||
        !find_mirror_pose(&state, &mirror)) goto unavailable;
    fontHash = hash_pixels(font->pixels, (int)font->width * (int)font->height);
    partyCount = state.world.party.championCount;

    /* Establish real M648/C10 material before the C127 panel takes ownership. */
    draw_at(&state, &text, framebuffer);
    M11_GameView_GetDm1InscriptionHostPresentationReceipt(&primary);
    if (!is_real_m648(&primary)) goto fail_setup;
    draw_at(&state, &mirror, framebuffer);
    M11_GameView_GetDm1InscriptionHostPresentationReceipt(&primary);
    if (!is_clear_m648(&primary) ||
        M11_GameView_GetFrontMirrorOrdinal(&state) != mirror.ordinal ||
        !M11_GameView_SelectFrontMirrorCandidate(&state)) goto fail_panel;
    candidateIndex = state.candidateMirrorPartyIndex;
    if (!state.candidateMirrorPanelActive || candidateIndex < 0 ||
        candidateIndex >= CHAMPION_MAX_PARTY || !state.world.party.champions[candidateIndex].present ||
        state.world.party.championCount != partyCount + 1) goto fail_panel;

    M11_GameView_Draw(&state, framebuffer, kFramebufferWidth, kFramebufferHeight);
    M11_GameView_GetDm1InscriptionHostPresentationReceipt(&primary);
    if (!is_clear_m648(&primary) || !state.candidateMirrorPanelActive) goto fail_panel;
    if (!M11_GameView_ConfirmMirrorCandidate(&state, 0) ||
        state.candidateMirrorPanelActive || state.candidateMirrorOrdinal != -1 ||
        state.candidateMirrorPartyIndex != -1 ||
        state.world.party.championCount != partyCount + 1 ||
        !state.world.party.champions[candidateIndex].present) goto fail_close;

    /* First viewport tuple after C160 targets a real corridor with no text. */
    draw_at(&state, &clearCorridor, framebuffer);
    M11_GameView_GetDm1InscriptionHostPresentationReceipt(&primary);
    M11_GameView_GetDm1UnreadableInscriptionHostPresentationReceipt(&unreadable);
    if (!is_clear_m648(&primary) || !is_clear_unreadable(&unreadable) ||
        fontHash != hash_pixels(font->pixels, (int)font->width * (int)font->height)) goto fail_clear;

    printf("ok: real PC34 C160 direct non-inscription corridor redraw clears M648/C10\n");
    M11_GameView_Shutdown(&state);
    return 0;
unavailable: fprintf(stderr, "authentic PC34 HoC C127/corridor corpus route unavailable\n"); goto fail;
fail_setup: fprintf(stderr, "could not establish real pre-C127 M648/C10 material\n"); goto fail;
fail_panel: fprintf(stderr, "C127 C040 panel did not clear M648/C10 material\n"); goto fail;
fail_close: fprintf(stderr, "C160 did not close only the real C127 candidate\n"); goto fail;
fail_clear: fprintf(stderr, "post-C160 non-inscription corridor retained stale M648/C10 material\n");
fail: M11_GameView_Shutdown(&state); return 1;
}

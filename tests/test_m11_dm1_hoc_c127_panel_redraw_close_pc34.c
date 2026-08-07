/* Real PC34 HoC C127 panel-close redraw regression.
 * ReDMCSB REVIVE.C F0282 C160 closes C040 after consuming its C127 source.
 * DUNVIEW.C F0128/F0107 then composes a fresh tuple: C346/C026 or an empty
 * wall must not inherit the prior M648/C10 inscription material. */

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
                          M11_Dm1InscriptionHostPresentationReceipt *receipt,
                          int excludedTextStringIndex)
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
                if (is_real_m648(receipt) &&
                    receipt->textStringIndex != excludedTextStringIndex) {
                    out->ordinal = -1;
                    out->textIndex = receipt->textStringIndex;
                    return 1;
                }
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
                        out->textIndex = -1;
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
    HocPosePc34 text, nextText, mirror;
    M11_Dm1InscriptionHostPresentationReceipt before, nextExpected, receipt,
        postClose, restored;
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
        return 1;
    }
    state.presentationMode = M12_PRESENTATION_V1_ORIGINAL;
    state.world.party.championCount = 0;
    font = M11_AssetLoader_Load(&state.assetLoader,
                                DM1_V1_INSCRIPTION_FONT_GRAPHIC_INDEX_PC34);
    if (!font || !font->loaded || !font->pixels ||
        font->width != DM1_V1_INSCRIPTION_FONT_WIDTH_PC34 ||
        font->height != DM1_V1_INSCRIPTION_FONT_HEIGHT_PC34 ||
        !find_text_pose(&state, &text, framebuffer, &before, -1) ||
        !find_text_pose(&state, &nextText, framebuffer, &nextExpected,
                        text.textIndex) ||
        !find_mirror_pose(&state, &mirror)) goto unavailable;
    fontHash = hash_pixels(font->pixels, (int)font->width * (int)font->height);
    partyCount = state.world.party.championCount;

    draw_at(&state, &mirror, framebuffer);
    M11_GameView_GetDm1InscriptionHostPresentationReceipt(&receipt);
    if (M11_GameView_GetFrontMirrorOrdinal(&state) != mirror.ordinal ||
        !is_clear_m648(&receipt) || !M11_GameView_SelectFrontMirrorCandidate(&state) ||
        !state.candidateMirrorPanelActive) goto fail_panel;
    candidateIndex = state.candidateMirrorPartyIndex;
    if (candidateIndex < 0 || candidateIndex >= CHAMPION_MAX_PARTY ||
        !state.world.party.champions[candidateIndex].present ||
        state.world.party.championCount != partyCount + 1) goto fail_panel;

    /* This is the ordinary panel redraw, before REVIVE.C F0282 consumes C160. */
    M11_GameView_Draw(&state, framebuffer, kFramebufferWidth, kFramebufferHeight);
    M11_GameView_GetDm1InscriptionHostPresentationReceipt(&receipt);
    if (!state.candidateMirrorPanelActive || state.candidateMirrorPartyIndex != candidateIndex ||
        !state.world.party.champions[candidateIndex].present || !is_clear_m648(&receipt)) goto fail_panel;

    if (!M11_GameView_ConfirmMirrorCandidate(&state, 0) ||
        state.candidateMirrorPanelActive || state.candidateMirrorOrdinal != -1 ||
        state.candidateMirrorPartyIndex != -1 ||
        state.world.party.championCount != partyCount + 1 ||
        !state.world.party.champions[candidateIndex].present) goto fail_close;
    /* This is the first ordinary DUNVIEW redraw after C160: it goes straight
     * to another corpus-backed inscription. F0107 must decode this current
     * TextString with M648/C10 rather than retain the pre-panel glyph run. */
    draw_at(&state, &nextText, framebuffer);
    M11_GameView_GetDm1InscriptionHostPresentationReceipt(&receipt);
    if (!is_real_m648(&receipt) || receipt.textStringIndex != nextText.textIndex ||
        receipt.textStringIndex == text.textIndex ||
        receipt.glyphByteCount != nextExpected.glyphByteCount ||
        memcmp(receipt.glyphBytes, nextExpected.glyphBytes,
               (size_t)nextExpected.glyphByteCount) != 0 ||
        fontHash != hash_pixels(font->pixels, (int)font->width * (int)font->height)) {
        goto fail_direct;
    }
    /* The next viewport redraw is not C162: it is the post-C160 normal DUNVIEW
     * frame. The source mirror wall used by the local PC34 corpus carries no
     * visible C02 TextString after the C127 sensor is disabled, so F0128/F0107
     * must publish clear-only M648 state here rather than retaining the prior
     * inscription or inventing a host/fallback glyph run. */
    draw_at(&state, &mirror, framebuffer);
    M11_GameView_GetDm1InscriptionHostPresentationReceipt(&receipt);
    if (!is_clear_m648(&receipt) ||
        M11_GameView_GetFrontMirrorOrdinal(&state) != -1) goto fail_close;
    postClose = receipt;
    draw_at(&state, &mirror, framebuffer);
    M11_GameView_GetDm1InscriptionHostPresentationReceipt(&receipt);
    if (!is_clear_m648(&receipt) ||
        receipt.glyphByteCount != postClose.glyphByteCount ||
        receipt.lineCount != postClose.lineCount) goto fail_close;

    draw_at(&state, &text, framebuffer);
    M11_GameView_GetDm1InscriptionHostPresentationReceipt(&restored);
    if (!is_real_m648(&restored) || restored.textStringIndex != text.textIndex ||
        restored.glyphByteCount != before.glyphByteCount ||
        memcmp(restored.glyphBytes, before.glyphBytes, (size_t)before.glyphByteCount) != 0 ||
        fontHash != hash_pixels(font->pixels, (int)font->width * (int)font->height)) goto fail_restore;
    printf("ok: real PC34 C127 C040 redraw/C160 close clears mirror M648 and repaints source text=%d\\n",
           nextText.textIndex);
    M11_GameView_Shutdown(&state);
    return 0;
unavailable: fprintf(stderr, "authentic PC34 HoC C127/M648 corpus route unavailable\\n"); goto fail;
fail_panel: fprintf(stderr, "C127 panel redraw retained or replaced M648/C10 material\\n"); goto fail;
fail_close: fprintf(stderr, "ordinary C160 close did not rebuild M648/C10 from a distinct PC34 TextString\\n"); goto fail;
fail_direct: fprintf(stderr, "direct post-C160 inscription redraw retained old M648/C10 glyph material\\n"); goto fail;
fail_restore: fprintf(stderr, "M648/C10 did not restore after ordinary close redraw\\n");
fail: M11_GameView_Shutdown(&state); return 1;
}

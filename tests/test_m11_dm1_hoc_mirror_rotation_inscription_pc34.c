/* Real PC34 HoC M648/C10 state transition across a C127 turn.
 * ReDMCSB DUNVIEW.C F0128 rebuilds the tuple after every direction change.
 * F0107 draws M648 only for the D1C inscription branch (3619-3706), while
 * C127 uses C346/C026 (3913-3928). */

#include "dm1_v1_inscription_font_pc34_compat.h"
#include "m11_game_view.h"
#include "memory_dungeon_dat_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { kFramebufferWidth = 320, kFramebufferHeight = 200 };

typedef struct HocPosePc34 { int x, y, direction, textIndex, mirrorOrdinal; } HocPosePc34;

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

static int find_real_text_pose(M11_GameViewState *state, HocPosePc34 *pose,
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
                pose->x = x; pose->y = y; pose->direction = direction;
                draw_at(state, pose, framebuffer);
                M11_GameView_GetDm1InscriptionHostPresentationReceipt(receipt);
                if (is_real_m648(receipt)) {
                    pose->textIndex = receipt->textStringIndex;
                    pose->mirrorOrdinal = -1;
                    return 1;
                }
            }
        }
    }
    return 0;
}

static int find_real_mirror_turn(M11_GameViewState *state, HocPosePc34 *mirror,
                                 HocPosePc34 *away, unsigned char *framebuffer)
{
    const struct DungeonMapDesc_Compat *map = &state->world.dungeon->maps[0];
    int y;
    for (y = 0; y < (int)map->height; ++y) {
        int x;
        for (x = 0; x < (int)map->width; ++x) {
            int direction;
            if (square_element(state, x, y) != DUNGEON_ELEMENT_CORRIDOR) continue;
            for (direction = 0; direction < 4; ++direction) {
                M11_Dm1InscriptionHostPresentationReceipt receipt;
                int turn;
                mirror->x = x; mirror->y = y; mirror->direction = direction;
                draw_at(state, mirror, framebuffer);
                M11_GameView_GetDm1InscriptionHostPresentationReceipt(&receipt);
                mirror->mirrorOrdinal = M11_GameView_GetFrontMirrorOrdinal(state);
                if (mirror->mirrorOrdinal < 0 || !is_clear_m648(&receipt)) continue;
                for (turn = 1; turn < 4; ++turn) {
                    away->x = x; away->y = y; away->direction = (direction + turn) & 3;
                    draw_at(state, away, framebuffer);
                    M11_GameView_GetDm1InscriptionHostPresentationReceipt(&receipt);
                    if (M11_GameView_GetFrontMirrorOrdinal(state) < 0 && is_clear_m648(&receipt)) {
                        mirror->textIndex = -1;
                        away->textIndex = -1; away->mirrorOrdinal = -1;
                        return 1;
                    }
                }
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
    HocPosePc34 text, mirror, away;
    M11_Dm1InscriptionHostPresentationReceipt first, receipt, restored;
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
    font = M11_AssetLoader_Load(&state.assetLoader, DM1_V1_INSCRIPTION_FONT_GRAPHIC_INDEX_PC34);
    if (!font || !font->loaded || !font->pixels ||
        font->width != DM1_V1_INSCRIPTION_FONT_WIDTH_PC34 ||
        font->height != DM1_V1_INSCRIPTION_FONT_HEIGHT_PC34 ||
        !find_real_text_pose(&state, &text, framebuffer, &first) ||
        !find_real_mirror_turn(&state, &mirror, &away, framebuffer)) {
        fprintf(stderr, "authentic PC34 HoC text/C127 rotation route unavailable\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    fontHash = hash_pixels(font->pixels, (int)font->width * (int)font->height);
    draw_at(&state, &mirror, framebuffer);
    M11_GameView_GetDm1InscriptionHostPresentationReceipt(&receipt);
    if (M11_GameView_GetFrontMirrorOrdinal(&state) != mirror.mirrorOrdinal || !is_clear_m648(&receipt)) goto fail_mirror;
    draw_at(&state, &away, framebuffer);
    M11_GameView_GetDm1InscriptionHostPresentationReceipt(&receipt);
    if (!is_clear_m648(&receipt)) goto fail_away;
    draw_at(&state, &mirror, framebuffer);
    M11_GameView_GetDm1InscriptionHostPresentationReceipt(&receipt);
    if (M11_GameView_GetFrontMirrorOrdinal(&state) != mirror.mirrorOrdinal || !is_clear_m648(&receipt)) goto fail_mirror;
    draw_at(&state, &text, framebuffer);
    M11_GameView_GetDm1InscriptionHostPresentationReceipt(&restored);
    if (!is_real_m648(&restored) || restored.textStringIndex != text.textIndex ||
        restored.glyphByteCount != first.glyphByteCount ||
        memcmp(restored.glyphBytes, first.glyphBytes, (size_t)first.glyphByteCount) != 0 ||
        fontHash != hash_pixels(font->pixels, (int)font->width * (int)font->height)) goto fail_restore;
    printf("ok: real PC34 HoC M648 text=%d survives C127=%d rotation and return\n",
           text.textIndex, mirror.mirrorOrdinal);
    M11_GameView_Shutdown(&state);
    return 0;
fail_mirror: fprintf(stderr, "C127 rotation frame retained M648/C10 receipt\n"); goto fail;
fail_away: fprintf(stderr, "turn past C127 retained M648/C10 receipt\n"); goto fail;
fail_restore: fprintf(stderr, "return did not rebuild original M648/C10 material\n");
fail: M11_GameView_Shutdown(&state); return 1;
}

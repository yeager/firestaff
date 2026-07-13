/*
 * Full real-PC34 F0107/F0124 inscription stale-state regression.
 *
 * ReDMCSB DUNGEON.C F0172:2573-2600 selects the current visible wall
 * TextString. DUNVIEW.C F0107:3590-3706 consumes only that F0168 decode via
 * M648 with C10 transparency, and F0124:7842-7845 invokes the D1C route.
 * Each frame must therefore replace, not retain, the prior inscription's
 * glyph material and palette index.
 */

#include "dm1_v1_inscription_font_pc34_compat.h"
#include "m11_game_view.h"
#include "memory_dungeon_dat_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
    kFramebufferWidth = 320,
    kFramebufferHeight = 200,
    kViewportY = 33,
    kMaxVisibleInscriptionPoses = 128
};

typedef struct VisibleInscriptionPosePc34 {
    int mapIndex;
    int partyX;
    int partyY;
    int direction;
    int textStringIndex;
} VisibleInscriptionPosePc34;

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

static int append_visible_poses(const M11_GameViewState *state,
                                VisibleInscriptionPosePc34 *poses,
                                int capacity)
{
    int mapIndex;
    int count = 0;

    if (!state || !state->world.dungeon || !state->world.things || !poses ||
        capacity <= 0) {
        return -1;
    }
    for (mapIndex = 0; mapIndex < (int)state->world.dungeon->header.mapCount;
         ++mapIndex) {
        const struct DungeonMapDesc_Compat *map = &state->world.dungeon->maps[mapIndex];
        int y;
        for (y = 0; y < (int)map->height; ++y) {
            int x;
            for (x = 0; x < (int)map->width; ++x) {
                unsigned short thing;
                int safety = 0;

                if (square_element(state, mapIndex, x, y) != DUNGEON_ELEMENT_WALL) {
                    continue;
                }
                thing = F0511_DUNGEON_GetSquareFirstThing_Compat(
                    state->world.dungeon, state->world.things, mapIndex, x, y);
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
                            if (square_element(state, mapIndex, partyX, partyY) !=
                                DUNGEON_ELEMENT_CORRIDOR) {
                                continue;
                            }
                            if (count >= capacity) {
                                return -1;
                            }
                            poses[count].mapIndex = mapIndex;
                            poses[count].partyX = partyX;
                            poses[count].partyY = partyY;
                            poses[count].direction = direction;
                            poses[count].textStringIndex = textIndex;
                            ++count;
                        }
                    }
                    thing = F0512_DUNGEON_GetThingNext_Compat(
                        state->world.things, thing);
                }
            }
        }
    }
    return count;
}

static uint32_t hash_pixels(const unsigned char *pixels, int byteCount)
{
    uint32_t hash = 2166136261u;
    int i;

    for (i = 0; pixels && i < byteCount; ++i) {
        hash ^= pixels[i];
        hash *= 16777619u;
    }
    return hash;
}

static int verify_receipt_and_glyph_pixels(
    const M11_GameViewState *state,
    const VisibleInscriptionPosePc34 *pose,
    const unsigned char *framebuffer,
    const M11_AssetSlot *font)
{
    M11_Dm1InscriptionHostPresentationReceipt receipt;
    unsigned char decoded[DM1_V1_INSCRIPTION_HOST_MATERIAL_MAX_GLYPHS_PC34];
    int decodedCount;
    int cursor = 0;
    int expectedLineCount = 0;
    int line;

    if (!state || !pose || !framebuffer || !font || !font->pixels ||
        !state->world.things) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    M11_GameView_GetDm1InscriptionHostPresentationReceipt(&receipt);
    decodedCount = DM1_V1_InscriptionDecodeRawGlyphsFromWordsPc34(
        state->world.things->textData, state->world.things->textDataWordCount,
        state->world.things->textStrings[pose->textStringIndex].textDataWordOffset,
        decoded, (int)sizeof(decoded));
    if (!receipt.valid || receipt.textStringIndex != pose->textStringIndex ||
        receipt.fontGraphicIndex != DM1_V1_INSCRIPTION_FONT_GRAPHIC_INDEX_PC34 ||
        receipt.transparentColor != DM1_V1_INSCRIPTION_TRANSPARENT_COLOR ||
        decodedCount <= 1 || decoded[decodedCount - 1] != 0x81U ||
        receipt.glyphByteCount != decodedCount - 1 ||
        memcmp(receipt.glyphBytes, decoded, (size_t)receipt.glyphByteCount) != 0) {
        fprintf(stderr,
                "receipt mismatch text=%d valid=%d actual=(text=%d glyphs=%d lines=%d) decoded=%d\n",
                pose->textStringIndex, receipt.valid, receipt.textStringIndex,
                receipt.glyphByteCount, receipt.lineCount, decodedCount);
        return 0;
    }
    for (line = 0; line < DM1_V1_INSCRIPTION_MAX_LINES; ++line) {
        DM1_V1_InscriptionFrontWallLineDrawPlanPc34 plan;
        int glyphOffset;

        if (!DM1_V1_InscriptionBuildFrontWallLineDrawPlanPc34(
                decoded, (int)sizeof(decoded), cursor, line, 160, 111, &plan) ||
            plan.glyphCount < 0) {
            fprintf(stderr,
                    "line plan failed text=%d line=%d\n",
                    pose->textStringIndex, line);
            return 0;
        }
        if (plan.glyphCount > 0) {
            ++expectedLineCount;
        }
        if (plan.glyphCount <= 0) {
            if (plan.done) {
                break;
            }
            cursor = plan.nextCursor;
            continue;
        }
        if (receipt.lineGlyphCount[line] != plan.glyphCount ||
            receipt.lineDestinationX[line] != plan.textX ||
            receipt.lineDestinationY[line] != kViewportY + plan.textY) {
            fprintf(stderr,
                    "line mismatch text=%d line=%d actual=(glyphs=%d x=%d y=%d) plan=(glyphs=%d x=%d y=%d)\n",
                    pose->textStringIndex, line, receipt.lineGlyphCount[line],
                    receipt.lineDestinationX[line], receipt.lineDestinationY[line],
                    plan.glyphCount, plan.textX, kViewportY + plan.textY);
            return 0;
        }
        for (glyphOffset = 0; glyphOffset < plan.glyphCount; ++glyphOffset) {
            int glyph = DM1_V1_InscriptionGlyphIndexFromSourceByte(
                decoded[plan.glyphStart + glyphOffset]);
            int yy;
            if (glyph < 0) {
                return 0;
            }
            for (yy = 0; yy < DM1_V1_INSCRIPTION_GLYPH_HEIGHT; ++yy) {
                int xx;
                for (xx = 0; xx < DM1_V1_INSCRIPTION_GLYPH_WIDTH; ++xx) {
                    const unsigned char source = font->pixels[
                        yy * font->width + glyph * DM1_V1_INSCRIPTION_GLYPH_WIDTH + xx];
                    const int pixel = (kViewportY + plan.textY + yy) *
                            kFramebufferWidth + plan.textX +
                        glyphOffset * DM1_V1_INSCRIPTION_GLYPH_WIDTH + xx;
                    if (source != DM1_V1_INSCRIPTION_TRANSPARENT_COLOR &&
                        framebuffer[pixel] != source) {
                        fprintf(stderr,
                                "glyph mismatch text=%d line=%d glyph=%d xy=(%d,%d) source=%u got=%u\n",
                                pose->textStringIndex, line, glyph,
                                xx, yy, (unsigned int)source,
                                (unsigned int)framebuffer[pixel]);
                        return 0;
                    }
                }
            }
        }
        if (plan.done) {
            break;
        }
        cursor = plan.nextCursor;
    }
    if (expectedLineCount != receipt.lineCount) {
        fprintf(stderr, "line-count mismatch text=%d actual=%d expected=%d\n",
                pose->textStringIndex, receipt.lineCount, expectedLineCount);
        return 0;
    }
    return 1;
}

static int find_no_front_inscription_pose(const M11_GameViewState *state,
                                          VisibleInscriptionPosePc34 *outPose)
{
    int mapIndex;

    if (!state || !outPose || !state->world.dungeon) {
        return 0;
    }
    for (mapIndex = 0; mapIndex < (int)state->world.dungeon->header.mapCount;
         ++mapIndex) {
        const struct DungeonMapDesc_Compat *map = &state->world.dungeon->maps[mapIndex];
        int y;
        for (y = 0; y < (int)map->height; ++y) {
            int x;
            for (x = 0; x < (int)map->width; ++x) {
                int direction;
                if (square_element(state, mapIndex, x, y) != DUNGEON_ELEMENT_CORRIDOR) {
                    continue;
                }
                for (direction = 0; direction < 4; ++direction) {
                    int aheadX = x + direction_dx(direction);
                    int aheadY = y + direction_dy(direction);
                    if (square_element(state, mapIndex, aheadX, aheadY) ==
                        DUNGEON_ELEMENT_CORRIDOR) {
                        outPose->mapIndex = mapIndex;
                        outPose->partyX = x;
                        outPose->partyY = y;
                        outPose->direction = direction;
                        outPose->textStringIndex = -1;
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
    VisibleInscriptionPosePc34 poses[kMaxVisibleInscriptionPoses];
    VisibleInscriptionPosePc34 noTextPose;
    M11_GameViewState state;
    const M11_AssetSlot *font;
    uint32_t fontHash;
    int poseCount;
    int i;
    unsigned char framebuffer[kFramebufferWidth * kFramebufferHeight];
    M11_Dm1InscriptionHostPresentationReceipt clearedReceipt;

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
    if (!font || !font->loaded || !font->pixels ||
        font->width != DM1_V1_INSCRIPTION_FONT_WIDTH_PC34 ||
        font->height != DM1_V1_INSCRIPTION_FONT_HEIGHT_PC34) {
        fprintf(stderr, "real PC34 M648 material unavailable\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    fontHash = hash_pixels(font->pixels, (int)font->width * (int)font->height);
    poseCount = append_visible_poses(&state, poses,
                                     kMaxVisibleInscriptionPoses);
    if (poseCount <= 0 || !find_no_front_inscription_pose(&state, &noTextPose)) {
        fprintf(stderr, "real PC34 visible/no-text inscription poses unavailable\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }

    for (i = 0; i < poseCount; ++i) {
        state.world.party.mapIndex = poses[i].mapIndex;
        state.world.party.mapX = poses[i].partyX;
        state.world.party.mapY = poses[i].partyY;
        state.world.party.direction = poses[i].direction;
        memset(framebuffer, 0, sizeof(framebuffer));
        M11_GameView_Draw(&state, framebuffer, kFramebufferWidth,
                          kFramebufferHeight);
        if (!verify_receipt_and_glyph_pixels(&state, &poses[i], framebuffer,
                                             font)) {
            M11_Dm1InscriptionHostPresentationReceipt receipt;
            memset(&receipt, 0, sizeof(receipt));
            M11_GameView_GetDm1InscriptionHostPresentationReceipt(&receipt);
            fprintf(stderr,
                    "F0107/F0124 stale or wrong M648 material at pose %d map=%d party=(%d,%d) dir=%d text=%d receipt=(valid=%d text=%d glyphs=%d lines=%d)\n",
                    i, poses[i].mapIndex, poses[i].partyX, poses[i].partyY,
                    poses[i].direction, poses[i].textStringIndex,
                    receipt.valid, receipt.textStringIndex,
                    receipt.glyphByteCount, receipt.lineCount);
            M11_GameView_Shutdown(&state);
            return 1;
        }
    }
    state.world.party.mapIndex = noTextPose.mapIndex;
    state.world.party.mapX = noTextPose.partyX;
    state.world.party.mapY = noTextPose.partyY;
    state.world.party.direction = noTextPose.direction;
    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&state, framebuffer, kFramebufferWidth, kFramebufferHeight);
    memset(&clearedReceipt, 0, sizeof(clearedReceipt));
    M11_GameView_GetDm1InscriptionHostPresentationReceipt(&clearedReceipt);
    if (clearedReceipt.valid || clearedReceipt.fontGraphicIndex != 0 ||
        clearedReceipt.transparentColor != 0 || clearedReceipt.glyphByteCount != 0 ||
        clearedReceipt.lineCount != 0 || clearedReceipt.glyphBytes[0] != 0 ||
        fontHash != hash_pixels(font->pixels,
                                (int)font->width * (int)font->height)) {
        fprintf(stderr, "F0107/F0124 retained M648 material after final no-text tuple\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    printf("ok: %d real PC34 visible inscriptions replace M648/C10 material frame-by-frame\n",
           poseCount);
    M11_GameView_Shutdown(&state);
    return 0;
}

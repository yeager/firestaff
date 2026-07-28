/*
 * Real PC34 non-HoC front-wall inscription consumption regression.
 *
 * ReDMCSB DUNVIEW.C F0107:3590-3639 selects the inscription TextString,
 * but draws it only for M587_VIEW_WALL_D1C_FRONT.  It centres the decoded
 * M648 glyphs at 112 - (count << 2) and uses C10 transparency.  DUNGEON.C
 * F0168:2255-2348 owns the TextString byte decode and 0x80/0x81 markers.
 * This test finds an actual map > 0 PC34 wall, chooses its legal D1C party
 * pose, and proves the final M11 pixels and receipt consume those bytes.
 */

#include "dm1_v1_inscription_font_pc34_compat.h"
#include "m11_game_view.h"
#include "memory_dungeon_dat_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
    kFramebufferWidth = 320,
    kFramebufferHeight = 200,
    kViewportY = 33
};

typedef struct NonHocInscriptionPose {
    int mapIndex;
    int wallX;
    int wallY;
    int partyX;
    int partyY;
    int direction;
    int textStringIndex;
} NonHocInscriptionPose;

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
        !state->world.dungeon->tiles[mapIndex].squareData) {
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

static int find_non_hoc_front_inscription(const M11_GameViewState* state,
                                          NonHocInscriptionPose* outPose)
{
    int mapIndex;
    if (!state || !state->world.dungeon || !state->world.things || !outPose) {
        return 0;
    }
    for (mapIndex = 1; mapIndex < (int)state->world.dungeon->header.mapCount;
         ++mapIndex) {
        const struct DungeonMapDesc_Compat* map =
            &state->world.dungeon->maps[mapIndex];
        int y;
        for (y = 0; y < (int)map->height; ++y) {
            int x;
            for (x = 0; x < (int)map->width; ++x) {
                unsigned short thing;
                if (square_element(state, mapIndex, x, y) != DUNGEON_ELEMENT_WALL) {
                    continue;
                }
                /* DUNGEON.C F0511 applies the required preceding-map
                 * thing-list offset; a map-local square index is invalid. */
                thing = F0511_DUNGEON_GetSquareFirstThing_Compat(
                    state->world.dungeon, state->world.things, mapIndex, x, y);
                while (thing != THING_ENDOFLIST && thing != THING_NONE) {
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
                            /* The party faces the wall.  ReDMCSB F0172's
                             * selected wall cell must therefore be behind
                             * the facing vector. */
                            if (((direction + 2) & 3) != THING_GET_CELL(thing)) {
                                continue;
                            }
                            partyX = x - direction_dx(direction);
                            partyY = y - direction_dy(direction);
                            if (square_element(state, mapIndex, partyX, partyY) !=
                                DUNGEON_ELEMENT_CORRIDOR) {
                                continue;
                            }
                            outPose->mapIndex = mapIndex;
                            outPose->wallX = x;
                            outPose->wallY = y;
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

static int verify_receipt_and_pixels(const M11_GameViewState* state,
                                     int textStringIndex,
                                     const unsigned char* withText,
                                     const unsigned char* withoutText)
{
    M11_Dm1InscriptionHostPresentationReceipt receipt;
    const M11_AssetSlot* font;
    DM1_V1_InscriptionHostMaterialReceiptPc34 material;
    DM1_V1_InscriptionSourceRasterCapturePc34 capture;
    unsigned char expected[DM1_V1_INSCRIPTION_HOST_MATERIAL_MAX_GLYPHS_PC34];
    int expectedCount;
    int cursor = 0;
    int line;

    if (!state || !withText || !withoutText || !state->world.things) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    M11_GameView_GetDm1InscriptionHostPresentationReceipt(&receipt);
    if (!receipt.valid || receipt.textStringIndex != textStringIndex ||
        receipt.fontGraphicIndex != DM1_V1_INSCRIPTION_FONT_GRAPHIC_INDEX_PC34 ||
        receipt.transparentColor != DM1_V1_INSCRIPTION_TRANSPARENT_COLOR ||
        receipt.glyphByteCount <= 0 || receipt.lineCount <= 0) {
        fprintf(stderr, "incomplete non-HoC PC34 M648 host receipt\n");
        return 0;
    }
    expectedCount = DM1_V1_InscriptionDecodeRawGlyphsFromWordsPc34(
        state->world.things->textData, state->world.things->textDataWordCount,
        state->world.things->textStrings[textStringIndex].textDataWordOffset,
        expected, (int)sizeof(expected));
    if (expectedCount <= 1 || expected[expectedCount - 1] != 0x81U ||
        receipt.glyphByteCount != expectedCount - 1 ||
        memcmp(receipt.glyphBytes, expected, (size_t)receipt.glyphByteCount) != 0) {
        fprintf(stderr, "M11 receipt did not retain the real F0168 glyph bytes\n");
        return 0;
    }
    font = M11_AssetLoader_Load((M11_AssetLoader*)&state->assetLoader,
                                DM1_V1_INSCRIPTION_FONT_GRAPHIC_INDEX_PC34);
    if (!font || !font->loaded || !font->pixels ||
        font->width != DM1_V1_INSCRIPTION_FONT_WIDTH_PC34 ||
        font->height != DM1_V1_INSCRIPTION_FONT_HEIGHT_PC34) {
        fprintf(stderr, "PC34 GRAPHICS.DAT lacks M648 inscription font\n");
        return 0;
    }
    memset(&material, 0, sizeof(material));
    memset(&capture, 0, sizeof(capture));
    if (!dm1_v1_inscription_host_material_from_selected_wall_pc34(
            state->world.things, textStringIndex, &material) ||
        !DM1_V1_InscriptionCaptureSourceRasterPc34(
            &material, font->pixels, (int)font->width, (int)font->height,
            &capture) || !capture.valid ||
        receipt.sourceCellsFNV1a != capture.sourceCellsFNV1a ||
        receipt.glyphCellCount != capture.glyphCellCount ||
        receipt.opaqueGlyphPixelCount != capture.opaquePixelCount ||
        receipt.transparentGlyphPixelCount != capture.transparentPixelCount) {
        fprintf(stderr, "M648 source-cell capture did not match real PC34 asset\n");
        return 0;
    }
    for (line = 0; line < DM1_V1_INSCRIPTION_MAX_LINES; ++line) {
        DM1_V1_InscriptionFrontWallLineDrawPlanPc34 plan;
        int glyphIndex;
        if (!DM1_V1_InscriptionBuildFrontWallLineDrawPlanPc34(
                expected, (int)sizeof(expected), cursor, line, 160, 111, &plan)) {
            return 0;
        }
        if (plan.glyphCount > 0) {
            if (receipt.lineGlyphCount[line] != plan.glyphCount ||
                receipt.lineDestinationX[line] != plan.textX ||
                receipt.lineDestinationY[line] != kViewportY + plan.textY) {
                fprintf(stderr, "D1C line %d placement differs from ReDMCSB M648 plan\n",
                        line);
                return 0;
            }
            for (glyphIndex = 0; glyphIndex < plan.glyphCount; ++glyphIndex) {
                int glyph = DM1_V1_InscriptionGlyphIndexFromSourceByte(
                    expected[plan.glyphStart + glyphIndex]);
                int yy;
                if (glyph < 0) return 0;
                for (yy = 0; yy < DM1_V1_INSCRIPTION_GLYPH_HEIGHT; ++yy) {
                    int xx;
                    for (xx = 0; xx < DM1_V1_INSCRIPTION_GLYPH_WIDTH; ++xx) {
                        int pixel = (kViewportY + plan.textY + yy) *
                            kFramebufferWidth + plan.textX +
                            glyphIndex * DM1_V1_INSCRIPTION_GLYPH_WIDTH + xx;
                        unsigned char source = font->pixels[yy * font->width +
                            glyph * DM1_V1_INSCRIPTION_GLYPH_WIDTH + xx];
                        if (source == DM1_V1_INSCRIPTION_TRANSPARENT_COLOR) {
                            const int screenX = plan.textX +
                                glyphIndex * DM1_V1_INSCRIPTION_GLYPH_WIDTH + xx;
                            const int screenY = kViewportY + plan.textY + yy;
                            /* ReDMCSB F0107 restores G202 (110..113,
                             * 37..62) from the D1C wall before M648. The
                             * dedicated HoC regression verifies that crop. */
                            if (screenX >= 110 && screenX <= 113 &&
                                screenY >= 70 && screenY <= 95) {
                                continue;
                            }
                            if (withText[pixel] != withoutText[pixel]) {
                                fprintf(stderr, "C10 changed wall pixel on line %d\n", line);
                                return 0;
                            }
                        } else if (withText[pixel] != source) {
                            fprintf(stderr, "M648 glyph pixel mismatch on line %d\n", line);
                            return 0;
                        }
                    }
                }
            }
        }
        if (plan.done) break;
        cursor = plan.nextCursor;
    }
    return 1;
}

int main(void)
{
    const char* dataDir = getenv("FIRESTAFF_DM1_DATA_DIR");
    char defaultDataDir[1024];
    const char* home;
    M11_GameViewState state;
    NonHocInscriptionPose pose;
    unsigned char withoutText[kFramebufferWidth * kFramebufferHeight];
    unsigned char withText[kFramebufferWidth * kFramebufferHeight];
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
        if (getenv("FIRESTAFF_DM1_DATA_DIR")) {
            fprintf(stderr, "configured DM1 data directory failed to start\n");
            return 1;
        }
        return 0;
    }
    state.presentationMode = M12_PRESENTATION_V1_ORIGINAL;
    state.world.party.championCount = 0;
    if (!find_non_hoc_front_inscription(&state, &pose)) {
        fprintf(stderr, "no real non-HoC D1C inscription pose in PC34 data\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    state.world.party.mapIndex = pose.mapIndex;
    state.world.party.mapX = pose.partyX;
    state.world.party.mapY = pose.partyY;
    state.world.party.direction = pose.direction;
    state.world.things->textStrings[pose.textStringIndex].visible = 0;
    memset(withoutText, 0, sizeof(withoutText));
    M11_GameView_Draw(&state, withoutText, kFramebufferWidth, kFramebufferHeight);
    state.world.things->textStrings[pose.textStringIndex].visible = 1;
    memset(withText, 0, sizeof(withText));
    M11_GameView_Draw(&state, withText, kFramebufferWidth, kFramebufferHeight);
    if (!verify_receipt_and_pixels(&state, pose.textStringIndex, withText, withoutText)) {
        M11_GameView_Shutdown(&state);
        return 1;
    }
    for (modeIndex = 0; modeIndex < sizeof(v2Modes) / sizeof(v2Modes[0]);
         ++modeIndex) {
        state.presentationMode = v2Modes[modeIndex];
        state.world.things->textStrings[pose.textStringIndex].visible = 0;
        memset(withoutText, 0, sizeof(withoutText));
        M11_GameView_Draw(&state, withoutText,
                          kFramebufferWidth, kFramebufferHeight);
        state.world.things->textStrings[pose.textStringIndex].visible = 1;
        memset(withText, 0, sizeof(withText));
        M11_GameView_Draw(&state, withText, kFramebufferWidth, kFramebufferHeight);
        if (!verify_receipt_and_pixels(&state, pose.textStringIndex,
                                       withText, withoutText)) {
            fprintf(stderr, "V2 mode %d did not preserve original M648 pixels\n",
                    (int)v2Modes[modeIndex]);
            M11_GameView_Shutdown(&state);
            return 1;
        }
    }
    printf("ok: real PC34 non-HoC D1C inscription map=%d wall=(%d,%d) dir=%d\n",
           pose.mapIndex, pose.wallX, pose.wallY, pose.direction);
    M11_GameView_Shutdown(&state);
    return 0;
}

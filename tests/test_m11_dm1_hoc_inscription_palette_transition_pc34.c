/*
 * Real PC34 HoC M648 palette-material transition regression.
 *
 * ReDMCSB DUNVIEW.C F0128 rebuilds the viewport for every party tuple, then
 * F0107:3619-3638 blits M648's indexed glyphs with C10 as the only
 * transparent source colour.  A prior corridor frame must not retain an
 * inscription palette/material receipt; conversely C10 pixels must preserve
 * the freshly rebuilt D1C wall beneath M648 for every HoC corridor transition.
 */

#include "dm1_v1_inscription_font_pc34_compat.h"
#include "dm1_v1_inscription_host_material_pc34_compat.h"
#include "dm1_v1_viewport_3d_pc34_compat.h"
#include "m11_game_view.h"
#include "memory_dungeon_dat_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
    kFramebufferWidth = 320,
    kFramebufferHeight = 200,
    kViewportX = 0,
    kViewportY = 33,
    kWallSetFirstGraphic = 86,
    kWallSetGraphicCount = 40,
    kWallD1cGraphicOffset = 11,
    kInscriptionPatchSourceX = 94,
    kInscriptionPatchSourceY = 28,
    kInscriptionPatchDestinationX = 110,
    kInscriptionPatchDestinationY = 37,
    kInscriptionPatchWidth = 4,
    kInscriptionPatchHeight = 26,
    /* PC34 map 0 can contain up to 32x32 corridor cells, with all four
     * source directions needing a distinct F0128 tuple rebuild. */
    kMaxPoses = 4096
};

typedef struct HocPosePc34 {
    int x;
    int y;
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
        state->world.dungeon->header.mapCount == 0) {
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

static int collect_hoc_inscriptions(const M11_GameViewState *state,
                                    HocPosePc34 *poses, int capacity)
{
    const struct DungeonMapDesc_Compat *map;
    int count = 0;
    int y;

    if (!state || !state->world.dungeon || !state->world.things || !poses ||
        state->world.dungeon->header.mapCount == 0) {
        return -1;
    }
    map = &state->world.dungeon->maps[0];
    for (y = 0; y < (int)map->height; ++y) {
        int x;
        for (x = 0; x < (int)map->width; ++x) {
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
                    const int direction = ((int)THING_GET_CELL(thing) + 2) & 3;
                    const int partyX = x - direction_dx(direction);
                    const int partyY = y - direction_dy(direction);
                    if (textIndex >= 0 &&
                        textIndex < state->world.things->textStringCount &&
                        state->world.things->textStrings[textIndex].visible &&
                        square_element(state, partyX, partyY) ==
                            DUNGEON_ELEMENT_CORRIDOR) {
                        if (count >= capacity) {
                            return -1;
                        }
                        poses[count].x = partyX;
                        poses[count].y = partyY;
                        poses[count].direction = direction;
                        poses[count].textStringIndex = textIndex;
                        ++count;
                    }
                }
                thing = F0512_DUNGEON_GetThingNext_Compat(state->world.things, thing);
            }
        }
    }
    return count;
}

static int collect_hoc_corridor_poses(const M11_GameViewState *state,
                                      HocPosePc34 *poses, int capacity)
{
    const struct DungeonMapDesc_Compat *map;
    int count = 0;
    int y;

    if (!state || !state->world.dungeon || !poses ||
        state->world.dungeon->header.mapCount == 0) {
        return -1;
    }
    map = &state->world.dungeon->maps[0];
    for (y = 0; y < (int)map->height; ++y) {
        int x;
        for (x = 0; x < (int)map->width; ++x) {
            int direction;
            if (square_element(state, x, y) != DUNGEON_ELEMENT_CORRIDOR) {
                continue;
            }
            for (direction = 0; direction < 4; ++direction) {
                if (count >= capacity) {
                    return -1;
                }
                poses[count].x = x;
                poses[count].y = y;
                poses[count].direction = direction;
                poses[count].textStringIndex = -1;
                ++count;
            }
        }
    }
    return count;
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

static int is_clear_receipt(const M11_Dm1InscriptionHostPresentationReceipt *receipt)
{
    return receipt && !receipt->valid && receipt->fontGraphicIndex == 0 &&
           receipt->transparentColor == 0 && receipt->glyphByteCount == 0 &&
           receipt->lineCount == 0 && receipt->glyphBytes[0] == 0;
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

static int glyph_is_opaque_at(
    const DM1_V1_InscriptionHostMaterialReceiptPc34 *material,
    const M11_AssetSlot *font,
    int screenX,
    int screenY)
{
    int line;

    if (!material || !font || !font->pixels) {
        return 0;
    }
    for (line = 0; line < DM1_V1_INSCRIPTION_MAX_LINES; ++line) {
        int glyphOffset;
        const DM1_V1_InscriptionFrontWallLineDrawPlanPc34 *plan =
            &material->lines[line];
        const int glyphY = kViewportY + plan->textY;
        const int glyphCount = plan->glyphCount;
        const int glyphX = kViewportX + plan->textX;
        if (screenY < glyphY || screenY >= glyphY + DM1_V1_INSCRIPTION_GLYPH_HEIGHT) {
            continue;
        }
        for (glyphOffset = 0; glyphOffset < glyphCount; ++glyphOffset) {
            const int x = glyphX + glyphOffset * DM1_V1_INSCRIPTION_GLYPH_WIDTH;
            const int glyph = material->glyphBytes[
                plan->glyphStart + glyphOffset];
            if (screenX >= x && screenX < x + DM1_V1_INSCRIPTION_GLYPH_WIDTH) {
                const int sx = glyph * DM1_V1_INSCRIPTION_GLYPH_WIDTH +
                    screenX - x;
                const int sy = screenY - glyphY;
                return font->pixels[sy * (int)font->width + sx] !=
                    DM1_V1_INSCRIPTION_TRANSPARENT_COLOR;
            }
        }
        if (plan->done) {
            break;
        }
    }
    return 0;
}

static int receipt_has_authenticated_source_span(
    const M11_GameViewState *state,
    const M11_Dm1InscriptionHostPresentationReceipt *receipt,
    const M11_AssetSlot *font)
{
    DM1_V1_InscriptionHostMaterialReceiptPc34 material;

    if (!state || !receipt || !font || !font->pixels || !receipt->valid ||
        !dm1_v1_inscription_host_material_from_selected_wall_pc34(
            state->world.things, receipt->textStringIndex, &material)) {
        return 0;
    }
    return receipt->textDataWordOffset == material.textDataWordOffset &&
           receipt->textDataWordCount == material.textDataWordCount &&
           receipt->textDataFNV1a == material.textDataFNV1a &&
           receipt->glyphBytesFNV1a == material.glyphBytesFNV1a &&
           receipt->fontPixelsFNV1a ==
               hash_pixels(font->pixels, (int)font->width * (int)font->height) &&
           receipt->glyphSourceWidth == DM1_V1_INSCRIPTION_GLYPH_WIDTH &&
           receipt->glyphSourceHeight == DM1_V1_INSCRIPTION_GLYPH_HEIGHT &&
           receipt->glyphScaleNumerator == 1 &&
           receipt->glyphScaleDenominator == 1 &&
           receipt->paletteMapValid == 0;
}

static int verify_c10_preserves_d1c_wall(
    const M11_GameViewState *state,
    const M11_Dm1InscriptionHostPresentationReceipt *receipt,
    const M11_AssetSlot *font,
    const unsigned char *baseline,
    const unsigned char *withText)
{
    unsigned char glyphs[DM1_V1_INSCRIPTION_HOST_MATERIAL_MAX_GLYPHS_PC34];
    int glyphCount;
    int cursor = 0;
    int line;
    int transparentPixels = 0;

    if (!state || !receipt || !font || !font->pixels || !baseline || !withText ||
        !state->world.things || !receipt->valid ||
        receipt->fontGraphicIndex != DM1_V1_INSCRIPTION_FONT_GRAPHIC_INDEX_PC34 ||
        receipt->transparentColor != DM1_V1_INSCRIPTION_TRANSPARENT_COLOR ||
        receipt->textStringIndex < 0 ||
        receipt->textStringIndex >= state->world.things->textStringCount) {
        return 0;
    }
    glyphCount = DM1_V1_InscriptionDecodeRawGlyphsFromWordsPc34(
        state->world.things->textData, state->world.things->textDataWordCount,
        state->world.things->textStrings[receipt->textStringIndex].textDataWordOffset,
        glyphs, (int)sizeof(glyphs));
    if (glyphCount <= 1 || glyphs[glyphCount - 1] != 0x81U ||
        receipt->glyphByteCount != glyphCount - 1) {
        return 0;
    }
    for (line = 0; line < DM1_V1_INSCRIPTION_MAX_LINES; ++line) {
        DM1_V1_InscriptionFrontWallLineDrawPlanPc34 plan;
        int offset;
        if (!DM1_V1_InscriptionBuildFrontWallLineDrawPlanPc34(
                glyphs, (int)sizeof(glyphs), cursor, line, 160, 111, &plan)) {
            return 0;
        }
        for (offset = 0; offset < plan.glyphCount; ++offset) {
            const int glyph = DM1_V1_InscriptionGlyphIndexFromSourceByte(
                glyphs[plan.glyphStart + offset]);
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
                            kFramebufferWidth + kViewportX + plan.textX +
                        offset * DM1_V1_INSCRIPTION_GLYPH_WIDTH + xx;
                    if (source == DM1_V1_INSCRIPTION_TRANSPARENT_COLOR) {
                        const int screenX = kViewportX + plan.textX +
                            offset * DM1_V1_INSCRIPTION_GLYPH_WIDTH + xx;
                        const int screenY = kViewportY + plan.textY + yy;
                        ++transparentPixels;
                        /* F0107 restores G202's real D1C wall crop before
                         * M648. The patch owns these transparent cells;
                         * verify_f0107_wall_patch checks their source pixels. */
                        if (screenX >= kViewportX + kInscriptionPatchDestinationX &&
                            screenX < kViewportX + kInscriptionPatchDestinationX +
                                kInscriptionPatchWidth &&
                            screenY >= kViewportY + kInscriptionPatchDestinationY &&
                            screenY < kViewportY + kInscriptionPatchDestinationY +
                                kInscriptionPatchHeight) {
                            continue;
                        }
                        if (withText[pixel] != baseline[pixel]) {
                            return 0;
                        }
                    }
                }
            }
        }
        if (plan.done) {
            break;
        }
        cursor = plan.nextCursor;
    }
    return transparentPixels > 0;
}

static int verify_f0107_wall_patch(
    const M11_GameViewState *state,
    const M11_Dm1InscriptionHostPresentationReceipt *receipt,
    const M11_AssetSlot *font,
    const unsigned char *withText)
{
    DM1_V1_InscriptionHostMaterialReceiptPc34 material;
    const M11_AssetSlot *wall;
    const struct DungeonMapDesc_Compat *map;
    int graphicIndex;
    int flipped;
    int compared = 0;
    int y;

    if (!state || !receipt || !font || !withText || !receipt->valid ||
        !state->world.dungeon || state->world.party.mapIndex != 0 ||
        !dm1_v1_inscription_host_material_from_selected_wall_pc34(
            state->world.things, receipt->textStringIndex, &material)) {
        return 0;
    }
    map = &state->world.dungeon->maps[state->world.party.mapIndex];
    graphicIndex = kWallSetFirstGraphic +
        (int)map->wallSet * kWallSetGraphicCount + kWallD1cGraphicOffset;
    wall = M11_AssetLoader_Load((M11_AssetLoader *)&state->assetLoader,
                                (unsigned int)graphicIndex);
    if (!wall || !wall->loaded || !wall->pixels ||
        wall->width != 160 || wall->height != 111) {
        return 0;
    }
    flipped = dm1_viewport_3d_use_flipped_walls_pc34(
        state->world.party.mapX, state->world.party.mapY,
        state->world.party.direction);
    for (y = 0; y < kInscriptionPatchHeight; ++y) {
        int x;
        for (x = 0; x < kInscriptionPatchWidth; ++x) {
            const int screenX = kViewportX + kInscriptionPatchDestinationX + x;
            const int screenY = kViewportY + kInscriptionPatchDestinationY + y;
            const int sourceX = flipped
                ? 160 - 1 - (kInscriptionPatchSourceX + x)
                : kInscriptionPatchSourceX + x;
            const unsigned char expected = wall->pixels[
                (kInscriptionPatchSourceY + y) * (int)wall->width + sourceX];
            if (glyph_is_opaque_at(&material, font, screenX, screenY)) {
                continue;
            }
            ++compared;
            if (withText[screenY * kFramebufferWidth + screenX] != expected) {
                return 0;
            }
        }
    }
    return compared > 0;
}

int main(void)
{
    const char *dataDir = getenv("FIRESTAFF_DM1_DATA_DIR");
    const char *home;
    char defaultDataDir[1024];
    HocPosePc34 inscriptions[kMaxPoses];
    HocPosePc34 corridors[kMaxPoses];
    HocPosePc34 clearCorridors[kMaxPoses];
    M11_GameViewState state;
    const M11_AssetSlot *font;
    unsigned char before[kFramebufferWidth * kFramebufferHeight];
    unsigned char baseline[kFramebufferWidth * kFramebufferHeight];
    unsigned char withText[kFramebufferWidth * kFramebufferHeight];
    uint32_t fontHash;
    int inscriptionCount;
    int corridorCount;
    int clearCount = 0;
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
    if (!font || !font->loaded || !font->pixels ||
        font->width != DM1_V1_INSCRIPTION_FONT_WIDTH_PC34 ||
        font->height != DM1_V1_INSCRIPTION_FONT_HEIGHT_PC34) {
        fprintf(stderr, "real PC34 M648 material unavailable\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    fontHash = hash_pixels(font->pixels, (int)font->width * (int)font->height);
    inscriptionCount = collect_hoc_inscriptions(&state, inscriptions, kMaxPoses);
    corridorCount = collect_hoc_corridor_poses(&state, corridors, kMaxPoses);
    if (inscriptionCount <= 0 || corridorCount <= 0) {
        fprintf(stderr, "real PC34 HoC inscription/corridor poses unavailable\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }

    for (i = 0; i < corridorCount; ++i) {
        M11_Dm1InscriptionHostPresentationReceipt receipt;
        draw_pose(&state, &corridors[i], before);
        M11_GameView_GetDm1InscriptionHostPresentationReceipt(&receipt);
        if (is_clear_receipt(&receipt)) {
            clearCorridors[clearCount++] = corridors[i];
        }
    }
    if (clearCount <= 0) {
        fprintf(stderr, "no clear HoC corridor frames found\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }

    for (i = 0; i < inscriptionCount; ++i) {
        const int textIndex = inscriptions[i].textStringIndex;
        const unsigned char originalVisible = state.world.things->textStrings[textIndex].visible;
        int transition;
        for (transition = 0; transition < clearCount; ++transition) {
            M11_Dm1InscriptionHostPresentationReceipt receipt;
            M11_Dm1InscriptionHostPresentationReceipt cleared;

            draw_pose(&state, &clearCorridors[transition], before);
            M11_GameView_GetDm1InscriptionHostPresentationReceipt(&cleared);
            if (!is_clear_receipt(&cleared)) {
                fprintf(stderr, "HoC corridor retained M648 before transition %d/%d\n",
                        i, transition);
                M11_GameView_Shutdown(&state);
                return 1;
            }
            state.world.things->textStrings[textIndex].visible = 0;
            draw_pose(&state, &inscriptions[i], baseline);
            M11_GameView_GetDm1InscriptionHostPresentationReceipt(&cleared);
            state.world.things->textStrings[textIndex].visible = originalVisible;
            if (!is_clear_receipt(&cleared)) {
                fprintf(stderr, "hidden HoC source retained M648 material\n");
                M11_GameView_Shutdown(&state);
                return 1;
            }
            draw_pose(&state, &inscriptions[i], withText);
            M11_GameView_GetDm1InscriptionHostPresentationReceipt(&receipt);
            if (receipt.textStringIndex != textIndex ||
                !receipt_has_authenticated_source_span(&state, &receipt, font) ||
                !verify_c10_preserves_d1c_wall(&state, &receipt, font,
                                                baseline, withText) ||
                !verify_f0107_wall_patch(&state, &receipt, font, withText)) {
                fprintf(stderr, "M648/C10 palette material mismatch at HoC transition %d/%d\n",
                        i, transition);
                M11_GameView_Shutdown(&state);
                return 1;
            }
            draw_pose(&state, &clearCorridors[transition], before);
            M11_GameView_GetDm1InscriptionHostPresentationReceipt(&cleared);
            if (!is_clear_receipt(&cleared)) {
                fprintf(stderr, "HoC corridor retained M648 after transition %d/%d\n",
                        i, transition);
                M11_GameView_Shutdown(&state);
                return 1;
            }
        }
    }
    if (fontHash != hash_pixels(font->pixels, (int)font->width * (int)font->height)) {
        fprintf(stderr, "M648 source pixels mutated during HoC transitions\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    printf("ok: %d real PC34 HoC inscriptions preserve C10 wall pixels across %d corridor transitions\n",
           inscriptionCount, clearCount);
    M11_GameView_Shutdown(&state);
    return 0;
}

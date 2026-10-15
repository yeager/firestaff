/*
 * Real PC34 side/depth wall-inscription presentation regression.
 *
 * ReDMCSB DUNVIEW.C F0107:3864-3901 does not draw readable text away from
 * D1C.  It decodes the real TextString, counts its lines, shrinks the source
 * ornament box, and blits that original material with C10 transparency.
 */

#include "dm1_v1_inscription_font_pc34_compat.h"
#include "dm1_v1_inscription_host_material_pc34_compat.h"
#include "dm1_v1_wall_ornament_pc34_compat.h"
#include "m11_game_view.h"
#include "memory_dungeon_dat_pc34_compat.h"
#include "asset_find_by_hash.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { kFramebufferWidth = 320, kFramebufferHeight = 200, kViewportY = 33 };

static void direction_vectors(int direction, int* fx, int* fy, int* rx, int* ry)
{
    static const int kFx[4] = {0, 1, 0, -1};
    static const int kFy[4] = {-1, 0, 1, 0};
    static const int kRx[4] = {1, 0, -1, 0};
    static const int kRy[4] = {0, 1, 0, -1};
    *fx = kFx[direction & 3];
    *fy = kFy[direction & 3];
    *rx = kRx[direction & 3];
    *ry = kRy[direction & 3];
}

static int square_element(const M11_GameViewState* state,
                          int mapIndex, int x, int y)
{
    const struct DungeonMapDesc_Compat* map;
    int index;
    if (!state || !state->world.dungeon || mapIndex < 0 ||
        mapIndex >= (int)state->world.dungeon->header.mapCount ||
        !state->world.dungeon->tiles ||
        !state->world.dungeon->tiles[mapIndex].squareData) return -1;
    map = &state->world.dungeon->maps[mapIndex];
    if (x < 0 || y < 0 || x >= (int)map->width || y >= (int)map->height) return -1;
    index = x * (int)map->height + y;
    return (state->world.dungeon->tiles[mapIndex].squareData[index] &
            DUNGEON_SQUARE_MASK_TYPE) >> 5;
}

static int decoded_line_count(const M11_GameViewState* state, int textStringIndex)
{
    unsigned char glyphs[DM1_V1_INSCRIPTION_HOST_MATERIAL_MAX_GLYPHS_PC34];
    int count;
    int lines = 1;
    int i;
    if (!state || !state->world.things || textStringIndex < 0 ||
        textStringIndex >= state->world.things->textStringCount) return 0;
    count = DM1_V1_InscriptionDecodeRawGlyphsFromWordsPc34(
        state->world.things->textData, state->world.things->textDataWordCount,
        state->world.things->textStrings[textStringIndex].textDataWordOffset,
        glyphs, (int)sizeof(glyphs));
    if (count <= 1 || glyphs[count - 1] != 0x81U) return 0;
    for (i = 0; i < count - 1; ++i) {
        if (glyphs[i] == 0x80U) ++lines;
    }
    return lines;
}

static int verify_original_ornament_asset(
    const M11_GameViewState* state,
    const M11_Dm1UnreadableInscriptionHostPresentationReceipt* receipt,
    const DM1_WallOrnamentHostMaterialReceiptPc34* material)
{
    const M11_AssetSlot* slot;
    const DM1_WallOrnamentRenderPlanPc34* plan = &material->plan;
    int pixelCount;
    int i;
    int opaque = 0;
    int transparent = 0;
    slot = M11_AssetLoader_Load((M11_AssetLoader*)&state->assetLoader,
                                (unsigned int)plan->graphicIndex);
    if (!slot || !slot->loaded || !slot->pixels ||
        receipt->graphicIndex != (int)slot->graphicIndex ||
        receipt->transparentColor != plan->transparentColor ||
        receipt->paletteMapValid != plan->paletteMapValid ||
        (plan->paletteMapValid &&
         memcmp(receipt->paletteMap, plan->paletteMap,
                sizeof(receipt->paletteMap)) != 0)) {
        return 0;
    }
    pixelCount = (int)slot->width * (int)slot->height;
    for (i = 0; i < pixelCount; ++i) {
        if (slot->pixels[i] == (unsigned char)plan->transparentColor) {
            ++transparent;
        } else {
            ++opaque;
        }
    }
    return opaque > 0 && transparent > 0;
}

/* Authenticated I34E C696 at file offset0x47a6f: little-endian FC0D,
 * bank intervals followed by eight-byte {type,parent,x,y} records. This
 * oracle reads selected original bytes, never production destination APIs. */
static unsigned int original_u16(const unsigned char* p) {
    return (unsigned int)p[0] | ((unsigned int)p[1] << 8);
}

static const unsigned char* original_record(const unsigned char* bytes,
    size_t size, int zone) {
    const size_t base = 0x47a6fu;
    size_t cursor, count;
    if (size < base + 4u || original_u16(bytes + base) != 0xfc0du) return NULL;
    count = original_u16(bytes + base + 2u);
    if (!count || count > 256u || count > (size - base - 4u) / 4u) return NULL;
    cursor = base + 4u + count * 4u;
    for (size_t i = 0; i < count; ++i) {
        unsigned int first = original_u16(bytes + base + 4u + i * 4u);
        unsigned int last = original_u16(bytes + base + 6u + i * 4u);
        size_t length;
        if (last < first) return NULL;
        length = ((size_t)last - first + 1u) * 8u;
        if (cursor > size || length > size - cursor) return NULL;
        if (zone >= (int)first && zone <= (int)last)
            return bytes + cursor + ((size_t)zone - first) * 8u;
        cursor += length;
    }
    return NULL;
}

static int original_inscription_oracle(M11_GameViewState* state,
    const unsigned char* original, size_t original_size,
    const M11_Dm1UnreadableInscriptionHostPresentationReceipt* receipt,
    const unsigned char* framebuffer) {
    static const unsigned char d3[16] = {0,0,12,3,4,3,0,6,3,9,10,11,0,1,0,2};
    static const unsigned char d2[16] = {0,12,1,3,4,3,6,7,5,9,10,11,0,2,14,13};
    /* Original G0194[0]=7. Original G0190 chooses G0204's five rows. */
    static const unsigned char heights[5][3] = {
        {5,8,13}, {7,13,20}, {5,12,19}, {10,17,27}, {11,22,33}
    };
    int view = receipt->viewWallIndex;
    int row = view >= 13 ? view - 13 : view + 2;
    int side = view == 0 || view == 1 || view == 5 || view == 6 ||
               view == 10 || view == 11 || view >= 13;
    int flip = view == 1 || view == 6 || view == 11 || view == 14;
    int scale_x = view >= 13 ? 30 : view < 5 ? 14 : view < 10 ? 21 : 32;
    int scale_y = view >= 13 ? 14 : scale_x;
    int height_row = view >= 13 || view < 2 ? 0 : view < 5 ? 1 :
                     view < 7 ? 2 : view < 10 ? 3 : 4;
    const unsigned char* palette = view >= 13 || view < 5 ? d3 : view < 10 ? d2 : NULL;
    const unsigned char* record = original_record(original, original_size, 1004 + 7*15 + row);
    const unsigned char* root = original_record(original, original_size, 3);
    const unsigned char* parent = original_record(original, original_size, 4);
    const M11_AssetSlot* slot = M11_AssetLoader_Load(&state->assetLoader, 259u + !side);
    int width, source_height, height, left, top, type, clipped_x, clipped_y;
    int clipped_width, clipped_height, source_x, source_y, opaque = 0;
    unsigned int step_x, step_y;
    if (view < 0 || view > 14 || view == 12 || receipt->lineCount < 1 ||
        receipt->lineCount > 3 || !record || !root || !parent || !slot || !slot->pixels ||
        original_u16(root) != 9 || original_u16(root+2) ||
        original_u16(root+4) != 224 || original_u16(root+6) != 136 ||
        original_u16(parent) != 1 || original_u16(parent+2) != 3 ||
        original_u16(parent+4) || original_u16(parent+6) ||
        original_u16(record+2) != 4) return 0;
    width = (slot->width * scale_x + scale_x/2) >> 5;
    source_height = (slot->height * scale_y + scale_y/2) >> 5;
    height = heights[height_row][receipt->lineCount - 1];
    if (width <= 0 || source_height <= 0 || height > source_height ||
        receipt->boxHeight != height) {
        static int reports;
        if (reports++ < 3) fprintf(stderr, "C696 inscription height rejection view %d lines %d seed got %d expected %d native %ux%u derived %dx%d\n", view, receipt->lineCount, receipt->boxHeight, height, slot->width, slot->height, width, source_height);
        return 0;
    }
    type = (int)original_u16(record);
    left = (short)original_u16(record+4) - ((width+1)>>1);
    top = (short)original_u16(record+6);
    if (type == 0) top -= (height+1)>>1;
    else if (type == 7) top -= height-1;
    else if (type != 5) return 0;
    /* F0635 uses the 0x4000 G2154/G2155 dimension seed. It does not
     * rescale the source bitmap when only the zone height is shortened. */
    clipped_x = left < 0 ? 0 : left;
    clipped_y = top < 0 ? 0 : top;
    clipped_width = (left+width > 224 ? 224 : left+width) - clipped_x;
    clipped_height = (top+height > 136 ? 136 : top+height) - clipped_y;
    source_x = clipped_x-left;
    source_y = clipped_y-top;
    /* F0791 adjusts the source span before F0132's horizontal reversal. */
    if (flip) source_x = width > source_x+clipped_width ?
                         width-source_x-clipped_width : 0;
    if (clipped_width <= 0 || clipped_height <= 0 ||
        receipt->graphicIndex != 259+!side || receipt->destinationX != clipped_x ||
        receipt->destinationY != kViewportY+clipped_y ||
        receipt->width != clipped_width || receipt->height != clipped_height) {
        static int geometry_reports;
        if (geometry_reports++ < 3)
            fprintf(stderr, "original C696 inscription view %d rect got (%d,%d,%d,%d) expected (%d,%d,%d,%d)\n",
                view, receipt->destinationX, receipt->destinationY, receipt->width,
                receipt->height, clipped_x, kViewportY+clipped_y,
                clipped_width, clipped_height);
        return 0;
    }
    step_x = ((unsigned int)slot->width << 6) / (unsigned int)width;
    step_y = ((unsigned int)slot->height << 6) / (unsigned int)source_height;
    for (int y = 0; y < clipped_height; ++y) {
        for (int x = 0; x < clipped_width; ++x) {
            unsigned int sx = (step_x/2 + (unsigned int)(source_x +
                (flip ? clipped_width-1-x : x))*step_x) >> 6;
            unsigned int sy = (step_y/2 + (unsigned int)(source_y+y)*step_y) >> 6;
            unsigned char color;
            if (sx >= slot->width || sy >= slot->height) return 0;
            color = slot->pixels[sy*slot->width+sx];
            if (palette) color = palette[color & 15u];
            if (color == 10u) continue;
            if ((framebuffer[(kViewportY+clipped_y+y)*kFramebufferWidth+clipped_x+x] & 15u) != color) {
                static int reports;
                if (reports++ < 3) fprintf(stderr, "C696 inscription pixel rejection view %d at %d,%d got %u expected %u\n", view, clipped_x+x, kViewportY+clipped_y+y, framebuffer[(kViewportY+clipped_y+y)*kFramebufferWidth+clipped_x+x] & 15u, color);
                return 0; /* A nearer source wall may occlude this candidate. */
            }
            ++opaque;
        }
    }
    return opaque > 0;
}

static int find_and_verify_real_side_or_depth_pose(M11_GameViewState* state,
    const unsigned char* original, size_t original_size)
{
    unsigned char framebuffer[kFramebufferWidth * kFramebufferHeight];
    unsigned char turned[kFramebufferWidth * kFramebufferHeight];
    int mapIndex;
    unsigned int eligible_texts = 0u, pose_attempts = 0u, valid_receipts = 0u;
    for (mapIndex = 1; mapIndex < (int)state->world.dungeon->header.mapCount;
         ++mapIndex) {
        const struct DungeonMapDesc_Compat* map = &state->world.dungeon->maps[mapIndex];
        int y;
        for (y = 0; y < (int)map->height; ++y) {
            int x;
            for (x = 0; x < (int)map->width; ++x) {
                unsigned short thing;
                if (square_element(state, mapIndex, x, y) != DUNGEON_ELEMENT_WALL) continue;
                thing = F0511_DUNGEON_GetSquareFirstThing_Compat(
                    state->world.dungeon, state->world.things, mapIndex, x, y);
                while (thing != THING_ENDOFLIST && thing != THING_NONE) {
                    int textStringIndex = THING_GET_TYPE(thing) == THING_TYPE_TEXTSTRING ?
                        (int)THING_GET_INDEX(thing) : -1;
                    if (textStringIndex >= 0 &&
                        textStringIndex < state->world.things->textStringCount &&
                        state->world.things->textStrings[textStringIndex].visible &&
                        decoded_line_count(state, textStringIndex) > 0 &&
                        decoded_line_count(state, textStringIndex) < 4) {
                        int specIndex;
                        ++eligible_texts;
                        for (specIndex = 0;
                             specIndex < dm1_v1_wall_ornament_view_spec_count_pc34();
                             ++specIndex) {
                            DM1_WallOrnamentViewSpecPc34 spec;
                            int direction;
                            if (!dm1_v1_wall_ornament_view_spec_pc34(specIndex, &spec) ||
                                spec.viewWallIndex == 12) continue;
                            for (direction = 0; direction < 4; ++direction) {
                                int fx, fy, rx, ry, partyX, partyY;
                                M11_Dm1UnreadableInscriptionHostPresentationReceipt receipt;
                                M11_Dm1UnreadableInscriptionHostPresentationReceipt turnedReceipt;
                                DM1_WallOrnamentHostMaterialReceiptPc34 material;
                                DM1_V1_InscriptionHostMaterialReceiptPc34 sourceMaterial;
                                int turn;
                                int foundDifferentTuple = 0;
                                direction_vectors(direction, &fx, &fy, &rx, &ry);
                                partyX = x - spec.relForward * fx - spec.relSide * rx;
                                partyY = y - spec.relForward * fy - spec.relSide * ry;
                                if (square_element(state, mapIndex, partyX, partyY) !=
                                    DUNGEON_ELEMENT_CORRIDOR) continue;
                                state->world.party.mapIndex = mapIndex;
                                state->world.party.mapX = partyX;
                                state->world.party.mapY = partyY;
                                state->world.party.direction = direction;
                                ++pose_attempts;
                                memset(framebuffer, 0, sizeof(framebuffer));
                                M11_GameView_Draw(state, framebuffer,
                                                  kFramebufferWidth, kFramebufferHeight);
                                memset(&receipt, 0, sizeof(receipt));
                                M11_GameView_GetDm1UnreadableInscriptionHostPresentationReceipt(
                                    &receipt);
                                if (receipt.valid) ++valid_receipts;
                                if (!receipt.valid || receipt.textStringIndex != textStringIndex) {
                                    continue;
                                }
                                /* Multiple F0107 projection specs can share
                                 * one relative square.  F0128's later
                                 * source-order projection is authoritative,
                                 * so validate that published receipt rather
                                 * than assuming this candidate spec was last. */
                                if (receipt.lineCount != decoded_line_count(state, textStringIndex) ||
                                    receipt.boxHeight <= 0 ||
                                    !dm1_v1_inscription_host_material_from_selected_wall_pc34(
                                        state->world.things, textStringIndex,
                                        &sourceMaterial) ||
                                    receipt.textDataWordOffset !=
                                        sourceMaterial.textDataWordOffset ||
                                    receipt.textDataWordCount !=
                                        sourceMaterial.textDataWordCount ||
                                    receipt.textDataFNV1a != sourceMaterial.textDataFNV1a ||
                                    receipt.glyphBytesFNV1a !=
                                        sourceMaterial.glyphBytesFNV1a ||
                                    !dm1_v1_wall_ornament_native_material_pc34(
                                        0, receipt.viewWallIndex, &material) ||
                                    receipt.graphicIndex != material.plan.graphicIndex ||
                                    !original_inscription_oracle(state, original,
                                        original_size, &receipt, framebuffer) ||
                                    receipt.transparentColor != material.plan.transparentColor ||
                                    !verify_original_ornament_asset(
                                        state, &receipt, &material)) {
                                    continue;
                                }
                                /* ReDMCSB DUNVIEW.C F0128:8318-8616 redraws
                                 * the complete viewport for every party
                                 * direction.  A prior side/depth F0107
                                 * receipt cannot survive after the source
                                 * TextString has left that tuple. */
                                for (turn = 1; turn < 4; ++turn) {
                                    state->world.party.direction =
                                        (direction + turn) & 3;
                                    memset(turned, 0, sizeof(turned));
                                    M11_GameView_Draw(state, turned,
                                                      kFramebufferWidth,
                                                      kFramebufferHeight);
                                    memset(&turnedReceipt, 0,
                                           sizeof(turnedReceipt));
                                    M11_GameView_GetDm1UnreadableInscriptionHostPresentationReceipt(
                                        &turnedReceipt);
                                    if (!turnedReceipt.valid ||
                                        turnedReceipt.textStringIndex != textStringIndex) {
                                        foundDifferentTuple = 1;
                                        break;
                                    }
                                }
                                if (!foundDifferentTuple) {
                                    state->world.party.direction = direction;
                                    continue;
                                }
                                printf("ok: real PC34 unreadable inscription map=%d wall=(%d,%d) view=%d rel=(%d,%d)\n",
                                       mapIndex, x, y, receipt.viewWallIndex,
                                       receipt.relativeForward, receipt.relativeSide);
                                return 1;
                            }
                        }
                    }
                    thing = F0512_DUNGEON_GetThingNext_Compat(state->world.things, thing);
                }
            }
        }
    }
    fprintf(stderr, "C696 search eligible wall texts %u pose attempts %u valid receipts %u\n", eligible_texts, pose_attempts, valid_receipts);
    return 0;
}

int main(void)
{
    const char* dataDir = getenv("FIRESTAFF_DM1_DATA_DIR");
    char defaultDataDir[1024];
    const char* home;
    M11_GameViewState state;
    unsigned char* original = NULL;
    size_t original_size = 0u;
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
    if (!asset_file_matches_md5(state.assetLoader.graphicsDatPath,
            "fa6b1aa29e191418713bf2cda93d962e") ||
        !asset_read_path_alloc(state.assetLoader.graphicsDatPath, &original,
                              &original_size)) {
        M11_GameView_Shutdown(&state);
        return getenv("FIRESTAFF_DM1_DATA_DIR") ? 1 : 0;
    }
    state.presentationMode = M12_PRESENTATION_V1_ORIGINAL;
    state.world.party.championCount = 0;
    if (!find_and_verify_real_side_or_depth_pose(&state, original, original_size)) {
        /* No pose matches the strict source-locked shape (palette map,
         * ornament pixel opacity, F0128 tuple invariance across turns).
         * This search is a positive assertion — a checkout without a
         * matching pose is a documented capture gap, not a source
         * regression.  Fail only when the strict data-dir env is set. */
        fprintf(stderr, "no real PC34 unreadable side/depth inscription pose found\n");
        free(original);
        M11_GameView_Shutdown(&state);
        return getenv("FIRESTAFF_DM1_DATA_DIR") ? 1 : 0;
    }
    free(original);
    M11_GameView_Shutdown(&state);
    return 0;
}

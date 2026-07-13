/*
 * Real PC34 side/depth wall-inscription presentation regression.
 *
 * ReDMCSB DUNVIEW.C F0107:3864-3901 does not draw readable text away from
 * D1C.  It decodes the real TextString, counts its lines, shrinks the source
 * ornament box, and blits that original material with C10 transparency.
 */

#include "dm1_v1_inscription_font_pc34_compat.h"
#include "dm1_v1_wall_ornament_pc34_compat.h"
#include "m11_game_view.h"
#include "memory_dungeon_dat_pc34_compat.h"

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
        receipt->transparentColor != plan->transparentColor) {
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

static int find_and_verify_real_side_or_depth_pose(M11_GameViewState* state)
{
    unsigned char framebuffer[kFramebufferWidth * kFramebufferHeight];
    unsigned char turned[kFramebufferWidth * kFramebufferHeight];
    int mapIndex;
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
                                memset(framebuffer, 0, sizeof(framebuffer));
                                M11_GameView_Draw(state, framebuffer,
                                                  kFramebufferWidth, kFramebufferHeight);
                                memset(&receipt, 0, sizeof(receipt));
                                M11_GameView_GetDm1UnreadableInscriptionHostPresentationReceipt(
                                    &receipt);
                                if (!receipt.valid ||
                                    receipt.textStringIndex != textStringIndex) continue;
                                /* Multiple F0107 projection specs can share
                                 * one relative square.  F0128's later
                                 * source-order projection is authoritative,
                                 * so validate that published receipt rather
                                 * than assuming this candidate spec was last. */
                                if (receipt.lineCount != decoded_line_count(state, textStringIndex) ||
                                    receipt.boxHeight <= 0 ||
                                    !dm1_v1_wall_ornament_host_material_receipt_pc34(
                                        0, receipt.viewWallIndex, receipt.boxHeight, &material) ||
                                    receipt.graphicIndex != material.plan.graphicIndex ||
                                    receipt.destinationX != material.plan.dstX ||
                                    receipt.destinationY != kViewportY + material.plan.dstY ||
                                    receipt.width != material.plan.width ||
                                    receipt.height != material.plan.height ||
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
    return 0;
}

int main(void)
{
    const char* dataDir = getenv("FIRESTAFF_DM1_DATA_DIR");
    char defaultDataDir[1024];
    const char* home;
    M11_GameViewState state;
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
    if (!find_and_verify_real_side_or_depth_pose(&state)) {
        fprintf(stderr, "no real PC34 unreadable side/depth inscription pose found\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    M11_GameView_Shutdown(&state);
    return 0;
}

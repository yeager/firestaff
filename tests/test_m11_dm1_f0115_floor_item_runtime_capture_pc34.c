#include "m11_game_view.h"
#include "asset_loader_m11.h"
#include "dm1_v1_viewport_floor_ceiling_items_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static int start_real_dm1_corpus(M11_GameViewState* state, const char* path)
{
    if (!state || !path || !path[0]) return 0;
    /* This is a renderer/material regression, not a launcher-selection test.
     * Load the explicitly selected PC34 archive directly so unrelated
     * installed editions cannot change the startup-menu default. */
    return M11_GameView_StartDm1(state, path);
}

static int find_real_floor_item_pose(M11_GameViewState* state,
                                     int relativeForward,
                                     int relativeSide,
                                     unsigned char* framebuffer)
{
    const struct DungeonDatState_Compat* dungeon = state->world.dungeon;
    int mapIndex;

    if (!dungeon || !dungeon->maps) return 0;
    for (mapIndex = 1; mapIndex < (int)dungeon->header.mapCount; ++mapIndex) {
        const struct DungeonMapDesc_Compat* map = &dungeon->maps[mapIndex];
        int x;
        for (x = 0; x < (int)map->width; ++x) {
                int y;
                for (y = 0; y < (int)map->height; ++y) {
                    int direction;
                    for (direction = 0; direction < 4; ++direction) {
                        static const int kForwardX[4] = { 0, 1, 0, -1 };
                        static const int kForwardY[4] = { -1, 0, 1, 0 };
                        int rightX = kForwardY[direction];
                        int rightY = -kForwardX[direction];
                        int partyX = x - relativeForward * kForwardX[direction]
                            - relativeSide * rightX;
                        int partyY = y - relativeForward * kForwardY[direction]
                            - relativeSide * rightY;
                        int sampledMapX;
                        int sampledMapY;
                        int elementType;
                        int floorItems = 0;
                        int summaryItems;
                        if (partyX < 0 || partyY < 0 ||
                            partyX >= (int)map->width ||
                            partyY >= (int)map->height) {
                            continue;
                        }
                        state->world.partyMapIndex = mapIndex;
                        state->world.newPartyMapIndex = mapIndex;
                        state->world.party.mapIndex = mapIndex;
                        state->world.party.mapX = partyX;
                        state->world.party.mapY = partyY;
                        state->world.party.direction = direction;
                        if (M11_GameView_ProbeViewportFloorItemCounts(
                                state, relativeForward, relativeSide,
                                &sampledMapX, &sampledMapY,
                                &elementType, &floorItems, &summaryItems) &&
                            floorItems > 0) {
                            M11_Dm1F0115FloorItemRuntimeCaptureReceipt receipt;
                            memset(framebuffer, 0, 320 * 200);
                            M11_GameView_Draw(state, framebuffer, 320, 200);
                            memset(&receipt, 0, sizeof(receipt));
                            M11_GameView_GetDm1F0115FloorItemRuntimeCaptureReceipt(&receipt);
                            if (receipt.valid && receipt.presentation.floorItemLane) {
                                printf("real F0115 floor-item pose: map=%d party=(%d,%d,d%d) item=(%d,%d) relative=(%d,%d) graphic=%d hit=(%d,%d %dx%d)\n",
                                       mapIndex, partyX, partyY, direction,
                                       sampledMapX, sampledMapY,
                                       relativeForward, relativeSide,
                                       receipt.presentation.graphicsId,
                                       receipt.presentation.destinationX,
                                       receipt.presentation.destinationY,
                                       receipt.presentation.destinationW,
                                       receipt.presentation.destinationH);
                                return 1;
                            }
                        }
                    }
                }
        }
    }
    return 0;
}

static uint32_t real_asset_fnv1a(const unsigned char* pixels, size_t count)
{
    uint32_t hash = UINT32_C(2166136261);
    size_t i;
    for (i = 0; i < count; ++i) {
        hash = (hash ^ pixels[i]) * UINT32_C(16777619);
    }
    return hash;
}

static int verify_real_apple_transparency(M11_GameViewState* state,
                                          unsigned char* framebuffer)
{
    const struct DungeonDatState_Compat* dungeon = state->world.dungeon;
    struct DungeonThings_Compat* things = state->world.things;
    unsigned char withApple[320 * 200];
    unsigned char withoutApple[320 * 200];
    int mapIndex;

    if (!dungeon || !things || !things->junks) return 0;
    for (mapIndex = 0; mapIndex < (int)dungeon->header.mapCount; ++mapIndex) {
        const struct DungeonMapDesc_Compat* map = &dungeon->maps[mapIndex];
        int x;
        for (x = 0; x < (int)map->width; ++x) {
            int y;
            for (y = 0; y < (int)map->height; ++y) {
                const unsigned short first = F0511_DUNGEON_GetSquareFirstThing_Compat(
                    dungeon, things, mapIndex, x, y);
                const int sftIndex = F0510_DUNGEON_GetSquareFirstThingIndex_Compat(
                    dungeon, mapIndex, x, y);
                const int thingIndex = (int)THING_GET_INDEX(first);
                M11_Dm1F0115FloorItemRuntimeCaptureReceipt receipt;
                const M11_AssetSlot* apple;
                unsigned short savedFirst;
                int row;
                int transparentPixels = 0;
                int paintedPixels = 0;

                /* Isolate one original HoC apple so this check compares the
                 * renderer's real C10 holes with the same frame underneath. */
                if (sftIndex < 0 || THING_GET_TYPE(first) != THING_TYPE_JUNK ||
                    thingIndex >= things->junkCount ||
                    things->junks[thingIndex].type != 29 ||
                    F0512_DUNGEON_GetThingNext_Compat(things, first) !=
                        THING_ENDOFLIST) {
                    continue;
                }
                for (int direction = 0; direction < 4; ++direction) {
                    state->world.partyMapIndex = mapIndex;
                    state->world.newPartyMapIndex = mapIndex;
                    state->world.party.mapIndex = mapIndex;
                    state->world.party.mapX = x;
                    state->world.party.mapY = y;
                    state->world.party.direction = direction;
                    ++state->world.gameTick;
                    memset(framebuffer, 0, 320 * 200);
                    M11_GameView_Draw(state, framebuffer, 320, 200);
                    memset(&receipt, 0, sizeof(receipt));
                    M11_GameView_GetDm1F0115FloorItemRuntimeCaptureReceipt(&receipt);
                    if (!receipt.valid || !receipt.presentation.floorItemLane ||
                        receipt.presentation.graphicsId != 526) {
                        continue;
                    }

                    apple = M11_AssetLoader_Load(&state->assetLoader, 526u);
                    if (!apple || apple->width != 19u || apple->height != 13u ||
                        receipt.presentation.assetWidth != (int)apple->width ||
                        receipt.presentation.assetHeight != (int)apple->height ||
                        receipt.presentation.destinationW != (int)apple->width ||
                        receipt.presentation.destinationH != (int)apple->height ||
                        receipt.presentation.transparentColor != 10) {
                        return 0;
                    }
                    memcpy(withApple, framebuffer, sizeof(withApple));
                    savedFirst = things->squareFirstThings[sftIndex];
                    things->squareFirstThings[sftIndex] = THING_ENDOFLIST;
                    ++state->world.gameTick;
                    memset(withoutApple, 0, sizeof(withoutApple));
                    M11_GameView_Draw(state, withoutApple, 320, 200);
                    things->squareFirstThings[sftIndex] = savedFirst;

                    for (row = 0; row < (int)apple->height; ++row) {
                        int col;
                        for (col = 0; col < (int)apple->width; ++col) {
                            const int px = receipt.presentation.destinationX + col;
                            const int py = receipt.presentation.destinationY + row;
                            const unsigned char source =
                                apple->pixels[row * apple->width + col];
                            const int offset = py * 320 + px;
                            if (source == 10u) {
                                ++transparentPixels;
                                if (withApple[offset] != withoutApple[offset]) {
                                    fprintf(stderr,
                                            "PC34 apple transparency painted a C10 source pixel at %d,%d\n",
                                            col, row);
                                    return 0;
                                }
                            } else {
                                const unsigned char expected = (unsigned char)(
                                    0x10u | (source & 0x0fu));
                                if (withApple[offset] != expected) {
                                    fprintf(stderr,
                                            "PC34 apple pixel mismatch at %d,%d: source=%u expected=%u actual=%u\n",
                                            col, row, source, expected,
                                            withApple[offset]);
                                    return 0;
                                }
                                if (withApple[offset] != withoutApple[offset]) {
                                    ++paintedPixels;
                                }
                            }
                        }
                    }
                    if (!transparentPixels || !paintedPixels) return 0;
                    printf("real PC34 apple transparency: map=%d square=(%d,%d) "
                           "graphic=526 C10 holes=%d painted=%d\n",
                           mapIndex, x, y, transparentPixels, paintedPixels);
                    return 1;
                }
            }
        }
    }
    return 0;
}

int main(void)
{
    const char* dataDir = getenv("FIRESTAFF_DM1_DATA_DIR");
    M11_GameViewState state;
    M11_Dm1F0115FloorItemRuntimeCaptureReceipt receipt;
    M11_Dm1FloorItemHostPresentationReceipt stalePresentation;
    unsigned char framebuffer[320 * 200];
    int originalAssetsAvailable;
    const M11_AssetSlot* capturedItemGraphic;

    if (!dataDir || !dataDir[0]) {
        puts("SKIP: FIRESTAFF_DM1_DATA_DIR is not selected");
        return 0;
    }
    M11_GameView_Init(&state);
    if (!start_real_dm1_corpus(&state, dataDir) || !state.assetsAvailable) {
        M11_GameView_Shutdown(&state);
        fputs("configured PC34 corpus could not start\n", stderr);
        return 1;
    }
    state.presentationMode = M12_PRESENTATION_V1_ORIGINAL;
    if (!find_real_floor_item_pose(&state, 0, 0, framebuffer)) {
        M11_GameView_Shutdown(&state);
        puts("skip: real PC34 corpus has no drawable D0C F0115 floor item");
        return 0;
    }
    /* ReDMCSB F7019/G0237 maps junk subtype 29 (apple) to aspect 27 and
     * I34E's first native graphic 526. The pose selected above renders
     * graphic 561, so keep its decoder oracle separate from apple mapping. */
    if (dm1_item_sprite_index(THING_TYPE_JUNK, 29) != 526u) {
        M11_GameView_Shutdown(&state);
        fputs("ReDMCSB PC34 apple graphic mapping changed\n", stderr);
        return 1;
    }
    capturedItemGraphic = M11_AssetLoader_Load(&state.assetLoader, 561u);
    if (!capturedItemGraphic || capturedItemGraphic->width != 16u ||
        capturedItemGraphic->height != 5u ||
        real_asset_fnv1a(capturedItemGraphic->pixels,
                        (size_t)capturedItemGraphic->width *
                            capturedItemGraphic->height) !=
            UINT32_C(0xc8072551)) {
        M11_GameView_Shutdown(&state);
        fputs("real PC34 graphic 561 differs from the ReDMCSB I34E IMG3 decode\n",
              stderr);
        return 1;
    }
    state.world.gameTick += 1u;
    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&state, framebuffer, 320, 200);
    memset(&receipt, 0, sizeof(receipt));
    M11_GameView_GetDm1F0115FloorItemRuntimeCaptureReceipt(&receipt);
    if (!receipt.valid || receipt.runtimeTick != state.world.gameTick ||
        receipt.sourceTick != state.world.gameTick ||
        receipt.materialFNV1a == 0u || !receipt.presentation.valid ||
        !receipt.presentation.floorItemLane ||
        !receipt.presentation.usesF0791Blit ||
        receipt.presentation.graphicsId != 561 ||
        receipt.presentation.assetWidth <= 0 || receipt.presentation.assetHeight <= 0) {
        fprintf(stderr, "real D0C F0115 material did not reach final M11 capture\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }

    if (!verify_real_apple_transparency(&state, framebuffer)) {
        fprintf(stderr,
                "authentic PC34 apple did not preserve transparent pixels over its dungeon background\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }

    /* A floor-item click target belongs to the most recently rendered
     * frame. If the next frame cannot draw the original material, the old
     * coordinates must not remain interactive. Keep the real source pose
     * and Thing chain; only withdraw the renderer binding for one frame. */
    stalePresentation = receipt.presentation;
    originalAssetsAvailable = state.assetsAvailable;
    state.assetsAvailable = 0;
    state.world.gameTick += 1u;
    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&state, framebuffer, 320, 200);
    (void)M11_GameView_HandlePointer(
        &state,
        stalePresentation.destinationX + stalePresentation.destinationW / 2,
        stalePresentation.destinationY + stalePresentation.destinationH / 2,
        1);
    if (M11_GameView_GetV1LeaderHandThing(&state) != THING_NONE) {
        fprintf(stderr,
                "stale floor-item hit target remained active after its source material disappeared\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }

    /* Restore the real media and obtain a fresh frame-local target before
     * proving the authentic source item still picks up normally. */
    state.assetsAvailable = originalAssetsAvailable;
    state.world.gameTick += 1u;
    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&state, framebuffer, 320, 200);
    memset(&receipt, 0, sizeof(receipt));
    M11_GameView_GetDm1F0115FloorItemRuntimeCaptureReceipt(&receipt);
    if (!receipt.valid || !receipt.presentation.floorItemLane ||
        receipt.runtimeTick != state.world.gameTick) {
        fprintf(stderr, "real floor-item target did not refresh after source material returned\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }

    /* F0115's real blit rectangle is C080's pickup target. This must not
     * fall back to an approximate pane hit-box or a different chain item. */
    if (M11_GameView_HandlePointer(
            &state,
            receipt.presentation.destinationX + receipt.presentation.destinationW / 2,
            receipt.presentation.destinationY + receipt.presentation.destinationH / 2,
            1) != M11_GAME_INPUT_REDRAW ||
        M11_GameView_GetV1LeaderHandThing(&state) == THING_NONE) {
        fprintf(stderr, "real F0115 floor-item rectangle did not pick up its rendered pile top\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }

    /* Keep the real DUNGEON.DAT pose but withdraw the decoded GRAPHICS.DAT
     * binding. The next frame must clear this consumer rather than retain it. */
    state.world.gameTick += 1u;
    state.assetsAvailable = 0;
    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&state, framebuffer, 320, 200);
    memset(&receipt, 0xff, sizeof(receipt));
    M11_GameView_GetDm1F0115FloorItemRuntimeCaptureReceipt(&receipt);
    if (receipt.valid || receipt.runtimeTick != 0u || receipt.materialFNV1a != 0u) {
        fprintf(stderr, "missing GRAPHICS.DAT material retained stale F0115 capture\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    M11_GameView_Shutdown(&state);
    puts("ok: real PC34 D0C F0115 material reaches and clears final M11 capture");
    return 0;
}

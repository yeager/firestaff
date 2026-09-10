#include "m11_game_view.h"
#include "main_loop_m11.h"
#include "menu_startup_m12.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static int start_real_dm1_corpus(M11_GameViewState* state, const char* path)
{
    struct stat st;
    if (!state || !path || !path[0]) return 0;
    if (stat(path, &st) == 0 && S_ISREG(st.st_mode)) {
        M12_StartupMenuState menu;
        int opened;
        M12_StartupMenu_InitWithDataDir(&menu, path, "dm1");
        if (!M11_PrepareDirectLaunchForGame(&menu, "dm1")) {
            M12_StartupMenu_Destroy(&menu);
            return 0;
        }
        opened = M11_GameView_OpenSelectedMenuEntry(state, &menu);
        M12_StartupMenu_Destroy(&menu);
        return opened;
    }
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

int main(void)
{
    const char* dataDir = getenv("FIRESTAFF_DM1_DATA_DIR");
    M11_GameViewState state;
    M11_Dm1F0115FloorItemRuntimeCaptureReceipt receipt;
    unsigned char framebuffer[320 * 200];

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
        receipt.presentation.graphicsId <= 0 ||
        receipt.presentation.assetWidth <= 0 || receipt.presentation.assetHeight <= 0) {
        fprintf(stderr, "real D0C F0115 material did not reach final M11 capture\n");
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

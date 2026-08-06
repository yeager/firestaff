#include "m11_game_view.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static int regular_file_has_bytes(const char* path)
{
    struct stat st;
    return path && path[0] && stat(path, &st) == 0 && st.st_size > 0;
}

static int data_dir_has_pc34(const char* dir)
{
    char dungeon[1024];
    char graphics[1024];
    char data_dir[1024];
    if (!dir || !dir[0]) return 0;
    snprintf(dungeon, sizeof(dungeon), "%s/DUNGEON.DAT", dir);
    snprintf(graphics, sizeof(graphics), "%s/GRAPHICS.DAT", dir);
    if (regular_file_has_bytes(dungeon) && regular_file_has_bytes(graphics)) {
        return 1;
    }
    /* The normal PC34 archive layout stores the originals below DATA/. */
    snprintf(data_dir, sizeof(data_dir), "%s/DATA", dir);
    snprintf(dungeon, sizeof(dungeon), "%s/DUNGEON.DAT", data_dir);
    snprintf(graphics, sizeof(graphics), "%s/GRAPHICS.DAT", data_dir);
    return regular_file_has_bytes(dungeon) && regular_file_has_bytes(graphics);
}

static const char* resolve_data_dir(void)
{
    static char path[2048];
    static char data_path[2048];
    const char* env = getenv("FIRESTAFF_DM1_DATA_DIR");
    const char* root = getenv("FIRESTAFF_DATA");
    const char* home = getenv("HOME");

    if (data_dir_has_pc34(env)) {
        snprintf(data_path, sizeof(data_path), "%s/DATA", env);
        return data_dir_has_pc34(data_path) ? data_path : env;
    }
    if (data_dir_has_pc34(root)) {
        snprintf(data_path, sizeof(data_path), "%s/DATA", root);
        return data_dir_has_pc34(data_path) ? data_path : root;
    }
    if (root && root[0]) {
        snprintf(path, sizeof(path), "%s/dm1", root);
        if (data_dir_has_pc34(path)) {
            snprintf(data_path, sizeof(data_path), "%s/DATA", path);
            return data_dir_has_pc34(data_path) ? data_path : path;
        }
    }
    if (home && home[0]) {
        snprintf(path, sizeof(path), "%s/.firestaff/data/dm1", home);
        if (data_dir_has_pc34(path)) {
            snprintf(data_path, sizeof(data_path), "%s/DATA", path);
            return data_dir_has_pc34(data_path) ? data_path : path;
        }
        snprintf(path, sizeof(path), "%s/.firestaff/data", home);
        if (data_dir_has_pc34(path)) {
            snprintf(data_path, sizeof(data_path), "%s/DATA", path);
            return data_dir_has_pc34(data_path) ? data_path : path;
        }
    }
    return NULL;
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
    const char* dataDir = resolve_data_dir();
    M11_GameViewState state;
    M11_Dm1F0115FloorItemRuntimeCaptureReceipt receipt;
    unsigned char framebuffer[320 * 200];

    if (!dataDir) {
        puts("skip: local DM1 PC34 DUNGEON.DAT/GRAPHICS.DAT not available");
        return 0;
    }
    M11_GameView_Init(&state);
    if (!M11_GameView_StartDm1(&state, dataDir) || !state.assetsAvailable) {
        M11_GameView_Shutdown(&state);
        puts("skip: local PC34 corpus could not start");
        return 0;
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

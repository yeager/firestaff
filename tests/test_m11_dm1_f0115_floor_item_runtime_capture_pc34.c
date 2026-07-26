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
    if (!dir || !dir[0]) return 0;
    snprintf(dungeon, sizeof(dungeon), "%s/DUNGEON.DAT", dir);
    snprintf(graphics, sizeof(graphics), "%s/GRAPHICS.DAT", dir);
    return regular_file_has_bytes(dungeon) && regular_file_has_bytes(graphics);
}

static const char* resolve_data_dir(void)
{
    static char path[2048];
    const char* env = getenv("FIRESTAFF_DM1_DATA_DIR");
    const char* root = getenv("FIRESTAFF_DATA");
    const char* home = getenv("HOME");

    if (data_dir_has_pc34(env)) return env;
    if (data_dir_has_pc34(root)) return root;
    if (root && root[0]) {
        snprintf(path, sizeof(path), "%s/dm1", root);
        if (data_dir_has_pc34(path)) return path;
    }
    if (home && home[0]) {
        snprintf(path, sizeof(path), "%s/.firestaff/data/dm1", home);
        if (data_dir_has_pc34(path)) return path;
        snprintf(path, sizeof(path), "%s/.firestaff/data", home);
        if (data_dir_has_pc34(path)) return path;
    }
    return NULL;
}

static int find_real_non_hoc_floor_item_pose(M11_GameViewState* state)
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
                    int floorItems = 0;
                    state->world.partyMapIndex = mapIndex;
                    state->world.newPartyMapIndex = mapIndex;
                    state->world.party.mapIndex = mapIndex;
                    state->world.party.mapX = x;
                    state->world.party.mapY = y;
                    state->world.party.direction = direction;
                    if (M11_GameView_ProbeViewportFloorItemCounts(
                            state, 1, 0, NULL, NULL, NULL,
                            &floorItems, NULL) && floorItems > 0) {
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
    const char* dataDir = resolve_data_dir();
    M11_GameViewState state;
    M11_Dm1F0115FloorItemRuntimeCaptureReceipt receipt;
    unsigned char framebuffer[320 * 200];

    if (!dataDir) {
        puts("skip: local DM1 PC34 DUNGEON.DAT/GRAPHICS.DAT not available");
        return 0;
    }
    M11_GameView_Init(&state);
    if (!M11_GameView_StartDm1(&state, dataDir) || !state.assetsAvailable ||
        !find_real_non_hoc_floor_item_pose(&state)) {
        M11_GameView_Shutdown(&state);
        puts("skip: real PC34 corpus has no reachable non-HoC F0115 floor item");
        return 0;
    }
    state.presentationMode = M12_PRESENTATION_V1_ORIGINAL;
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
        fprintf(stderr, "real non-HoC F0115 material did not reach final M11 capture\n");
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
    puts("ok: real PC34 non-HoC F0115 material reaches and clears final M11 capture");
    return 0;
}

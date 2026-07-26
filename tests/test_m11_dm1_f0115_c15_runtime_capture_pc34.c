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

static int find_real_c15_capture(M11_GameViewState* state,
                                 unsigned char* framebuffer,
                                 M11_Dm1F0115C15RuntimeCaptureReceipt* receipt)
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
                    state->world.partyMapIndex = mapIndex;
                    state->world.newPartyMapIndex = mapIndex;
                    state->world.party.mapIndex = mapIndex;
                    state->world.party.mapX = x;
                    state->world.party.mapY = y;
                    state->world.party.direction = direction;
                    state->world.gameTick++;
                    memset(framebuffer, 0, 320 * 200);
                    M11_GameView_Draw(state, framebuffer, 320, 200);
                    memset(receipt, 0, sizeof(*receipt));
                    M11_GameView_GetDm1F0115C15RuntimeCaptureReceipt(receipt);
                    if (receipt->valid) return 1;
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
    M11_Dm1F0115C15RuntimeCaptureReceipt receipt;
    unsigned char framebuffer[320 * 200];

    if (!dataDir) {
        puts("skip: local DM1 PC34 DUNGEON.DAT/GRAPHICS.DAT not available");
        return 0;
    }
    M11_GameView_Init(&state);
    if (!M11_GameView_StartDm1(&state, dataDir) || !state.assetsAvailable ||
        !find_real_c15_capture(&state, framebuffer, &receipt)) {
        M11_GameView_Shutdown(&state);
        puts("skip: configured PC34 corpus has no visible non-HoC live C15 record");
        return 0;
    }
    if (receipt.runtimeTick != state.world.gameTick ||
        receipt.sourceTick != state.world.gameTick ||
        receipt.materialFNV1a == 0u || receipt.requestedMaterialCount <= 0 ||
        receipt.completedMaterialCount != receipt.requestedMaterialCount) {
        fprintf(stderr, "real C15 material did not complete final M11 capture\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    state.world.gameTick++;
    state.assetsAvailable = 0;
    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&state, framebuffer, 320, 200);
    memset(&receipt, 0xff, sizeof(receipt));
    M11_GameView_GetDm1F0115C15RuntimeCaptureReceipt(&receipt);
    if (receipt.valid || receipt.runtimeTick != 0u ||
        receipt.materialFNV1a != 0u || receipt.requestedMaterialCount != 0 ||
        receipt.completedMaterialCount != 0) {
        fprintf(stderr, "missing PC34 explosion material retained stale C15 capture\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    M11_GameView_Shutdown(&state);
    puts("ok: real PC34 C15 deferred explosion material reaches final M11 capture");
    return 0;
}

/* Real PC34 F0115 alcove-object presentation regression.
 *
 * F0121/F0124 call F0115 after F0107's real alcove wall material.  This
 * walks every loaded DM1 map/party pose and requires one genuine C2548
 * object blit to reach the dedicated wall-alcove receipt. */

#include "m11_game_view.h"
#include "memory_dungeon_dat_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { kFramebufferWidth = 320, kFramebufferHeight = 200 };

static int square_is_walkable(const M11_GameViewState *state,
                              int mapIndex, int x, int y)
{
    const struct DungeonMapDesc_Compat *map;
    unsigned char square;
    int index;

    if (!state || !state->world.dungeon || !state->world.dungeon->tiles ||
        mapIndex < 0 || mapIndex >= (int)state->world.dungeon->header.mapCount) {
        return 0;
    }
    map = &state->world.dungeon->maps[mapIndex];
    if (x < 0 || y < 0 || x >= (int)map->width || y >= (int)map->height ||
        !state->world.dungeon->tiles[mapIndex].squareData) {
        return 0;
    }
    index = x * (int)map->height + y;
    square = state->world.dungeon->tiles[mapIndex].squareData[index];
    return ((square & DUNGEON_SQUARE_MASK_TYPE) >> 5) ==
        DUNGEON_ELEMENT_CORRIDOR;
}

int main(void)
{
    const char *dataDir = getenv("FIRESTAFF_DM1_DATA_DIR");
    char defaultDataDir[1024];
    const char *home;
    M11_GameViewState state;
    unsigned char framebuffer[kFramebufferWidth * kFramebufferHeight];
    int mapIndex;

    if (!dataDir || !dataDir[0]) {
        home = getenv("HOME");
        if (!home || !home[0]) return 0;
        snprintf(defaultDataDir, sizeof(defaultDataDir),
                 "%s/.firestaff/data/dm1", home);
        dataDir = defaultDataDir;
    }
    M11_GameView_Init(&state);
    if (!M11_GameView_StartDm1(&state, dataDir)) {
        M11_GameView_Shutdown(&state);
        return getenv("FIRESTAFF_DM1_DATA_DIR") ? 1 : 0;
    }
    /* OBJECT.C F0031 resolves display names from M564 by icon index.  A
     * verified PC34 launch must therefore own the decoded original table,
     * rather than falling back to the legacy subtype-name bridge. */
    if (!state.dm1ObjectNameTableValid ||
        state.dm1ObjectNames[0][0] == '\0') {
        fprintf(stderr, "DM1 M564 object-name table was not loaded\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    state.presentationMode = M12_PRESENTATION_V1_ORIGINAL;
    state.world.party.championCount = 0;

    for (mapIndex = 0;
         mapIndex < (int)state.world.dungeon->header.mapCount;
         ++mapIndex) {
        const struct DungeonMapDesc_Compat *map = &state.world.dungeon->maps[mapIndex];
        int y;
        for (y = 0; y < (int)map->height; ++y) {
            int x;
            for (x = 0; x < (int)map->width; ++x) {
                int direction;
                if (!square_is_walkable(&state, mapIndex, x, y)) continue;
                for (direction = 0; direction < 4; ++direction) {
                    M11_Dm1FloorItemHostPresentationReceipt receipt;
                    state.world.party.mapIndex = mapIndex;
                    state.world.party.mapX = x;
                    state.world.party.mapY = y;
                    state.world.party.direction = direction;
                    ++state.world.gameTick;
                    memset(framebuffer, 0, sizeof(framebuffer));
                    M11_GameView_Draw(&state, framebuffer,
                                      kFramebufferWidth, kFramebufferHeight);
                    memset(&receipt, 0, sizeof(receipt));
                    M11_GameView_GetDm1AlcoveItemHostPresentationReceipt(&receipt);
                    if (receipt.valid && !receipt.floorItemLane &&
                        receipt.usesF0791Blit && receipt.transparentColor == 10 &&
                        receipt.sourceZone >= 2548 && receipt.destinationW > 0 &&
                        receipt.destinationH > 0 && receipt.graphicsId > 0) {
                        printf("ok: real PC34 alcove item map=%d party=(%d,%d,%d) graphic=%d zone=%d\n",
                               mapIndex, x, y, direction, receipt.graphicsId,
                               receipt.sourceZone);
                        M11_GameView_Shutdown(&state);
                        return 0;
                    }
                }
            }
        }
    }

    fprintf(stderr, "no real PC34 F0115 alcove object was presented\n");
    M11_GameView_Shutdown(&state);
    return 1;
}

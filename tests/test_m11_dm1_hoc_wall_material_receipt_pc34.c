/* Real PC34 HoC wall material receipt regression.
 * DUNVIEW.C F0096 materializes the map wall set before F0115/F0128 consumes
 * wall panels; F0172/F0107 route C127 through C346/C026, not host art. */

#include "dm1_v1_champion_mirror_pc34_compat.h"
#include "dm1_v1_graphic_ids_pc34_compat.h"
#include "dm1_v1_viewport_3d_pc34_compat.h"
#include "m11_game_view.h"
#include "memory_dungeon_dat_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { kFramebufferWidth = 320, kFramebufferHeight = 200 };

static int dx(int direction) { return (direction & 3) == 1 ? 1 : ((direction & 3) == 3 ? -1 : 0); }
static int dy(int direction) { return (direction & 3) == 2 ? 1 : ((direction & 3) == 0 ? -1 : 0); }

static int square_element(const M11_GameViewState *state, int x, int y)
{
    const struct DungeonMapDesc_Compat *map;
    int index;
    if (!state || !state->world.dungeon || !state->world.dungeon->tiles) return -1;
    map = &state->world.dungeon->maps[0];
    if (x < 0 || y < 0 || x >= (int)map->width || y >= (int)map->height) return -1;
    index = x * (int)map->height + y;
    return (state->world.dungeon->tiles[0].squareData[index] & DUNGEON_SQUARE_MASK_TYPE) >> 5;
}

static int find_front_mirror(M11_GameViewState *state,
                             unsigned char *framebuffer)
{
    const struct DungeonMapDesc_Compat *map = &state->world.dungeon->maps[0];
    int y;
    for (y = 0; y < (int)map->height; ++y) {
        int x;
        for (x = 0; x < (int)map->width; ++x) {
            unsigned short thing;
            int safety = 0;
            if (square_element(state, x, y) != DUNGEON_ELEMENT_WALL) continue;
            thing = F0511_DUNGEON_GetSquareFirstThing_Compat(
                state->world.dungeon, state->world.things, 0, x, y);
            while (thing != THING_NONE && thing != THING_ENDOFLIST && safety++ < 64) {
                if (THING_GET_TYPE(thing) == THING_TYPE_SENSOR) {
                    int sensorIndex = (int)THING_GET_INDEX(thing);
                    int direction = ((int)THING_GET_CELL(thing) + 2) & 3;
                    int partyX = x - dx(direction);
                    int partyY = y - dy(direction);
                    if (sensorIndex >= 0 && sensorIndex < state->world.things->sensorCount &&
                        state->world.things->sensors[sensorIndex].sensorType == 127 &&
                        square_element(state, partyX, partyY) == DUNGEON_ELEMENT_CORRIDOR) {
                        state->world.party.mapIndex = 0;
                        state->world.party.mapX = partyX;
                        state->world.party.mapY = partyY;
                        state->world.party.direction = direction;
                        memset(framebuffer, 0, kFramebufferWidth * kFramebufferHeight);
                        M11_GameView_Draw(state, framebuffer,
                                          kFramebufferWidth, kFramebufferHeight);
                        return M11_GameView_GetFrontMirrorOrdinal(state) ==
                               (int)state->world.things->sensors[sensorIndex].sensorData;
                    }
                }
                thing = F0512_DUNGEON_GetThingNext_Compat(state->world.things, thing);
            }
        }
    }
    return 0;
}

int main(void)
{
    const char *dataDir = getenv("FIRESTAFF_DM1_DATA_DIR");
    const char *home;
    char defaultDataDir[1024];
    M11_GameViewState state;
    DM1_ViewportWallHostMaterialReceiptPc34 receipt;
    const M11_AssetSlot *slot;
    unsigned char framebuffer[kFramebufferWidth * kFramebufferHeight];
    int mapWallSet;
    static const struct { int graphic, width, height; } kPanels[] = {
        {97, 160, 111}, {102, 106, 74}, {107, 70, 49}
    };
    size_t i;

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
    mapWallSet = (int)state.world.dungeon->maps[0].wallSet;
    for (i = 0; i < sizeof(kPanels) / sizeof(kPanels[0]); ++i) {
        if (!dm1_viewport_3d_wall_host_material_receipt_pc34(
                mapWallSet, kPanels[i].graphic, -1, false,
                kPanels[i].width, kPanels[i].height, &receipt) || !receipt.valid ||
            receipt.graphic_index < 0 || receipt.transparent_color != -1 ||
            receipt.expected_width != kPanels[i].width ||
            receipt.expected_height != kPanels[i].height) goto fail;
        slot = M11_AssetLoader_Load(&state.assetLoader,
                                    (unsigned int)receipt.graphic_index);
        if (!slot || !slot->loaded || !slot->pixels ||
            slot->width != (unsigned int)receipt.expected_width ||
            slot->height != (unsigned int)receipt.expected_height) goto fail;
    }
    slot = M11_AssetLoader_Load(&state.assetLoader,
                                DM1_V1_CHAMPION_PORTRAIT_GRAPHIC_PC34);
    if (!slot || !slot->loaded || !slot->pixels ||
        !dm1_v1_graphic_validate_champion_portrait_atlas_pc34(
            (int)slot->width, (int)slot->height) ||
        !find_front_mirror(&state, framebuffer)) goto fail;
    printf("ok: real PC34 HoC F0096 wall receipts and C127/C026 route are exact\n");
    M11_GameView_Shutdown(&state);
    return 0;
fail:
    fprintf(stderr, "real PC34 HoC wall material receipt or C127 route failed\n");
    M11_GameView_Shutdown(&state);
    return 1;
}

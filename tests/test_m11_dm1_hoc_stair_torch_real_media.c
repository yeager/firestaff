/* Authentic-media DM1 HoC stair torch interaction regression for Atari ST
 * and Amiga 2.0. This deliberately checks source ownership and runtime
 * transactions; platform-specific wall art assertions stay in their own
 * graphics-bound tests.
 * ReDMCSB source: CLIKVIEW.C:F0373 (78-129) transfers the pile-top Thing to
 * the leader hand; CLIKVIEW.C's C080 dispatch (406-438) routes the front-wall
 * hit to that path; CLIKMENU.C:F0364 (124-140) changes levels through stairs;
 * PANEL.C:F0337 (329-432) derives the dungeon palette from equipped torch
 * charge. */

#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "memory_dungeon_dat_pc34_compat.h"
#include "dm1_v1_dungeon_thing_data_pc34_compat.h"
#include "dm1_v1_mouse_routes_pc34_compat.h"
#include "dm1_v1_inventory_slot_placement_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

enum { kFrameW = 320, kFrameH = 200 };

static int start_corpus(M11_GameViewState *state, const char *archive)
{
    M12_StartupMenuState menu;
    M12_StartupMenuInitOptions options;
    int version;
    int opened;
    memset(&options, 0, sizeof(options));
    options.skipScreenshotGalleryScan = 1;
    M12_StartupMenu_InitWithOptions(&menu, archive, "dm1", &options);
#if defined(M11_DM1_TORCH_AMIGA20)
    version = M12_AssetStatus_FindVersionIndex("dm1", "amiga20-en");
    if (version >= 0 && M12_AssetStatus_GetVersion(&menu.assetStatus,
            "dm1", (size_t)version)->matched)
        menu.gameOptions[0].architectureIndex = M12_ARCH_AMIGA;
#elif defined(M11_DM1_TORCH_FMTOWNS)
    version = M12_AssetStatus_FindVersionIndex("dm1", "fmtowns-en");
    if (version >= 0 && M12_AssetStatus_GetVersion(&menu.assetStatus,
            "dm1", (size_t)version)->matched)
        menu.gameOptions[0].architectureIndex = M12_ARCH_FM_TOWNS;
#else
    version = M12_AssetStatus_FindVersionIndex("dm1", "st12-en");
    if (version >= 0 && M12_AssetStatus_GetVersion(&menu.assetStatus,
            "dm1", (size_t)version)->matched)
        menu.gameOptions[0].architectureIndex = M12_ARCH_ATARI_ST;
#endif
    if (version < 0 || !M12_AssetStatus_GetVersion(&menu.assetStatus,
            "dm1", (size_t)version)->matched) {
        M12_StartupMenu_Destroy(&menu);
        return 0;
    }
    menu.selectedIndex = menu.activatedIndex = 0;
    menu.launchRequested = 1;
    menu.gameOptions[0].versionIndex = version;
    menu.gameOptions[0].presentationModeIndex = M12_PRESENTATION_V1_ORIGINAL;
    opened = M11_GameView_OpenSelectedMenuEntry(state, &menu);
    M12_StartupMenu_Destroy(&menu);
    return opened;
}

static int find_source_champion(M11_GameViewState *state,
                                unsigned char *frame)
{
    const struct DungeonMapDesc_Compat *map = &state->world.dungeon->maps[0];
    static const int dx[4] = {0, 1, 0, -1};
    static const int dy[4] = {-1, 0, 1, 0};
    for (int y = 0; y < (int)map->height; ++y) {
        for (int x = 0; x < (int)map->width; ++x) {
            unsigned short thing = F0511_DUNGEON_GetSquareFirstThing_Compat(
                state->world.dungeon, state->world.things, 0, x, y);
            int guard = 0;
            while (thing != THING_NONE && thing != THING_ENDOFLIST && guard++ < 64) {
                if (THING_GET_TYPE(thing) == THING_TYPE_SENSOR) {
                    const int si = THING_GET_INDEX(thing);
                    if (si >= 0 && si < state->world.things->sensorCount &&
                        state->world.things->sensors[si].sensorType == 127) {
                        const int d = (THING_GET_CELL(thing) + 2) & 3;
                        state->world.party.mapIndex = 0;
                        state->world.party.mapX = x - dx[d];
                        state->world.party.mapY = y - dy[d];
                        state->world.party.direction = d;
                        memset(frame, 0, kFrameW * kFrameH);
                        M11_GameView_Draw(state, frame, kFrameW, kFrameH);
                        if (M11_GameView_GetFrontMirrorOrdinal(state) >= 0 &&
                            M11_GameView_SelectFrontMirrorCandidate(state) &&
                            M11_GameView_ConfirmMirrorCandidate(state, 0) &&
                            state->world.party.championCount > 0 &&
                            state->world.party.activeChampionIndex >= 0)
                            return 1;
                    }
                }
                thing = F0512_DUNGEON_GetThingNext_Compat(state->world.things, thing);
            }
        }
    }
    return 0;
}

static unsigned short find_wall_torch(M11_GameViewState *state)
{
    unsigned short thing = F0511_DUNGEON_GetSquareFirstThing_Compat(
        state->world.dungeon, state->world.things, 0, 3, 14);
    for (int guard = 0; thing != THING_NONE && thing != THING_ENDOFLIST && guard < 64; ++guard) {
        if (THING_GET_TYPE(thing) == THING_TYPE_WEAPON &&
            M11_GameView_GetObjectIconIndexForThing(state, thing) == 4)
            return thing;
        thing = F0512_DUNGEON_GetThingNext_Compat(state->world.things, thing);
    }
    return THING_NONE;
}

int main(void)
{
    const char *archive = getenv("FIRESTAFF_DM1_DATA_DIR");
    M11_GameViewState state;
    unsigned char frame[kFrameW * kFrameH];
    unsigned short torch;
    struct stat st;
    int result = 1;

    if (!archive || !archive[0] || stat(archive, &st) != 0 || !S_ISREG(st.st_mode)) {
        puts("SKIP: authentic DM1 archive is not selected");
        return 77;
    }
    M11_GameView_Init(&state);
    if (!start_corpus(&state, archive)) {
        M11_GameView_Shutdown(&state);
        fputs("selected authentic DM1 platform archive did not open through M12 -> M11\n", stderr);
        return 1;
    }
    state.presentationMode = M12_PRESENTATION_V1_ORIGINAL;
    if (state.world.dungeon->header.mapCount < 2 ||
        state.world.dungeon->maps[0].width <= 3 ||
        state.world.dungeon->maps[0].height <= 15 ||
        ((state.world.dungeon->tiles[0].squareData[3 *
            (int)state.world.dungeon->maps[0].height + 14] &
            DUNGEON_SQUARE_MASK_TYPE) >> 5) != DUNGEON_ELEMENT_WALL ||
        ((state.world.dungeon->tiles[0].squareData[3 *
            (int)state.world.dungeon->maps[0].height + 15] &
            DUNGEON_SQUARE_MASK_TYPE) >> 5) != DUNGEON_ELEMENT_STAIRS ||
        !(torch = find_wall_torch(&state)) || torch != 0x5463u) {
        fputs("authentic HoC cell (3,14)/(3,15) lacks wall, stairs, or icon-4 torch Thing\n", stderr);
        goto done;
    }
    /* The original chain includes the storage and wall-ornament sensors. */
    {
        unsigned short thing = F0511_DUNGEON_GetSquareFirstThing_Compat(
            state.world.dungeon, state.world.things, 0, 3, 14);
        int saw_ornament_sensor = 0, saw_storage_sensor = 0;
        int saw_torch = 0, guard = 0;
        while (thing != THING_NONE && thing != THING_ENDOFLIST && guard++ < 64) {
            if (THING_GET_TYPE(thing) == THING_TYPE_SENSOR) {
                const int index = THING_GET_INDEX(thing);
                if (index >= 0 && index < state.world.things->sensorCount &&
                    index == 152 &&
                    state.world.things->sensors[index].sensorType == 0 &&
                    state.world.things->sensors[index].ornamentOrdinal == 7)
                    saw_ornament_sensor = 1;
                if (index >= 0 && index < state.world.things->sensorCount &&
                    index == 7 &&
                    state.world.things->sensors[index].sensorType == 13 &&
                    state.world.things->sensors[index].sensorData == 4 &&
                    state.world.things->sensors[index].ornamentOrdinal == 8)
                    saw_storage_sensor = 1;
            }
            saw_torch |= thing == torch;
            thing = F0512_DUNGEON_GetThingNext_Compat(state.world.things, thing);
        }
        if (!saw_ornament_sensor || !saw_storage_sensor || !saw_torch || guard >= 64) {
            fputs("authentic stair-wall Thing chain is incomplete or cyclic\n", stderr);
            goto done;
        }
    }
    if (!find_source_champion(&state, frame)) {
        fputs("could not admit an original C127 champion through the runtime mirror path\n", stderr);
        goto done;
    }
    /* ReDMCSB C080 handles the C05 wall target from the corridor immediately
     * east of the stair-wall cell. The shared source click zone is 112,82. */
    state.world.party.mapIndex = 0;
    state.world.party.mapX = 4;
    state.world.party.mapY = 14;
    state.world.party.direction = 3;
    memset(frame, 0, sizeof(frame));
    M11_GameView_Draw(&state, frame, kFrameW, kFrameH);
    if (M11_GameView_HandlePointerButton(&state,
            112, 82, DM1_V1_MOUSE_MASK_LEFT_PC34) != M11_GAME_INPUT_REDRAW ||
        DM1_V1_M11Runtime_GetLeaderHandThingPc34Compat(&state) != torch) {
        fputs("native C080 wall click did not pick up the original HoC torch\n", stderr);
        goto done;
    }
    state.inventoryPanelActive = 1;
    state.dm1InventoryChampionOrdinal = state.world.party.activeChampionIndex + 1;
    {
        int x, y, w, h;
        if (!M11_GameView_GetV1InventorySourceSlotBoxZone(9, &x, &y, &w, &h) ||
            M11_GameView_HandlePointerButton(&state, x + w / 2,
                33 + y + h / 2, DM1_V1_MOUSE_MASK_LEFT_PC34) != M11_GAME_INPUT_REDRAW ||
            state.leaderHandObjectPresent ||
            state.world.party.champions[state.world.party.activeChampionIndex]
                .inventory[CHAMPION_SLOT_ACTION_HAND] != torch) {
            fputs("native C508 transaction did not equip the original torch\n", stderr);
            goto done;
        }
    }
    state.inventoryPanelActive = 0;
    state.dm1InventoryChampionOrdinal = 0;
    state.world.party.mapIndex = 0;
    state.world.party.mapX = 3;
    state.world.party.mapY = 15;
    if (M11_GameView_HandleInput(&state, M12_MENU_INPUT_USE_STAIRS) != M11_GAME_INPUT_REDRAW ||
        state.world.party.mapIndex != 1 || M11_GameView_GetLightLevel(&state) <= 0) {
        fprintf(stderr, "authentic stair descent did not retain torch light: map=%d light=%d\n",
            state.world.party.mapIndex, M11_GameView_GetLightLevel(&state));
        goto done;
    }
    printf("ok: authentic HoC wall torch Thing %u picked up, equipped, and carried through stairs with level-one light\n",
        (unsigned)torch);
    result = 0;
done:
    M11_GameView_Shutdown(&state);
    return result;
}

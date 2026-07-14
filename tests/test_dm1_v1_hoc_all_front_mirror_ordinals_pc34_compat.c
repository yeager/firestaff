#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "memory_dungeon_dat_pc34_compat.h"

#include <stdio.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

static unsigned short next_thing(const struct DungeonThings_Compat* things,
                                 unsigned short thing)
{
    int type;
    int index;
    const unsigned char* raw;

    if (!things || thing == THING_NONE || thing == THING_ENDOFLIST) {
        return THING_ENDOFLIST;
    }
    type = (int)THING_GET_TYPE(thing);
    index = (int)THING_GET_INDEX(thing);
    if (type < 0 || type >= DUNGEON_THING_TYPE_COUNT ||
        index < 0 || index >= things->thingCounts[type] ||
        !things->rawThingData[type] ||
        s_thingDataByteCount[type] < 2) {
        return THING_ENDOFLIST;
    }
    raw = things->rawThingData[type] + index * (int)s_thingDataByteCount[type];
    return (unsigned short)(raw[0] | ((unsigned short)raw[1] << 8));
}

static int open_game(const char* dataDir,
                     M12_StartupMenuState* menu,
                     M11_GameViewState* game)
{
    M12_StartupMenu_InitWithDataDir(menu, dataDir, NULL);
    M11_GameView_Init(game);
    return M11_GameView_OpenSelectedMenuEntry(game, menu);
}

static int step_x(int direction)
{
    static const int kDx[4] = { 0, 1, 0, -1 };
    return kDx[direction & 3];
}

static int step_y(int direction)
{
    static const int kDy[4] = { -1, 0, 1, 0 };
    return kDy[direction & 3];
}

int main(int argc, char** argv)
{
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    const struct DungeonMapDesc_Compat* map;
    int seen[32];
    int expectedCount = 0;
    int ok = 1;
    int x;
    int y;

    if (argc < 2) {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    dataDir = argv[1];
    memset(seen, 0, sizeof(seen));

    if (!open_game(dataDir, &menu, &game)) {
        fprintf(stderr, "FAIL could not open DM1 V1 game from %s\n", dataDir);
        return 1;
    }
    if (!game.world.dungeon || !game.world.things ||
        game.world.dungeon->header.mapCount < 1 ||
        !game.world.things->sensors) {
        fprintf(stderr, "FAIL DM1 dungeon/things unavailable\n");
        M11_GameView_Shutdown(&game);
        return 1;
    }

    map = &game.world.dungeon->maps[0];
    for (x = 0; x < (int)map->width; ++x) {
        for (y = 0; y < (int)map->height; ++y) {
            unsigned short thing =
                F0511_DUNGEON_GetSquareFirstThing_Compat(
                    game.world.dungeon, game.world.things, 0, x, y);
            int safety = 0;

            while (thing != THING_ENDOFLIST && thing != THING_NONE &&
                   safety < 64) {
                if (THING_GET_TYPE(thing) == THING_TYPE_SENSOR) {
                    int sensorIndex = (int)THING_GET_INDEX(thing);
                    if (sensorIndex >= 0 &&
                        sensorIndex < game.world.things->sensorCount &&
                        game.world.things->sensors[sensorIndex].sensorType == 127) {
                        int cell = (int)THING_GET_CELL(thing);
                        int expectedOrdinal =
                            (int)game.world.things->sensors[sensorIndex].sensorData;
                        int direction = (cell + 2) & 3;
                        int partyX = x - step_x(direction);
                        int partyY = y - step_y(direction);
                        int gotOrdinal;

                        if (expectedOrdinal >= 0 &&
                            expectedOrdinal < game.mirrorCatalog.count &&
                            partyX >= 0 && partyX < (int)map->width &&
                            partyY >= 0 && partyY < (int)map->height) {
                            game.world.party.mapIndex = 0;
                            game.world.party.mapX = partyX;
                            game.world.party.mapY = partyY;
                            game.world.party.direction = direction;
                            game.candidateMirrorPanelActive = 0;
                            game.candidateMirrorOrdinal = -1;
                            game.candidateMirrorPartyIndex = -1;

                            gotOrdinal = M11_GameView_GetFrontMirrorOrdinal(&game);
                            if (gotOrdinal != expectedOrdinal) {
                                fprintf(stderr,
                                        "FAIL HoC mirror at sensor=(%d,%d) cell=%d party=(%d,%d,%d) got=%d want=%d\n",
                                        x, y, cell, partyX, partyY, direction,
                                        gotOrdinal, expectedOrdinal);
                                ok = 0;
                            } else if (!seen[expectedOrdinal]) {
                                seen[expectedOrdinal] = 1;
                                expectedCount++;
                            }
                        }
                    }
                }
                thing = next_thing(game.world.things, thing);
                ++safety;
            }
        }
    }

    if (expectedCount != 24) {
        fprintf(stderr, "FAIL HoC visible mirror ordinal count got=%d want=24\n",
                expectedCount);
        ok = 0;
    }
    for (x = 0; x < 24; ++x) {
        if (!seen[x]) {
            fprintf(stderr, "FAIL HoC mirror ordinal %d was not reachable\n", x);
            ok = 0;
        }
    }

    printf("probe=dm1_v1_hoc_all_front_mirror_ordinals_pc34_compat\n");
    printf("sourceEvidence=ReDMCSB DUNGEON.C:2573,2608-2612 C127 front-wall portrait; MOVESENS.C:1501-1503; REVIVE.C F0280\n");
    printf("visibleMirrorOrdinals=%d\n", expectedCount);

    M11_GameView_Shutdown(&game);
    return ok ? 0 : 1;
}

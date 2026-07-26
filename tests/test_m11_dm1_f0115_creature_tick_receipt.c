#include "m11_game_view.h"
#include "memory_dungeon_dat_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char* pc34_dungeon_path(void)
{
    static char path[2048];
    const char* configured = getenv("DM1_PC34_DUNGEON_DAT");
    const char* home;

    if (configured && configured[0]) return configured;
    home = getenv("HOME");
    if (!home || !home[0]) return NULL;
    snprintf(path, sizeof(path), "%s/.firestaff/data/dm1/DUNGEON.DAT", home);
    return path;
}

int main(void)
{
    const char* path = pc34_dungeon_path();
    struct DungeonDatState_Compat dungeon;
    struct DungeonThings_Compat things;
    struct GameWorld_Compat world;
    M11_GameViewState state;
    int mapIndex;

    if (!path) return 0;
    memset(&dungeon, 0, sizeof(dungeon));
    memset(&things, 0, sizeof(things));
    memset(&world, 0, sizeof(world));
    if (!F0500_DUNGEON_LoadDatHeader_Compat(path, &dungeon) ||
        !F0502_DUNGEON_LoadTileData_Compat(path, &dungeon) ||
        !F0504_DUNGEON_LoadThingData_Compat(path, &dungeon, &things)) {
        if (getenv("DM1_PC34_DUNGEON_DAT")) {
            fprintf(stderr, "configured PC34 DUNGEON.DAT failed to load\n");
            return 1;
        }
        F0504_DUNGEON_FreeThingData_Compat(&things);
        F0500_DUNGEON_FreeDatHeader_Compat(&dungeon);
        return 0;
    }

    world.dungeon = &dungeon;
    world.things = &things;
    M11_GameView_Init(&state);
    state.world = world;
    for (mapIndex = 0; mapIndex < (int)dungeon.header.mapCount; ++mapIndex) {
        const struct DungeonMapDesc_Compat* map = &dungeon.maps[mapIndex];
        int x;
        for (x = 0; x < (int)map->width; ++x) {
            int y;
            for (y = 0; y < (int)map->height; ++y) {
                DM1_F0115WorldCandidatesPc34 sourceReceipt;
                DM1_F0115WorldGroupCandidatePc34 consumed;

                memset(&sourceReceipt, 0, sizeof(sourceReceipt));
                memset(&consumed, 0, sizeof(consumed));
                if (!dm1_v1_f0115_world_candidates_pc34(
                        &world, mapIndex, x, y, NULL, NULL, &sourceReceipt) ||
                    sourceReceipt.groupCount == 0) {
                    continue;
                }
                if (!M11_GameView_ProbeDm1F0115CreatureTickCandidate(
                        &state, mapIndex, x, y, &consumed) ||
                    consumed.thing != sourceReceipt.groups[0].thing ||
                    consumed.creatureType != sourceReceipt.groups[0].creatureType ||
                    consumed.creatureCount != sourceReceipt.groups[0].creatureCount ||
                    consumed.direction != sourceReceipt.groups[0].direction) {
                    fprintf(stderr, "M11 did not consume the real F0115 C04 receipt\n");
                    M11_GameView_Shutdown(&state);
                    F0504_DUNGEON_FreeThingData_Compat(&things);
                    F0500_DUNGEON_FreeDatHeader_Compat(&dungeon);
                    return 1;
                }
                M11_GameView_Shutdown(&state);
                F0504_DUNGEON_FreeThingData_Compat(&things);
                F0500_DUNGEON_FreeDatHeader_Compat(&dungeon);
                puts("ok: real PC34 F0115 creature tick receipt consumed by M11");
                return 0;
            }
        }
    }

    M11_GameView_Shutdown(&state);
    F0504_DUNGEON_FreeThingData_Compat(&things);
    F0500_DUNGEON_FreeDatHeader_Compat(&dungeon);
    puts("skip: PC34 corpus contains no F0115 creature candidate");
    return 0;
}

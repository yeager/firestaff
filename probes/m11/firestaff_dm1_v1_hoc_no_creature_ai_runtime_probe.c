/*
 * DM1 V1 Hall of Champions no-creature-AI runtime guard.
 *
 * The Hall of Champions has no active creature GROUP chains.  This probe
 * proves the real DM1 data reports zero compact GROUP chains on map 0 and
 * documents the dense-index false positives that the M11 creature-AI layer
 * must not process as HoC groups.
 *
 * Source-locked to ReDMCSB:
 *   DUNGEON.C F0160/F0161: SquareFirstThings is compact and indexed only by
 *     squares with MASK0x0010_THING_LIST_PRESENT.
 *   GROUP.C F0209/F0215: group processing is driven by real GROUP things.
 */

#include "asset_status_m12.h"
#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "memory_dungeon_dat_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

enum {
    HOC_MAP = 0
};

static int g_pass;
static int g_fail;

#define CHECK(cond, fmt, ...) \
    do { \
        if (cond) { \
            printf("PASS " fmt "\n", ##__VA_ARGS__); \
            ++g_pass; \
        } else { \
            printf("FAIL " fmt "\n", ##__VA_ARGS__); \
            ++g_fail; \
        } \
    } while (0)

static int file_exists(const char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) {
        return 0;
    }
    fclose(f);
    return 1;
}

static const char* narrow_dm1_data_dir(const char* dataDir,
                                       char* out,
                                       size_t outSize) {
    char graphicsPath[512];
    char dungeonPath[512];
    if (!dataDir || !out || outSize == 0U) {
        return dataDir;
    }
    snprintf(graphicsPath, sizeof(graphicsPath), "%s/dm1/GRAPHICS.DAT", dataDir);
    snprintf(dungeonPath, sizeof(dungeonPath), "%s/dm1/DUNGEON.DAT", dataDir);
    if (file_exists(graphicsPath) && file_exists(dungeonPath)) {
        snprintf(out, outSize, "%s/dm1", dataDir);
        return out;
    }
    return dataDir;
}

static unsigned short raw_next_thing(const struct DungeonThings_Compat* things,
                                     unsigned short thing) {
    static const unsigned char kThingDataByteCount[16] = {
        4, 6, 4, 8, 16, 4, 4, 4, 4, 8, 4, 0, 0, 0, 8, 4
    };
    int type;
    int index;
    const unsigned char* raw;
    if (!things || thing == THING_ENDOFLIST || thing == THING_NONE) {
        return THING_ENDOFLIST;
    }
    type = (int)THING_GET_TYPE(thing);
    index = (int)THING_GET_INDEX(thing);
    if (type < 0 || type >= 16 || kThingDataByteCount[type] == 0 ||
        !things->rawThingData[type] ||
        index < 0 || index >= things->thingCounts[type]) {
        return THING_ENDOFLIST;
    }
    raw = things->rawThingData[type] + index * (int)kThingDataByteCount[type];
    return (unsigned short)(raw[0] | ((unsigned short)raw[1] << 8));
}

static int count_group_chain(const struct DungeonThings_Compat* things,
                             unsigned short firstThing) {
    int count = 0;
    int safety = 0;
    unsigned short thing = firstThing;
    while (thing != THING_ENDOFLIST && thing != THING_NONE && safety < 64) {
        if (THING_GET_TYPE(thing) == THING_TYPE_GROUP) {
            ++count;
        }
        thing = raw_next_thing(things, thing);
        ++safety;
    }
    return count;
}

static int compact_group_count_on_hoc(const M11_GameViewState* state) {
    const struct DungeonMapDesc_Compat* map;
    int total = 0;
    int x;
    int y;
    if (!state || !state->world.dungeon || !state->world.things ||
        HOC_MAP >= (int)state->world.dungeon->header.mapCount) {
        return -1;
    }
    map = &state->world.dungeon->maps[HOC_MAP];
    for (x = 0; x < (int)map->width; ++x) {
        for (y = 0; y < (int)map->height; ++y) {
            unsigned short firstThing = F0511_DUNGEON_GetSquareFirstThing_Compat(
                state->world.dungeon, state->world.things, HOC_MAP, x, y);
            total += count_group_chain(state->world.things, firstThing);
        }
    }
    return total;
}

static int dense_group_count_on_hoc(const M11_GameViewState* state) {
    const struct DungeonMapDesc_Compat* map;
    int total = 0;
    int x;
    int y;
    if (!state || !state->world.dungeon || !state->world.things ||
        !state->world.things->squareFirstThings ||
        HOC_MAP >= (int)state->world.dungeon->header.mapCount) {
        return -1;
    }
    map = &state->world.dungeon->maps[HOC_MAP];
    for (x = 0; x < (int)map->width; ++x) {
        for (y = 0; y < (int)map->height; ++y) {
            int denseIndex = x * (int)map->height + y;
            if (denseIndex >= 0 &&
                denseIndex < state->world.things->squareFirstThingCount) {
                total += count_group_chain(
                    state->world.things,
                    state->world.things->squareFirstThings[denseIndex]);
            }
        }
    }
    return total;
}

static int message_log_contains(const M11_MessageLog* log, const char* needle) {
    int i;
    if (!log || !needle) {
        return 0;
    }
    for (i = 0; i < M11_MESSAGE_LOG_CAPACITY; ++i) {
        if (strstr(log->entries[i].text, needle)) {
            return 1;
        }
    }
    return 0;
}

int main(int argc, char** argv) {
    const char* root = argc > 1 ? argv[1] : getenv("FIRESTAFF_DATA");
    const char* dataDir;
    char narrowed[512];
    M12_StartupMenuState menu;
    M11_GameViewState game;
    int compactGroups;
    int denseGroups;

    if (!root || root[0] == '\0') {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    dataDir = narrow_dm1_data_dir(root, narrowed, sizeof(narrowed));

    printf("=== DM1 V1 HoC no-creature-AI runtime probe ===\n");
    printf("dataDir=%s\n", dataDir);

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    if (!M12_AssetStatus_GameAvailable(&menu.assetStatus, "dm1")) {
        printf("SKIP no hash-verified DM1 data under %s\n", dataDir);
        return 0;
    }

    M11_GameView_Init(&game);
    if (!M11_GameView_OpenSelectedMenuEntry(&game, &menu)) {
        fprintf(stderr, "FAIL could not open DM1 V1 game view\n");
        M11_GameView_Shutdown(&game);
        return 1;
    }

    game.world.party.mapIndex = HOC_MAP;
    game.world.party.mapX = 1;
    game.world.party.mapY = 5;
    game.world.party.direction = 1;
    memset(&game.messageLog, 0, sizeof(game.messageLog));

    compactGroups = compact_group_count_on_hoc(&game);
    denseGroups = dense_group_count_on_hoc(&game);

    CHECK(compactGroups == 0, "compact HoC GROUP chains=%d", compactGroups);
    CHECK(denseGroups > 0, "old dense HoC false GROUP chains=%d", denseGroups);
    CHECK(!message_log_contains(&game.messageLog, "REACHES THE PARTY"),
          "no HoC creature reach-party log at startup");
    CHECK(game.damageFlashTimer == 0,
          "no HoC creature damage flash at startup");

    printf("summary passed=%d failed=%d compactGroups=%d denseGroups=%d\n",
           g_pass, g_fail, compactGroups, denseGroups);

    M11_GameView_Shutdown(&game);
    return g_fail == 0 ? 0 : 1;
}

/*
 * DM1 V1 Hall of Champions floor runtime false-item guard.
 *
 * The data-only HoC floor gate proves the compact SquareFirstThings payload.
 * This runtime gate proves M11's viewport summary does not still draw fallback
 * objects from the old dense mapX*height+mapY interpretation.
 *
 * Source-locked to ReDMCSB:
 *   DUNGEON.C F0160/F0161: SquareFirstThings is compact and indexed only by
 *     squares with MASK0x0010_THING_LIST_PRESENT.
 *   DUNVIEW.C F0115: object drawing is driven by the source cell's real thing
 *     chain, not by a dense square index.
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
    HALL_MAP_INDEX = 0,
    MAX_VISIBLE_ITEMS = 4
};

static int g_pass = 0;
static int g_fail = 0;

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

static int thing_is_item(int thingType) {
    return thingType == THING_TYPE_WEAPON ||
           thingType == THING_TYPE_ARMOUR ||
           thingType == THING_TYPE_JUNK ||
           thingType == THING_TYPE_POTION ||
           thingType == THING_TYPE_SCROLL ||
           thingType == THING_TYPE_CONTAINER;
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

static int count_item_chain(const struct DungeonThings_Compat* things,
                            unsigned short firstThing) {
    int count = 0;
    int safety = 0;
    unsigned short thing = firstThing;
    while (thing != THING_ENDOFLIST && thing != THING_NONE && safety < 64) {
        if (thing_is_item((int)THING_GET_TYPE(thing))) {
            ++count;
        }
        thing = raw_next_thing(things, thing);
        ++safety;
    }
    return count;
}

static int square_index_for(const M11_GameViewState* state, int mapX, int mapY) {
    const struct DungeonMapDesc_Compat* map;
    if (!state || !state->world.dungeon ||
        HALL_MAP_INDEX >= (int)state->world.dungeon->header.mapCount) {
        return -1;
    }
    map = &state->world.dungeon->maps[HALL_MAP_INDEX];
    if (mapX < 0 || mapY < 0 ||
        mapX >= (int)map->width || mapY >= (int)map->height) {
        return -1;
    }
    return mapX * (int)map->height + mapY;
}

static int square_element_for(const M11_GameViewState* state, int mapX, int mapY) {
    int squareIndex = square_index_for(state, mapX, mapY);
    unsigned char square;
    if (squareIndex < 0 || !state->world.dungeon->tiles ||
        !state->world.dungeon->tiles[HALL_MAP_INDEX].squareData) {
        return -1;
    }
    square = state->world.dungeon->tiles[HALL_MAP_INDEX].squareData[squareIndex];
    return (square & DUNGEON_SQUARE_MASK_TYPE) >> 5;
}

static int dense_item_count_for_cell(const M11_GameViewState* state,
                                     int mapX,
                                     int mapY) {
    int squareIndex = square_index_for(state, mapX, mapY);
    if (squareIndex < 0 || !state->world.things ||
        !state->world.things->squareFirstThings ||
        squareIndex >= state->world.things->squareFirstThingCount) {
        return 0;
    }
    return count_item_chain(state->world.things,
                            state->world.things->squareFirstThings[squareIndex]);
}

static int compact_item_count_for_cell(const M11_GameViewState* state,
                                       int mapX,
                                       int mapY) {
    unsigned short firstThing;
    if (!state || !state->world.dungeon || !state->world.things) {
        return 0;
    }
    firstThing = F0511_DUNGEON_GetSquareFirstThing_Compat(
        state->world.dungeon, state->world.things,
        HALL_MAP_INDEX, mapX, mapY);
    return count_item_chain(state->world.things, firstThing);
}

int main(int argc, char** argv) {
    const char* dataDir = argc > 1 ? argv[1] : getenv("FIRESTAFF_DATA");
    M12_StartupMenuState menu;
    M11_GameViewState state;
    const struct DungeonMapDesc_Compat* map;
    int sampled = 0;
    int trueItemCells = 0;
    int denseFalsePositiveCells = 0;
    int correctedFalsePositiveSamples = 0;
    int mismatches = 0;
    int px;
    int py;
    int dir;

    if (!dataDir || dataDir[0] == '\0') {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }

    printf("=== DM1 V1 HoC floor runtime no-false-items probe ===\n");
    printf("dataDir=%s\n", dataDir);

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    if (!M12_AssetStatus_GameAvailable(&menu.assetStatus, "dm1")) {
        printf("SKIP no hash-verified DM1 data under %s\n", dataDir);
        return 0;
    }

    M11_GameView_Init(&state);
    if (!M11_GameView_OpenSelectedMenuEntry(&state, &menu)) {
        fprintf(stderr, "FAIL could not open DM1 V1 game view from %s\n", dataDir);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    state.world.party.championCount = 0;
    state.world.party.mapIndex = HALL_MAP_INDEX;

    if (!state.world.dungeon ||
        HALL_MAP_INDEX >= (int)state.world.dungeon->header.mapCount) {
        fprintf(stderr, "FAIL DM1 Hall map unavailable\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    map = &state.world.dungeon->maps[HALL_MAP_INDEX];

    for (py = 0; py < (int)map->height; ++py) {
        for (px = 0; px < (int)map->width; ++px) {
            if (square_element_for(&state, px, py) != DUNGEON_ELEMENT_CORRIDOR) {
                continue;
            }
            for (dir = 0; dir < 4; ++dir) {
                int relForward;
                state.world.party.mapX = px;
                state.world.party.mapY = py;
                state.world.party.direction = dir;
                for (relForward = 1; relForward <= 3; ++relForward) {
                    int relSide;
                    for (relSide = -1; relSide <= 1; ++relSide) {
                        int mapX = -1;
                        int mapY = -1;
                        int elementType = -1;
                        int floorItems = -1;
                        int summaryItems = -1;
                        int compactItems;
                        int denseItems;
                        if (!M11_GameView_ProbeViewportFloorItemCounts(
                                &state, relForward, relSide,
                                &mapX, &mapY, &elementType,
                                &floorItems, &summaryItems)) {
                            continue;
                        }
                        if (elementType == DUNGEON_ELEMENT_WALL) {
                            continue;
                        }
                        compactItems = compact_item_count_for_cell(&state, mapX, mapY);
                        denseItems = dense_item_count_for_cell(&state, mapX, mapY);
                        if (compactItems > MAX_VISIBLE_ITEMS) {
                            compactItems = MAX_VISIBLE_ITEMS;
                        }
                        ++sampled;
                        if (compactItems > 0) {
                            ++trueItemCells;
                        }
                        if (denseItems > 0 && compactItems == 0) {
                            ++denseFalsePositiveCells;
                            if (floorItems == 0 && summaryItems == 0) {
                                ++correctedFalsePositiveSamples;
                            }
                        }
                        if (floorItems != compactItems ||
                            summaryItems != floorItems) {
                            printf("MISMATCH party=(%d,%d,%d) rel=(%d,%d) cell=(%d,%d) dense=%d compact=%d floor=%d summary=%d\n",
                                   px, py, dir, relForward, relSide,
                                   mapX, mapY, denseItems, compactItems,
                                   floorItems, summaryItems);
                            ++mismatches;
                        }
                    }
                }
            }
        }
    }

    CHECK(sampled > 0, "sampled open HoC viewport cells count=%d", sampled);
    CHECK(trueItemCells > 0, "sampled real compact HoC floor item cells count=%d",
          trueItemCells);
    CHECK(denseFalsePositiveCells > 0,
          "found dense-index false-positive floor cells count=%d",
          denseFalsePositiveCells);
    CHECK(correctedFalsePositiveSamples == denseFalsePositiveCells,
          "corrected dense false-positive samples %d/%d",
          correctedFalsePositiveSamples, denseFalsePositiveCells);
    CHECK(mismatches == 0, "viewport floor summary matches compact floor chains mismatches=%d",
          mismatches);

    M11_GameView_Shutdown(&state);
    printf("summary=%d passed %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}

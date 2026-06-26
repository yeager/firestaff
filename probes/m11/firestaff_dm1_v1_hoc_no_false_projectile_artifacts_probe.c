/*
 * DM1 V1 Hall of Champions no false projectile/explosion artifact guard.
 *
 * HoC has no static fireballs or explosion things.  This probe proves the
 * real compact SquareFirstThings data for map 0 has zero projectile/explosion
 * chains and that M11's viewport summary no longer renders old dense-index
 * false positives as floating fireballs or floor artifacts.
 *
 * Source-locked to ReDMCSB:
 *   DUNGEON.C F0160/F0161: SquareFirstThings is compact and indexed only by
 *     squares with MASK0x0010_THING_LIST_PRESENT.
 *   DUNVIEW.C F0115/F0141: projectiles/explosions are drawn from the real
 *     current cell thing chain or runtime projectile/explosion lists.
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

static int chain_artifact_count(const struct DungeonThings_Compat* things,
                                unsigned short firstThing,
                                int* outProjectiles,
                                int* outExplosions) {
    int count = 0;
    int projectiles = 0;
    int explosions = 0;
    int safety = 0;
    unsigned short thing = firstThing;
    while (thing != THING_ENDOFLIST && thing != THING_NONE && safety < 64) {
        int type = (int)THING_GET_TYPE(thing);
        if (type == THING_TYPE_PROJECTILE) {
            ++projectiles;
            ++count;
        } else if (type == THING_TYPE_EXPLOSION) {
            ++explosions;
            ++count;
        }
        thing = raw_next_thing(things, thing);
        ++safety;
    }
    if (outProjectiles) *outProjectiles = projectiles;
    if (outExplosions) *outExplosions = explosions;
    return count;
}

static int square_index_for(const M11_GameViewState* state, int mapX, int mapY) {
    const struct DungeonMapDesc_Compat* map;
    if (!state || !state->world.dungeon ||
        HOC_MAP >= (int)state->world.dungeon->header.mapCount) {
        return -1;
    }
    map = &state->world.dungeon->maps[HOC_MAP];
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
        !state->world.dungeon->tiles[HOC_MAP].squareData) {
        return -1;
    }
    square = state->world.dungeon->tiles[HOC_MAP].squareData[squareIndex];
    return (square & DUNGEON_SQUARE_MASK_TYPE) >> 5;
}

static int dense_artifact_count_for_cell(const M11_GameViewState* state,
                                         int mapX,
                                         int mapY,
                                         int* outProjectiles,
                                         int* outExplosions) {
    int squareIndex = square_index_for(state, mapX, mapY);
    if (outProjectiles) *outProjectiles = 0;
    if (outExplosions) *outExplosions = 0;
    if (squareIndex < 0 || !state->world.things ||
        !state->world.things->squareFirstThings ||
        squareIndex >= state->world.things->squareFirstThingCount) {
        return 0;
    }
    return chain_artifact_count(
        state->world.things,
        state->world.things->squareFirstThings[squareIndex],
        outProjectiles,
        outExplosions);
}

static int compact_artifact_count_for_cell(const M11_GameViewState* state,
                                           int mapX,
                                           int mapY,
                                           int* outProjectiles,
                                           int* outExplosions) {
    unsigned short firstThing;
    if (outProjectiles) *outProjectiles = 0;
    if (outExplosions) *outExplosions = 0;
    if (!state || !state->world.dungeon || !state->world.things) {
        return 0;
    }
    firstThing = F0511_DUNGEON_GetSquareFirstThing_Compat(
        state->world.dungeon, state->world.things, HOC_MAP, mapX, mapY);
    return chain_artifact_count(state->world.things, firstThing,
                                outProjectiles, outExplosions);
}

int main(int argc, char** argv) {
    const char* root = argc > 1 ? argv[1] : getenv("FIRESTAFF_DATA");
    const char* dataDir;
    char narrowed[512];
    M12_StartupMenuState menu;
    M11_GameViewState state;
    const struct DungeonMapDesc_Compat* map;
    int compactProjectiles = 0;
    int compactExplosions = 0;
    int denseFalsePositiveSamples = 0;
    int correctedFalsePositiveSamples = 0;
    int sampled = 0;
    int viewportLeaks = 0;
    int firstGfxLeaks = 0;
    int px;
    int py;
    int dir;

    if (!root || root[0] == '\0') {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    dataDir = narrow_dm1_data_dir(root, narrowed, sizeof(narrowed));

    printf("=== DM1 V1 HoC no false projectile/explosion artifacts probe ===\n");
    printf("dataDir=%s\n", dataDir);

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    if (!M12_AssetStatus_GameAvailable(&menu.assetStatus, "dm1")) {
        printf("SKIP no hash-verified DM1 data under %s\n", dataDir);
        return 0;
    }

    M11_GameView_Init(&state);
    if (!M11_GameView_OpenSelectedMenuEntry(&state, &menu)) {
        fprintf(stderr, "FAIL could not open DM1 V1 game view\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    state.world.party.championCount = 0;
    state.world.party.mapIndex = HOC_MAP;

    if (!state.world.dungeon ||
        HOC_MAP >= (int)state.world.dungeon->header.mapCount) {
        fprintf(stderr, "FAIL DM1 Hall map unavailable\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    map = &state.world.dungeon->maps[HOC_MAP];

    for (py = 0; py < (int)map->height; ++py) {
        for (px = 0; px < (int)map->width; ++px) {
            int compactCellProjectiles = 0;
            int compactCellExplosions = 0;
            if (square_element_for(&state, px, py) == DUNGEON_ELEMENT_WALL) {
                continue;
            }
            (void)compact_artifact_count_for_cell(
                &state, px, py, &compactCellProjectiles, &compactCellExplosions);
            compactProjectiles += compactCellProjectiles;
            compactExplosions += compactCellExplosions;
        }
    }

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
                        int viewProjectiles = -1;
                        int viewExplosions = -1;
                        int firstProjectileGfx = -1;
                        int firstExplosionType = -1;
                        int denseProjectiles = 0;
                        int denseExplosions = 0;
                        int compactCellProjectiles = 0;
                        int compactCellExplosions = 0;
                        if (!M11_GameView_ProbeViewportArtifactCounts(
                                &state, relForward, relSide,
                                &mapX, &mapY, &elementType,
                                &viewProjectiles, &viewExplosions,
                                &firstProjectileGfx, &firstExplosionType)) {
                            continue;
                        }
                        if (elementType == DUNGEON_ELEMENT_WALL) {
                            continue;
                        }
                        ++sampled;
                        (void)dense_artifact_count_for_cell(
                            &state, mapX, mapY, &denseProjectiles, &denseExplosions);
                        (void)compact_artifact_count_for_cell(
                            &state, mapX, mapY,
                            &compactCellProjectiles, &compactCellExplosions);
                        if ((denseProjectiles + denseExplosions) > 0 &&
                            (compactCellProjectiles + compactCellExplosions) == 0) {
                            ++denseFalsePositiveSamples;
                            if (viewProjectiles == 0 && viewExplosions == 0) {
                                ++correctedFalsePositiveSamples;
                            }
                        }
                        if (viewProjectiles != compactCellProjectiles ||
                            viewExplosions != compactCellExplosions) {
                            printf("LEAK party=(%d,%d,%d) rel=(%d,%d) cell=(%d,%d) denseP=%d denseE=%d compactP=%d compactE=%d viewP=%d viewE=%d gfx=%d expType=%d\n",
                                   px, py, dir, relForward, relSide,
                                   mapX, mapY, denseProjectiles, denseExplosions,
                                   compactCellProjectiles, compactCellExplosions,
                                   viewProjectiles, viewExplosions,
                                   firstProjectileGfx, firstExplosionType);
                            ++viewportLeaks;
                        }
                        if ((viewProjectiles == 0 && firstProjectileGfx >= 0) ||
                            (viewExplosions == 0 && firstExplosionType >= 0)) {
                            ++firstGfxLeaks;
                        }
                    }
                }
            }
        }
    }

    CHECK(sampled > 0, "sampled open HoC viewport cells count=%d", sampled);
    CHECK(compactProjectiles == 0, "compact HoC static projectiles=%d",
          compactProjectiles);
    CHECK(compactExplosions == 0, "compact HoC static explosions=%d",
          compactExplosions);
    printf("INFO old dense projectile/explosion false-positive samples=%d\n",
           denseFalsePositiveSamples);
    CHECK(correctedFalsePositiveSamples == denseFalsePositiveSamples,
          "corrected dense artifact samples %d/%d",
          correctedFalsePositiveSamples, denseFalsePositiveSamples);
    CHECK(viewportLeaks == 0, "viewport projectile/explosion leaks=%d",
          viewportLeaks);
    CHECK(firstGfxLeaks == 0, "stale first projectile/explosion fields=%d",
          firstGfxLeaks);

    M11_GameView_Shutdown(&state);
    printf("summary=%d passed %d failed sampled=%d denseFalse=%d compactP=%d compactE=%d\n",
           g_pass, g_fail, sampled, denseFalsePositiveSamples,
           compactProjectiles, compactExplosions);
    return g_fail == 0 ? 0 : 1;
}

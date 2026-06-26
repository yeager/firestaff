#include "m11_game_view.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

enum {
    kFbW = 320,
    kFbH = 200,
    kThingGuard = 64
};

typedef struct ProbeCounts {
    int poses;
    int rendered;
    int blank;
    int mapsTouched;
    int sourceCells;
    int partyCells;
    int visibleSamples;
    int visibleThingSamples;
    int thingChains;
    int chainOverflows;
    int doors;
    int teleporters;
    int textStrings;
    int sensors;
    int groups;
    int items;
    int projectiles;
    int explosions;
} ProbeCounts;

static void ensure_output_dir(const char* outDir) {
    if (!outDir || outDir[0] == '\0') return;
#ifdef _WIN32
    (void)_mkdir(outDir);
#else
    (void)mkdir(outDir, 0777);
#endif
}

static const char* element_name(int elementType) {
    if (elementType >= 0 && elementType < DUNGEON_ELEMENT_COUNT) {
        return F0503_DUNGEON_GetElementName_Compat(elementType);
    }
    return "Unknown";
}

static const char* dir_name(int dir) {
    switch (dir & 3) {
        case 0: return "NORTH";
        case 1: return "EAST";
        case 2: return "SOUTH";
        case 3: return "WEST";
    }
    return "UNKNOWN";
}

static void direction_vectors(int dir, int* fx, int* fy, int* rx, int* ry) {
    switch (dir & 3) {
        case 0:
            *fx = 0; *fy = -1; *rx = 1; *ry = 0;
            break;
        case 1:
            *fx = 1; *fy = 0; *rx = 0; *ry = 1;
            break;
        case 2:
            *fx = 0; *fy = 1; *rx = -1; *ry = 0;
            break;
        default:
            *fx = -1; *fy = 0; *rx = 0; *ry = -1;
            break;
    }
}

static int get_square_byte(const struct GameWorld_Compat* world,
                           int mapIndex,
                           int mapX,
                           int mapY,
                           unsigned char* outSquare) {
    const struct DungeonMapDesc_Compat* map;
    const struct DungeonMapTiles_Compat* tiles;
    int index;
    if (!world || !world->dungeon || !world->dungeon->tilesLoaded ||
        !world->dungeon->maps || !outSquare) {
        return 0;
    }
    if (mapIndex < 0 || mapIndex >= (int)world->dungeon->header.mapCount) {
        return 0;
    }
    map = &world->dungeon->maps[mapIndex];
    if (mapX < 0 || mapY < 0 ||
        mapX >= (int)map->width || mapY >= (int)map->height) {
        return 0;
    }
    tiles = &world->dungeon->tiles[mapIndex];
    index = mapX * (int)map->height + mapY;
    if (!tiles->squareData || index < 0 || index >= tiles->squareCount) {
        return 0;
    }
    *outSquare = tiles->squareData[index];
    return 1;
}

static int is_party_render_cell(unsigned char square) {
    int elementType = (square & DUNGEON_SQUARE_MASK_TYPE) >> 5;
    return elementType != DUNGEON_ELEMENT_WALL;
}

static unsigned short raw_next_thing(const struct DungeonThings_Compat* things,
                                     unsigned short thing) {
    int type;
    int index;
    const unsigned char* raw;
    if (!things || thing == THING_NONE || thing == THING_ENDOFLIST) {
        return THING_ENDOFLIST;
    }
    type = (int)THING_GET_TYPE(thing);
    index = (int)THING_GET_INDEX(thing);
    if (type < 0 || type >= DUNGEON_THING_TYPE_COUNT ||
        !things->rawThingData[type] ||
        index < 0 || index >= things->thingCounts[type]) {
        return THING_ENDOFLIST;
    }
    raw = things->rawThingData[type] + index * s_thingDataByteCount[type];
    return (unsigned short)(raw[0] | ((unsigned short)raw[1] << 8));
}

static int thing_is_item(int thingType) {
    switch (thingType) {
        case THING_TYPE_WEAPON:
        case THING_TYPE_ARMOUR:
        case THING_TYPE_SCROLL:
        case THING_TYPE_POTION:
        case THING_TYPE_CONTAINER:
        case THING_TYPE_JUNK:
            return 1;
        default:
            return 0;
    }
}

static void count_square_things(const struct GameWorld_Compat* world,
                                int mapIndex,
                                int mapX,
                                int mapY,
                                ProbeCounts* counts) {
    unsigned short thing;
    int guard = 0;
    int saw = 0;
    if (!world || !world->dungeon || !world->things || !counts) return;

    thing = F0511_DUNGEON_GetSquareFirstThing_Compat(
        world->dungeon, world->things, mapIndex, mapX, mapY);
    while (thing != THING_ENDOFLIST && thing != THING_NONE &&
           guard < kThingGuard) {
        int thingType = (int)THING_GET_TYPE(thing);
        saw = 1;
        switch (thingType) {
            case THING_TYPE_DOOR: ++counts->doors; break;
            case THING_TYPE_TELEPORTER: ++counts->teleporters; break;
            case THING_TYPE_TEXTSTRING: ++counts->textStrings; break;
            case THING_TYPE_SENSOR: ++counts->sensors; break;
            case THING_TYPE_GROUP: ++counts->groups; break;
            case THING_TYPE_PROJECTILE: ++counts->projectiles; break;
            case THING_TYPE_EXPLOSION: ++counts->explosions; break;
            default:
                if (thing_is_item(thingType)) ++counts->items;
                break;
        }
        thing = raw_next_thing(world->things, thing);
        ++guard;
    }
    if (saw) {
        ++counts->thingChains;
    }
    if (guard >= kThingGuard) {
        ++counts->chainOverflows;
    }
}

static void count_visible_samples(const M11_GameViewState* game,
                                  ProbeCounts* counts) {
    int fx = 0, fy = 0, rx = 0, ry = 0;
    int depth;
    if (!game || !counts) return;
    direction_vectors(game->world.party.direction, &fx, &fy, &rx, &ry);
    for (depth = 1; depth <= 3; ++depth) {
        int side;
        for (side = -1; side <= 1; ++side) {
            int x = game->world.party.mapX + depth * fx + side * rx;
            int y = game->world.party.mapY + depth * fy + side * ry;
            unsigned char square = 0;
            if (get_square_byte(&game->world,
                                game->world.party.mapIndex,
                                x, y, &square)) {
                unsigned short first;
                ++counts->visibleSamples;
                first = F0511_DUNGEON_GetSquareFirstThing_Compat(
                    game->world.dungeon, game->world.things,
                    game->world.party.mapIndex, x, y);
                if (first != THING_ENDOFLIST && first != THING_NONE) {
                    ++counts->visibleThingSamples;
                }
                count_square_things(&game->world,
                                    game->world.party.mapIndex, x, y,
                                    counts);
            }
        }
    }
}

static int viewport_non_black(const unsigned char* fb,
                              int vx,
                              int vy,
                              int vw,
                              int vh) {
    int y;
    int count = 0;
    for (y = 0; y < vh; ++y) {
        int x;
        for (x = 0; x < vw; ++x) {
            if (fb[(vy + y) * kFbW + (vx + x)] != 0) {
                ++count;
            }
        }
    }
    return count;
}

static int write_report(const char* outDir,
                        const M11_GameViewState* game,
                        const ProbeCounts* counts,
                        int pass) {
    char mdPath[1024];
    char jsonPath[1024];
    FILE* md;
    FILE* js;
    int m;
    ensure_output_dir(outDir);
    snprintf(mdPath, sizeof(mdPath), "%s/dm1_v1_all_map_render_probe.md", outDir);
    snprintf(jsonPath, sizeof(jsonPath), "%s/dm1_v1_all_map_render_probe.json", outDir);
    md = fopen(mdPath, "w");
    js = fopen(jsonPath, "w");
    if (!md || !js) {
        if (md) fclose(md);
        if (js) fclose(js);
        return 0;
    }

    fprintf(md, "# DM1 V1 M11 all-map render probe\n\n");
    fprintf(md, "Source lock: ReDMCSB `DUNGEON.C` F0151 lines 1423-1478, F0161 lines 1730-1748, F0172 lines 2466-2710; `DUNVIEW.C` F0115 line 4547 and F0128/F0118+ lines 6642-8542.\n\n");
    fprintf(md, "This probe opens real DM1 data, moves the live M11 party view across every non-wall source cell on every map and renders each of the four directions through `M11_GameView_Draw`.\n\n");
    fprintf(md, "| metric | value |\n| --- | ---: |\n");
    fprintf(md, "| maps touched | %d |\n", counts->mapsTouched);
    fprintf(md, "| source cells | %d |\n", counts->sourceCells);
    fprintf(md, "| party-render cells | %d |\n", counts->partyCells);
    fprintf(md, "| rendered poses | %d / %d |\n", counts->rendered, counts->poses);
    fprintf(md, "| blank viewport failures | %d |\n", counts->blank);
    fprintf(md, "| visible sampled cells | %d |\n", counts->visibleSamples);
    fprintf(md, "| visible samples with thing chains | %d |\n", counts->visibleThingSamples);
    fprintf(md, "| traversed thing chains | %d |\n", counts->thingChains);
    fprintf(md, "| chain guard failures | %d |\n", counts->chainOverflows);
    fprintf(md, "| doors | %d |\n", counts->doors);
    fprintf(md, "| teleporters | %d |\n", counts->teleporters);
    fprintf(md, "| text strings | %d |\n", counts->textStrings);
    fprintf(md, "| sensors | %d |\n", counts->sensors);
    fprintf(md, "| groups | %d |\n", counts->groups);
    fprintf(md, "| item objects | %d |\n", counts->items);
    fprintf(md, "| projectiles | %d |\n", counts->projectiles);
    fprintf(md, "| explosions | %d |\n", counts->explosions);
    fprintf(md, "| status | %s |\n\n", pass ? "PASS" : "FAIL");
    fprintf(md, "## Maps\n\n");
    fprintf(md, "| map | level | size | floor set | wall set | door sets | ornaments | difficulty |\n");
    fprintf(md, "| ---: | ---: | --- | ---: | ---: | --- | --- | ---: |\n");
    for (m = 0; game && game->world.dungeon &&
                m < (int)game->world.dungeon->header.mapCount; ++m) {
        const struct DungeonMapDesc_Compat* map = &game->world.dungeon->maps[m];
        fprintf(md, "| %d | %u | %ux%u | %u | %u | %u/%u | wall %u, floor %u, door %u | %u |\n",
                m, (unsigned int)map->level,
                (unsigned int)map->width, (unsigned int)map->height,
                (unsigned int)map->floorSet, (unsigned int)map->wallSet,
                (unsigned int)map->doorSet0, (unsigned int)map->doorSet1,
                (unsigned int)map->wallOrnamentCount,
                (unsigned int)map->floorOrnamentCount,
                (unsigned int)map->doorOrnamentCount,
                (unsigned int)map->difficulty);
    }

    fprintf(js,
            "{\n"
            "  \"schema\": \"dm1_v1_m11_all_map_render_probe.v1\",\n"
            "  \"sourceLock\": \"ReDMCSB DUNGEON.C F0151/F0161/F0172; DUNVIEW.C F0115/F0128\",\n"
            "  \"status\": \"%s\",\n"
            "  \"mapsTouched\": %d,\n"
            "  \"sourceCells\": %d,\n"
            "  \"partyCells\": %d,\n"
            "  \"poses\": %d,\n"
            "  \"rendered\": %d,\n"
            "  \"blank\": %d,\n"
            "  \"visibleSamples\": %d,\n"
            "  \"visibleThingSamples\": %d,\n"
            "  \"thingChains\": %d,\n"
            "  \"chainOverflows\": %d,\n"
            "  \"things\": {\"doors\": %d, \"teleporters\": %d, \"textStrings\": %d, \"sensors\": %d, \"groups\": %d, \"items\": %d, \"projectiles\": %d, \"explosions\": %d}\n"
            "}\n",
            pass ? "PASS" : "FAIL",
            counts->mapsTouched, counts->sourceCells, counts->partyCells,
            counts->poses, counts->rendered, counts->blank,
            counts->visibleSamples, counts->visibleThingSamples,
            counts->thingChains, counts->chainOverflows,
            counts->doors, counts->teleporters, counts->textStrings,
            counts->sensors, counts->groups, counts->items,
            counts->projectiles, counts->explosions);

    fclose(md);
    fclose(js);
    printf("wrote %s and %s\n", mdPath, jsonPath);
    return 1;
}

int main(int argc, char** argv) {
    const char* dataDir;
    const char* outDir;
    M11_GameViewState game;
    unsigned char framebuffer[kFbW * kFbH];
    ProbeCounts counts;
    int vx = 0, vy = 0, vw = 0, vh = 0;
    int m;
    int pass;

    if (argc < 3) {
        fprintf(stderr, "usage: %s DATA_DIR OUT_DIR\n", argv[0]);
        return 2;
    }
    dataDir = argv[1];
    outDir = argv[2];
    memset(&counts, 0, sizeof(counts));

    M11_GameView_Init(&game);
    if (!M11_GameView_StartDm1(&game, dataDir)) {
        fprintf(stderr, "FAIL open DM1 data dir=%s\n", dataDir);
        M11_GameView_Shutdown(&game);
        return 1;
    }
    if (!game.world.dungeon || !game.world.dungeon->tilesLoaded ||
        !game.world.things || !M11_GameView_GetViewportRect(&vx, &vy, &vw, &vh)) {
        fprintf(stderr, "FAIL world or viewport not ready\n");
        M11_GameView_Shutdown(&game);
        return 1;
    }

    for (m = 0; m < (int)game.world.dungeon->header.mapCount; ++m) {
        const struct DungeonMapDesc_Compat* map = &game.world.dungeon->maps[m];
        int mapHadPose = 0;
        int x;
        for (x = 0; x < (int)map->width; ++x) {
            int y;
            for (y = 0; y < (int)map->height; ++y) {
                unsigned char square = 0;
                int elementType;
                int dir;
                if (!get_square_byte(&game.world, m, x, y, &square)) {
                    continue;
                }
                ++counts.sourceCells;
                elementType = (square & DUNGEON_SQUARE_MASK_TYPE) >> 5;
                if (!is_party_render_cell(square)) {
                    continue;
                }
                ++counts.partyCells;
                for (dir = 0; dir < 4; ++dir) {
                    int nonBlack;
                    memset(framebuffer, 0, sizeof(framebuffer));
                    game.world.party.mapIndex = m;
                    game.world.party.mapX = x;
                    game.world.party.mapY = y;
                    game.world.party.direction = dir;
                    game.candidateMirrorPanelActive = 0;
                    game.dialogOverlayActive = 0;
                    game.mapOverlayActive = 0;
                    game.inventoryPanelActive = 0;
                    game.v1ObjectDescriptionPanelActive = 0;
                    game.v1ScrollPanelActive = 0;
                    game.v1ChampionStatsPanelActive = 0;
                    game.v1FoodWaterPanelActive = 0;
                    ++counts.poses;
                    count_visible_samples(&game, &counts);
                    M11_GameView_Draw(&game, framebuffer, kFbW, kFbH);
                    nonBlack = viewport_non_black(framebuffer, vx, vy, vw, vh);
                    if (nonBlack <= 200) {
                        ++counts.blank;
                        fprintf(stderr,
                                "FAIL blank-ish viewport map=%d x=%d y=%d dir=%s element=%s nonBlack=%d\n",
                                m, x, y, dir_name(dir), element_name(elementType),
                                nonBlack);
                    } else {
                        ++counts.rendered;
                        mapHadPose = 1;
                    }
                }
            }
        }
        if (mapHadPose) {
            ++counts.mapsTouched;
        }
    }

    pass = counts.poses > 0 &&
           counts.rendered == counts.poses &&
           counts.blank == 0 &&
           counts.chainOverflows == 0 &&
           counts.mapsTouched == (int)game.world.dungeon->header.mapCount &&
           counts.visibleThingSamples > 0 &&
           (counts.items + counts.groups + counts.doors + counts.sensors +
            counts.textStrings + counts.teleporters) > 0;

    (void)write_report(outDir, &game, &counts, pass);
    printf("%s DM1 V1 all-map M11 render probe: maps=%d/%d poses=%d rendered=%d visibleThingSamples=%d items=%d groups=%d doors=%d sensors=%d text=%d teleporters=%d\n",
           pass ? "PASS" : "FAIL",
           counts.mapsTouched,
           (int)game.world.dungeon->header.mapCount,
           counts.poses,
           counts.rendered,
           counts.visibleThingSamples,
           counts.items,
           counts.groups,
           counts.doors,
           counts.sensors,
           counts.textStrings,
           counts.teleporters);
    M11_GameView_Shutdown(&game);
    return pass ? 0 : 1;
}

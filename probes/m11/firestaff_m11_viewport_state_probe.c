#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif

#include "asset_loader_m11.h"
#include "memory_dungeon_dat_pc34_compat.h"
#include "memory_tick_orchestrator_pc34_compat.h"

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

/*
 * Deterministic DM1 viewport-state composer.
 *
 * This probe intentionally does not drive DOSBox or keyboard input.  It loads
 * the original DUNGEON.DAT into the same source-backed GameWorld_Compat layer
 * used by M11, samples the 3-depth x 3-lane viewport neighborhood from the
 * party's current map/X/Y/direction, traverses square thing chains, and locks
 * the key GRAPHICS.DAT viewport asset dimensions.  The output is a stable
 * state anchor for later pixel overlay work.
 */

typedef struct ViewCellProbe {
    int valid;
    int relForward;
    int relSide;
    int mapX;
    int mapY;
    unsigned char square;
    int elementType;
    unsigned short firstThing;
    int thingCount;
    int doorCount;
    int groupCount;
    int itemCount;
    int sensorCount;
    int textCount;
    int teleporterCount;
    int projectileCount;
    int explosionCount;
    int firstDoorType;
    int firstDoorVertical;
    int firstDoorOrnament;
    int firstGroupCreatureType;
    int firstGroupCount;
    int firstGroupDirection;
} ViewCellProbe;

static void join_path(char* out, size_t outSize, const char* dir, const char* name) {
    size_t len;
    if (!out || outSize == 0) return;
    if (!dir || dir[0] == '\0') {
        snprintf(out, outSize, "%s", name ? name : "");
        return;
    }
    len = strlen(dir);
    if (len > 0 && (dir[len - 1] == '/' || dir[len - 1] == '\\')) {
        snprintf(out, outSize, "%s%s", dir, name ? name : "");
    } else {
        snprintf(out, outSize, "%s/%s", dir, name ? name : "");
    }
}

static int file_exists(const char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) return 0;
    fclose(f);
    return 1;
}

static int resolve_data_file(char* out, size_t outSize, const char* dataDir, const char* upper, const char* lower) {
    join_path(out, outSize, dataDir, upper);
    if (file_exists(out)) return 1;
    join_path(out, outSize, dataDir, lower);
    if (file_exists(out)) return 1;
    return 0;
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

static void direction_vectors(int direction, int* fx, int* fy, int* rx, int* ry) {
    switch (direction & 3) {
        case 0: *fx = 0; *fy = -1; *rx = 1; *ry = 0; break;
        case 1: *fx = 1; *fy = 0; *rx = 0; *ry = 1; break;
        case 2: *fx = 0; *fy = 1; *rx = -1; *ry = 0; break;
        default: *fx = -1; *fy = 0; *rx = 0; *ry = -1; break;
    }
}

static int map_square_base(const struct DungeonDatState_Compat* dungeon, int mapIndex) {
    int i;
    int base = 0;
    if (!dungeon || !dungeon->maps || mapIndex < 0 || mapIndex >= (int)dungeon->header.mapCount) return -1;
    for (i = 0; i < mapIndex; ++i) {
        base += (int)dungeon->maps[i].width * (int)dungeon->maps[i].height;
    }
    return base;
}

static int get_square_byte(const struct GameWorld_Compat* world, int mapIndex, int mapX, int mapY, unsigned char* outSquare) {
    const struct DungeonMapDesc_Compat* map;
    const struct DungeonMapTiles_Compat* tiles;
    int index;
    if (!world || !world->dungeon || !world->dungeon->tilesLoaded || !outSquare) return 0;
    if (mapIndex < 0 || mapIndex >= (int)world->dungeon->header.mapCount) return 0;
    map = &world->dungeon->maps[mapIndex];
    if (mapX < 0 || mapY < 0 || mapX >= (int)map->width || mapY >= (int)map->height) return 0;
    tiles = &world->dungeon->tiles[mapIndex];
    index = mapX * (int)map->height + mapY;
    if (!tiles->squareData || index < 0 || index >= tiles->squareCount) return 0;
    *outSquare = tiles->squareData[index];
    return 1;
}

static unsigned short raw_next_thing(const struct DungeonThings_Compat* things, unsigned short thing) {
    int type;
    int index;
    const unsigned char* raw;
    if (!things || thing == THING_NONE || thing == THING_ENDOFLIST) return THING_ENDOFLIST;
    type = THING_GET_TYPE(thing);
    index = THING_GET_INDEX(thing);
    if (type < 0 || type >= 16 || !things->rawThingData[type] || index < 0 || index >= things->thingCounts[type]) {
        return THING_ENDOFLIST;
    }
    raw = things->rawThingData[type] + (index * s_thingDataByteCount[type]);
    return (unsigned short)(raw[0] | ((unsigned short)raw[1] << 8));
}

static unsigned short first_square_thing(const struct GameWorld_Compat* world, int mapIndex, int mapX, int mapY) {
    int base;
    int squareIndex;
    const struct DungeonMapDesc_Compat* map;
    if (!world || !world->dungeon || !world->things || !world->things->squareFirstThings) return THING_ENDOFLIST;
    if (mapIndex < 0 || mapIndex >= (int)world->dungeon->header.mapCount) return THING_ENDOFLIST;
    map = &world->dungeon->maps[mapIndex];
    if (mapX < 0 || mapY < 0 || mapX >= (int)map->width || mapY >= (int)map->height) return THING_ENDOFLIST;
    base = map_square_base(world->dungeon, mapIndex);
    if (base < 0) return THING_ENDOFLIST;
    squareIndex = base + mapX * (int)map->height + mapY;
    if (squareIndex < 0 || squareIndex >= world->things->squareFirstThingCount) return THING_ENDOFLIST;
    return world->things->squareFirstThings[squareIndex];
}

static void inspect_thing(ViewCellProbe* cell, const struct DungeonThings_Compat* things, unsigned short thing) {
    int type = THING_GET_TYPE(thing);
    int index = THING_GET_INDEX(thing);
    if (!cell || !things || type < 0 || type >= 16 || index < 0 || index >= things->thingCounts[type]) return;
    switch (type) {
        case THING_TYPE_DOOR:
            ++cell->doorCount;
            if (cell->firstDoorType < 0 && things->doors && index < things->doorCount) {
                const struct DungeonDoor_Compat* d = &things->doors[index];
                cell->firstDoorType = d->type;
                cell->firstDoorVertical = d->vertical;
                cell->firstDoorOrnament = d->ornamentOrdinal;
            }
            break;
        case THING_TYPE_TELEPORTER: ++cell->teleporterCount; break;
        case THING_TYPE_TEXTSTRING: ++cell->textCount; break;
        case THING_TYPE_SENSOR: ++cell->sensorCount; break;
        case THING_TYPE_GROUP:
            ++cell->groupCount;
            if (cell->firstGroupCreatureType < 0 && things->groups && index < things->groupCount) {
                const struct DungeonGroup_Compat* g = &things->groups[index];
                cell->firstGroupCreatureType = g->creatureType;
                cell->firstGroupCount = (int)g->count + 1;
                cell->firstGroupDirection = g->direction;
            }
            break;
        case THING_TYPE_WEAPON:
        case THING_TYPE_ARMOUR:
        case THING_TYPE_SCROLL:
        case THING_TYPE_POTION:
        case THING_TYPE_CONTAINER:
        case THING_TYPE_JUNK:
            ++cell->itemCount;
            break;
        case THING_TYPE_PROJECTILE: ++cell->projectileCount; break;
        case THING_TYPE_EXPLOSION: ++cell->explosionCount; break;
        default: break;
    }
}

static ViewCellProbe sample_cell(const struct GameWorld_Compat* world, int relForward, int relSide) {
    ViewCellProbe cell;
    int fx = 0, fy = 0, rx = 0, ry = 0;
    unsigned short thing;
    int guard = 0;
    memset(&cell, 0, sizeof(cell));
    cell.relForward = relForward;
    cell.relSide = relSide;
    cell.firstThing = THING_ENDOFLIST;
    cell.firstDoorType = -1;
    cell.firstDoorVertical = -1;
    cell.firstDoorOrnament = -1;
    cell.firstGroupCreatureType = -1;
    cell.firstGroupCount = -1;
    cell.firstGroupDirection = -1;
    if (!world || !world->dungeon) return cell;
    direction_vectors(world->party.direction, &fx, &fy, &rx, &ry);
    cell.mapX = world->party.mapX + relForward * fx + relSide * rx;
    cell.mapY = world->party.mapY + relForward * fy + relSide * ry;
    if (!get_square_byte(world, world->party.mapIndex, cell.mapX, cell.mapY, &cell.square)) return cell;
    cell.valid = 1;
    cell.elementType = (cell.square & DUNGEON_SQUARE_MASK_TYPE) >> 5;
    thing = first_square_thing(world, world->party.mapIndex, cell.mapX, cell.mapY);
    cell.firstThing = thing;
    while (thing != THING_ENDOFLIST && thing != THING_NONE && guard < 64) {
        ++cell.thingCount;
        inspect_thing(&cell, world->things, thing);
        thing = raw_next_thing(world->things, thing);
        ++guard;
    }
    return cell;
}

static void ensure_output_dir(const char* outDir) {
    if (!outDir || outDir[0] == '\0') return;
#ifdef _WIN32
    (void)_mkdir(outDir);
#else
    (void)mkdir(outDir, 0777);
#endif
}

static int write_outputs(const char* outDir,
                         const char* dungeonPath,
                         const char* graphicsPath,
                         const struct GameWorld_Compat* world,
                         M11_AssetLoader* loader,
                         const ViewCellProbe cells[3][3]) {
    char mdPath[512];
    char jsonPath[512];
    FILE* md;
    FILE* js;
    int depth, lane;
    unsigned int assetIndices[] = {0u, 78u, 79u, 8u, 9u, 10u, 42u};
    int pass = 1;

    ensure_output_dir(outDir);
    join_path(mdPath, sizeof(mdPath), outDir, "dm1_viewport_state_probe.md");
    join_path(jsonPath, sizeof(jsonPath), outDir, "dm1_viewport_state_probe.json");
    md = fopen(mdPath, "w");
    js = fopen(jsonPath, "w");
    if (!md || !js) {
        fprintf(stderr, "FAIL: cannot write probe outputs in %s: %s\n", outDir, strerror(errno));
        if (md) fclose(md);
        if (js) fclose(js);
        return 0;
    }

    fprintf(md, "# DM1 source-driven viewport state probe\n\n");
    fprintf(md, "This is a deterministic source-state anchor from `DUNGEON.DAT` + `GRAPHICS.DAT`; it does not use DOSBox input or screenshots.\n\n");
    fprintf(md, "## Inputs\n\n");
    fprintf(md, "- DUNGEON.DAT: `%s`\n", dungeonPath);
    fprintf(md, "- GRAPHICS.DAT: `%s`\n", graphicsPath);
    fprintf(md, "- Maps: %d\n", (int)world->dungeon->header.mapCount);
    fprintf(md, "- Square-first-thing entries: %d\n\n", world->things ? world->things->squareFirstThingCount : 0);
    fprintf(md, "## Party source state\n\n");
    fprintf(md, "| Field | Value |\n| --- | --- |\n");
    fprintf(md, "| mapIndex | %d |\n", world->party.mapIndex);
    fprintf(md, "| mapX | %d |\n", world->party.mapX);
    fprintf(md, "| mapY | %d |\n", world->party.mapY);
    fprintf(md, "| direction | %d / %s |\n\n", world->party.direction, dir_name(world->party.direction));

    fprintf(md, "## Viewport neighborhood\n\n");
    fprintf(md, "Rows are relative forward depth 0..2; lanes are left/center/right relative to party facing.\n\n");
    fprintf(md, "| depth | lane | mapX | mapY | square | element | firstThing | things | door | groups | items | sensors | text | teleporters | projectiles | explosions |\n");
    fprintf(md, "| --- | --- | ---: | ---: | --- | --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |\n");
    for (depth = 0; depth < 3; ++depth) {
        for (lane = 0; lane < 3; ++lane) {
            const ViewCellProbe* c = &cells[depth][lane];
            int relSide = lane - 1;
            if (!c->valid) {
                fprintf(md, "| %d | %+d | %d | %d | OUT | OUT_OF_BOUNDS | - | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |\n",
                        depth, relSide, c->mapX, c->mapY);
                pass = 0;
                continue;
            }
            fprintf(md, "| %d | %+d | %d | %d | 0x%02X | %s | 0x%04X | %d | %d | %d | %d | %d | %d | %d | %d | %d |\n",
                    depth, relSide, c->mapX, c->mapY, c->square, element_name(c->elementType), c->firstThing,
                    c->thingCount, c->doorCount, c->groupCount, c->itemCount, c->sensorCount, c->textCount,
                    c->teleporterCount, c->projectileCount, c->explosionCount);
        }
    }

    fprintf(md, "\n## GRAPHICS.DAT viewport asset lock\n\n");
    fprintf(md, "| graphic | width | height | status |\n| ---: | ---: | ---: | --- |\n");
    fprintf(js, "{\n  \"party\": {\"mapIndex\": %d, \"mapX\": %d, \"mapY\": %d, \"direction\": %d},\n",
            world->party.mapIndex, world->party.mapX, world->party.mapY, world->party.direction);
    fprintf(js, "  \"cells\": [\n");
    for (depth = 0; depth < 3; ++depth) {
        for (lane = 0; lane < 3; ++lane) {
            const ViewCellProbe* c = &cells[depth][lane];
            int last = (depth == 2 && lane == 2);
            fprintf(js, "    {\"depth\":%d,\"lane\":%d,\"valid\":%d,\"mapX\":%d,\"mapY\":%d,\"square\":%u,\"elementType\":%d,\"thingCount\":%d,\"doorCount\":%d,\"groupCount\":%d,\"itemCount\":%d,\"sensorCount\":%d}%s\n",
                    depth, lane - 1, c->valid, c->mapX, c->mapY, (unsigned int)c->square, c->elementType,
                    c->thingCount, c->doorCount, c->groupCount, c->itemCount, c->sensorCount, last ? "" : ",");
        }
    }
    fprintf(js, "  ],\n  \"assets\": [\n");
    for (depth = 0; depth < (int)(sizeof(assetIndices) / sizeof(assetIndices[0])); ++depth) {
        unsigned short w = 0, h = 0;
        int ok = M11_AssetLoader_QuerySize(loader, assetIndices[depth], &w, &h);
        fprintf(md, "| %u | %u | %u | %s |\n", assetIndices[depth], (unsigned int)w, (unsigned int)h, ok ? "PASS" : "FAIL");
        fprintf(js, "    {\"graphic\":%u,\"width\":%u,\"height\":%u,\"ok\":%d}%s\n",
                assetIndices[depth], (unsigned int)w, (unsigned int)h, ok, depth == (int)(sizeof(assetIndices) / sizeof(assetIndices[0])) - 1 ? "" : ",");
        if (!ok || w == 0 || h == 0) pass = 0;
    }
    fprintf(js, "  ],\n  \"pass\": %s\n}\n", pass ? "true" : "false");

    fprintf(md, "\n## Invariants\n\n");
    fprintf(md, "- %s: DUNGEON.DAT loaded into GameWorld_Compat\n", world->dungeon && world->dungeon->loaded ? "PASS" : "FAIL");
    fprintf(md, "- %s: tile layer loaded\n", world->dungeon && world->dungeon->tilesLoaded ? "PASS" : "FAIL");
    fprintf(md, "- %s: thing layer loaded\n", world->things && world->things->loaded ? "PASS" : "FAIL");
    fprintf(md, "- %s: 3x3 viewport sample stayed inside source map\n", pass ? "PASS" : "FAIL");
    fprintf(md, "- %s: GRAPHICS.DAT critical viewport sizes queryable\n", pass ? "PASS" : "FAIL");

    fclose(md);
    fclose(js);
    printf("%s source-driven viewport state probe\n", pass ? "PASS" : "FAIL");
    printf("%s\n%s\n", mdPath, jsonPath);
    return pass;
}

int main(int argc, char** argv) {
    char dungeonPath[512];
    char graphicsPath[512];
    struct GameWorld_Compat world;
    M11_AssetLoader loader;
    ViewCellProbe cells[3][3];
    int depth, lane;
    int ok;

    if (argc < 3) {
        fprintf(stderr, "Usage: %s <data_dir> <output_dir>\n", argv[0]);
        return 2;
    }
    memset(&world, 0, sizeof(world));
    memset(&loader, 0, sizeof(loader));

    if (!resolve_data_file(dungeonPath, sizeof(dungeonPath), argv[1], "DUNGEON.DAT", "dungeon.dat")) {
        fprintf(stderr, "FAIL: DUNGEON.DAT not found under %s\n", argv[1]);
        return 1;
    }
    if (!resolve_data_file(graphicsPath, sizeof(graphicsPath), argv[1], "GRAPHICS.DAT", "graphics.dat")) {
        fprintf(stderr, "FAIL: GRAPHICS.DAT not found under %s\n", argv[1]);
        return 1;
    }
    if (F0882_WORLD_InitFromDungeonDat_Compat(dungeonPath, 0xF1A5u, &world) != 1) {
        fprintf(stderr, "FAIL: could not initialise GameWorld_Compat from %s\n", dungeonPath);
        return 1;
    }
    if (!M11_AssetLoader_Init(&loader, graphicsPath)) {
        fprintf(stderr, "FAIL: could not initialise GRAPHICS.DAT loader from %s\n", graphicsPath);
        F0883_WORLD_Free_Compat(&world);
        return 1;
    }

    for (depth = 0; depth < 3; ++depth) {
        for (lane = 0; lane < 3; ++lane) {
            cells[depth][lane] = sample_cell(&world, depth, lane - 1);
        }
    }

    ok = write_outputs(argv[2], dungeonPath, graphicsPath, &world, &loader, cells);
    M11_AssetLoader_Shutdown(&loader);
    F0883_WORLD_Free_Compat(&world);
    return ok ? 0 : 1;
}

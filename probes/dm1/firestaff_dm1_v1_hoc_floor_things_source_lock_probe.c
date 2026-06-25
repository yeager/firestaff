#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory_dungeon_dat_pc34_compat.h"

#define MAKE_THING(type, index, cell) \
    ((unsigned short)((((cell) & 0x03) << 14) | (((type) & 0x0F) << 10) | ((index) & 0x03FF)))

struct ExpectedFloorChain {
    int x;
    int y;
    int count;
    unsigned short things[2];
};

static const struct ExpectedFloorChain s_expectedFloorChains[] = {
    { 0, 16, 1, { MAKE_THING(THING_TYPE_JUNK, 242, 3), THING_ENDOFLIST } },
    { 2, 11, 1, { MAKE_THING(THING_TYPE_JUNK, 146, 1), THING_ENDOFLIST } },
    { 2, 12, 1, { MAKE_THING(THING_TYPE_JUNK, 249, 1), THING_ENDOFLIST } },
    { 2, 13, 1, { MAKE_THING(THING_TYPE_JUNK, 248, 2), THING_ENDOFLIST } },
    { 4,  9, 1, { MAKE_THING(THING_TYPE_JUNK, 148, 0), THING_ENDOFLIST } },
    { 4, 15, 2, { MAKE_THING(THING_TYPE_JUNK,  11, 0), MAKE_THING(THING_TYPE_SCROLL, 0, 1) } },
    { 5, 11, 1, { MAKE_THING(THING_TYPE_JUNK, 150, 1), THING_ENDOFLIST } }
};

static int file_exists(const char* path)
{
    FILE* file;
    unsigned char byte;
    if (!path || !path[0]) {
        return 0;
    }
    file = fopen(path, "rb");
    if (!file) {
        return 0;
    }
    if (fread(&byte, 1, 1, file) != 1) {
        fclose(file);
        return 0;
    }
    fclose(file);
    return 1;
}

static int try_candidate(char* outPath, size_t outSize, const char* base, const char* suffix)
{
    if (!base || !base[0]) {
        return 0;
    }
    if (suffix && suffix[0]) {
        snprintf(outPath, outSize, "%s/%s", base, suffix);
    } else {
        snprintf(outPath, outSize, "%s", base);
    }
    return file_exists(outPath);
}

static int resolve_dungeon_path(int argc, char** argv, char* outPath, size_t outSize)
{
    const char* home;

    if (argc >= 2) {
        if (try_candidate(outPath, outSize, argv[1], "")) return 1;
        if (try_candidate(outPath, outSize, argv[1], "DUNGEON.DAT")) return 1;
        if (try_candidate(outPath, outSize, argv[1], "dm1/DUNGEON.DAT")) return 1;
    }

    home = getenv("HOME");
    if (home && home[0]) {
        char dataRoot[512];
        snprintf(dataRoot, sizeof(dataRoot), "%s/.firestaff/data", home);
        if (try_candidate(outPath, outSize, dataRoot, "dm1/DUNGEON.DAT")) return 1;
        if (try_candidate(outPath, outSize, dataRoot, "DUNGEON.DAT")) return 1;
    }
    return 0;
}

static int is_floor_object_type(int type)
{
    return type == THING_TYPE_WEAPON ||
           type == THING_TYPE_ARMOUR ||
           type == THING_TYPE_SCROLL ||
           type == THING_TYPE_POTION ||
           type == THING_TYPE_CONTAINER ||
           type == THING_TYPE_JUNK ||
           type == THING_TYPE_PROJECTILE ||
           type == THING_TYPE_EXPLOSION;
}

static unsigned short next_thing(const struct DungeonThings_Compat* things, unsigned short thing)
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

static const struct ExpectedFloorChain* expected_chain_at(int x, int y)
{
    size_t i;
    for (i = 0; i < sizeof(s_expectedFloorChains) / sizeof(s_expectedFloorChains[0]); ++i) {
        if (s_expectedFloorChains[i].x == x && s_expectedFloorChains[i].y == y) {
            return &s_expectedFloorChains[i];
        }
    }
    return NULL;
}

static int compare_item_chain(
    const struct ExpectedFloorChain* expected,
    const unsigned short* itemChain,
    int itemCount,
    int x,
    int y)
{
    int i;
    if (!expected) {
        if (itemCount == 0) {
            return 1;
        }
        fprintf(stderr, "FAIL unexpected Hall floor item chain at (%d,%d):", x, y);
        for (i = 0; i < itemCount; ++i) {
            fprintf(stderr, " 0x%04X", itemChain[i]);
        }
        fprintf(stderr, "\n");
        return 0;
    }
    if (itemCount != expected->count) {
        fprintf(stderr, "FAIL Hall floor chain length at (%d,%d) got=%d want=%d\n",
            x, y, itemCount, expected->count);
        return 0;
    }
    for (i = 0; i < itemCount; ++i) {
        if (itemChain[i] != expected->things[i]) {
            fprintf(stderr,
                "FAIL Hall floor thing at (%d,%d)[%d] got=0x%04X type=%u index=%u cell=%u want=0x%04X\n",
                x, y, i, itemChain[i], THING_GET_TYPE(itemChain[i]),
                THING_GET_INDEX(itemChain[i]), THING_GET_CELL(itemChain[i]),
                expected->things[i]);
            return 0;
        }
    }
    return 1;
}

int main(int argc, char** argv)
{
    char dungeonPath[512];
    struct DungeonDatState_Compat dungeon;
    struct DungeonThings_Compat things;
    const struct DungeonMapDesc_Compat* map;
    int ok = 1;
    int flaggedCount = 0;
    int matchedExpected = 0;
    int totalFloorObjects = 0;
    int floorContainers = 0;
    int floorProjectiles = 0;
    int floorExplosions = 0;

    printf("probe=dm1_v1_hoc_floor_things_source_lock\n");
    printf("sourceEvidence=ReDMCSB LOADSAVE.C:1970-1984; DUNGEON.C:F0160 lines 1715-1727; DUNGEON.C:F0161 lines 1730-1746; DUNVIEW.C:F0115 lines 4477-4503\n");

    if (!resolve_dungeon_path(argc, argv, dungeonPath, sizeof(dungeonPath))) {
        printf("SKIP no DM1 DUNGEON.DAT found\n");
        return 77;
    }

    if (!F0500_DUNGEON_LoadDatHeader_Compat(dungeonPath, &dungeon)) {
        fprintf(stderr, "FAIL could not load dungeon header: %s\n", dungeonPath);
        return 1;
    }
    if (!F0502_DUNGEON_LoadTileData_Compat(dungeonPath, &dungeon)) {
        fprintf(stderr, "FAIL could not load dungeon tiles: %s\n", dungeonPath);
        F0500_DUNGEON_FreeDatHeader_Compat(&dungeon);
        return 1;
    }
    if (!F0504_DUNGEON_LoadThingData_Compat(dungeonPath, &dungeon, &things)) {
        fprintf(stderr, "FAIL could not load dungeon things: %s\n", dungeonPath);
        F0502_DUNGEON_FreeTileData_Compat(&dungeon);
        F0500_DUNGEON_FreeDatHeader_Compat(&dungeon);
        return 1;
    }

    if (dungeon.header.mapCount < 1 || !dungeon.tiles || !dungeon.tiles[0].squareData) {
        fprintf(stderr, "FAIL DM1 map0 Hall of Champions unavailable\n");
        ok = 0;
        goto cleanup;
    }
    map = &dungeon.maps[0];
    if (map->width != 18 || map->height != 19) {
        fprintf(stderr, "FAIL map0 Hall dimensions got=%ux%u want=18x19\n",
            (unsigned)map->width, (unsigned)map->height);
        ok = 0;
        goto cleanup;
    }

    for (int x = 0; x < (int)map->width; ++x) {
        for (int y = 0; y < (int)map->height; ++y) {
            int squareOffset = x * (int)map->height + y;
            unsigned char square = dungeon.tiles[0].squareData[squareOffset];
            int hasThingList = (square & DUNGEON_SQUARE_MASK_THING_LIST) != 0;
            int sftIndex = F0510_DUNGEON_GetSquareFirstThingIndex_Compat(&dungeon, 0, x, y);
            unsigned short thing = F0511_DUNGEON_GetSquareFirstThing_Compat(&dungeon, &things, 0, x, y);

            if (!hasThingList) {
                if (sftIndex != -1 || thing != THING_ENDOFLIST) {
                    fprintf(stderr,
                        "FAIL unflagged Hall square (%d,%d) returned sftIndex=%d thing=0x%04X\n",
                        x, y, sftIndex, thing);
                    ok = 0;
                }
                continue;
            }

            if (sftIndex != flaggedCount) {
                fprintf(stderr,
                    "FAIL compact Hall SFT index at (%d,%d) got=%d want=%d\n",
                    x, y, sftIndex, flaggedCount);
                ok = 0;
            }
            ++flaggedCount;

            if (((square & DUNGEON_SQUARE_MASK_TYPE) >> 5) != DUNGEON_ELEMENT_WALL) {
                unsigned short itemChain[8];
                int itemCount = 0;
                const struct ExpectedFloorChain* expected = expected_chain_at(x, y);
                int safety = 0;

                while (thing != THING_ENDOFLIST && thing != THING_NONE && safety < 64) {
                    int type = (int)THING_GET_TYPE(thing);
                    if (is_floor_object_type(type)) {
                        if (itemCount < (int)(sizeof(itemChain) / sizeof(itemChain[0]))) {
                            itemChain[itemCount] = thing;
                        }
                        ++itemCount;
                        ++totalFloorObjects;
                        if (type == THING_TYPE_CONTAINER) ++floorContainers;
                        if (type == THING_TYPE_PROJECTILE) ++floorProjectiles;
                        if (type == THING_TYPE_EXPLOSION) ++floorExplosions;
                    }
                    thing = next_thing(&things, thing);
                    ++safety;
                }

                if (itemCount > (int)(sizeof(itemChain) / sizeof(itemChain[0]))) {
                    fprintf(stderr, "FAIL Hall floor item chain overflow at (%d,%d)\n", x, y);
                    ok = 0;
                } else if (!compare_item_chain(expected, itemChain, itemCount, x, y)) {
                    ok = 0;
                } else if (expected) {
                    ++matchedExpected;
                }
            }
        }
    }

    if (flaggedCount != 70) {
        fprintf(stderr, "FAIL Hall compact thing-list square count got=%d want=70\n", flaggedCount);
        ok = 0;
    }
    if (matchedExpected != (int)(sizeof(s_expectedFloorChains) / sizeof(s_expectedFloorChains[0]))) {
        fprintf(stderr, "FAIL matched Hall expected floor chains got=%d want=%lu\n",
            matchedExpected, (unsigned long)(sizeof(s_expectedFloorChains) / sizeof(s_expectedFloorChains[0])));
        ok = 0;
    }
    if (totalFloorObjects != 8) {
        fprintf(stderr, "FAIL Hall floor visible object count got=%d want=8\n", totalFloorObjects);
        ok = 0;
    }
    if (floorContainers != 0 || floorProjectiles != 0 || floorExplosions != 0) {
        fprintf(stderr,
            "FAIL Hall floor illegal object classes containers=%d projectiles=%d explosions=%d want=0/0/0\n",
            floorContainers, floorProjectiles, floorExplosions);
        ok = 0;
    }

cleanup:
    F0504_DUNGEON_FreeThingData_Compat(&things);
    F0502_DUNGEON_FreeTileData_Compat(&dungeon);
    F0500_DUNGEON_FreeDatHeader_Compat(&dungeon);

    if (!ok) {
        return 1;
    }

    printf("PASS Hall of Champions floor things: 70 compact lists, 7 expected floor chains, 8 visible objects, no containers/projectiles/explosions\n");
    return 0;
}

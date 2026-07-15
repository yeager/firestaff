#include <stdio.h>
#include <string.h>

#include "memory_dungeon_dat_pc34_compat.h"

#define MAKE_THING(type, index, cell) \
    ((unsigned short)((((cell) & 0x03) << 14) | (((type) & 0x0F) << 10) | ((index) & 0x03FF)))

static int expect_int(const char* label, int got, int want)
{
    if (got != want) {
        fprintf(stderr, "FAIL %s got=%d want=%d\n", label, got, want);
        return 0;
    }
    return 1;
}

static int expect_thing(const char* label, unsigned short got, unsigned short want)
{
    if (got != want) {
        fprintf(stderr, "FAIL %s got=0x%04X want=0x%04X\n", label, got, want);
        return 0;
    }
    return 1;
}

static unsigned char corridor_square(int hasThingList)
{
    return (unsigned char)((DUNGEON_ELEMENT_CORRIDOR << 5) |
        (hasThingList ? DUNGEON_SQUARE_MASK_THING_LIST : 0));
}

int main(void)
{
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat maps[2];
    struct DungeonMapTiles_Compat tiles[2];
    struct DungeonThings_Compat things;
    unsigned char map0Squares[9];
    unsigned char map1Squares[4];
    unsigned short squareFirstThings[6];
    unsigned short columnSftBases[5] = { 0, 2, 2, 3, 3 };
    unsigned char rawDoor[4];
    unsigned char rawText[4];
    unsigned char rawSensor[8];
    unsigned char rawGroup[16];
    unsigned char rawWeapon[4];
    unsigned short staticDoor;
    unsigned short staticText;
    unsigned short staticSensor;
    unsigned short firstGroup;
    unsigned short firstObject;
    unsigned short linkedWeapon;
    int ok = 1;

    printf("probe=dm1_v1_dun07_f0510_square_first_thing_index_pc34_compat\n");
    printf("sourceEvidence=ReDMCSB DUNGEON.C:F0160 lines 1715-1727; DUNGEON.C:F0161 lines 1730-1746; DUNGEON.C:F0162 lines 1752-1766\n");

    memset(&dungeon, 0, sizeof(dungeon));
    memset(maps, 0, sizeof(maps));
    memset(tiles, 0, sizeof(tiles));
    memset(&things, 0, sizeof(things));
    memset(map0Squares, 0, sizeof(map0Squares));
    memset(map1Squares, 0, sizeof(map1Squares));
    memset(rawDoor, 0, sizeof(rawDoor));
    memset(rawText, 0, sizeof(rawText));
    memset(rawSensor, 0, sizeof(rawSensor));
    memset(rawGroup, 0, sizeof(rawGroup));
    memset(rawWeapon, 0, sizeof(rawWeapon));

    dungeon.header.mapCount = 2;
    dungeon.maps = maps;
    dungeon.tiles = tiles;
    dungeon.loaded = 1;
    dungeon.tilesLoaded = 1;
    dungeon.columnsCumulativeSquareFirstThingCount = columnSftBases;
    dungeon.dungeonColumnCount = (int)(sizeof(columnSftBases) /
                                       sizeof(columnSftBases[0]));

    maps[0].width = 3;
    maps[0].height = 3;
    maps[1].width = 2;
    maps[1].height = 2;
    tiles[0].squareData = map0Squares;
    tiles[0].squareCount = 9;
    tiles[1].squareData = map1Squares;
    tiles[1].squareCount = 4;

    for (int i = 0; i < 9; ++i) {
        map0Squares[i] = corridor_square(0);
    }
    for (int i = 0; i < 4; ++i) {
        map1Squares[i] = corridor_square(0);
    }

    /* Column-major square storage: index = x * height + y.
     * ReDMCSB F0160 counts only squares with MASK0x0010_THING_LIST_PRESENT,
     * so these five flagged squares map compactly to SFT[0..4]. */
    map0Squares[0 * 3 + 0] = corridor_square(1);
    map0Squares[0 * 3 + 2] = corridor_square(1);
    map0Squares[2 * 3 + 1] = corridor_square(1);
    map1Squares[1 * 2 + 0] = corridor_square(1);
    map1Squares[1 * 2 + 1] = corridor_square(1);

    squareFirstThings[0] = MAKE_THING(THING_TYPE_JUNK, 10, 0);
    squareFirstThings[1] = MAKE_THING(THING_TYPE_SCROLL, 11, 1);
    squareFirstThings[2] = MAKE_THING(THING_TYPE_CONTAINER, 12, 2);
    squareFirstThings[3] = MAKE_THING(THING_TYPE_WEAPON, 13, 3);
    squareFirstThings[4] = MAKE_THING(THING_TYPE_ARMOUR, 14, 0);
    things.loaded = 1;
    things.squareFirstThings = squareFirstThings;
    squareFirstThings[5] = THING_NONE;
    things.squareFirstThingCount = 6;

    staticDoor = MAKE_THING(THING_TYPE_DOOR, 0, 0);
    staticText = MAKE_THING(THING_TYPE_TEXTSTRING, 0, 0);
    staticSensor = MAKE_THING(THING_TYPE_SENSOR, 0, 0);
    firstGroup = MAKE_THING(THING_TYPE_GROUP, 0, 1);
    firstObject = MAKE_THING(THING_TYPE_WEAPON, 0, 2);
    rawDoor[0] = (unsigned char)(staticText & 0xffu);
    rawDoor[1] = (unsigned char)(staticText >> 8);
    rawText[0] = (unsigned char)(staticSensor & 0xffu);
    rawText[1] = (unsigned char)(staticSensor >> 8);
    rawSensor[0] = (unsigned char)(firstGroup & 0xffu);
    rawSensor[1] = (unsigned char)(firstGroup >> 8);
    rawGroup[0] = (unsigned char)(THING_ENDOFLIST & 0xffu);
    rawGroup[1] = (unsigned char)(THING_ENDOFLIST >> 8);
    things.rawThingData[THING_TYPE_DOOR] = rawDoor;
    things.rawThingData[THING_TYPE_TEXTSTRING] = rawText;
    things.rawThingData[THING_TYPE_SENSOR] = rawSensor;
    things.rawThingData[THING_TYPE_GROUP] = rawGroup;
    things.rawThingData[THING_TYPE_WEAPON] = rawWeapon;
    things.thingCounts[THING_TYPE_DOOR] = 1;
    things.thingCounts[THING_TYPE_TEXTSTRING] = 1;
    things.thingCounts[THING_TYPE_SENSOR] = 1;
    things.thingCounts[THING_TYPE_GROUP] = 1;
    things.thingCounts[THING_TYPE_WEAPON] = 1;

    ok &= expect_int("map0 first flagged square index",
        F0510_DUNGEON_GetSquareFirstThingIndex_Compat(&dungeon, 0, 0, 0), 0);
    ok &= expect_int("map0 same-column second flagged square index",
        F0510_DUNGEON_GetSquareFirstThingIndex_Compat(&dungeon, 0, 0, 2), 1);
    ok &= expect_int("map0 later-column flagged square index",
        F0510_DUNGEON_GetSquareFirstThingIndex_Compat(&dungeon, 0, 2, 1), 2);
    ok &= expect_int("map1 compact offset includes prior map flags",
        F0510_DUNGEON_GetSquareFirstThingIndex_Compat(&dungeon, 1, 1, 0), 3);
    ok &= expect_int("map1 second compact entry",
        F0510_DUNGEON_GetSquareFirstThingIndex_Compat(&dungeon, 1, 1, 1), 4);

    ok &= expect_int("unflagged square has no SFT index",
        F0510_DUNGEON_GetSquareFirstThingIndex_Compat(&dungeon, 0, 1, 1), -1);
    ok &= expect_thing("unflagged square returns end-of-list",
        F0511_DUNGEON_GetSquareFirstThing_Compat(&dungeon, &things, 0, 1, 1),
        THING_ENDOFLIST);

    ok &= expect_thing("flagged square returns compact SFT[1]",
        F0511_DUNGEON_GetSquareFirstThing_Compat(&dungeon, &things, 0, 0, 2),
        squareFirstThings[1]);
    ok &= expect_thing("later map flagged square returns compact SFT[4]",
        F0511_DUNGEON_GetSquareFirstThing_Compat(&dungeon, &things, 1, 1, 1),
        squareFirstThings[4]);

    squareFirstThings[0] = staticDoor;
    ok &= expect_thing("F0162 skips static door/text/sensor records",
        F0513_DUNGEON_GetSquareFirstObject_Compat(&dungeon, &things, 0, 0, 0),
        firstGroup);
    squareFirstThings[0] = firstObject;
    ok &= expect_thing("F0162 retains the first live object",
        F0513_DUNGEON_GetSquareFirstObject_Compat(&dungeon, &things, 0, 0, 0),
        firstObject);
    squareFirstThings[0] = staticDoor;
    rawSensor[0] = (unsigned char)(THING_ENDOFLIST & 0xffu);
    rawSensor[1] = (unsigned char)(THING_ENDOFLIST >> 8);
    ok &= expect_thing("F0162 does not fabricate an object after static records",
        F0513_DUNGEON_GetSquareFirstObject_Compat(&dungeon, &things, 0, 0, 0),
        THING_ENDOFLIST);

    /* F0163/F0164 preserve the compact G0280 bases and the raw Generic.Next
     * chain. The spare original SFT slot is the only admissible new head. */
    linkedWeapon = MAKE_THING(THING_TYPE_WEAPON, 0, 3);
    squareFirstThings[0] = staticDoor;
    rawSensor[0] = (unsigned char)(THING_ENDOFLIST & 0xffu);
    rawSensor[1] = (unsigned char)(THING_ENDOFLIST >> 8);
    ok &= expect_int("F0163 inserts an original-backed empty square head",
        F0514_DUNGEON_LinkThingToList_Compat(&dungeon, &things, linkedWeapon,
            THING_ENDOFLIST, 0, 1, 1), 1);
    ok &= expect_int("F0163 increments later compact column base", columnSftBases[2], 3);
    ok &= expect_thing("F0163 stores new SFT head", squareFirstThings[2], linkedWeapon);
    ok &= expect_thing("F0163 terminates new raw thing", F0512_DUNGEON_GetThingNext_Compat(&things, linkedWeapon), THING_ENDOFLIST);
    ok &= expect_int("F0163 marks square list present",
        (map0Squares[1 * 3 + 1] & DUNGEON_SQUARE_MASK_THING_LIST) != 0, 1);
    ok &= expect_int("F0164 removes final compact square head",
        F0515_DUNGEON_UnlinkThingFromList_Compat(&dungeon, &things, linkedWeapon,
            THING_ENDOFLIST, 0, 1, 1), 1);
    ok &= expect_int("F0164 restores later compact column base", columnSftBases[2], 2);
    ok &= expect_thing("F0164 restores SFT tail sentinel", squareFirstThings[5], THING_NONE);
    ok &= expect_int("F0164 clears square list present",
        (map0Squares[1 * 3 + 1] & DUNGEON_SQUARE_MASK_THING_LIST) != 0, 0);

    rawGroup[0] = (unsigned char)(THING_ENDOFLIST & 0xffu);
    rawGroup[1] = (unsigned char)(THING_ENDOFLIST >> 8);
    ok &= expect_int("F0163 appends after an existing raw tail",
        F0514_DUNGEON_LinkThingToList_Compat(&dungeon, &things, linkedWeapon,
            firstGroup, 0, -1, 0), 1);
    ok &= expect_thing("F0163 writes predecessor raw next", F0512_DUNGEON_GetThingNext_Compat(&things, firstGroup), linkedWeapon);
    ok &= expect_int("F0164 removes a non-square tail",
        F0515_DUNGEON_UnlinkThingFromList_Compat(&dungeon, &things, linkedWeapon,
            firstGroup, 0, -1, 0), 1);
    ok &= expect_thing("F0164 restores predecessor raw next", F0512_DUNGEON_GetThingNext_Compat(&things, firstGroup), THING_ENDOFLIST);

    if (!ok) {
        return 1;
    }
    printf("PASS compact SquareFirstThings indexing matches ReDMCSB F0160/F0161\n");
    return 0;
}

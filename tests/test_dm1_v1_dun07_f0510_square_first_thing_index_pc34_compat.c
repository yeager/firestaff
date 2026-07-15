#include <stdio.h>
#include <string.h>

#include "memory_dungeon_dat_pc34_compat.h"
#include "dm1_v1_dungeon_thing_data_pc34_compat.h"
#include "dm1_v1_dungeon_weapon_info_pc34_compat.h"

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

static unsigned short discard_thing_for_f0166(void* context, unsigned short thingType)
{
    const unsigned short* replacement = (const unsigned short*)context;
    return thingType == THING_TYPE_JUNK ? *replacement : THING_NONE;
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
    unsigned char rawJunk[20];
    struct DungeonJunk_Compat decodedJunks[5];
    struct DungeonThings_Compat objectThings;
    unsigned char objectWeapons[8];
    unsigned char objectJunks[16];
    struct DungeonWeapon_Compat decodedObjectWeapons[2];
    struct DungeonJunk_Compat decodedObjectJunks[4];
    unsigned short staticDoor;
    unsigned short staticText;
    unsigned short staticSensor;
    unsigned short firstGroup;
    unsigned short firstObject;
    unsigned short linkedWeapon;
    unsigned short discardedJunk;
    unsigned short allocatedJunk;
    unsigned short generatedThing;
    const unsigned char *thingData;
    DM1_WeaponInfo weaponInfo;
    int ok = 1;

    printf("probe=dm1_v1_dun07_f0510_square_first_thing_index_pc34_compat\n");
    printf("sourceEvidence=ReDMCSB DUNGEON.C:F0160-F0167 lines 1715-2200\n");

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
    memset(rawJunk, 0, sizeof(rawJunk));
    memset(decodedJunks, 0, sizeof(decodedJunks));
    memset(&objectThings, 0, sizeof(objectThings));
    memset(objectWeapons, 0, sizeof(objectWeapons));
    memset(objectJunks, 0, sizeof(objectJunks));
    memset(decodedObjectWeapons, 0, sizeof(decodedObjectWeapons));
    memset(decodedObjectJunks, 0, sizeof(decodedObjectJunks));

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
    things.rawThingData[THING_TYPE_JUNK] = rawJunk;
    things.thingCounts[THING_TYPE_DOOR] = 1;
    things.thingCounts[THING_TYPE_TEXTSTRING] = 1;
    things.thingCounts[THING_TYPE_SENSOR] = 1;
    things.thingCounts[THING_TYPE_GROUP] = 1;
    things.thingCounts[THING_TYPE_WEAPON] = 1;
    things.thingCounts[THING_TYPE_JUNK] = 5;
    things.junks = decodedJunks;
    things.junkCount = 5;

    /* F0158 is F0156(weaponThing) followed by the original G0238 table.
     * The decoded weapon mirror is deliberately not needed for this lookup. */
    rawWeapon[2] = 7; /* PC3.4 WEAPON.Type: THE FIRESTAFF */
    ok &= expect_int("F0158 reads raw WEAPON.Type through F0156",
        dm1_v1_dungeon_get_weapon_info_pc34(
            &things, MAKE_THING(THING_TYPE_WEAPON, 0, 3), &weaponInfo), 1);
    ok &= expect_int("F0158 selects original G0238 Firestaff class",
        weaponInfo.weaponClass, 255);
    rawWeapon[2] = 127;
    ok &= expect_int("F0158 rejects a raw weapon type outside G0238",
        dm1_v1_dungeon_get_weapon_info_pc34(
            &things, MAKE_THING(THING_TYPE_WEAPON, 0, 0), &weaponInfo), 0);
    ok &= expect_int("F0158 invalid type leaves no class fallback",
        weaponInfo.weaponClass, -1);
    rawWeapon[2] = 7;

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

    /* F0156 uses the Thing reference's type/index only: the cell bits do
     * not participate in its G0284 base + G0235 stride calculation. */
    rawJunk[16] = 0xA5u;
    rawJunk[17] = 0x5Au;
    thingData = dm1_v1_dungeon_get_thing_data_pc34(
        &things, MAKE_THING(THING_TYPE_JUNK, 4, 3));
    ok &= expect_int("F0156 returns the exact typed raw record",
        thingData == rawJunk + 16, 1);
    ok &= expect_int("F0156 preserves raw record contents",
        thingData && thingData[1] == 0x5Au, 1);
    ok &= expect_int("F0156 rejects absent source records",
        dm1_v1_dungeon_get_thing_data_pc34(&things,
            MAKE_THING(THING_TYPE_WEAPON, 1, 0)) == NULL, 1);
    ok &= expect_int("F0156 rejects end-of-list sentinel",
        dm1_v1_dungeon_get_thing_data_pc34(&things, THING_ENDOFLIST) == NULL,
        1);

    squareFirstThings[0] = staticDoor;
    ok &= expect_thing("F0175 follows raw static chain to first C04 group",
        dm1_v1_group_get_thing_f0175_pc34(&dungeon, &things, 0, 0, 0),
        firstGroup);
    rawGroup[4] = 0;
    rawGroup[14] = 0;
    ok &= expect_int("F0177 resolves raw adjacent C04 melee target",
        dm1_v1_group_get_melee_target_ordinal_f0177_pc34(
            &dungeon, &things, 0, 0, 0, 0, 1, 0u, 0x03u, 0u), 1);
    ok &= expect_int("F0177 rejects a diagonal non-melee target",
        dm1_v1_group_get_melee_target_ordinal_f0177_pc34(
            &dungeon, &things, 0, 0, 0, 1, 1, 0u, 0x03u, 0u), 0);
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
    ok &= expect_thing("F0175 does not invent a group after static records",
        dm1_v1_group_get_thing_f0175_pc34(&dungeon, &things, 0, 0, 0),
        THING_ENDOFLIST);
    ok &= expect_int("F0177 does not invent a target without C04",
        dm1_v1_group_get_melee_target_ordinal_f0177_pc34(
            &dungeon, &things, 0, 0, 0, 0, 1, 0u, 0x03u, 0u), 0);
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
    rawGroup[4] = 0; /* Giant Scorpion: full-square, G0243 size 2. */
    rawGroup[14] = (unsigned char)(2u << 5); /* Three creatures, slots 0..2. */
    rawGroup[15] = 0;
    ok &= expect_int("F0176 resolves the highest matching raw creature slot",
        dm1_v1_group_get_creature_ordinal_in_cell_f0176_pc34(
            &things, firstGroup, 0x24u, 0u, 2u), 3);
    rawGroup[4] = 4; /* G0243 size 1: half-square. */
    rawGroup[14] = 0;
    ok &= expect_int("F0176 retains half-square cell occupancy",
        dm1_v1_group_get_creature_ordinal_in_cell_f0176_pc34(
            &things, firstGroup, 0x03u, 0u, 3u), 1);
    ok &= expect_int("F0176 retains centered-creature all-cell ordinal",
        dm1_v1_group_get_creature_ordinal_in_cell_f0176_pc34(
            &things, firstGroup, 0xffu, 3u, 2u), 1);
    rawGroup[4] = 27;
    ok &= expect_int("F0176 rejects a group outside G0243",
        dm1_v1_group_get_creature_ordinal_in_cell_f0176_pc34(
            &things, firstGroup, 0x00u, 0u, 0u), 0);
    ok &= expect_int("F0176 rejects a non-group Thing",
        dm1_v1_group_get_creature_ordinal_in_cell_f0176_pc34(
            &things, firstObject, 0x00u, 0u, 0u), 0);
    rawGroup[4] = 0;
    ok &= expect_int("F0163 appends after an existing raw tail",
        F0514_DUNGEON_LinkThingToList_Compat(&dungeon, &things, linkedWeapon,
            firstGroup, 0, -1, 0), 1);
    ok &= expect_thing("F0163 writes predecessor raw next", F0512_DUNGEON_GetThingNext_Compat(&things, firstGroup), linkedWeapon);
    ok &= expect_int("F0164 removes a non-square tail",
        F0515_DUNGEON_UnlinkThingFromList_Compat(&dungeon, &things, linkedWeapon,
            firstGroup, 0, -1, 0), 1);
    ok &= expect_thing("F0164 restores predecessor raw next", F0512_DUNGEON_GetThingNext_Compat(&things, firstGroup), THING_ENDOFLIST);

    /* F0166 scans only the non-reserved JUNK range. A bones request may use
     * the final three records, and a full normal range uses the real F0165
     * discard callback rather than allocating a synthetic slot. */
    rawJunk[0] = (unsigned char)(THING_ENDOFLIST & 0xffu);
    rawJunk[1] = (unsigned char)(THING_ENDOFLIST >> 8);
    rawJunk[4] = (unsigned char)(THING_ENDOFLIST & 0xffu);
    rawJunk[5] = (unsigned char)(THING_ENDOFLIST >> 8);
    rawJunk[8] = (unsigned char)(THING_NONE & 0xffu);
    rawJunk[9] = (unsigned char)(THING_NONE >> 8);
    decodedJunks[2].next = THING_NONE;
    allocatedJunk = F0516_DUNGEON_GetUnusedThing_Compat(&things,
        THING_TYPE_JUNK, NULL, NULL);
    ok &= expect_thing("F0166 reserves normal JUNK bones slots", allocatedJunk, THING_NONE);
    allocatedJunk = F0516_DUNGEON_GetUnusedThing_Compat(&things,
        (unsigned short)(0x8000u | THING_TYPE_JUNK), NULL, NULL);
    ok &= expect_thing("F0166 bones request uses reserved JUNK slot", allocatedJunk,
        MAKE_THING(THING_TYPE_JUNK, 2, 0));
    ok &= expect_thing("F0166 clears and terminates bones raw record",
        F0512_DUNGEON_GetThingNext_Compat(&things, allocatedJunk), THING_ENDOFLIST);
    discardedJunk = MAKE_THING(THING_TYPE_JUNK, 0, 0);
    allocatedJunk = F0516_DUNGEON_GetUnusedThing_Compat(&things,
        THING_TYPE_JUNK, discard_thing_for_f0166, &discardedJunk);
    ok &= expect_thing("F0166 obtains full normal pool from F0165 callback", allocatedJunk, discardedJunk);

    objectWeapons[0] = (unsigned char)(THING_NONE & 0xffu);
    objectWeapons[1] = (unsigned char)(THING_NONE >> 8);
    objectJunks[0] = (unsigned char)(THING_NONE & 0xffu);
    objectJunks[1] = (unsigned char)(THING_NONE >> 8);
    objectThings.loaded = 1;
    objectThings.rawThingData[THING_TYPE_WEAPON] = objectWeapons;
    objectThings.rawThingData[THING_TYPE_JUNK] = objectJunks;
    objectThings.thingCounts[THING_TYPE_WEAPON] = 2;
    objectThings.thingCounts[THING_TYPE_JUNK] = 4;
    objectThings.weapons = decodedObjectWeapons;
    objectThings.weaponCount = 2;
    objectThings.junks = decodedObjectJunks;
    objectThings.junkCount = 4;
    generatedThing = F0517_DUNGEON_GetObjectForProjectileLauncherOrObjectGenerator_Compat(
        &objectThings, 51, NULL, NULL);
    ok &= expect_thing("F0167 creates only an arrow from C051", generatedThing,
        MAKE_THING(THING_TYPE_WEAPON, 0, 0));
    ok &= expect_int("F0167 writes C27 arrow type", decodedObjectWeapons[0].type, 27);
    generatedThing = F0517_DUNGEON_GetObjectForProjectileLauncherOrObjectGenerator_Compat(
        &objectThings, 128, NULL, NULL);
    ok &= expect_thing("F0167 creates only a boulder from C128", generatedThing,
        MAKE_THING(THING_TYPE_JUNK, 0, 0));
    ok &= expect_int("F0167 writes C25 boulder type", decodedObjectJunks[0].type, 25);
    ok &= expect_thing("F0167 rejects a non-source icon",
        F0517_DUNGEON_GetObjectForProjectileLauncherOrObjectGenerator_Compat(
            &objectThings, 127, NULL, NULL), THING_NONE);

    if (!ok) {
        return 1;
    }
    printf("PASS compact SFT and object allocation match ReDMCSB F0160-F0167\n");
    return 0;
}

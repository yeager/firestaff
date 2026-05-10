#include <stdio.h>
#include <string.h>

#include "dm1_v1_collision_door_pc34_compat.h"

static int expect_int(const char* label, int got, int want)
{
    if (got != want) {
        fprintf(stderr, "FAIL %s got=%d want=%d\n", label, got, want);
        return 0;
    }
    return 1;
}

static int expect_contains(const char* label, const char* haystack, const char* needle)
{
    if (!haystack || !needle || !strstr(haystack, needle)) {
        fprintf(stderr, "FAIL %s missing substring: %s\n", label, needle ? needle : "(null)");
        return 0;
    }
    return 1;
}

static unsigned char square_type(int elementType, int attrs)
{
    return (unsigned char)((elementType << 5) | (attrs & DUNGEON_SQUARE_MASK_ATTRIBS));
}

static void setup_fixture(struct DungeonDatState_Compat* dungeon,
    struct DungeonMapDesc_Compat* map,
    struct DungeonMapTiles_Compat* tiles,
    unsigned char* squares,
    struct DungeonThings_Compat* things,
    struct DungeonDoor_Compat* doors,
    unsigned short* squareFirstThings,
    struct PartyState_Compat* party,
    int doorButton)
{
    int x, y;
    memset(dungeon, 0, sizeof(*dungeon));
    memset(map, 0, sizeof(*map));
    memset(tiles, 0, sizeof(*tiles));
    memset(things, 0, sizeof(*things));
    memset(doors, 0, sizeof(*doors));
    memset(squareFirstThings, 0, sizeof(unsigned short));
    memset(party, 0, sizeof(*party));

    map->width = 3;
    map->height = 3;
    tiles->squareData = squares;
    tiles->squareCount = 9;
    dungeon->header.mapCount = 1;
    dungeon->maps = map;
    dungeon->tiles = tiles;
    dungeon->loaded = 1;
    dungeon->tilesLoaded = 1;
    for (x = 0; x < 3; ++x) {
        for (y = 0; y < 3; ++y) {
            squares[x * 3 + y] = square_type(DUNGEON_ELEMENT_CORRIDOR, 0);
        }
    }

    /* Party at (1,2) faces north.  Front cell (1,1) is the door.
     * The square thing-list bit and SFT[0] model F0161/F0157 returning
     * a DOOR thing; the decoded DOOR->Button flag is the parity gate. */
    squares[1 * 3 + 1] = square_type(DUNGEON_ELEMENT_DOOR, 0x10 | DM1_DOOR_STATE_CLOSED);
    squareFirstThings[0] = 0; /* type 0 / door, index 0 */
    doors[0].next = THING_ENDOFLIST;
    doors[0].button = (unsigned char)doorButton;
    things->loaded = 1;
    things->squareFirstThings = squareFirstThings;
    things->squareFirstThingCount = 1;
    things->doors = doors;
    things->doorCount = 1;

    party->mapIndex = 0;
    party->mapX = 1;
    party->mapY = 2;
    party->direction = DIR_NORTH;
    party->championCount = 1;
}

int main(void)
{
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat map;
    struct DungeonMapTiles_Compat tiles;
    unsigned char squares[9];
    struct DungeonThings_Compat things;
    struct DungeonDoor_Compat doors[1];
    unsigned short squareFirstThings[1];
    struct PartyState_Compat party;
    struct Dm1V1DoorInteractionResult result;
    int ok = 1;

    printf("probe=dm1_v1_door_button_click_pc34_compat\n");
    ok &= expect_contains("source evidence door button gate",
        DM1_V1_CollisionDoor_SourceEvidence(),
        "CLIKVIEW.C:F0377:365-385");

    setup_fixture(&dungeon, &map, &tiles, squares, &things, doors, squareFirstThings, &party, 0);
    ok &= expect_int("door without button ignored",
        DM1_V1_Door_ProcessClick(&dungeon, &things, &party, &result), 0);
    ok &= expect_int("door without button code", result.code, DM1_DOOR_CLICK_NO_BUTTON);
    ok &= expect_int("door without button does not claim gate", result.doorHasButton, 0);
    ok &= expect_int("door without button no target state", result.newDoorState, -1);

    setup_fixture(&dungeon, &map, &tiles, squares, &things, doors, squareFirstThings, &party, 1);
    ok &= expect_int("door with button accepted",
        DM1_V1_Door_ProcessClick(&dungeon, &things, &party, &result), 1);
    ok &= expect_int("door with button code", result.code, DM1_DOOR_CLICK_TOGGLED_OPEN);
    ok &= expect_int("door with button gate set", result.doorHasButton, 1);
    ok &= expect_int("door with button previous state", result.previousDoorState, DM1_DOOR_STATE_CLOSED);
    ok &= expect_int("door with button target state", result.newDoorState, DM1_DOOR_STATE_OPEN);

    /* A bounded legacy fixture may omit decoded thing data; preserve the old
     * pure door-state toggle resolution in that specific no-things mode. */
    ok &= expect_int("no things legacy path remains accepted",
        DM1_V1_Door_ProcessClick(&dungeon, NULL, &party, &result), 1);
    ok &= expect_int("no things target state", result.newDoorState, DM1_DOOR_STATE_OPEN);

    if (!ok) return 1;
    printf("ok: DM1 V1 door button click gate matches ReDMCSB DOOR->Button path\n");
    return 0;
}

#include "memory_movement_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void expect(const char *label, int actual, int expected)
{
    if (actual != expected) {
        fprintf(stderr, "FAIL: %s: got %d expected %d\n", label, actual, expected);
        ++failures;
    }
}

int main(void)
{
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat map;
    struct DungeonMapTiles_Compat tiles;
    unsigned char square = 0;

    memset(&dungeon, 0, sizeof(dungeon));
    memset(&map, 0, sizeof(map));
    memset(&tiles, 0, sizeof(tiles));
    dungeon.header.mapCount = 1;
    dungeon.maps = &map;
    dungeon.tiles = &tiles;
    dungeon.tilesLoaded = 1;
    map.width = 1;
    map.height = 1;
    tiles.squareData = &square;

    square = (unsigned char)((DUNGEON_ELEMENT_PIT << 5) | 0x08);
    expect("ground creature rejects open pit",
           F0708_MOVEMENT_IsSquarePassableForCreature_Compat(
               &dungeon, 0, 0, 0, 0x0000, 0, 1), 0);
    expect("levitating creature crosses open pit",
           F0708_MOVEMENT_IsSquarePassableForCreature_Compat(
               &dungeon, 0, 0, 0, 0x0020, 0, 1), 1);

    square = (unsigned char)((DUNGEON_ELEMENT_DOOR << 5) | 0x02);
    expect("short creature rejects vertical half door",
           F0708_MOVEMENT_IsSquarePassableForCreature_Compat(
               &dungeon, 0, 0, 0, 0x0080, 1, 1), 0);
    expect("height-two creature crosses vertical half door",
           F0708_MOVEMENT_IsSquarePassableForCreature_Compat(
               &dungeon, 0, 0, 0, 0x0100, 1, 1), 1);
    expect("horizontal half door blocks regardless of height",
           F0708_MOVEMENT_IsSquarePassableForCreature_Compat(
               &dungeon, 0, 0, 0, 0x0100, 0, 1), 0);
    expect("non-material creature ignores closed door",
           F0708_MOVEMENT_IsSquarePassableForCreature_Compat(
               &dungeon, 0, 0, 0, 0x0040, 0, 1), 1);

    square = (unsigned char)((DUNGEON_ELEMENT_DOOR << 5) | 0x05);
    expect("destroyed door is passable",
           F0708_MOVEMENT_IsSquarePassableForCreature_Compat(
               &dungeon, 0, 0, 0, 0x0000, 0, 1), 1);

    if (failures) return 1;
    puts("PASS: DM1 creature terrain passability matches GROUP.C F0203");
    return 0;
}

/*
 * test_dm1_v1_dor01_f0715_door_resolve_toggle_action_pc34_compat.c
 *
 * Source-locked to ReDMCSB door actuator branch (adjacent to
 * F0275_SENSOR_IsTriggeredByClickOnWall in MOVESENS.C) and the
 * front-door toggle resolver in DUNGEON.C that the Firestaff
 * compat layer exposes as F0715_DOOR_ResolveToggleAction_Compat.
 *
 * DOR-01 (DM1 V1 functional-divergence-report.md):
 *   "F0715 front-door toggle resolver is amalgam-only."
 *
 * F0715 is the new-path replacement that resolves the target
 * door action (OPEN / CLOSE / DESTROYED) for a click on a front
 * wall door.  It is a pure function: reads the square byte via
 * the dungeon tile state and fills outResult.  Never mutates
 * dungeon or party.  The caller is expected to apply
 * outResult.newDoorState to the square via the existing compat
 * square accessor if outResult.kind is OPEN or CLOSE, and skip
 * mutation for DESTROYED.
 *
 * Pins:
 *  T1   NULL outResult returns 0
 *  T2   NULL dungeon returns 0
 *  T3   mapIndex out of range returns 0
 *  T4   mapX out of range returns 0
 *  T5   mapY out of range returns 0
 *  T6   Non-door element (corridor) returns 0
 *  T7   Door state 0 (fully open)  -> kind=CLOSE, newDoorState=4
 *  T8   Door state 4 (fully closed)-> kind=OPEN,  newDoorState=0
 *  T9   Door state 5 (destroyed)   -> kind=DESTROYED, newDoorState=-1
 *  T10  Door state 1 (animating)   -> kind=OPEN,  newDoorState=0 (snap)
 *  T11  Door state 2 (animating)   -> kind=OPEN,  newDoorState=0 (snap)
 *  T12  Door state 3 (animating)   -> kind=OPEN,  newDoorState=0 (snap)
 *  T13  Vertical-bit (0x08) set    -> doorVertical=1
 *  T14  Vertical-bit (0x08) clear  -> doorVertical=0
 *  T15  outResult mapIndex/mapX/mapY are populated even on early reject
 *  T16  Square byte is byte-stable across F0715 call (pure resolver)
 *  T17  Old-state low-nibble is recorded (T8 keeps doorState=4 as oldDoorState)
 *  T18  Hash stability — FNV-1a 32-bit over a 12-fixture sweep
 *
 * Source-locked to ReDMCSB door actuator branch in MOVESENS.C /
 * DUNGEON.C (front-door toggle resolver exposed as F0715).
 */

#include <stdio.h>
#include <string.h>

#include "memory_door_action_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"
#include "dm1_v1_collision_door_pc34_compat.h"

#define CHECK(cond, msg) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, msg); \
        return 1; \
    } \
} while (0)

#define MAP_W 4
#define MAP_H 4

static unsigned char square_type(int elementType, int attrs)
{
    return (unsigned char)((elementType << 5) | (attrs & DUNGEON_SQUARE_MASK_ATTRIBS));
}

static void setup_fixture(struct DungeonDatState_Compat* dungeon,
                          struct DungeonMapDesc_Compat* map,
                          struct DungeonMapTiles_Compat* tiles,
                          unsigned char* squares,
                          unsigned char frontDoorByte)
{
    int x, y;
    memset(dungeon, 0, sizeof(*dungeon));
    memset(map, 0, sizeof(*map));
    memset(tiles, 0, sizeof(*tiles));
    memset(squares, 0, MAP_W * MAP_H);

    map->width = MAP_W;
    map->height = MAP_H;
    tiles->squareData = squares;
    tiles->squareCount = MAP_W * MAP_H;
    dungeon->header.mapCount = 1;
    dungeon->maps = map;
    dungeon->tiles = tiles;
    dungeon->loaded = 1;
    dungeon->tilesLoaded = 1;

    /* All cells start as corridor; the door cell is set by the caller. */
    for (x = 0; x < MAP_W; ++x) {
        for (y = 0; y < MAP_H; ++y) {
            squares[x * MAP_H + y] = square_type(DUNGEON_ELEMENT_CORRIDOR, 0);
        }
    }
    squares[1 * MAP_H + 1] = frontDoorByte;
}

static unsigned int fnv1a_32(const unsigned char* data, size_t n)
{
    /* FNV-1a 32-bit, offset basis 0x811C9DC5, prime 0x01000193. */
    unsigned int h = 0x811C9DC5u;
    size_t i;
    for (i = 0; i < n; ++i) {
        h ^= (unsigned int)data[i];
        h *= 0x01000193u;
    }
    return h;
}

int main(void)
{
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat map;
    struct DungeonMapTiles_Compat tiles;
    unsigned char squares[MAP_W * MAP_H];
    struct DoorToggleResult_Compat r;
    unsigned char doorByte;
    unsigned char saved;
    unsigned char sweep[12 * 12];
    unsigned char* sp;
    unsigned int hash;

    printf("probe=dm1_v1_dor01_f0715_door_resolve_toggle_action_pc34_compat\n");

    /* T1: NULL outResult returns 0. */
    setup_fixture(&dungeon, &map, &tiles, squares,
                  square_type(DUNGEON_ELEMENT_DOOR, DM1_DOOR_STATE_CLOSED));
    CHECK(F0715_DOOR_ResolveToggleAction_Compat(&dungeon, 0, 1, 1, NULL) == 0,
          "T1: NULL outResult returns 0");

    /* T2: NULL dungeon returns 0. */
    memset(&r, 0xFF, sizeof(r));
    CHECK(F0715_DOOR_ResolveToggleAction_Compat(NULL, 0, 1, 1, &r) == 0,
          "T2: NULL dungeon returns 0");

    /* T3: mapIndex out of range. */
    setup_fixture(&dungeon, &map, &tiles, squares,
                  square_type(DUNGEON_ELEMENT_DOOR, DM1_DOOR_STATE_CLOSED));
    CHECK(F0715_DOOR_ResolveToggleAction_Compat(&dungeon, 5, 1, 1, &r) == 0,
          "T3: mapIndex out of range returns 0");

    /* T4: mapX out of range. */
    CHECK(F0715_DOOR_ResolveToggleAction_Compat(&dungeon, 0, 99, 1, &r) == 0,
          "T4: mapX out of range returns 0");

    /* T5: mapY out of range. */
    CHECK(F0715_DOOR_ResolveToggleAction_Compat(&dungeon, 0, 1, 99, &r) == 0,
          "T5: mapY out of range returns 0");

    /* T6: Non-door element (corridor) returns 0. */
    setup_fixture(&dungeon, &map, &tiles, squares,
                  square_type(DUNGEON_ELEMENT_CORRIDOR, 0));
    memset(&r, 0xFF, sizeof(r));
    CHECK(F0715_DOOR_ResolveToggleAction_Compat(&dungeon, 0, 1, 1, &r) == 0,
          "T6: corridor element returns 0");

    /* T7: Door state 0 (fully open) -> CLOSE, newDoorState=4. */
    doorByte = square_type(DUNGEON_ELEMENT_DOOR, DM1_DOOR_STATE_OPEN);
    setup_fixture(&dungeon, &map, &tiles, squares, doorByte);
    saved = squares[1 * MAP_H + 1];
    memset(&r, 0, sizeof(r));
    CHECK(F0715_DOOR_ResolveToggleAction_Compat(&dungeon, 0, 1, 1, &r) == 1,
          "T7a: door state 0 returns 1 (recognized)");
    CHECK(r.kind == DOOR_ACTION_CLOSE,
          "T7b: door state 0 -> kind=CLOSE");
    CHECK(r.newDoorState == 4,
          "T7c: door state 0 -> newDoorState=4");
    CHECK(r.oldDoorState == 0,
          "T7d: door state 0 -> oldDoorState=0");
    CHECK(squares[1 * MAP_H + 1] == saved,
          "T7e: square byte stable (F0715 is pure)");

    /* T8: Door state 4 (fully closed) -> OPEN, newDoorState=0. */
    doorByte = square_type(DUNGEON_ELEMENT_DOOR, DM1_DOOR_STATE_CLOSED);
    setup_fixture(&dungeon, &map, &tiles, squares, doorByte);
    saved = squares[1 * MAP_H + 1];
    memset(&r, 0, sizeof(r));
    CHECK(F0715_DOOR_ResolveToggleAction_Compat(&dungeon, 0, 1, 1, &r) == 1,
          "T8a: door state 4 returns 1");
    CHECK(r.kind == DOOR_ACTION_OPEN,
          "T8b: door state 4 -> kind=OPEN");
    CHECK(r.newDoorState == 0,
          "T8c: door state 4 -> newDoorState=0");
    CHECK(r.oldDoorState == 4,
          "T8d: door state 4 -> oldDoorState=4");
    CHECK(squares[1 * MAP_H + 1] == saved,
          "T8e: square byte stable (F0715 is pure)");

    /* T9: Door state 5 (destroyed) -> DESTROYED, newDoorState=-1. */
    doorByte = square_type(DUNGEON_ELEMENT_DOOR, DM1_DOOR_STATE_DESTROYED);
    setup_fixture(&dungeon, &map, &tiles, squares, doorByte);
    saved = squares[1 * MAP_H + 1];
    memset(&r, 0, sizeof(r));
    CHECK(F0715_DOOR_ResolveToggleAction_Compat(&dungeon, 0, 1, 1, &r) == 1,
          "T9a: door state 5 returns 1");
    CHECK(r.kind == DOOR_ACTION_DESTROYED,
          "T9b: door state 5 -> kind=DESTROYED");
    CHECK(r.newDoorState == -1,
          "T9c: door state 5 -> newDoorState=-1 (no mutation)");
    CHECK(r.oldDoorState == 5,
          "T9d: door state 5 -> oldDoorState=5");
    CHECK(squares[1 * MAP_H + 1] == saved,
          "T9e: square byte stable (F0715 does not write DESTROYED)");

    /* T10: Door state 1 (animating) -> snap to OPEN, newDoorState=0. */
    doorByte = square_type(DUNGEON_ELEMENT_DOOR, 0x11);
    setup_fixture(&dungeon, &map, &tiles, squares, doorByte);
    memset(&r, 0, sizeof(r));
    CHECK(F0715_DOOR_ResolveToggleAction_Compat(&dungeon, 0, 1, 1, &r) == 1,
          "T10a: door state 1 returns 1");
    CHECK(r.kind == DOOR_ACTION_OPEN,
          "T10b: door state 1 -> kind=OPEN (snap)");
    CHECK(r.newDoorState == 0,
          "T10c: door state 1 -> newDoorState=0 (snap)");

    /* T11: Door state 2 (animating) -> snap to OPEN, newDoorState=0. */
    doorByte = square_type(DUNGEON_ELEMENT_DOOR, 0x12);
    setup_fixture(&dungeon, &map, &tiles, squares, doorByte);
    memset(&r, 0, sizeof(r));
    CHECK(F0715_DOOR_ResolveToggleAction_Compat(&dungeon, 0, 1, 1, &r) == 1,
          "T11a: door state 2 returns 1");
    CHECK(r.kind == DOOR_ACTION_OPEN,
          "T11b: door state 2 -> kind=OPEN (snap)");
    CHECK(r.newDoorState == 0,
          "T11c: door state 2 -> newDoorState=0 (snap)");

    /* T12: Door state 3 (animating) -> snap to OPEN, newDoorState=0. */
    doorByte = square_type(DUNGEON_ELEMENT_DOOR, 0x13);
    setup_fixture(&dungeon, &map, &tiles, squares, doorByte);
    memset(&r, 0, sizeof(r));
    CHECK(F0715_DOOR_ResolveToggleAction_Compat(&dungeon, 0, 1, 1, &r) == 1,
          "T12a: door state 3 returns 1");
    CHECK(r.kind == DOOR_ACTION_OPEN,
          "T12b: door state 3 -> kind=OPEN (snap)");
    CHECK(r.newDoorState == 0,
          "T12c: door state 3 -> newDoorState=0 (snap)");

    /* T13: Vertical-bit (0x08) set on door state 4 -> doorVertical=1. */
    doorByte = square_type(DUNGEON_ELEMENT_DOOR, 0x08 | DM1_DOOR_STATE_CLOSED);
    setup_fixture(&dungeon, &map, &tiles, squares, doorByte);
    memset(&r, 0, sizeof(r));
    CHECK(F0715_DOOR_ResolveToggleAction_Compat(&dungeon, 0, 1, 1, &r) == 1,
          "T13a: vertical door returns 1");
    CHECK(r.doorVertical == 1,
          "T13b: vertical door -> doorVertical=1");

    /* T14: Vertical-bit (0x08) clear on door state 4 -> doorVertical=0. */
    doorByte = square_type(DUNGEON_ELEMENT_DOOR, DM1_DOOR_STATE_CLOSED);
    setup_fixture(&dungeon, &map, &tiles, squares, doorByte);
    memset(&r, 0, sizeof(r));
    CHECK(F0715_DOOR_ResolveToggleAction_Compat(&dungeon, 0, 1, 1, &r) == 1,
          "T14a: horizontal door returns 1");
    CHECK(r.doorVertical == 0,
          "T14b: horizontal door -> doorVertical=0");

    /* T15: outResult mapIndex/mapX/mapY are populated on early reject. */
    setup_fixture(&dungeon, &map, &tiles, squares,
                  square_type(DUNGEON_ELEMENT_CORRIDOR, 0));
    memset(&r, 0xFF, sizeof(r));
    CHECK(F0715_DOOR_ResolveToggleAction_Compat(&dungeon, 0, 2, 2, &r) == 0,
          "T15a: non-door returns 0");
    CHECK(r.mapIndex == 0 && r.mapX == 2 && r.mapY == 2,
          "T15b: outResult coords populated even on early reject");
    CHECK(r.oldDoorState == -1 && r.newDoorState == -1,
          "T15c: outResult old/new door state are -1 on early reject");

    /* T16: Square byte is byte-stable across F0715 (pure resolver). */
    doorByte = square_type(DUNGEON_ELEMENT_DOOR, 0x08 | 0x03);
    setup_fixture(&dungeon, &map, &tiles, squares, doorByte);
    saved = squares[1 * MAP_H + 1];
    memset(&r, 0, sizeof(r));
    (void)F0715_DOOR_ResolveToggleAction_Compat(&dungeon, 0, 1, 1, &r);
    CHECK(squares[1 * MAP_H + 1] == saved,
          "T16: square byte identical before/after F0715 (no mutation)");

    /* T17: Old-state low-nibble is recorded. */
    doorByte = square_type(DUNGEON_ELEMENT_DOOR, 0x08 | 0x02);
    setup_fixture(&dungeon, &map, &tiles, squares, doorByte);
    memset(&r, 0, sizeof(r));
    CHECK(F0715_DOOR_ResolveToggleAction_Compat(&dungeon, 0, 1, 1, &r) == 1,
          "T17a: door with attrs 0x08|0x02 returns 1");
    CHECK(r.oldDoorState == 0x02,
          "T17b: oldDoorState=0x02 (low-nibble of attrs)");

    /* T18: Hash stability — 12-fixture sweep is deterministic.
     *
     * Sweep covers all 6 door states (0..5) x 2 orientations
     * (vertical / horizontal).  The hash depends only on the
     * deterministic F0715 contract — it must be stable across
     * runs and platforms. */
    sp = sweep;
    {
        int state, vert;
        for (state = 0; state < 6; ++state) {
            for (vert = 0; vert < 2; ++vert) {
                unsigned char b = (unsigned char)((vert ? 0x08 : 0) | state);
                doorByte = square_type(DUNGEON_ELEMENT_DOOR, b);
                setup_fixture(&dungeon, &map, &tiles, squares, doorByte);
                memset(&r, 0, sizeof(r));
                (void)F0715_DOOR_ResolveToggleAction_Compat(&dungeon, 0, 1, 1, &r);
                memcpy(sp, &r, sizeof(r));
                sp += sizeof(r);
            }
        }
    }
    hash = fnv1a_32(sweep, sizeof(sweep));
    CHECK(hash == 0xEC4F85A7u,
          "T18: FNV-1a 32-bit hash stable for 12-fixture sweep (0xEC4F85A7)");

    printf("PASS: DOR-01 F0715 front-door toggle resolver (18 scenarios)\n");
    return 0;
}

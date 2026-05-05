#include <stdio.h>
#include <string.h>

#include "dm1_v1_dungeon_square_structs_pc34_compat.h"
#include "memory_movement_pc34_compat.h"

/*
 * Focused DM1 V1 wall/occlusion/blocker probe.
 *
 * Source lock (ReDMCSB WIP20210206, Toolchains/Common/Source):
 * - DEFS.H:1001-1015 locks M034/M035 raw square-byte decoding and the
 *   stored/runtime element values C00..C06 and C16..C19.
 * - DUNGEON.C:35-44,1371-1421 locks direction-to-step tables and
 *   F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement.
 * - DUNGEON.C:1423-1475 locks F0151_DUNGEON_GetSquare and out-of-bounds
 *   wall semantics.
 * - DUNGEON.C:2466-2721 locks F0172_DUNGEON_SetSquareAspect; key cases:
 *   2628-2648 pit open/closed classification, 2651-2664 fakewall wall/
 *   corridor classification, 2693-2707 stairs/door side-vs-front aspect.
 * - CLIKMENU.C:224-233,264-323 locks movement command relative steps,
 *   wall/door/fakewall blocker decisions, input discard, and unchanged
 *   party state on blocked movement.
 * - DUNVIEW.C:8445-8542 locks viewport draw order: far extras, then rows
 *   D3L,D3R,D3C; D2L,D2R,D2C; D1L,D1R,D1C; D0L,D0R,D0C.
 * - DUNVIEW.C:7727-7926,7960-8162,8164+ locks near front/side wall draw
 *   functions and returns for solid near walls.
 */

#define MAP_W 5
#define MAP_H 5

struct MapFixture {
    unsigned char squares[MAP_W * MAP_H];
};

static int expect_int(const char* label, int got, int want)
{
    if (got != want) {
        fprintf(stderr, "FAIL %s got=%d want=%d\n", label, got, want);
        return 0;
    }
    return 1;
}

static unsigned char sqb(int element, int flags)
{
    return (unsigned char)(((element & 7) << 5) | (flags & 0x1f));
}

static void set_square(unsigned char* squares, int x, int y, unsigned char value)
{
    squares[x * MAP_H + y] = value;
}

static unsigned char read_square_for_view(int map_x, int map_y, void* user_data)
{
    struct MapFixture* fixture = (struct MapFixture*)user_data;
    if (map_x < 0 || map_x >= MAP_W || map_y < 0 || map_y >= MAP_H) {
        return sqb(DM1_ELEMENT_WALL, 0);
    }
    return fixture->squares[map_x * MAP_H + map_y];
}

static void reset_fixture(struct MapFixture* fixture,
    struct DungeonDatState_Compat* dungeon,
    struct DungeonMapDesc_Compat* map,
    struct DungeonMapTiles_Compat* tiles,
    struct PartyState_Compat* party)
{
    memset(fixture, 0, sizeof(*fixture));
    memset(dungeon, 0, sizeof(*dungeon));
    memset(map, 0, sizeof(*map));
    memset(tiles, 0, sizeof(*tiles));
    memset(party, 0, sizeof(*party));

    for (int x = 0; x < MAP_W; ++x) {
        for (int y = 0; y < MAP_H; ++y) {
            set_square(fixture->squares, x, y, sqb(DM1_ELEMENT_CORRIDOR, 0));
        }
    }

    map->width = MAP_W;
    map->height = MAP_H;
    tiles->squareData = fixture->squares;
    tiles->squareCount = MAP_W * MAP_H;
    dungeon->header.mapCount = 1;
    dungeon->maps = map;
    dungeon->tiles = tiles;
    dungeon->loaded = 1;
    dungeon->tilesLoaded = 1;

    party->mapIndex = 0;
    party->mapX = 2;
    party->mapY = 2;
    party->direction = DIR_NORTH;
    party->championCount = 1;
}

static int expect_front_aspect(struct MapFixture* fixture, unsigned char raw,
    int expected_element, int expected_blocks, const char* label)
{
    dm1_viewport_state_t vp;
    int ok = 1;

    set_square(fixture->squares, 2, 1, raw);
    dm1_build_viewport(2, 2, DIR_NORTH, 0, read_square_for_view, fixture, &vp);

    printf("frontClassification label=%s raw=0x%02x aspectElement=%d blocksMovement=%d source=DUNGEON.C:2522-2523,2628-2707 CLIKMENU.C:278-288\n",
        label, raw, vp.squares[6].aspect[DM1_SQA_ELEMENT], dm1_square_blocks_movement(raw));

    ok &= expect_int(label, vp.squares[6].aspect[DM1_SQA_ELEMENT], expected_element);
    ok &= expect_int("movement block classification", dm1_square_blocks_movement(raw) ? 1 : 0, expected_blocks);
    return ok;
}

static int verify_draw_order(struct MapFixture* fixture)
{
    dm1_viewport_state_t vp;
    int visible[DM1_VIEWPORT_SQUARE_COUNT];
    const int expected[DM1_VIEWPORT_SQUARE_COUNT] = { 1, 2, 0, 4, 5, 3, 7, 8, 6, 10, 11, 9 };
    const char* names[DM1_VIEWPORT_SQUARE_COUNT] = {
        "D3C", "D3L", "D3R", "D2C", "D2L", "D2R", "D1C", "D1L", "D1R", "D0C", "D0L", "D0R"
    };
    int count;
    int ok = 1;

    dm1_build_viewport(2, 2, DIR_NORTH, 0, read_square_for_view, fixture, &vp);
    count = dm1_get_visible_squares(&vp, visible);
    ok &= expect_int("visible count no occlusion", count, DM1_VIEWPORT_SQUARE_COUNT);

    printf("drawOrder source=DUNVIEW.C:8490-8542 order=");
    for (int i = 0; i < count; ++i) {
        printf("%s%s", i ? "," : "", names[visible[i]]);
        ok &= expect_int("draw order index", visible[i], expected[i]);
    }
    printf("\n");

    set_square(fixture->squares, 2, 1, sqb(DM1_ELEMENT_WALL, 0));
    dm1_build_viewport(2, 2, DIR_NORTH, 0, read_square_for_view, fixture, &vp);
    count = dm1_get_visible_squares(&vp, visible);
    printf("occludedByD1CWall visibleCount=%d firstVisible=%s source=DUNVIEW.C:8532-8533,7784-7872\n",
        count, count ? names[visible[0]] : "none");
    ok &= expect_int("D1C wall occludes D2/D3 rows", count, 6);
    ok &= expect_int("D1L remains first visible after D1C wall", visible[0], 7);
    ok &= expect_int("D1C wall front flag", dm1_is_front_wall_at_depth(&vp, 1) ? 1 : 0, 1);

    return ok;
}

int main(void)
{
    struct MapFixture fixture;
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat map;
    struct DungeonMapTiles_Compat tiles;
    struct PartyState_Compat party;
    struct MovementResult_Compat move;
    int ok = 1;

    printf("probe=firestaff_dm1_v1_walls_occlusion_blockers_probe\n");
    printf("primarySource=ReDMCSB_WIP20210206/Toolchains/Common/Source\n");
    printf("sourceEvidence=DEFS.H:1001-1015; DUNGEON.C:35-44,1371-1475,2466-2721; CLIKMENU.C:224-233,264-323; DUNVIEW.C:7727-7926,7960-8162,8164+,8445-8542\n");

    reset_fixture(&fixture, &dungeon, &map, &tiles, &party);
    set_square(fixture.squares, 2, 1, sqb(DM1_ELEMENT_WALL, 0));
    ok &= expect_int("front wall blocks forward movement",
        F0702_MOVEMENT_TryMove_Compat(&dungeon, &party, MOVE_FORWARD, &move), 0);
    ok &= expect_int("front wall result code", move.resultCode, MOVE_BLOCKED_WALL);
    ok &= expect_int("blocked party mapX stable", party.mapX, 2);
    ok &= expect_int("blocked party mapY stable", party.mapY, 2);
    ok &= expect_int("blocked party direction stable", party.direction, DIR_NORTH);
    ok &= expect_int("blocked result mapX stable", move.newMapX, 2);
    ok &= expect_int("blocked result mapY stable", move.newMapY, 2);
    ok &= expect_int("blocked result direction stable", move.newDirection, DIR_NORTH);
    printf("frontWallMovement resultCode=%d party=(%d,%d,%d) source=CLIKMENU.C:269-323\n",
        move.resultCode, party.mapX, party.mapY, party.direction);

    reset_fixture(&fixture, &dungeon, &map, &tiles, &party);
    ok &= expect_front_aspect(&fixture, sqb(DM1_ELEMENT_WALL, 0), DM1_ELEMENT_WALL, 1, "wall");
    ok &= expect_front_aspect(&fixture, sqb(DM1_ELEMENT_CORRIDOR, 0), DM1_ELEMENT_CORRIDOR, 0, "corridor");
    ok &= expect_front_aspect(&fixture, sqb(DM1_ELEMENT_PIT, DM1_PIT_OPEN), DM1_ELEMENT_PIT, 0, "open-pit");
    ok &= expect_front_aspect(&fixture, sqb(DM1_ELEMENT_PIT, 0), DM1_ELEMENT_CORRIDOR, 0, "closed-pit-as-corridor");
    ok &= expect_front_aspect(&fixture, sqb(DM1_ELEMENT_DOOR, 0), DM1_ELEMENT_DOOR_SIDE, 0, "north-south-door-seen-side-facing-north");
    ok &= expect_front_aspect(&fixture, sqb(DM1_ELEMENT_DOOR, DM1_DOOR_NS_ORIENTATION | 2), DM1_ELEMENT_DOOR_FRONT, 1, "east-west-closed-door-seen-front-facing-north");
    ok &= expect_front_aspect(&fixture, sqb(DM1_ELEMENT_FAKEWALL, 0), DM1_ELEMENT_WALL, 1, "closed-real-fakewall");
    ok &= expect_front_aspect(&fixture, sqb(DM1_ELEMENT_FAKEWALL, DM1_FAKEWALL_IMAGINARY), DM1_ELEMENT_WALL, 0, "closed-imaginary-fakewall-renders-wall-but-walks");
    ok &= expect_front_aspect(&fixture, sqb(DM1_ELEMENT_FAKEWALL, DM1_FAKEWALL_OPEN), DM1_ELEMENT_CORRIDOR, 0, "open-fakewall");

    reset_fixture(&fixture, &dungeon, &map, &tiles, &party);
    ok &= verify_draw_order(&fixture);

    if (!ok) {
        fprintf(stderr, "probe failed\n");
        return 1;
    }
    printf("result=pass\n");
    return 0;
}

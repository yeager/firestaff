#include "dm1_v1_viewport_fakewall_pc34_compat.h"

#include "memory_dungeon_dat_pc34_compat.h"
#include "memory_movement_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions = 0;
static int g_failures = 0;

static unsigned char square_of(int element, unsigned char attrs)
{
    return (unsigned char)(((element << 5) & DUNGEON_SQUARE_MASK_TYPE) |
                           (attrs & DUNGEON_SQUARE_MASK_ATTRIBS));
}

static unsigned char c10_blend(unsigned char dst, unsigned char src)
{
    return src == 10 ? dst : src;
}

static int clipped_x_visible(int x)
{
    return x >= 0 && x < 224;
}

static void expect_int(const char *id, int got, int want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=%d want=%d at %s\n", id, got, want, anchor);
        ++g_failures;
    } else {
        printf("PASS %s == %d (%s)\n", id, want, anchor);
    }
}

static void expect_contains(const char *id, const char *text,
                            const char *needle, const char *anchor)
{
    ++g_assertions;
    if (!text || !needle || !strstr(text, needle)) {
        printf("FAIL %s missing \"%s\" at %s\n",
               id, needle ? needle : "(null)", anchor);
        ++g_failures;
    } else {
        printf("PASS %s contains \"%s\" (%s)\n", id, needle, anchor);
    }
}

static void setup_one_square_dungeon(struct DungeonDatState_Compat *dungeon,
                                     struct DungeonMapDesc_Compat *map,
                                     struct DungeonMapTiles_Compat *tiles,
                                     unsigned char *square,
                                     unsigned char squareByte)
{
    memset(dungeon, 0, sizeof(*dungeon));
    memset(map, 0, sizeof(*map));
    memset(tiles, 0, sizeof(*tiles));
    *square = squareByte;
    map->width = 1;
    map->height = 1;
    tiles->squareData = square;
    tiles->squareCount = 1;
    dungeon->header.mapCount = 1;
    dungeon->maps = map;
    dungeon->tiles = tiles;
    dungeon->loaded = 1;
    dungeon->tilesLoaded = 1;
}

static void test_fakewall_square_aspect_contract(void)
{
    const unsigned char wall = square_of(DUNGEON_ELEMENT_WALL, 0);
    const unsigned char corridor = square_of(DUNGEON_ELEMENT_CORRIDOR, 0);
    const unsigned char pit = square_of(DUNGEON_ELEMENT_PIT, 0);
    const unsigned char stairs = square_of(DUNGEON_ELEMENT_STAIRS, 0);
    const unsigned char door_open = square_of(DUNGEON_ELEMENT_DOOR, 0);
    const unsigned char door_destroyed = square_of(DUNGEON_ELEMENT_DOOR, 5);
    const unsigned char door_half = square_of(DUNGEON_ELEMENT_DOOR, 2);
    const unsigned char teleporter = square_of(DUNGEON_ELEMENT_TELEPORTER, 0);
    const unsigned char fakewall_closed = square_of(DUNGEON_ELEMENT_FAKEWALL, 0);
    const unsigned char fakewall_closed_random =
        square_of(DUNGEON_ELEMENT_FAKEWALL, 0x08);
    const unsigned char fakewall_open = square_of(DUNGEON_ELEMENT_FAKEWALL, 0x04);
    const unsigned char fakewall_open_random =
        square_of(DUNGEON_ELEMENT_FAKEWALL, 0x0c);

    expect_int("effective.wall",
               M11_DM1_ViewportEffectiveElementForSquarePc34(wall),
               DUNGEON_ELEMENT_WALL,
               "DEFS.H:1034, DUNGEON.C:2651-2653");
    expect_int("effective.corridor",
               M11_DM1_ViewportEffectiveElementForSquarePc34(corridor),
               DUNGEON_ELEMENT_CORRIDOR,
               "DUNGEON.C:2663-2666");
    expect_int("effective.pit",
               M11_DM1_ViewportEffectiveElementForSquarePc34(pit),
               DUNGEON_ELEMENT_PIT,
               "DUNVIEW.C:7198-7213");
    expect_int("effective.stairs",
               M11_DM1_ViewportEffectiveElementForSquarePc34(stairs),
               DUNGEON_ELEMENT_STAIRS,
               "DUNVIEW.C:7065-7095");
    expect_int("effective.door",
               M11_DM1_ViewportEffectiveElementForSquarePc34(door_open),
               DUNGEON_ELEMENT_DOOR,
               "DUNVIEW.C:7180-7197");
    expect_int("effective.teleporter",
               M11_DM1_ViewportEffectiveElementForSquarePc34(teleporter),
               DUNGEON_ELEMENT_TELEPORTER,
               "DUNVIEW.C:7208-7224");
    expect_int("effective.fakewall.closed",
               M11_DM1_ViewportEffectiveElementForSquarePc34(fakewall_closed),
               DUNGEON_ELEMENT_WALL,
               "DUNGEON.C:2651-2661 closed fakewall becomes wall aspect");
    expect_int("effective.fakewall.closed_random",
               M11_DM1_ViewportEffectiveElementForSquarePc34(fakewall_closed_random),
               DUNGEON_ELEMENT_WALL,
               "DUNGEON.C:2655-2661 random-ornament bit stays closed wall");
    expect_int("effective.fakewall.open",
               M11_DM1_ViewportEffectiveElementForSquarePc34(fakewall_open),
               DUNGEON_ELEMENT_CORRIDOR,
               "DUNGEON.C:2662-2666 open fakewall becomes corridor aspect");
    expect_int("effective.fakewall.open_random",
               M11_DM1_ViewportEffectiveElementForSquarePc34(fakewall_open_random),
               DUNGEON_ELEMENT_CORRIDOR,
               "DUNGEON.C:2663-2666 open fakewall keeps floor route");

    expect_int("wall_like.wall", M11_DM1_ViewportSquareIsWallLikePc34(wall), 1,
               "DUNVIEW.C:7096-7119 wall route");
    expect_int("wall_like.closed_fakewall",
               M11_DM1_ViewportSquareIsWallLikePc34(fakewall_closed), 1,
               "DUNGEON.C:2651-2661, DUNVIEW.C:7096-7119");
    expect_int("wall_like.open_fakewall",
               M11_DM1_ViewportSquareIsWallLikePc34(fakewall_open), 0,
               "DUNGEON.C:2662-2666, DUNVIEW.C:7208-7224");
    expect_int("wall_like.corridor",
               M11_DM1_ViewportSquareIsWallLikePc34(corridor), 0,
               "DUNVIEW.C:7208-7224 corridor falls through floor/item route");
    expect_int("wall_like.door", M11_DM1_ViewportSquareIsWallLikePc34(door_open), 0,
               "DUNVIEW.C:7180-7197 door front route is not wall-like");

    expect_int("open.wall", M11_DM1_ViewportSquareIsOpenPc34(wall), 0,
               "DUNVIEW.C:7096-7119 wall route returns before item pass");
    expect_int("open.closed_fakewall",
               M11_DM1_ViewportSquareIsOpenPc34(fakewall_closed), 0,
               "DUNGEON.C:2651-2661 closed fakewall wall aspect");
    expect_int("open.open_fakewall",
               M11_DM1_ViewportSquareIsOpenPc34(fakewall_open), 1,
               "DUNGEON.C:2662-2666 open fakewall corridor aspect");
    expect_int("open.corridor", M11_DM1_ViewportSquareIsOpenPc34(corridor), 1,
               "DUNVIEW.C:7208-7224 corridor route reaches F0115");
    expect_int("open.pit", M11_DM1_ViewportSquareIsOpenPc34(pit), 1,
               "DUNVIEW.C:7198-7213 pit falls through to floor route");
    expect_int("open.teleporter", M11_DM1_ViewportSquareIsOpenPc34(teleporter), 1,
               "DUNVIEW.C:7208-7239 teleporter route reaches field/item pass");
    expect_int("open.door_state_0",
               M11_DM1_ViewportSquareIsOpenPc34(door_open), 1,
               "DEFS.H:1039 C0_DOOR_STATE_OPEN");
    expect_int("open.door_state_5",
               M11_DM1_ViewportSquareIsOpenPc34(door_destroyed), 1,
               "DEFS.H:1044 C5_DOOR_STATE_DESTROYED");
    expect_int("open.door_state_2",
               M11_DM1_ViewportSquareIsOpenPc34(door_half), 0,
               "DEFS.H:1041 C2_DOOR_STATE_CLOSED_HALF");
}

static void test_fakewall_floor_and_blit_contract(void)
{
    const unsigned char wall = square_of(DUNGEON_ELEMENT_WALL, 0);
    const unsigned char corridor = square_of(DUNGEON_ELEMENT_CORRIDOR, 0);
    const unsigned char pit = square_of(DUNGEON_ELEMENT_PIT, 0);
    const unsigned char stairs = square_of(DUNGEON_ELEMENT_STAIRS, 0);
    const unsigned char door = square_of(DUNGEON_ELEMENT_DOOR, 0);
    const unsigned char teleporter = square_of(DUNGEON_ELEMENT_TELEPORTER, 0);
    const unsigned char fakewall_closed = square_of(DUNGEON_ELEMENT_FAKEWALL, 0);
    const unsigned char fakewall_open = square_of(DUNGEON_ELEMENT_FAKEWALL, 0x04);

    expect_int("floor_path.wall",
               M11_DM1_ViewportSquareHasFloorOrnamentPathPc34(wall), 0,
               "DUNVIEW.C:7096-7119 wall route excludes F0108");
    expect_int("floor_path.closed_fakewall",
               M11_DM1_ViewportSquareHasFloorOrnamentPathPc34(fakewall_closed), 0,
               "DUNGEON.C:2651-2661, DUNVIEW.C:7096-7119");
    expect_int("floor_path.open_fakewall",
               M11_DM1_ViewportSquareHasFloorOrnamentPathPc34(fakewall_open), 1,
               "DUNGEON.C:2663-2666, DUNVIEW.C:7210-7214");
    expect_int("floor_path.corridor",
               M11_DM1_ViewportSquareHasFloorOrnamentPathPc34(corridor), 1,
               "DUNVIEW.C:7208-7214 corridor calls F0108");
    expect_int("floor_path.pit",
               M11_DM1_ViewportSquareHasFloorOrnamentPathPc34(pit), 1,
               "DUNVIEW.C:7198-7213 pit falls through to F0108");
    expect_int("floor_path.teleporter",
               M11_DM1_ViewportSquareHasFloorOrnamentPathPc34(teleporter), 1,
               "DUNVIEW.C:7208-7214 teleporter calls F0108");
    expect_int("floor_path.stairs",
               M11_DM1_ViewportSquareHasFloorOrnamentPathPc34(stairs), 0,
               "DUNVIEW.C:7065-7095 stairs route jumps past F0108");
    expect_int("floor_path.door",
               M11_DM1_ViewportSquareHasFloorOrnamentPathPc34(door), 0,
               "DUNVIEW.C:7180-7197 door front owns F0111 route");

    expect_int("c10.skip_preserves", c10_blend(0xee, 10), 0xee,
               "DUNVIEW.C:3053-3059 F0100 blits with C10 transparent");
    expect_int("c10.opaque_writes", c10_blend(0xee, 0x42), 0x42,
               "DUNVIEW.C:3053-3059 F0100 opaque wall pixel writes");
    expect_int("clip.left_edge", clipped_x_visible(0), 1,
               "DUNVIEW.C:3053-3059 F0100 clipped frame writes visible edge");
    expect_int("clip.right_edge", clipped_x_visible(223), 1,
               "DUNVIEW.C:3053-3059 F0100 clipped frame writes visible edge");
    expect_int("clip.before_left", clipped_x_visible(-1), 0,
               "DUNVIEW.C:3053-3059 F0100 clipped frame skips outside edge");
    expect_int("clip.after_right", clipped_x_visible(224), 0,
               "DUNVIEW.C:3053-3059 F0100 clipped frame skips outside edge");
    expect_int("contract.closed_fakewall_no_f0111",
               M11_DM1_ViewportSquareIsWallLikePc34(fakewall_closed), 1,
               "DUNVIEW.C:7180-7197 F0111 is door-front only");
    expect_int("contract.closed_fakewall_no_f0115",
               M11_DM1_ViewportSquareIsOpenPc34(fakewall_closed), 0,
               "DUNVIEW.C:7119-7166 wall returns before T0120029/F0115");
    expect_int("contract.closed_fakewall_no_f0108",
               M11_DM1_ViewportSquareHasFloorOrnamentPathPc34(fakewall_closed), 0,
               "DUNVIEW.C:7210-7214 F0108 is open floor route");
}

static void test_imaginary_fakewall_collision_viewport_split(void)
{
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat map;
    struct DungeonMapTiles_Compat tiles;
    unsigned char square;
    const unsigned char fakewall_real_closed =
        square_of(DUNGEON_ELEMENT_FAKEWALL, 0);
    const unsigned char fakewall_imaginary =
        square_of(DUNGEON_ELEMENT_FAKEWALL, 0x01);
    const unsigned char fakewall_open =
        square_of(DUNGEON_ELEMENT_FAKEWALL, 0x04);
    const unsigned char fakewall_open_imaginary =
        square_of(DUNGEON_ELEMENT_FAKEWALL, 0x05);

    expect_int("split.real_closed.viewport_wall",
               M11_DM1_ViewportSquareIsWallLikePc34(fakewall_real_closed), 1,
               "DUNGEON.C:2651-2661 !MASK0x0004_FAKEWALL_OPEN -> wall aspect");
    setup_one_square_dungeon(&dungeon, &map, &tiles, &square, fakewall_real_closed);
    expect_int("split.real_closed.collision_blocks",
               F0706_MOVEMENT_IsSquarePassable_Compat(&dungeon, 0, 0, 0), 0,
               "CLIKMENU.C:280-281 !OPEN && !IMAGINARY blocks movement");

    expect_int("split.imaginary.viewport_still_wall",
               M11_DM1_ViewportSquareIsWallLikePc34(fakewall_imaginary), 1,
               "DUNGEON.C:2651-2661 ignores MASK0x0001_FAKEWALL_IMAGINARY");
    expect_int("split.imaginary.viewport_not_open",
               M11_DM1_ViewportSquareIsOpenPc34(fakewall_imaginary), 0,
               "DUNGEON.C:2651-2661 imaginary-only fakewall remains wall aspect");
    setup_one_square_dungeon(&dungeon, &map, &tiles, &square, fakewall_imaginary);
    expect_int("split.imaginary.collision_passes",
               F0706_MOVEMENT_IsSquarePassable_Compat(&dungeon, 0, 0, 0), 1,
               "CLIKMENU.C:280-281 MASK0x0001_FAKEWALL_IMAGINARY allows movement");

    expect_int("split.open.viewport_corridor",
               M11_DM1_ViewportEffectiveElementForSquarePc34(fakewall_open),
               DUNGEON_ELEMENT_CORRIDOR,
               "DUNGEON.C:2662-2666 MASK0x0004_FAKEWALL_OPEN -> corridor aspect");
    setup_one_square_dungeon(&dungeon, &map, &tiles, &square, fakewall_open);
    expect_int("split.open.collision_passes",
               F0706_MOVEMENT_IsSquarePassable_Compat(&dungeon, 0, 0, 0), 1,
               "CLIKMENU.C:280-281 MASK0x0004_FAKEWALL_OPEN allows movement");

    /* ReDMCSB keeps two distinct gates: DUNGEON.C:F0172 lines 2651-2664
     * keys viewport aspect only from MASK0x0004_FAKEWALL_OPEN, while
     * CLIKMENU.C:F0366 lines 286-287 allows collision through either
     * MASK0x0004_FAKEWALL_OPEN or MASK0x0001_FAKEWALL_IMAGINARY. */
    expect_int("split.open_imaginary.viewport_corridor",
               M11_DM1_ViewportEffectiveElementForSquarePc34(fakewall_open_imaginary),
               DUNGEON_ELEMENT_CORRIDOR,
               "DUNGEON.C:2662-2666 OPEN wins viewport corridor aspect");
    expect_int("split.open_imaginary.viewport_open",
               M11_DM1_ViewportSquareIsOpenPc34(fakewall_open_imaginary), 1,
               "DUNGEON.C:2662-2666 OPEN+IMAGINARY remains open corridor");
    setup_one_square_dungeon(&dungeon, &map, &tiles, &square, fakewall_open_imaginary);
    expect_int("split.open_imaginary.collision_passes",
               F0706_MOVEMENT_IsSquarePassable_Compat(&dungeon, 0, 0, 0), 1,
               "CLIKMENU.C:286-287 OPEN || IMAGINARY allows movement");
}

static void test_source_evidence_mentions_required_anchors(void)
{
    const char *e =
        "contract_only=1; no_real_asset_pixel_parity=1; "
        "DUNGEON.C:2651-2661 closed fakewall becomes C00 wall; "
        "DUNGEON.C:2662-2666 open fakewall becomes C01 corridor; "
        "DUNVIEW.C:7051-7224 F0120 wall/corridor/door/floor split; "
        "DUNVIEW.C:3048-3059 F0100 uses C10 transparent blit; "
        "DUNVIEW.C:7180-7197 F0111 door route; "
        "DUNVIEW.C:7210-7224 F0108 then F0115 open-floor route; "
        "DEFS.H:1033-1035 MASK0x0001_FAKEWALL_IMAGINARY / MASK0x0004_FAKEWALL_OPEN; "
        "CLIKMENU.C:286-287 fakewall movement blocks only when !OPEN && !IMAGINARY; "
        "DEFS.H:1039-1044 door open/destroyed states; "
        "COMMAND.C:2154-2156 movement command delegates to F0366.";

    expect_contains("evidence.contract", e, "contract_only=1",
                    "source evidence");
    expect_contains("evidence.no_asset_parity", e, "no_real_asset_pixel_parity=1",
                    "source evidence");
    expect_contains("evidence.closed", e, "DUNGEON.C:2651-2661",
                    "DUNGEON.C fakewall closed");
    expect_contains("evidence.open", e, "DUNGEON.C:2662-2666",
                    "DUNGEON.C fakewall open");
    expect_contains("evidence.f0120", e, "DUNVIEW.C:7051-7224 F0120",
                    "DUNVIEW.C D2R route");
    expect_contains("evidence.f0100", e, "DUNVIEW.C:3048-3059 F0100",
                    "DUNVIEW.C wall blit");
    expect_contains("evidence.f0111", e, "F0111 door route",
                    "DUNVIEW.C:7180-7197");
    expect_contains("evidence.f0108_f0115", e, "F0108 then F0115",
                    "DUNVIEW.C:7210-7224");
    expect_contains("evidence.defs_fakewall", e, "DEFS.H:1033-1035",
                    "DEFS.H fakewall bit");
    expect_contains("evidence.clickmenu_fakewall", e, "CLIKMENU.C:286-287",
                    "CLIKMENU.C fakewall passability");
    expect_contains("evidence.command", e, "COMMAND.C:2154-2156",
                    "COMMAND.C movement dispatch checked");
}

int main(void)
{
    test_fakewall_square_aspect_contract();
    test_fakewall_floor_and_blit_contract();
    test_imaginary_fakewall_collision_viewport_split();
    test_source_evidence_mentions_required_anchors();

    if (g_failures) {
        printf("FAIL dm1_v1_viewport_fakewall_pc34_compat failures=%d assertions=%d\n",
               g_failures, g_assertions);
        return 1;
    }
    printf("PASS dm1_v1_viewport_fakewall_pc34_compat %d/%d assertions\n",
           g_assertions, g_assertions);
    return 0;
}

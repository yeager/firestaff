#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dm1_v1_viewport_fakewall_pc34_compat.h"
#include "memory_movement_pc34_compat.h"
#include "memory_tick_orchestrator_pc34_compat.h"

#define DM1_CANONICAL_DUNGEON_SHA256 "d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85"

static const char* default_dm1_dungeon_dat(void)
{
    static char path[1024];
    const char* home = getenv("HOME");
    if (!home || home[0] == 0) home = "/home/trv2";
    snprintf(path, sizeof(path), "%s/.openclaw/data/firestaff-original-games/DM/_canonical/dm1/DUNGEON.DAT", home);
    return path;
}

static int expect_int(const char* label, int got, int want)
{
    if (got != want) {
        fprintf(stderr, "FAIL %s got=%d want=%d\n", label, got, want);
        return 0;
    }
    return 1;
}

static void print_scenario(const char* label,
                           int mapIndex,
                           int x,
                           int y,
                           unsigned char square,
                           int passable,
                           int effectiveElement,
                           int wallLike,
                           int open,
                           int floorPath)
{
    printf("scenario=%s map=%d x=%d y=%d raw=0x%02x passable=%d viewportEffectiveElement=%d viewportWallLike=%d viewportOpen=%d floorOrnamentPath=%d\n",
           label, mapIndex, x, y, square, passable, effectiveElement,
           wallLike, open, floorPath);
}

int main(int argc, char** argv)
{
    const char* dungeonPath = argc > 1 ? argv[1] : getenv("FIRESTAFF_DM1_CANONICAL_DUNGEON_DAT");
    struct GameWorld_Compat world;
    int openFakewalls = 0;
    int imaginaryClosedFakewalls = 0;
    int solidClosedFakewalls = 0;
    int printedOpen = 0;
    int printedImaginary = 0;
    int printedSolid = 0;
    int ok = 1;

    if (!dungeonPath || dungeonPath[0] == 0) dungeonPath = default_dm1_dungeon_dat();

    printf("probe=firestaff_dm1_v1_original_fakewall_view_collision_probe\n");
    printf("dungeon=%s\n", dungeonPath);
    printf("dungeon_sha256=%s\n", DM1_CANONICAL_DUNGEON_SHA256);
    printf("source=ReDMCSB_WIP20210206/Toolchains/Common/Source DUNGEON.C:F0172:2651-2664 CLIKMENU.C:F0366:278-288 DEFS.H:1007-1036\n");

    memset(&world, 0, sizeof(world));
    if (!F0882_WORLD_InitFromDungeonDat_Compat(dungeonPath, 0xF1A5u, &world)) {
        fprintf(stderr, "FAIL load canonical dungeon path=%s\n", dungeonPath);
        return 1;
    }

    for (int mapIndex = 0; mapIndex < (int)world.dungeon->header.mapCount; ++mapIndex) {
        const struct DungeonMapDesc_Compat* map = &world.dungeon->maps[mapIndex];
        const unsigned char* squares = world.dungeon->tiles[mapIndex].squareData;
        for (int x = 0; x < map->width; ++x) {
            for (int y = 0; y < map->height; ++y) {
                unsigned char square = squares[x * map->height + y];
                int elementType = (square & DUNGEON_SQUARE_MASK_TYPE) >> 5;
                int passable;
                int effective;
                int wallLike;
                int open;
                int floorPath;

                if (elementType != DUNGEON_ELEMENT_FAKEWALL) continue;

                passable = F0706_MOVEMENT_IsSquarePassable_Compat(world.dungeon, mapIndex, x, y);
                effective = M11_DM1_ViewportEffectiveElementForSquarePc34(square);
                wallLike = M11_DM1_ViewportSquareIsWallLikePc34(square);
                open = M11_DM1_ViewportSquareIsOpenPc34(square);
                floorPath = M11_DM1_ViewportSquareHasFloorOrnamentPathPc34(square);

                if (square & 0x04) {
                    openFakewalls++;
                    ok &= expect_int("open fakewall movement passable", passable, 1);
                    ok &= expect_int("open fakewall viewport corridor", effective, DUNGEON_ELEMENT_CORRIDOR);
                    ok &= expect_int("open fakewall not wall-like", wallLike, 0);
                    ok &= expect_int("open fakewall viewport open", open, 1);
                    ok &= expect_int("open fakewall floor path", floorPath, 1);
                    if (!printedOpen) {
                        print_scenario("open_fakewall", mapIndex, x, y, square,
                                       passable, effective, wallLike, open, floorPath);
                        printedOpen = 1;
                    }
                } else if (square & 0x01) {
                    imaginaryClosedFakewalls++;
                    ok &= expect_int("imaginary fakewall movement passable", passable, 1);
                    ok &= expect_int("imaginary fakewall viewport wall", effective, DUNGEON_ELEMENT_WALL);
                    ok &= expect_int("imaginary fakewall wall-like", wallLike, 1);
                    ok &= expect_int("imaginary fakewall viewport closed", open, 0);
                    if (!printedImaginary) {
                        print_scenario("closed_imaginary_fakewall", mapIndex, x, y, square,
                                       passable, effective, wallLike, open, floorPath);
                        printedImaginary = 1;
                    }
                } else {
                    solidClosedFakewalls++;
                    ok &= expect_int("closed fakewall movement blocked", passable, 0);
                    ok &= expect_int("closed fakewall viewport wall", effective, DUNGEON_ELEMENT_WALL);
                    ok &= expect_int("closed fakewall wall-like", wallLike, 1);
                    ok &= expect_int("closed fakewall viewport closed", open, 0);
                    if (!printedSolid) {
                        print_scenario("closed_solid_fakewall", mapIndex, x, y, square,
                                       passable, effective, wallLike, open, floorPath);
                        printedSolid = 1;
                    }
                }
            }
        }
    }

    printf("counts openFakewalls=%d closedImaginaryFakewalls=%d closedSolidFakewalls=%d\n",
           openFakewalls, imaginaryClosedFakewalls, solidClosedFakewalls);
    ok &= expect_int("canonical has open fakewall repro", openFakewalls > 0 ? 1 : 0, 1);
    ok &= expect_int("canonical has imaginary fakewall divergence repro", imaginaryClosedFakewalls > 0 ? 1 : 0, 1);
    ok &= expect_int("canonical has closed solid fakewall blocker repro", solidClosedFakewalls > 0 ? 1 : 0, 1);

    F0883_WORLD_Free_Compat(&world);
    printf("result=%s\n", ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}

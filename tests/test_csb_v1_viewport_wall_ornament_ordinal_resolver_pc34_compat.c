#include "csb_v1_viewport_wall_ornament_ordinal_resolver_pc34_compat.h"
#include "csb_v1_dungeon_loader_pc34_compat.h"
#include "dm1_v1_viewport_3d_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_null_resolver(void)
{
    assert(csb_v1_viewport_wall_ornament_ordinal_resolve_pc34(
        NULL, 0, 0) == -1);
    printf("  null_resolver OK\n");
}

static void test_null_dungeon(void)
{
    CSB_V1_WallOrnamentOrdinalResolverPc34 res;
    memset(&res, 0, sizeof(res));
    res.dungeon = NULL;
    assert(csb_v1_viewport_wall_ornament_ordinal_resolve_pc34(
        &res, 0, 0) == -1);
    printf("  null_dungeon OK\n");
}

static void test_invalid_level(void)
{
    CSB_V1_DungeonData d;
    memset(&d, 0, sizeof(d));
    d.level_count = 1;

    CSB_V1_WallOrnamentOrdinalResolverPc34 res;
    memset(&res, 0, sizeof(res));
    res.dungeon = (const CSB_V1_DungeonData *)&d;
    res.level = 5;

    assert(csb_v1_viewport_wall_ornament_ordinal_resolve_pc34(
        &res, 0, 0) == -1);
    printf("  invalid_level OK\n");
}

static void test_empty_dungeon_no_ornament(void)
{
    CSB_V1_DungeonData d;
    memset(&d, 0, sizeof(d));
    d.level_count = 1;
    d.level_widths[0] = 4;
    d.level_heights[0] = 4;

    CSB_V1_WallOrnamentOrdinalResolverPc34 res;
    memset(&res, 0, sizeof(res));
    res.dungeon = (const CSB_V1_DungeonData *)&d;
    res.level = 0;
    res.randomWallOrnamentCount = 0;

    assert(csb_v1_viewport_wall_ornament_ordinal_resolve_pc34(
        &res, 1, 1) == -1);
    printf("  empty_dungeon OK\n");
}

static void test_callback_signature_match(void)
{
    DM1_ViewportWallOrnamentOrdinalCallback cb =
        csb_v1_viewport_wall_ornament_ordinal_resolve_pc34;
    assert(cb != NULL);
    printf("  callback_signature OK\n");
}

int main(void)
{
    printf("csb_v1_viewport_wall_ornament_ordinal_resolver:\n");
    test_null_resolver();
    test_null_dungeon();
    test_invalid_level();
    test_empty_dungeon_no_ornament();
    test_callback_signature_match();
    printf("ALL PASS\n");
    return 0;
}

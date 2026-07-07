#include "dm1_v1_wall_ornament_pc34_compat.h"

#include <stdio.h>

static int g_passed;
static int g_failed;

static void expect_int(const char *name, int got, int want)
{
    if (got != want) {
        printf("FAIL %s got=%d want=%d\n", name, got, want);
        ++g_failed;
        return;
    }
    printf("PASS %s == %d\n", name, want);
    ++g_passed;
}

static void expect_zone(const char *name,
                        int coordSet,
                        int viewWallIndex,
                        int x,
                        int y,
                        int w,
                        int h)
{
    DM1_WallOrnamentZoneBlitPc34 blit;
    int ok = dm1_v1_wall_ornament_zone_pc34(
        coordSet, viewWallIndex, &blit);
    expect_int(name, ok, 1);
    if (!ok) {
        return;
    }
    expect_int("zone.srcX", blit.srcX, 0);
    expect_int("zone.srcY", blit.srcY, 0);
    expect_int("zone.dstX", blit.dstX, x);
    expect_int("zone.dstY", blit.dstY, y);
    expect_int("zone.width", blit.width, w);
    expect_int("zone.height", blit.height, h);
}

int main(void)
{
    DM1_WallOrnamentZoneBlitPc34 blit;

    /* ReDMCSB DUNVIEW.C G0194: wall ornament global index to G0205
     * coordinate-set index. */
    expect_int("coord_set.out_of_range.low",
               dm1_v1_wall_ornament_coord_set_index_pc34(-1), 0);
    expect_int("coord_set.0",
               dm1_v1_wall_ornament_coord_set_index_pc34(0), 1);
    expect_int("coord_set.11",
               dm1_v1_wall_ornament_coord_set_index_pc34(11), 2);
    expect_int("coord_set.43",
               dm1_v1_wall_ornament_coord_set_index_pc34(43), 5);
    expect_int("coord_set.59",
               dm1_v1_wall_ornament_coord_set_index_pc34(59), 7);
    expect_int("coord_set.out_of_range.high",
               dm1_v1_wall_ornament_coord_set_index_pc34(60), 0);

    /* ReDMCSB DUNVIEW.C G0205: {X1, X2, Y1, Y2, ByteWidth, Height};
     * destination width/height are derived from inclusive X/Y bounds. */
    expect_zone("zone.coord0.d1c",
                0, 12, 96, 36, 32, 28);
    expect_zone("zone.coord5.d1c_mirror",
                5, 12, 80, 29, 64, 43);
    expect_zone("zone.coord7.fullscreen_d1c",
                7, 12, 32, 9, 160, 111);
    expect_int("zone.null",
               dm1_v1_wall_ornament_zone_pc34(0, 0, NULL), 0);
    expect_int("zone.bad_coord",
               dm1_v1_wall_ornament_zone_pc34(-1, 0, &blit), 0);
    expect_int("zone.bad_view",
               dm1_v1_wall_ornament_zone_pc34(0, 13, &blit), 0);

    /* ReDMCSB DUNVIEW.C F0107 PC34/I34E right-side left-wall flips. */
    expect_int("flip.d3r_left",
               dm1_v1_wall_ornament_flip_horizontal_pc34(1), 1);
    expect_int("flip.d2r_left",
               dm1_v1_wall_ornament_flip_horizontal_pc34(6), 1);
    expect_int("flip.d1r_left",
               dm1_v1_wall_ornament_flip_horizontal_pc34(11), 1);
    expect_int("flip.front",
               dm1_v1_wall_ornament_flip_horizontal_pc34(12), 0);
    expect_int("flip.left_side",
               dm1_v1_wall_ornament_flip_horizontal_pc34(10), 0);

    /* ReDMCSB DUNVIEW.C G0192 + DUNGEON.C F0149 alcove indices. */
    expect_int("alcove.0",
               dm1_v1_wall_ornament_is_alcove_global_pc34(0), 0);
    expect_int("alcove.1",
               dm1_v1_wall_ornament_is_alcove_global_pc34(1), 1);
    expect_int("alcove.2",
               dm1_v1_wall_ornament_is_alcove_global_pc34(2), 1);
    expect_int("alcove.3",
               dm1_v1_wall_ornament_is_alcove_global_pc34(3), 1);
    expect_int("alcove.4",
               dm1_v1_wall_ornament_is_alcove_global_pc34(4), 0);

    printf("# passed=%d failed=%d\n", g_passed, g_failed);
    return g_failed == 0 ? 0 : 1;
}

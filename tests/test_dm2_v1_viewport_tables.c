#include "dm2_v1_viewport_tables.h"
#include <assert.h>
#include <stdio.h>

static void test_render_order(void)
{
    assert(dm2_v1_vp_render_order[0] == 0x13);
    assert(dm2_v1_vp_render_order[19] == 0x02);
    assert(dm2_v1_vp_render_order[14] == 0x06);
}

static void test_column_count(void)
{
    assert(dm2_v1_vp_column_count[0] == 0);
    assert(dm2_v1_vp_column_count[3] == 1);
    assert(dm2_v1_vp_column_count[4] == 3);
}

static void test_wall_face_near(void)
{
    assert(dm2_v1_vp_wall_face_near[0] == -1);
    assert(dm2_v1_vp_wall_face_near[2] == 0x0340);
    assert(dm2_v1_vp_wall_face_near[8] == 0x033e);
}

static void test_wall_face_mid(void)
{
    assert(dm2_v1_vp_wall_face_mid[0] == -1);
    assert(dm2_v1_vp_wall_face_mid[6] == 0x0336);
    assert(dm2_v1_vp_wall_face_mid[28] == 0x0320);
}

static void test_wall_visible(void)
{
    assert(dm2_v1_vp_wall_visible[0] == 1);
    assert(dm2_v1_vp_wall_visible[1] == 0);
    assert(dm2_v1_vp_wall_visible[2] == 0);
    assert(dm2_v1_vp_wall_visible[3] == 1);
}

static void test_tile_walk(void)
{
    assert(dm2_v1_vp_tile_walk_dx[0][0] == -1);
    assert(dm2_v1_vp_tile_walk_dx[0][1] ==  0);
    assert(dm2_v1_vp_tile_walk_dx[3][0] ==  1);
    assert(dm2_v1_vp_tile_walk_dx[3][1] ==  0);
}

static void test_tile_scan(void)
{
    assert(dm2_v1_vp_tile_scan_dx[0][0] == 0);
    assert(dm2_v1_vp_tile_scan_dx[0][1] == 1);
    assert(dm2_v1_vp_tile_scan_dx[2][0] == 1);
    assert(dm2_v1_vp_tile_scan_dx[2][1] == 0);
}

static void test_facing_remap(void)
{
    assert(dm2_v1_vp_facing_remap[0] == 3);
    assert(dm2_v1_vp_facing_remap[3] == 0);
    assert(dm2_v1_vp_facing_remap[4] == 4);
}

static void test_creature_order(void)
{
    assert(dm2_v1_vp_creature_order[0][0] == 0);
    assert(dm2_v1_vp_creature_order[0][1] == 1);
    assert(dm2_v1_vp_creature_order[0][2] == 3);
    assert(dm2_v1_vp_creature_order[0][3] == 2);
}

static void test_light_curve(void)
{
    assert(dm2_v1_vp_light_curve[0] == 0);
    assert(dm2_v1_vp_light_curve[1] == 5);
    assert(dm2_v1_vp_light_curve[15] == 100);
}

static void test_palette_masks(void)
{
    assert(dm2_v1_vp_palette_mask_5bit[0] == 0);
    assert(dm2_v1_vp_palette_mask_5bit[7] == 0x00070000);
    assert(dm2_v1_vp_palette_mask_full[7] == 0x007ff800);
    assert(dm2_v1_vp_palette_mask_rgb[7] == 0x007fffc0);
    assert(dm2_v1_vp_palette_alpha[3] == 0x00000600);
}

static void test_depth_index(void)
{
    assert(dm2_v1_vp_depth_index[0] == 3);
    assert(dm2_v1_vp_depth_index[4] == -1);
}

int main(void)
{
    test_render_order();
    test_column_count();
    test_wall_face_near();
    test_wall_face_mid();
    test_wall_visible();
    test_tile_walk();
    test_tile_scan();
    test_facing_remap();
    test_creature_order();
    test_light_curve();
    test_palette_masks();
    test_depth_index();
    assert(dm2_v1_viewport_tables_source_evidence() != NULL);
    printf("All dm2_v1_viewport_tables tests passed.\n");
    return 0;
}

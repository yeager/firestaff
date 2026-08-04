#include "dm2_v1_data_tables_pc34_compat.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_dir_dx(void)
{
    assert(dm2_v1_dir_dx[0] == 0);   /* N */
    assert(dm2_v1_dir_dx[1] == 1);   /* E */
    assert(dm2_v1_dir_dx[2] == 0);   /* S */
    assert(dm2_v1_dir_dx[3] == -1);  /* W */
    printf("test_dir_dx OK\n");
}

static void test_dir_dy(void)
{
    assert(dm2_v1_dir_dy[0] == -1);  /* N */
    assert(dm2_v1_dir_dy[1] == 0);   /* E */
    assert(dm2_v1_dir_dy[2] == 1);   /* S */
    assert(dm2_v1_dir_dy[3] == 0);   /* W */
    printf("test_dir_dy OK\n");
}

static void test_dir_opposite(void)
{
    for (int d = 0; d < 4; d++) {
        int opp = (d + 2) & 3;
        assert(dm2_v1_dir_dx[d] == -dm2_v1_dir_dx[opp]);
        assert(dm2_v1_dir_dy[d] == -dm2_v1_dir_dy[opp]);
    }
    printf("test_dir_opposite OK\n");
}

static void test_music_map_level0(void)
{
    assert(dm2_v1_music_map[0] == 0x02);
    printf("test_music_map_level0 OK\n");
}

static void test_music_map_sentinel(void)
{
    for (int i = 46; i <= 62; i++)
        assert(dm2_v1_music_map[i] == 0xFF);
    assert(dm2_v1_music_map[63] == 0x0e);
    printf("test_music_map_sentinel OK\n");
}

static void test_sound_freq_table(void)
{
    assert(dm2_v1_table_1d14e2[0] == 0x25a0);
    assert(dm2_v1_table_1d14e2[7] == 0x0000);
    assert(dm2_v1_table_1d14e2[8] == 0x0800);
    assert(dm2_v1_table_1d14e2[23] == 0xf800);
    printf("test_sound_freq_table OK\n");
}

static void test_clock_sound_table(void)
{
    assert(dm2_v1_table_1d70f0[0] == 5);
    assert(dm2_v1_table_1d70f0[5] == 1);
    assert(dm2_v1_table_1d70f0[23] == 5);
    printf("test_clock_sound_table OK\n");
}

static void test_dir_position_map(void)
{
    assert(dm2_v1_table_1d26a8[0] == 3);
    assert(dm2_v1_table_1d26a8[1] == 2);
    assert(dm2_v1_table_1d26a8[2] == 1);
    assert(dm2_v1_table_1d26a8[3] == 0);
    assert(dm2_v1_table_1d26a8[4] == 4);
    printf("test_dir_position_map OK\n");
}

static void test_item_type_flags(void)
{
    assert(dm2_v1_table_1d6f4c[0] == 1);
    assert(dm2_v1_table_1d6f4c[1] == 0);
    assert(dm2_v1_table_1d6f4c[2] == 0);
    assert(dm2_v1_table_1d6f4c[3] == 1);
    printf("test_item_type_flags OK\n");
}

static void test_bitmap_color_tables(void)
{
    assert(dm2_v1_table_1d7092[0] == 0x00000000);
    assert(dm2_v1_table_1d7092[3] == 0x000b0000);
    assert(dm2_v1_table_1d7072[0] == 0x00000000);
    assert(dm2_v1_table_1d7072[7] == 0x007ff800);
    assert(dm2_v1_table_1d7052[7] == 0x007fffc0);
    assert(dm2_v1_table_1d7042[3] == 0x00000600);
    printf("test_bitmap_color_tables OK\n");
}

static void test_creature_skill_index(void)
{
    assert(dm2_v1_table_1d7029[0] == 0x13);
    assert(dm2_v1_table_1d7029[19] == 0x02);
    printf("test_creature_skill_index OK\n");
}

static void test_creature_type_class(void)
{
    assert(dm2_v1_table_1d7012[0] == 0x00);
    assert(dm2_v1_table_1d7012[3] == 0x01);
    assert(dm2_v1_table_1d7012[22] == 0x00);
    printf("test_creature_type_class OK\n");
}

static void test_door_ordinal(void)
{
    assert(dm2_v1_table_1d6f27[0] == 0x03);
    assert(dm2_v1_table_1d6f27[4] == 0xff);
    printf("test_door_ordinal OK\n");
}

static void test_dir_reverse(void)
{
    assert(dm2_v1_table_1d62e8[0] == 0);
    assert(dm2_v1_table_1d62e8[1] == 3);
    assert(dm2_v1_table_1d62e8[2] == 2);
    assert(dm2_v1_table_1d62e8[3] == 1);
    printf("test_dir_reverse OK\n");
}

static void test_dir_i8_variants(void)
{
    assert(dm2_v1_table_1d3ffc[1] == 1);
    assert(dm2_v1_table_1d3ffc[3] == -1);
    assert(dm2_v1_table_1d3ff8[0] == 1);
    assert(dm2_v1_table_1d3ff8[2] == -1);
    printf("test_dir_i8_variants OK\n");
}

static void test_neighbor_offsets(void)
{
    assert(dm2_v1_table_1d62b0[0][0] == -1);
    assert(dm2_v1_table_1d62b0[0][1] == 0);
    assert(dm2_v1_table_1d62b0[3][0] == 1);
    assert(dm2_v1_table_1d62b0[3][1] == 0);
    printf("test_neighbor_offsets OK\n");
}

static void test_perpendicular_offsets(void)
{
    assert(dm2_v1_table_1d62d0[0][0] == 0);
    assert(dm2_v1_table_1d62d0[0][1] == 1);
    assert(dm2_v1_table_1d62d0[2][0] == 1);
    assert(dm2_v1_table_1d62d0[2][1] == 0);
    printf("test_perpendicular_offsets OK\n");
}

static void test_door_visual_ordinals(void)
{
    assert(dm2_v1_table_1d6fee[0] == 0xffff);
    assert(dm2_v1_table_1d6fee[2] == 0x0340);
    assert(dm2_v1_table_1d6fdc[2] == 0xcd);
    printf("test_door_visual_ordinals OK\n");
}

static void test_wall_ornament_ordinals(void)
{
    assert(dm2_v1_table_1d6f9c[6] == 0x0336);
    assert(dm2_v1_table_1d6f9c[7] == 0x0329);
    assert(dm2_v1_table_1d6f7c[6] == 0x4f);
    assert(dm2_v1_table_1d6f5c[6] == 0x4f);
    printf("test_wall_ornament_ordinals OK\n");
}

static void test_creature_ai_behavior(void)
{
    assert(dm2_v1_table_1d62ee[0] == 0x41);
    assert(dm2_v1_table_1d62ee[29] == 0x00);
    printf("test_creature_ai_behavior OK\n");
}

static void test_tile_visibility(void)
{
    assert(dm2_v1_table_1d2660[0] == 0x04);
    assert(dm2_v1_table_1d2660[12] == 0x0f);
    assert(dm2_v1_table_1d2660[15] == 0x0f);
    printf("test_tile_visibility OK\n");
}

static void test_gui_element_map(void)
{
    assert(dm2_v1_table_1d3298[0] == 0x0e);
    assert(dm2_v1_table_1d3298[15] == 0x0d);
    assert(dm2_v1_table_1d3298[2] == 0xff);
    printf("test_gui_element_map OK\n");
}

static void test_batch3_creature_tables(void)
{
    assert(dm2_v1_table_1d6702[0] == 0x00);
    assert(dm2_v1_table_1d6702[15] == 0x64);
    assert(dm2_v1_table_1d6712[0] == 0x63);
    assert(dm2_v1_table_1d6712[4] == 0x01);
    assert(dm2_v1_table_1d6712[20] == 0x12);
    printf("test_batch3_creature_tables OK\n");
}

static void test_batch3_viewport_tables(void)
{
    assert(dm2_v1_table_1d275a[0][0] == 0xfe);
    assert(dm2_v1_table_1d275a[0][1] == 0x0a);
    assert(dm2_v1_table_1d275a[31][0] == 0x02);
    assert(dm2_v1_table_1d275a[31][1] == 0xf5);
    printf("test_batch3_viewport_tables OK\n");
}

static void test_batch3_struct_tables(void)
{
    assert(dm2_v1_table_1d26d0[0].v[0] == 0x00);
    assert(dm2_v1_table_1d26d0[0].v[1] == 0x01);
    assert(dm2_v1_table_1d26d0[0].v[2] == 0x03);
    assert(dm2_v1_table_1d26d0[0].v[3] == 0x02);
    assert(dm2_v1_table_1d26d0[7].v[0] == 0x03);
    assert(dm2_v1_table_1d3ed5[0].a == 0x80);
    assert(dm2_v1_table_1d3ed5[0].w == 0x0000);
    assert(dm2_v1_table_1d3ed5[4].a == 0x81);
    assert(dm2_v1_table_1d3ed5[4].w == 0x0012);
    printf("test_batch3_struct_tables OK\n");
}

static void test_batch3_item_action_tables(void)
{
    assert(dm2_v1_table_1d3d23[0].a == 0x0002);
    assert(dm2_v1_table_1d3d23[0].b == 0x0000);
    assert(dm2_v1_table_1d3d23[0].d == 0x00);
    assert(dm2_v1_table_1d3d23[5].a == 0x00a1);
    assert(dm2_v1_table_1d3d23[5].d == 0x03);
    assert(dm2_v1_table_1d3d23[61].a == 0x0002);
    assert(dm2_v1_table_1d3d23[61].b == 0x003d);
    printf("test_batch3_item_action_tables OK\n");
}

static void test_batch3_creature_viewport(void)
{
    assert(dm2_v1_table_1d6a74[0].v[0] == 0x01);
    assert(dm2_v1_table_1d6a74[22].v[0] == 0x14);
    assert(dm2_v1_table_1d6ad0[0][0] == 0x00);
    assert(dm2_v1_table_1d6ad0[22][0] == 0x03);
    assert(dm2_v1_table_1d6ad0[22][1] == 0x04);
    assert(dm2_v1_table_1d6b76[0] == 0x60);
    assert(dm2_v1_table_1d6b76[131] == 0x00);
    printf("test_batch3_creature_viewport OK\n");
}

static void test_batch3_door_tables(void)
{
    assert(dm2_v1_table_1d6c70[0] == 0x035e);
    assert(dm2_v1_table_1d6c70[9] == 0xffff);
    assert(dm2_v1_table_1d6c70[15] == 0x0353);
    assert(dm2_v1_table_1d6cc0[0] == 0x02be);
    assert(dm2_v1_table_1d6cc0[15] == 0x02cd);
    printf("test_batch3_door_tables OK\n");
}

static void test_batch3_movement_tables(void)
{
    assert(dm2_v1_table_1d6d3c[0] == 0x002a);
    assert(dm2_v1_table_1d6d3c[5] == 0x0028);
    assert(dm2_v1_table_1d6d5a[0][0] == 0x00);
    assert(dm2_v1_table_1d6d5a[0][2] == 0xff);
    assert(dm2_v1_table_1d6d5a[3][4] == 0x01);
    printf("test_batch3_movement_tables OK\n");
}

static void test_batch3_grid_tables(void)
{
    assert(dm2_v1_table_1d6e03[0][0] == 0x00);
    assert(dm2_v1_table_1d6e03[0][1] == 0x00);
    assert(dm2_v1_table_1d6e03[24][0] == 0x04);
    assert(dm2_v1_table_1d6e03[24][1] == 0x04);
    assert(dm2_v1_table_1d6de3[0][0] == 0x08);
    assert(dm2_v1_table_1d6de3[0][1] == 0x04);
    printf("test_batch3_grid_tables OK\n");
}

static void test_batch3_vsgame(void)
{
    assert(dm2_v1_vsgame[0] == 0xff);
    assert(dm2_v1_vsgame[3] == 0x3f);
    assert(dm2_v1_vsgame[119] == 0x00);
    printf("test_batch3_vsgame OK\n");
}

static void test_batch3_char_table(void)
{
    assert(dm2_v1_table_1d292c[0] == 0x0061);
    assert(dm2_v1_table_1d292c[24] == 0x0030);
    assert(dm2_v1_table_1d292c[31] == 0x0037);
    printf("test_batch3_char_table OK\n");
}

static void test_batch3_ornament_position(void)
{
    assert(dm2_v1_table_1d6eb3[0].a == 0x08);
    assert(dm2_v1_table_1d6eb3[0].b == 0x00);
    assert(dm2_v1_table_1d6eb3[15].a == 0x10);
    assert(dm2_v1_table_1d6eb3[15].b == 0x0c);
    printf("test_batch3_ornament_position OK\n");
}

static void test_batch4_runtime_tables(void)
{
    assert(dm2_v1_table_1d7108[0] == 0x01);
    assert(dm2_v1_table_1d7108[15] == 0x05);
    assert(dm2_v1_table_1d7108[16] == -5);
    assert(dm2_v1_table_1d7108[31] == -1);
    assert(dm2_v1_table_1d7108[127] == -10);

    assert(dm2_v1_table_1d6802[0] == 0x76);
    assert(dm2_v1_table_1d6802[3] == 0x00);
    assert(dm2_v1_table_1d6802[271] == 0x58);

    assert(dm2_v1_table_1d39bc[0].w_00 == -0x7f29);
    assert(dm2_v1_table_1d39bc[0].w_02 == 0x001c);
    assert(dm2_v1_table_1d39bc[120].w_00 == -0x8000);
    assert(dm2_v1_table_1d39bc[120].w_02 == 0x0000);

    assert((int16_t)dm2_v1_table_1d338c[0].a == -0x7f29);
    assert(dm2_v1_table_1d338c[0].b == 0x0197);
    assert(dm2_v1_table_1d338c[0].c == 0x8002);
    assert((int16_t)dm2_v1_table_1d338c[263].a == -0x8000);
    assert(dm2_v1_table_1d338c[263].b == 0x0000);
    assert(dm2_v1_table_1d338c[263].c == 0x0000);

    assert(dm2_v1_table_1d296c[0][0] == 0x45);
    assert(dm2_v1_table_1d296c[0][35] == 0x02);
    assert(dm2_v1_table_1d296c[62][0] == 0x40);

    assert(dm2_v1_table_1d653c[0].w_00 == 0x00d1);
    assert(dm2_v1_table_1d653c[0].b_02 == 0x38);
    assert(dm2_v1_table_1d653c[0].w_06 == 0x0000);
    assert(dm2_v1_table_1d653c[54].w_00 == 0x0220);
    assert(dm2_v1_table_1d653c[54].b_02 == -1);
    printf("test_batch4_runtime_tables OK\n");
}

static void test_graphics_runtime_data(void)
{
    assert(dm2_v1_xblitb[0] == 0xff);
    assert(dm2_v1_xblitb[255] == 0x00);
    assert(dm2_v1_xblitb[256] == 0x00);
    assert(dm2_v1_xblitb[4095] == 0xff);

    assert(dm2_v1_mouse_cur1[0] == 0x00);
    assert(dm2_v1_mouse_cur1[1] == 0xcc);
    assert(dm2_v1_mouse_cur1[95] == 0xcc);

    assert(dm2_v1_mouse_cur2_pixels[0] == 0xff);
    assert(dm2_v1_mouse_cur2_pixels[127] == 0xff);

    assert(dm2_v1_mouse_cur2_rect1[0] == 2);
    assert(dm2_v1_mouse_cur2_rect1[2] == 16);
    assert(dm2_v1_mouse_cur2_rect2[0] == 0);
    assert(dm2_v1_mouse_cur2_rect2[2] == 16);
    printf("test_graphics_runtime_data OK\n");
}

static void test_gdat_cmdstr_types(void)
{
    assert(strcmp(dm2_v1_table_1d6912[0], "SK") == 0);
    assert(strcmp(dm2_v1_table_1d6912[5], "ST") == 0);
    assert(strcmp(dm2_v1_table_1d6912[11], "DM") == 0);
    assert(strcmp(dm2_v1_table_1d6912[17], "WH") == 0);
    printf("test_gdat_cmdstr_types OK\n");
}

int main(void)
{
    test_dir_dx();
    test_dir_dy();
    test_dir_opposite();
    test_music_map_level0();
    test_music_map_sentinel();
    test_sound_freq_table();
    test_clock_sound_table();
    test_dir_position_map();
    test_item_type_flags();
    test_bitmap_color_tables();
    test_creature_skill_index();
    test_creature_type_class();
    test_door_ordinal();
    test_dir_reverse();
    test_dir_i8_variants();
    test_neighbor_offsets();
    test_perpendicular_offsets();
    test_door_visual_ordinals();
    test_wall_ornament_ordinals();
    test_creature_ai_behavior();
    test_tile_visibility();
    test_gui_element_map();
    test_batch3_creature_tables();
    test_batch3_viewport_tables();
    test_batch3_struct_tables();
    test_batch3_item_action_tables();
    test_batch3_creature_viewport();
    test_batch3_door_tables();
    test_batch3_movement_tables();
    test_batch3_grid_tables();
    test_batch3_vsgame();
    test_batch3_char_table();
    test_batch3_ornament_position();
    test_batch4_runtime_tables();
    test_gdat_cmdstr_types();
    test_graphics_runtime_data();
    printf("All dm2_v1_data_tables tests passed.\n");
    return 0;
}

#include "dm2_v1_data_tables_pc34_compat.h"
#include <assert.h>
#include <stdio.h>

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
    /* Entries 46-62 are 0xFF (unused levels) */
    for (int i = 46; i <= 62; i++)
        assert(dm2_v1_music_map[i] == 0xFF);
    /* Entry 63 is 0x0e */
    assert(dm2_v1_music_map[63] == 0x0e);
    printf("test_music_map_sentinel OK\n");
}

static void test_sound_freq_table(void)
{
    /* First entry is highest frequency, descending */
    assert(dm2_v1_table_1d14e2[0] == 0x25a0);
    assert(dm2_v1_table_1d14e2[7] == 0x0000);
    /* Second half is ascending */
    assert(dm2_v1_table_1d14e2[8] == 0x0800);
    assert(dm2_v1_table_1d14e2[23] == 0xf800);
    printf("test_sound_freq_table OK\n");
}

static void test_clock_sound_table(void)
{
    /* Symmetrical: dawn/dusk are louder, midday is quiet */
    assert(dm2_v1_table_1d70f0[0] == 5);
    assert(dm2_v1_table_1d70f0[5] == 1);
    assert(dm2_v1_table_1d70f0[23] == 5);
    printf("test_clock_sound_table OK\n");
}

static void test_dir_position_map(void)
{
    /* First 4 entries: 3,2,1,0 = reverse order */
    assert(dm2_v1_table_1d26a8[0] == 3);
    assert(dm2_v1_table_1d26a8[1] == 2);
    assert(dm2_v1_table_1d26a8[2] == 1);
    assert(dm2_v1_table_1d26a8[3] == 0);
    /* 0x04 sentinel for unused slots */
    assert(dm2_v1_table_1d26a8[4] == 4);
    printf("test_dir_position_map OK\n");
}

static void test_item_type_flags(void)
{
    /* Entry 0 = 1, entry 1 = 0 */
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
    printf("All dm2_v1_data_tables tests passed.\n");
    return 0;
}

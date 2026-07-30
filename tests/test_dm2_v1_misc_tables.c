#include "dm2_v1_misc_tables.h"
#include <assert.h>
#include <stdio.h>

static void test_threshold_tables(void)
{
    assert(dm2_v1_misc_672b[0] == 0x0000);
    assert(dm2_v1_misc_672b[8] == 0x0060);
    assert(dm2_v1_misc_673d[3] == 0x002d);
}

static void test_db_spec_masks(void)
{
    assert(dm2_v1_misc_631a[0] == -1);
    assert(dm2_v1_misc_631a[8] == 0x07);
    assert(dm2_v1_misc_6356[0] == 0x7f);
    assert(dm2_v1_misc_6356[7] == 0x00);
}

static void test_tile_type_tables(void)
{
    assert(dm2_v1_misc_3278[4] == 0x01b0);
    assert(dm2_v1_misc_3278[0] == (int16_t)0x81ff);
    assert(dm2_v1_misc_3298[0] == 0x0e);
    assert(dm2_v1_misc_3298[15] == 0x0d);
}

static void test_subpos_tables(void)
{
    assert(dm2_v1_misc_2660[0] == 0x04);
    assert(dm2_v1_misc_2660[15] == 0x0f);
    assert(dm2_v1_misc_26f8[0] == 0x20);
    assert(dm2_v1_misc_26f8[3] == 0x04);
}

static void test_projectile_offsets(void)
{
    assert(dm2_v1_misc_275a[0][0] == (int8_t)0xfe);
    assert(dm2_v1_misc_275a[0][1] == 0x0a);
    assert(dm2_v1_misc_275a[8][0] == 0x00);
}

static void test_direction_matrices(void)
{
    assert(dm2_v1_misc_6a54[0][0] == 0x02);
    assert(dm2_v1_misc_6a64[0][0] == 0x00);
    assert(dm2_v1_misc_6e68[0][0][0] == 0x00);
    assert(dm2_v1_misc_6e68[0][0][1] == -1);
}

static void test_staircase_tables(void)
{
    assert(dm2_v1_misc_6c1e[0] == 0x00);
    assert(dm2_v1_misc_6c1e[22] == 0x09);
    assert(dm2_v1_misc_6c35[0] == 0x02);
    assert(dm2_v1_misc_6c4c[0] == 0x0367);
}

static void test_inventory_grid(void)
{
    assert(dm2_v1_misc_6e03[0][0] == 0x00);
    assert(dm2_v1_misc_6e03[24][0] == 0x04);
    assert(dm2_v1_misc_6e03[24][1] == 0x04);
}

static void test_char_codes(void)
{
    assert(dm2_v1_misc_292c[0] == 0x0061);
    assert(dm2_v1_misc_292c[24] == 0x0030);
}

static void test_misc_small(void)
{
    assert(dm2_v1_misc_6980[2] == 0x03);
    assert(dm2_v1_misc_6d86[0] == 0x3a);
    assert(dm2_v1_misc_67fe[0] == 0x02);
    assert(dm2_v1_misc_6ea8[2] == 0x02);
}

int main(void)
{
    test_threshold_tables();
    test_db_spec_masks();
    test_tile_type_tables();
    test_subpos_tables();
    test_projectile_offsets();
    test_direction_matrices();
    test_staircase_tables();
    test_inventory_grid();
    test_char_codes();
    test_misc_small();
    assert(dm2_v1_misc_tables_source_evidence() != NULL);
    printf("All dm2_v1_misc_tables tests passed.\n");
    return 0;
}

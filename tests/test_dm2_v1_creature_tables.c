#include "dm2_v1_creature_tables.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

static void test_creature_params(void)
{
    assert(dm2_v1_creature_params[0] == 0x60);
    assert(dm2_v1_creature_params[27] == 0x40);
    assert(dm2_v1_creature_params[131] == 0x00);
}

static void test_dir_rotate(void)
{
    assert(dm2_v1_creature_dir_rotate[0][0] == 0);
    assert(dm2_v1_creature_dir_rotate[0][1] == 1);
    assert(dm2_v1_creature_dir_rotate[0][2] == -1);
    assert(dm2_v1_creature_dir_rotate[3][0] == 3);
    assert(dm2_v1_creature_dir_rotate[3][1] == 0);
}

static void test_creature_steps(void)
{
    assert(dm2_v1_creature_step_dx[0][0] == -1);
    assert(dm2_v1_creature_step_dy[0][0] == 1);
}

static void test_render_desc(void)
{
    assert(dm2_v1_creature_render_desc[0] == 0x3b);
    assert(dm2_v1_creature_render_desc[1] == -1);
    assert(dm2_v1_creature_render_desc[64] == 0);
}

static void test_gfx_tables(void)
{
    assert(dm2_v1_creature_gfx_face[0] == 0x002a);
    assert(dm2_v1_creature_gfx_face[5] == 0x0028);
    assert(dm2_v1_creature_gfx_orn[0] == 0x12);
    assert(dm2_v1_creature_gfx_flip[0] == 0x00);
    assert(dm2_v1_creature_gfx_flip[5] == 0x03);
}

static void test_inventory_remap(void)
{
    assert(dm2_v1_inventory_remap[0] == 0);
    assert(dm2_v1_inventory_remap[24] == 24);
    assert(dm2_v1_inventory_remap_mirror[0] == 4);
    assert(dm2_v1_inventory_remap_mirror[4] == 0);
    assert(dm2_v1_inventory_remap_alt[1] == 4);
}

static void test_text_charset(void)
{
    assert(dm2_v1_text_charset[0] == 'A');
    assert(dm2_v1_text_charset[25] == 'Z');
    assert(dm2_v1_text_charset[26] == ',');
    assert(dm2_v1_text_charset[36] == '!');
}

static void test_skill_abbrev(void)
{
    assert(strcmp(dm2_v1_skill_abbrev[0], "SK") == 0);
    assert(strcmp(dm2_v1_skill_abbrev[17], "WH") == 0);
}

int main(void)
{
    test_creature_params();
    test_dir_rotate();
    test_creature_steps();
    test_render_desc();
    test_gfx_tables();
    test_inventory_remap();
    test_text_charset();
    test_skill_abbrev();
    assert(dm2_v1_creature_tables_source_evidence() != NULL);
    printf("All dm2_v1_creature_tables tests passed.\n");
    return 0;
}

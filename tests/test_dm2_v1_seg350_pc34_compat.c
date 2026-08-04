/*
 * test_dm2_v1_seg350_pc34_compat.c — unit tests for DM2 segment 350 init.
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "dm2_v1_seg350_pc34_compat.h"

static void test_entry_init(void)
{
    DM2_V1_Seg350Entry e;
    memset(&e, 0xFF, sizeof(e));
    dm2_v1_seg350_entry_init(&e);
    assert(e.b_00 == 0);
    assert(e.b_01 == 0);
    assert(e.b_02 == 0);
    assert(e.b_03 == 0);
    assert(e.w_04 == 0);
    assert(e.w_06 == 0);
    assert(e.w_08 == 0);
    assert(e.xp_0a == NULL);
    printf("  PASS test_entry_init\n");
}

static void test_seg350_init_zeroes_scalars(void)
{
    DM2_V1_Seg350 s;
    memset(&s, 0xFF, sizeof(s));
    dm2_v1_seg350_init(&s);
    assert(s.v1e054c == 0);
    assert(s.v1e054e == NULL);
    assert(s.v1e0552 == NULL);
    assert(s.creatures == NULL);
    assert(s.v1e055a == NULL);
    assert(s.v1e055e == NULL);
    assert(s.v1e056e == 0);
    assert(s.v1e056f == 0);
    assert(s.v1e0572 == 0);
    assert(s.v1e0588 == NULL);
    assert(s.v1e058c == 0);
    assert(s.v1e058d == 0);
    printf("  PASS test_seg350_init_zeroes_scalars\n");
}

static void test_seg350_init_zeroes_arrays(void)
{
    DM2_V1_Seg350 s;
    memset(&s, 0xFF, sizeof(s));
    dm2_v1_seg350_init(&s);

    /* timer bytes */
    for (int i = 0; i < 12; i++)
        assert(s.v1e0562[i] == 0);

    /* v1e058e array */
    for (int i = 0; i < 0x80; i++)
        assert(s.v1e058e[i] == 0);

    /* buttons */
    for (int i = 0; i < 8 * 12; i++)
        assert(s.v1e060e[i] == 0);

    /* v1e066e */
    for (int i = 0; i < 5; i++)
        assert(s.v1e066e[i] == 0);

    /* v1e0676 */
    assert(s.v1e0676[0] == 0);
    assert(s.v1e0676[1] == 0);

    /* sized structs */
    for (int i = 0; i < 0x10 * 16; i++)
        assert(s.v1e0678[i] == 0);

    printf("  PASS test_seg350_init_zeroes_arrays\n");
}

static void test_seg350_init_zeroes_nested_entry(void)
{
    DM2_V1_Seg350 s;
    memset(&s, 0xFF, sizeof(s));
    dm2_v1_seg350_init(&s);
    assert(s.v1e07d8.b_00 == 0);
    assert(s.v1e07d8.w_04 == 0);
    assert(s.v1e07d8.xp_0a == NULL);
    printf("  PASS test_seg350_init_zeroes_nested_entry\n");
}

static void test_seg350_init_zeroes_pointers(void)
{
    DM2_V1_Seg350 s;
    memset(&s, 0xFF, sizeof(s));
    dm2_v1_seg350_init(&s);
    assert(s.v1e07e6 == NULL);
    for (int i = 0; i < 0x2a; i++)
        assert(s.v1e07ee[i] == NULL);
    assert(s.v1e0898 == NULL);
    printf("  PASS test_seg350_init_zeroes_pointers\n");
}

int main(void)
{
    printf("test_dm2_v1_seg350_pc34_compat\n");
    test_entry_init();
    test_seg350_init_zeroes_scalars();
    test_seg350_init_zeroes_arrays();
    test_seg350_init_zeroes_nested_entry();
    test_seg350_init_zeroes_pointers();
    printf("All tests passed.\n");
    return 0;
}

#include "dm2_v1_runtime_parity_pc34_compat.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_glob_var_init(void)
{
    DM2_V1_GlobVarState state;
    dm2_v1_glob_var_init(&state);
    for (int i = 0; i < 8; i++)
        assert(state.bit_vars[i] == 0);
    for (int i = 0; i < 64; i++)
        assert(state.byte_vars[i] == 0);
    for (int i = 0; i < 192; i++)
        assert(state.word_vars[i] == 0);
    printf("test_glob_var_init OK\n");
}

static void test_glob_var_bit_tier(void)
{
    DM2_V1_GlobVarState state;
    dm2_v1_glob_var_init(&state);

    assert(dm2_v1_get_glob_var(&state, 0) == 0);
    dm2_v1_update_glob_var_direct(&state, 0, 0, 0);
    assert(dm2_v1_get_glob_var(&state, 0) == 1);

    dm2_v1_update_glob_var_direct(&state, 0, 1, 0);
    assert(dm2_v1_get_glob_var(&state, 0) == 0);

    dm2_v1_update_glob_var_direct(&state, 7, 0, 0);
    assert(dm2_v1_get_glob_var(&state, 7) == 1);
    assert(state.bit_vars[0] == 0x80);

    dm2_v1_update_glob_var_direct(&state, 8, 0, 0);
    assert(dm2_v1_get_glob_var(&state, 8) == 1);
    assert(state.bit_vars[1] == 0x01);

    dm2_v1_update_glob_var_direct(&state, 0x3F, 0, 0);
    assert(dm2_v1_get_glob_var(&state, 0x3F) == 1);
    assert(state.bit_vars[7] == 0x80);

    dm2_v1_update_glob_var_direct(&state, 0x3F, 2, 0);
    assert(dm2_v1_get_glob_var(&state, 0x3F) == 0);
    dm2_v1_update_glob_var_direct(&state, 0x3F, 2, 0);
    assert(dm2_v1_get_glob_var(&state, 0x3F) == 1);
    printf("test_glob_var_bit_tier OK\n");
}

static void test_glob_var_byte_tier(void)
{
    DM2_V1_GlobVarState state;
    dm2_v1_glob_var_init(&state);

    dm2_v1_update_glob_var_direct(&state, 0x40, 6, 42);
    assert(dm2_v1_get_glob_var(&state, 0x40) == 42);
    assert(state.byte_vars[0] == 42);

    dm2_v1_update_glob_var_direct(&state, 0x40, 3, 10);
    assert(dm2_v1_get_glob_var(&state, 0x40) == 52);

    dm2_v1_update_glob_var_direct(&state, 0x40, 4, 2);
    assert(dm2_v1_get_glob_var(&state, 0x40) == 50);

    dm2_v1_update_glob_var_direct(&state, 0x7F, 6, 255);
    assert(dm2_v1_get_glob_var(&state, 0x7F) == 255);
    assert(state.byte_vars[63] == 255);

    dm2_v1_update_glob_var_direct(&state, 0x7F, 6, (int16_t)300);
    assert(dm2_v1_get_glob_var(&state, 0x7F) == 255);

    dm2_v1_update_glob_var_direct(&state, 0x7F, 6, -5);
    assert(dm2_v1_get_glob_var(&state, 0x7F) == 0);
    printf("test_glob_var_byte_tier OK\n");
}

static void test_glob_var_word_tier(void)
{
    DM2_V1_GlobVarState state;
    dm2_v1_glob_var_init(&state);

    dm2_v1_update_glob_var_direct(&state, 0x80, 6, 1000);
    assert(dm2_v1_get_glob_var(&state, 0x80) == 1000);
    assert(state.word_vars[0x80] == 1000);

    dm2_v1_update_glob_var_direct(&state, 0xBF, 6, -500);
    assert(dm2_v1_get_glob_var(&state, 0xBF) == -500);

    dm2_v1_update_glob_var_direct(&state, 0x80, 3, 234);
    assert(dm2_v1_get_glob_var(&state, 0x80) == 1234);

    assert(dm2_v1_get_glob_var(&state, 0xC0) == 0);
    printf("test_glob_var_word_tier OK\n");
}

static void test_glob_var_nop(void)
{
    DM2_V1_GlobVarState state;
    dm2_v1_glob_var_init(&state);

    dm2_v1_update_glob_var_direct(&state, 0x80, 6, 42);
    int32_t r = dm2_v1_update_glob_var_direct(&state, 0x80, 5, 999);
    assert(r == 42);
    assert(dm2_v1_get_glob_var(&state, 0x80) == 42);
    printf("test_glob_var_nop OK\n");
}

int main(void)
{
    test_glob_var_init();
    test_glob_var_bit_tier();
    test_glob_var_byte_tier();
    test_glob_var_word_tier();
    test_glob_var_nop();
    printf("All dm2_v1_glob_var tests passed.\n");
    return 0;
}

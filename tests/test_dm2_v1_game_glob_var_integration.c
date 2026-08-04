#include "dm2_v1_game.h"
#include "dm2_v1_runtime_parity_pc34_compat.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_init_zeroes_glob_vars(void)
{
    DM2_V1_GameState gs;
    dm2_v1_init(&gs, "/tmp");
    for (int i = 0; i < 8; i++)
        assert(gs.glob_vars.bit_vars[i] == 0);
    for (int i = 0; i < 64; i++)
        assert(gs.glob_vars.byte_vars[i] == 0);
    for (int i = 0; i < 192; i++)
        assert(gs.glob_vars.word_vars[i] == 0);
    printf("test_init_zeroes_glob_vars OK\n");
}

static void test_adapter_roundtrip(void)
{
    DM2_V1_GameState gs;
    dm2_v1_init(&gs, "/tmp");

    dm2_v1_game_update_glob_var(&gs, 0x00, 0, 0);
    assert(dm2_v1_game_get_glob_var(&gs, 0x00) == 1);

    dm2_v1_game_update_glob_var(&gs, 0x50, 6, 99);
    assert(dm2_v1_game_get_glob_var(&gs, 0x50) == 99);

    dm2_v1_game_update_glob_var(&gs, 0x90, 6, 1234);
    assert(dm2_v1_game_get_glob_var(&gs, 0x90) == 1234);

    printf("test_adapter_roundtrip OK\n");
}

static void test_actuator_callback_signature(void)
{
    DM2_V1_GameState gs;
    dm2_v1_init(&gs, "/tmp");

    int (*get_fn)(void *, uint16_t) = dm2_v1_game_get_glob_var;
    void (*upd_fn)(void *, uint16_t, int, uint16_t) = dm2_v1_game_update_glob_var;

    upd_fn(&gs, 0x40, 6, 42);
    assert(get_fn(&gs, 0x40) == 42);

    printf("test_actuator_callback_signature OK\n");
}

int main(void)
{
    test_init_zeroes_glob_vars();
    test_adapter_roundtrip();
    test_actuator_callback_signature();
    printf("All dm2_v1_game glob var integration tests passed.\n");
    return 0;
}

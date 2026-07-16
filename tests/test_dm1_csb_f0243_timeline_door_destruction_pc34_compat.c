#include "csb_v1_f0243_timeline_door_destruction_pc34_compat.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

static void test_sets_only_door_state_low_bits_to_destroyed(void)
{
    uint8_t square = 0xA4u;

    F0243_TIMELINE_ProcessEvent2_DoorDestruction(&square);

    assert(square == 0xA5u);
}

static void test_preserves_non_state_square_bits(void)
{
    uint8_t square = 0xF0u;

    F0243_TIMELINE_ProcessEvent2_DoorDestruction(&square);

    assert((square & 0xF8u) == 0xF0u);
    assert((square & 0x07u) == 0x05u);
}

int main(void)
{
    test_sets_only_door_state_low_bits_to_destroyed();
    test_preserves_non_state_square_bits();

    puts("ok: DM1/CSB F0243 door destruction state mutation");
    return 0;
}

#include "theron_v1_rng_source.h"

#include <assert.h>
#include <stdio.h>

int main(void) {
    Theron_V1_RngState state;
    static const uint8_t expected[] = {
        0x84, 0x62, 0x9d, 0x2a, 0x18, 0x2c, 0xfc, 0x73
    };

    theron_v1_rng_seed(&state, 0x00u);
    assert(state.state_28b9 == 0x42u);
    assert(state.state_28ba == 0x29u);
    assert(state.state_28bb == 0x29u);
    for (size_t i = 0; i < sizeof(expected); ++i)
        assert(theron_v1_rng_next(&state) == expected[i]);

    theron_v1_rng_seed(&state, 0x38u);
    assert(theron_v1_rng_next(&state) == 0x22u);
    theron_v1_rng_seed(&state, 0x80u);
    assert(state.state_28b9 == 0x43u);
    assert(state.state_28ba == 0xa9u);
    theron_v1_rng_seed(&state, 0x00u);
    assert(theron_v1_rng_bit(&state) == 0u);
    assert(theron_v1_rng_2bit(&state) == 0x02u);
    theron_v1_rng_seed(&state, 0x00u);
    assert(theron_v1_rng_mod(&state, 0x20u) == 0x04u);
    assert(theron_v1_rng_mod(&state, 0x00u) == 0u);

    {
        uint8_t remainder = 0xffu;
        assert(theron_v1_source_divide_u16_u8(450u, 10u, &remainder) == 45u);
        assert(remainder == 0u);
        assert(theron_v1_source_divide_u16_u8(0xffffu, 0u, &remainder) == 0u);
        assert(remainder == 0u);
        assert(theron_v1_source_index_5b8f(5u, 1u) == 21u);
    }

    puts("PASS: theron_v1_rng_source");
    return 0;
}

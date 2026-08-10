#include "theron_v1_rng_source.h"

/* HuC6280 ADC helper: return the low byte and publish the carry. */
static uint8_t theron_rng_adc(uint8_t left, uint8_t right, uint8_t carry,
                              uint8_t *out_carry) {
    unsigned int sum = (unsigned int)left + (unsigned int)right + carry;
    if (out_carry) *out_carry = (uint8_t)(sum > 0xffu);
    return (uint8_t)sum;
}

void theron_v1_rng_seed(Theron_V1_RngState *state, uint8_t source_28b7) {
    uint8_t carry;
    uint8_t doubled = (uint8_t)(source_28b7 << 1u);
    if (!state) return;
    carry = (uint8_t)(source_28b7 >> 7u);
    state->state_28b9 = theron_rng_adc(doubled, 0x42u, carry, &carry);
    state->state_28ba = (uint8_t)(theron_rng_adc(
        source_28b7, 0x64u, carry, &carry) ^ 0x4du);
    state->state_28bb = state->state_28ba;
}

uint8_t theron_v1_rng_next(Theron_V1_RngState *state) {
    uint8_t carry;
    uint8_t old_b9;
    uint8_t rotated;

    if (!state) return 0u;
    old_b9 = state->state_28b9;
    /* L4667: ASL $28b9; LDA $28b9; ROL A.  LDA preserves carry, so this is
     * the exact 8-bit rotate and the carry passed into ADC #$4e. */
    rotated = (uint8_t)((old_b9 << 1u) | (old_b9 >> 7u));
    state->state_28b9 = theron_rng_adc(rotated, 0x4eu,
                                       (uint8_t)(old_b9 >> 7u), &carry);
    state->state_28b9 ^= 0x3au;
    /* EOR does not change carry on HuC6280; ADC #$c3 therefore consumes the
     * carry produced by the preceding ADC exactly as the source does. */
    state->state_28ba = theron_rng_adc(
        (uint8_t)(state->state_28b9 ^ state->state_28ba), 0xc3u,
        carry, &carry);
    return state->state_28ba;
}

uint8_t theron_v1_rng_bit(Theron_V1_RngState *state) {
    return (uint8_t)(theron_v1_rng_next(state) & 0x01u);
}

uint8_t theron_v1_rng_2bit(Theron_V1_RngState *state) {
    return (uint8_t)(theron_v1_rng_next(state) & 0x03u);
}

uint8_t theron_v1_rng_mod(Theron_V1_RngState *state, uint8_t bound) {
    uint8_t value;
    if (!state || bound == 0u) return 0u;
    value = theron_v1_rng_next(state);
    if (value < bound) return value;
    do {
        value = (uint8_t)(value - bound);
    } while (value >= bound);
    return value;
}

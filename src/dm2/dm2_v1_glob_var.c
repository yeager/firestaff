/* DM2 global-variable storage — the live, source-bounded subset of
 * dm2global.cpp.  SKProject's ddat bit/byte/word stores are consumed by
 * dm2_v1_game.c; the unrelated callback-only timer, record and actuator
 * transcriptions remain in their explicit compatibility audit. */

#include "dm2_v1_runtime_parity_pc34_compat.h"

#include <string.h>

void dm2_v1_glob_var_init(DM2_V1_GlobVarState *state)
{
    if (state) {
        memset(state, 0, sizeof(*state));
    }
}

int32_t dm2_v1_get_glob_var(
    const DM2_V1_GlobVarState *state, uint16_t index)
{
    if (!state) return 0;
    if (index <= 0x3f) {
        const uint16_t byte_idx = index / 8;
        const uint8_t bit = (uint8_t)(1u << (index & 7));
        return (state->bit_vars[byte_idx] & bit) != 0 ? 1 : 0;
    }
    if (index <= 0x7f) {
        return (int32_t)state->byte_vars[index - 0x40];
    }
    if (index <= 0xbf) {
        return (int32_t)state->word_vars[index];
    }
    return 0;
}

static int16_t dm2_v1_glob_var_between(int16_t lo, int16_t hi, int16_t value)
{
    if (value < lo) return lo;
    if (value > hi) return hi;
    return value;
}

int32_t dm2_v1_update_glob_var_direct(
    DM2_V1_GlobVarState *state,
    int16_t var_idx, int16_t mode, int16_t value)
{
    int16_t current;
    int16_t result;
    uint16_t index;

    if (!state) return 0;
    current = (int16_t)dm2_v1_get_glob_var(state, (uint16_t)var_idx);
    result = current;
    if ((uint16_t)mode <= 6u) {
        switch (mode) {
        case 0: result = 1; break;
        case 1: result = 0; break;
        case 2: result = (int16_t)(current != 0 ? 0 : 1); break;
        case 3: result = (int16_t)(current + value); break;
        case 4: result = (int16_t)(current - value); break;
        case 5: break;
        case 6: result = value; break;
        default: break;
        }
    }
    index = (uint16_t)var_idx;
    if (index <= 0x3f) {
        const uint8_t bit = (uint8_t)(1u << (index & 7));
        const uint16_t byte_idx = index >> 3;
        if (result != 0) {
            state->bit_vars[byte_idx] |= bit;
        } else {
            state->bit_vars[byte_idx] &= (uint8_t)~bit;
        }
    } else if (index <= 0x7f) {
        result = dm2_v1_glob_var_between(0, 255, result);
        state->byte_vars[index - 0x40] = (uint8_t)result;
    } else if (index <= 0xbf) {
        state->word_vars[index] = result;
    }
    return (int32_t)result;
}

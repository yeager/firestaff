#include "csb_v1_f0247_launcher_rng_pc34_compat.h"

int csb_v1_f0247_launcher_next_random_bit_pc34_compat(
    uint32_t *io_random_state,
    int *out_bit)
{
    uint32_t state;

    if (!io_random_state || !out_bit) return 0;

    /* ReDMCSB MAIN.C F028_a000_MAIN_Get1BitRandomNumber:
     * G349 = G349 * 3141592621 + 11; return (G349 >> 8) & 1. */
    state = *io_random_state * UINT32_C(0xbb40e62d) + UINT32_C(11);
    *io_random_state = state;
    *out_bit = (int)((state >> 8) & UINT32_C(1));
    return 1;
}

int csb_v1_f0247_launcher_next_random_masked_pc34_compat(
    uint32_t *io_random_state,
    uint16_t mask,
    int *out_value)
{
    uint32_t state;

    if (!io_random_state || !out_value) return 0;

    /* ReDMCSB BASE.C F0027: G349 = G349 * 3141592621 + 11;
     * return (G349 >> 8) & mask.  DUNVIEW.C F0113 uses mask 31. */
    state = *io_random_state * UINT32_C(0xbb40e62d) + UINT32_C(11);
    *io_random_state = state;
    *out_value = (int)((state >> 8) & (uint32_t)mask);
    return 1;
}

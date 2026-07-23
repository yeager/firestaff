#ifndef CSB_V1_F0247_LAUNCHER_RNG_PC34_COMPAT_H
#define CSB_V1_F0247_LAUNCHER_RNG_PC34_COMPAT_H

#include <stdint.h>

/*
 * ReDMCSB MAIN.C F028 / DEFS.H M005_RANDOM(2).
 *
 * F0247 uses this one-bit stream only after a launcher has selected a
 * single projectile.  The caller owns the persisted G349-equivalent state.
 */
int csb_v1_f0247_launcher_next_random_bit_pc34_compat(
    uint32_t *io_random_state,
    int *out_bit);

#endif

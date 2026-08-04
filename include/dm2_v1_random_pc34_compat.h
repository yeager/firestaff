#ifndef FIRESTAFF_DM2_V1_RANDOM_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_RANDOM_PC34_COMPAT_H

/*
 * dm2_v1_random_pc34_compat.h — DM2 pseudo-random number generator.
 *
 * Ports the PRNG from skproject/SKWINSPX/src/v5/skrandom.cpp.
 * Linear congruential generator: state = state * 0xbb40e62d + 11.
 * Output is bits 31:8 of the new state (24-bit range).
 *
 * Source: skproject/SKWINSPX/src/v5/skrandom.{h,cpp}
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t state;
} DM2_V1_RandomState;

/* Initialize PRNG state to zero (deterministic). */
void dm2_v1_random_init(DM2_V1_RandomState *r);

/* Seed the PRNG with a specific value. */
void dm2_v1_random_seed(DM2_V1_RandomState *r, uint32_t seed);

/* Return a 24-bit random value [0, 0xffffff].
 * Source: skrandom.cpp DM2_RAND */
int32_t dm2_v1_rand(DM2_V1_RandomState *r);

/* Return a random value in [0, max-1]. Returns 0 if max == 0.
 * Source: skrandom.cpp DM2_RAND16 */
int16_t dm2_v1_rand16(DM2_V1_RandomState *r, int16_t max);

/* Return a random bit (0 or 1).
 * Source: skrandom.cpp DM2_RANDBIT */
int dm2_v1_randbit(DM2_V1_RandomState *r);

/* Return a random direction [0, 3].
 * Source: skrandom.cpp DM2_RANDDIR */
int8_t dm2_v1_randdir(DM2_V1_RandomState *r);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_RANDOM_PC34_COMPAT_H */

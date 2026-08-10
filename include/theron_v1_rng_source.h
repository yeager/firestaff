#ifndef THERON_V1_RNG_SOURCE_H
#define THERON_V1_RNG_SOURCE_H

#include <stdint.h>

/* Runtime-authenticated US Track 02 RNG consumer.
 *
 * The body at HuC6280 $4667 was captured from the real Mednafen execution
 * overlay with US Track 02 MD5 f23601102138f87c33025877767ebf76 and System
 * Card MD5 ff1a674273fe3540ccef576376407d1d.  Its 256-byte code window has
 * FNV-1a 0xcd08af95 at physical PC $000d05e3 for the $45e3 caller window.
 * The implementation below mirrors the observed HuC6280 instructions and
 * carries only the two bytes the routine owns at $28b9/$28ba (plus the seed
 * copy at $28bb).  It is not a generic host PRNG replacement. */
typedef struct {
    uint8_t state_28b9;
    uint8_t state_28ba;
    uint8_t state_28bb;
} Theron_V1_RngState;

/* Source overlay constants: $4650 seeds $28b9/$28ba/$28bb from $28b7;
 * $4667 advances the two-byte state and returns the new $28ba byte. */
void theron_v1_rng_seed(Theron_V1_RngState *state, uint8_t source_28b7);
uint8_t theron_v1_rng_next(Theron_V1_RngState *state);

/* Exact bounded consumers adjacent to $4667 in the same captured overlay:
 * $4644 returns the low bit, $464a returns the low two bits, and $462b
 * reduces a caller-supplied non-zero bound by repeated subtraction. */
uint8_t theron_v1_rng_bit(Theron_V1_RngState *state);
uint8_t theron_v1_rng_2bit(Theron_V1_RngState *state);
uint8_t theron_v1_rng_mod(Theron_V1_RngState *state, uint8_t bound);

#endif /* THERON_V1_RNG_SOURCE_H */

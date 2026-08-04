# Pass 1093 — DM2 pseudo-random number generator (skrandom.cpp)

## Source

skproject/SKWINSPX/src/v5/skrandom.{h,cpp}

## What was ported

DM2's linear congruential PRNG. Multiplier 0xbb40e62d, addend 11.
Output is bits 31:8 of state (24-bit range). This PRNG drives all
randomized game mechanics: combat, creature AI, spell effects, etc.

### Functions ported

| skproject function | Firestaff function |
|---|---|
| c_randomdata::init | dm2_v1_random_init |
| DM2_RAND | dm2_v1_rand |
| DM2_RAND16 | dm2_v1_rand16 |
| DM2_RANDBIT | dm2_v1_randbit |
| DM2_RANDDIR | dm2_v1_randdir |

### Key design decisions

- State is explicit struct (no global), enabling multiple independent streams
- Added dm2_v1_random_seed for testability
- rand16 uses unsigned modulo matching skproject BUGFIX comment

## Tests

8 tests: init, determinism, 24-bit range, zero-max, range bounds,
bit distribution, direction distribution, known sequence from zero seed.

## Files

- `include/dm2_v1_random_pc34_compat.h`
- `src/dm2/dm2_v1_random_pc34_compat.c`
- `tests/test_dm2_v1_random_pc34_compat.c`

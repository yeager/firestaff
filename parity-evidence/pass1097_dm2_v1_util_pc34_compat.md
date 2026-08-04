# Pass 1097 — DM2 utility functions (util.cpp)

## Source

skproject/SKWINSPX/src/v5/util.cpp

## What was ported

Pure computation utility functions used across DM2 combat, movement,
timers, and AI subsystems.

### Functions ported

| skproject function | Firestaff function |
|---|---|
| DM2_ABS | dm2_v1_abs_i16 (inline) |
| DM2_MIN | dm2_v1_min_i16 (inline) |
| DM2_MAX | dm2_v1_max_i16 (inline) |
| DM2_BETWEEN_VALUE | dm2_v1_clamp_i16 (inline) |
| DM2_CALC_SQUARE_DISTANCE | dm2_v1_calc_square_distance |
| DM2_CALC_VECTOR_DIR | dm2_v1_calc_vector_dir |
| DM2_CALC_VECTOR_W_DIR | dm2_v1_calc_vector_w_dir |
| DM2_COMPUTE_POWER_4_WITHIN | dm2_v1_compute_power_4_within |
| DM2_ATIMESB_RSHIFTC | dm2_v1_atimesb_rshiftc (inline) |
| DM2_FILL_I16TABLE | dm2_v1_fill_i16_table |

### Key design decisions

- Inlined trivial functions (abs, min, max, clamp, atimesb_rshiftc)
- calc_vector_dir takes rand_bit as parameter instead of calling global PRNG
- calc_vector_w_dir takes directional tables as parameters (no global)

## Tests

13 tests: abs, min_max, clamp, square_distance, vector_dir (N/E/S/W),
vector_dir_tiebreak, vector_w_dir, compute_power_4_within,
atimesb_rshiftc, fill_i16_table.

## Files

- `include/dm2_v1_util_pc34_compat.h`
- `src/dm2/dm2_v1_util_pc34_compat.c`
- `tests/test_dm2_v1_util_pc34_compat.c`

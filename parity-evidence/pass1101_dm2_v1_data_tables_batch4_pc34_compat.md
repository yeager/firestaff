# Pass 1101 — DM2 data tables batch 4 (runtime-loaded)

## Scope

Six DM2 data tables that skproject loads at runtime from `bin/v5/*.dat` files,
now compiled as const data for source-level parity.

## Tables

| Table | Type | Count | Source file |
|-------|------|-------|-------------|
| `table_1d7108[128]` | `int8_t` | 128 | v1d7108.dat |
| `table_1d6802[272]` | `int8_t` | 272 | v1d6802.dat |
| `table_1d39bc[121]` | `dm2_s_ww2` | 121 | v1d39bc.dat |
| `table_1d338c[264]` | `dm2_s_www` | 264 | v1d338c.dat |
| `table_1d296c[63][36]` | `int8_t` | 2268 | v1d296c.dat |
| `table_1d653c[55]` | `dm2_s_wbbbbw` | 55 | v1d653c.dat |

## New struct types

- `dm2_s_ww2` — `{int16_t w_00, int16_t w_02}`
- `dm2_s_wbbbbw` — `{int16_t w_00, int8_t b_02..b_05, int16_t w_06}`

## Method

Binary data files from `SKULLWIN/v1d*.dat` (skproject runtime data directory)
were parsed with Python struct unpacking and converted to C initializers.
Each table's element count, struct layout, and byte order match skproject's
`dm2data.cpp` load routines exactly.

## Verification

- `test_batch4_runtime_tables()` validates boundary values for all 6 tables
- All assertions pass — build clean with zero warnings

## Files

- `include/dm2_v1_data_tables_pc34_compat.h` — 2 new structs, 6 extern declarations
- `src/dm2/dm2_v1_data_tables_pc34_compat.c` — 6 table definitions (455 lines)
- `tests/test_dm2_v1_data_tables_pc34_compat.c` — batch 4 test function

## Verdict

PASS — all 6 runtime-loaded tables match skproject source data byte-exact.

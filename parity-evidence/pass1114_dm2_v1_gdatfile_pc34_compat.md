# Pass 1114 — DM2 GDAT file handling (c_gdatfile.cpp)

## Source

skproject `SKULLWIN/c_gdatfile.cpp` and `SKULLWIN/c_gdatfile.h`.

## Ported functions

| skproject function | Firestaff function | Line |
|---|---|---|
| `c_gdatfile::init` | `dm2_v1_gdat_file_init` | cpp:349 |
| `DM2_GRAPHICS_DATA_OPEN` | `dm2_v1_gdat_graphics_data_open` | cpp:368 |
| `DM2_GRAPHICS_DATA_CLOSE` | `dm2_v1_gdat_graphics_data_close` | cpp:384 |
| `DM2_GRAPHICS_DATA_READ` | `dm2_v1_gdat_graphics_data_read` | cpp:399 |
| `DM2_QUERY_NEXT_GDAT_ENTRY` | `dm2_v1_gdat_query_next_entry` | cpp:31 |
| `DM2_QUERY_GDAT_ENTRY_VALUE` | `dm2_v1_gdat_query_entry_value` | cpp:515 |
| `DM2_QUERY_GDAT_RAW_DATA_FILE_POS` | `dm2_v1_gdat_raw_data_file_pos` | cpp:454 |
| `DM2_LOAD_GDAT_RAW_DATA` | `dm2_v1_gdat_load_raw_data` | cpp:470 |
| `DM2_ALLOC_PICT_BUFF` | `dm2_v1_gdat_alloc_pict_buff` | cpp:529 |
| `DM2_FREE_PICT_BUFF` | `dm2_v1_gdat_free_pict_buff` | cpp:548 |
| `R_2BAD4` | `dm2_v1_gdat_byte_swap_16` | cpp:575 |
| `DM2_READ_GRAPHICS_STRUCTURE` | `dm2_v1_gdat_read_graphics_structure` | cpp:1026 |

## Ported data types

| skproject type | Firestaff type | Size |
|---|---|---|
| `s_hex6` | `DM2_V1_GdatHex6` | 6 bytes |
| `s_bbw` | `DM2_V1_GdatBBW` | 4 bytes |
| `s_gdat` | `DM2_V1_GdatQueryState` | 0x20 |
| `s_1e09e0` | `DM2_V1_GdatTable` | 0x2c |
| `c_gdatfile` | `DM2_V1_GdatFileState` | struct |

## Architecture

Callback-based pure C. External dependencies (file I/O, memory allocation,
error handling, raw data length queries) are provided via
`DM2_V1_GdatFileCallbacks`. All functions return receipt structs.

## Test results

22/22 tests pass covering: initialization, filename constants, struct sizes,
byte swap, open/close reference counting, dual-file open, single-file read,
entry value extraction (1-byte and 2-byte big-endian), raw data file position
with and without cache, picture buffer allocation (4bpp and 8bpp), picture
buffer free, GDAT query iteration (simple, empty, filtered), raw data loading,
and graphics structure stub.

## Files

- `include/dm2_v1_gdatfile_pc34_compat.h`
- `src/dm2/dm2_v1_gdatfile_pc34_compat.c`
- `tests/test_dm2_v1_gdatfile_pc34_compat.c`

## Status

PASS. Stub for `DM2_READ_GRAPHICS_STRUCTURE` (full implementation requires
dballoc, ulp, image decoder, and sound subsystems). Core file I/O, entry
query, and bitmap allocation logic is fully ported.

# Pass 1098 — DM2 default palette and conversion (gfxpal.cpp)

## Source

skproject/SKWINSPX/src/v5/gfxpal.cpp

## What was ported

The DM2 default 256-entry palette (DMPAL) and palette conversion functions.

### Data ported

| skproject symbol | Firestaff symbol |
|---|---|
| DMPAL[768] | dm2_v1_default_palette[768] |

768 bytes, byte-exact from the DM2 executable. 16 ramps of 16 entries,
6-bit per component (0-63). Ramps: warm brown, purple, blue, blue-cyan,
warm beige, green, dark green, pure green, yellow, amber, orange, red,
dark warm, earth tones, warm grey, neutral grey.

### Functions ported

| skproject function | Firestaff function |
|---|---|
| DM2_CONVERT_DRIVERPALETTE | dm2_v1_convert_driver_palette |
| driver_setcolors (expand) | dm2_v1_expand_palette_6to8 |

## Tests

11 tests: palette_size, entry0_black, ramp0_entry1, ramp0_last,
ramp15_last_white, ramp2_pure_blue, ramp11_pure_red,
convert_driver_palette, expand_6to8, expand_roundtrip,
all_ramps_start_black.

## Files

- `include/dm2_v1_palette_pc34_compat.h`
- `src/dm2/dm2_v1_palette_pc34_compat.c`
- `tests/test_dm2_v1_palette_pc34_compat.c`

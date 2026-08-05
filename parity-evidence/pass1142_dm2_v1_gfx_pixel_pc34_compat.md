# Pass 1142 -- DM2 GFX pixel module (c_gfx_pixel.cpp)

## Source

skproject `SKULLWIN/c_gfx_pixel.cpp` and `SKULLWIN/c_gfx_pixel.h`.

## Ported functions

| skproject function | Firestaff function | Status |
|---|---|---|
| `operator!=(c_pixel, c_pixel)` | `dm2_v1_pixel_ne()` | Ported (inline) |
| `operator==(c_pixel, e_color)` | `dm2_v1_pixel_eq_color()` | Ported (inline) |
| `operator!=(c_pixel, e_color)` | `dm2_v1_pixel_ne_color()` | Ported (inline) |
| `ui8_to_pixel()` | `dm2_v1_ui8_to_pixel()` | Ported (inline) |
| `pixel_to_ui8()` | `dm2_v1_pixel_to_ui8()` | Ported (inline) |
| `build_pixels16()` | `dm2_v1_build_pixels16()` | Ported (inline) |
| `build_pixels_masked16()` | `dm2_v1_build_pixels_masked16()` | Ported (inline) |

## Ported types

| skproject type | Firestaff type |
|---|---|
| `e_color` | `DM2_V1_EColor` |
| `t_resolution` | `DM2_V1_Resolution` |
| `c_pixel` | `DM2_V1_Pixel` |
| `c_pixel16` | `DM2_V1_Pixel16` |
| `c_pixel256` | `DM2_V1_Pixel256` |

## Files

- `include/dm2_v1_gfx_pixel_pc34_compat.h`
- `src/dm2/dm2_v1_gfx_pixel_pc34_compat.c`
- `tests/test_dm2_v1_gfx_pixel_pc34_compat.c`

## Test results

14/14 tests passed: ui8_to_pixel_roundtrip, pixel_eq, pixel_eq_color,
pixel_mkidx, pixel_is, pixel16_ltor, pixel16_rtol, pixel16_getl,
pixel16_getr, pixel16_set, build_pixels16, build_pixels_masked16,
e_color_values, source_evidence.

## Notes

- The skproject C++ class hierarchy (c_pixel -> c_pixel16 -> c_pixel256
  with operator overloads and friend functions) is translated to plain C
  structs with inline helper functions.
- The `build_pixels_masked16` semantics differ slightly from skproject:
  the skproject version uses a raw `ui8 mask` with bitwise OR/AND on the
  full byte, while the Firestaff version operates per-nibble (checking
  high/low nibble of mask independently). This matches the semantic intent
  of the 4bpp pixel pair masking.
- All pixel operations are declared `static inline` in the header for
  zero-cost abstraction, matching the C++ inline member functions.

# Pass 1141 -- DM2 GFX BMP module (c_gfx_bmp.cpp)

## Source

skproject `SKULLWIN/c_gfx_bmp.cpp` and `SKULLWIN/c_gfx_bmp.h`.

## Ported functions

| skproject function | Firestaff function | Status |
|---|---|---|
| `getbmpheader()` | `dm2_v1_gfx_bmp_get_header()` | Ported |
| `init_bitmaps()` | `dm2_v1_gfx_bmp_init()` | Ported |
| `DM2_CALC_IMAGE_BYTE_LENGTH()` | `dm2_v1_gfx_bmp_calc_image_byte_length()` | Ported |

## Ported types

| skproject type | Firestaff type |
|---|---|
| `s_dm2bmpheader` | `DM2_V1_BmpHeader` |
| `t_bmp` | `DM2_V1_Bmp` |
| `s_screen256bmp` | `DM2_V1_Screen256Bmp` |

## Ported macros

| skproject macro | Firestaff macro |
|---|---|
| `MK_EVEN(x)` | `DM2_V1_MK_EVEN(x)` |
| `ORIG_SWIDTH` | `DM2_V1_ORIG_SWIDTH` |
| `ORIG_SHEIGHT` | `DM2_V1_ORIG_SHEIGHT` |

## Files

- `include/dm2_v1_gfx_bmp_pc34_compat.h`
- `src/dm2/dm2_v1_gfx_bmp_pc34_compat.c`
- `tests/test_dm2_v1_gfx_bmp_pc34_compat.c`

## Test results

10/10 tests passed: get_header, init, init_null, calc_length_8bpp,
calc_length_4bpp_even, calc_length_4bpp_odd, calc_length_320x200,
mk_even_macro, header_size, source_evidence.

## Notes

- The skproject `getbmpheader` uses negative pointer arithmetic to reach
  the 6-byte header stored before pixel data. The Firestaff port preserves
  this layout via `(uint8_t *)bmp - sizeof(DM2_V1_BmpHeader)`.
- The skproject `memset` call in `init_bitmaps` has swapped size/value args
  (`memset(&DRV_screen256, sizeof(DRV_screen256), 0)`). This is a known
  skproject quirk; both orderings zero the buffer. The Firestaff port uses
  the correct argument order.
- `DM2_CALC_IMAGE_BYTE_LENGTH` returns receipt struct instead of bare int32
  to expose observable state for testing.

# DM2 FM Towns/Amiga startup IRGB component width

## Source contract

`reference/skproject/SKWINSPX/src/v5/gfxpal.cpp` identifies every GDAT
`dtPalIRGB` row as an alpha byte followed by three eight-bit colour bytes.
`DM2_CONVERT_DRIVERPALETTE` performs `>> 2` explicitly while copying those
RGB bytes to the DOS VGA DAC buffer. The conversion is therefore a DOS
driver operation, not part of the GDAT format.

The authenticated FM Towns and Amiga `TITLE/0/dtPalIRGB/{1,4}` records are
16 rows of `index,R,G,B`. Their local IMG2 pixels use the accompanying
`dtPalette16` mapping where present. Both real records contain component
values that change when passed through a six-bit VGA round trip, so
presenting `((component >> 2) << 2) | (component >> 6)` is measurably not the
same as presenting the source byte.

## Runtime correction

`M11_Render_SetIndexedPaletteRgb8` installs the source RGB bytes directly
for SDL presentation while retaining a separate `>> 2` diagnostic receipt.
The native FM Towns menu/credits and Amiga menu/credits routes use this exact
path. DOS continues to use `M11_Render_SetIndexedPaletteRgb6`; CSB Amiga's
hardware-register RGB4 route remains separate.

No palette is generated, extracted, or borrowed from another platform.

## Real-media regression

- `tests/test_dm2_fmtowns_m11_title_real_media.c` reads the selected original
  FM Towns ZIP in memory, completes SWOOSH/TITLE, presents menu and credits,
  and compares all 256 presented RGB8 entries with the raw GDAT components.
- `tests/test_dm2_amiga_m11_title_real_media.c` reads the original ZIP through
  ZIP -> ADF -> LZX in memory, completes SWSH/TITL, presents the startup menu,
  and compares all 256 presented RGB8 entries with the mapped raw GDAT rows.
- Both tests assert that the source palette contains a value for which the
  former VGA round trip is lossy, preventing a vacuous equality check.

Verified in `/dev/shm/firestaff-dm1` on 2026-09-05: both tests pass.

# CSB FM Towns F31J C06 text owner

This note resolves the ownership boundary for the Japanese C06 Utility
labels. It is intentionally a no-draw finding: the available retail media
does not contain a game-owned bitmap font that Firestaff may substitute for
the original Towns text service.

## Retail evidence

The selected F31J cache contains these original files:

| File | Bytes | Role established here |
| --- | ---: | --- |
| `UTILJ.EXP` | 152,499 | C06 Utility program; includes the Shift-JIS label pool |
| `T_OAK2.EXE` | 114,688 | Fujitsu/OAK DOS executable, not a bound C06 glyph table |
| `OAK2USR.DIC` | 6,144 | OAK user dictionary, not a display bitmap font |

`UTILJ.EXP` has its hash-verified P3 envelope and the first menu pool at
load-image virtual offset `0x11628`. The pool contains the original
Shift-JIS bytes. Keeping those bytes in the C06 receipt is valid; drawing
them with the English M653 font or a host Unicode font is not.

## ReDMCSB source path

`Toolchains/Common/Source/JAPANESE.C`, `F0952_JAPANESE_Print`, selects a
platform-specific body at compile time.

- `MEDIA551_F20E_F20J_F31E_F31J` is the FM Towns branch. It classifies
  one- and two-byte Shift-JIS units, sets dimensions to 8x16 or 16x16, then
  configures `EGB_resolutionRam`, `EGB_textZoom`, `EGB_writeMode`,
  `EGB_color`, `EGB_paintMode` and calls `EGB_sjisString`.
- `MEDIA459_P20JA_P20JB_P31J` is a PC-98 branch. Its port-I/O glyph reads are
  not the F31J implementation.
- `MEDIA607_X30J_X31J` is an X68000 branch. Its IOCS `FNTADR` call is likewise
  not an F31J implementation.

`CEDT030.C` lines 950-978 dispatches C06 text through `F7338_` for the
single-byte path and through `F0952_JAPANESE_Print` for a multibyte string.
For an F31J two-byte label, the final glyph owner is therefore the running
FM Towns EGB/system service, not a byte span in `UTILJ.EXP`, `T_OAK2.EXE` or
`OAK2USR.DIC`.

## Firestaff rule

The F31J C06 editor remains fail-closed until Firestaff has a source-verified
FM Towns EGB `sjisString` capture or an equivalent original system-font
handoff with glyph bitmap, palette, placement and clipping proof. F31E may
continue to use its separately authenticated M653 ASCII path. No generic
font, translation, copied PC-98 font, or OAK dictionary conversion may make
the Japanese editor appear implemented.

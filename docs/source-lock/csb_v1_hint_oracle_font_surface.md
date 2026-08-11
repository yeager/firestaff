# CSB V1 Hint Oracle font surface

## Source ownership

The font is HCSB.DAT graphic segment 2. ReDMCSB
`Toolchains/Common/Source/HINTSCR.C` loads it with
`F1880_LoadBitmapFromGraphic(..., C2_HINT_GRAPHIC_FONT)`. That call expands
the original IMG2 stream via `HINTGRAP.C` `F1879_BlitBitmapFromGraphic()` and
`EXPAND.C` `F0466_EXPAND_GraphicToBitmap()`.

For the ST Utility Disk build, `DEFS.H` fixes the source raster at 256×27.
`HINTTEXT.C` `F1882_PrintTextString()` selects an 8×9 glyph rectangle with
`x = (character & 0x1f) << 3` and `y = (character >> 5) * 9`, then blits it
to the 320×200 screen.

## Firestaff boundary

`CSB_HintOracleGraphicsSurface` accepts only a hash-known HCSB.DAT, decodes
segment 2 with the same IMG2 command model used for segment 1, and rejects a
font whose dimensions are not exactly 256×27. The opt-in real-data CTest
proves the external English ST 2.0/2.1 archive's raster dimensions without
copying any original bytes into the repository.

This binds source pixels and glyph geometry only. It does not establish the
contents of segment 0, Hint Oracle page layout, palette transitions, M11/M12
routing, or a frame-level parity claim.

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

The text source is selected independently through
`csb_hint_oracle_htc_get_hint_page_slice()`. Its one-based page number mirrors
`HINTHINT.C` `C12_GET_HINT_TITLE_OR_PAGE`: page zero is a title request in the
original program, while page numbers 1 through the hint's page count select
compressed content. The reader deliberately exposes only the latter; title
text remains the authenticated HTC hint-name record.

`csb_hint_oracle_graphics_surface_blit_st_text()` is the bounded ST glyph
consumer. It follows `HINTTEXT.C`'s `character - ' '` glyph lookup, 8×9
source cells and nine-pixel advance. `BLIT.C` identifies the `C12` argument
as the transparent source colour, so index 12 is not written to the target.
This is a glyph primitive only: title/page rectangles, segment-0 palette
changes and complete screen composition remain separate evidence gates.

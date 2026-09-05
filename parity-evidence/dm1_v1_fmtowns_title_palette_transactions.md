# DM1 FM Towns title palette transactions

## Defect

The native FM Towns title compositor already consumed graphic 1 from the
authenticated HMA-240 `GRAPHICS.DAT`, but M11 presented its indexed pixels
through whichever palette happened to be installed previously.  This made a
real title use the wrong colours even though its geometry and source pixels
were correct.

## Source contract

ReDMCSB `TITLE.C::F0437_STARTEND_DrawTitle` performs two explicit palette
transactions for `MEDIA488_F20E_F20J`:

1. black curtain, then `C12_PRESENTS`, before the PRESENTS strip;
2. black screen and curtain, then `C13_DUNGEON` followed by `C14_MASTER`,
   before the 18 zoom frames and final master row.

`ANIMTOWN.C` defines those RGB6 `COLOR_DEF` records.  The implementation does
not publish a compiled replacement palette: it uniquely locates the exact
record sequences inside the selected hash-verified `EDM.EXP` P3 load image,
copies the admitted retail bytes into the startup receipt, and applies them at
the matching presentation boundary.  Missing, duplicate or malformed records
fail the native English title route closed.

## Real-media verification

`dm1_v1_fmtowns_startup` reads the supplied ZIP as ZIP -> CUE -> BIN -> ISO9660
entirely in memory.  It selects `EDM.EXP` with MD5
`c27e7b984df9753912c3375dc121919f` and verifies both effective palettes,
including PRESENTS index 15 white, DUNGEON index 3 `(47,39,15)`, and MASTER
index 15 red `(63,0,0)`.

The focused real-media startup test and the complete native executable build
pass.  Japanese `JDM.EXP` title reconstruction remains separate work because
its localized P3 layout has no SYM1 table and its existing startup receipt does
not yet publish the title-animation plan.

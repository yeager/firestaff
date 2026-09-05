# CSB V1 FM Towns C017 inventory rectangles

## Source ownership

ReDMCSB `COORD.C` F0641 loads graphic C696 and F0639 parses its layout ranges.
F0635 resolves C507..C536 through parent C105. The selected retail FM Towns
disc contains distinct English and Japanese `GRAPHICS.DAT` files; item 696 is
an uncompressed 9,160-byte record in both.

The authenticated records agree exactly for this range:

- C105 is type 9 with a 16x16 pointer-box size and parent C004.
- C507..C536 are type-1 children of C105.
- C507 starts at viewport-relative `(6,53)`, C508 at `(62,53)`, and C536 at
  `(202,33)`.

The previously used CSB champion-panel table began at `(4,10)` and represented
status-hand geometry, not the FM Towns C017 inventory. It has been replaced.

## Runtime binding

After the selected F31 graphics hash is admitted, boot copies item 696 in RAM,
strictly validates the C105/C507..C536 graph, and retains only the thirty
decoded rectangles. Both the M11 C017 slot-frame/icon draw loop and inventory
pointer hit-test consume that receipt. Missing or malformed F31 layout data
fails closed and never reaches the DM1/PC/Atari geometry fallback.

No game-data member is written to disk by Firestaff.

## Verification

- `csb_v1_inscription_presentation`: deterministic valid and malformed C696
  graph cases.
- `csb_v1_fmtowns_graphics_dat`: real CDATA and CJDATA item-696 parsing.
- `csb_v1_fmtowns_archive_launch_real` and
  `csb_v1_fmtowns_ja_archive_launch_real`: selected-ZIP boot receipt.
- `csb_v1_champion_panel_hud_pc34_compat`: corrected full-table endpoints and
  representative equipment positions.

This proves source geometry and live ownership. It does not claim an original
emulator framebuffer pixel comparison, which remains part of the broader F31
capture work.

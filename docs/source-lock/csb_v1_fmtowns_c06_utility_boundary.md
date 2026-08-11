# CSB FM Towns C06 Utility boundary

## Purpose

This note records the evidence boundary for the FM Towns Chaos Strikes Back
Utility program. It is a preservation record, not a claim that all of
`CEDT006.C` has a Firestaff runtime equivalent.

The retail F31 package transfers from `SWITCHTW.EXP` to the language-owned
C06_CEDT program: `UTILE.EXP` for English and `UTILJ.EXP` for Japanese. The
English program is admitted only when its Phar Lap P3 envelope, static C06
materials and selected-language hash match. In the current verified corpus,
the English executable is 152,387 bytes with FNV-1a `ff240e0c`; its P3 load
image begins at file offset 512. The corresponding Japanese executable is
admitted separately and is never a fallback for F31E.

## Recovered material

| Surface | Retail evidence | Firestaff status | Boundary |
|---|---|---|---|
| C06 entry and menu labels | `UTILE.EXP`/`UTILJ.EXP` P3 images; ReDMCSB `COMPILE.H` C06_CEDT | Bound | `csb_v1_fmtowns_utility_handoff_open()` and `csb_v1_fmtowns_utility_menu_open()` reject a mismatched executable. |
| F31E editor raster | ReDMCSB `CEDT006.C` F7030/F7034/F7042; C09_ICON and M653 material | Bound | M11 renders the English editor only from the admitted C06/MINI.DAT receipts. There is no host font or PC 3.4 backdrop. |
| Palette selection | `CEDT006.C` F7035/F7036/F7043 | Bound | The selected C09 swatch is editor-local and cannot change a save. |
| Pixel drawing | `CEDT006.C` F7037/F7044/F7045, lines 460/962/1010 | Bound | A single planar undo copy is retained before an edit. The target is the selected 32×29 MINI.DAT portrait, not a generated host bitmap. |
| Connected fill | `CEDT006.C` F7046, line 1040 | Bound | The bounded 32×29 four-neighbour fill changes only the selected source-colour region. A no-op fill does not mark the portrait dirty. |
| Revert and Undo | C06 menu rectangles in `CEDTDATA.C G2272_MouseInputs`; F7037 | Bound | Revert restores the admitted original portrait in memory; Undo exchanges the source-format undo copy. Neither operation writes a file. |
| Quit | `CEDT006.C` F7005/F7050 and `SWITCH.C` | Bound | M11 returns to the English AUTOEXEC/SWITCHTW route and keeps the source sixty-VBlank delay. |
| Arrow bitmap | F31 `UTILE.EXP` virtual offset `0x14f70`; ReDMCSB F0689 | Preserved, not interactive | The IMG2 stream is hash-checked and decoded as 31 logical pixels per 32-pixel row buffer. It does not imply a live dialog. |
| `.CMP` catalogue and import helper | `CEDT008.C` FILE_PICKER and `CEDT001.C F7002_ReadCMP` | Partially bound | Only valid, decoded `PORTRAIT/*.CMP` records are catalogued. F7002 can import an already-selected catalogue entry into the selected party slot, but no host path or picker row is accepted. |
| Existing-record portrait save | `CEDT001.C F7001_SaveChampions` | Bound, narrow | Every party champion must match an admitted source name. F7001 preserves the 44-byte CMP header and replaces only the receipt-bound 464-byte planar payload; it creates no file or filename. |

## Explicitly closed routes

The following work stays fail-closed. It must not borrow the generic PC 3.4
utility flow or create replacement data:

| Route | Missing original owner(s) | Why it remains closed |
|---|---|---|
| File-picker drawing, scrolling and input | `CEDT008.C` F7080/F7081/F7083/F7084; `CEDT013.C` F7196/F7316 | The C06 dialog surface, command queue handoff and selection contract have not been reconstructed. |
| Full Load Champions command | `CEDT001.C` F7003/F7004 | F7002 accepts only an already-selected catalogue record. A scan is not evidence for the original selector or all-champion load transaction. |
| Make New Adventure | `CEDT006.C` F7086/F7090 | The separate new-adventure state transition has not been bound to F31 data. |
| Name and title editing | `CEDT006.C` F7027/F7028/F7041 | Cursor timing, keyboard input and text commit remain unbound. |
| F31J editor | `CEDT030.C` F7341 | The native Shift-JIS glyph consumer is not recovered; drawing host text would fabricate the screen. |

## Verification

`tests/test_csb_v1_fmtowns_m11_game_handoff.c` is an opt-in, real-media test.
With a hash-admitted F31E/F31J source tree it verifies the C06 P3 envelope,
language-specific executable choice, menu bytes, icon palette, F31E font,
the 24-record retail portrait catalogue and the 31/32 F0689 arrow stride.
It skips when licensed game data is absent. No original game bytes are stored
in the repository.

The implementation boundary is in `src/csb/csb_v1_fmtowns_game.c`,
`src/csb/csb_v1_fmtowns_utility_render.c` and
`src/engine/m11_game_view.c`. The callable-symbol inventory deliberately
keeps the unbound CEDT001/CEDT008/CEDT013 functions as `MISSING`; this note
does not change those audit dispositions.

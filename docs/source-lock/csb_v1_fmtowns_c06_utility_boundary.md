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
| Arrow bitmap | F31 `UTILE.EXP` virtual offset `0x14f70`; ReDMCSB F0689 | Bound to raster | The IMG2 stream is hash-checked and decoded as 31 logical pixels per 32-pixel row buffer for the C06 picker surface. |
| `.CMP` catalogue and picker state | `CEDT008.C` F7080/F7081/F7083/F7084 and `CEDT001.C F7002_ReadCMP` | Bound | Only valid, decoded `PORTRAIT/*.CMP` records are catalogued. M11 enters the source picker on `LOAD CHAMPIONS`, renders its nine-row F31E surface, sends source-coordinate clicks through F7084, then imports only the selected admitted record through F7002. |
| Existing-record CMP serializer | CMP layout; `CEDT006.C` C12 copy sequence | Internal only | The tested serializer preserves CMP format/identity bytes and writes C06's 8-byte name, 20-byte title and planar payload fields. It is not exposed as C06 save behavior. |

## Explicitly closed routes

The following work stays fail-closed. It must not borrow the generic PC 3.4
utility flow or create replacement data:

| Route | Missing original owner(s) | Why it remains closed |
|---|---|---|
| Make New Adventure | `CEDT006.C` F7086/F7090 | The separate new-adventure state transition has not been bound to F31 data. |
| Save Champions dialog and destination | `CEDT001.C` F7000/F7001; F31 `#CHAMP_NAME#` file-operation mapping | F7001 asks whether to save the whole game, selected champion or cancel; F7000 then creates the selected portrait through the platform file-operation mapping. Rewriting all existing catalogue records was not source behavior, so the product route is disabled until this mapping is recovered. |
| Name and title editing | `CEDT006.C` F7027/F7028/F7041 | Cursor timing, keyboard input and text commit remain unbound. |
| F31J editor | `CEDT030.C` F7341 | The native Shift-JIS glyph consumer is not recovered; drawing host text would fabricate the screen. |

## Verification

`tests/test_csb_v1_fmtowns_m11_game_handoff.c` is an opt-in, real-media test.
With a hash-admitted F31E/F31J source tree it verifies the C06 P3 envelope,
language-specific executable choice, menu bytes, icon palette, F31E font,
the 24-record retail portrait catalogue, F7083/F7084 picker-to-F7002 import,
the internal CMP name/title/payload serializer, and the 31/32 F0689 arrow
stride.
It skips when licensed game data is absent. No original game bytes are stored
in the repository.

The implementation boundary is in `src/csb/csb_v1_fmtowns_game.c`,
`src/csb/csb_v1_fmtowns_utility_render.c` and
`src/engine/m11_game_view.c`. The callable-symbol inventory deliberately
keeps the unbound CEDT001/CEDT008/CEDT013 functions as `MISSING`; this note
does not change those audit dispositions.

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
| F31J initial game-source chooser | Original `UTILJ.EXP` C06 handoff, Tsugaru capture, `FMT_FNT.ROM` and Tsugaru `KanjiROMAccess::FontROMCode()` | Bound and presented, 640×400 | M11 retains the observed 640×400 indexed page and maps it with nearest-neighbour presentation to its active host canvas; the source frame remains available at native resolution. The four chooser strings remain exact Shift-JIS capture bytes and every 16×16 glyph is fetched from the user-authorised font ROM. No Unicode or host-font fallback exists. |
| F31E editor raster | ReDMCSB `CEDT006.C` F7030/F7034/F7042; C09_ICON and M653 material | Bound | M11 renders the English editor only from the admitted C06/MINI.DAT receipts. There is no host font or PC 3.4 backdrop. |
| Palette selection | `CEDT006.C` F7035/F7036/F7043 | Bound | The selected C09 swatch is editor-local and cannot change a save. |
| Pixel drawing | `CEDT006.C` F7037/F7044/F7045, lines 460/962/1010 | Bound | A single planar undo copy is retained before an edit. The target is the selected 32×29 MINI.DAT portrait, not a generated host bitmap. |
| Connected fill | `CEDT006.C` F7046, line 1040 | Bound | The bounded 32×29 four-neighbour fill changes only the selected source-colour region. A no-op fill does not mark the portrait dirty. |
| Revert and Undo | C06 menu rectangles in `CEDTDATA.C G2272_MouseInputs`; F7037 | Bound | Revert restores the admitted original portrait in memory; Undo exchanges the source-format undo copy. Neither operation writes a file. |
| Quit | `CEDT006.C` F7005/F7050 and `SWITCH.C` | Bound | M11 returns to the selected language's AUTOEXEC/SWITCHTW route and keeps the source sixty-VBlank delay. |
| Arrow bitmap | F31 `UTILE.EXP` virtual offset `0x14f70`; ReDMCSB F0689 | Bound to raster | The IMG2 stream is hash-checked and decoded as 31 logical pixels per 32-pixel row buffer for the C06 picker surface. |
| `.CMP` catalogue and picker state | `CEDT001.C` F7004, `CEDT008.C` F7080/F7081/F7083/F7084 and `CEDT001.C F7002_ReadCMP` | Bound | `LOAD CHAMPIONS` first renders F7004's G7068/G7064 `GAME`/`PORTRAIT`/`CANCEL` dialog using G2261. Only its `PORTRAIT` choice opens the valid, decoded `PORTRAIT/*.CMP` catalogue. M11 renders its nine-row F31E surface, sends source-coordinate clicks through F7084, then imports only the selected admitted record through F7002. |
| F7000 portrait destination mapping | `CEDT001.C` F7000; `CEDTDATA.C` M747; verified UTILE/UTILJ strings | Bound to receipt | The native `2:\\#CHAMP_NAME#.CMP` mapping is read from the authenticated C06 image for English and Japanese. |
| Selected CMP save | `CEDT001.C` F7000/F7001; `CEDTDATA.C` G2261/G2297 | Bound | The native GAME/PORTRAIT/CANCEL dialog is rendered with the verified F31 font. PORTRAIT writes only the selected champion to the mapped dynamic medium and preserves an admitted CMP header. |
| Portrait-medium reload | `CEDT008.C` F7083/F7084 `NEW DISK` | Bound | `NEW DISK` reopens the verified F7000 portrait medium and reuses F7002's strict `.CMP` admission. An empty medium stays an explicit empty-disk result. |
| Name and title editing | `CEDTDATA.C` G2272 entries 13/14; `CEDT006.C` F7027/F7028/F7038/F7041; `DEFS.H` `CHAMPION` | Bound for F31E | M11 routes real mouse fields and SDL text to the selected C06 champion. It reproduces the 6-pixel cursor geometry, uppercase A–Z plus `.,;:` filter, non-leading spaces, insertion/backspace, arrow/Home/End/Page Up/Page Down navigation and the source 30-VBlank blink cadence. The recovered source layout is `Name[8]` and `Title[20]`: C06 admits 7 name and 19 title characters. The field raster, underscores and cursor use only the admitted 5×6 C06 font. |
| Native party save patch | ReDMCSB `DEFS.H` `CHAMPION`; `LOADSAVE.C` F0433/F0435; `CEDTINC8.C` F7052 | Bound | The F31 party part uses `Name[8]`, `Title[20]`, `Direction` at byte 28 and `Cell` at byte 29. Native F7052/F0433 writing clears and rewrites only these fixed source fields before re-obfuscating the part; F0435 readback verifies the complete identity and pose. |
| C06 whole-game save | `CEDT001.C` F7001; `CEDTINC8.C` F7052; `LOADSAVE.C` F0433/F0435/F7062 | Bound for F31E CSB | The `GAME` choice makes a private runtime from the admitted `MINI.DAT` bootstrap on its first save, applies only C06's party edits and writes the five native parts, raw portrait receipt and F7062 tail to `CSBGAME.DAT`. Later saves use F0433's update path and retain the source `.BAK` recovery rule. It never serializes an unrelated M11/PC runtime. |
| C06 whole-game load | `CEDT001.C` F7004; `CEDTINC8.C` F7051; `LOADSAVE.C` F0435/F7063 | Bound for F31E CSB | `GAME` load opens the same selected native slot, validates its header, five parts, portraits and tail, and restores the C06 party/portrait receipt. A damaged primary is recovered only through the native `CSBGAME.BAK` path. |

## Explicitly closed routes

The following work stays fail-closed. It must not borrow the generic PC 3.4
utility flow or create replacement data:

| Route | Missing original owner(s) | Why it remains closed |
|---|---|---|
| Make New Adventure | `CEDT006.C` F7086/F7090; `CEDTINCH.C` F7086; `CEDTINCI.C` F7088/F7089/F7090 | F7086 requires a valid source dungeon and unique party names. F7090 then copies destination header state, portraits and party placement, normalizes resources/status and removes every equipped item's statistic modifiers before collision resolution. Firestaff has a generic normalization contract, but no verified F31 source/destination-object transaction; it must not approximate F7020 modifier removal or invent a destination `MINI.DAT`. |
| Dungeon Master versus CSB chooser | `CEDTINCD.C` F7051 | C06's separate destination game selection is not a CSB-only toggle. The bound route opens only the selected CSB native medium, rather than presenting a fabricated Dungeon Master choice. |
| F31J editor after the initial chooser | C06 edit and save-dialog execution beyond the captured selector | The initial selector is now ROM-bound, but the later editor, media prompts and write paths do not yet have their own F31J runtime captures and command receipts. They remain closed rather than reusing F31E geometry or strings. |

## Verification

### Reproducible F31J C06 witness (Tsugaru, 2026-08-12)

The local original F31J CUE was run with the user-owned full Towns ROM set in
Tsugaru CUI. Tsugaru identified the firmware as `TBIOS_V31L22A`; this is the
same BIOS identity recorded by the F31J selector boundary. Its own event-log
playback was used for input, so the capture did not depend on a host mouse
coordinate transform. From the Japanese AUTOEXEC menu, the second illustrated
entry opens C06's 640×400 source chooser. The chooser visibly offers Dungeon
Master, Chaos Strikes Back and Cancel. Selecting Chaos Strikes Back reaches
the separate prompt that requires the game-save disk in `A:` before any editor
or save transaction starts.

This witness confirms the ordering and A:-medium gate only. It does **not**
authorize a F31J editor, file picker, write path, or a generated save medium.
The captures and event scripts remain in the user-owned external capture
directory and are deliberately not repository assets.

`tests/test_csb_v1_fmtowns_m11_game_handoff.c` is an opt-in, real-media test.
With a hash-admitted F31E/F31J source tree it verifies the C06 P3 envelope,
language-specific executable choice, menu bytes, icon palette, F31E font,
the 24-record retail portrait catalogue, F7083/F7084 picker-to-F7002 import,
the F7001 dialog, F7000 selected-portrait writer, native F7001/F7052 first and
repeated GAME saves, F7004/F7051 GAME loads including `.BAK` recovery, the C06
text-edit contract and the 31/32 F0689 arrow stride.  The source `2:\\#CHAMP_NAME#.CMP` drive maps to
`~/.firestaff/portraits` on macOS/Linux and `INSTALLDIR\\portraits` on Windows;
the scanned CD `PORTRAIT` catalogue remains read-only.
It skips when licensed game data is absent. No original game bytes are stored
in the repository.

For the F31J selector route, set `FIRESTAFF_FMTOWNS_FONT_ROM` to the user's
licensed, exact 256 KiB `FMT_FNT.ROM`. Firestaff reads it in place at runtime,
validates its extent through the TBIOS shim and retains it only in process
memory. It neither scans it into game data nor distributes it. Without that
ROM the Japanese C06 route stays closed.

The opt-in `csb_v1_fmtowns_user_save_corpus` regression accepts either one
already selected F31 package through `FIRESTAFF_CSB_FMTOWNS_GAME_DATA_DIR`, or
a mixed licensed CD tree through `FIRESTAFF_CSB_FMTOWNS_LOOSE_DATA_DIR`.  In
the latter case it materializes `fmtowns-en` and `fmtowns-ja` separately before
opening each candidate save.  This keeps a rejected English `CSBGAME.DAT` from
being tested against Japanese media, and proves that the admitted Japanese
`CSBGAME-JP.DAT` stays bound to the F31J package.

The implementation boundary is in `src/csb/csb_v1_fmtowns_game.c`,
`src/csb/csb_v1_fmtowns_utility_render.c` and
`src/engine/m11_game_view.c`. The callable-symbol inventory deliberately
keeps the unbound CEDT001/CEDT008/CEDT013 functions as `MISSING`; this note
does not change those audit dispositions.

# CSB Hint Oracle Atari R1 runtime source lock

`CSB_HintOracleAtariRuntime` is the complete source-owned consumer for the
documented Atari ST 2.0/2.1 Utility Disk R1 Oracle path.

## Admitted original inputs

The runtime fixes the matching R1 hashes instead of independently selecting
an HTC and DAT file:

| Input | Identity | Ownership |
|---|---|---|
| HCSB.HTC | `8ce69b54cf255a15e98e909bb45b9742` | HINT names, locations and compressed pages |
| HCSB.DAT | `708e113c869ab922633e885aa72a3c77` | 320×200 screen, font, palette and controls |
| MINI.DAT | checked `CSB_V1_AtariSaveInfo` | `party_map_index`, `party_x`, `party_y` only |

dmweb identifies the HTC R1 file as shared by Atari ST 2.0/2.1 and Amiga
English R1. ReDMCSB `HINTLOAD.C` names `HCSB.HTC`, `HCSB.DAT` and the CSB
saved-game input. No runtime fallback admits a filename, a host font, a
generated frame, a generic CSBWin save, or a mixed release/language pair.

## Source state and draw chain

The runtime composes the existing source-locked pieces in this order:

1. native MINI.DAT decoder → checked GAMEBLOCK2 position receipt;
2. `HINTHINT.C` C09 authored-order exact/wildcard selection, capped at seven;
3. C06/F1940 row and one-based LAST/NEXT state;
4. `HINTSCR.C` / `HINTTEXT.C` title and page boxes with the original DAT
   background, font and C26 palette remap;
5. `HINTMAIN.C` state-1 LOAD/EXIT prompt, `HINTHINT.C` C06 source-title rows
   and DONE, state-2 no-clue/OK, and `F1940`'s conditional LAST/NEXT/DONE.

`csb_hint_oracle_atari_runtime_render_frame()` is the single presentation
entry point. It begins every admitted state with the decoded HCSB.DAT
320×200 indexed image, then renders only the source strings in their original
`STRUCT22B` boxes: prompt `31..290,50..150`, no clue `31..290,70..150`, seven
list rows `40..280,30..138`, page title `10..309,5..30`, page body
`34..285,31..164`, and the `HINTDATA.C` button spans. It has no generated
frame or host glyph fallback. `render_page()` remains a page-only compatibility
entry point.

`test_csb_hint_oracle_atari_runtime` is fail-closed without the media. With
`FIRESTAFF_CSB_HINT_ORACLE_DATA_DIR` set to the staged R1 directory, it reads
and validates MINI.DAT, loads both exact hashes, selects its location and
renders the prompt, list, first selected page and no-clue state. The current
staged receipt is map 4, x 22, y 18 with two matching hints.

## Remaining boundary

Firestaff exposes the same runtime through both `--csb-hint-oracle --data-dir
<root> --save <MINI.DAT>` and the `CSB UTILITY DISK — HINT ORACLE` start-menu
entry. The latter binds `<selected-data-dir>/MINI.DAT`; both paths hash-admit
the R1 HCSB pair before the native save decoder may provide the original LOAD
control with a location receipt. Its pointer dispatcher uses the Atari R1
`HINTDATA.C` LOAD/LAST/NEXT/DONE/EXIT/OK and seven hint-row rectangles, then
invokes the corresponding `HINTMAIN.C` transition.

Original event cadence, captured original clicks and full-frame pixel
comparison remain open. The exposed route must not be presented as pixel
parity until those recordings exist.

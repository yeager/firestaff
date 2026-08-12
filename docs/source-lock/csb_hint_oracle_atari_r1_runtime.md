# CSB Hint Oracle Atari R1 runtime source lock

`CSB_HintOracleAtariRuntime` is the complete non-UI consumer for the
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
   background, font and C26 palette remap.

`test_csb_hint_oracle_atari_runtime` is fail-closed without the media. With
`FIRESTAFF_CSB_HINT_ORACLE_DATA_DIR` set to the staged R1 directory, it reads
and validates MINI.DAT, loads both exact hashes, selects its location and
renders the first selected page. The current staged receipt is map 4, x 22,
y 18 with two matching hints.

## Remaining boundary

This is a C runtime owner, not an M11/M12 menu feature. `HINTDATA.C` command
rectangles, `HINTMAIN.C` event cadence, original click capture and full frame
pixel comparison must be bound before a user-facing claim is made.

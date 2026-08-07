# Dungeon Master II Skullkeep FM Towns — technical guide

DM2's FM Towns port sits alongside DM1 and CSB in the Fujitsu HMA-240
Phar Lap family. Read this together with the DM1 and CSB FM Towns
guides plus `docs/fmtowns/CROSS_GAME_COVERAGE.md`.

## Retail media

Disc archive: `Dungeon-Master-II-Skullkeep_FM-Towns_JA.zip`
(Victor Entertainment, 1993). Track 01 is a MODE1/2352 image; strip
288-byte-per-sector CD headers to get the 2048-byte ISO stream.

Byte-verified file inventory:

| Path | Bytes | sha256 (start) |
|---|---:|---|
| SKULL.EXP | 374,416 | 068218bb.. |
| TWANIM.EXP | 72,184 | f82e5332.. |
| DATA/GRAPHICS.DAT | 2,783,791 | 634e7004.. |
| DATA/DUNGEON.DAT | 37,954 | d1d608a1.. |
| DATA/CD.DAT | 40 | 8352b173.. |
| TBIOS.BIN / TBIOS.SYS | — | (BIOS blobs) |

Full manifest in `docs/fmtowns/all_games_real_data_hashes.json`.

## Phar Lap P3 layout

SKULL.EXP: level-1 header, load offset 0x200, init EIP 0x5741c,
no SYM1 table (stripped from release). TWANIM.EXP is the
animation binary: load offset 0x200, init EIP 0x10470.

## Phar Lap real-mode bridge

Same 4-slot layout as DM1 and CSB (`fmtowns_pharlap_all_games`):

| Slot | Calls in SKULL.EXP | Calls in TWANIM.EXP |
|:---:|---:|---:|
| fs:[0x20] TBIOS | 17 | 13 |
| fs:[0x40] Secondary | 10 | 0 |
| fs:[0x48] Timing | 0 | 0 |
| fs:[0x80] Hardware init | 2 | 2 |

## Direct hardware I/O

SKULL.EXP touches ONLY port 0x04E9 (SOUND_INT_REASON, one read),
consistent with DM1 EDM.EXP and CSB CHTWE.EXP. TWANIM.EXP same.
Verified via `fmtowns_direct_io_cross_game_profiles`.

## Byte-verified DM1↔DM2 shared payloads

DM2 shares less with DM1 than CSB does (different game mechanics,
different asset atlas). Confirmed shared:

| Payload | Bytes | DM1 vaddr | DM2 vaddr | Notes |
|---|---:|:---:|:---:|---|
| Menu font raster | 768 | (asset 557) | file@0x2f5a3 in DATA/GRAPHICS.DAT | 768/768 identical |
| CHAR geometry | 14 | 0x26c8a | 0x1f6 | 5,6,1,1,1,6,7 |
| ICON geometry | 8 | 0x26c68 | 0x1de | 320,256,16,16 |
| SPELL_COSTS | 32 | 0x24388 | 0x3bb0 | identical |
| SPELL_MULT | 8 | 0x243a0 | 0x3bc8 | identical |
| Phar Lap 4-slot bridge | — | ✓ | ✓ | universal |
| Direct I/O 0x04E9 | — | ✓ | ✓ | universal |

DM2 does NOT share DYNA_BUTTONS labels (Skullkeep has different
action set — no BLOCK/CHOP/FIREBALL/FUSE strings in SKULL.EXP)
or OICON descriptors (different item/thing set).

## GRAPHICS.DAT format — extended v4 (open)

DATA/GRAPHICS.DAT starts with signature 0x8004 (extended format
v4). Layout:

  [u16 sig=0x8004]
  [u16 count=3407]
  [u16 record[3407] * 4 bytes each]  -- {u16 size, u16 flags} per record
  [payload]

The per-record payload sum doesn't balance the file directly
(0x271694 sum vs 0x2a44ef payload space), so the record fields
aren't simple size+size or size+offset. Further RE work required
to decode individual DM2 assets. Format classifier at
`fmtowns_graphics_dat_format` identifies the container as
`FMTOWNS_GRAPHICS_DAT_FORMAT_EXT_V4` so consumers fail closed
until a decoder ships.

## What is NOT yet ported

- DM2 SKULL.EXP SYM1 symbol table (stripped from release binary).
- DM2 extended-v4 per-record decode (needs more disassembly).
- DM2 region table (different menu layout from DM1/CSB).
- DM2-specific game tables (DOOR_PAL, LEVEL_SONGS, DYNA_BUTTONS
  labels, OICON descriptors).

## What is shared with DM1/CSB

Everything under "Byte-verified DM1↔DM2 shared payloads" above,
plus the Phar Lap bridge and TownsOS BIOS integration surface
common to all three games. Consumers can use the shared cross-game
modules (`fmtowns_geometry_all_games`, `fmtowns_shared_tables_all_games`,
`fmtowns_font_raster_all_games`) to access DM2's data through the
same code that handles DM1 and CSB.

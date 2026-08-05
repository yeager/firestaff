# DM2 V1 Save File Format

## Overview
DM2 uses a single-file save system (`SKSave.dat`) with a custom binary format driven by a run-length-like compression scheme called SUPPRESS (variable-bit-width encoding). DM1 used a simpler record-based format.

## File: `SKSave.dat` (primary) / `SKSave.bak` (backup)
Located alongside `Dungeon.ftl`. On save, `SKSave.dat` is renamed to `.bak` and a new `.dat` is written. On load, if `.dat` fails, `.bak` is tried as fallback.

## Header (42 bytes, `sksave_header_asc`)
```
w0       : version/flags (PC-DOS corpus: 1)
b2[36]   : bounded null-terminated ASCII save name (`c_hex2a::text`)
l38       : opaque `c_hex2a::l_26`, retained from the prior file on save
```

The real DOS files do not use Firestaff's former `0xBEEF/0xDEAD` fixture
markers. `0xDEADBEEF` is the in-memory empty-entry sentinel used by the
original load dialog and is not accepted as an on-disk header. On save,
SKProject retains `l38` from the previous header, changes `w0` to 1 and copies
the entered name. Header shape is only a container gate; the raw dungeon and
SUPPRESS stream must parse before a save is admitted.

## Save Sections (in order)
1. **Dungeon header** (`DunHeader`, 44 bytes)
2. **Map headers array** (`dunMapsHeaders`, `nMaps << 4` bytes)
3. **Tile->object index per column** (`dunMapTilesObjectIndexPerColumn`, `_4976_4cb4 << 1` bytes)
4. **Ground stacks** (`dunGroundStacks`, `cwListSize << 1` bytes)
5. **Text data** (`dunTextData`, `cwTextData << 1` bytes)
6. **16 DB record pools** (each `dbSize[db] * nRecords[db]` bytes)
7. **Map data** (`dunMapData`, `cbMapData` bytes)
8. **Game state block** (`s_savegamebuffer`, 60 bytes, SUPPRESS-encoded):
   - `dwGameTick` - global game tick counter
   - `dwRandomSeed` - RNG seed
   - `wChampionsCount` - number of champions (1-4)
   - `wPlayerPosX/Y` - player grid position
   - `wPlayerDir` - facing direction
   - `wPlayerMap` - current dungeon map index
   - `wChampionLeader` - party leader champion slot
   - `wTimersCount` - active timer count
   - opaque source fields at offsets `0x16..0x3b`; their names and sizes are
     exactly those in `SKWINSPX/src/v5/sksvgame.cpp::s_savegamebuffer`

   This is the packed `s_savegamebuffer` layout in SKProject
   `SKWINSPX/src/v5/sksvgame.cpp` (size `0x3c`), not the former Firestaff
   56-byte diagnostic convenience view and not an eight-byte weather-state
   array. `table1d631a[60]` in `dm2data.cpp` selects its source bits.
9. **Ingame global flags** (8 bytes, SUPPRESS)
10. **Ingame global bytes** (64 bytes, SUPPRESS)
11. **Ingame global words** (64 words, SUPPRESS)
12. **Champion squad** (261 bytes x `wChampionsCount`, SUPPRESS)
13. **Global spell effects** (6 bytes, SUPPRESS)
14. **Timers table** (12-byte `c_tim` rows x `wTimersCount`, SUPPRESS;
    mask starts at `vsgame[0]`)
15. **Champion inventories** - each champion's 30 inventory slots written as record-link chains via `WRITE_RECORD_CHECKCODE`
16. **Leader hand possession** - single record link
17. **Saved tile and record-chain data** (the source
    `STORE_EXTRA_DUNGEON_DATA()` / `DM2_READ_SKSAVE_DUNGEON` continuation on
    the same SUPPRESS stream)
18. **Possession indices** (source `DM2_WRITE_POSSESSION_INDICES` /
    `DM2_2066_062b` continuation)

## Compression: SUPPRESS (Bit-level RLE)
`SUPPRESS_WRITER` writes selected source bits using per-field masks. Fields
with mask=0 are skipped. Set mask bits are scanned from bit 7 to bit 0 and
packed MSB-first into one continuous byte stream; the fixed state, globals,
heroes, timers and later record chains share that stream until the final
flush. A companion `SUPPRESS_READER` decodes in the same order.

## DM1 vs DM2 Key Format Differences
| Aspect | DM1 | DM2 |
|--------|-----|-----|
| File | `CHAMP.DAT` per champion + `DUNGEON.DAT` per dungeon | Single `SKSave.dat` with all state |
| Compression | None (raw records) | SUPPRESS bit-level RLE |
| Dungeon state | Separate .DAT per level | Fully embedded in save |
| Header | None visible | Version/name container; remaining words are opaque |
| Extra data | None | `STORE_EXTRA_DUNGEON_DATA()` hook |
| Backup | None | `.bak` auto-created on each save |

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
8. **Extra dungeon data** (via `STORE_EXTRA_DUNGEON_DATA()`)
9. **Game state block** (`skload_table_60`, 56 bytes, SUPPRESS-encoded):
   - `dwGameTick` - global game tick counter
   - `dwRandomSeed` - RNG seed
   - `wChampionsCount` - number of champions (1-4)
   - `wPlayerPosX/Y` - player grid position
   - `wPlayerDir` - facing direction
   - `wPlayerMap` - current dungeon map index
   - `wChampionLeader` - party leader champion slot
   - `wTimersCount` - active timer count
   - `dw22`, `dw26`, `w30`, `wPlayerThrowCounter`, `w34`, and bytes 36–39
   - `wRainFlagSomething`, ambient-light modifier, direction, strength,
     sky/ground levels, multiplicator, storm controller, two related bytes,
     and `dwRainSpecialNextTick` (bytes 40–55)

   This is the packed `skload_table_60` layout in SKProject
   `SKWIN/DME.h`; it is not an eight-byte weather-state array. The matching
   `SKWIN/SkGlobal.cpp::_4976_395a` SUPPRESS mask selects source bits across
   all 56 bytes, including the terminating zero mask byte at offset 55.
10. **Ingame global flags** (8 bytes, SUPPRESS)
11. **Ingame global bytes** (64 bytes, SUPPRESS)
12. **Ingame global words** (64 words, SUPPRESS)
13. **Champion squad** (261 bytes x `wChampionsCount`, SUPPRESS)
14. **Global spell effects** (6 bytes, SUPPRESS)
15. **Timers table** (10 bytes x `wTimersCount`, SUPPRESS)
16. **Champion inventories** - each champion's 30 inventory slots written as record-link chains via `WRITE_RECORD_CHECKCODE`
17. **Leader hand possession** - single record link
18. **Extra dungeon data**
19. **Minion association table** (via `WRITE_MINION_ASSOC`)

## Compression: SUPPRESS (Bit-level RLE)
`SUPPRESS_WRITER` writes bit-planes using per-field masks. Fields with mask=0 are skipped. Non-zero nibbles from data+mask are packed LSB-first into a byte stream. A companion `SUPPRESS_READER` decodes on load. Flush at end of save.

## DM1 vs DM2 Key Format Differences
| Aspect | DM1 | DM2 |
|--------|-----|-----|
| File | `CHAMP.DAT` per champion + `DUNGEON.DAT` per dungeon | Single `SKSave.dat` with all state |
| Compression | None (raw records) | SUPPRESS bit-level RLE |
| Dungeon state | Separate .DAT per level | Fully embedded in save |
| Header | None visible | Version/name container; remaining words are opaque |
| Extra data | None | `STORE_EXTRA_DUNGEON_DATA()` hook |
| Backup | None | `.bak` auto-created on each save |

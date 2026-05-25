# DM2 V1 Save File Format

## Overview
DM2 uses a single-file save system (`SKSave.dat`) with a custom binary format driven by a run-length-like compression scheme called SUPPRESS (variable-bit-width encoding). DM1 used a simpler record-based format.

## File: `SKSave.dat` (primary) / `SKSave.bak` (backup)
Located alongside `Dungeon.ftl`. On save, `SKSave.dat` is renamed to `.bak` and a new `.dat` is written. On load, if `.dat` fails, `.bak` is tried as fallback.

## Header (42 bytes, `sksave_header_asc`)
```
w0       : version/flags (written as 1 on each save)
b2[34]   : ASCII null-terminated save name string (max 34 chars)
w36      : (slot index in ASCII + 0x30, e.g. slot 0 -> 0x30 = '0')
w38      : magic marker 0xBEEF (valid slot indicator)
w40      : magic marker 0xDEAD (valid slot indicator)
```

A slot is valid when `w38 == 0xBEEF && w40 == 0xDEAD`.

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
   - Rain state fields (8 bytes)
   - `_dw22/26`, `_w30`, `_w34` - misc state
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
| Header magic | None visible | Magic 0xBEEF/0xDEAD slot markers |
| Extra data | None | `STORE_EXTRA_DUNGEON_DATA()` hook |
| Backup | None | `.bak` auto-created on each save |
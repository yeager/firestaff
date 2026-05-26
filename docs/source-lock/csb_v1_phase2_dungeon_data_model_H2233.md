# CSB V1 Phase 2: Dungeon Data Model Source-Lock

**pass608 / cron:682364a5-8ae8-4dc2-9cde-0cfaa4b0b81f**
**Audit:** 2026-05-26
**Sources:** ReDMCSB DEFS.H:985–1070 · LOADSAVE.C:1803–2165 · DUNGEON.C:1423–1750 · COMMAND.C · CEDTINCH.C · CEDTINCU.C · HINTLOAD.C · CSBWin CSBCode.cpp DBank::Initialize (TAG00332a) · memory_dungeon_dat_pc34_compat.c (existing Firestaff DM1 implementation)

---

## Finding: CSB Dungeon Format Is the Same Structure as DM1

Both DM1 and CSB use the **identical** DUNGEON_HEADER (44 bytes) and MAP descriptor (16 bytes) structure. The difference is in the **file-level wrapper** (CSB uses compressed format) and the **level count** (up to 24 vs 14). The in-memory representation, column-major layout, square format, and thing system are shared.

---

## Part I: Dungeon File Format — Two-Level Structure

### Level 1: Optional Compressed Wrapper (CSB)

```
Offset 0x0000: COMPRESSED_DUNGEON_HEADER (8 bytes)
  +0x00: uint16_t Signature       = 0x8104 LE (C0x8104_SIGNATURE_COMPRESSED_DUNGEON)
  +0x02: uint32_t DecompressedByteCount  (big-endian on disk, 68k format)
  +0x06: uint16_t DungeonID        (LE)
```

If `Signature != 0x8104`, the file is **uncompressed** and starts directly with DUNGEON_HEADER.

Source: DEFS.H:983–987 · LOADSAVE.C:1876–1895 · memory_dungeon_dat_pc34_compat.c:119–212

### Level 2: DUNGEON_HEADER (44 bytes, shared DM1+CSB)

```
Offset +0x00: uint16_t OrnamentRandomSeed
Offset +0x02: uint16_t RawMapDataByteCount
Offset +0x04: uint8_t  MapCount
Offset +0x05: uint8_t  Unreferenced (padding for alignment on Atari/Amiga)
Offset +0x06: uint16_t TextDataWordCount
Offset +0x08: uint16_t InitialPartyLocation  (encoded: X[4:0], Y[9:5], Direction[11:10])
Offset +0x0A: uint16_t SquareFirstThingCount
Offset +0x0C: uint16_t ThingCount[16]         (one per THING_TYPE)
```

Source: DEFS.H:989–998 · memory_dungeon_dat_pc34_compat.c:219–232

### DUNGEON_HEADER Fields in Detail

| Offset | Field | Type | Description |
|--------|-------|------|-------------|
| 0x00 | OrnamentRandomSeed | uint16 | Seed for random ornament placement |
| 0x02 | RawMapDataByteCount | uint16 | Total bytes of raw square data across all maps |
| 0x04 | MapCount | uint8 | Number of MAP descriptors (1–32) |
| 0x05 | Unreferenced | uint8 | Padding byte (compiler-inserted on Atari/Amiga; explicit on PC) |
| 0x06 | TextDataWordCount | uint16 | Number of 3-character words in text data |
| 0x08 | InitialPartyLocation | uint16 | Encoded party start: X=bits[4:0], Y=bits[9:5], Dir=bits[11:10] |
| 0x0A | SquareFirstThingCount | uint16 | Number of square-first-thing entries (2 bytes each) |
| 0x0C | ThingCount[16] | uint16[16] | Count of things per type |

Source: DEFS.H:989–998

### After DUNGEON_HEADER: MAP Descriptors

```
MAP[0]: RawMapDataByteOffset, aUnreferenced, bUnreferenced, OffsetMapX, OffsetMapY,
        A (bitfield: Level[6], Width[5], Height[5]),
        B (bitfield: WallOrnamentCount[4], RandomWallOrnamentCount[4], FloorOrnamentCount[4], RandomFloorOrnamentCount[4]),
        C (bitfield: DoorOrnamentCount[4], CreatureTypeCount[4], Unreferenced[4], Difficulty[4]),
        D (bitfield: FloorSet[4], WallSet[4], DoorSet0[4], DoorSet1[4])
  ... (16 bytes each, MapCount entries)
```

Source: DEFS.H:1048–1070

**MAP A.Width / A.Height are stored as (actual - 1)** — the bitfield stores Width-1 and Height-1. Actual width = A.Width + 1, actual height = A.Height + 1. Maps are max 16×16 squares (stored as 15×15 in bitfields).

Source: DUNGEON.C:2735–2736: `G0273_i_CurrentMapWidth = G0269_ps_CurrentMap->A.Width + 1; G0274_i_CurrentMapHeight = G0269_ps_CurrentMap->A.Height + 1;`

### After MAP Descriptors: Raw Map Data

```
RawMapData: (RawMapDataByteCount bytes)
  Column 0: (A.Height+1) * 2 bytes (column-major, 2 bytes per square)
  Column 1: (A.Height+1) * 2 bytes
  ... for (A.Width+1) columns
  (Map descriptor RawMapDataByteOffset points to first column of this map in the overall raw data)
```

Source: LOADSAVE.C:2157–2163 · DUNGEON.C:1073 · memory_dungeon_dat_pc34_compat.c:379

**Square layout within raw data**: `G0271_ppuc_CurrentMapData[x][y]` — column x, row y. Each square is 2 bytes. Index: `base + x * (height+1) * 2 + y * 2`. This is column-major (x varies fastest in the linear offset, but y indexes within the column — equivalent to `x * (height+1) + y` for the square index, then `* 2` for byte offset).

Source: DUNGEON.C:1440: `G0271_ppuc_CurrentMapData[P0258_i_MapX][P0259_i_MapY]` (x first, y second) — column-major.

**Clarification on "column-major"**: In C terms `map[x][y]` means x is the outer dimension (column index), y is inner (row within column). The raw data stores columns sequentially. For a map of width W and height H: column 0 occupies bytes 0..(H*2-1), column 1 occupies bytes H*2..(2*H*2-1), etc. Within each column, y goes from 0 (top) to H-1 (bottom).

Source: DUNGEON.C:2161–2163:
```c
*L1357_ppuc_ColumnFirstSquares++ = (L1358_puc_Square = G0276_puc_DungeonRawMapData + G0277_ps_DungeonMaps[L1354_i_Counter].RawMapDataByteOffset);
for (AL1353_i_Counter = 1; AL1353_i_Counter <= G0277_ps_DungeonMaps[L1354_i_Counter].A.Width; AL1353_i_Counter++) {
    *L1357_ppuc_ColumnFirstSquares++ = (L1358_puc_Square += (G0277_ps_DungeonMaps[L1354_i_Counter].A.Height + 1));
}
```

### Square Format (2 bytes, shared DM1+CSB)

```
uint16_t square:
  bits [4:0]   = Square type (M034_SQUARE_TYPE)
  bits [14:5]  = First thing index (10 bits)
  bits [15]    = THING_LIST_PRESENT flag
```

- `square & 0x001F` = element type (Wall=0, Corridor=1, Pit=2, Stairs=3, Door=4, Teleporter=5, FakeWall=6)
- `(square >> 5) & 0x3FF` = first thing index
- `square & 0x8000` = thing list present flag

Source: DEFS.H:1001–1002: `M034_SQUARE_TYPE(square) = ((square) >> 5)` · DUNGEON.C:1717

### After Raw Map Data: SquareFirstThings + Thing Data

```
SquareFirstThings: SquareFirstThingCount * 2 bytes (uint16_t per entry)
ThingType0 data:  ThingCount[0] * thingDataByteCount[0] bytes
ThingType1 data:  ThingCount[1] * thingDataByteCount[1] bytes
...
ThingType15 data: ThingCount[15] * thingDataByteCount[15] bytes
TextData: TextDataWordCount * 2 bytes (3 characters per 16-bit word)
```

Source: LOADSAVE.C:1643–1680 · memory_dungeon_dat_pc34_compat.c:322–341

---

## Part II: CSB vs DM1 Dungeon Format Differences

| Feature | DM1 | CSB |
|---------|-----|-----|
| Dungeon format | Uncompressed DUNGEON_HEADER + data | Compressed (0x8104 sig) or uncompressed |
| DUNGEON_HEADER structure | Same 44 bytes | Same 44 bytes |
| MAP descriptor structure | Same 16 bytes | Same 16 bytes |
| Square format (2 bytes) | Same | Same |
| Column-major layout | Same | Same |
| Level count | 14 (PC 3.4) | Up to 24 (from M13_PLAN.md:303) |
| DungeonID values | C10_DUNGEON_DM = 10 | C12_DUNGEON_CSB_PRISON = 12, C13_DUNGEON_CSB_GAME = 13 |
| Save file | DMSAVE.DAT | CSBGAME.DAT (CEDTINC8.C:101–118) |
| Compressed dungeon support | None | DECOMPDU.C / FTL decompress in memory_dungeon_dat_pc34_compat.c |
| End Game Sensor (type 18) | None | NEW |
| Version Checker Sensor | None | NEW (MOVESENS.C CHANGE7_23) |
| Projectile speed | Buggy on non-party maps | Fixed (PROJEXPL.C CHANGE7_20) |

Source: DEFS.H:522–523 · CEDTINC8.C:101–118 · M13_PLAN.md:303 · PROJEXPL.C (CHANGE7_20)

---

## Part III: Dungeon Data Model — Key Structural Numbers

### Geometry Bytes

For a 16×16 map: RawMapDataByteCount = 16 * 16 * 2 = **512 bytes** per full map.

However, actual maps vary — the MAP descriptor's A.Width+1 and A.Height+1 give the actual dimensions. CSB's 24 levels each have variable sizes.

Source: DUNGEON.C:2161–2163: the step between columns in raw data is `(map->A.Height + 1)` (in 2-byte units).

### The "2098" Reference

The value 2098 appears in:
- `firestaff_known_hashes.c:7`: CSB DUNGEON.DAT hash entry with size 2098 — this is the **file size** of the compressed CSB Atari DUNGEON.DAT, not a geometry value.
- DEFS.H:6357: `G2098_i_CreditsPaletteIndex` — unrelated.

The 2098-byte file is the compressed CSB dungeon. The uncompressed size is larger (from the COMPRESSED_DUNGEON_HEADER DecompressedByteCount field).

**The DM1 2098 geometry reference (16×16 grid, 2 bytes/square) is**: `16 * 16 * 2 = 512` per map, not 2098. The 2098 in CSB context refers to the compressed file size of the Atari ST CSB DUNGEON.DAT.

Source: firestaff_known_hashes.c · DEFS.H:6357

### Dungeon Header Size

- **DUNGEON_HEADER**: 44 bytes (defined as DUNGEON_HEADER_SIZE in memory_dungeon_dat_pc34_compat.h:19)
- **MAP descriptor**: 16 bytes each (DUNGEON_MAP_DESC_SIZE)
- **COMPRESSED_DUNGEON_HEADER**: 8 bytes (defined in DEFS.H:983–985)

Source: memory_dungeon_dat_pc34_compat.h:19–20

---

## Part IV: Champion Transfer / Import State

### CSB Prison Dungeon (DungeonID = C12 = 12)

CSB introduces a prison dungeon (C12_DUNGEON_CSB_PRISON) distinct from the main game dungeon (C13_DUNGEON_CSB_GAME). Champions can be transferred between dungeons.

Source: DEFS.H:522–523 · CEDTINCH.C · CEDTINCU.C

### Dungeon Validation Gate

ReDMCSB `CEDTINCU.C:5–77`: `F7272_IsDungeonValid` switches on save-header format and accepts CSB prison/game IDs only through CSB-aware validation criteria.

Source: PARITY_MATRIX_CSB_V1.md · ReDMCSB CEDTINCU.C

### Champion Import Path

CSB Win: `DBank::Initialize` (TAG00332a, lines 318–480) handles initialization. Champion import/reincarnation routes through the Utility flow in CSBWin `CSB.cpp` and `Mouse.cpp`. 

The `MINI.DAT` file (C13_DUNGEON_CSB_GAME marker in ReDMCSB DEFS.H:523) manages champion state between the prison and game dungeons.

Source: DEFS.H:523 · CSBWin CSBCode.cpp:318–480 · CSB lineage `README` (Utility flow)

### Start Position Semantics

InitialPartyLocation encodes: `X = bits[4:0]`, `Y = bits[9:5]`, `Direction = bits[11:10]`.
- Decoded by `F0501_DUNGEON_DecodePartyLocation_Compat` in memory_dungeon_dat_pc34_compat.c:80–93
- Both DM1 and CSB use the same encoding

Source: DEFS.H:995 · memory_dungeon_dat_pc34_compat.c:80–93 · LOADSAVE.C:1941–1943:
```c
G0306_i_PartyMapX = (AL1353_i_InitialPartyLocation = G0278_ps_DungeonHeader->InitialPartyLocation) & 0x001F;
G0307_i_PartyMapY = (AL1353_i_InitialPartyLocation >>= 5) & 0x001F;
G0308_i_PartyDirection = (AL1353_i_InitialPartyLocation >> 5) & 0x0003;
```

---

## Part V: Wall Format

Walls are the square type (element) = 0. The square's other bits store:
- Door state (lowest 3 bits when element = Door)
- Wall aspect (MASK0x0008_STAIRS_NORTH_SOUTH_ORIENTATION, MASK0x0010_THING_LIST_PRESENT, etc.)
- Ornament indices and random ornament flags

There is **no separate geometry array** in the dungeon format. Walls are squares in the map grid, just like corridors, pits, stairs, etc. The "wall format" is simply the element type field + aspect bits in the 2-byte square record.

Source: DEFS.H:1004–1008 · M034_SQUARE_TYPE · M035_SQUARE · M036_DOOR_STATE · M037_SET_DOOR_STATE

---

## Part VI: Object Records and Thing System

Things (objects, creatures, sensors, groups, etc.) are stored in a separate thing data area after the square data. Each map has a `RawMapDataByteOffset` in its MAP descriptor pointing to where its square data starts in the overall `RawMapData` buffer.

Square records store an index into the `SquareFirstThings` array (the `FirstThingIndex`). Each `SquareFirstThing` entry is a THING index that links to the thing's actual data.

Thing data is stored by type: all things of type T are stored consecutively in `ThingData[T]`. The THING index is an offset within that type's data area.

Source: LOADSAVE.C:2044–2070 · DUNGEON.C:1584–1750 · memory_dungeon_dat_pc34_compat.c:688–915

**Thing type enum** (DUNGEON_THING_TYPE_COUNT = 16):
- Type 4: GROUP (16 bytes) — includes direction and position
- Type 15: DSA (CSB-specific) — Dungeon Scripting Architecture scripts embedded in the dungeon

Source: memory_dungeon_dat_pc34_compat.h:21 · DEFS.H · csb_v1_dungeon_loader_pc34_compat.h:32

---

## Part VII: CSB Dungeon Loading — What Already Exists

Firestaff's `memory_dungeon_dat_pc34_compat.c` **already handles the compressed dungeon format** (0x8104 detection and FTL decompression) via the `ftl_decompress_dungeon()` call at line 169. This is the same decompression used for CSB.

The existing `F0500_DUNGEON_LoadDatHeader_Compat()` reads DUNGEON_HEADER + MAP descriptors and handles both compressed and uncompressed formats. The decompression path writes a temp `.decompressed` file and recursively calls itself to parse.

**What the existing code does NOT do for CSB**:
1. It does not distinguish DM1 vs CSB by DungeonID — it parses any DUNGEON_HEADER format
2. CSB-specific DungeonIDs (12, 13) are not specially handled
3. CSA prison/game dungeon routing is not implemented
4. End Game Sensor type 18 handler is not in the sensor system
5. Version Checker Sensor is not implemented

Source: memory_dungeon_dat_pc34_compat.c:119–212 · csb_v1_dungeon_loader_pc34_compat.c (existing stub)

---

## Part VIII: DungeonID Routing

CSB saves to `CSBGAME.DAT`, DM1 saves to `DMSAVE.DAT`. The routing in ReDMCSB:
- `CEDTINC8.C:101–118`: save-file routing separates `DMSAVE.DAT` and `CSBGAME.DAT`
- `HINTLOAD.C:11–18`: Atari CSB hint/runtime loader names `HCSB.HTC`, `HCSB.DAT`, `CSBGAME.DAT`, `CSBGAME.BAK`
- `FLOPPYST.C:7–18`: Atari CSB save filenames `A:\CSBGAME.DAT` and `A:\CSBGAME.BAK`

Source: ReDMCSB CEDTINC8.C · HINTLOAD.C · FLOPPYST.C

---

## Source Citations

| Source | Lines | Content |
|--------|-------|---------|
| ReDMCSB DEFS.H | 983–998 | COMPRESSED_DUNGEON_HEADER, DUNGEON_HEADER |
| ReDMCSB DEFS.H | 1001–1002 | M034_SQUARE_TYPE, M035_SQUARE |
| ReDMCSB DEFS.H | 1048–1070 | MAP descriptor bitfields A/B/C/D |
| ReDMCSB DEFS.H | 522–523 | C12_DUNGEON_CSB_PRISON, C13_DUNGEON_CSB_GAME |
| ReDMCSB DEFS.H | 1004–1008 | Element types, door state macros |
| ReDMCSB DUNGEON.C | 1004–1016 | G0271_ppuc_CurrentMapData, G0276_puc_DungeonRawMapData, G0279_pppuc_DungeonMapData |
| ReDMCSB DUNGEON.C | 1423–1440 | F0151_DUNGEON_GetSquare, column-major lookup |
| ReDMCSB DUNGEON.C | 1584–1648 | F0156_DUNGEON_GetThingData, F0157_DUNGEON_GetSquareFirstThingData |
| ReDMCSB DUNGEON.C | 1699–1746 | F0160_DUNGEON_GetSquareFirstThingIndex, F0161_DUNGEON_GetSquareFirstThing |
| ReDMCSB DUNGEON.C | 2735–2736 | CurrentMapWidth/Height = A.Width/A.Height + 1 |
| ReDMCSB LOADSAVE.C | 1643–1680 | Save: dungeon maps, SquareFirstThings, thing data, raw map data |
| ReDMCSB LOADSAVE.C | 1803–2165 | Load: F0434_STARTEND_IsLoadDungeonSuccessful_CPSC (compressed/uncompressed split, map loading) |
| ReDMCSB LOADSAVE.C | 1941–1943 | InitialPartyLocation decode: X, Y, Direction |
| ReDMCSB LOADSAVE.C | 2157–2163 | Column-pointer construction for DungeonMapData |
| ReDMCSB LOADSAVE.C | 1876–1895 | Compressed dungeon detection and DecompressedByteCount |
| ReDMCSB COMMAND.C | — | Champion transfer, reincarnation, prison/game routing |
| ReDMCSB CEDTINC8.C | 101–118 | CSBGAME.DAT vs DMSAVE.DAT routing |
| ReDMCSB CEDTINCH.C | 5–64 | Make-New-Adventure gate requires valid CSB dungeon |
| ReDMCSB CEDTINCU.C | 5–77 | F7272_IsDungeonValid — CSB prison/game ID validation |
| ReDMCSB HINTLOAD.C | 11–18, 300–386 | Atari CSB hint/runtime loader: HCSB.HTC, HCSB.DAT, CSBGAME.DAT |
| ReDMCSB FLOPPYST.C | 7–18 | Atari CSB save filenames |
| CSBWin CSBCode.cpp | 318–480 | DBank::Initialize (TAG00332a) — initialization, paths, save filenames |
| CSB lineage README | — | Utility flow, champion import, MINI.DAT |
| memory_dungeon_dat_pc34_compat.c | 80–212 | F0500_DUNGEON_LoadDatHeader_Compat with compressed dungeon support |
| memory_dungeon_dat_pc34_compat.c | 80–93 | F0501_DUNGEON_DecodePartyLocation_Compat |
| memory_dungeon_dat_pc34_compat.c | 322–341 | Dungeon file layout: header + maps + raw data + things |
| memory_dungeon_dat_pc34_compat.h | 19–22 | DUNGEON_HEADER_SIZE=44, DUNGEON_MAP_DESC_SIZE=16, DUNGEON_THING_TYPE_COUNT=16, DUNGEON_MAX_MAPS=32 |
| csb_v1_dungeon_loader_pc34_compat.c | — | Existing stub with column-major index comment |
| M13_PLAN.md | 303 | 24 levels in CSB vs 14 in DM1 |
| PARITY_MATRIX_CSB_V1.md | — | CSB front-door render smoke, save-file routing evidence |
| PROJEXPL.C (CHANGE7_20) | — | Projectile speed normalization on non-party maps |

---

## Findings Summary

1. **Dungeon format is shared**: DM1 and CSB both use DUNGEON_HEADER (44 bytes) + MAP descriptors (16 bytes) + column-major square data (2 bytes/sq). The shared structure is proven by ReDMCSB's unified LOADSAVE.C and DUNGEON.C.

2. **CSB file-level difference**: CSB DUNGEON.DAT is compressed (0x8104 signature, big-endian DecompressedByteCount) — decompression is already in `memory_dungeon_dat_pc34_compat.c`.

3. **Column-major layout confirmed**: `G0271_ppuc_CurrentMapData[x][y]`, square offset = `x * (height+1) * 2 + y * 2`. The "2098" in the hash table is a compressed file size, not a geometry number.

4. **24 levels**: CSB has up to 24 levels (vs DM1's 14). The level count comes from the MapCount field in DUNGEON_HEADER — 24 maps can be stored in one dungeon file.

5. **Wall format**: No separate geometry array. Walls are element type 0 in the 2-byte square record with additional aspect bits. Door state uses the low 3 bits of the square.

6. **CSB prison/game split**: DungeonID values 12 and 13 distinguish the prison and main game dungeons. Champion transfer is mediated by the prison dungeon's state and the `MINI.DAT` file.

7. **Start position**: Same encoding in both DM1 and CSB — 16-bit InitialPartyLocation with X[4:0], Y[9:5], Direction[11:10].

8. **Existing Firestaff code**: `memory_dungeon_dat_pc34_compat.c` already handles the compressed format and the DUNGEON_HEADER/MAP structures. CSB-specific additions needed: DungeonID routing, End Game Sensor type 18, Version Checker Sensor, and champion transfer state.
# DM1 V1 Data File Formats — Source Audit

## DUNGEON.DAT Format

### Source References
- ReDMCSB/LOADSAVE.C:1803–2070 — F0434_STARTEND_IsLoadDungeonSuccessful_CPSC (full load sequence)
- ReDMCSB/DEFS.H:975–1116 — DUNGEON_HEADER, COMPRESSED_DUNGEON_HEADER, MAP structs

### Header (44 bytes, offset 0)
  Offset 0  (2): OrnamentRandomSeed (uint16)
  Offset 2  (2): RawMapDataByteCount (uint16)
  Offset 4  (1): MapCount (uint8)
  Offset 5  (1): Unreferenced padding (Atari ST/Amiga alignment)
  Offset 6  (2): TextDataWordCount (uint16)
  Offset 8  (2): InitialPartyLocation (uint16) — dir(11-10), Y(9-5), X(4-0)
  Offset 10 (2): SquareFirstThingCount (uint16)
  Offset 12 (32): ThingCount[16] (uint16[16]) — counts per thing type

### FTL Compression Support (DEFS.H:975–984)
Signature 0x8104 at offset 0 triggers FTL decompression.
COMPRESSED_DUNGEON_HEADER: Signature(2) + DecompressedByteCount(4, big-endian) + DungeonID(2)
Decompression is performed before parse.

### Map Section (after 44 bytes header)
MAP[MapCount], each 16 bytes (DEFS.H:1048–1116):
  RawMapDataByteOffset (uint16, relative to end of SFT array)
  aUnreferenced, bUnreferenced (2x uint16)
  OffsetMapX, OffsetMapY (2x uint8)
  Bitfield A: Level(6)=raw&0x3F, Width-1(5)=(raw>>6)&0x1F, Height-1(5)=(raw>>11)&0x1F
  Bitfield B: WallOrnament(4), RandomWall(4), Floor(4), RandomFloor(4)
  Bitfield C: DoorOrnament(4), CreatureType(4), Unref(4), Difficulty(4)
  Bitfield D: FloorSet(4), WallSet(4), DoorSet0(4), DoorSet1(4)

### Tile Data (column-major)
Per-map tile data: square[col*Height+row] — each byte:
  Upper 3 bits (element type << 5): 0=WALL, 1=CORRIDOR, 2=PIT, 3=STAIRS, 4=DOOR, 5=TELEPORTER, 6=FAKEWALL
  Lower bits: element-specific data (ornament index, door state, etc.)

### File Load Offset Formula
  dataOffset = 44 + mapCount*16 + dungeonColumnCount*2 + squareFirstThingCount*2 + textDataWordCount*2 + sum(thingCounts[i]*thingDataByteCount[i])

### Thing Types (LOADSAVE.C:1996–2032)
  Type 0: Nothing/Empty
  Type 1: Chest (4 bytes each)
  Type 2: Weapon/Armor (4 bytes each)
  Type 3: Scroll/Book (4 bytes each)
  Type 4: Potion (4 bytes each)
  Type 5: Key (4 bytes each)
  Type 6: Magic user creature (8 bytes each)
  ... (up to type 15, documented in DEFS.H)

## GRAPHICS.DAT Format

### Source References
- ReDMCSB/DEFS.H:3000–3400 — GRAPHICS-related constants and struct definitions
- ReDMCSB/LOADSAVE.C:500–800 — Graphics loading
- ReDMCSB/BLIT*.C — Blitting/rendering code

GRAPHICS.DAT is a collection of sprite/screen graphics data:
  Screen bitmaps (title screen, menu backgrounds, ending sequence)
  Wall/floor/ceiling tiles (wall sets, floor sets, ceiling tiles)
  Creature sprites (frames, directions)
  Object/item sprites
  UI elements (portraits, icons, buttons)

No comprehensive GRAPHICS.DAT format documentation exists in ReDMCSB beyond
the loading code. The graphics are largely opaque without disassembly analysis.

## SOUND.DAT Format

### Source References
- ReDMCSB/DEFS.H:4000–4100 — Sound-related constants
- ReDMCSB/SOUND.C — Sound playback
- ReDMCSB/ADLIB*.C (if present) — AdLib/SB audio

DM1 V1 uses a custom sound subsystem. Sound format is not extensively documented
in ReDMCSB beyond the playback API.

## Modding Implications

### DUNGEON.DAT: Best Documented, Most Accessible
  Header format fully documented in ReDMCSB
  FTL compression documented
  Tile format documented
  Thing types partially documented
  Custom DUNGEON.DAT could be generated with reverse-engineering effort

### GRAPHICS.DAT: Poorly Documented
  Sprite format partially understood (planar EGA/VGA)
  Wall/floor tile encoding partially documented (BLIT*.C references)
  No sprite editor tooling in ReDMCSB
  Custom graphics would require custom encoder

### SOUND.DAT: Largely Unknown
  No documented format
  Custom sound would require audio format reverse-engineering

## References
- ReDMCSB/LOADSAVE.C:1803–2070 — Complete DUNGEON.DAT load sequence
- ReDMCSB/DEFS.H:975–1116 — DUNGEON_HEADER and MAP structs
- ReDMCSB/DEFS.H:659–705 — THING and CHAMPION structs
- ReDMCSB/DEFS.H:1001–1047 — Square element types and door states
- Firestaff/include/memory_dungeon_dat_pc34_compat.h — DungeonHeader_Compat, MAP structs

STATUS: DOCUMENTED — DUNGEON.DAT format is well-documented via ReDMCSB. GRAPHICS.DAT
and SOUND.DAT are poorly documented. Binary replacement of DUNGEON.DAT is the only
practical modding path; GRAPHICS.DAT and SOUND.DAT editing requires custom tooling.

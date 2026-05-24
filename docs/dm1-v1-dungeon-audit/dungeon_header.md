# DM1 V1 DUNGEON.DAT Header Format — Source Audit

## ReDMCSB Source
- DEFS.H lines 985–998: DUNGEON_HEADER struct definition
- LOADSAVE.C lines 1803–2070: F0434_STARTEND_IsLoadDungeonSuccessful_CPSC — full load sequence
- DEFS.H lines 1048–1116: MAP descriptor struct

## DUNGEON_HEADER Layout (DEFS.H:985–998)
44 bytes total:
- Offset 0: OrnamentRandomSeed (uint16)
- Offset 2: RawMapDataByteCount (uint16)
- Offset 4: MapCount (uint8)
- Offset 5: Unreferenced (uint8) — padding for alignment (Atari ST/Amiga)
- Offset 6: TextDataWordCount (uint16)
- Offset 8: InitialPartyLocation (uint16) — dir(11-10), Y(9-5), X(4-0)
- Offset 10: SquareFirstThingCount (uint16)
- Offset 12: ThingCount[16] (uint16[16])

### InitialPartyLocation Bitfield (DEFS.H:992)
Bits 11-10: Direction (0=N,1=E,2=S,3=W), Bits 9-5: Y, Bits 4-0: X

### Compressed Dungeon (DEFS.H:975–984)
0x8104 signature (big-endian: 04 81 on disk) triggers FTL decompression.
COMPRESSED_DUNGEON_HEADER: Signature(2) + DecompressedByteCount(4, BE) + DungeonID(2)

## Load Sequence (LOADSAVE.C:1930–2070)
1. Offset 0: Read DUNGEON_HEADER (44 bytes)
2. Offset 44: Read MAP[MapCount] (16 bytes each)
3. Read uint16[DungeonColumnCount] — cumulative square-first-thing count per column
4. Read THING[SquareFirstThingCount] — square-first-thing index array
5. Read uint16[TextDataWordCount] — text data (3 codes per word)
6. Per-thing-type: read thingCounts[type] * thingDataByteCount[type] bytes

DungeonColumnCount = sum(map[i].Width + 1) for all maps (DEFS.H:1058).

## MAP Descriptor Layout (DEFS.H:1048–1116, 16 bytes)
- RawMapDataByteOffset (uint16)
- aUnreferenced (uint16), bUnreferenced (uint16)
- OffsetMapX (uint8), OffsetMapY (uint8)
- Bitfield A (PC LSB-first): Level(6)=raw&0x3F, Width-1(5)=(raw>>6)&0x1F, Height-1(5)=(raw>>11)&0x1F
- Bitfield B: WallOrnament(4)=raw&0x0F, RandomWall(4)=(raw>>4)&0x0F, Floor(4)=(raw>>8)&0x0F, RandomFloor(4)=(raw>>12)&0x0F
- Bitfield C: DoorOrnament(4)=raw&0x0F, CreatureType(4)=(raw>>4)&0x0F, Unref(4)=(raw>>8)&0x0F, Difficulty(4)=(raw>>12)&0x0F
- Bitfield D: FloorSet(4)=raw&0x0F, WallSet(4)=(raw>>4)&0x0F, DoorSet0(4)=(raw>>8)&0x0F, DoorSet1(4)=(raw>>12)&0x0F

## Firestaff Implementation
File: include/memory_dungeon_dat_pc34_compat.h — DungeonHeader_Compat, DungeonMapDesc_Compat
Functions: F0500_DUNGEON_LoadDatHeader_Compat(), F0501_DUNGEON_DecodePartyLocation_Compat()
All DUNGEON_HEADER fields aligned. MAP bitfield decoding in decode_map_bitfield_a/b/c/d (lines 17-66).
Compressed: F0500 detects 0x8104, invokes ftl_decompress_dungeon(), applies BE byte-swapping.

STATUS: ALIGNED — Header and map descriptor parsing matches ReDMCSB LOADSAVE.C exactly.

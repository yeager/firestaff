# DM1 V1 Dungeon Terrain Types — Source Audit

## ReDMCSB Source
- DEFS.H:1001-1013: Element type constants C00_ELEMENT_WALL through C06_ELEMENT_FAKEWALL
- DEFS.H:1001: M034_SQUARE_TYPE(square) = (square) >> 5
- DEFS.H:1002: M035_SQUARE(element, mask) = ((element) << 5) | mask
- DEFS.H:1020-1034: Bit mask definitions per element type
- DUNGEON.C:2523-2660: F0172_DUNGEON_SetSquareAspect

## Element Types (7 total)

| Value | Name       | Attributes (lower 5 bits) |
|-------|------------|---------------------------|
| 0     | WALL       | MASK0x0001..0x0008 random ornament per side |
| 1     | CORRIDOR   | MASK0x0008 random floor ornament |
| 2     | PIT        | MASK0x0001 IMAGINARY, 0x0004 INVISIBLE, 0x0008 OPEN |
| 3     | STAIRS     | MASK0x0004 UP, 0x0008 NORTH_SOUTH_ORIENTATION |
| 4     | DOOR       | state 0-5 + ornament; 0x0008 N_S_ORIENTATION |
| 5     | TELEPORTER | 0x0004 VISIBLE, 0x0008 OPEN |
| 6     | FAKEWALL   | 0x0001 IMAGINARY, 0x0004 OPEN, 0x0008 FOOTPRINTS |

## Square Encoding (DEFS.H:1001)

Each dungeon square is one byte.
- Bits 7-5: Element type (0-6)
- Bits 4-0: Element-specific attributes

## Firestaff Implementation

include/memory_dungeon_dat_pc34_compat.h:79-86:
  DUNGEON_ELEMENT_WALL=0 through DUNGEON_ELEMENT_FAKEWALL=6, DUNGEON_ELEMENT_COUNT=7

src/memory/memory_dungeon_dat_pc34_compat.c:288-297:
  s_elementNames[7] = {Wall,Corridor,Pit,Stairs,Door,Teleporter,FakeWall}
  F0503_DUNGEON_GetElementName_Compat()

## STATUS: ALIGNED
Square cell layout matches ReDMCSB DEFS.H:1001-1034 exactly.

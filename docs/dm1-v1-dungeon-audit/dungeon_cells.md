# DM1 V1 Map Cell Data — Source Audit

## ReDMCSB Source
- DEFS.H:1001–1013: Square element types (M034_SQUARE_TYPE, M035_SQUARE macros)
- DEFS.H:1046–1047: M036_DOOR_STATE / M037_SET_DOOR_STATE
- LOADSAVE.C:303–418: Map tile loading (column-major format)

## Square Cell Format (DEFS.H:1001–1047)
M034_SQUARE_TYPE(square) = (square) >> 5

Element types: 0=WALL, 1=CORRIDOR, 2=PIT, 3=STAIRS, 4=DOOR, 5=TELEPORTER, 6=FAKEWALL
Element type stored as element<<5 in square byte.

Door state (lower 3 bits): 0=open,1=closed1/4,2=closed1/2,3=closed3/4,4=closed,5=destroyed
M036_DOOR_STATE(square) = square & 0x0007

Lower 5 bits interpretation per element:
- Wall/FakeWall: wallOrnamentIndex = square&0x0F, wallOrnamentRandom = (square>>4)&1
- Corridor: floorOrnamentIndex = square&0x0F, randomFloor = (square>>4)&1
- Door: doorState = square&0x07, doorOrnamentIndex = (square>>3)&0x0F

## Column-Major Storage (LOADSAVE.C:303–418)
Each map stored as Width columns of Height rows, column-major: square[col*Height+row]
DungeonColumnCount = sum(map[i].Width + 1) per map (LOADSAVE.C:1962-1970)
Note: +1 per map creates column-0 sentinel in cumulative SFT table.

## Tile Data File Offset (F0502_DUNGEON_LoadTileData_Compat)
rawDataFileOffset = 44 + mapCount*16 + totalColumns*2 + squareFirstThingCount*2 + textDataWordCount*2 + sum(thingCounts[i]*thingDataByteCount[i])
Each map RawMapDataByteOffset is relative to this raw data section.

## Firestaff Implementation
File: src/memory/memory_dungeon_dat_pc34_compat.c — F0502_DUNGEON_LoadTileData_Compat()
Element type constants in memory_dungeon_dat_pc34_compat.h: DUNGEON_ELEMENT_WALL(0) through DUNGEON_ELEMENT_FAKEWALL(6)
Column-major layout matches ReDMCSB. CreatureTypeCount array read after square data.

STATUS: ALIGNED — Square cell layout matches ReDMCSB DUNGEON.C / LOADSAVE.C exactly.

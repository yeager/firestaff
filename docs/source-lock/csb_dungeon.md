# CSB V1 - Dungeon Layout Changes Audit

## Source Paths
- CSB: /home/trv2/.openclaw/data/firestaff-csb-source/CSB/src/CSB.h, Data.h, Viewport.cpp
- DM1: ReDMCSB DUNGEON.C, DEFS.H, BugsAndChanges.htm

## Dungeon Structure (Shared)

Both DM1 and CSB use the same core dungeon system:
- Multiple dungeon maps (maps 0-N)
- Each map has 16x16 squares
- Party map (map 0) is the main map
- Maps connected by teleporters, pits, stairs
- FLOOR_BITMAP_SIZE = 7840 bytes (CSB.h:33)

## New Floor Sensor Types in CSB (DM1 did not have)

### 1. End Game Sensor (CHANGE7_21_IMPROVEMENT)
- DM1: No end game sensor
- CSB: New floor sensor type: end game
- File: TIMELINE.C (new in CSB)
- Effect: Game ends immediately when triggered
- CHANGE8_02_IMPROVEMENT: Optional delay before drawing end of game (2.1)

### 2. Version Checker Sensor (CHANGE7_23_IMPROVEMENT)
- New floor sensor type: version checker
- File: MOVESENS.C
- Triggered only if data value <= game engine version
- CHANGE8_06: Engine version 21 hardcoded for version 2.1
- Not used in original dungeon (custom dungeon feature)

## Map/Dungeon Count

- CSB Data.h: No explicit numMaps/DungeonEntrance count found in source grep
- DM1 uses C017_DUNGEON_MAP_COUNT in various files
- Both use 16x16 square grids per map
- No evidence CSB extends map grid size beyond 16x16

## Compressed Dungeon Support (CHANGE7_30_IMPROVEMENT)
- CSB adds support for compressed dungeons
- New type of temporary memory allocation for decompression
- Files: DECOMPDU.C, MEMORY.C, READWRIT.C, LOADSAVE.C
- DM1 did not support compressed dungeons

## Teleporter Behavior Change (CHANGE7_20_IMPROVEMENT)
- DM1: Projectiles move slower on maps other than party map
- CSB: Projectiles move at same speed on all maps
- File: PROJEXPL.C

## Other Structural

- No evidence of new dungeon floors/levels beyond what DM1 supports
- No new map connection mechanisms beyond teleporters/pits
- Floor decorations: floorDecorDesc[3][9] in Data.h same structure

## Conclusion
CSB does NOT extend the dungeon grid. The main dungeon changes are:
1. New end game sensor type
2. New version checker sensor type
3. Compressed dungeon support (new file format feature)
4. Projectile speed normalization across maps

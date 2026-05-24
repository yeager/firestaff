# DM1 V1 World Coordinates — Source Locked

## Finding: No Unified World Coordinate System

DM1 V1 does not use a global world-space tile grid that spans all dungeons.
Each dungeon complex is an independent unit. The world coordinates concept
only applies within a single dungeon complex for the purpose of matching
stairs to adjacent levels.

## Map Descriptor (MAP struct)

Source: memory_dungeon_dat_pc34_compat.h — struct DungeonMapDesc_Compat

Each map has:
  Level       : uint8  — dungeon depth (-1 to -14 typically)
  OffsetMapX  : uint8  — world X origin of this map's grid
  OffsetMapY  : uint8  — world Y origin of this map's grid
  Width       : uint8  — actual width  = stored_value + 1
  Height      : uint8  — actual height = stored_value + 1

- OffsetMapX/Y give the map's origin in world space
- Each map covers world grid [OffsetMapX, OffsetMapX+Width] × [OffsetMapY, OffsetMapY+Height]
- Coordinates are 0-indexed, local square coordinates are relative to map origin

## World Coordinate Computation (Stairs)

Source: DUNGEON.C:1537-1550:
  worldX = currentMap.OffsetMapX + localX
  worldY = currentMap.OffsetMapY + localY
  newLevel = currentMap.Level + levelDelta

  // Find target map at same world position on new level
  for each map in DungeonMaps:
    if map.Level == newLevel &&
       worldX >= map.OffsetMapX &&
       worldX <= map.OffsetMapX + map.Width &&
       worldY >= map.OffsetMapY &&
       worldY <= map.OffsetMapY + map.Height:
       localX = worldX - map.OffsetMapX
       localY = worldY - map.OffsetMapY

The world origin (0,0) is at the top-left. North = decreasing Y, East = increasing X.

## Coordinate Limits

- Map dimensions: max 256×256 (but typically much smaller)
- Map count: up to 32 maps per dungeon complex
- OffsetMapX/Y stored as uint8 — max 255, sufficient for dungeon complexity

## Entrance Map

Entrance (index 255) has no MAP descriptor in G0277_ps_DungeonMaps[]. It is
a special-cased screen rendered by ENTRANCE.C, not a tile map.

Source: DUNGEON.C:1534-1535 — blocks stairs usage while in entrance view.

## No Global World Grid

There is no global tile grid spanning all dungeon complexes. The world position
is meaningful only within a dungeon complex's map set. A stair from Dungeon A's
level -3 cannot land in Dungeon B's level -3 because there is no shared
coordinate system — the world positions are relative to each complex's origin.

## Coordinate Space Summary

| Space        | Reference   | Used For                    |
|--------------|-------------|-----------------------------|
| World X/Y    | Dungeon complex origin | Stairs level matching |
| Local X/Y    | MAP.OffsetMapX/Y | Party position within map  |
| Map index    | G0277_ps_DungeonMaps[] | Current dungeon map     |
| Level        | MAP.Level   | Depth z-axis (stairs delta) |

No source file defines a global world-space tile array covering multiple
dungeon sites. The game has no concept of visible overworld geography.

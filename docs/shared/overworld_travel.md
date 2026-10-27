# DM1 V1 Inter-Dungeon Travel — Source Locked

## 1. Stairs (Intradungeon Level Transitions)

Stairs are the primary mechanism for moving between dungeon levels.

Source: CLIKMENU.C:124-138
F0364_COMMAND_TakeStairs(P0733_B_StairsGoDown)
  P0733_B_StairsGoDown ? -1 : +1  ← level delta
  → F0154_DUNGEON_GetLocationAfterLevelChange(MapIndex, Delta, &X, &Y)
  → F0155_DUNGEON_GetStairsExitDirection(X, Y)
  → F0284_CHAMPION_SetPartyDirection(dir)

Source: DUNGEON.C:1508-1557 — F0154_DUNGEON_GetLocationAfterLevelChange

Input: current map index, level delta (±1), local X/Y on current map
Algorithm:
  newWorldX = currentMap.OffsetMapX + localX
  newWorldY = currentMap.OffsetMapY + localY
  newLevel  = currentMap.Level + levelDelta

  Scan all maps in G0277_ps_DungeonMaps[]
  For each map:
    if (map.Level == newLevel &&
        newWorldX >= map.OffsetMapX &&
        newWorldX <= map.OffsetMapX + map.Width &&
        newWorldY >= map.OffsetMapY &&
        newWorldY <= map.OffsetMapY + map.Height)
    → MATCH: return that map's index,
             local position = newWorldX/Y - map.OffsetMapX/Y

  If no match found: return CM1_MAP_INDEX_NONE (cannot descend further)

Stairs orientation: MASK0x0008_STAIRS_NORTH_SOUTH_ORIENTATION in square
attribute byte. N/S-oriented stairs advance EAST/WEST on the target map.
E/W-oriented stairs advance NORTH/SOUTH on the target map.

Source: CLIKMENU.C:167-179 — stairs sensor rules:
- Non-stairs → stairs: sensors fire on SOURCE square only (no arrival sensor)
- Stairs → non-stairs: sensors fire on DESTINATION square only (no departure sensor)
- Stairs → stairs (turning around on stairs): normal sensor processing both sides

## 2. Teleporter Things (In-Dungeon, Potentially Cross-Map)

Teleporters are THING_TYPE_TELEPORTER (1) objects with a targetMapIndex field
(8 bits, allowing up to 256 maps).

Source: memory_dungeon_dat_pc34_compat.h — struct DungeonTeleporter_Compat:
  targetMapIndex  : 8 bits  — destination map index
  targetMapX/Y    : 5 bits  — destination square X,Y on target map
  rotation        : 2 bits  — facing direction after teleport
  absoluteRotation: 1 bit   — rotation is absolute vs relative
  scope           : 2 bits  — 0=party only, 1=party+champions, etc.
  audible         : 1 bit   — play sound effect

Source: INPUT.C — Teleporter sensor types: SENSOR_FLOOR_TYPE_MIN to MAX.
When party steps on a teleporter sensor, the movement command system processes
the teleport event, setting G0327_i_NewPartyMapIndex.

Source: GAMELOOP.C:58-62:
  if (G0327_i_NewPartyMapIndex != CM1_MAP_INDEX_NONE) {
      F0267_MOVE_GetMoveResult_CPSCE(C0xFFFF_THING_PARTY, CM1_MAPX_NOT_ON_A_SQUARE, 0,
                                     G0306_i_PartyMapX, G0307_i_PartyMapY);
      G0327_i_NewPartyMapIndex = CM1_MAP_INDEX_NONE;
  }

Note from dm1_v1_field_teleporter_effect_pc34_compat.h:5:
  DM1 has no overworld/field map — teleporters are in-dungeon

## 3. Entrance/Exit Flow

Entrance Map (C255_MAP_INDEX_ENTRANCE = 255):
- Shown at title screen as the dungeon facade
- ENTRANCE.C:68 — G0309_i_PartyMapIndex = C255_MAP_INDEX_ENTRANCE
- On ENTER command: loads dungeon MAP 0 (or saved game's map) via
  F0174_DUNGEON_SetCurrentMapAndPartyMap()

Exit to Entrance:
- Special exit dungeon sensor or command transitions party to map index 255
- This shows the entrance view again; party can re-enter.

Source: DUNGEON.C:1534-1535:
  if (G0309_i_PartyMapIndex == C255_MAP_INDEX_ENTRANCE)
      return CM1_MAP_INDEX_NONE;  // stair use blocked in entrance view

## 4. No Seamless Inter-Dungeon Travel

There is no mechanism for walking from one dungeon complex to another on a shared
world map. Dungeon boundaries are hard limits. If a stair leads to a level
with no map covering the target world coordinates, F0154 returns
CM1_MAP_INDEX_NONE and the transition is blocked.

## Summary Table

| Mechanism       | Scope                  | Source Function              |
|-----------------|------------------------|------------------------------|
| Stairs up/down  | Within dungeon complex | F0154_DUNGEON_GetLocationAfterLevelChange |
| Teleporter      | Any map index (0-255)  | Sensor activation → GAMELOOP |
| Entrance exit   | → map 255              | ENTRANCE.C:68                |
| Entrance enter  | 255 → dungeon map 0    | F0174_DUNGEON_SetCurrentMapAndPartyMap |

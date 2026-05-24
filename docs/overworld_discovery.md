# DM1 V1 Map Discovery — Source Locked

## Finding: DM1 V1 Has No Fog-of-War

DM1 V1 (PC/Amiga/Atari ST) does NOT implement explored-area tracking or fog
of war. There is no memory map that records which tiles have been visited.

- m11_mark_explored() and m11_is_explored() exist only in the DM2 v2
  renderer (src/engine/m11_game_view.c:2697-2728).
- For DM1 V1, there is no exploredBits equivalent in any source file.

## DM1 V1 Rendering: Full Map Visibility

Source: m11_game_view.c:2757 (DM2 only, not DM1):
  memset(state->exploredBits, 0, sizeof(state->exploredBits));
  m11_mark_explored(state);

This pattern does not exist in DM1-compatible code.

Source: DUNGEON.C:1440-1470 — F0151_DUNGEON_GetSquare:
- When party is at the edge of a map and looks at an out-of-bounds coordinate,
  the function returns WALL (for corridor/wall boundary) or PIT (for pit boundary).
- These boundary adjustments are the ONLY form of edge awareness — no explored
  state is stored.

## Dungeon Viewport Rendering

The dungeon view (first-person perspective) is drawn entirely based on the
current G0271_ppuc_CurrentMapData[][] tile grid. There is no conditional
rendering based on whether a tile has been seen before.

Source: DUNVIEW.C:8606-8614 — guards against drawing dungeon view
when in entrance (map index 255), but no explored-tile guard.

## What Exists for Discovery

1. Viewport culling: Only tiles within the potentially-visible cone are
   rendered in the 3D view. This is view-geometry culling, not fog-of-war.

2. Object/creature activation range: Some sensors and creature AI only
   trigger when the party is within a certain radius. This is proximity-based,
   not visibility-based.

3. Sound: Some events (slamming doors, monster sounds) trigger based on
   proximity regardless of line-of-sight.

## M11 (DM2 v2 renderer) Has Fog-of-War

For reference only (DM2, not DM1):
- exploredBits — uint32 array, one bit per cell, marks visited tiles
- m11_mark_explored() — marks cells in view cone as explored
- m11_is_explored() — checks if a cell has been visited
- Fog of war is reset on new game and on certain dungeon-changing events

This is a DM2-specific feature not present in DM1 V1.

## Implication for Firestaff

DM1 V1's map discovery is binary: the entire map is either loaded or not loaded.
There is no per-cell explored tracking. Implementing fog-of-war for DM1 V1 would
require adding new state to M11_GameViewState and modifying the dungeon view
renderer to check that state before drawing tiles.
